/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ExternalPluginAlarmMng.h
 * @brief  The implemention about ExternalPluginAlarmMng.h
 * @version 1.0.0.0
 * @date 2021-12-17
 * @author jwx966562
 */
#ifndef _EXTERNAL_PLUGIN_ALARM_MNG_H
#define _EXTERNAL_PLUGIN_ALARM_MNG_H

#include <map>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include "common/Defines.h"
#include "common/CMpThread.h"
#include "alarm/AlarmHandle.h"

class ExternalPluginAlarmMng {
public:
    enum ExPluginAlarmType {
        START_ERROR_ALARM = 0  // external start error alarm
    };

    ExternalPluginAlarmMng() {}
    ~ExternalPluginAlarmMng();
    void Init();
    mp_int32 BroadcastAlarm(ExPluginAlarmType alarmType, const mp_string &pluginName);
    mp_int32 ClearAlarm(ExPluginAlarmType alarmType, const mp_string &pluginName);
private:
    mp_int32 BroadcastStartErrorAlarm(const mp_string &pluginName);
    mp_int32 ClearStartErrorAlarm(const mp_string &pluginName);
    mp_int32 GenStartErrAlarmInfo(alarm_param_t &alarmParam, const mp_string &pluginName);
    mp_int32 SendAlarm(const alarm_param_t &alarmParam, bool isClearAlarm = false);
    void ResendAlarm();
    void StartResendThread();
    void StopResendThread();
    void EraseFromResendVec(const alarm_param_t &alarmParam, std::vector<alarm_param_t> &resendVec);
private:
    using AlarmHandlerFun = std::function<mp_int32(const mp_string&)>;
    std::map<ExPluginAlarmType, std::vector<mp_string>> m_existAlarms; // value: pluginName list
    std::map<ExPluginAlarmType, AlarmHandlerFun> m_broadcastAlarmHandler;
    std::map<ExPluginAlarmType, AlarmHandlerFun> m_clearAlarmHandler;
    std::vector<alarm_param_t> m_resendAlarmInfo; // 发送失败的告警
    std::vector<alarm_param_t> m_resendClearAlarmInfo; // 发送失败的恢复告警
    std::unique_ptr<std::thread> m_resendThread;
    bool m_needRsendAlarm;
    std::mutex m_existAlarmMutex;
    std::mutex m_resendMutex;
};

#endif