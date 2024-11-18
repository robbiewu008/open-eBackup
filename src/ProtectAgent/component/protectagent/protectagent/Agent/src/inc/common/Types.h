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
#ifndef _AGENT_TYPES_H_
#define _AGENT_TYPES_H_

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


// 布尔值mp_bool取值
static const int MP_TRUE = 1;
static const int MP_FALSE = 0;
// 函数返回值，如果需要其他特定返回值，各函数方法自己定义
static const int MP_SUCCESS = 0;
static const int MP_FAILED = -1;
static const int MP_ERROR = -2;
static const int MP_NOEXISTS = -3;
static const int MP_TIMEOUT = -4;
static const int MP_TASK_FAILED_NEED_RETRY = -5;
static const int MP_TASK_COMPLETE = -10;
static const int MP_TASK_RUNNING = -11;
// 非阻塞模式，如果返回
static const int MP_EAGAIN = -6;

static const int MP_REDO = -12;
static const int MP_INC_TO_FULL = -13;
static const int MP_ARCHIVE_TOO_MUCH_CONNECTION = -14;
static const int MP_TASK_FAILED_NO_REPORT = -15;

static const int MP_ABORTED = -16;

static const int MP_INC_TO_DIFF = -17;

// invalid handle
static const int MP_INVALID_HANDLE = -1;

typedef void mp_void;
typedef int mp_bool;
typedef float mp_float;
typedef double mp_double;
typedef char mp_char;
typedef unsigned char mp_uchar;
typedef int mp_int32;
typedef unsigned int mp_uint32;
typedef short mp_int16;
typedef unsigned short mp_uint16;
typedef long mp_long;
typedef unsigned long mp_ulong;
typedef std::string mp_string;
typedef std::wstring mp_wstring;
typedef std::size_t mp_size;
typedef time_t mp_time;
typedef tm mp_tm;

#ifdef WIN32
typedef __int64 mp_int64;
typedef unsigned __int64 mp_uint64;
typedef WCHAR mp_wchar;
typedef unsigned int mp_socket;
typedef HMODULE mp_handle_t;
typedef HANDLE mp_semaf;
#else
typedef long long mp_int64;
typedef unsigned long long mp_uint64;
typedef wchar_t mp_wchar;
typedef int mp_socket;
typedef void* mp_handle_t;
typedef sem_t mp_semaf;
#endif


// 告警的定位信息，格式为：Name1=Value1,...,NameN=ValueN
static const mp_string HW_STORAGE_REPORTING_ALARM_LOCATION_INFO = ".1.3.6.1.4.1.2011.2.251.20.1.3.2";
// 告警修复建议
static const mp_string HW_STORAGE_REPORTING_ALARM_RESTORE_ADVICE = ".1.3.6.1.4.1.2011.2.251.20.1.3.3";
// 告警信息标题
static const mp_string HW_STORAGE_REPORTING_ALARM_FAULT_TITLE = ".1.3.6.1.4.1.2011.2.251.20.1.3.4";
// 告警级别，其取值范围为： 1-紧急告警 2-重要告警 3-次要告警 4-警告告警 5-事件告警
static const mp_string HW_STORAGE_REPORTING_ALARM_FAULT_LEVEL = ".1.3.6.1.4.1.2011.2.251.20.1.3.6";
// 原始告警ID
static const mp_string HW_STORAGE_REPORTING_ALARM_ALARM_ID = ".1.3.6.1.4.1.2011.2.251.20.1.3.7";
// 告警产生时间
static const mp_string HW_STORAGE_REPORTING_ALARM_FAULT_TIME = ".1.3.6.1.4.1.2011.2.251.20.1.3.8";
// 告警流水号
static const mp_string HW_STORAGE_REPORTING_ALARM_SERIAL_NO = ".1.3.6.1.4.1.2011.2.251.20.1.3.9";
// 告警原因描述
static const mp_string HW_STORAGE_REPORTING_ALARM_ADDITION_INFO = ".1.3.6.1.4.1.2011.2.251.20.1.3.10";
// 告警类别，其取值范围为： 1－故障告警 2－恢复告警 3－事件告警
static const mp_string HW_STORAGE_REPORTING_ALARM_FAULT_CATEGORY = ".1.3.6.1.4.1.2011.2.251.20.1.3.11";
// 告警类型，其取值范围为： 告警类型：2-设备告警 3-事件告警
static const mp_string HW_STORAGE_REPORTING_ALARM_FAULT_TYPE;

static const mp_string OID_ISM_ALARM_REPORTING = "1.3.6.1.4.1.2011.2.91.10.2.1.0.1";
static const mp_string OID_ISM_ALARM_REPORTING_NODECODE = "1.3.6.1.4.1.2011.2.91.10.3.1.1.1";
static const mp_string OID_ISM_ALARM_REPORTING_LOCATIONINFO = "1.3.6.1.4.1.2011.2.91.10.3.1.1.2";
static const mp_string OID_ISM_ALARM_REPORTING_RESTOREADVICE = "1.3.6.1.4.1.2011.2.91.10.3.1.1.3";
static const mp_string OID_ISM_ALARM_REPORTING_FAULTTITLE = "1.3.6.1.4.1.2011.2.91.10.3.1.1.4";
static const mp_string OID_ISM_ALARM_REPORTING_FAULTTYPE = "1.3.6.1.4.1.2011.2.91.10.3.1.1.5";
static const mp_string OID_ISM_ALARM_REPORTING_FAULTLEVEL = "1.3.6.1.4.1.2011.2.91.10.3.1.1.6";
static const mp_string OID_ISM_ALARM_REPORTING_ALARMID = "1.3.6.1.4.1.2011.2.91.10.3.1.1.7";
static const mp_string OID_ISM_ALARM_REPORTING_FAULTTIME = "1.3.6.1.4.1.2011.2.91.10.3.1.1.8";
static const mp_string OID_ISM_ALARM_REPORTING_SERIALNO = "1.3.6.1.4.1.2011.2.91.10.3.1.1.9";
static const mp_string OID_ISM_ALARM_REPORTING_ADDITIONINFO = "1.3.6.1.4.1.2011.2.91.10.3.1.1.10";
static const mp_string OID_ISM_ALARM_REPORTING_FAULTCATEGORY = "1.3.6.1.4.1.2011.2.91.10.3.1.1.11";
static const mp_string OID_ISM_ALARM_REPORTING_LOCATIONID = "1.3.6.1.4.1.2011.2.91.10.3.1.1.12";

#endif  // _AGENT_TYPES_H_
