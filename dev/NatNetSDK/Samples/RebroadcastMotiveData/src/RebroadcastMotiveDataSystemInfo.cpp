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


#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <string>

#include <iostream>
#include "RebroadcastMotiveDataSystemInfo.h"


void cSystemInfo::setIPAddress( char * input )
{
    ipAddress = input;
}

const char* cSystemInfo::getIPAddress() const
{
    return ipAddress;
}

void cSystemInfo::setPort( char * input )
{
    port = input;
}

const char* cSystemInfo::getPort() const
{
    return port;
}

void cSystemInfo::setType( char * input )
{
    type = input;
}

const char* cSystemInfo::getType() const
{
    return type;
}

 bool cSystemInfo::isTypeValid() const
{
    if ( !strcmp( type, "unity" ) )
        return true;
    if ( !strcmp( type, "lightcraft" ) )
        return true;

    return false;
}

int cSystemInfo::getParsedPort() const
{
    int val = atoi( port );
    return val;
}

void cSystemInfo::setTestMode( bool input )
{
    testMode = input;
}

bool cSystemInfo::InTestMode() const 
{
    return testMode;
}