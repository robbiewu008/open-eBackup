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
#include "alarm/AlarmMgr.h"
#include "common/Log.h"
#include "common/Ip.h"
#include "common/Types.h"
#include "common/Utils.h"
#include "alarm/AlarmHandle.h"

AlarmMgr AlarmMgr::m_Instance;

mp_void AlarmMgr::SendAlarm(const mp_string &alarmId, const mp_string &param)
{
    alarm_param_t alarmObj;
    if (UpdateAlarmObj(alarmObj, alarmId, ALARM_CLASS::ALARM, true, param) != MP_TRUE) {
        return;
    }
    if (m_alarmHandle.Alarm(alarmObj) != MP_SUCCESS) {
        ERRLOG("Send alarm %s failed, sleep 1s and retry.", alarmId.c_str());
        DoSleep(mp_retryInterval);
        if (m_alarmHandle.Alarm(alarmObj) != MP_SUCCESS) {
            ERRLOG("Send alarm %s failed.", alarmId.c_str());
            return;
        }
    }
    INFOLOG("Send alarm %s success, param(%s).", alarmId.c_str(), alarmObj.strAlarmParam.c_str());
}

mp_void AlarmMgr::ResumeAlarm(const mp_string &alarmId, const mp_string &param)
{
    alarm_param_t alarmObj;
    if (UpdateAlarmObj(alarmObj, alarmId, ALARM_CLASS::ALARM, true, param) != MP_TRUE) {
        return;
    }
    if (m_alarmHandle.ClearAlarm(alarmObj) != MP_SUCCESS) {
        ERRLOG("Clear alarm %s failed, sleep 1s and retry.", alarmId.c_str());
        DoSleep(mp_retryInterval);
        if (m_alarmHandle.ClearAlarm(alarmObj) != MP_SUCCESS) {
            ERRLOG("Clear alarm %s failed.", alarmId.c_str());
            return;
        }
    }
    INFOLOG("Resume alarm %s success, param(%s).", alarmId.c_str(), alarmObj.strAlarmParam.c_str());
}

mp_void AlarmMgr::SendEvent(const mp_string &eventId, bool isSuccess, const mp_string &param)
{
    alarm_param_t eventObj;
    if (UpdateAlarmObj(eventObj, eventId, ALARM_CLASS::EVENT, isSuccess, param) != MP_TRUE) {
        return;
    }
    if (m_alarmHandle.Event(eventObj) != MP_SUCCESS) {
        ERRLOG("Send event %s failed, sleep 1s and retry.", eventId.c_str());
        DoSleep(mp_retryInterval);
        if (m_alarmHandle.Alarm(eventObj) != MP_SUCCESS) {
            ERRLOG("Send event %s failed.", eventId.c_str());
            return;
        }
    }
    INFOLOG("Send event %s success, param(%s).", eventId.c_str(), eventObj.strAlarmParam.c_str());
}

mp_bool AlarmMgr::UpdateAlarmObj(alarm_param_t &alarmObj, const mp_string &alarmId, mp_int32 alarmClass,
    bool isSuccess, const mp_string &param)
{
    alarmObj.iAlarmID = alarmId;
    alarmObj.iAlarmClass = alarmClass;
    alarmObj.isSuccess = isSuccess;
    mp_string listenIp;
    mp_string listenPort;
    mp_int32 ret = CIP::GetListenIPAndPort(listenIp, listenPort);
    if (ret != MP_SUCCESS) {
        ERRLOG("Fail to get listen IP and Port.");
        return MP_FALSE;
    }
    if (param.empty()) {
        alarmObj.strAlarmParam = std::move(listenIp);
    } else {
        alarmObj.strAlarmParam = listenIp + m_paramSeparator + param;
    }
    return MP_TRUE;
}