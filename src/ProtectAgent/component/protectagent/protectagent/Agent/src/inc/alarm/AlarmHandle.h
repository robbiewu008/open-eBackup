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
#ifndef _ALARM_HANDLE_H_
#define _ALARM_HANDLE_H_

#include "alarm/alarmdb.h"
#include "message/curlclient/CurlHttpClient.h"

class AlarmHandle {
public:
    AlarmHandle();
    ~AlarmHandle();

    mp_int32 Alarm(const alarm_param_t& alarmParam);
    mp_int32 Event(const alarm_param_t& eventParam);
    mp_int32 ClearAlarm(const alarm_param_t& alarmParam);
    mp_int32 ForwardAlarmReq(const alarm_param_t& alarmParam, bool sendAlarm = true);

private:
    mp_int32 NewAlarm(const alarm_param_t& alarmParam, alarm_Info_t& alarmInfo);
    mp_void UpdateAlmInfo(const alarm_param_t& alarmParam, alarm_Info_t& alarmInfo);
    mp_void TransAlarmParam2AlarmInfo(const alarm_param_t& alarmParam, alarm_Info_t& alarmInfo);
    EXTER_ATTACK mp_int32 SendAlarm_Http(const alarm_Info_t& alarmInfo);
    mp_int32 BuildHttpRequest(HttpRequest& req, const mp_string& ip, const mp_string& port);
    mp_void BuildHttpBody(HttpRequest& req, const alarm_Info_t& alarmInfo);
    mp_int32 SendRequest(const HttpRequest& req);

private:
    bool m_bAlarm;
};

#endif