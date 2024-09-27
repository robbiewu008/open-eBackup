/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef HOST_OS_PLATFORM_DEFINES_H
#define HOST_OS_PLATFORM_DEFINES_H

#include <string>

#ifdef WIN32
#include "Win32ServiceTask.h"
#else
#include "PosixServiceTask.h"
#endif

#ifdef WIN32
const std::string OS_PLATFORM_NAME = "Win32";
#else
const std::string OS_PLATFORM_NAME = "Posix";
#endif

#ifdef WIN32
using OsPlatformServiceTask = Win32ServiceTask;
#else
using OsPlatformServiceTask = PosixServiceTask;
#endif

#endif