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

#include "stdafx.h"
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
#include <asio.hpp>
#include <ctime>
#include <bitset>
#include <math.h>

#include "RebroadcastMotiveDataSystemInfo.h"
#include "NatNetClient.h"
#include "NatNetTypes.h"
#include "HiTimer.h"
#include "Vector3.h"
#include "Quaternion.h"

#define SCK_VERSION1            0x0101
#define SCK_VERSION2            0x0202

#define SOCK_STREAM      1
#define SOCK_DGRAM      2
#define SOCK_RAW      3

#define AF_INET      2 

#define IPPROTO_TCP      6


// Reverse Bytes of floating point numbers
float ReverseFloat( const float inFloat );

// Serial Port Transmision 
void CBSSendToSerial();

    /*************************************************************************
    |Sending 48 BYTES over Serial:
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

// MessageHandler receives NatNet error/debug messages
void __cdecl MessageHandler( int msgType, char* msg );

// Establish a NatNet Client connection
int LightcraftCreateClient( int iConnectionType );

// DataHandler receives data from the server
void __cdecl DataHandler( sFrameOfMocapData* data, void* pUserData );
    /****************************************************************************************
    |
    | Data Handler for sending data to serial port
    |   Assumes:
    |       systemInfo is set correctly
    |       serialPositions is declared to nulls
    |       serialTimeCode is declared to nulls
    |       port is defined
    |       serialQuaternion is defined
    |       theClient is working
    |   Uses:
    |       systemInfo
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

// Main function to be called from RebroadcastMotiveData
int RunLightcraftSender( const cSystemInfo &info );
    /**************************************************************************************************
    |
    | Lightcraft Sending sends data through serial port
    |
    |   Assumes:
    |       Serial port baud rate is 115200
    |       Motive is running on local machine using local IpAddress
    |       Motive using one Rigid Body
    |   Uses:
    |       SystemInfo
    |       serial port pointer: port
    |       Global Connection type: iConnectionType
    |       theClient
    |   Sets:
    |       serialPort
    |       port
    |       serialPositions: Null data
    |       serialtimecode: Null dat
    |       serialQuaternion: Abritrary data
    |       szServerIPAddress
    |       szMyIPAddress
    |   Processes:
    |       Create NatNet client to communicate with Motive (create client starts data handler)
    |       A test to check client communication
    |       Runs main loop
    |
    **************************************************************************************************/
