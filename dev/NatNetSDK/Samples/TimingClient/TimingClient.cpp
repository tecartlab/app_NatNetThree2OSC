/* 
Copyright © 2014 NaturalPoint Inc.

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

TimingClient.cpp

This program connects to a NatNet server and can be used as a quick check to determine packet timing information.

Usage [optional]:

	TimingClient [ServerIP] [LocalIP] [OutputFilename]

	[ServerIP]			IP address of the server (e.g. 192.168.0.107) ( defaults to local machine)
	[OutputFilename]	Name of points file (pts) to write out.  defaults to Client-output.pts

*/

#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <winsock2.h>

#include "NatNetTypes.h"
#include "NatNetCAPI.h"
#include "NatNetClient.h"
#include "HiTimer.h"

#pragma warning( disable : 4996 )

void _WriteHeader(FILE* fp);
void _WriteFrame(FILE* fp, sFrameOfMocapData* data);
void _WriteFooter(FILE* fp);
void __cdecl DataHandler(sFrameOfMocapData* data, void* pUserData);		// receives data from the server
int CreateClient();

int initializeLastFrame = 0;
double lastfTimeStamp = 0.0;
double fRate = 0.0;
double expectedFramePeriod = 0.0;
double deltafTimeStamp = 0.0;
int goodfTimeStamp = 0;
int duplicatefTimeStamp = 0;
int skippedfTimeStamp = 0;
int totalfTimeStamp = 0;
float tolerance = 0.2f; // Tolerance for frame drop criteria

// local timing for verification
HiTimer timer;
double localDelta = 0.0f;

NatNetClient* theClient;
FILE* fp;

char szMyIPAddress[128] = "";
char szServerIPAddress[128] = "";
bool ready = false;

int _tmain(int argc, _TCHAR* argv[])
{
    int iResult;
    
    // parse command line args
    if(argc>1)
    {
        strcpy(szServerIPAddress, argv[1]);	// specified on command line
        printf("Connecting to server at %s...\n", szServerIPAddress);
    }
    else
    {
        strcpy(szServerIPAddress, "");		// not specified - assume server is local machine
        printf("Connecting to server at LocalMachine\n");
    }
    if(argc>2)
    {
        strcpy(szMyIPAddress, argv[2]);	    // specified on command line
        printf("Connecting from %s...\n", szMyIPAddress);
    }
    else
    {
        strcpy(szMyIPAddress, "");          // not specified - assume server is local machine
        printf("Connecting from LocalMachine...\n");
    }

    // create NatNet Client
    iResult = CreateClient();
    if(iResult != ErrorCode_OK)
    {
        printf("Error initializing client.  See log for details.  Exiting");
        exit(1);
    }
    else
    {
        printf("Client initialized and ready.\n");
    }

	// get frame rate from host
	void* pResult;
	int ret = 0;
	int nBytes = 0;
	ret = theClient->SendMessageAndWait("FrameRate", &pResult, &nBytes);
 	if (ret == ErrorCode_OK)
	{
		fRate = *((float*)pResult);
		if (fRate != 0.0f)
			expectedFramePeriod = (1/fRate);
	}
	if (expectedFramePeriod == 0.0)
		printf("Error establishing Frame Rate.");

	// create data file for writing received stream into
	char szFile[MAX_PATH];
	char szFolder[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szFolder);
	if(argc > 3)
		sprintf(szFile, "%s\\%s", szFolder, argv[3]);
	else
		sprintf(szFile, "%s\\timing_output.txt",szFolder);
	fp = fopen(szFile, "w");
	if(!fp)
	{
		printf("error opening output file %s.  Exiting.", szFile);
		exit(1);
	}
	_WriteHeader(fp);
	
	// ready to receive timing data!
	ready = true;

	int c;
	bool bExit = false;
	while(c =_getch())
	{
		switch(c)
		{
			case 'q':
				bExit = true;		
				break;	
			default:
				break;
		}
		if(bExit)
			break;
	}

	// done - clean up.
	theClient->Disconnect();
	_WriteFooter(fp);
	fclose(fp);

	return ErrorCode_OK;
}

// Establish a NatNet Client connection
int CreateClient()
{
    // release previous server
    if(theClient)
    {
        theClient->Disconnect();
        delete theClient;
    }

    // create NatNet client
    theClient = new NatNetClient();

     // print version info
    unsigned char ver[4];
    NatNet_GetVersion(ver);
    printf("NatNet Timing Client (NatNet ver. %d.%d.%d.%d)\n", ver[0], ver[1], ver[2], ver[3]);

    // set callback handlers
    theClient->SetFrameReceivedCallback( DataHandler, theClient );	// this function will receive data from the server

    // initialize a NatNet client and connect it to a NatNet server
    sNatNetClientConnectParams connectParams;
    connectParams.connectionType = ConnectionType_Multicast;
    connectParams.localAddress = szMyIPAddress;
    connectParams.serverAddress = szServerIPAddress;
    int retCode = theClient->Connect( connectParams );
    if (retCode != ErrorCode_OK)
    {
        printf("Unable to connect to server.  Error code: %d. Exiting", retCode);
        return ErrorCode_Internal;
    }
    else
    {
        // we should now be successfully connected and receiving streaming data in a separate thread on the DataHandler callback.
        
        // confirm by printing out server info
        sServerDescription ServerDescription;
        memset(&ServerDescription, 0, sizeof(ServerDescription));
        theClient->GetServerDescription(&ServerDescription);
        if(!ServerDescription.HostPresent)
        {
            printf("Unable to connect to server. Host not present. Exiting.");
            return 1;
        }

        printf("[SampleClient] Server application info:\n");
        printf("Application: %s (ver. %d.%d.%d.%d)\n", ServerDescription.szHostApp, ServerDescription.HostAppVersion[0],
            ServerDescription.HostAppVersion[1],ServerDescription.HostAppVersion[2],ServerDescription.HostAppVersion[3]);
        printf("NatNet Version: %d.%d.%d.%d\n", ServerDescription.NatNetVersion[0], ServerDescription.NatNetVersion[1],
            ServerDescription.NatNetVersion[2], ServerDescription.NatNetVersion[3]);
        printf("Client IP:%s\n", szMyIPAddress);
        printf("Server IP:%s\n", szServerIPAddress);
        printf("Server Name:%s\n\n", ServerDescription.szHostComputerName);
    }

    return ErrorCode_OK;
}

// DataHandler receives data from the server in a separate NatNet data thread.
// This thread services the socket, so perform as little processing here as possible to 
// avoid any frame drop/buffering on the socket.
void __cdecl DataHandler(sFrameOfMocapData* data, void* pUserData)
{
	if (!ready)
		return;

	NatNetClient* pClient = (NatNetClient*) pUserData;

	if (initializeLastFrame == 0)
	{
		lastfTimeStamp = data->fTimestamp;
		initializeLastFrame = 1;
        timer.Start();
		return;
	}
	else
	{
        // local timing info
        timer.Stop();
        localDelta = timer.Duration();
        timer.Start();

        // Motive timing info
		deltafTimeStamp = data->fTimestamp - lastfTimeStamp;
		printf("Frame Rate:%3.2lf  ", fRate);
		printf("Time Stamp:%3.6lf  ", data->fTimestamp);
		printf("Expected Period:%3.6lf  ", expectedFramePeriod);
		printf("Period:%3.6lf ", deltafTimeStamp);
		lastfTimeStamp = data->fTimestamp;
		totalfTimeStamp += 1;
		
        // check for frame drop within expected range of +- 20%.
		if (deltafTimeStamp >= (expectedFramePeriod)*(1.0f-tolerance) && deltafTimeStamp <= (expectedFramePeriod)*(1.0f+tolerance))
		{
			goodfTimeStamp += 1;
		}
		else if (deltafTimeStamp < (expectedFramePeriod)*(1.0f-tolerance))
		{
			duplicatefTimeStamp += 1;
		}
		else if (deltafTimeStamp > (expectedFramePeriod)*(1.0f+tolerance))
		{
			skippedfTimeStamp += 1;
		}

		printf("Total:%d  ", totalfTimeStamp);
		printf("Good:%d  ", goodfTimeStamp);
		printf("Duplicate:%d  ", duplicatefTimeStamp);
		printf("Skipped:%d  ", skippedfTimeStamp);
		
		if (totalfTimeStamp != goodfTimeStamp + duplicatefTimeStamp + skippedfTimeStamp)
			printf("Counter Off \r");
		else
			printf("Counter On  \r");

		if(fp)
			_WriteFrame(fp,data);
	}

}

void _WriteHeader(FILE* fp)
{
    fprintf(fp ,"FrameID\tTimestamp(s)\tDelta(ms)\tLocalDelta(ms)\tIrregular?\n");
}

void _WriteFrame(FILE* fp, sFrameOfMocapData* data)
{
    fprintf(fp ,"%d\t%3.6lf\t%3.6lf\t%3.6lf\t",
                data->iFrame,
                data->fTimestamp,
                deltafTimeStamp*1000.0f,
                localDelta*1000.0f );

    // indicate frame irregularity
	if (deltafTimeStamp <= (expectedFramePeriod)*(1.0f-tolerance) || deltafTimeStamp >= (expectedFramePeriod)*(1.0f+tolerance))
	{
		fprintf(fp, "1");
	}
	fprintf(fp, "\n");
}

void _WriteFooter(FILE* fp)
{
    fprintf(fp, "\n");
    fprintf(fp, "Skipped Timestamps\t%d\n", skippedfTimeStamp);
    fprintf(fp, "Duplicate Timestamps\t%d\n", duplicatefTimeStamp);
}
