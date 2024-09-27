#include "tools/monitor/AbnormalHandlerTest.h"
#include "common/Utils.h"
#include "securecom/RootCaller.h"
#include "alarm/Trap.h"
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#ifndef WIN32
#include <sys/time.h>
#endif
using namespace std;

namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

static mp_int32 StubGetFolderFile(mp_void* pThis, mp_string& strFolder, vector<mp_string>& vecFileList)
{
    vecFileList.push_back(OM_DPP_EXEC_NAME);
    return MP_SUCCESS;
}

mp_int32 StubSuccess(mp_void* pThis)
{
    return MP_SUCCESS;
}

mp_void StubDoSleepTest100ms(mp_uint32 ms)
{
#ifdef WIN32
    Sleep(100);
#else
    struct timeval stTimeOut;
    const mp_int32 timeUnitChange = 1000;
    stTimeOut.tv_sec  = 100 / timeUnitChange;
    stTimeOut.tv_usec = (100 % timeUnitChange) * timeUnitChange;
    (mp_void)select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}

static mp_int32 StubExecSystemWithEcho(const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect){
    
    mp_string memInfo1 = "1024";
    strEcho.push_back(memInfo1);
    return MP_SUCCESS;
}

mp_int32 StubGetAgentMonitorData(mp_void *ptr, monitor_data_t& stMonitorData)
{
    static mp_int32 icounter_inner = 0;
    if (icounter_inner++ == 0)
    {
        stMonitorData.bExist = MP_FAILED;
        return MP_FAILED;
    }
    else
    {
        stMonitorData.bExist = MP_TRUE;
        stMonitorData.ulPmSize = 1;
        stMonitorData.fCpuUsage = 1;
        return MP_SUCCESS;
    }
}


static mp_int32 StubFileSize(const mp_char* pszFilePath, mp_uint32& uiSize)
{
    uiSize = 1;
    return MP_SUCCESS;
}


static void * timeoutHandle(mp_void *ptr)
{
    int nCnt = 0;
    AbnormalHandler *pAbnormalHandler = (AbnormalHandler *)ptr;
    while ( nCnt++ < 3)
    {
        DoSleep(1000);
    }
    
    pAbnormalHandler->m_bNeedExit = MP_TRUE;
    return (void *)0;
}
 static  mp_string stubGetLogFilePath(mp_void)

{
    return mp_string("./");    
}

static mp_int32 stubReadFile(const mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    static mp_int32 icounter_inner = 0;
    if (icounter_inner++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        vecOutput.push_back("123");
        return MP_SUCCESS;
    }
    
}

static monitor_data_t stubAddMonitorData(mp_void)
{
    return tag_monitor_data();
}

static mp_int32 stubReadMonitorPidFile(const mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    static mp_int32 iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    if (iCounter++ == 1)
    {
        pid_t monitorPid = getpid();
        ostringstream oss;
        oss<<(mp_int32)monitorPid;
        vecOutput.push_back(oss.str().c_str());
    }
    else
    {
        vecOutput.push_back("123");
    }
    return MP_SUCCESS;
}

static mp_int32 stubWriteMonitorPidFile(mp_string& strFilePath, const vector<mp_string>& vecInput)
{
    static mp_int32 iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 StubInitMonitorNginxData(mp_void* pThis, monitor_data_t &monitorNginxData)
{
    monitorNginxData.iHandlerNum = 100;
    monitorNginxData.iThreadNum = 100;
    monitorNginxData.ulPmSize = 102400;
    monitorNginxData.ulVmSize = 10240000;
    monitorNginxData.fCpuUsage = 50;
    monitorNginxData.uiNginxLogSize = 1024000;
    monitorNginxData.ulTmpFileTotalSize = 1024000;
    return MP_SUCCESS;
}

mp_int32 StubGetDataProcessMonitorData(mp_void* pThis, monitor_data_t& monitorNginxData, const mp_string& processName)
{
    monitorNginxData.bExist = MP_TRUE;
    monitorNginxData.iHandlerNum = 100;
    monitorNginxData.iThreadNum = 100;
    monitorNginxData.ulPmSize = 102400;
    monitorNginxData.ulVmSize = 10240000;
    monitorNginxData.fCpuUsage = 50;
    monitorNginxData.uiNginxLogSize = 1024000;
    monitorNginxData.ulTmpFileTotalSize = 1024000;
    return MP_SUCCESS;
}
}

TEST_F(CAbnormalHandlerTest, Handle)
{
    mp_int32 iRet = MP_SUCCESS;
    monitor_data_t stMonitorData;
    AbnormalHandler AbnormalHandler;

    AbnormalHandler.m_stAgentMointorCfg.bMonitored = MP_TRUE;
    AbnormalHandler.m_stNginxMointorCfg.bMonitored = MP_TRUE;

    stub.set(&CMpFile::ReadFile, stubReadMonitorPidFile);
    stub.set(&CIPCFile::WriteFile, stubWriteMonitorPidFile);
    stub.set(&AbnormalHandler::MonitorAgent, stub_return_ret);
    stub.set(&AbnormalHandler::MonitorNginx, stub_return_ret);

    iRet = AbnormalHandler.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.Handle();
    EXPECT_EQ(MP_SUCCESS, iRet);

    iRet = AbnormalHandler.Handle();
    EXPECT_EQ(MP_SUCCESS, iRet);
}



TEST_F(CAbnormalHandlerTest, GetAgentMonitorData)
{
    mp_int32 iRet = MP_SUCCESS;
    monitor_data_t stMonitorData;
    AbnormalHandler AbnormalHandler;

    stub.set(&CPath::GetLogFilePath, stubGetLogFilePath);

    stub.set(&CMpFile::ReadFile, stubReadFile);

    iRet = AbnormalHandler.GetAgentMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);

    //ExecSystemWithEcho;
    iRet = AbnormalHandler.GetAgentMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(&AbnormalHandler::GetTmpFileTotalSize, stub_return_nothing);
    stub.set(&AbnormalHandler::GetMonitorData, stub_return_ret);

    iRet = AbnormalHandler.GetAgentMonitorData(stMonitorData);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.GetAgentMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CAbnormalHandlerTest, GetNginxMonitorData)
{
    mp_int32 iRet = MP_SUCCESS;
    monitor_data_t stMonitorData;
    AbnormalHandler AbnormalHandler;

    stub.set(&CPath::GetLogFilePath, stubGetLogFilePath);
    stub.set(&CMpFile::ReadFile, stubReadFile);

    iRet = AbnormalHandler.GetNginxMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);

    // ExecSystemWithEcho;
    iRet = AbnormalHandler.GetNginxMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(&CPath::GetNginxLogsFilePath, stub_return_string);
    stub.set(&CMpFile::FileSize, stub_return_ret);    
    stub.set(&AbnormalHandler::GetMonitorData, stub_return_ret);
    stub.set(&AbnormalHandler::AddMonitorData, stubAddMonitorData);

    iRet = AbnormalHandler.GetNginxMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);
}


TEST_F(CAbnormalHandlerTest, GetMonitorData)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CSystemExec, ExecSystemWithEcho), StubExecSystemWithEcho);

    mp_string strProcessID;
    monitor_data_t stMonitorData;

    AbnormalHandler AbnormalHandler;
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.GetMonitorData(strProcessID, stMonitorData));
    EXPECT_EQ(1024, stMonitorData.iThreadNum);
    EXPECT_EQ(1024, stMonitorData.iHandlerNum);
    EXPECT_EQ(1024, stMonitorData.ulPmSize);
    EXPECT_EQ(1024, stMonitorData.ulVmSize);
    EXPECT_EQ(1024, stMonitorData.fCpuUsage);
}

TEST_F(CAbnormalHandlerTest, GetHPMonitorData)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubSuccess);
    stub.set(ADDR(CSystemExec, ExecSystemWithEcho), StubExecSystemWithEcho);
    stub.set(ADDR(CMpFile, DelFile), StubSuccess);

    mp_string strProcessID;
    monitor_data_t stMonitorData;
    AbnormalHandler AbnormalHandler;
    
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.GetHPMonitorData(strProcessID, stMonitorData));
    EXPECT_EQ(1, stMonitorData.ulPmSize);
    EXPECT_EQ(1, stMonitorData.ulVmSize);
    EXPECT_EQ(1024, stMonitorData.fCpuUsage);
}

TEST_F(CAbnormalHandlerTest, StartAgent)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubSuccess);

    AbnormalHandler AbnormalHandler;
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.StartAgent());
}

TEST_F(CAbnormalHandlerTest, StartNginx)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubSuccess);

    AbnormalHandler AbnormalHandler;
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.StartNginx());
}

TEST_F(CAbnormalHandlerTest, RestartAgent)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubSuccess);

    AbnormalHandler AbnormalHandler;
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.RestartAgent());
}

TEST_F(CAbnormalHandlerTest, RestartNginx)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubSuccess);

    AbnormalHandler AbnormalHandler;
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.RestartNginx());
}


TEST_F(CAbnormalHandlerTest, SendCPUAlarm)
{
    mp_int32 iRet = MP_SUCCESS;
    AbnormalHandler AbnormalHandler;

    AbnormalHandler.SendCPUAlarm(AGENT_PROCESS);
}

TEST_F(CAbnormalHandlerTest, ResumeCPUAlarm)
{
    mp_int32 iRet = MP_SUCCESS;
    AbnormalHandler AbnormalHandler;

    AbnormalHandler.ResumeCPUAlarm();
}


TEST_F(CAbnormalHandlerTest, MonitorAgent)
{
    mp_int32 iRet = MP_SUCCESS;
    AbnormalHandler AbnormalHandler;

    stub.set(&AbnormalHandler::GetAgentMonitorData, StubGetAgentMonitorData);
    stub.set(&AbnormalHandler::RestartAgent, stub_return_ret);
    
    iRet = AbnormalHandler.MonitorAgent();
    EXPECT_EQ(MP_FAILED, iRet);

    AbnormalHandler.m_stAgentAbnormal.iPmSizeOverTimes = 2;
    AbnormalHandler.m_stCommonMonitorCfg.iRetryTime = 1;
    iRet = AbnormalHandler.MonitorAgent();
    EXPECT_EQ(MP_FAILED, iRet);

    AbnormalHandler.m_stAgentAbnormal.iPmSizeOverTimes = 0;
    AbnormalHandler.m_stCommonMonitorCfg.iRetryTime = 0;
    AbnormalHandler.m_stAgentAbnormal.iCpuUsageOverTimes = 2;
    AbnormalHandler.m_stCommonMonitorCfg.iRetryTime = 1;
    AbnormalHandler.m_iAgentAlarmSendFailedTimes = ALARM_SEND_FAILED_TIME + 1;
    AbnormalHandler.m_bAgentCpuAlarmSend = MP_FALSE;
    AbnormalHandler.m_stAgentAbnormal.iTmpFileSizeOverTimes = ALARM_SEND_FAILED_TIME;
    iRet = AbnormalHandler.MonitorAgent();
    EXPECT_EQ(MP_SUCCESS, iRet);
    
}

TEST_F(CAbnormalHandlerTest, InitMonitorNginxData)
{
    DoGetJsonStringTest();
    stub.set(ADDR(AbnormalHandler, GetNginxMonitorData), StubSuccess);
    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubSuccess);

    AbnormalHandler AbnormalHandler;
    monitor_data_t monitorNginxData;
    monitorNginxData.bExist = MP_FALSE;
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.InitMonitorNginxData(monitorNginxData));
}

TEST_F(CAbnormalHandlerTest, MonitorNginx)
{
    DoGetJsonStringTest();
    stub.set(ADDR(AbnormalHandler, InitMonitorNginxData), StubInitMonitorNginxData);
    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubSuccess);

    AbnormalHandler AbnormalHandler;
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.MonitorNginx());
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.MonitorNginx());
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.MonitorNginx());
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.MonitorNginx());
}


TEST_F(CAbnormalHandlerTest, DeleteTmpFile)
{
    mp_int32 iRet = MP_SUCCESS;
    AbnormalHandler AbnormalHandler;

    stub.set(CMpFile::GetFolderFile, StubGetFolderFile);

    iRet = AbnormalHandler.DeleteTmpFile();
    EXPECT_EQ(MP_FAILED, iRet);

}


TEST_F(CAbnormalHandlerTest, GetTmpFileTotalSize)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_uint64 ulSize = 0;
    mp_string strFolder;
    vector<mp_string> vecFileList;
    AbnormalHandler AbnormalHandler;

    stub.set(CMpFile::GetFolderFile, StubGetFolderFile);
    stub.set(CMpFile::FileSize, StubFileSize);

    iRet = AbnormalHandler.GetTmpFileTotalSize(ulSize);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CAbnormalHandlerTest, GetKSize)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strSize;
    mp_uint64 usize;
    AbnormalHandler AbnormalHandler;

    AbnormalHandler.GetKSize(strSize);
}


TEST_F(CAbnormalHandlerTest, NeedExit)
{
    mp_int32 bRet = MP_TRUE;
    AbnormalHandler AbnormalHandler;

    bRet = AbnormalHandler.NeedExit();
    EXPECT_EQ(MP_FALSE, bRet);
}

TEST_F(CAbnormalHandlerTest, HandleFunc)
{
    DoGetJsonStringTest();
    stub.set(&DoSleep, StubDoSleepTest100ms);
    
    AbnormalHandler AbnormalHandler;
    mp_void* pThis = &AbnormalHandler;
    // start thread;
    pthread_t tid;
    pthread_create(&tid, NULL, timeoutHandle, &AbnormalHandler);
    pthread_detach(tid);
    EXPECT_EQ(NULL, AbnormalHandler.HandleFunc(pThis));
}

TEST_F(CAbnormalHandlerTest, xfunc)
{
    monitor_data_t stMonitorData1;
    monitor_data_t stMonitorData2;
    mp_int32 iThreadStatus;
    abnormal_occur_times_t stAbnormalOccurTimes;
    AbnormalHandler AbnormalHandler;

    AbnormalHandler.GetAgentMonitorCfg();
    AbnormalHandler.GetNginxMonitorCfg();
    AbnormalHandler.GetCommonMonitorCfg();
    AbnormalHandler.GetDataProcessMonitorCfg();

    AbnormalHandler.AddMonitorData(stMonitorData1, stMonitorData2);
    AbnormalHandler.ClearMonitorData(stMonitorData1);
    AbnormalHandler.ClearAbnormalOccurTimes(stAbnormalOccurTimes);
    AbnormalHandler.SetThreadStatus(iThreadStatus);
}

TEST_F(CAbnormalHandlerTest, MonitorDataProcess)
{
    DoGetJsonStringTest();
    stub.set(CMpFile::GetFolderFile, StubGetFolderFile);
    stub.set(ADDR(AbnormalHandler, GetDataProcessMonitorData), StubGetDataProcessMonitorData);
    stub.set(ADDR(CSystemExec, ExecSystemWithEcho), StubExecSystemWithEcho);
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    
    AbnormalHandler AbnormalHandler;
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.MonitorDataProcess());
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.MonitorDataProcess());
    EXPECT_EQ(MP_SUCCESS, AbnormalHandler.MonitorDataProcess());
}