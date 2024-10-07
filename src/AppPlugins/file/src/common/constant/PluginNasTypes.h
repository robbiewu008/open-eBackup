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
#ifndef PLUGIN_NAS_TYPES_H
#define PLUGIN_NAS_TYPES_H

#include <climits>
#ifndef WIN32
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#endif
#include <semaphore.h>
#include <csignal>
#include <ctime>
#include <cerrno>
#include <cstdio>
#include <string>

// 布尔值mp_bool取值
#ifndef MP_TRUE
static const int MP_TRUE = 1;
static const int MP_FALSE = 0;
#endif
// 函数返回值，如果需要其他特定返回值，各函数方法自己定义
#ifndef MP_SUCCESS
static const int MP_SUCCESS = 0;
static const int MP_FAILED = -1;
#endif
static const int MP_ERROR = -2;
static const int MP_NOEXISTS = -3;
static const int MP_TIMEOUT = -4;
static const int MP_TASK_FAILED_NEED_RETRY = -5;
static const int MP_TASK_COMPLETE = -10;
static const int MP_TASK_RUNNING = -11;
// 非阻塞模式，如果返回
static const int MP_EAGAIN = -6;

// invalid handle
static const int MP_INVALID_HANDLE = -1;

#define NAS_PLUGIN_LOG(logLevel, ...) printf(__VA_ARGS__)

#endif  // _PLUGIN_NAS_TYPES_H_