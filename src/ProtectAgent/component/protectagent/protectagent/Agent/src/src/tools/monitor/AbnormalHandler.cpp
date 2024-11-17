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
#include "tools/monitor/AbnormalHandler.h"

#include <sstream>
#include <fstream>
#include <cmath>
#ifdef WIN32
#include <Windows.h>
#include "tlhelp32.h"
#include "Psapi.h"
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#endif
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/Utils.h"
#include "common/CMpThread.h"
#include "common/Path.h"
#include "common/CMpTime.h"
#include "common/File.h"
#include "common/Defines.h"
#include "common/CSystemExec.h"
#include "common/Ip.h"
#include "securecom/RootCaller.h"
#include "securecom/CryptAlg.h"
#include "securecom/SecureUtils.h"
#include "securecom/UniqueId.h"
#include "alarm/AlarmMgr.h"
#include "common/SecureCmdExecutor.h"

using namespace std;
AbnormalHandler AbnormalHandler::m_instance;

namespace {
// If configured log max size: 2G
//   then if logsize >= 2G * 80%, send AGENT_LOGSIZE_WARNING_ALARM
//   or   if logsize >= 2G, send AGENT_LOGSIZE_STOP_ALARM
const mp_string AGENT_LOGSIZE_WARNING_ALARM = "0x6403400005";
const mp_string AGENT_LOGSIZE_STOP_ALARM = "0x6403400006";
const mp_float LOGSIZE_ALARM_MIN = 0.1;
const mp_float LOGSIZE_ALARM_MAX = 0.9;
const mp_float LOGSIZE_ALARM_DEFAULT = 0.8;
const mp_int32 MEMORY_ALARM_DEFAULT = 90;
const mp_string PARAM_SEPERATOR = ",";
const mp_string PARAM_PERCENT_SIGN = "%";
// if agent memory usage more then 90%, send AGENT_MEMORY_WARNING_ALARM
const mp_string AGENT_MEMORY_WARNING_ALARM = "0x6403400010";
}

/* ------------------------------------------------------------
Function Name: Init
Description  : 初始函数，创建监控处理线程
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::Init()
{
    return CMpThread::Create(&m_hHandleThread, HandleFunc, this);
}

/* ------------------------------------------------------------
Function Name: HandleFunc
Description  : 监控处理线程处理函数
Others       :-------------------------------------------------------- */
#ifdef WIN32
DWORD WINAPI AbnormalHandler::HandleFunc(mp_void* pThis)
#else
EXTER_ATTACK mp_void* AbnormalHandler::HandleFunc(mp_void* pThis)
#endif
{
    LOGGUARD("");
    if (pThis == nullptr) {
        CMPTHREAD_RETURN;
    }
    AbnormalHandler* pHandle = static_cast<AbnormalHandler*>(pThis);

    pHandle->SetThreadStatus(THREAD_STATUS_RUNNING);
    mp_uint32 iCount = 0;
    while (!pHandle->NeedExit()) {
        // 从配置文件中读取配置
        pHandle->GetAgentMonitorCfg();
        pHandle->GetNginxMonitorCfg();
        pHandle->GetDataProcessMonitorCfg();
        monitor_common_config_t commonCfg = pHandle->GetCommonMonitorCfg();
        pHandle->Handle();
        if (iCount == 0) {
            CallCryptTimer();
        }
        // 从配置文件读取的值，不会溢出
        DoSleep(static_cast<mp_uint32>(commonCfg.iMonitorInterval * ABNORMALHANDLER_NUM_1000));
        iCount++;
        iCount %= ABNORMALHANDLER_NUM_10;
    }
    pHandle->SetThreadStatus(THREAD_STATUS_EXITED);
    CMPTHREAD_RETURN;
}

/* ------------------------------------------------------------
Function Name: HandleFunc
Description  : 对所有监控进程逐条处理
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::Handle()
{
    LOGGUARD("");
    mp_int32 iRet;
#ifndef WIN32
    vector<mp_string> vecReadPid;
    mp_string strPidFilePath = CPath::GetInstance().GetLogFilePath(MONITOR_PID);
    iRet = CMpFile::ReadFile(strPidFilePath, vecReadPid);  // 从文件中读取monitor的pid
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Read monitor pid from %s failed, iRet = %d", MONITOR_PID.c_str(), iRet);
    } else {
        pid_t iRealPid = getpid();  // 获取真实的PID
        mp_int32 iReadMonitorPid = 0;
        if (!vecReadPid.empty()) {
            iReadMonitorPid = atoi(vecReadPid.front().c_str());
        }
        if ((mp_int32)iRealPid != iReadMonitorPid) {  // 真实PID与文件中的不一样
            COMMLOG(OS_LOG_WARN, "Read monitor pid(%d) isn't the real pid(%d).", iReadMonitorPid, iRealPid);
            ostringstream oss;
            oss << (mp_int32)iRealPid;
            vector<mp_string> vecRealPid;
            vecRealPid.push_back(oss.str().c_str());
            iRet = CIPCFile::WriteFile(strPidFilePath, vecRealPid);  // 将真实的PID写入文件。
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "Write real monitor pid(%d) failed, iRet: %d. ", (mp_int32)iRealPid, iRet);
            } else {
                COMMLOG(OS_LOG_WARN, "Write real monitor pid(%d) to file sucess.", (mp_int32)iRealPid);
            }
        }
    }
#else
    iRet = MP_SUCCESS;
#endif
    
    if (m_stAgentMointorCfg.bMonitored) {
        iRet = MonitorAgent();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Monitor agent failed, iRet = %d", iRet);
        }
    }

    mp_int32 iRet1 = MonitorAllProcess();
    if (iRet1 != MP_SUCCESS) {
        return iRet1;
    }

    return iRet;
}

mp_int32 AbnormalHandler::MonitorAllProcess()
{
    mp_int32 iRet = MP_SUCCESS;
    
    if (m_stNginxMointorCfg.bMonitored) {
        iRet = MonitorNginx();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Monitor nginx failed, iRet = %d", iRet);
            return iRet;
        }
    }

    if (m_stDataProcessMointorCfg.bMonitored) {
        iRet = MonitorDataProcess();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Monitor dataProcess failed, iRet = %d", iRet);
            return iRet;
        }
    }

    return iRet;
}
/* ------------------------------------------------------------
Function Name: MonitorAgent
Description  : agent进程监控处理函数
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::MonitorAgent()
{
    LOGGUARD("");
    monitor_data_t monitorAgentData;
    mp_int32 iRet = InitMonitorAgentData(monitorAgentData);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // 检测agent监控数据
    CheckAgentMonitorValue(monitorAgentData);

    // 监测物理内存占用率，虚拟内存占用率，句柄个数，线程个数
    mp_bool bFlag = m_stAgentAbnormal.iPmSizeOverTimes > m_stCommonMonitorCfg.iRetryTime ||
                    m_stAgentAbnormal.iVmSizeOverTimes > m_stCommonMonitorCfg.iRetryTime ||
                    m_stAgentAbnormal.iHandlerOverTimes > m_stCommonMonitorCfg.iRetryTime ||
                    m_stAgentAbnormal.iThreadOverTimes > m_stCommonMonitorCfg.iRetryTime;
    if (bFlag) {
        COMMLOG(OS_LOG_INFO,
            "Agent process is abnormal and will be restarted, iPmSizeOverTimes is %d,"
            "iVmSizeOverTimes is %d, iHandlerOverTimes is %d, iThreadOverTimes is %d",
            m_stAgentAbnormal.iPmSizeOverTimes,
            m_stAgentAbnormal.iVmSizeOverTimes,
            m_stAgentAbnormal.iHandlerOverTimes,
            m_stAgentAbnormal.iThreadOverTimes);
        iRet = RestartAgent();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Restart agent failed, iRet = %d", iRet);
        } else {
            COMMLOG(OS_LOG_INFO, "Agent has been restarted successfully.");
        }
        return iRet;
    }

    // 检测cpu利用率是否需要发送告警
    CheckAgentCpuUsage(monitorAgentData.fCpuUsage);

    // 检测agent安装目录剩余空间是否需要发送告警
    CheckAgentFreeSpace();

    // 检测临时文件总大小
    if (m_stAgentAbnormal.iTmpFileSizeOverTimes > m_stCommonMonitorCfg.iRetryTime) {
        mp_int32 iRet1 = DeleteTmpFile();
        if (iRet1 != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Temp file size exceed, but delete failed, iRet = %d", iRet1);
            return iRet1;
        }
    }

    return iRet;
}

mp_int32 AbnormalHandler::InitMonitorAgentData(monitor_data_t &monitorAgentData)
{
    LOGGUARD("");
    mp_int32 iRet = GetAgentMonitorData(monitorAgentData);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get agent monitor data failed, iRet = %d", iRet);
        return iRet;
    }
 
    DBGLOG("Agent monitor data: bExist=%d, iHandlerNum=%d, iThreadNum=%d, ulPmSize=%lluK, KulVmSize=%lluK",
        monitorAgentData.bExist, monitorAgentData.iHandlerNum, monitorAgentData.iThreadNum,
        monitorAgentData.ulPmSize, monitorAgentData.ulVmSize);
    DBGLOG("Agent monitor data: fCpuUsage=%f, ulTmpFileTotalSize=%lluK, ulLogFileTotalSize=%lluK.",
        monitorAgentData.fCpuUsage,
        monitorAgentData.ulTmpFileTotalSize, monitorAgentData.ulLogFileTotalSize);
    DBGLOG("The log monitor configuration, logfile_size: %ld, logfile_size_alarm_threshold: %f",
        m_stAgentMointorCfg.iLogFileTotalSizeCfg, m_stAgentMointorCfg.fLogAlarmPercentage);
 
    if (m_stAgentMointorCfg.fLogAlarmPercentage < LOGSIZE_ALARM_MIN ||
        m_stAgentMointorCfg.fLogAlarmPercentage > LOGSIZE_ALARM_MAX) {
        WARNLOG("The logfile_size_alarm_threshold:%ld is invalid, will auto set to 0.8 by default. \
            Valid range: [0.1, 0.9], please check the configuration in agent_cfg.xml.",
            m_stAgentMointorCfg.fLogAlarmPercentage);
        m_stAgentMointorCfg.fLogAlarmPercentage = LOGSIZE_ALARM_DEFAULT;
    }
 
    mp_uint64 stopThreshold =
        static_cast<mp_uint64>(m_stAgentMointorCfg.iLogFileTotalSizeCfg) * ABNORMALHANDLER_NUM_1024;
    mp_uint64 alarmThreshold = stopThreshold * m_stAgentMointorCfg.fLogAlarmPercentage;
    INFOLOG("Current log size: %ld KB, log size max threshold: %d * 1024 KB, log size alarm threshold: %f (%ld KB)",
        monitorAgentData.ulLogFileTotalSize, m_stAgentMointorCfg.iLogFileTotalSizeCfg,
        m_stAgentMointorCfg.fLogAlarmPercentage, alarmThreshold);
 
    return MonitorAgentData(monitorAgentData, alarmThreshold, stopThreshold, m_stAgentMointorCfg.fLogAlarmPercentage);
}

mp_int32 AbnormalHandler::MonitorAgentData(monitor_data_t &monitorAgentData,
    const mp_uint64& alarmThreshold, const mp_uint64& stopThreshold, const mp_float& alarmPer)
{
    LOGGUARD("");
    mp_int32 iRet = MP_FAILED;
    if (monitorAgentData.bExist == MP_TRUE) {
        if (monitorAgentData.ulLogFileTotalSize >= alarmThreshold &&
            monitorAgentData.ulLogFileTotalSize < stopThreshold) {
                mp_int32 iAlarmThd = static_cast<mp_int32>(ceil(alarmPer * ABNORMALHANDLER_NUM_100));
                mp_string params =
                    std::to_string(monitorAgentData.ulLogFileTotalSize / ABNORMALHANDLER_NUM_1024) + PARAM_SEPERATOR
                    + std::to_string(stopThreshold / ABNORMALHANDLER_NUM_1024) + PARAM_SEPERATOR
                    + std::to_string(iAlarmThd) + PARAM_PERCENT_SIGN;
                WARNLOG("Send warning-alarm %s, params is: %s", AGENT_LOGSIZE_WARNING_ALARM.c_str(), params.c_str());
                AlarmMgr::GetInstance().SendAlarm(AGENT_LOGSIZE_WARNING_ALARM, params);
        } else if (monitorAgentData.ulLogFileTotalSize >= stopThreshold) {
            INFOLOG("The space occupied by the log directory is greater than %dMB, rdagent will be stop.",
                m_stAgentMointorCfg.iLogFileTotalSizeCfg);

            mp_string params = std::to_string(monitorAgentData.ulLogFileTotalSize / ABNORMALHANDLER_NUM_1024)
                + PARAM_SEPERATOR + std::to_string(stopThreshold / ABNORMALHANDLER_NUM_1024);
            WARNLOG("Send stop-alarm %s, params: %s", AGENT_LOGSIZE_STOP_ALARM.c_str(), params.c_str());
            AlarmMgr::GetInstance().SendAlarm(AGENT_LOGSIZE_STOP_ALARM, params);

            iRet = StopAgent();
            if (iRet != MP_SUCCESS) {
                ERRLOG("Stop agent failed, iRet = %d", iRet);
            }
            StopPlugins();
            return iRet;
        } else {
            WARNLOG("Clear warning-alarm %s", AGENT_LOGSIZE_WARNING_ALARM.c_str());
            AlarmMgr::GetInstance().ResumeAlarm(AGENT_LOGSIZE_WARNING_ALARM);
        }
    } else {
        if (monitorAgentData.ulLogFileTotalSize < stopThreshold) {
            INFOLOG("Agent process is not exist and will be started");
            iRet = StartAgent();
            if (iRet != MP_SUCCESS) {
                ERRLOG("Start agent failed, iRet = %d", iRet);
            } else {
                WARNLOG("Clear stop-alarm %s", AGENT_LOGSIZE_STOP_ALARM.c_str());
                AlarmMgr::GetInstance().ResumeAlarm(AGENT_LOGSIZE_STOP_ALARM);
            }
            return iRet;
        }
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: MonitorNginx
Description  : nginx进程监控处理函数
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::MonitorNginx()
{
    LOGGUARD("");
    monitor_data_t monitorNginxData;
    mp_int32 iRet = InitMonitorNginxData(monitorNginxData);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // 检测nginx监控数据
    CheckNginxMonitorValue(monitorNginxData);

    // 监测物理内存占用率，虚拟内存占用率，句柄个数，线程个数
    mp_bool bFlag = m_stNginxAbnormal.iPmSizeOverTimes > m_stCommonMonitorCfg.iRetryTime ||
                    m_stNginxAbnormal.iVmSizeOverTimes > m_stCommonMonitorCfg.iRetryTime ||
                    m_stNginxAbnormal.iHandlerOverTimes > m_stCommonMonitorCfg.iRetryTime ||
                    m_stNginxAbnormal.iThreadOverTimes > m_stCommonMonitorCfg.iRetryTime;
    if (bFlag) {
        COMMLOG(OS_LOG_INFO,
            "Nginx process is abnormal and will be restarted, iPmSizeOverTimes is %d, \
            iVmSizeOverTimes is %d, iHandlerOverTimes is %d, iThreadOverTimes is %d",
            m_stNginxAbnormal.iPmSizeOverTimes,
            m_stNginxAbnormal.iVmSizeOverTimes,
            m_stNginxAbnormal.iHandlerOverTimes,
            m_stNginxAbnormal.iThreadOverTimes);
        iRet = RestartNginx();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Restart nginx failed, iRet = %d", iRet);
        } else {
            COMMLOG(OS_LOG_INFO, "Nginx has been restarted successfully.");
        }
        return iRet;
    }

    // 检测cpu利用率
    CheckNginxCpuUsage(monitorNginxData.fCpuUsage);

    // 检测nginx日志大小
    if (monitorNginxData.uiNginxLogSize >=
        static_cast<mp_uint32>(m_stNginxMointorCfg.iNginxLogSizeCfg * ABNORMALHANDLER_NUM_1024)) {
        COMMLOG(OS_LOG_INFO,
            "Nginx log size[%d] is overceed[%d]",
            monitorNginxData.uiNginxLogSize,
            m_stNginxMointorCfg.iNginxLogSizeCfg * ABNORMALHANDLER_NUM_1024);
        RotateNginxLog();
    }
    return iRet;
}

mp_int32 AbnormalHandler::InitMonitorNginxData(monitor_data_t &monitorNginxData)
{
    mp_int32 iRet = GetNginxMonitorData(monitorNginxData);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get nginx monitor data failed, iRet = %d", iRet);
        return iRet;
    }

    if (!monitorNginxData.bExist) {
        COMMLOG(OS_LOG_INFO, "Nginx process is not exist and will be started");
        iRet = StartNginx();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Start nginx failed, iRet = %d", iRet);
        }
        return iRet;
    }

    // 打印nginx monitor数据
    COMMLOG(OS_LOG_DEBUG,
        "Nginx monitor data: bExist=%d, iHandlerNum=%d, iThreadNum=%d, ulPmSize=%dK.",
        monitorNginxData.bExist,
        monitorNginxData.iHandlerNum,
        monitorNginxData.iThreadNum,
        monitorNginxData.ulPmSize);
    COMMLOG(OS_LOG_DEBUG,
        "Nginx monitor data: ulVmSize=%dK, fCpuUsage=%f, NginxLogSize=%dK",
        monitorNginxData.ulVmSize,
        monitorNginxData.fCpuUsage,
        monitorNginxData.uiNginxLogSize);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: MonitorDataProcess
Description  : dataProcess进程监控处理函数
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::MonitorDataProcess()
{
    LOGGUARD("");
    // 获取所有配置文件的名称
    mp_string configPathStr = CPath::GetInstance().GetConfPath();
    vector<mp_string> vecFileList;
    CMpFile::GetFolderFile(configPathStr, vecFileList);
    mp_int32 iRet = MP_FAILED;
    // solaris不支持C++11
    for (int i = 0; i < vecFileList.size(); i++) {
        if (vecFileList[i].find(OM_DPP_EXEC_NAME) != std::string::npos) {
            // 替换下划线为空格
            std::string::size_type pos = 0;
            while ((pos = vecFileList[i].find("_")) != std::string::npos) {
                vecFileList[i].replace(pos, 1, " ");
            }
            COMMLOG(OS_LOG_DEBUG, "get dataprocess id, id = %s", vecFileList[i].c_str());
            // 入口处校验命令参数
            CHECK_FAIL_EX(CheckCmdDelimiter(vecFileList[i]));
            iRet = MonitorDataProcessOnce(vecFileList[i]);
            if (iRet != MP_SUCCESS) {
                return iRet;
            }
        }
    }
    
    return MP_SUCCESS;
}

mp_int32 AbnormalHandler::MonitorDataProcessOnce(const mp_string& processName)
{
    LOGGUARD("");
    monitor_data_t monitorDataProcessData;
    mp_int32 iRet = GetDataProcessMonitorData(monitorDataProcessData, processName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get dataprocess monitor data failed, iRet = %d", iRet);
        return iRet;
    }

    if (!monitorDataProcessData.bExist) {
        COMMLOG(OS_LOG_INFO, "dataprocess process is not exist.");
        return iRet;
    }

    // 打印agent monitor数据
    COMMLOG(OS_LOG_DEBUG,
        "DataProcess monitor data: bExist=%d, iHandlerNum=%d, iThreadNum=%d, ulPmSize=%dK",
        monitorDataProcessData.bExist,
        monitorDataProcessData.iHandlerNum,
        monitorDataProcessData.iThreadNum,
        monitorDataProcessData.ulPmSize);
    COMMLOG(OS_LOG_DEBUG,
        "DataProcess monitor data: ulVmSize=%dK, fCpuUsage=%f, ulTmpFileTotalSize=%dByte.",
        monitorDataProcessData.ulVmSize,
        monitorDataProcessData.fCpuUsage,
        monitorDataProcessData.ulTmpFileTotalSize);
    
    // 检测dataprocess监控数据
    CheckDataProcessMonitorValue(monitorDataProcessData);

    // 监测物理内存占用率，虚拟内存占用率，句柄个数，线程个数
    mp_bool bFlag = m_stDataProcessAbnormal.iPmSizeOverTimes > m_stCommonMonitorCfg.iRetryTime ||
                    m_stDataProcessAbnormal.iVmSizeOverTimes > m_stCommonMonitorCfg.iRetryTime ||
                    m_stDataProcessAbnormal.iHandlerOverTimes > m_stCommonMonitorCfg.iRetryTime ||
                    m_stDataProcessAbnormal.iThreadOverTimes > m_stCommonMonitorCfg.iRetryTime;

    if (bFlag) {
        COMMLOG(OS_LOG_INFO,
            "dataProcess process is abnormal, iPmSizeOverTimes is %d,"
            "iVmSizeOverTimes is %d, iHandlerOverTimes is %d, iThreadOverTimes is %d",
            m_stDataProcessAbnormal.iPmSizeOverTimes,
            m_stDataProcessAbnormal.iVmSizeOverTimes,
            m_stDataProcessAbnormal.iHandlerOverTimes,
            m_stDataProcessAbnormal.iThreadOverTimes);
        iRet = KillDataProcess(processName);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "kill dataProcess failed, iRet = %d", iRet);
        } else {
            COMMLOG(OS_LOG_INFO, "kill dataProcess successfully.");
        }
        return iRet;
    }
    
    return iRet;
}


/* ------------------------------------------------------------
Function Name: GetAgentMonitorData
Description  : 获取agent进程所有监控数据
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::GetAgentMonitorData(monitor_data_t& stMonitorData)
{
    (mp_void) GetLogFileTotalSize(stMonitorData.ulLogFileTotalSize);
#ifndef WIN32
    // 获取agent进程id
    // 获取nginx进程id
    mp_string strPidFilePath = CPath::GetInstance().GetLogFilePath(AGENT_PID);
    mp_string strPid;
    vector<mp_string> vecRlt;
    mp_int32 iRet = CMpFile::ReadFile(strPidFilePath, vecRlt);
    if (iRet != MP_SUCCESS || vecRlt.empty()) {
        COMMLOG(OS_LOG_INFO, "read rdagent.pid failed");
        strPid = " ";
    } else {
        strPid = vecRlt.front();
    }
    CHECK_FAIL_EX(CheckCmdDelimiter(strPid));
#ifdef SOLARIS
    mp_string strCmd =
        "ps -aef | grep '" + mp_string(AGENT_EXEC_NAME) + "' |grep '" + strPid +
        "' | grep -v grep | grep -v gdb | grep -v monitor | grep -v vi | grep -v tail | nawk '{print $2}'";
#else
    mp_string strCmd =
        "ps -aef | grep '" + mp_string(AGENT_EXEC_NAME) + "' |grep '" + strPid +
        "' | grep -v grep | grep -v gdb | grep -v monitor | grep -v vi | grep -v tail | awk '{print $2}'";
#endif
    vecRlt.clear();
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((iRet != MP_SUCCESS) || (vecRlt.empty())) {
        COMMLOG(OS_LOG_INFO, "Get agent process ID failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        stMonitorData.bExist = MP_FALSE;
        // 进程不存在按成功处理
        return MP_SUCCESS;
    }

    mp_string strRdAgentLogFilePath = CMpString::BlankComma(CPath::GetInstance().GetLogFilePath(AGENT_LOG_NAME));

    // rdagent.log没有创建的情况下，以上system执行之后新生成的rdagent.log
    // 权限为644或者664，这种情况需要这里改成660
    (void)ChmodFile(strRdAgentLogFilePath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    stMonitorData.bExist = MP_TRUE;

#ifdef HP_UX_IA
    iRet = GetHPMonitorData(vecRlt.front(), stMonitorData);
#else
    iRet = GetMonitorData(vecRlt.front(), stMonitorData);
#endif
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get monitor data failed, iRet = %d", iRet);
    }

    return iRet;
#else
    mp_string strWinAgent = mp_string(AGENT_EXEC_NAME) + ".exe";
    return GetWinMonitorData(strWinAgent, stMonitorData);
#endif
}

mp_string AbnormalHandler::GetNginxPid()
{
    mp_string strPidFilePath = CPath::GetInstance().GetNginxLogsFilePath(NGINX_PID);
    mp_string strPid = " ";
    vector<mp_string> vecRlt;
    mp_int32 iRet = CMpFile::ReadFile(strPidFilePath, vecRlt);
    if (iRet == MP_SUCCESS && !vecRlt.empty()) {
        COMMLOG(OS_LOG_DEBUG, "read ngnix.pid successed");
        strPid = vecRlt.front();
        vecRlt.clear();
    }
    return strPid;
}

/* ------------------------------------------------------------
Function Name: GetNginxMonitorData
Description  : 获取nginx进程所有监控数据
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::GetNginxMonitorData(monitor_data_t& stMonitorData)
{
    // 获取nginx日志大小，单位为k
    mp_string strNginxLogPath = CPath::GetInstance().GetNginxLogsFilePath(NGINX_LOG_FILE);
    (mp_void) CMpFile::FileSize(strNginxLogPath.c_str(), stMonitorData.uiNginxLogSize);
    stMonitorData.uiNginxLogSize = stMonitorData.uiNginxLogSize / ABNORMALHANDLER_NUM_1024;

#ifndef WIN32
    // 获取nginx进程id
    mp_string strPid = GetNginxPid();
    CHECK_FAIL_EX(CheckCmdDelimiter(strPid));
#ifdef SOLARIS
    mp_string strCmd =
        "ps -aef | grep '" + mp_string(NGINX_EXEC_NAME) + "' |grep '" + strPid +
        "' |grep -v grep | grep -v gdb | grep -v monitor | grep -v vi | grep -v tail | nawk '{print $2}'";
#else
    mp_string strCmd = "ps -aef | grep '" + mp_string(NGINX_EXEC_NAME) + "' |grep '" + strPid +
                       "' |grep -v grep | grep -v gdb | grep -v monitor | grep -v vi | grep -v tail | awk '{print $2}'";
#endif

    vector<mp_string> vecRlt;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((iRet != MP_SUCCESS) || (vecRlt.empty())) {
        COMMLOG(OS_LOG_INFO, "Get nginx process ID failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        stMonitorData.bExist = MP_FALSE;
        // 进程不存在按成功处理
        return MP_SUCCESS;
    }
    mp_string strRdAgentLogFilePath = CMpString::BlankComma(CPath::GetInstance().GetLogFilePath(AGENT_LOG_NAME));

    // rdagent.log没有创建的情况下，以上system执行之后新生成的rdagent.log
    // 权限为644或者664，这种情况需要这里改成660
    (void)ChmodFile(strRdAgentLogFilePath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

    stMonitorData.bExist = MP_TRUE;

    for (vector<mp_string>::iterator it = vecRlt.begin(); it != vecRlt.end(); ++it) {
        monitor_data_t monitorData;
#ifdef HP_UX_IA
        iRet = GetHPMonitorData(*it, monitorData);
#else
        iRet = GetMonitorData(*it, monitorData);
#endif
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get monitor data failed, iRet = %d", iRet);
            return iRet;
        }
        stMonitorData = AddMonitorData(stMonitorData, monitorData);
    }
    return MP_SUCCESS;
#else
    mp_string strWinNginx = mp_string(NGINX_EXEC_NAME) + ".exe";
    return GetWinMonitorData(strWinNginx, stMonitorData);
#endif
}

/* ------------------------------------------------------------
Function Name: GetDataProcessMonitorData
Description  : 获取dataprocess进程所有监控数据
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::GetDataProcessMonitorData(monitor_data_t& stMonitorData, const mp_string& processName)
{
    CHECK_FAIL_EX(CheckCmdDelimiter(processName));
#ifdef SOLARIS
    mp_string strCmd = "ps -aef | grep '" + mp_string(processName) +
        "' | grep -v grep | grep -v gdb | grep -v monitor | grep -v vi | grep -v tail | nawk '{print $2}'";
#elif defined(WIN32) && WIN32
    mp_string strCmd = "ps -aef | grep '" + mp_string(NGINX_EXEC_NAME) + "' |grep '"  +
                       "' |grep -v grep | grep -v gdb | grep -v monitor | grep -v vi | grep -v tail | awk '{print $2}'";
#else
    mp_string strCmd = "ps -aef | grep '" + mp_string(processName) +
        "' | grep -v grep | grep -v gdb | grep -v monitor | grep -v vi | grep -v tail | awk '{print $2}'";
#endif
    vector<mp_string> vecRlt;
    vecRlt.clear();
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((iRet != MP_SUCCESS) || (vecRlt.empty())) {
        COMMLOG(OS_LOG_INFO, "Get dataproces process ID failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        stMonitorData.bExist = MP_FALSE;
        // 进程不存在按成功处理
        return MP_SUCCESS;
    }
    stMonitorData.bExist = MP_TRUE;

#ifdef HP_UX_IA
    iRet = GetHPMonitorData(vecRlt.front(), stMonitorData);
#else
    iRet = GetMonitorData(vecRlt.front(), stMonitorData);
#endif
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get monitor data failed, iRet = %d", iRet);
    }

    return iRet;
#ifdef WIN32
    mp_string strWinDataProcess = mp_string(OM_DPP_EXEC_NAME) + ".exe";
    return GetWinMonitorData(strWinDataProcess, stMonitorData);
#endif
    
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetHPMonitorData
Description  : 获取hp环境agent进程所有监控数据
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::GetHPMonitorData(mp_string strProcessID, monitor_data_t& stMonitorData)
{
    // hp下无法获取线程数量和句柄数量
    stMonitorData.iHandlerNum = 0;
    stMonitorData.iThreadNum = 0;

    // 将top命令转存为文件
    mp_string strTopFilePath = CPath::GetInstance().GetTmpFilePath(TOP_TMP_FILE);
    CHECK_FAIL_EX(CheckCmdDelimiter(strTopFilePath));
    strTopFilePath = CMpString::BlankComma(strTopFilePath);
    mp_string strCmd = mp_string(HP_TOP_CMD) + " \"" + strTopFilePath + "\"";
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));

    // 获取物理内存
    vector<mp_string> vecRlt;
    strCmd = mp_string("cat \"") + strTopFilePath + "\" | grep rdagent | awk '{print $8}'";
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((iRet != MP_SUCCESS) || (vecRlt.empty())) {
        COMMLOG(OS_LOG_ERROR, "Get pysical memory failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return iRet;
    }
    // 只对返回数据的第一个元素进行处理
    stMonitorData.ulPmSize = GetKSize(vecRlt.front());

    // 获取虚拟内存
    vecRlt.clear();
    strCmd = mp_string("cat \"") + strTopFilePath + "\" | grep rdagent | awk '{print $7}'";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((iRet != MP_SUCCESS) || (vecRlt.empty())) {
        COMMLOG(OS_LOG_ERROR, "Get virtual memory failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return iRet;
    }
    // Agent只会有一个进程，只对返回数据的第一个元素进行处理
    stMonitorData.ulVmSize = GetKSize(vecRlt.front());

    // 获取cpu利用率
    vecRlt.clear();
    strCmd = mp_string("cat \"") + strTopFilePath + "\" | grep rdagent | awk '{print $11}'";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((iRet != MP_SUCCESS) || (vecRlt.empty())) {
        COMMLOG(OS_LOG_ERROR, "Get cpu usage failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return iRet;
    }
    stMonitorData.fCpuUsage = (mp_float)atof(vecRlt.front().c_str());

    // 删除临时文件
    iRet = CMpFile::DelFile(strTopFilePath);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Delete tmp file failed, ret is %d", iRet);
    }

    return iRet;
}

mp_int32 GetThreadAndHandler(const mp_string& strProcessID, monitor_data_t& stMonitorData)
{
    // 获取线程数
    mp_string strCmd;
    CHECK_FAIL_EX(CheckCmdDelimiter(strProcessID));
#ifdef AIX
    strCmd = "ps -p " + strProcessID + " -o thcount | grep -v THCNT";
#else
    strCmd = "ps -p " + strProcessID + " -o nlwp | grep -v NLWP";
#endif
    vector<mp_string> vecRlt;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((iRet != MP_SUCCESS) || (vecRlt.empty())) {
        COMMLOG(OS_LOG_ERROR, "Get thread num failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return iRet;
    }

    stMonitorData.iThreadNum = atoi(CMpString::Trim(vecRlt.front()).c_str());
    COMMLOG(OS_LOG_DEBUG, "Thread number of process rdagent is: %d.", stMonitorData.iThreadNum);

    // 获取句柄数
    strCmd = "ls /proc/" + strProcessID + "/fd | wc -l";
    vecRlt.clear();
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((iRet != MP_SUCCESS) || (vecRlt.empty())) {
        COMMLOG(OS_LOG_ERROR, "Get handler num failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return iRet;
    }
    stMonitorData.iHandlerNum = atoi(vecRlt.front().c_str());
    COMMLOG(OS_LOG_DEBUG, "Handler number of process rdagent is: %d.", stMonitorData.iHandlerNum);
    return iRet;
}

/* ------------------------------------------------------------
Function Name: GetMonitorData
Description  : 根据进程id获取非hp环境各种监控数据
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::GetMonitorData(const mp_string& strProcessID, monitor_data_t& stMonitorData)
{
    // 获取线程数和句柄数
    mp_int32 iRet = GetThreadAndHandler(strProcessID, stMonitorData);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    vector<mp_string> vecRlt;
    mp_string strCmd;
    // 获取物理内存(进程使用的总物理内存数, Kbytes字节)
    CHECK_FAIL_EX(CheckCmdDelimiter(strProcessID));
#ifdef SOLARIS
    strCmd = "ps -efo rss,pid,comm | grep -w '" + strProcessID + "' | grep -v 'grep' | nawk '{print $1}'";
#else
    strCmd = "ps auxw | grep -w '" + strProcessID + "' | grep -v 'grep' | awk '{print $6}'";
#endif
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    mp_bool bFlag = (iRet != MP_SUCCESS) || (vecRlt.empty());
    if (bFlag) {
        COMMLOG(OS_LOG_ERROR, "Get pysical memory failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return iRet;
    }
    stMonitorData.ulPmSize = (mp_uint64)(atol(vecRlt.front().c_str()));
    // 获取虚拟内存(读取的数值单位为 bytes字节(包含code+data+stack))
#ifdef SOLARIS
    strCmd = "ps -efo vsz,pid,comm | grep -w '" + strProcessID + "' | grep -v 'grep' | nawk '{print $1}'";
#else
    strCmd = "ps -p " + strProcessID + " -o vsz | grep -v VSZ";
#endif
    vecRlt.clear();
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    bFlag = (iRet != MP_SUCCESS) || (vecRlt.empty());
    if (bFlag) {
        COMMLOG(OS_LOG_ERROR, "Get virtual memory failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return iRet;
    }
    stMonitorData.ulVmSize = (mp_uint64)(atol(vecRlt.front().c_str()));
    // 获取CPU利用率
#ifdef SOLARIS
    strCmd = "ps -efo pcpu,pid,comm | grep -w '" + strProcessID + "' | grep -v 'grep' | nawk '{print $1}'";
#else
    strCmd = "ps auxw | grep -w '" + strProcessID + "' | grep -v 'grep' | awk '{print $3}'";
#endif
    vecRlt.clear();
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet != MP_SUCCESS || vecRlt.empty()) {
        COMMLOG(OS_LOG_ERROR, "Get cpu usage failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return iRet;
    }
    stMonitorData.fCpuUsage = (mp_float)atof(vecRlt.front().c_str());

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: AddMonitorData
Description  : 对多条监控数据进行数据叠加
Others       :-------------------------------------------------------- */
monitor_data_t AbnormalHandler::AddMonitorData(monitor_data_t& stMonitorData1, monitor_data_t& stMonitorData2)
{
    monitor_data_t stSumMointorData;
    stSumMointorData.bExist = stMonitorData1.bExist || stMonitorData2.bExist;
    stSumMointorData.fCpuUsage = stMonitorData1.fCpuUsage + stMonitorData2.fCpuUsage;
    stSumMointorData.iHandlerNum = stMonitorData1.iHandlerNum + stMonitorData2.iHandlerNum;
    stSumMointorData.ulPmSize = stMonitorData1.ulPmSize + stMonitorData2.ulPmSize;
    stSumMointorData.iThreadNum = stMonitorData1.iThreadNum + stMonitorData2.iThreadNum;
    stSumMointorData.ulVmSize = stMonitorData1.ulVmSize + stMonitorData2.ulVmSize;
    (stMonitorData1.ulTmpFileTotalSize == 0)
        ? (stSumMointorData.ulTmpFileTotalSize = stMonitorData2.ulTmpFileTotalSize)
        : (stSumMointorData.ulTmpFileTotalSize = stMonitorData1.ulTmpFileTotalSize);
    (stMonitorData1.ulLogFileTotalSize == 0)
        ? (stSumMointorData.ulLogFileTotalSize = stMonitorData2.ulLogFileTotalSize)
        : (stSumMointorData.ulLogFileTotalSize = stMonitorData1.ulLogFileTotalSize);
    (stMonitorData1.uiNginxLogSize == 0) ? (stSumMointorData.uiNginxLogSize = stMonitorData2.uiNginxLogSize)
                                         : (stSumMointorData.uiNginxLogSize = stMonitorData1.uiNginxLogSize);
    return stSumMointorData;
}

/* ------------------------------------------------------------
Function Name: ClearMonitorData
Description  : 清除监控数据
Others       :-------------------------------------------------------- */
mp_void AbnormalHandler::ClearMonitorData(monitor_data_t& stMonitorData)
{
    stMonitorData.bExist = MP_FALSE;
    stMonitorData.ulPmSize = 0;
    stMonitorData.ulVmSize = 0;
    stMonitorData.iHandlerNum = 0;
    stMonitorData.iThreadNum = 0;
    stMonitorData.fCpuUsage = 0.0;
    stMonitorData.ulTmpFileTotalSize = 0;
    stMonitorData.uiNginxLogSize = 0;
}

/* ------------------------------------------------------------
Function Name: ClearAbnormalOccurTimes
Description  : 清除异常发生次数
Others       :-------------------------------------------------------- */
mp_void AbnormalHandler::ClearAbnormalOccurTimes(abnormal_occur_times_t& stAbnormalOccurTimes)
{
    stAbnormalOccurTimes.iPmSizeOverTimes = 0;
    stAbnormalOccurTimes.iVmSizeOverTimes = 0;
    stAbnormalOccurTimes.iHandlerOverTimes = 0;
    stAbnormalOccurTimes.iThreadOverTimes = 0;
    stAbnormalOccurTimes.iCpuUsageOverTimes = 0;
    stAbnormalOccurTimes.iTmpFileSizeOverTimes = 0;
}

mp_int32 AbnormalHandler::StopAgent()
{
    LOGGUARD("");
#ifdef WIN32
    mp_string strCmd = mp_string("sc stop ") + AGENT_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + AGENT_EXEC_NAME;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif
    return MP_SUCCESS;
}

mp_void AbnormalHandler::StopPlugins()
{
    LOGGUARD("");
    mp_string pluginPidDir = CPath::GetInstance().GetPluginPidPath();
    std::vector<mp_string> PluginFileNames;
    CMpFile::GetFolderFile(pluginPidDir, PluginFileNames);

    mp_string strPID;
    std::ifstream stream;
    for (const mp_string& strPluginName : PluginFileNames) {
        mp_string pluginPidPath = pluginPidDir + PATH_SEPARATOR + strPluginName;
        stream.open(pluginPidPath.c_str(), std::ios::in);
        if (!stream.is_open()) {
            ERRLOG("Failed to open file(%s).", pluginPidPath.c_str());
            continue;
        }
        getline(stream, strPID);
        ERRLOG("Begin to kill process %s plugin:%s.", strPID.c_str(), strPluginName.c_str());
#ifdef WIN32
        mp_string strCommand = "cmd.exe /c taskkill /f /pid " + strPID + " > nul";
        mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCommand);
#else
        CRootCaller rootCaller;
        mp_int32 iRet = rootCaller.Exec((mp_uint32)ROOT_COMMAND_KILL, strPID, NULL);
#endif
        if (iRet != MP_SUCCESS) {
            ERRLOG("Fail to kill process %s plugin:%s.", strPID.c_str(), strPluginName.c_str());
        }
        stream.close();
        CMpFile::DelFile(pluginPidPath);
    }
}

/* ------------------------------------------------------------
Function Name: StartAgent
Description  : 启动agent进程
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::StartAgent()
{
    LOGGUARD("");
#ifdef WIN32
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    // 校验脚本签名
    mp_int32 iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init crypt failed, ret = %d.", iRet);
        return iRet;
    }
    // 程序即将退出，此处不判断返回值
    (mp_void) FinalizeCrypt();

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Script sign check failed, script name is \"%s\", iRet = %d.", START_SCRIPT, iRet);
        return iRet;
    }
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + AGENT_EXEC_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    SecureCmdExecutor::ExecuteWithoutEcho("%s %s", {strCmd, AGENT_EXEC_NAME});
#endif
    RestartNginx();
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: RestartAgent
Description  : 重启agent进程
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::RestartAgent()
{
    LOGGUARD("");

#ifdef WIN32
    mp_string strCmd = mp_string("sc stop ") + AGENT_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    StartAgent();
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + AGENT_EXEC_NAME;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + AGENT_EXEC_NAME;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif
    RestartNginx();
    // 重启成功，清空异常监控数据
    ClearAbnormalOccurTimes(m_stAgentAbnormal);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: StartNginx
Description  : 启动nginx进程
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::StartNginx()
{
    LOGGUARD("");
#ifdef WIN32
    // windows下nginx服务需要先停止再启动
    mp_string strCmd = mp_string("sc stop ") + NGINX_SERVICE_NAME;
    CSystemExec::ExecSystemWithoutEcho(strCmd);

    strCmd = CPath::GetInstance().GetBinFilePath(AGENTCLI_EXE);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_START;
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Start nginx failed, iRet = %d.", iRet);
        return iRet;
    }
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(AGENTCLI_UNIX);
    strCmd = CMpString::BlankComma(strCmd);
    SecureCmdExecutor::ExecuteWithoutEcho("%s %s", {strCmd, NGINX_START});
#endif
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: RestartNginx
Description  : 重新启动nginx进程
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::RestartNginx()
{
    LOGGUARD("");
    INFOLOG("Start restart nginx");
#ifdef WIN32
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(PROCESS_STOP_SCRIPT);
    // 校验脚本签名
    mp_int32 iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init crypt failed, ret = %d.", iRet);
        return iRet;
    }
    // 程序即将退出，此处不判断返回值
    (mp_void) FinalizeCrypt();

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Script sign check failed, script name is \"%s\", iRet = %d.", PROCESS_STOP_SCRIPT, iRet);
        return iRet;
    }
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_AS_PARAM_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_AS_PARAM_NAME;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));

    strCmd = CPath::GetInstance().GetBinFilePath(AGENTCLI_UNIX);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_START;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif
    // 重启成功，清空异常监控数据
    ClearAbnormalOccurTimes(m_stNginxAbnormal);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: KillDataProcess
Description  : 杀掉dataprocess进程
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::KillDataProcess(const mp_string &processName)
{
    LOGGUARD("");
#ifdef WIN32
#else
    CHECK_FAIL_EX(CheckCmdDelimiter(processName));
    vector<mp_string> vecResult;
    mp_string strCmd = "ps -aef | grep '" + mp_string(processName) + "' | grep -v grep | awk '{print $2}'";
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecResult);
    if (iRet != MP_SUCCESS || vecResult.empty()) {
        COMMLOG(OS_LOG_INFO, "query dataprocess pid, process name= %s", processName.c_str());
        return MP_FAILED;
    }
    CRootCaller rootCaller;
    iRet = rootCaller.Exec(ROOT_COMMAND_KILL, " " + vecResult.front(), NULL);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Kill dataprocess failed, pid= %s, iRet = %d", vecResult.front().c_str(), iRet);
        return MP_FAILED;
    }
    // 杀掉进程后，清空异常监控数据
    ClearAbnormalOccurTimes(m_stDataProcessAbnormal);
#endif
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: SendCPUAlarm
Description  : 发送agent cpu超出阈值告警
Others       :-------------------------------------------------------- */
mp_void AbnormalHandler::SendCPUAlarm(mp_int32 iProcessType)
{
    LOGGUARD("");
    // 从配置文件读取门限值
    ostringstream oss;
    if (iProcessType == AGENT_PROCESS) {  // Agent CPU
        oss << m_stAgentMointorCfg.fCpuUsageCfg;
    } else {  // NGINX CPU
        oss << m_stNginxMointorCfg.fCpuUsageCfg;
    }
}

/* ------------------------------------------------------------
Function Name: SendAgentCPUAlarm
Description  : 发送agent cpu超出阈值恢复告警
Others       :-------------------------------------------------------- */
mp_void AbnormalHandler::ResumeCPUAlarm()
{
    LOGGUARD("");
    // 从配置文件读取门限值
    ostringstream oss;
    oss << m_stAgentMointorCfg.fCpuUsageCfg;
}

/* ------------------------------------------------------------
Function Name: DeleteTmpFile
Description  : 产出tmp目录下时间超长的临时文件
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::DeleteTmpFile()
{
    LOGGUARD("");
    mp_string strTmpFileFolder = CPath::GetInstance().GetTmpPath();
    vector<mp_string> vecFileList;
    mp_time timeNow;
    CMpTime::Now(timeNow);
    CHECK_FAIL_EX(CMpFile::GetFolderFile(strTmpFileFolder, vecFileList));
    mp_int32 iRet = MP_SUCCESS;
    for (vector<mp_string>::iterator it = vecFileList.begin(); it != vecFileList.end(); ++it) {
        mp_time timeLastModified = 0;
        mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(*it);
        iRet = CMpFile::GetlLastModifyTime(strFilePath.c_str(), timeLastModified);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get last modify time of %s failed.", BaseFileName(strFilePath).c_str());
            continue;
        }
        // 存留时间超过最大时间，删除
        if (timeNow < timeLastModified || timeNow - timeLastModified > MAX_TMP_EXIST_TIME) {
            mp_int32 iRet1 = CMpFile::DelFile(strFilePath);
            if (iRet1 != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "Delete file \"%s\" failed.", BaseFileName(strFilePath).c_str());
                iRet = iRet1;
            }
        }
    }

    return iRet;
}

/* ------------------------------------------------------------
Function Name: GetTmpFileTotalSize
Description  : 获取tmp目录总大小，单位字节
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::GetTmpFileTotalSize(mp_uint64& ulSize)
{
    ulSize = 0;
    mp_string strTmpFileFolder = CPath::GetInstance().GetTmpPath();
    vector<mp_string> vecFileList;
    CHECK_FAIL_EX(CMpFile::GetFolderFile(strTmpFileFolder, vecFileList));
    for (vector<mp_string>::iterator it = vecFileList.begin(); it != vecFileList.end(); ++it) {
        mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(*it);
        mp_uint32 uiSize = 0;
        CHECK_FAIL_EX(CMpFile::FileSize(strFilePath.c_str(), uiSize));
        ulSize += uiSize;
    }
    return MP_SUCCESS;
}

mp_int32 AbnormalHandler::GetLogFileTotalSize(mp_uint64& ulSize)
{
    ulSize = 0;
    std::string sceneType;
    mp_int32 iRet = CIP::GetInstallScene(sceneType);
    if (iRet != MP_SUCCESS) {
        ERRLOG("The testcfg.tmp file format error");
        return MP_FAILED;
    }
    if (sceneType == "1") {
        // 内置agent不检查日志目录大小
        return MP_SUCCESS;
    }
#ifdef WIN32
    mp_string strTmpFile = CPath::GetInstance().GetTmpFilePath("filesize" + CUniqueID::GetInstance().GetString());
    mp_string strDir = CPath::GetInstance().GetLogPath();
    mp_string strCMD = "cmd.exe /c for /f %a in ('dir " + strDir + " /b /s /a-d') do echo %~za >> " + strTmpFile;
    iRet = CSystemExec::ExecSystemWithoutEcho(strCMD);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Exec dir %s failed.", strDir.c_str());
        return MP_FAILED;
    }
    std::vector<mp_string> vecRet;
    CMpFile::ReadFile(strTmpFile, vecRet);
    CMpFile::DelFile(strTmpFile);
    for (const mp_string& str : vecRet) {
        ulSize += atoi(str.c_str()); // unit Byte
    }
    ulSize /= ABNORMALHANDLER_NUM_1024; // unit KB
#else
    std::vector<mp_string> vecRet;
    CRootCaller rootCaller;
    mp_string strParam = CPath::GetInstance().GetLogPath() + NODE_COLON + CPath::GetInstance().GetSlogPath();
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_DU, strParam, &vecRet);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Exec du %s failed.", strParam.c_str());
        return MP_FAILED;
    }
    vecRet = Awk(vecRet, AWK_COL_FIRST_1);
    for (const mp_string& str : vecRet) {
        ulSize += atoi(str.c_str()); // unit KB
    }
#endif
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetKSize
Description  : 从获取到的容量字符串中对单位为k和m进行计算，统一转换成k
Others       :-------------------------------------------------------- */
mp_uint64 AbnormalHandler::GetKSize(const mp_string& strSize)
{
    // CodeDex误报，Type Mismatch:Signed to Unsigned
    if (mp_string::npos != strSize.find("M")) {
        return mp_uint64(atol(strSize.c_str())) * ABNORMALHANDLER_NUM_1024;
    } else if (mp_string::npos != strSize.find("K")) {
        return mp_uint64(atol(strSize.c_str()));
    } else {
        return mp_uint64(atol(strSize.c_str()) / ABNORMALHANDLER_NUM_1024);
    }
}

mp_bool AbnormalHandler::NeedExit()
{
    return m_bNeedExit;
}

mp_void AbnormalHandler::SetThreadStatus(mp_int32 iThreadStatus)
{
    m_iThreadStatus = iThreadStatus;
}

mp_void AbnormalHandler::RotateNginxLog()
{
    mp_int32 iRet = SecureCom::SysExecScript(ROATE_NGINX_LOG_SCRIPT, "", NULL);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Roate nginx log failed, iRet = %d.", iRet);
    }
}

/* ------------------------------------------------------------
Function Name: GetAgentMonitorCfg
Description  : 从配置文件中获取agent监控配置
Others       :-------------------------------------------------------- */
monitor_process_config_t& AbnormalHandler::GetAgentMonitorCfg()
{
    CConfigXmlParser::GetInstance().GetValueBool(MONITOR_SECTION, AGENT_EXEC_NAME, m_stAgentMointorCfg.bMonitored);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, AGENT_EXEC_NAME, THRD_CNT, m_stAgentMointorCfg.iThreadNumCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, AGENT_EXEC_NAME, HANDLER_CNT, m_stAgentMointorCfg.iHandlerNumCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, AGENT_EXEC_NAME, PM_SIZE, m_stAgentMointorCfg.iPmSizeCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, AGENT_EXEC_NAME, VM_SIZE, m_stAgentMointorCfg.iVmSizeCfg);
    CConfigXmlParser::GetInstance().GetValueFloat(
        MONITOR_SECTION, AGENT_EXEC_NAME, CPU_USAGE, m_stAgentMointorCfg.fCpuUsageCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, AGENT_EXEC_NAME, TMPFILE_SIZE, m_stAgentMointorCfg.iTmpFileTotalSizeCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, AGENT_EXEC_NAME, LOGFILE_SIZE, m_stAgentMointorCfg.iLogFileTotalSizeCfg);
    CConfigXmlParser::GetInstance().GetValueFloat(
        MONITOR_SECTION, AGENT_EXEC_NAME, LOGFILE_SIZE_ALARM_THRESHOLD,
        m_stAgentMointorCfg.fLogAlarmPercentage);
    return m_stAgentMointorCfg;
}

/* ------------------------------------------------------------
Function Name: GetDataProcessMonitorCfg
Description  : 从配置文件中获取dataProcess监控配置
Others       :-------------------------------------------------------- */
monitor_process_config_t& AbnormalHandler::GetDataProcessMonitorCfg()
{
    CConfigXmlParser::GetInstance().GetValueBool(
        MONITOR_SECTION, OM_DPP_EXEC_NAME, m_stDataProcessMointorCfg.bMonitored);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, OM_DPP_EXEC_NAME, THRD_CNT, m_stDataProcessMointorCfg.iThreadNumCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, OM_DPP_EXEC_NAME, HANDLER_CNT, m_stDataProcessMointorCfg.iHandlerNumCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, OM_DPP_EXEC_NAME, PM_SIZE, m_stDataProcessMointorCfg.iPmSizeCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, OM_DPP_EXEC_NAME, VM_SIZE, m_stDataProcessMointorCfg.iVmSizeCfg);
    CConfigXmlParser::GetInstance().GetValueFloat(
        MONITOR_SECTION, OM_DPP_EXEC_NAME, CPU_USAGE, m_stDataProcessMointorCfg.fCpuUsageCfg);
    return m_stDataProcessMointorCfg;
}

/* ------------------------------------------------------------
Function Name: GetNginxMonitorCfg
Description  : 从配置文件中获取nginx监控配置
Others       :-------------------------------------------------------- */
monitor_process_config_t& AbnormalHandler::GetNginxMonitorCfg()
{
    CConfigXmlParser::GetInstance().GetValueBool(MONITOR_SECTION, CFG_NGINX_SECTION, m_stNginxMointorCfg.bMonitored);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, CFG_NGINX_SECTION, THRD_CNT, m_stNginxMointorCfg.iThreadNumCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, CFG_NGINX_SECTION, HANDLER_CNT, m_stNginxMointorCfg.iHandlerNumCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, CFG_NGINX_SECTION, PM_SIZE, m_stNginxMointorCfg.iPmSizeCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, CFG_NGINX_SECTION, VM_SIZE, m_stNginxMointorCfg.iVmSizeCfg);
    CConfigXmlParser::GetInstance().GetValueFloat(
        MONITOR_SECTION, CFG_NGINX_SECTION, CPU_USAGE, m_stNginxMointorCfg.fCpuUsageCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, CFG_NGINX_SECTION, NGINX_LOG_SIZE, m_stNginxMointorCfg.iNginxLogSizeCfg);
    return m_stNginxMointorCfg;
}

/* ------------------------------------------------------------
Function Name: GetCommonMonitorCfg
Description  : 从配置文件中获取通用监控配置
Others       :-------------------------------------------------------- */
monitor_common_config_t& AbnormalHandler::GetCommonMonitorCfg()
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, RETRY_TIME, m_stCommonMonitorCfg.iRetryTime);
    if (iRet != MP_SUCCESS) {
        m_stCommonMonitorCfg.iRetryTime = DEFAULT_XML_RETRY_TIME_VALUE;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        MONITOR_SECTION, MONITOR_INTERVAL, m_stCommonMonitorCfg.iMonitorInterval);
    if (iRet != MP_SUCCESS) {
        m_stCommonMonitorCfg.iRetryTime = DEFAULT_XML_INTERVAL_VALUE;
    }
    return m_stCommonMonitorCfg;
}

/* ------------------------------------------------------------
Function Name: CheckNginxMonitorValue
Description  : 检查nginx监控数据
Others       :-------------------------------------------------------- */
mp_void AbnormalHandler::CheckNginxMonitorValue(monitor_data_t& monitorNginxData)
{
    // 单位转换
    // 配置文件中涉及容量的单位均为M，而此处PmSize容量为Kbyte，VmSize单位为Kbyte
    mp_uint64 ulTransPmSizeCfg = static_cast<mp_uint64>(m_stNginxMointorCfg.iPmSizeCfg) * ABNORMALHANDLER_NUM_1024;
    mp_uint64 ulTransVmSizeCfg = static_cast<mp_uint64>(m_stNginxMointorCfg.iVmSizeCfg) * ABNORMALHANDLER_NUM_1024;
    CHECK_VALUE(monitorNginxData.ulPmSize, ulTransPmSizeCfg, m_stNginxAbnormal.iPmSizeOverTimes);
    CHECK_VALUE(monitorNginxData.ulVmSize, ulTransVmSizeCfg, m_stNginxAbnormal.iVmSizeOverTimes);
    CHECK_VALUE(monitorNginxData.iHandlerNum, m_stNginxMointorCfg.iHandlerNumCfg, m_stNginxAbnormal.iHandlerOverTimes);
    CHECK_VALUE(monitorNginxData.iThreadNum, m_stNginxMointorCfg.iThreadNumCfg, m_stNginxAbnormal.iThreadOverTimes);
    CHECK_VALUE(monitorNginxData.fCpuUsage, m_stNginxMointorCfg.fCpuUsageCfg, m_stNginxAbnormal.iCpuUsageOverTimes);
}

/* ------------------------------------------------------------
Function Name: CheckAgentMonitorValue
Description  : 检查agent监控数据
Others       :-------------------------------------------------------- */
mp_void AbnormalHandler::CheckAgentMonitorValue(monitor_data_t& monitorAgentData)
{
    // 单位转换
    // 配置文件中涉及容量的单位均为M，而此处PmSize容量为Kbyte，VmSize单位为Kbyte，iTmpFile总容量的单位为byte
    mp_uint64 ulTransPmSizeCfg = static_cast<mp_uint64>(m_stAgentMointorCfg.iPmSizeCfg) * ABNORMALHANDLER_NUM_1024;
    mp_uint64 ulTransVmSizeCfg = static_cast<mp_uint64>(m_stAgentMointorCfg.iVmSizeCfg) * ABNORMALHANDLER_NUM_1024;
    mp_uint64 ulTmpFileSizeCfg = static_cast<mp_uint64>(m_stAgentMointorCfg.iTmpFileTotalSizeCfg) *
                                 ABNORMALHANDLER_NUM_1024 * ABNORMALHANDLER_NUM_1024;
    CHECK_VALUE(monitorAgentData.ulPmSize, ulTransPmSizeCfg, m_stAgentAbnormal.iPmSizeOverTimes);
    CHECK_VALUE(monitorAgentData.ulVmSize, ulTransVmSizeCfg, m_stAgentAbnormal.iVmSizeOverTimes);
    CHECK_VALUE(monitorAgentData.iHandlerNum, m_stAgentMointorCfg.iHandlerNumCfg, m_stAgentAbnormal.iHandlerOverTimes);
    CHECK_VALUE(monitorAgentData.iThreadNum, m_stAgentMointorCfg.iThreadNumCfg, m_stAgentAbnormal.iThreadOverTimes);
    CHECK_VALUE(monitorAgentData.fCpuUsage, m_stAgentMointorCfg.fCpuUsageCfg, m_stAgentAbnormal.iCpuUsageOverTimes);
    CHECK_VALUE(monitorAgentData.ulTmpFileTotalSize, ulTmpFileSizeCfg, m_stAgentAbnormal.iTmpFileSizeOverTimes);
}

/* ------------------------------------------------------------
Function Name: CheckDataProcessMonitorValue
Description  : 检查dataProcess监控数据
Others       :-------------------------------------------------------- */
mp_void AbnormalHandler::CheckDataProcessMonitorValue(monitor_data_t& monitorAgentData)
{
        // 单位转换
    // 配置文件中涉及容量的单位均为M，而此处PmSize容量为Kbyte，VmSize单位为Kbyte，iTmpFile总容量的单位为byte
    mp_uint64 ulTransPmSizeCfg = 0;
    mp_uint64 ulTransVmSizeCfg = 0;
    ulTransPmSizeCfg = static_cast<mp_uint64>(m_stDataProcessMointorCfg.iPmSizeCfg) * ABNORMALHANDLER_NUM_1024;
    ulTransVmSizeCfg = static_cast<mp_uint64>(m_stDataProcessMointorCfg.iVmSizeCfg) * ABNORMALHANDLER_NUM_1024;
    CHECK_VALUE(monitorAgentData.ulPmSize, ulTransPmSizeCfg,
        m_stDataProcessAbnormal.iPmSizeOverTimes);
    CHECK_VALUE(monitorAgentData.ulVmSize, ulTransVmSizeCfg,
        m_stDataProcessAbnormal.iVmSizeOverTimes);
    CHECK_VALUE(monitorAgentData.iHandlerNum, m_stDataProcessMointorCfg.iHandlerNumCfg,
        m_stDataProcessAbnormal.iHandlerOverTimes);
    CHECK_VALUE(monitorAgentData.iThreadNum, m_stDataProcessMointorCfg.iThreadNumCfg,
        m_stDataProcessAbnormal.iThreadOverTimes);
    CHECK_VALUE(monitorAgentData.fCpuUsage, m_stDataProcessMointorCfg.fCpuUsageCfg,
        m_stDataProcessAbnormal.iCpuUsageOverTimes);
}

mp_void AbnormalHandler::CheckAgentCpuUsage(mp_float fCpuUsage)
{
    // 检测cpu利用率
    if ((m_stAgentAbnormal.iCpuUsageOverTimes > m_stCommonMonitorCfg.iRetryTime) && !m_bAgentCpuAlarmSend) {
        // 发送cpu利用率过高告警，只要进程超高都要发送
        COMMLOG(OS_LOG_INFO,
            "Agent process cpu usage overceed %f over %d times.",
            m_stAgentMointorCfg.fCpuUsageCfg,
            m_stAgentAbnormal.iCpuUsageOverTimes);
        SendCPUAlarm(AGENT_PROCESS);
        m_stAgentAbnormal.iCpuUsageOverTimes = 0;
        m_iAgentAlarmSendFailedTimes = 0;
        m_bAgentCpuAlarmSend = MP_TRUE;
        m_bAgentCpuAlarmResumed = MP_FALSE;
        m_bNeedResumed = MP_TRUE;

        // 失败超过指定次数后，不再发送恢复告警
        if (m_iAgentAlarmSendFailedTimes > ALARM_SEND_FAILED_TIME) {
            COMMLOG(OS_LOG_INFO,
                "Alarm has been sent failed over %d times and will not send again.",
                ALARM_SEND_FAILED_TIME);
            m_stAgentAbnormal.iCpuUsageOverTimes = 0;
            m_bAgentCpuAlarmSend = MP_TRUE;
            m_bAgentCpuAlarmResumed = MP_FALSE;
        }
    }

    // agent和nginx进程cpu利用率都低于阈值才发送恢复告警
    if (fCpuUsage <= m_stAgentMointorCfg.fCpuUsageCfg) {
        m_stAgentAbnormal.iCpuUsageOverTimes = 0;
        m_bAgentCpuAlarmResumed = MP_TRUE;
        m_bNginxCpuAlarmResumed = MP_FALSE;
    }
}


mp_void AbnormalHandler::GetAgentSpaceUsage(mp_string& agentSpaceUse, const mp_string& strAgentRootPath)
{
    mp_string strCmd = "";
#ifdef WIN32
    mp_int32 pos = strAgentRootPath.find(':', 0);
    mp_string agentInstallDisk = "";
    if (pos != string::npos) {
        agentInstallDisk = strAgentRootPath.substr(pos - 1, 1);
    } else {
        WARNLOG("Get agent disk failed.");
        return;
    }
    strCmd = "wmic logicaldisk where \"DeviceID='" + agentInstallDisk + ":'\" get FreeSpace";
    vector<mp_string> vecRlt;
    mp_int32 memorySite = 2;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((iRet != MP_SUCCESS) || (vecRlt.size() < memorySite)) {
        WARNLOG("Get agent directory free space failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return;
    }
    mp_string agentFreeSpace = vecRlt[1].erase(vecRlt[1].find(' '));

    vecRlt.clear();
    strCmd = "wmic logicaldisk where \"DeviceID='" + agentInstallDisk + ":'\" get Size";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((iRet != MP_SUCCESS) || (vecRlt.size() < memorySite)) {
        WARNLOG("Get agent directory total size failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return;
    }
    mp_string agentTotalSize = vecRlt[1].erase(vecRlt[1].find(' '));

    INFOLOG("AgentFreeSpace is %s, AgentTotalSize is %s.", agentFreeSpace.c_str(), agentTotalSize.c_str());
    mp_int64 spaceUse = 100 - static_cast<mp_int64>((CMpString::SafeStoll(agentFreeSpace) * 100) /
                        CMpString::SafeStoll(agentTotalSize));
    agentSpaceUse = std::to_string(spaceUse) + "%";
#else
#if defined LINUX
    strCmd = "df -k " + strAgentRootPath + "| grep -n '' | awk 'END{print $5}'";
#elif defined AIX
    strCmd = "df -k " + strAgentRootPath + "| grep -n '' | awk '{print $4}' | sed -n '2p'";
#elif defined SOLARIS
    strCmd = "df -k " + strAgentRootPath + "|awk '{print $5}' | sed -n '2p'";
#endif
    vector<mp_string> vecRlt;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((iRet != MP_SUCCESS) || (vecRlt.empty())) {
        WARNLOG("Get agent directory free space failed, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return;
    }
    agentSpaceUse = vecRlt.front();
#endif
}

mp_void AbnormalHandler::CheckAgentFreeSpace()
{
    LOGGUARD("");
    mp_string strAgentRootPath = CPath::GetInstance().GetRootPath();
    DBGLOG("Agent root path is %s", strAgentRootPath.c_str());
    mp_string agentSpaceUse = "";
    GetAgentSpaceUsage(agentSpaceUse, strAgentRootPath);

    if (agentSpaceUse == "") {
        WARNLOG("Get agent space info failed.");
        return;
    }
    INFOLOG("Get agent space usage success, space usage is %s", agentSpaceUse.c_str());
    mp_int64 agentSpaceUseValue = CMpString::SafeStoll(agentSpaceUse.substr(0, agentSpaceUse.size() - 1));
    if (agentSpaceUseValue > MEMORY_ALARM_DEFAULT && !m_bAgentSpaceAlarmSend) {
        mp_string params = strAgentRootPath;
        WARNLOG("Send memory warning-alarm %s, params is: %s", AGENT_MEMORY_WARNING_ALARM.c_str(), params.c_str());
        AlarmMgr::GetInstance().SendAlarm(AGENT_MEMORY_WARNING_ALARM, params);
        m_bAgentSpaceAlarmSend = MP_TRUE;
    }
    
    if (agentSpaceUseValue <= MEMORY_ALARM_DEFAULT) {
        WARNLOG("Clear memory warning-alarm %s", AGENT_MEMORY_WARNING_ALARM.c_str());
        AlarmMgr::GetInstance().ResumeAlarm(AGENT_MEMORY_WARNING_ALARM);
        m_bAgentSpaceAlarmSend = MP_FALSE;
    }
}

mp_void AbnormalHandler::CheckNginxCpuUsagePart()
{
    // 发送cpu利用率过高告警
    COMMLOG(OS_LOG_INFO, "Nginx cpu usage overceed %f over %d times.", m_stNginxMointorCfg.fCpuUsageCfg,
        m_stNginxAbnormal.iCpuUsageOverTimes);
    SendCPUAlarm(NGINX_PROCESS);
    m_stNginxAbnormal.iCpuUsageOverTimes = 0;
    m_iNginxAlarmSendFailedTimes = 0;
    COMMLOG(OS_LOG_INFO, "Send alarm successfully.");
    m_bNginxCpuAlarmSend = MP_TRUE;
    m_bNginxCpuAlarmResumed = MP_FALSE;
    m_bNeedResumed = MP_TRUE;

    // 超过指定次数后，不再发送恢复告警
    if (m_iNginxAlarmSendFailedTimes > ALARM_SEND_FAILED_TIME) {
        COMMLOG(OS_LOG_INFO, "Alarm has been sent failed over %d times, no send again.", ALARM_SEND_FAILED_TIME);
        m_stNginxAbnormal.iCpuUsageOverTimes = 0;
        m_bNginxCpuAlarmSend = MP_TRUE;
        m_bNginxCpuAlarmResumed = MP_FALSE;
    }
}
mp_void AbnormalHandler::CheckNginxCpuUsage(mp_float fCpuUsage)
{
    mp_bool bCheckFirst = ((m_stNginxAbnormal.iCpuUsageOverTimes > m_stCommonMonitorCfg.iRetryTime) &&
                           !m_bNginxCpuAlarmSend);
    if (bCheckFirst) {
        CheckNginxCpuUsagePart();
    }

    // agent和nginx进程cpu利用率都低于阈值才发送恢复告警
    if (fCpuUsage <= m_stNginxMointorCfg.fCpuUsageCfg) {
        m_stNginxAbnormal.iCpuUsageOverTimes = 0;
        mp_bool Flag = !m_bNginxCpuAlarmResumed && m_bAgentCpuAlarmResumed && m_bNeedResumed;
        if (Flag) {
            COMMLOG(OS_LOG_INFO, "Nginx process cpu usage overceed alarm is resumed.");
            ResumeCPUAlarm();
            m_iNginxAlarmSendFailedTimes = 0;
            m_bNginxCpuAlarmSend = MP_FALSE;
            m_bAgentCpuAlarmSend = MP_FALSE;
            m_bNginxCpuAlarmResumed = MP_TRUE;
            m_bNeedResumed = MP_FALSE;

            // 超过指定次数后，不再发送恢复告警
            if (m_iNginxAlarmSendFailedTimes > ALARM_SEND_FAILED_TIME) {
                m_bNginxCpuAlarmSend = MP_FALSE;
                m_bNginxCpuAlarmResumed = MP_TRUE;
                m_bNeedResumed = MP_FALSE;
            }
        }
    }
}

#ifdef WIN32
/* ------------------------------------------------------------
Function Name: GetWinMonitorData
Description  : 获取windows监控配置
Others       :-------------------------------------------------------- */
mp_int32 AbnormalHandler::GetWinMonitorData(mp_string strPorcessName, monitor_data_t& stMonitorData)
{
    // 获取进程句柄
    HANDLE hToolHelp = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (NULL == hToolHelp) {
        COMMLOG(OS_LOG_ERROR, "CreateToolhelp32Snapshot failed!");
        CloseHandle(hToolHelp);
        return MP_FAILED;
    }

    PROCESSENTRY32W process;
    process.dwSize = sizeof(process);
    BOOL bMore = ::Process32FirstW(hToolHelp, &process);
    while (bMore) {
        mp_char buf[MAX_BUF_SIZE] = {0};
        ::WideCharToMultiByte(CP_ACP, 0, process.szExeFile, -1, buf, MAX_BUF_SIZE, NULL, NULL);
        if (strcmp(buf, strPorcessName.c_str()) == 0) {
            monitor_data_t stMonitorDataTmp;
            stMonitorDataTmp.bExist = MP_TRUE;
            stMonitorDataTmp.iThreadNum = process.cntThreads;
            // 尝试提权（64位机器必须提权才能对另外一个进程的参数进行操作）
            HANDLE hToken;
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken)) {
                COMMLOG(OS_LOG_WARN, "%s", "Get Access Token failed!");
                CloseHandle(hToken);
                CloseHandle(hToolHelp);
                return MP_FAILED;
            }
            SetWinPrivilege(hToken, SE_DEBUG_NAME, TRUE);

            HANDLE hPro = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, process.th32ProcessID);
            if (NULL == hPro) {
                COMMLOG(OS_LOG_ERROR, "Get handle faild,system error: %d", GetLastError());
                CloseHandle(hPro);
                CloseHandle(hToken);
                CloseHandle(hToolHelp);
                return false;
            }

            // 获取进程句柄
            DWORD dwHandleCount = 0;
            ::GetProcessHandleCount(hPro, &dwHandleCount);
            stMonitorDataTmp.iHandlerNum = dwHandleCount;

            // 获取物理内存和虚拟内存使用量
            PROCESS_MEMORY_COUNTERS pmc;
            ::GetProcessMemoryInfo(hPro, &pmc, sizeof(pmc));
            stMonitorDataTmp.ulVmSize = pmc.PagefileUsage / ABNORMALHANDLER_NUM_1024;   // 单位kbyte
            stMonitorDataTmp.ulPmSize = pmc.WorkingSetSize / ABNORMALHANDLER_NUM_1024;  // 单位kbyte

            // 获取cpu利用率
            stMonitorDataTmp.fCpuUsage = GetWinCPUUsage(hPro);
            CloseHandle(hToken);
            CloseHandle(hPro);
            stMonitorData = AddMonitorData(stMonitorData, stMonitorDataTmp);
        }
        bMore = ::Process32NextW(hToolHelp, &process);
    }
    CloseHandle(hToolHelp);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: SetWinPrivilege
Description  : 设置win权限
Others       :-------------------------------------------------------- */
mp_bool AbnormalHandler::SetWinPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) {
        return MP_TRUE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege) {
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    } else {
        tp.Privileges[0].Attributes = 0;
    }

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
        COMMLOG(OS_LOG_ERROR, "Call AdjustTokenPrivileges failed");
        return MP_FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
        COMMLOG(OS_LOG_ERROR, "GetLastError = %d", ERROR_NOT_ALL_ASSIGNED);
        return MP_FALSE;
    }
    return MP_TRUE;
}

/* ------------------------------------------------------------
Function Name: GetWinCPUUsage
Description  : 获取cpu利用率
Others       :-------------------------------------------------------- */
mp_float AbnormalHandler::GetWinCPUUsage(const HANDLE hAgent)
{
    static mp_int32 processor_count_ = FAULT_NUM;

    static mp_int64 last_time_;
    static mp_int64 last_system_time_;

    FILETIME now;
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    mp_int64 system_time;
    mp_int64 now_time;

    if (processor_count_ == FAULT_NUM) {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        processor_count_ = info.dwNumberOfProcessors;
    }

    GetSystemTimeAsFileTime(&now);

    if (!GetProcessTimes(hAgent, &creation_time, &exit_time, &kernel_time, &user_time)) {
        return FAULT_NUM;
    }

    system_time = (mp_int64)((mp_double)FileTimeTOUTC(kernel_time) / processor_count_ +
                             (mp_double)FileTimeTOUTC(user_time) / processor_count_);
    now_time = FileTimeTOUTC(now);

    last_system_time_ = system_time;
    last_time_ = now_time;

    Sleep(ABNORMALHANDLER_NUM_200);

    if (!GetProcessTimes(hAgent, &creation_time, &exit_time, &kernel_time, &user_time)) {
        return FAULT_NUM;
    }
    GetSystemTimeAsFileTime(&now);
    system_time = (mp_int64)((mp_double)FileTimeTOUTC(kernel_time) / processor_count_ +
                             (mp_double)FileTimeTOUTC(user_time) / processor_count_);
    now_time = FileTimeTOUTC(now);

    mp_double dDifSysTime = (mp_double)(system_time - last_system_time_);
    mp_double dDifNowTime = (mp_double)(now_time - last_time_);
    return (mp_float)((dDifSysTime / dDifNowTime) * ABNORMALHANDLER_NUM_100);
}

/* ------------------------------------------------------------
Function Name: FileTimeTOUTC
Description  : filetime转换成utc时间
Others       :-------------------------------------------------------- */
mp_int64 AbnormalHandler::FileTimeTOUTC(const FILETIME& inputTime)
{
    ULARGE_INTEGER tmpTime;
    tmpTime.LowPart = inputTime.dwLowDateTime;
    tmpTime.HighPart = inputTime.dwHighDateTime;
    return (mp_int64)tmpTime.QuadPart;
}
#endif
