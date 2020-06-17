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

#pragma once

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <string>

#include <iostream>


// System Information
class cSystemInfo
{
public:
    cSystemInfo() {};
    virtual ~cSystemInfo() {};

    void setIPAddress( char *input );
    const char* getIPAddress() const;
    void setPort( char * input );
    const char* getPort() const;
    void setType( char *input );
    const char* getType() const;
    bool isTypeValid() const;
    int getParsedPort() const;
    void setTestMode( bool input );
    bool InTestMode() const;

private:
    // Variables
    char * ipAddress;
    char * port;
    char * type;
    bool testMode;

};
