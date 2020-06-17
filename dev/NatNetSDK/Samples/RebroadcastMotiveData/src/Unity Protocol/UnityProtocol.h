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

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

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
#include "RebroadcastMotiveDataSystemInfo.h"

void runUnityProtocol( const cSystemInfo& systemInfo );


