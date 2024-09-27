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
#ifndef MODULE_TYPES_H
#define MODULE_TYPES_H

#ifdef WIN32
#include <time.h>
#include <winsock2.h>
#include <windows.h>

#else
#include <limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <string>

namespace Module {

constexpr int FAILED { -1 };
constexpr int SUCCESS { 0 };
constexpr int RETRY { 9 };
constexpr int INTERNAL_ERR { 200 };

#ifdef WIN32
typedef HMODULE handle_t;
#else
typedef void* handle_t;
#endif

}

#endif  // MODULE_TYPES_H
