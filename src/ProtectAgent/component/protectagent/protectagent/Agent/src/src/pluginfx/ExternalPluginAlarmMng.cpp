#include "pluginfx/ExternalPluginAlarmMng.h"

#include "common/Log.h"
#include "common/Ip.h"
#include "alarm/AlarmCode.h"
#include "alarm/Trap.h"
#include "host/host.h"
#include "common/Path.h"

namespace {
    const int RESEND_INTERVAL = 30 * 1000; // 30s
}

ExternalPluginAlarmMng::~ExternalPluginAlarmMng()
{
    StopResendThread();
}

void ExternalPluginAlarmMng::Init()
{
    m_broadcastAlarmHandler.insert(std::make_pair(ExPluginAlarmType::START_ERROR_ALARM,
        std::bind(&ExternalPluginAlarmMng::BroadcastStartErrorAlarm, this, std::placeholders::_1)));
    m_clearAlarmHandler.insert(std::make_pair(ExPluginAlarmType::START_ERROR_ALARM,
        std::bind(&ExternalPluginAlarmMng::ClearStartErrorAlarm, this, std::placeholders::_1)));
    m_needRsendAlarm = false;
}

mp_int32 ExternalPluginAlarmMng::BroadcastAlarm(ExPluginAlarmType alarmType, const mp_string &pluginName)
{
    LOGGUARD("");
    std::lock_guard<std::mutex> lck(m_existAlarmMutex);
    auto existIter = m_existAlarms.find(alarmType);
    if (existIter != m_existAlarms.end()) {
        if (std::find(existIter->second.begin(), existIter->second.end(), pluginName) != existIter->second.end()) {
            WARNLOG("Alarm type %d for plugin %s already broadcast.", alarmType, pluginName.c_str());
            return MP_SUCCESS;
        }
    }

    if (m_broadcastAlarmHandler.find(alarmType) != m_broadcastAlarmHandler.end()) {
        m_existAlarms[alarmType].push_back(pluginName);
        return m_broadcastAlarmHandler[alarmType](pluginName);
    }
    return MP_SUCCESS;
}

mp_int32 ExternalPluginAlarmMng::ClearAlarm(ExPluginAlarmType alarmType, const mp_string &pluginName)
{
    LOGGUARD("");
    std::lock_guard<std::mutex> lck(m_existAlarmMutex);
    if (ClearStartErrorAlarm(pluginName) != MP_SUCCESS) {
        ERRLOG("Clear start error alarm for %s failed", pluginName.c_str());
        return MP_FAILED;
    }
    // 查询删除保存的告警如果有
    auto existIter = m_existAlarms.find(alarmType);
    if (existIter != m_existAlarms.end()) {
        auto tmpPluginIter = std::find(existIter->second.begin(), existIter->second.end(), pluginName);
        if (tmpPluginIter != existIter->second.end()) {
            INFOLOG("Find Alarm type %d for plugin %s.", alarmType, pluginName.c_str());
            existIter->second.erase(tmpPluginIter);
        }
    }
    return  MP_SUCCESS;
}

mp_int32 ExternalPluginAlarmMng::BroadcastStartErrorAlarm(const mp_string &pluginName)
{
    alarm_param_t alarmParam;
    if (GenStartErrAlarmInfo(alarmParam, pluginName) != MP_SUCCESS) {
        ERRLOG("Broadcast start error alarm for plugin %s failed.", pluginName.c_str());
        return MP_FAILED;
    }
    return SendAlarm(alarmParam);
}

mp_int32 ExternalPluginAlarmMng::ClearStartErrorAlarm(const mp_string &pluginName)
{
    alarm_param_t alarmParam;
    if (GenStartErrAlarmInfo(alarmParam, pluginName) != MP_SUCCESS) {
        ERRLOG("Clear start error alarm for plugin %s failed.", pluginName.c_str());
        return MP_FAILED;
    }
    return SendAlarm(alarmParam, true);
}

mp_int32 ExternalPluginAlarmMng::GenStartErrAlarmInfo(alarm_param_t &alarmParam, const mp_string &pluginName)
{
    alarmParam.iAlarmID = ALARM_START_PLUGIN;
    CHost host;
    mp_string strSN;
    mp_int32 ret = host.GetHostSN(strSN);
    if (ret != MP_SUCCESS) {
        WARNLOG("GetHostSN failed, iRet %d.", ret);
        strSN = "";
    }
    alarmParam.resourceId = strSN;
    mp_string listenIp;
    mp_string listenPort;
    ret = CIP::GetListenIPAndPort(listenIp, listenPort);
    if (ret != MP_SUCCESS) {
        ERRLOG("Fail to get listen IP and Port.");
        return MP_FAILED;
    }
    mp_string pluginPath = CPath::GetInstance().GetPluginsPath() + PATH_SEPARATOR + pluginName;
    alarmParam.strAlarmParam = listenIp + "," + pluginName + "," + pluginPath;
    return MP_SUCCESS;
}

mp_int32 ExternalPluginAlarmMng::SendAlarm(const alarm_param_t &alarmParam, bool isClearAlarm)
{
    INFOLOG("Start to send alarm %s.", alarmParam.iAlarmID.c_str());
    AlarmHandle handle;
    if (isClearAlarm) {
        // 如果是恢复告警
        if (handle.ClearAlarm(alarmParam) != MP_SUCCESS) {
            ERRLOG("Send clear alarm %s failed.", alarmParam.iAlarmID.c_str());
            std::lock_guard<std::mutex> resendLock(m_resendMutex);
            m_resendClearAlarmInfo.push_back(alarmParam);
            StartResendThread();
            CTrapSenderManager::CreateSender(A8000).ResumeAlarm(alarmParam);
            return MP_FAILED;
        }
    } else {
        if (handle.Alarm(alarmParam) != MP_SUCCESS) {
            ERRLOG("Send alarm %s failed.", alarmParam.iAlarmID.c_str());
            std::lock_guard<std::mutex> resendLock(m_resendMutex);
            m_resendAlarmInfo.push_back(alarmParam);
            StartResendThread();
            CTrapSenderManager::CreateSender(A8000).SendAlarm(alarmParam);
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

void ExternalPluginAlarmMng::ResendAlarm()
{
    LOGGUARD("");
    while (m_needRsendAlarm) {
        std::vector<alarm_param_t> tempAlarmVec;
        std::vector<alarm_param_t> tempClearAlarmVec;
        AlarmHandle handle;
        {
            std::lock_guard<std::mutex> resendLock(m_resendMutex);
            tempAlarmVec = m_resendAlarmInfo;
            tempClearAlarmVec = m_resendClearAlarmInfo;
        }
        // resend alarm
        auto iter = tempAlarmVec.begin();
        for (; iter != tempAlarmVec.end(); ++iter) {
            if (handle.Alarm(*iter) == MP_SUCCESS) {
                DBGLOG("Rsend alarm %s success.", (*iter).iAlarmID.c_str());
                EraseFromResendVec(*iter, m_resendAlarmInfo);
            }
        }
        // resend clear clarm
        iter = tempClearAlarmVec.begin();
        for (; iter != tempClearAlarmVec.end(); ++iter) {
            if (handle.ClearAlarm(*iter) == MP_SUCCESS) {
                DBGLOG("Rsend clear alarm %s success.", (*iter).iAlarmID.c_str());
                EraseFromResendVec(*iter, m_resendClearAlarmInfo);
            }
        }

        {
            std::lock_guard<std::mutex> resendLock(m_resendMutex);
            if (m_resendAlarmInfo.empty() && m_resendClearAlarmInfo.empty()) {
                DBGLOG("All alarms have been resent success.");
                m_needRsendAlarm = false;
                break;
            }
        }

        CMpTime::DoSleep(RESEND_INTERVAL);
    }
}

void ExternalPluginAlarmMng::EraseFromResendVec(const alarm_param_t &alarmParam, std::vector<alarm_param_t> &resendVec)
{
    std::lock_guard<std::mutex> resendLock(m_resendMutex);
    auto alarmIter = std::find_if(resendVec.begin(), resendVec.end(),
        [&alarmParam](const alarm_param_t& param) {
            if ((alarmParam.iAlarmID == param.iAlarmID) && (alarmParam.strAlarmParam == param.strAlarmParam)) {
                return true;
            }
            return false;
        });
    if (alarmIter != resendVec.end()) {
        resendVec.erase(alarmIter);
    }
}

void ExternalPluginAlarmMng::StartResendThread()
{
    LOGGUARD("");
    if (!m_needRsendAlarm) {
        m_needRsendAlarm = true;
        if (m_resendThread) {
            m_resendThread->join();
            m_resendThread.reset();
        }
        m_resendThread.reset(new std::thread(std::bind(&ExternalPluginAlarmMng::ResendAlarm, this)));
        INFOLOG("Start resend thread.");
    }
}

void ExternalPluginAlarmMng::StopResendThread()
{
    LOGGUARD("");
    m_needRsendAlarm = false;
    if (m_resendThread) {
        m_resendThread->join();
        m_resendThread.reset();
    }
    INFOLOG("Stop resend thread.");
}
