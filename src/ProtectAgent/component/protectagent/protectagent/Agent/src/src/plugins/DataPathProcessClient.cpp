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
#include "plugins/DataPathProcessClient.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "common/CMpThread.h"
#include "common/Path.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "message/tcp/CConnection.h"
#include "message/tcp/CDppMessage.h"
#include "dataprocess/datamessage/DataMessage.h"
#ifdef WIN32
#include "tlhelp32.h"
#include "Psapi.h"
#include <Windows.h>
#endif
#include <sys/types.h>
#include <map>
#include <string>

using namespace std;

namespace {
const mp_string TCP_SERVER_IP = "127.0.0.1";             // Communication IP
const mp_uint32 PERIOD_WATCHDOG = 10000;          // watchdog recheck period, unit millisecond
const mp_uint32 THREAD_SLEEP_TIME = 5000;         // unit millisecond
const mp_uint32 RETRY_TIMES = 3;                  // Retry timeout implementation for Data Path Process
const mp_uint32 SYSTEM_PREALLOCATED_PORT = 1024;  // Max Number of ports reserved for system
const mp_string INVALID_PROCESS_ID = "-1";
const mp_int32 NOCONNECTED_SLEEP = 500;
}

DataPathProcessClient::DataPathProcessClient(mp_int32 serviceType, const mp_string &dpParam)
    : m_iServiceType(serviceType), m_dpParam(dpParam)
{
    m_strCurrentDppPid = "";
    m_strCurrentDppName = "";
    m_bWdExitFlag = MP_FALSE;
    m_bReqExitFlag = MP_FALSE;
    m_bRspExitFlag = MP_FALSE;
    m_iWdRetries = 0;
    seqNum = 0;
    (mp_void) memset_s(&m_wdTid, sizeof(m_wdTid), 0, sizeof(m_wdTid));
    (mp_void) memset_s(&m_reqTid, sizeof(m_reqTid), 0, sizeof(m_reqTid));
    (mp_void) memset_s(&m_rspTid, sizeof(m_rspTid), 0, sizeof(m_rspTid));
    (mp_void) CMpThread::InitLock(&reqMsgMutext);
    (mp_void) CMpThread::InitLock(&rspMsgMutext);
    (mp_void) CMpThread::InitLock(&seqNoMutext);
}

DataPathProcessClient::~DataPathProcessClient()
{
    EndDataPathClient();
    (mp_void) CMpThread::DestroyLock(&reqMsgMutext);
    (mp_void) CMpThread::DestroyLock(&rspMsgMutext);
    (mp_void) CMpThread::DestroyLock(&seqNoMutext);
    CDppMessage *msg = NULL;
    for (std::vector<CDppMessage *>::iterator iter = requestMsgs.begin(); iter != requestMsgs.end(); ++iter) {
        msg = *iter;
        delete msg;
    }
    requestMsgs.clear();

    for (RspMsgIter iter = responseMsgs.begin(); iter != responseMsgs.end(); ++iter) {
        for (std::vector<CDppMessage *>::iterator it = iter->second.begin(); it != iter->second.end(); ++it) {
            msg = *it;
            delete msg;
        }
    }
    responseMsgs.clear();
}

mp_int32 DataPathProcessClient::Init()
{
    if (IsDataProcessStarted() == MP_FALSE) {
        mp_int32 iRet = StartCommonDataPathProcess();
        if (iRet != MP_SUCCESS) {
            ERRLOG("Unable to start data process service, ret: '%d'.", iRet);
            return iRet;
        }
    } else {
        // when rdagent restart
        ERRLOG("data process service is alread started.");
    }

    INFOLOG("Start to enstablish connection with data process service.");
    mp_int32 iRet = EstablishClient();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to create connection with data process service, ret: '%d'.", iRet);
        return iRet;
    }

    return StartLoopThreads();
}

mp_int32 DataPathProcessClient::SetUpClient(const mp_string& strIP, mp_uint16 iPort)
{
    mp_socket clientfd = 0;
    mp_int32 iRet = MP_FAILED;

    CConnection &conn = m_dataMsg.GetConnection();
    iRet = m_dataMsg.Init(clientfd, strIP, iPort);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init data process service communication client failed, ret: '%d'.", iRet);
        return iRet;
    }

    iRet = m_dataMsg.CreateClient(clientfd);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create data process service communication client failed, ret: '%d'.", iRet);
        return iRet;
    }

    conn.SetSocket(clientfd);
    return MP_SUCCESS;
}

mp_int32 DataPathProcessClient::InitializeClient()
{
    INFOLOG("Start to initialize data process communication client.");

    // need to read from Port file
    mp_int32 iPort = GetDataPathPort();
    INFOLOG("The port to communicate with data process service is: '%d'.", iPort);
    if (iPort <= static_cast<mp_int32>(SYSTEM_PREALLOCATED_PORT)) {
        ERRLOG("Invalid communication port '%d'", iPort);
        return MP_FAILED;
    }

    mp_int32 iRet = SetUpClient(TCP_SERVER_IP, iPort);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Setup data process communication client failed, ret: '%d'.", iRet);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_void DataPathProcessClient::SendDPMessage(
    const mp_string &taskId, CDppMessage *reqMsg, CDppMessage *&rspMsg, mp_uint32 timeout)
{
    static const mp_uint32 sleepTime = 1000;
    // 10 min message have not been dispatch, release message
    static const double timeoutMsg = 600;
    if (reqMsg == NULL || rspMsg != NULL) {
        ERRLOG("request message is NULL or response message isn't NULL, taskid=%s.", taskId.c_str());
        return;
    }

    mp_uint32 cmd = reqMsg->GetManageCmd();
    mp_uint64 seqNo = reqMsg->GetOrgSeqNo();
    // push into send queue
    PushMsg2Queue(reqMsg);

    while (timeout != 0) {
        if (!IsDataProcessStarted()) { // dp进程已经重启，不需要继续等待，直接返回
            ERRLOG("dataprocess does not exists.");
            break;
        }

        CMpTime::DoSleep(sleepTime);

        CThreadAutoLock lock(&rspMsgMutext);
        mp_time nowTime;
        CMpTime::Now(nowTime);

        RspMsgIter iter = responseMsgs.find(taskId);
        if (iter == responseMsgs.end()) {
            timeout--;
            continue;
        }

        for (std::vector<CDppMessage *>::iterator it = iter->second.begin(); it != iter->second.end();) {
            // found response message
            if ((*it)->GetOrgSeqNo() == seqNo) {
                rspMsg = *it;
                it = iter->second.erase(it);
                break;
            }

            // message haven't been pop, delete it
            if (CMpTime::Difftime(nowTime, (*it)->GetUpdateTime()) > timeoutMsg) {
                WARNLOG("msg timeout, cmd=0x%x, seq=%llu.", (*it)->GetManageCmd(), (*it)->GetOrgSeqNo());
                delete *it;
                it = iter->second.erase(it);
            } else {
                ++it;
            }
        }

        if (iter->second.size() == 0) {
            responseMsgs.erase(iter);
        }

        // found message
        if (rspMsg != NULL) {
            break;
        }
        timeout--;
    }

    if (rspMsg == NULL) {
        ERRLOG("Receive message from DP service failed, taskid=%s, cmd=0x%x, seq=%llu.", taskId.c_str(), cmd, seqNo);
    }
}

// stop data process service once error occurs
#ifdef WIN32
mp_int32 DataPathProcessClient::EndDataProcessOnTimeout()
{
    INFOLOG("Timeout occurs, will stop current data process service!");

    mp_int32 iRet = MP_FAILED;
    DWORD dwCodeWin = 0;
    // max loop three times
    while (RETRY_TIMES--) {
        if (WAIT_TIMEOUT == WaitForSingleObject(stProcessInfo.hProcess,
            THREAD_SLEEP_TIME * RETRY_TIMES)) { // Need to reverify the TimeoutInterval
            ERRLOG("WaitForSingleObject timeout.");
            iRet = MP_FAILED;
        } else {
            GetExitCodeProcess(stProcessInfo.hProcess, &dwCodeWin);
            iRet = dwCodeWin;
            INFOLOG("GetExitCodeProcess is '%d'.", iRet);
            break;
        }
    }

    CloseHandle(stProcessInfo.hProcess);
    CloseHandle(stProcessInfo.hThread);

    INFOLOG("Stop data process service successfully!");

    return iRet;
}
#else
mp_int32 DataPathProcessClient::EndDataProcessOnTimeout()
{
    INFOLOG("Timeout occurs, will stop current data process service!");

    if (IsDataProcessStarted() == MP_FALSE) {
        INFOLOG("data process service isn't running!");
        return MP_SUCCESS;
    }
    CHECK_FAIL_EX(CheckCmdDelimiter(m_strCurrentDppPid));
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec(ROOT_COMMAND_KILL, " " + m_strCurrentDppPid, NULL);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Kill dataprocess failed, pid= %s, iRet = %d", m_strCurrentDppPid.c_str(), iRet);
        return MP_FAILED;
    } else {
        if (m_pStream != NULL && MP_SUCCESS != pclose(m_pStream)) {
            mp_int32 iErr = GetOSError();
            ERRLOG("Close data process service hanlder failed, ret: '%d'.", iErr);
            return MP_FAILED;
        }
    }

    INFOLOG("Stop data process service successfully!");
    return MP_SUCCESS;
}
#endif

/*------------------------------------------------------------
Description  : EndDataPathClient - This function closes the Data Path Process
               It should first close the Watchdog thread, then send the Close Message
               to the Data Path process, then wait for the Data path process to end
               automatically. Retry this mechanism for retry times and then stop Data
               Path Process. After closing Data Path Proces, this function will close the
               Data Path process communication client.
Input        : None
Output       : Data Path Process should be closed.
Return       : MP_SUCCESS -- SUCCESS
               MP_FAILED -- FAILED
Create By    : Sarath M.S wx907374
Modification :
    windows
    linux -- Same Behavior for Both OS
-------------------------------------------------------------*/
mp_void DataPathProcessClient::EndDataPathClient()
{
    INFOLOG("Start to close data process service client.");
    m_bWdExitFlag = MP_TRUE;
    if (m_wdTid.os_id != 0) {
        CMpThread::WaitForEnd(&m_wdTid, NULL);
    }

    // close watchdog, and close connection
    m_dataMsg.CloseDppConnection();

    m_bReqExitFlag = MP_TRUE;
    if (m_reqTid.os_id != 0) {
        CMpThread::WaitForEnd(&m_reqTid, NULL);
    }
    m_bRspExitFlag = MP_TRUE;
    if (m_rspTid.os_id != 0) {
        CMpThread::WaitForEnd(&m_rspTid, NULL);
    }
}

/*------------------------------------------------------------
Description  : GetDataPathPort - This function gets the port to communicate to data path process.
               This function would read the file in log folder names dataprocess_pid(where pid
               is the processid of the process to communicate) and then delete the file.
Input        : None
Output       : Port ID for data path process Communication
Return       : Any other Value -- SUCCESS
               -1 -- FAILED
Create By    : Sarath M.S wx907374
Modification :
    windows
    linux -- Same Behavior for Both OS
-------------------------------------------------------------*/
mp_int32 DataPathProcessClient::GetDataPathPort()
{
    static const mp_int32 waitFileExists = 500;
    static const mp_int32 retryNum = 3;
    mp_int32 iDataPathPortNum = -1;
    vector<mp_string> vecReadPid;

    std::ostringstream oss;
    oss << OM_DPP_EXEC_NAME << "_" << m_iServiceType << "_" << m_dpParam;
    mp_string strPortIdFilePath = CPath::GetInstance().GetConfFilePath(oss.str());

    mp_int32 num = retryNum;
    while (num > 0) {
        if (CMpFile::FileExist(strPortIdFilePath)) {
            break;
        }
        num--;
        CMpTime::DoSleep(waitFileExists);
    }

    INFOLOG("Read port from file '%s'.", oss.str().c_str());
    mp_int32 iRet = CMpFile::ReadFile(strPortIdFilePath, vecReadPid);
    if ((iRet != MP_SUCCESS) || (vecReadPid.size() == 0)) {
        ERRLOG("Read data process port from file '%s' failed, iRet = '%d'", oss.str().c_str(), iRet);
        return MP_FAILED;
    } else {
        iDataPathPortNum = atoi(vecReadPid.front().c_str());
    }

    return iDataPathPortNum;
}

/*------------------------------------------------------------
Description  : IsDataProcessStarted - This function gets the running status of data path process.
Input        : None
Output       : Whether Data Path Process is started or not
Return       : MP_TRUE -- Started
               MP_FALSE -- Not Started
Create By    : Sarath M.S wx907374
Modification :
    windows -- This function would check process snapshot and return True or False
    linux -- This function would check ps -aef output and return True or False
-------------------------------------------------------------*/
#ifdef WIN32
mp_bool DataPathProcessClient::IsDataProcessStarted()
{
    // Haven't implemented Return Success
    // Will Service names be different for Windows processes
    // Maybe can be better method to get from the process ID, Can recheck
    HANDLE hToolHelp = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (NULL == hToolHelp) {
        ERRLOG("CreateToolhelp32Snapshot exec failed!");
        CloseHandle(hToolHelp);
        return MP_FALSE;
    }
    PROCESSENTRY32W process;
    process.dwSize = sizeof(process);
    BOOL bMore = ::Process32FirstW(hToolHelp, &process);
    while (bMore) {
        mp_char buf[MAX_BUF_SIZE] = {0};
        ::WideCharToMultiByte(CP_ACP, 0, process.szExeFile, -1, buf, MAX_BUF_SIZE, NULL, NULL);
        if (strcmp(buf, m_strCurrentDppName.c_str()) == 0) {
            CloseHandle(hToolHelp);
            return MP_TRUE;
        }
        bMore = ::Process32NextW(hToolHelp, &process);
    }
    CloseHandle(hToolHelp);
    return MP_FALSE;
}
#else
mp_bool DataPathProcessClient::IsDataProcessStarted()
{
    std::vector<mp_string> vecOutput;
    ostringstream oss;
    oss << OM_DPP_EXEC_NAME << "_" << m_iServiceType << "_" << m_dpParam;

    ostringstream queryStr;
    // 非外部入参
    queryStr << "ps -aef | grep '" << OM_DPP_EXEC_NAME << " " << m_iServiceType << " " << m_dpParam
             << "' | grep -v grep | grep -v gdb | grep -v vi | grep -v tail | awk '{print $2}'";
    CHECK_FAIL_EX(CheckCmdDelimiter(m_dpParam));
    if (CSystemExec::ExecSystemWithEcho(queryStr.str(), vecOutput, MP_FALSE) != MP_SUCCESS) {
        ERRLOG("dataprocess ExecSystemWithEcho failed and not delete file.");
        return MP_FALSE;
    }

    if (vecOutput.size() == 0) {
        ERRLOG("dataprocess does not exist and not delete file.");
        return MP_FALSE;
    }

    m_strCurrentDppPid = vecOutput[0];
    DBGLOG("dataprocess pid=%s, serviceType %d, version %s.",
        m_strCurrentDppPid.c_str(),
        m_iServiceType,
        m_dpParam.c_str());
    return MP_TRUE;
}
#endif

// Create connection client with data process service, if failure, close the data process service
mp_int32 DataPathProcessClient::EstablishClient()
{
    if (m_dataMsg.GetConnection().GetLinkState() == LINK_STATE_LINKED) {
        return MP_SUCCESS;
    }

    if (MP_SUCCESS != InitializeClient()) {
        ERRLOG("Init data process communication client failed, and will stop the data process service started!");

        // stop data process service
        if (MP_SUCCESS != EndDataProcessOnTimeout()) {
            ERRLOG("Close data process service failed!");
        } else {
            INFOLOG("Close data process service successfully!");
        }
        return ERROR_AGENT_INTERNAL_ERROR;
    }

    return MP_SUCCESS;
}

// ensure data process service is running
mp_void DataPathProcessClient::DoWatchDogProcess()
{
    INFOLOG("Start watchdog main process.");
    mp_int32 iRet = MP_FAILED;
    while (!m_bWdExitFlag) {
        CMpTime::DoSleep(PERIOD_WATCHDOG);

        if (IsDataProcessStarted() == MP_FALSE) {
            INFOLOG("The data process service is not running, will start it!");

            m_dataMsg.GetConnection().DisConnect();
            iRet = StartCommonDataPathProcess();
            if (iRet != MP_SUCCESS) {
                ERRLOG("Unable to start data process service, ret: '%d'.", iRet);
                continue;
            }
            INFOLOG("The data process service has been started succ, will build connection.");
        }

        if (m_dataMsg.GetConnection().GetLinkState() != LINK_STATE_LINKED) {
            iRet = EstablishClient();
            if (iRet != MP_SUCCESS) {
                ERRLOG("Create connection client failed, will retry, ret: '%d'.", iRet);
                continue;
            }
        }
    }
    INFOLOG("Exit dp watch log thread.");
}

mp_void DataPathProcessClient::SendMsgToDP()
{
    static const mp_uint32 sendInterTime = 100;
    INFOLOG("Start Send dp message process.");
    while (!m_bReqExitFlag) {
        CConnection &conn = m_dataMsg.GetConnection();
        // check connection status, if don't connection, wait for watch dog to connect
        if (conn.GetLinkState() != LINK_STATE_LINKED) {
            INFOLOG("The connection between rdagent and dataprocess isn't linked.");
            CMpTime::DoSleep(NOCONNECTED_SLEEP);
            continue;
        }

        // send message in pool
        {
            CThreadAutoLock lock(&reqMsgMutext);
            while (requestMsgs.size() > 0) {
                std::vector<CDppMessage *>::iterator iter = requestMsgs.begin();
                CDppMessage *msg = *iter;
                requestMsgs.erase(iter);

                msg->SetLinkInfo(conn.GetClientIpAddrStr(), conn.GetClientPort());
                mp_int32 iRet = m_dataMsg.SendDpp(*msg);
                if (iRet != MP_SUCCESS) {
                    ERRLOG("send message failed, cmd:%u, seq=%llu.", msg->GetManageCmd(), msg->GetOrgSeqNo());
                }

                // need delete message
                delete msg;
            }
        }
        CMpTime::DoSleep(sendInterTime);
    }
    INFOLOG("Exit dp send message thread.");
}

mp_void DataPathProcessClient::RecvMsgFromDP()
{
    INFOLOG("Start receive dp message process.");
    m_dataMsg.GetConnection().InitMsgHead();
    while (!m_bRspExitFlag) {
        if (m_dataMsg.GetConnection().GetLinkState() != LINK_STATE_LINKED) {
            CMpTime::DoSleep(NOCONNECTED_SLEEP);
            continue;
        }

        if (m_dataMsg.StartReceiveDpp() != MP_SUCCESS) {
            ERRLOG("Unable to receive message from datapath process service.");
            continue;
        }

        CDppMessage *msg = m_dataMsg.recvdMsg;
        if (msg == NULL) {
            WARNLOG("Recv null message from dataprocess.");
            continue;
        }
        m_dataMsg.recvdMsg = NULL;

        Json::Value msgBody;
        mp_uint32 cmd = msg->GetManageCmd();
        mp_uint64 seq = msg->GetOrgSeqNo();
        if (msg->GetManageBody(msgBody) != MP_SUCCESS) {
            ERRLOG("Unable to push message cmd=0x%x, seq=%llu into received message list.", cmd, seq);
            delete msg;
            continue;
        }

        if (!msgBody.isObject() || !msgBody.isMember(MANAGECMD_KEY_BODY) ||
            !msgBody[MANAGECMD_KEY_BODY].isMember(MANAGECMD_KEY_TASKID) ||
            !msgBody[MANAGECMD_KEY_BODY][MANAGECMD_KEY_TASKID].isString()) {
            ERRLOG("Dp message cmd=0x%x, seq=%llu have no taskid.", cmd, seq);
            delete msg;
            continue;
        }

        // Get taskId
        mp_string taskId = msgBody[MANAGECMD_KEY_BODY][MANAGECMD_KEY_TASKID].asString();
        msg->UpdateTime();
        CThreadAutoLock lock(&rspMsgMutext);
        RspMsgIter iter = responseMsgs.find(taskId);
        if (iter == responseMsgs.end()) {
            std::vector<CDppMessage *> vecMsgs;
            vecMsgs.push_back(msg);
            responseMsgs.insert(std::pair<mp_string, std::vector<CDppMessage *> >(taskId, vecMsgs));
        } else {
            iter->second.push_back(msg);
        }
    }
    WARNLOG("Exit dp recv message thread.");
}

#ifdef WIN32
DWORD WINAPI DataPathProcessClient::WatchDog(mp_void *client)
#else
mp_void *DataPathProcessClient::WatchDog(mp_void *client)
#endif
{
    // Call the Watchdog main process
    DataPathProcessClient *pDataPathProcessClient = static_cast<DataPathProcessClient *>(client);
    pDataPathProcessClient->DoWatchDogProcess();
#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

#ifdef WIN32
DWORD WINAPI DataPathProcessClient::StartSendMsgLoop(mp_void *client)
#else
mp_void *DataPathProcessClient::StartSendMsgLoop(mp_void *client)
#endif
{
    DataPathProcessClient *pDpClient = static_cast<DataPathProcessClient *>(client);
    pDpClient->SendMsgToDP();
#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

#ifdef WIN32
DWORD WINAPI DataPathProcessClient::StartRecvMsgLoop(mp_void *client)
#else
mp_void *DataPathProcessClient::StartRecvMsgLoop(mp_void *client)
#endif
{
    DataPathProcessClient *pDpClient = static_cast<DataPathProcessClient *>(client);
    pDpClient->RecvMsgFromDP();
#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

mp_int32 DataPathProcessClient::StartLoopThreads()
{
    INFOLOG("Begin to start loop threads.");
    mp_int32 iRet = CMpThread::Create(&m_wdTid, WatchDog, this);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init watchdog failed, ret: '%d'.", iRet);
        return iRet;
    }

    iRet = CMpThread::Create(&m_reqTid, StartSendMsgLoop, this);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init send message thread failed, ret: '%d'.", iRet);
        return iRet;
    }

    iRet = CMpThread::Create(&m_rspTid, StartRecvMsgLoop, this);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init recv message thread failed, ret: '%d'.", iRet);
        return iRet;
    }

    INFOLOG("Start loop threads succ.");
    return MP_SUCCESS;
}

// obtain process id from process name
#ifndef WIN32
mp_string DataPathProcessClient::GetProcessIDbyProcessName(const mp_string& strCurrentDataProcessName)
{
    mp_int32 iRet = MP_FAILED;
    mp_string strPid = INVALID_PROCESS_ID;
    INFOLOG("Find data process service id by service name '%s'.", OM_DPP_EXEC_NAME.c_str());
    // cmd: ps -aef | grep -v grep | grep 'strCurrentDataProcessName' | awk '{print $2}'
    std::vector<mp_string> vecResult;
    if (GetProgressInfo(vecResult) != MP_SUCCESS) {
        ERRLOG("GetProgressInfo failure.");
        return strPid;
    }
    vecResult = GrepV(vecResult, "grep");
    vecResult = GrepE(vecResult, strCurrentDataProcessName);
    vecResult = Awk(vecResult, AWK_COL_FIRST_2);
    if (!vecResult.empty()) {
        strPid = vecResult.front();
        vecResult.clear();
    } else {
        ERRLOG("Unable to find data process service id.");
    }

    INFOLOG("The data process service id is: %s", strPid.c_str());
    return strPid;
}
#endif

/*------------------------------------------------------------
Description  : StartOMDataPathProcess - This function starts the OM Data Path Process
Input        : None
Output       : OM Data Path Process should be started
Return       : MP_SUCCESS: SUCCESS
               MP_FAILED: FAILED
Create By    : Sarath M.S wx907374
Modification :
    windows  -- Create Process Implementation and store process information into stProcessInfo
    linux -- Start process Implementation and store the file stream handle into pStream
-------------------------------------------------------------*/
#ifdef WIN32
mp_int32 DataPathProcessClient::StartOMDataPathProcess()
{
    mp_string strNewExecCmd;
    mp_string strPid = INVALID_PROCESS_ID;
    strNewExecCmd = CPath::GetInstance().GetBinPath() + mp_string(PATH_SEPARATOR) + OM_DPP_EXEC_NAME;
    mp_uint32 uiRetCode = 0;
    INFOLOG("Begin to Start OM Data path process");
    mp_bool bStartted = IsDataProcessStarted();  // Check Windows Version
    if (bStartted) {
        INFOLOG("Already started Data Path Process");
        return MP_SUCCESS;
    }
    STARTUPINFO stStartupInfo;
    DWORD dwCode = 0;
    ZeroMemory(&stStartupInfo, sizeof(stStartupInfo));
    stStartupInfo.cb = sizeof(stStartupInfo);
    stStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    stStartupInfo.wShowWindow = SW_HIDE;
    ZeroMemory(&stProcessInfo, sizeof(stProcessInfo));

    if (!CreateProcess(NULL,
        TEXT((mp_char *)strNewExecCmd.c_str()),
        NULL,
        NULL,
        MP_FALSE,
        0,
        NULL,
        NULL,
        &stStartupInfo,
        &stProcessInfo)) {
        uiRetCode = GetLastError();
        ERRLOG("Create Process Failed, Errorcode = %d", uiRetCode);
        return MP_FAILED;
    }
    m_strCurrentDppName = strNewExecCmd;
    strPid = stProcessInfo.dwProcessId;
    INFOLOG("OM Data Path Process is started.");
    return MP_SUCCESS;
}
#else
mp_int32 DataPathProcessClient::StartOMDataPathProcess()
{
    INFOLOG("Begin to Start OM Data path process");
    if (IsDataProcessStarted()) {
        INFOLOG("Already started Data Path Process");
        return MP_SUCCESS;
    }
    mp_char cmdBuff[512];
    memset_s(&cmdBuff, sizeof(cmdBuff), 0, sizeof(cmdBuff));
    mp_string strNewExecCmd = CPath::GetInstance().GetBinPath() + mp_string(PATH_SEPARATOR) + OM_DPP_EXEC_NAME;
    if (sprintf_s(cmdBuff, sizeof(cmdBuff), "%s %d", strNewExecCmd.c_str(), OCEAN_MOBILITY_SERVICE) == -1) {
        ERRLOG("Func 'sprintf_s' exec failure.");
        return MP_FAILED;
    }
    m_pStream = popen(cmdBuff, "r");  // Data process should run with root
    if (NULL == m_pStream) {
        ERRLOG("popen failed.");
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }
    m_strCurrentDppName = cmdBuff;
    mp_string strPid = GetProcessIDbyProcessName(m_strCurrentDppName);
    if (INVALID_PROCESS_ID == strPid) {
        ERRLOG("Find %s Process ID failed.", m_strCurrentDppName.c_str());
        return MP_FAILED;
    }
    m_strCurrentDppPid = std::move(strPid);
    INFOLOG("OM Data Path Process is started.");
    return MP_SUCCESS;
}
#endif

mp_int32 DataPathProcessClient::StartVMwareNativeDataPathProcess()
{
    static const mp_int32 retryMaxNum = 3;
    static const mp_int32 sleepTime = 1000;
#ifdef WIN32
    ERRLOG("Current version of data process service does not support windows platform.");
    return MP_FAILED;
#else
    // run data process service
    ostringstream oss;
    oss << m_iServiceType << ";" << m_dpParam;
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_DATAPROCESS_START, oss.str(), NULL);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to start vmware data process service, ret: '%d'.", iRet);
        return iRet;
    }

    mp_int32 retryNum = 0;
    mp_bool bFlag = MP_FALSE;
    while (retryNum++ < retryMaxNum) {
        bFlag = IsDataProcessStarted();
        if (bFlag != MP_TRUE) {
            ERRLOG("start VMware native data process service failed.");
            CMpTime::DoSleep(sleepTime);
            continue;
        }

        INFOLOG("VMware native data process service is started successfully!");
        return MP_SUCCESS;
    }

    return MP_FAILED;
#endif
}

mp_int32 DataPathProcessClient::StartCommonDataPathProcess()
{
    // start dp one time only one enterpoint, int or watchdog
    mp_int32 iRet = MP_FAILED;
    INFOLOG("Begin to start data process service with type: '%d'.", m_iServiceType);

    switch (m_iServiceType) {
        case OCEAN_MOBILITY_SERVICE:  // not used
            iRet = StartOMDataPathProcess();
            if (iRet != MP_SUCCESS) {
                ERRLOG("Start data process service failed, ret: '%d'.", iRet);
                return iRet;
            }
            break;
        case OCEAN_VMWARENATIVE_SERVICE:
            iRet = StartVMwareNativeDataPathProcess();
            if (iRet != MP_SUCCESS) {
                ERRLOG("Start vmware native data process service failed, ret: '%d'.", iRet);
                return iRet;
            }
            break;
        default:
            iRet = MP_FAILED;
            INFOLOG("Invalid data process service type, please check!");
            return iRet;
    }
    return iRet;
}

mp_int32 DataPathProcessClient::ResetConnection()
{
    m_dataMsg.GetConnection().DisConnect();
    return EstablishClient();
}

mp_void DataPathProcessClient::PushMsg2Queue(CDppMessage *reqMsg)
{
    CThreadAutoLock lock(&reqMsgMutext);
    requestMsgs.push_back(reqMsg);
}

mp_uint64 DataPathProcessClient::GetSeqNo()
{
    CThreadAutoLock lock(&seqNoMutext);
    return seqNum++;
}
