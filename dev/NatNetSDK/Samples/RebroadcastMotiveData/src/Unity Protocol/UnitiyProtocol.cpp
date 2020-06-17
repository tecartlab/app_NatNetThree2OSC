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

UnitySample.cpp

This program connects to a NatNet server, receives a data stream, encodes a skeleton to XML, and
outputs XML locally over UDP to Unity.  The purpose is to illustrate how to get data into Unity3D.

Usage [optional]:

UnitySample [ServerIP] [LocalIP] [Unity3D IP]

[ServerIP]			IP address of the server (e.g. 192.168.0.107) ( defaults to local machine)
*/
#pragma once

#include "..\stdafx.h"
#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <winsock2.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "NatNetTypes.h"
#include "NatNetClient.h"
#include "NatNetCAPI.h"
#include "..\RebroadcastMotiveDataSystemInfo.h"


#include "NatNetRepeater.h"   //== for transport of data over UDP to Unity3D
//== Slip Stream globals ==--
cSlipStream *gSlipStream;
std::map<int, std::string> gBoneNames;

void __cdecl DataHandler( sFrameOfMocapData* data, void* pUserData );   // receives data from the server
void __cdecl MessageHandler( Verbosity msgType, const char* msg );     // receives NatNet error messages
void resetClient();
int CreateClient( ConnectionType connectionType );

NatNetClient* theClient;
FILE* fp;

char szMyIPAddress[128] = "";
char szServerIPAddress[128] = "";
char szUnityIPAddress[128] = "";
//== Helpers for text indicator of data flowing... ==--

enum eStatusIndicator
{
    Uninitialized = 0,
    Listening,
    Streaming
};

eStatusIndicator gIndicator;

double gIndicatorTimer = 0;
double gFrequency = 0;

long long getTickCount()
{
    long long   curTime;
    QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>(&curTime) );
    return curTime;
}

void getFrequency()
{
    if ( gFrequency == 0 )
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency( &freq );
        gFrequency = double( freq.QuadPart );

        gIndicatorTimer = (double)getTickCount();
    }
}

double elapsedTimer()
{
    getFrequency();
    long long   curTime = getTickCount();
    return (double)(curTime - gIndicatorTimer) / gFrequency;
}

void catchUp()
{
    gIndicatorTimer = (double)getTickCount();
}


void SendDescriptionsToUnity( sDataDescriptions * dataDescriptions );

void runUnityProtocol( const cSystemInfo& systemInfo)
{
    int iResult;

    printf( "[UnityClient] Motive->Unity3D setting up relay\n" );
    // parse command line args
        strcpy_s( szServerIPAddress, systemInfo.getIPAddress() );	// specified on command line
        printf( "[UnityClient] Connecting to server at %s\n", szServerIPAddress );
        strcpy_s( szServerIPAddress, "127.0.0.1" );
    
        strcpy_s( szUnityIPAddress, systemInfo.getPort() );	    // specified on command line
        strcpy_s( szUnityIPAddress, "127.0.0.1" );
        printf( "[UnityClient] Connecting to Unity3D at %s\n", szUnityIPAddress );
    
        strcpy_s( szMyIPAddress, "127.0.0.1" );          // assume server is local machine
        printf( "[UnityClient] Connecting from LocalMachine\n" );

    // Create SlipStream
    gSlipStream = new cSlipStream( szUnityIPAddress, 16000 );

    // Create NatNet Client
    iResult = CreateClient( ConnectionType_Multicast );

    if ( iResult != ErrorCode_OK )
    {
        printf( "Error initializing client.  See log for details.  Exiting" );
        return;
    }

    // Retrieve Data Descriptions from server
    printf( "[UnityClient] Requesting assets from Motive\n" );
	sDataDescriptions* pTemporaryDefs = NULL;
	iResult = theClient->GetDataDescriptionList(&pTemporaryDefs);
	
	// Get a copy of the data descriptions
	sDataDescriptions* pDataDefs = pTemporaryDefs;
    
    if ( iResult != ErrorCode_OK || pDataDefs == NULL )
    {
        printf( "[UnityClient] Unable to retrieve avatars\n" );
    }
    else
    {
        int skeletonCount = 0;
        int rigidBodyCount = 0;

        for ( int i = 0; i < pDataDefs->nDataDescriptions; i++ )
        {
            if ( pDataDefs->arrDataDescriptions[i].type == Descriptor_Skeleton )
            {
                // Skeleton
                skeletonCount++;
                sSkeletonDescription* pSK = pDataDefs->arrDataDescriptions[i].Data.SkeletonDescription;
                printf( "[UnityClient] Received skeleton description: %s\n", pSK->szName );
                for ( int j = 0; j < pSK->nRigidBodies; j++ )
                {
                    sRigidBodyDescription* pRB = &pSK->RigidBodies[j];
                    // populate bone name dictionary for use in xml ==--
                    gBoneNames[pRB->ID + pSK->skeletonID * 100] = pRB->szName;
                }
            }
            if ( pDataDefs->arrDataDescriptions[i].type == Descriptor_RigidBody )
            {
                rigidBodyCount++;
                sRigidBodyDescription* pRB = pDataDefs->arrDataDescriptions[i].Data.RigidBodyDescription;
                printf( "[UnityClient] Received rigid body description: %s\n", pRB->szName );

            }
        }

        //printf( "[UnityClient] Received %d Skeleton Description(s)\n", skeletonCount );
        //printf( "[UnityClient] Received %d Rigid Body Description(s)\n", rigidBodyCount );

        SendDescriptionsToUnity( pDataDefs );
    }

    // Ready to receive marker stream!
    printf( "[UnityClient] Connected to server and ready to relay data to Unity3D\n" );
    printf( "[UnityClient] Listening for first frame of data...\n" );
    gIndicator = Listening;

    int c;
    bool bExit = false;
    while ( !bExit )
    {
        while ( !_kbhit() )
        {
            Sleep( 10 );

            if ( gIndicator == Streaming )
            {
                if ( elapsedTimer()>1.0 )
                {
                    printf( "[UnityClient] Data stream stalled.  Listening for more frame data...\n" );
                    gIndicator = Listening;
                }
            }
        }

        c = _getch();

        switch ( c )
        {
        case 'q':
            bExit = true;
            break;
        case 'r':
            resetClient();
            break;
        case 'p':
            sServerDescription ServerDescription;
            memset( &ServerDescription, 0, sizeof( ServerDescription ) );
            theClient->GetServerDescription( &ServerDescription );
            if ( !ServerDescription.HostPresent )
            {
                printf( "Unable to connect to server. Host not present. Exiting." );
                return;
            }
            break;
        case 'm':	                        // change to multicast
            iResult = CreateClient( ConnectionType_Multicast );
            if ( iResult == ErrorCode_OK )
                printf( "Client connection type changed to Multicast.\n\n" );
            else
                printf( "Error changing client connection type to Multicast.\n\n" );
            break;
        case 'u':	                        // change to unicast
            iResult = CreateClient( ConnectionType_Unicast );
            if ( iResult == ErrorCode_OK )
                printf( "Client connection type changed to Unicast.\n\n" );
            else
                printf( "Error changing client connection type to Unicast.\n\n" );
            break;


        default:
            break;
        }
        if ( bExit )
            break;
    }

    // Done - clean up.
    theClient->Disconnect();

    return;
}

// Establish a NatNet Client connection
int CreateClient( ConnectionType connectionType )
{
    // release previous server
    if ( theClient )
    {
        theClient->Disconnect();
        delete theClient;
    }

    // create NatNet client
    theClient = new NatNetClient( connectionType );

    // print version info
    unsigned char ver[4];
    NatNet_GetVersion( ver );

    // Set callback handlers
    NatNet_SetLogCallback( MessageHandler );
    theClient->SetFrameReceivedCallback( DataHandler, theClient );	// this function will receive data from the server

    // Init Client and connect to NatNet server
    // to use NatNet default port assigments
    sNatNetClientConnectParams connectParams;
    connectParams.localAddress = szMyIPAddress;
    connectParams.serverAddress = szServerIPAddress;
    int retCode = theClient->Connect( connectParams );
    // to use a different port for commands and/or data:
    //int retCode = theClient->Initialize(szMyIPAddress, szServerIPAddress, MyServersCommandPort, MyServersDataPort);
    if ( retCode != ErrorCode_OK )
    {
        printf( "Unable to connect to server.  Error code: %d. Exiting", retCode );
        return ErrorCode_Internal;
    }
    else
    {
        // print server info
        sServerDescription ServerDescription;
        memset( &ServerDescription, 0, sizeof( ServerDescription ) );
        theClient->GetServerDescription( &ServerDescription );
        if ( !ServerDescription.HostPresent )
        {
            printf( "Unable to connect to server. Host not present. Exiting." );
            return 1;
        }
        printf( "[UnityClient] Server    : %s\n", ServerDescription.szHostComputerName );
        printf( "[UnityClient] Server  IP: %s\n", szServerIPAddress );
        printf( "[UnityClient] Unity3D IP: %s\n", szUnityIPAddress );
        printf( "[UnityClient] Current IP: %s\n", szMyIPAddress );
    }

    return ErrorCode_OK;

}

void SendDescriptionsToUnity( sDataDescriptions * dataDescriptions )
{
    //== lets send the skeleton descriptions to Unity ==--

    std::vector<int> skeletonDescriptions;

    for ( int index = 0; index<dataDescriptions->nDataDescriptions; index++ )
    {
        if ( dataDescriptions->arrDataDescriptions[index].type == Descriptor_Skeleton )
        {
            skeletonDescriptions.push_back( index );
        }
    }

    //== early out if no skeleton descriptions to stream ==--

    if ( skeletonDescriptions.size() == 0 )
    {
        return;
    }

	// Flush buffer
	std::cout.flush();

    //== now we stream skeleton descriptions over XML ==--

    std::ostringstream xml;

    xml << "<?xml version=\"1.0\" ?>\n";
    xml << "<Stream>\n";
    xml << "<SkeletonDescriptions>\n";

    // skeletons first

    for ( int descriptions = 0; descriptions<(int)skeletonDescriptions.size(); descriptions++ )
    {
        int index = skeletonDescriptions[descriptions];

        // Skeleton
        sSkeletonDescription* pSK = dataDescriptions->arrDataDescriptions[index].Data.SkeletonDescription;

        xml << "<SkeletonDescription ID=\"" << pSK->skeletonID << "\" ";
        xml << "Name=\"" << pSK->szName << "\" ";
        xml << "BoneCount=\"" << pSK->nRigidBodies << "\">\n";

        for ( int j = 0; j < pSK->nRigidBodies; j++ )
        {
			std::cout.flush();

            sRigidBodyDescription* pRB = &pSK->RigidBodies[j];

            xml << "<BoneDefs ID=\"" << pRB->ID << "\" ";

            xml << "ParentID=\"" << pRB->parentID << "\" ";
            xml << "Name=\"" << pRB->szName << "\" ";
            xml << "x=\"" << pRB->offsetx << "\" ";
            xml << "y=\"" << pRB->offsety << "\" ";
            xml << "z=\"" << pRB->offsetz << "\"/>\n";
        }

        xml << "</SkeletonDescription>\n";
    }

    xml << "</SkeletonDescriptions>";
    xml << "</Stream>\n";

	std::cout.flush();
    // convert xml document into a buffer filled with data ==--

    std::string str = xml.str();
    const char* buffer = str.c_str();

    // stream xml data over UDP via SlipStream ==--

    gSlipStream->Stream( (unsigned char *)buffer, (int)strlen( buffer ) );
}



// Create XML from frame data and output to Unity
void SendFrameToUnity( sFrameOfMocapData *data, void *pUserData )
{
    if ( data->Skeletons>0 )
    {
		std::cout.flush();

        std::ostringstream xml;

        xml << "<?xml version=\"1.0\" ?>\n";
        xml << "<Stream>\n";
        xml << "<Skeletons>\n";

        for ( int i = 0; i<data->nSkeletons; i++ )
        {
            sSkeletonData skData = data->Skeletons[i]; // first skeleton ==--

            xml << "<Skeleton ID=\"" << skData.skeletonID << "\">\n";

            for ( int i = 0; i<skData.nRigidBodies; i++ )
            {
				std::cout.flush();

                sRigidBodyData rbData = skData.RigidBodyData[i];

                xml << "<Bone ID=\"" << rbData.ID << "\" ";
                xml << "Name=\"" << gBoneNames[LOWORD( rbData.ID ) + skData.skeletonID * 100].c_str() << "\" ";
                xml << "x=\"" << rbData.x << "\" ";
                xml << "y=\"" << rbData.y << "\" ";
                xml << "z=\"" << rbData.z << "\" ";
                xml << "qx=\"" << rbData.qx << "\" ";
                xml << "qy=\"" << rbData.qy << "\" ";
                xml << "qz=\"" << rbData.qz << "\" ";
                xml << "qw=\"" << rbData.qw << "\" />\n";
            }

            xml << "</Skeleton>\n";
        }

        xml << "</Skeletons>\n";
        xml << "<RigidBodies>\n";

        // rigid bodies ==--

        for ( int i = 0; i<data->nRigidBodies; i++ )
        {
            sRigidBodyData rbData = data->RigidBodies[i];

            xml << "<RigidBody ID=\"" << rbData.ID << "\" ";
            xml << "x=\"" << rbData.x << "\" ";
            xml << "y=\"" << rbData.y << "\" ";
            xml << "z=\"" << rbData.z << "\" ";
            xml << "qx=\"" << rbData.qx << "\" ";
            xml << "qy=\"" << rbData.qy << "\" ";
            xml << "qz=\"" << rbData.qz << "\" ";
            xml << "qw=\"" << rbData.qw << "\" />\n";
        }


        xml << "</RigidBodies>\n";
        xml << "</Stream>\n";

		std::cout.flush();

        std::string str = xml.str();
        const char* buffer = str.c_str();

        // stream xml data over UDP via SlipStream ==--

        gSlipStream->Stream( (unsigned char *)buffer, (int)strlen( buffer ) );
    }
}


double gLastDescription = 0;

// DataHandler receives data from the server
void NATNET_CALLCONV DataHandler( sFrameOfMocapData* data, void* pUserData )
{
    NatNetClient* pClient = (NatNetClient*)pUserData;

    catchUp();

    if ( gIndicator == Listening )
    {
        gIndicator = Streaming;
        printf( "[UnityClient] Receiving data and streaming to Unity3D\n" );
    }

    SendFrameToUnity( data, pUserData );

    double timer = data->fTimestamp;

    if ( timer - gLastDescription > 1 || gLastDescription > timer )
    {
        //== stream skeleton definitions once per second ==--
        gLastDescription = timer;
        sDataDescriptions* pDataDefs = NULL;
        ErrorCode result = theClient->GetDataDescriptionList( &pDataDefs );
        if ( result == ErrorCode_OK && pDataDefs != NULL )
        {
            for ( int i = 0; i < pDataDefs->nDataDescriptions; i++ )
            {
                if ( pDataDefs->arrDataDescriptions[i].type == Descriptor_Skeleton )
                {
                    // Skeleton
                    sSkeletonDescription* pSK = pDataDefs->arrDataDescriptions[i].Data.SkeletonDescription;
                    for ( int j = 0; j < pSK->nRigidBodies; j++ )
                    {
                        sRigidBodyDescription* pRB = &pSK->RigidBodies[j];
                        // populate bone name dictionary for use in xml ==--
                        gBoneNames[pRB->ID + pSK->skeletonID * 100] = pRB->szName;
                    }
                }
            }
            SendDescriptionsToUnity( pDataDefs );
            NatNet_FreeDescriptions( pDataDefs );
        }
    }
}

// MessageHandler receives NatNet error/debug messages
void NATNET_CALLCONV MessageHandler( Verbosity msgType, const char* msg )
{
    printf( "\n%s\n", msg );
}

void resetClient()
{
    int iSuccess;

    printf( "\n\nre-setting Client\n\n." );

    iSuccess = theClient->Disconnect();
    if ( iSuccess != 0 )
        printf( "error un-initting Client\n" );

    sNatNetClientConnectParams connectParams;
    connectParams.localAddress = szMyIPAddress;
    connectParams.serverAddress = szServerIPAddress;
    iSuccess = theClient->Connect( connectParams );
    if ( iSuccess != 0 )
        printf( "error re-initting Client\n" );
}