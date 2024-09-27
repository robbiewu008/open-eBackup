#ifndef MONITOR_HANDLER_H
#define MONITOR_HANDLER_H

#include "common/Types.h"
#include "common/CMpThread.h"
#include <sstream>

static const mp_string MONITOR_SECTION = "Monitor";

static const mp_string THRD_CNT = "thread_count";
static const mp_string HANDLER_CNT = "handle_count";
static const mp_string PM_SIZE = "pm_size";
static const mp_string VM_SIZE = "vm_size";
static const mp_string CPU_USAGE = "cpu_usage";
static const mp_string TMPFILE_SIZE = "tmpfile_size";
static const mp_string LOGFILE_SIZE = "logfile_size";
static const mp_string LOGFILE_SIZE_ALARM_THRESHOLD = "logfile_size_alarm_threshold";
static const mp_string RETRY_TIME = "retry_time";
static const mp_string MONITOR_INTERVAL = "monitor_interval";
static const mp_string NGINX_LOG_SIZE = "nginx_log_size";
static const mp_string NGINX_LOG_FILE = "error.log";

#ifdef WIN32
static const mp_string ROATE_NGINX_LOG_SCRIPT = "rotatenginxlog.bat";
#else
static const mp_string ROATE_NGINX_LOG_SCRIPT = "rotatenginxlog.sh";
#endif

// 临时文件存在的最长时间，单位秒 3600 * 24
static const mp_int32 MAX_TMP_EXIST_TIME = 86400;
static const mp_int32 MONITOR_SLEEP_TIME = 1000;
static const mp_uchar ALARM_SEND_FAILED_TIME = 3;
static const mp_uchar MAX_BUF_SIZE = 200;
static const mp_uchar FAULT_NUM = -1;

static const mp_uchar DEFAULT_XML_RETRY_TIME_VALUE = 3;
static const mp_uchar DEFAULT_XML_INTERVAL_VALUE = 30;

static const mp_string HP_TOP_CMD = "top -n 1000 -f";

static const mp_uchar AGENT_PROCESS = 0;
static const mp_uchar NGINX_PROCESS = 1;
static const mp_uchar DATAPROCESS_PROCESS = 2;

static const mp_int32 RETRY_TIMES = 3;
static const mp_int32 MONITOR_INTERVAL_TIME = 30;

typedef struct tag_abnormal_occur_times {
    tag_abnormal_occur_times()
    {
        iPmSizeOverTimes = 0;
        iVmSizeOverTimes = 0;
        iHandlerOverTimes = 0;
        iThreadOverTimes = 0;
        iCpuUsageOverTimes = 0;
        iTmpFileSizeOverTimes = 0;
    }
    mp_int32 iPmSizeOverTimes;    // 物理内存连续超过配置文件次数
    mp_int32 iVmSizeOverTimes;    // 虚拟内存连续超过配置文件次数
    mp_int32 iHandlerOverTimes;   // 文件描述符个数连续超过配置文件次数
    mp_int32 iThreadOverTimes;    // 线程个数连续超过配置文件次数
    mp_int32 iCpuUsageOverTimes;  // CPU利用率连续超过配置文件次数
    mp_int32 iTmpFileSizeOverTimes;
} abnormal_occur_times_t;

typedef struct tag_monitor_data {
    mp_bool bExist = MP_FALSE;
    mp_int32 iHandlerNum = 0;
    mp_int32 iThreadNum = 0;
    mp_uint64 ulPmSize = 0;            // 单位Kbyte
    mp_uint64 ulVmSize = 0;            // 单位Kbyte
    mp_float fCpuUsage = 0.0;            // 单位%
    mp_uint32 uiNginxLogSize = 0;      // 单位Kbyte
    mp_uint64 ulTmpFileTotalSize = 0;  // 单位Kbyte
    mp_uint64 ulLogFileTotalSize = 0;  // 单位Kbyte
} monitor_data_t;

typedef struct tag_monitor_process_config {
    mp_bool bMonitored = MP_FALSE;
    mp_int32 iPmSizeCfg = 0;
    mp_int32 iVmSizeCfg = 0;
    mp_int32 iHandlerNumCfg = 0;
    mp_int32 iThreadNumCfg = 0;
    mp_float fCpuUsageCfg = 0.0;
    mp_int32 iTmpFileTotalSizeCfg = 0;  // 单位M,agent进程特有参数
    mp_float fLogAlarmPercentage = 0;   // 单位0.1, 日志告警阈值
    mp_int32 iLogFileTotalSizeCfg = 0;  // 单位M,agent进程特有参数
    mp_int32 iNginxLogSizeCfg = 0;      // 单位K,nginx进程特有参数
} monitor_process_config_t;

typedef struct tag_monitor_common_config {
    tag_monitor_common_config()
    {
        iRetryTime = RETRY_TIMES;
        iMonitorInterval = MONITOR_INTERVAL_TIME;
    }
    mp_int32 iRetryTime;
    mp_int32 iMonitorInterval;
} monitor_common_config_t;

class AbnormalHandler {
public:
    static AbnormalHandler& GetInstance()
    {
        return m_instance;
    }

    ~AbnormalHandler()
    {
        m_bNeedExit = MP_TRUE;
        if (m_hHandleThread.os_id != 0) {
            CMpThread::WaitForEnd(&m_hHandleThread, NULL);
        }
    }

    mp_int32 Init();
    mp_int32 Handle();

private:
    AbnormalHandler()
    {
        m_bAgentCpuAlarmSend = MP_FALSE;
        m_bAgentCpuAlarmResumed = MP_FALSE;
        m_bNginxCpuAlarmSend = MP_FALSE;
        m_bNginxCpuAlarmResumed = MP_FALSE;
        m_bNeedResumed = MP_TRUE;
        m_bAgentSpaceAlarmSend = MP_FALSE;
        m_iAgentAlarmSendFailedTimes = 0;
        m_iNginxAlarmSendFailedTimes = 0;
        (mp_void) memset_s(&m_hHandleThread, sizeof(m_hHandleThread), 0, sizeof(m_hHandleThread));
        // Coverity&Fortify误报:UNINIT_CTOR
        // Coveirty&Fortify不认识公司安全函数memset_s，提示m_dispatchTid.os_id未初始化
        m_iThreadStatus = THREAD_STATUS_IDLE;
        m_bNeedExit = MP_FALSE;
    }
    mp_int32 MonitorAllProcess();
    mp_int32 GetAgentMonitorData(monitor_data_t& stMonitorData);
    mp_int32 GetNginxMonitorData(monitor_data_t& stMonitorData);
    mp_int32 GetDataProcessMonitorData(monitor_data_t& stMonitorData, const mp_string& processName);
    mp_int32 GetMonitorData(const mp_string& strProcessID, monitor_data_t& stMonitorData);
    mp_int32 GetHPMonitorData(mp_string strProcessID, monitor_data_t& stMonitorData);
    mp_int32 StopAgent();
    mp_void StopPlugins();
    mp_int32 StartAgent();
    mp_int32 StartNginx();
    mp_int32 RestartAgent();
    mp_int32 RestartNginx();
    mp_int32 KillDataProcess(const mp_string &processName);
    mp_void SendCPUAlarm(mp_int32 iProcessType);
    mp_void ResumeCPUAlarm();
    monitor_process_config_t& GetAgentMonitorCfg();
    monitor_process_config_t& GetDataProcessMonitorCfg();
    monitor_process_config_t& GetNginxMonitorCfg();
    monitor_common_config_t& GetCommonMonitorCfg();
    mp_int32 MonitorAgent();
    mp_int32 InitMonitorAgentData(monitor_data_t &monitorAgentData);
    mp_int32 MonitorNginx();
    mp_int32 InitMonitorNginxData(monitor_data_t &monitorNginxData);
    mp_int32 MonitorDataProcess();
    mp_int32 MonitorDataProcessOnce(const mp_string& processName);
    static monitor_data_t AddMonitorData(monitor_data_t& stMonitorData1, monitor_data_t& stMonitorData2);
    static mp_void ClearMonitorData(monitor_data_t& stMonitorData);
    static mp_void ClearAbnormalOccurTimes(abnormal_occur_times_t& stAbnormalOccurTimes);
    static mp_int32 DeleteTmpFile();
    static mp_int32 GetTmpFileTotalSize(mp_uint64& ulSize);
    static mp_int32 GetLogFileTotalSize(mp_uint64& ulSize);
    static mp_uint64 GetKSize(const mp_string& strSize);
    mp_bool NeedExit();
    mp_void SetThreadStatus(mp_int32 iThreadStatus);
    mp_void RotateNginxLog();
    mp_void CheckNginxMonitorValue(monitor_data_t& monitorNginxData);
    mp_void CheckAgentMonitorValue(monitor_data_t& monitorAgentData);
    mp_void CheckDataProcessMonitorValue(monitor_data_t& monitorAgentData);
    mp_void CheckAgentCpuUsage(mp_float fCpuUsage);
    mp_void CheckNginxCpuUsage(mp_float fCpuUsage);
    mp_void CheckNginxCpuUsagePart();
    mp_void GetAgentSpaceUsage(mp_string& agentSpaceUse, const mp_string& strAgentRootPath);
    mp_void CheckAgentFreeSpace();
    mp_string GetNginxPid();
    mp_int32 MonitorAgentData(monitor_data_t &monitorAgentData, const mp_uint64& logSizeAgentAlarmThreshold,
        const mp_uint64& logSizeAgentStopThreshold, const mp_float& alarmPer);

#ifdef WIN32
    static DWORD WINAPI HandleFunc(mp_void* pThis);
    mp_int32 GetWinMonitorData(mp_string strPorcessName, monitor_data_t& stMonitorData);
    mp_float GetWinCPUUsage(const HANDLE hAgent);
    static mp_bool SetWinPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
    static mp_int64 FileTimeTOUTC(const FILETIME& inputTime);
#else
    EXTER_ATTACK static mp_void* HandleFunc(mp_void* pThis);
#endif

private:
    static AbnormalHandler m_instance;            // 单例对象
    thread_id_t m_hHandleThread;                   // 线程句柄
    abnormal_occur_times_t m_stAgentAbnormal;      // Agent监控进程异常处理数据
    abnormal_occur_times_t m_stNginxAbnormal;      // Nginx监控进程异常处理数据
    abnormal_occur_times_t m_stDataProcessAbnormal; // DataProcess监控进程异常处理数据
    monitor_process_config_t m_stAgentMointorCfg;  // Agent进程监控配置数据
    monitor_process_config_t m_stNginxMointorCfg;  // Nginx进程监控配置数据
    monitor_common_config_t m_stCommonMonitorCfg;  // 进程监控公共配置
    mp_bool m_bAgentCpuAlarmSend;                  // 是否已经发送agent cpu利用率高告警标识
    mp_bool m_bAgentCpuAlarmResumed;               // 是否已经发送agent cpu利用率高恢复告警标识
    mp_bool m_bNginxCpuAlarmSend;                  // 是否已经发送nginx cpu利用率高告警标识
    mp_bool m_bNginxCpuAlarmResumed;               // 是否已经发送nginx cpu利用率高恢复告警标识
    mp_bool m_bNeedResumed;                        // 是否需要发送恢复告警
    mp_int64 m_iAgentAlarmSendFailedTimes;         // Agent cpu告警发送失败次数
    mp_int64 m_iNginxAlarmSendFailedTimes;         // Nginx cpu告警发送失败次数
    mp_bool m_bAgentSpaceAlarmSend = MP_FALSE;     // 是否已经发送agent 空间使用率高告警标识
    volatile mp_int32 m_iThreadStatus;
    volatile mp_bool m_bNeedExit;
    monitor_process_config_t m_stDataProcessMointorCfg;  // dataprocess进程监控配置数据

    static const mp_int32 ABNORMALHANDLER_NUM_1000 = 1000;
    static const mp_int32 ABNORMALHANDLER_NUM_1024 = 1024;
    static const mp_int32 ABNORMALHANDLER_NUM_2000 = 2000;
    static const mp_int32 ABNORMALHANDLER_NUM_200  = 200;
    static const mp_uchar ABNORMALHANDLER_NUM_100  = 100;
    static const mp_uchar ABNORMALHANDLER_NUM_10   = 10;
};

#define CHECK_VALUE(iValue, iConfigValue, iOverTimes)                                                       \
    do {                                                                                                    \
        (iValue > iConfigValue) ? iOverTimes++ : (iOverTimes = 0);                                          \
        if (iValue > iConfigValue) {                                                                        \
            ostringstream oss;                                                                              \
            oss << #iValue << " overceed config value(" << iConfigValue << ") " << iOverTimes << " times."; \
            COMMLOG(OS_LOG_INFO, "%s", oss.str().c_str());                                 \
        }                                                                                  \
    } while (0)

#endif
