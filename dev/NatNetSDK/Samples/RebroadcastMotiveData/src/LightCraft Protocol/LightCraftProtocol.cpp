/*
Copyright © 2016 NaturalPoint Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

/*

    This is code to run the lightcraft protocol. It is code that is called from 
    RebroadcastMotiveData (main) as one of the protocals that Transmits Motive 
    Data. It uses the ASIO library to send bytes through serial in the format
    specifically for the Previzion © software. 
    
    Process:
        - Connects to the NatNet Server by establishing a clent connected to the
            address specified from the command line
        - Sets up a serial port connection
        - Every frame calculates a vector for a rigid body (Z vector) and roll
            angle
        - Sends values for position, vector and roll angle through serial
            (Currently supports one rigid body)
        - Test mode (optional) print hexidecimal for positions and timecode 
            each frame

*/

#pragma once

#include "..\stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <stdio.h>
#include <tchar.h>
#include <conio.h>

#include <WinSock2.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <conio.h>

#include <iostream>
#include <asio.hpp>
#include <conio.h>
#include <ctime>
#include <string>
#include <vector>
#include <bitset>
#include <math.h>

#include "..\RebroadcastMotiveDataSystemInfo.h"
#include "NatNetClient.h"
#include "NatNetCAPI.h"
#include "NatNetTypes.h"
//#include "Core\Quaternion.h"
//#include "Core\Vector3.h"
#include "Quaternion.h"
#include "Vector3.h"
#include "HiTimer.h"
#include "NatNetCAPI.h"

#define SCK_VERSION1            0x0101
#define SCK_VERSION2            0x0202

#define SOCK_STREAM      1
#define SOCK_DGRAM      2
#define SOCK_RAW      3

#define AF_INET      2 

#define IPPROTO_TCP      6

// Globals
cSystemInfo G_systemInfo;

// NatNet Globals
void __cdecl LightcraftDataHandle( sFrameOfMocapData* data, void* pUserData );		// receives data from the server
void __cdecl LightcraftMessageHandler( int msgType, char* msg );		            // receives NatNet error mesages
void resetClient();
int LightcraftCreateClient( int iConnectionType );



// Set connection type here
int iConnectionType = ConnectionType_Multicast;
//int iConnectionType = ConnectionType_Unicast;

NatNetClient* LightcraftClient;

char LightcraftIPAddress[128] = "";
char LightcraftServerIPAddress[128] = "";
int analogSamplesPerMocapFrame = 0;


// local timing for verification
HiTimer timer;
double localDelta = 0.0f;

// Serial port Globals
asio::serial_port* port = nullptr;
asio::io_service io;
std::vector<float> serialPositions;
std::vector<unsigned _int8> serialtimeCode;
char lightcraftHeader( 0xD1 );
std::bitset<1> emptyByte( 0x0 );
std::bitset<16> emptyTwoByte( 0x0000 );
std::bitset<16> NaNTwoByte( 0xffff );
std::bitset<32> emptyFourByte( 0x00000000 );
std::bitset<32> NaNFourByte( 0xffffffff );
std::bitset<128> emptyFourteenBytes( 0x0000000000000000000000000000 );
std::bitset<32> testfloat1( 0xc0066666 );
float testfloat2 = -2.1f;

std::bitset<16> focus( 0x0019 );
std::bitset<16> iris( 0x0001 );
std::bitset<32> IOD( 0x40a00000 );
std::bitset<32> conv( 0x41200000 );


float serialRollDegrees;
unsigned long sum;
unsigned char * helper;     // Helper to properly send Bytes through serial
unsigned _int8 checkSum;

Core::cVector3f serialGlobalY( 0.0, 1.0, 0.0 );
Core::cVector3f serialGlobalZ( 0.0, 0.0, 1.0 );
Core::cVector3f serialLocalZ;
Core::cVector3f serialLocalY;
Core::cVector3f serialTempU( 0.0, 1.0, 0.0 );
Core::cQuaternionf serialQuaternion( 0.0, 0.0, 0.0, 1.0 );
// End serial port Globals

// Reverse Bytes of floating point numbers
float ReverseFloat( const float inFloat )
{
    float retVal;
    char *floatToConvert = (char*)& inFloat;
    char *returnFloat = (char*)& retVal;

    // swap the bytes into a temporary buffer
    returnFloat[0] = floatToConvert[3];
    returnFloat[1] = floatToConvert[2];
    returnFloat[2] = floatToConvert[1];
    returnFloat[3] = floatToConvert[0];

    return retVal;
}

// Serial Port Transmision 
void CBSSendToSerial()
{
    /*************************************************************************
    |Sending 48 BYTES over Serial Following SpyderCam Protocol (Prevision System):
    |
    |   Assumes:
    |       serialQuaternion is set
    |       serialTimeCode is set
    |       serialPositions is set
    |   Uses
    |       Vectors Gloabal Z, Gloabal Y
    |       serialQuaternion
    |   Sets
    |       Vector Local z, Local y, u (helper vector)
    |       Float rollDegrees
    |   Process:
    |       local z = Global Z rotated by serialQuaternion
    |       local y = Global Y rotated by serialQuaternion
    |       u = Global Y projected onto plane with normal vector local z
    |           then transformed onto the local coordinate system
    |       serialRollDegrees = two dimensional angle of vector u from y axis
    |       Then it reverses all float values for big endian transmission *DO not use any of the floats modified in this function* undefined behavior
    |
    |   Serial Sends:
    |       1 BYTE: (hex) D1
    |       1 BYTE: (unsigned _int8) Timecode Hours
    |       1 BYTE: (unsigned _int8) Timecode Minutes
    |       1 BYTE: (unsigned _int8) Timecode Seconds
    |       1 BYTE: (unsigned _int8) Timecode Frames
    |       4 BYTE: (float) local z-> x value
    |       4 BYTE: (float) local z-> y value
    |       4 BYTE: (float) local z-> z value
    |       4 BYTE: (float) local rollDegrees
    |       4 BYTE: (float) serialPositions[0]-> x position
    |       4 BYTE: (float) serialPositions[1]-> y position
    |       4 BYTE: (float) serialPositions[2]-> z position
    |       2 BYTE: (hex) 00 00
    |       2 BYTE: (hex) 00 00
    |       2 BYTE: (hex) 00 00
    |       4 BYTE: (hex) 00 00 00 00
    |       4 BYTE: (hex) 00 00 00 00
    |       1 BYTE: (unsigned _int8) sum of of Bytes sent
    |
    *************************************************************************/



    // Reset Global Y and helper vector
    serialGlobalY.SetValues( 0.0, 1.0, 0.0 );
    serialTempU.SetValues( 0.0, 1.0, 0.0 );

    // Reseting BTYE Count
    checkSum = 0;
    sum = '\0';

    // Calculate local y and local z vectors
    serialLocalY = (serialQuaternion.Rotate( serialGlobalY ));
    serialLocalZ = (serialQuaternion.Rotate( serialGlobalZ ));

    // Calculate u helper vector  
    serialGlobalY = ((serialLocalZ)* (serialTempU.Dot( serialLocalZ )));
    serialTempU = (serialTempU - serialGlobalY);
    serialQuaternion.Invert();
    serialTempU = (serialQuaternion.Rotate( serialTempU ));

    // Calculate Roll Angle
    serialRollDegrees = (((180.0f) / (3.14159f)) * atan2f( serialTempU.X(), serialTempU.Y() )); // Find angle and convert to degrees

    // Reverse BYTES of floats
    serialLocalZ.SetValues( ReverseFloat( serialLocalZ.X() ), ReverseFloat( serialLocalZ.Y() ), ReverseFloat( serialLocalZ.Z() ) );
    serialRollDegrees = ReverseFloat( serialRollDegrees );
    for ( unsigned int i = 0; i < serialPositions.size(); i++ )
    {
        serialPositions[i] = ReverseFloat( serialPositions[i] );
    }

    // Send values through serial
    // Send Header
    asio::write( *port, asio::buffer( &lightcraftHeader, 1 ) );

    sum += lightcraftHeader;

    // Send TimeCode
    for ( unsigned int i = 0; i < serialtimeCode.size(); i++ )
    {
        asio::write( *port, asio::buffer( &serialtimeCode[i], 1 ) );
        sum += serialtimeCode[i];
    }

    // Send Vector Values
    asio::write( *port, asio::buffer( &serialLocalZ.X(), 4 ) );
    helper = (unsigned char *)&serialLocalZ.X();
    sum += helper[0];
    sum += helper[1];
    sum += helper[2];
    sum += helper[3];

    asio::write( *port, asio::buffer( &serialLocalZ.Y(), 4 ) );
    helper = (unsigned char *)&serialLocalZ.Y();
    sum += helper[0];
    sum += helper[1];
    sum += helper[2];
    sum += helper[3];

    asio::write( *port, asio::buffer( &serialLocalZ.Z(), 4 ) );
    helper = (unsigned char *)&serialLocalZ.Z();
    sum += helper[0];
    sum += helper[1];
    sum += helper[2];
    sum += helper[3];

    // Send Roll
    asio::write( *port, asio::buffer( &serialRollDegrees, 4 ) );
    helper = (unsigned char *)&serialRollDegrees;
    sum += helper[0];
    sum += helper[1];
    sum += helper[2];
    sum += helper[3];

    // Send Positions

    for ( unsigned int i = 0; i < serialPositions.size(); i++ )
    {
        asio::write( *port, asio::buffer( &serialPositions[i], 4 ) );
        helper = (unsigned char *)&serialPositions[i];
        sum += helper[0];
        sum += helper[1];
        sum += helper[2];
        sum += helper[3];
    }

    // Calculate checkSum from running total of BYTES 
    // Sum = total value of BYTES mod 256
    checkSum = sum % 256;

    // Sending Empty bytes: Values for Zoom, Focus, Iris, Interocular ditance, Convergence (All IEEE 4 BYTE Floats)
    asio::write( *port, asio::buffer( &emptyFourteenBytes, 14 ) );

    // Send sum of bytes
    asio::write( *port, asio::buffer( &checkSum, 1 ) );

    // Flush the buffer
    if ( !PurgeComm( port->lowest_layer().native_handle(), PURGE_TXCLEAR ) )
    {
        CHAR szBuff[80];
        LPWSTR IpszFunction = nullptr;
        DWORD dw = GetLastError();

        sprintf_s( szBuff, "%s failed: GetLastError returned %u", IpszFunction, dw );

        ExitProcess( dw );
        return;
    }
}

// LightcraftMessageHandler receives NatNet error/debug messages
void __cdecl LightcraftMessageHandler( int msgType, char* msg )
{
    printf( "\n%s\n", msg );
}

// Establish a NatNet Client connection
int LightcraftCreateClient( int iConnectionType )
{

    // release previous server
    if ( LightcraftClient )
    {
        LightcraftClient->Disconnect();
        delete LightcraftClient;
    }

    // create NatNet client
    LightcraftClient = new NatNetClient( iConnectionType );

    // set the callback handlers
    LightcraftClient->SetMessageCallback( LightcraftMessageHandler );
    LightcraftClient->SetFrameReceivedCallback( LightcraftDataHandle, LightcraftClient );	// this function will receive data from the server and start the data handler

    // print version info
    unsigned char ver[4];
    LightcraftClient->NatNetVersion( ver );
    printf( "NatNet Sample Client (NatNet ver. %d.%d.%d.%d)\n", ver[0], ver[1], ver[2], ver[3] );

    // Init Client and connect to NatNet server
    // to use NatNet default port assignments
    int retCode = LightcraftClient->Initialize( LightcraftIPAddress, LightcraftServerIPAddress );
    // to use a different port for commands and/or data:
    //int retCode = LightcraftClient->Initialize(LightcraftIPAddress, LightcraftServerIPAddress, MyServersCommandPort, MyServersDataPort);
    if ( retCode != ErrorCode_OK )
    {
        printf( "Unable to connect to server.  Error code: %d. Exiting", retCode );
        return ErrorCode_Internal;
    }
    else
    {
        // get # of analog samples per mocap frame of data
        void* pResult;
        int ret = 0;
        int nBytes = 0;
        ret = LightcraftClient->SendMessageAndWait( "AnalogSamplesPerMocapFrame", &pResult, &nBytes );
        if ( ret == ErrorCode_OK )
        {
            analogSamplesPerMocapFrame = *((int*)pResult);
            printf( "Analog Samples Per Mocap Frame : %d", analogSamplesPerMocapFrame );
        }

        // print server info
        sServerDescription ServerDescription;
        memset( &ServerDescription, 0, sizeof( ServerDescription ) );
        LightcraftClient->GetServerDescription( &ServerDescription );
        if ( !ServerDescription.HostPresent )
        {
            printf( "Unable to connect to server. Host not present. Exiting." );
            return 1;
        }
        printf( "NatNet Client Server application info:\n" );
        printf( "Application: %s (ver. %d.%d.%d.%d)\n", ServerDescription.szHostApp, ServerDescription.HostAppVersion[0],
            ServerDescription.HostAppVersion[1], ServerDescription.HostAppVersion[2], ServerDescription.HostAppVersion[3] );
        printf( "NatNet Version: %d.%d.%d.%d\n", ServerDescription.NatNetVersion[0], ServerDescription.NatNetVersion[1],
            ServerDescription.NatNetVersion[2], ServerDescription.NatNetVersion[3] );
        printf( "Client IP:%s\n", LightcraftIPAddress );
        printf( "Server IP:%s\n", LightcraftServerIPAddress );
        printf( "Server Name:%s\n\n", ServerDescription.szHostComputerName );
    }

    return ErrorCode_OK;

}


// LightcraftDataHandle receives data from the server
void __cdecl LightcraftDataHandle( sFrameOfMocapData* data, void* pUserData )
{
    /****************************************************************************************
    |
    | Data Handler for sending data to serial port
    |   Assumes:
    |       G_systemInfo is set correctly
    |       serialPositions is declared to nulls
    |       serialTimeCode is declared to nulls
    |       port is defined
    |       serialQuaternion is defined
    |       LightcraftClient is working
    |   Uses:
    |       G_systemInfo
    |       bIsRecording
    |       bTrackedModeIsChanged
    |       data
    |   Sets:
    |       bIsRecording
    |       bTrackedModeIsChanged
    |       serialPositions
    |       serialTimeCode
    |       serialQuaternion
    |   Processes:
    |       Test mode to print additional information
    |       Set data needed to send to serial port
    |       Calls function to send raw bit stream
    |
    ****************************************************************************************/
    NatNetClient* pClient = (NatNetClient*)pUserData;

    int i = 0;


    // Portocals for added print statements in test mode *REDUCED PERFORMANCE* 
    if ( G_systemInfo.InTestMode() )
    {
        // FrameOfMocapData params
        bool bIsRecording = ((data->params & 0x01) != 0);
        bool bTrackedModelsChanged = ((data->params & 0x02) != 0);
        if ( bIsRecording )
            printf( "RECORDING\n" );
        if ( bTrackedModelsChanged )
            printf( "Models Changed.\n" );


        // timecode - for systems with an eSync and SMPTE timecode generator - decode to values
        int hour, minute, second, frame, subframe;
        bool bValid = pClient->DecodeTimecode( data->Timecode, data->TimecodeSubframe, &hour, &minute, &second, &frame, &subframe );
        // decode to friendly string
        char szTimecode[128] = "";
        pClient->TimecodeStringify( data->Timecode, data->TimecodeSubframe, szTimecode, 128 );
        printf( "Timecode : %s\n", szTimecode );

        // Rigid Bodies
        printf( "Rigid Bodies [Count=%d]\n", data->nRigidBodies );
        for ( i = 0; i < data->nRigidBodies; i++ )
        {
            if ( data->RigidBodies[i].ID == 42 )
            {
                // params
                // 0x01 : bool, rigid body was successfully tracked in this frame
                bool bTrackingValid = data->RigidBodies[i].params & 0x01;

                printf( "Rigid Body [ID=%d  Error=%3.2f  Valid=%d]\n", data->RigidBodies[i].ID, data->RigidBodies[i].MeanError, bTrackingValid );
                printf( "\tx\ty\tz\tqx\tqy\tqz\tqw\n" );
                printf( "\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\n",
                    data->RigidBodies[i].x,
                    data->RigidBodies[i].y,
                    data->RigidBodies[i].z,
                    data->RigidBodies[i].qx,
                    data->RigidBodies[i].qy,
                    data->RigidBodies[i].qz,
                    data->RigidBodies[i].qw );

                // Code to send data through serial port for cbs digital *SUPPORTS ONE RIGID BODY* 

                // Setting new values into our quaternion
                timer.Start(); // Testing elapsed time for serial port sending

                serialQuaternion.SetValues( data->RigidBodies[i].qx, data->RigidBodies[i].qy, data->RigidBodies[i].qz, data->RigidBodies[i].qw );

                serialPositions[0] = (data->RigidBodies[i].x * 100);
                serialPositions[1] = (data->RigidBodies[i].y * 100);
                serialPositions[2] = (data->RigidBodies[i].z * 100);

                serialtimeCode[0] = hour;
                serialtimeCode[1] = minute;
                serialtimeCode[2] = second;
                serialtimeCode[3] = frame;

                // Function that sends raw bitstream
                CBSSendToSerial();

                timer.Stop();

                localDelta = timer.Duration();

                float f;
                f = serialPositions[0];
                printf( "\n%X x sent in hex", *(unsigned int*)&f );
                f = serialPositions[1];
                printf( "\n%X y sent in hex", *(unsigned int*)&f );
                f = serialPositions[2];
                printf( "\n%X z sent in hex", *(unsigned int*)&f );

                printf( "\nTime Duration of send to serial function in miliseconds: %f\n", (localDelta * 1000) );
            }
        }
    }
    // END Test Mode

    else
    {
        for ( i = 0; i < data->nRigidBodies; i++ )
        {

            if ( data->RigidBodies[i].ID == 42 )
            {
                // Code to send data through serial port for cbs digital *SUPPORTS ONE RIGID BODY*

                // Setting new values into our quaternion
                serialQuaternion.SetValues( data->RigidBodies[i].qx, data->RigidBodies[i].qy, data->RigidBodies[i].qz, data->RigidBodies[i].qw );

                // XYZ position
                serialPositions[0] = data->RigidBodies[i].x * 100;
                serialPositions[1] = data->RigidBodies[i].y * 100;
                serialPositions[2] = data->RigidBodies[i].z * 100;

                // TimeCode
                int hour, minute, second, frame, subframe;
                bool bValid = pClient->DecodeTimecode( data->Timecode, data->TimecodeSubframe, &hour, &minute, &second, &frame, &subframe );
                serialtimeCode[0] = hour;
                serialtimeCode[1] = minute;
                serialtimeCode[2] = second;
                serialtimeCode[3] = frame;

                // Function that sends raw bitstream
                CBSSendToSerial();
            }
        }

    }
}


// Main function to be called from RebroadcastMotiveData
int RunLightcraftSender( const cSystemInfo& G_systemInfo )
{
    /**************************************************************************************************
    |
    | Lightcraft Sending sends data through serial port
    |
    |   Assumes:
    |       Serial port baud rate is 115200
    |       Motive is running on local machine using local IpAddress
    |       Motive using one Rigid Body
    |   Uses:
    |       G_systemInfo
    |       serial port pointer: port
    |       Global Connection type: iConnectionType
    |       LightcraftClient
    |   Sets:
    |       serialPort
    |       port
    |       serialPositions: Null data
    |       serialtimecode: Null dat
    |       serialQuaternion: Abritrary data
    |       LightcraftServerIPAddress
    |       LightcraftIPAddress
    |   Processes:
    |       Create NatNet client to communicate with Motive (create client starts data handler)
    |       A test to check client communication
    |       Runs main loop
    |
    **************************************************************************************************/

    

    try
    {

        printf( "[Lightcraft] Motive->Previzion setting up data relay\n" );

        // Create link to port and open connection
        asio::serial_port serialPort( io, G_systemInfo.getPort() );
        port = &serialPort;
        if ( port->is_open() )
            port->close();
        port->open( G_systemInfo.getPort() );
        port->set_option( asio::serial_port_base::baud_rate( 115200 ) );
        port->set_option( asio::serial_port_base::parity( asio::serial_port_base::parity::none ) );
        port->set_option( asio::serial_port_base::flow_control( asio::serial_port_base::flow_control::none ) );
        port->set_option( asio::serial_port_base::stop_bits( asio::serial_port_base::stop_bits::one ) );
        port->set_option( asio::serial_port_base::character_size( 8 ) );

        // Declare global arrays for storing data
        serialPositions = { NULL /*x position*/, NULL /*y position*/, NULL /*z position*/ };
        serialtimeCode = { NULL /*HOURS*/, NULL /*MINUTES*/, NULL /*SECONDS*/, NULL /*FRAMES*/ };



        // ****************************************************** testing of print from nat net stream
        int iResult;
        
        // Setup client for NatNet server assumed on local machine with same ipAddress 
        strcpy_s( LightcraftServerIPAddress, G_systemInfo.getIPAddress() );
        printf( "Connecting to server at %s...\n", LightcraftServerIPAddress );
        strcpy_s( LightcraftIPAddress, G_systemInfo.getIPAddress() );
        printf( "Connecting from %s...\n", LightcraftIPAddress );


        // Create NatNet Client
        iResult = LightcraftCreateClient( iConnectionType ); // Creates the client which in turn starts the data handler
        // Data handler is running and sending data through serial port communication
        if ( iResult != ErrorCode_OK )
        {
            printf( "Error initializing client.  See log for details.  Exiting" );
            return 1;
        }
        else
        {
            printf( "Client initialized and ready.\n" );
        }

        // Variables for message reporting
        void* response;
        int nBytes;

        if ( G_systemInfo.InTestMode() )
        {
            // Send/receive test request
            printf( "NatNet Client Sending Test Request\n" );

            iResult = LightcraftClient->SendMessageAndWait( "TestRequest", &response, &nBytes );
            if ( iResult == ErrorCode_OK )
            {
                printf( "NatNet Client Received: %s", (char*)response );
            }
        }


        // Ready to receive marker stream!
        printf( "\nClient is connected to server and listening for data..." );
        printf( "\nSending data through to serial..." );
        printf( "\nPress 'Q' to quit: " );


        char c; // Character for user interaction

        // Loop to keep main thread open while LightcraftDataHandle thread runs
        while ( c = _getch() )
        {
            if ( c == 'q' )
            {
                // End Process
                break;
            }
        }

        // Done - clean up.
        LightcraftClient->SendMessageAndWait( "Disconnect", &response, &nBytes );
        printf( "\nShutting down serial port communication..." );
        LightcraftClient->Uninitialize();
        port->close();


        return ErrorCode_OK;

    }
    // Error Handling for serial communication
    catch ( asio::system_error& e )
    {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }

}