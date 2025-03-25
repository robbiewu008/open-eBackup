/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * @file AlarmMgr.h
 * @brief alarm management
 * @date 2022-2-21
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