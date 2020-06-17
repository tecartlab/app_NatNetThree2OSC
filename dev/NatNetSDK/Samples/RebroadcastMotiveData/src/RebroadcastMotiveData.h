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

RebroadcastMotiveData

This program will take arguments from a user that will determine a Message and how the message.
is transmitted. It will then connect to a NatNet server, encode the data and output it based on
the parameters passed.

Usage:

RebroadcastMotiveData.exe [a- address][p- port][ string ]

string values consist of:
unity | lightcraft

*/

#pragma once

#include "stdafx.h"
#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <winsock2.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "NatNetTypes.h"
#include "NatNetCAPI.h"
#include "NatNetClient.h"

#include "NatNetRepeater.h"   //== for transport of data over UDP to Unity3D



//== Slip Stream globals ==--

extern cSlipStream *gSlipStream;
std::map<int, std::string> gBoneNames;

void NATNET_CALLCONV DataHandler( sFrameOfMocapData* data, void* pUserData );   // receives data from the server
void NATNET_CALLCONV MessageHandler( Verbosity msgType, const char* msg );     // receives NatNet error messages
void resetClient();
int CreateClient( ConnectionType connectionType );

unsigned int MyServersDataPort = 3130;
unsigned int MyServersCommandPort = 3131;

NatNetClient* theClient;
FILE* fp;

char szMyIPAddress[128] = "";
char szServerIPAddress[128] = "";
char szUnityIPAddress[128] = "";

int analogSamplesPerMocapFrame = 0;
ConnectionType iConnectionType = ConnectionType_Multicast;
//extern ConnectionType iConnectionType = ConnectionType_Unicast;

//== Helpers for text indicator of data flowing... ==--
