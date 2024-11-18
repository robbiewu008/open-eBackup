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
#ifndef _ALARM_MGR_H
#define _ALARM_MGR_H
#include <set>
#include "common/Types.h"
#include "alarm/alarmdb.h"
#include "alarm/AlarmHandle.h"
class AlarmMgr {
public:
    static AlarmMgr& GetInstance()
    {
        return m_Instance;
    }
    mp_void SendAlarm(const mp_string &alarmId, const mp_string &param = "");
    mp_void ResumeAlarm(const mp_string &alarmId, const mp_string &param = "");
    mp_void SendEvent(const mp_string &eventId, bool isSuccess = true, const mp_string &param = "");

private:
    mp_int32 UpdateAlarmObj(alarm_param_t &alarmInfo, const mp_string &alarmId, mp_int32 alarmClass, bool isSuccess,
        const mp_string &param);
private:
    AlarmHandle m_alarmHandle;
    static AlarmMgr m_Instance;
    mp_int32 mp_retryInterval = 1000;
    mp_string m_paramSeparator = ",";
};
#endif