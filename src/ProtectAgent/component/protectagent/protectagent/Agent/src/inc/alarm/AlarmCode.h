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
#ifndef _ALARMCODE_H_
#define _ALARMCODE_H_
#include "common/Types.h"
const mp_string RUN_EVENT_JOB_LONG_TIME_FULL_LOAD = "0x106403400002";
const mp_string ALARM_THRIFT_SERVER_START_FAILED = "0x106403400001";
const mp_string ALARM_ORACLE_ARCHIVE_AREA = "0x6403400001";
#ifdef WIN32
const mp_string ALARM_START_PLUGIN = "0x6403400007";
#else
const mp_string ALARM_START_PLUGIN = "0x6403400002";
#endif

#endif
