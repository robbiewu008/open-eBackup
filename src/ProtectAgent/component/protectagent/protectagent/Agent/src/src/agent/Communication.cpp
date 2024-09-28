#ifndef WIN32
#include <unistd.h>
#include <fcntl.h>
#endif
#include "agent/Authentication.h"
#include "agent/FTExceptionHandle.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/ErrorCode.h"
#include "message/rest/interfaces.h"
#include "message/tcp/CSocket.h"
#include "fcgi/include/fcgios.h"
#include "agent/Communication.h"

using namespace std;

namespace {
    const std::string DEFAULT_LISTEN_IP = "127.0.0.1";
    const mp_uint16 DEFAULT_FCGI_START_PORT = 59540;
    const mp_uint16 DEFAULT_FCGI_END_PORT = 59559;
}
/* ------------------------------------------------------------
Function Name: ~Communication
Description  : 析构函数，对资源进行释放
Others       :-------------------------------------------------------- */
Communication::~Communication()
{
    try {
        DeleteRequest();
        // 线程回收
        m_bNeedExit = MP_TRUE;
        if (m_hReceiveThread.os_id != 0) {
            CMpThread::WaitForEnd(&m_hReceiveThread, NULL);
        }
        if (m_hSendThread.os_id != 0) {
            CMpThread::WaitForEnd(&m_hSendThread, NULL);
        }
        // 释放互斥锁
        (mp_void) CMpThread::DestroyLock(&m_reqTableMutex);
        (mp_void) CMpThread::DestroyLock(&m_reqMsgQueueMutex);
        (mp_void) CMpThread::DestroyLock(&m_rspMsgQueueMutex);
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "Communication destructor have exception.");
    }
}

/* ------------------------------------------------------------
Function Name: init
Description  : 初始化函数，开启端口，创建线程
Others       :-------------------------------------------------------- */
EXTER_ATTACK mp_int32 Communication::Init()
{
    static mp_int32 fcgiPort = 100;
    LOGGUARD("");
    // 初始化FCGI环境信息
    mp_int32 iRet = FCGX_Init();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "FCGX_Init error! iRet=%d.", iRet);
        return ERROR_COMMON_OPER_FAILED;
    }

    // 开启FCGI监听端口
    mp_string strPort;
    mp_string strAddress;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_PORT, strPort);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get port number from xml config failed.");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    Communication::CheckIPandPort(strPort);
#ifdef WIN32
    strAddress = "localhost:" + strPort;
#else
    strAddress = "127.0.0.1:" + strPort;
#endif
    mp_int32 handler = 0;
    try {
        handler = FCGX_OpenSocket(strAddress.c_str(), fcgiPort);
        if (handler < 0) {
            COMMLOG(OS_LOG_ERROR, "FCGX_OpenSocket failed! handler = %d\n.", handler);
            return ERROR_COMMON_OPER_FAILED;
        }
    } catch (std::exception& ex) {
        ERRLOG("FCGI bind port failed exception %s", ex.what());
        return ERROR_COMMON_OPER_FAILED;
    }
    CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, CFG_PORT, strPort);
    // 设置文件描述符的FD_CLOEXEC标志，保证fork出来的子进程不继承父进程的资源
#ifndef WIN32
    mp_int32 iFlag = fcntl(handler, F_GETFD);
    if (iFlag == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "fcntl failed! handler = %d\n.", handler);
        (mp_void) OS_Close(handler, MP_TRUE);
        return ERROR_COMMON_OPER_FAILED;
    }
    iFlag = iFlag | FD_CLOEXEC;
    iRet = fcntl(handler, F_SETFD, iFlag);
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "fcntl failed! handler = %d\n.", handler);
        (mp_void) OS_Close(handler, MP_TRUE);
        return ERROR_COMMON_OPER_FAILED;
    }
#endif

    return InitThread(handler);
}

mp_int32 Communication::InitThread(mp_int32 handler)
{
    mp_int32 iRet = InitRequest(handler);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init request failed! iRet = %d\n.", iRet);
        (mp_void) OS_Close(handler, MP_TRUE);
        return iRet;
    }
    // CodeDex误报，KLOCWORK.RH.LEAK
    // CodeDex误报，Unreleased Resource
    iRet = CMpThread::Create(&m_hReceiveThread, ReceiveThreadFunc, this);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create receive thread failed! iRet = %d\n.", iRet);
        (mp_void) OS_Close(handler, MP_TRUE);
        return iRet;
    }

    iRet = CMpThread::Create(&m_hSendThread, SendThreadFunc, this);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create send thread failed! iRet = %d\n.", iRet);
        (mp_void) OS_Close(handler, MP_TRUE);
        return iRet;
    }

    // 初始化鉴权模块
    iRet = security::Authentication::GetInstance().Init();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init authentication failed! iRet = %d\n.", iRet);
        (mp_void) OS_Close(handler, MP_TRUE);
        return iRet;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: InitRequest
Description  : 预分配FCGX_Request对象
Others       :-------------------------------------------------------- */
mp_int32 Communication::InitRequest(mp_int32 handler)
{
    static mp_uint32 MAX_REQUEST_HANDLER = 100;
    LOGGUARD("");
    CThreadAutoLock lock(&m_reqTableMutex);
    for (mp_uint32 i = 0; i < MAX_REQUEST_HANDLER; i++) {
        request_handler_info_t rh;
        rh.pFcgxReq = NULL;
        rh.isUsed = false;
        m_ReqHandlerTable.push_back(rh);
    }

    mp_int32 iRet = MP_SUCCESS;
    // 提前分配100个req对象，解决fastcgi只能分配128个request请求问题
    for (vector<request_handler_info_t>::iterator it = m_ReqHandlerTable.begin(); it != m_ReqHandlerTable.end();
        ++it) {
        FCGX_Request* pfcgxReq = NULL;
        // CodeDex误报，Memory Leak
        NEW_CATCH(pfcgxReq, FCGX_Request);
        if (NULL == pfcgxReq) {
            // 记录日志
            COMMLOG(OS_LOG_ERROR, "new FCGX_Request failed!");
            iRet = ERROR_COMMON_OPER_FAILED;
            break;
        }
        (mp_void) FCGX_InitRequest(pfcgxReq, handler, 0);
        it->pFcgxReq = pfcgxReq;
    }

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Allocate request handler failed!");
        DeleteRequest();
    }

    return iRet;
}

/* ------------------------------------------------------------
Function Name: GetFcgxReq
Description  : 获取空闲的FCGX_Request对象
Others       :-------------------------------------------------------- */
FCGX_Request* Communication::GetFcgxReq()
{
    LOGGUARD("");
    CThreadAutoLock lock(&m_reqTableMutex);
    for (vector<request_handler_info_t>::iterator it = m_ReqHandlerTable.begin(); it != m_ReqHandlerTable.end();
        ++it) {
        if (!it->isUsed) {
            it->isUsed = MP_TRUE;
            return it->pFcgxReq;
        }
    }

    COMMLOG(OS_LOG_ERROR, "All Request handlers have been used");
    return NULL;
}

/* ------------------------------------------------------------
Function Name: DeleteRequest
Description  : 删除所有FCGX_Request对象，析构时调用
Others       :-------------------------------------------------------- */
mp_void Communication::DeleteRequest()
{
    CThreadAutoLock lock(&m_reqTableMutex);
    for (vector<request_handler_info_t>::iterator it = m_ReqHandlerTable.begin(); it != m_ReqHandlerTable.end();
        ++it) {
        if (it->pFcgxReq != NULL) {
            delete it->pFcgxReq;
            it->pFcgxReq = NULL;
        }
    }
}

mp_bool Communication::NeedExit() const
{
    return m_bNeedExit;
}

mp_void Communication::SetRecvThreadStatus(mp_int32 iThreadStatus)
{
    m_iRecvThreadStatus = iThreadStatus;
}

mp_void Communication::SetSendThreadStatus(mp_int32 iThreadStatus)
{
    m_iSendThreadStatus = iThreadStatus;
}

/* ------------------------------------------------------------
Function Name: ReleaseRequest
Description  : 释放FCGX_Request对象
Others       :-------------------------------------------------------- */
mp_void Communication::ReleaseRequest(const FCGX_Request& pReq)
{
    LOGGUARD("");
    CThreadAutoLock lock(&m_reqTableMutex);
    for (vector<request_handler_info_t>::iterator it = m_ReqHandlerTable.begin(); it != m_ReqHandlerTable.end();
        ++it) {
        if (it->pFcgxReq == &pReq) {
            it->isUsed = false;
            break;
        }
    }
}

/* ---------------------------------------------------------------------
Function Name: SendUnAuthedMsg
Description  : 发送失败消息
Others       :-------------------------------------------------------- */
mp_void Communication::SendFailedMsg(
    Communication& pInstance, FCGX_Request& pFcgiReq, mp_int32 iHttpStatus, mp_int32 iRetCode)
{
    CResponseMsg rsp(pFcgiReq);
    rsp.SetRetCode((mp_int64)iRetCode);
    rsp.SetHttpStatus(iHttpStatus);  // 给server端返回具体状态
    (mp_void) rsp.Send();
    pInstance.ReleaseRequest(pFcgiReq);
}

/* ---------------------------------------------------------------------
Function Name: HandleReceiveMsg
Description  : 处理通过fastcgi接受到的消息
Others       :-------------------------------------------------------- */
void Communication::HandleReceiveMsg(Communication& pInstance, FCGX_Request& pFcgiReq)
{
    CRequestMsg* pReq;
    CResponseMsg* pRsp;
    try {
        // CodeDex误报，Memory Leak
        pReq = new CRequestMsg(pFcgiReq);
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "New CRequestMsg failed");
        pReq = NULL;
    }
    if (!pReq) {
        COMMLOG(OS_LOG_ERROR, "New CRequestMsg failed.");
        SendFailedMsg(pInstance, pFcgiReq, SC_NOT_ACCEPTABLE, MP_FAILED);
        return;
    }

    try {
        pRsp = new CResponseMsg(pFcgiReq);
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "New CResponseMsg failed");
        pRsp = NULL;
    }
    if (!pRsp) {
        delete pReq;
        COMMLOG(OS_LOG_ERROR, "New CResponseMsg failed.");
        SendFailedMsg(pInstance, pFcgiReq, SC_NOT_ACCEPTABLE, MP_FAILED);
        return;
    }

    // 解析消息中的内容
    mp_int32 iRet = pReq->Parse();
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "Parse request failed! iRet = %d.", iRet);
        SendFailedMsg(pInstance, pFcgiReq, SC_NOT_ACCEPTABLE, iRet);
        delete pReq;
        delete pRsp;
        return;
    }

    // 提前对冻结请求进行处理，防止冻结过程中agent异常退出导致解冻操作没有记录
    FTExceptionHandle::GetInstance().MonitorFreezeOper(*pReq);

    message_pair_t stReqMsg(*pReq, *pRsp);
    // 将消息加入队列中
    pInstance.PushReqMsgQueue(stReqMsg);
}

/* ---------------------------------------------------------------------
Function Name: ReceiveThreadFunc
Description  : 接收线程处理函数
Others       :-------------------------------------------------------- */
#ifdef WIN32
DWORD WINAPI Communication::ReceiveThreadFunc(mp_void* pThis)
#else
mp_void* Communication::ReceiveThreadFunc(mp_void* pThis)
#endif
{
    LOGGUARD("");
    Communication* pInstance = static_cast<Communication*>(pThis);
    static mp_int32 sleepTime = 100;

    pInstance->SetRecvThreadStatus(THREAD_STATUS_RUNNING);
    while (!pInstance->NeedExit()) {
        FCGX_Request* pFcgiReq = pInstance->GetFcgxReq();
        if (pFcgiReq == NULL) {
            // 记录日志
            COMMLOG(OS_LOG_INFO, "Call GetFcgxReq failed!");
            // 休眠2秒钟;
            DoSleep(RCV_SLEEP_TIME * sleepTime);
            continue;
        }

        COMMLOG(OS_LOG_DEBUG, "Begin accept fcgx");
        mp_int32 iRet = FCGX_Accept_r(pFcgiReq);
        if (iRet < 0) {
            // 记录日志
            pInstance->ReleaseRequest(*pFcgiReq);
            COMMLOG(OS_LOG_ERROR, "FCGX accept failed! iRet = %d.", iRet);
            DoSleep(RCV_SLEEP_TIME);
            continue;
        }
        COMMLOG(OS_LOG_DEBUG, "End accept fcgx");
        CRequestMsg req(*pFcgiReq);
        if (CheckHeader(*pInstance, *pFcgiReq, req) != MP_SUCCESS) {
            continue;
        }

        if (CheckAuth(*pInstance, *pFcgiReq, req) != MP_SUCCESS) {
            continue;
        }

        HandleReceiveMsg(*pInstance, *pFcgiReq);
    }

    pInstance->SetRecvThreadStatus(THREAD_STATUS_EXITED);
    CMPTHREAD_RETURN;
}

mp_int32 Communication::CheckHeader(Communication& pInstance, FCGX_Request& pFcgiReq, CRequestMsg& req)
{
    mp_char** allHead = req.GetHttpReq().GetAllHead();
    if (!allHead) {
        COMMLOG(OS_LOG_ERROR, "Http head is null!");
        SendFailedMsg(pInstance, pFcgiReq, SC_UNAUTHORIZED, MP_FAILED);
        return MP_FAILED;
    }
    for (mp_uint32 i = 0; allHead[i] != 0; i++) {
        // 不打印敏感信息
        mp_bool bRet = (!allHead[i] || NULL != strstr(allHead[i], PW.c_str()) ||
                        NULL != strstr(allHead[i], HTTPPARAM_DBPASSWORD.c_str()) ||
                        NULL != strstr(allHead[i], HTTPPARAM_ASMPASSWORD.c_str()) ||
                        NULL != strstr(allHead[i], HTTPPARAM_SNMPAUTHPW.c_str()) ||
                        NULL != strstr(allHead[i], HTTPPARAM_SNMPENCRYPW.c_str()));
        if (bRet) {
            continue;
        }
        COMMLOG(OS_LOG_DEBUG, "Http Head Recieved: \"%s\"", allHead[i]);
    }

    return MP_SUCCESS;
}

mp_int32 Communication::CheckAuth(Communication& pInstance, FCGX_Request& pFcgiReq, CRequestMsg& req)
{
    mp_string strUser = req.GetHttpReq().GetHead(UNAME);
    mp_string strPW = req.GetHttpReq().GetHead(PW);
    mp_string strClientIP = req.GetHttpReq().GetRemoteIP();
    mp_string strClientCertDN = req.GetHttpReq().GetClientCertDN();
    mp_int32 iRet = security::Authentication::GetInstance().Auth(strClientIP, strUser, strPW, strClientCertDN);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Authenticate failed! IP = \"%s\", User name = \"%s\".",
            strClientIP.c_str(),
            strUser.c_str());
        SendFailedMsg(pInstance, pFcgiReq, SC_UNAUTHORIZED, iRet);
        ClearString(strPW);
        return MP_FAILED;
    } else {
        COMMLOG(OS_LOG_DEBUG,
            "User \"%s\" from \"%s\" has been authenticated successfully!",
            strUser.c_str(),
            strClientIP.c_str());
    }

    ClearString(strPW);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: SendThreadFunc
Description  : 发送线程处理函数
Others       :-------------------------------------------------------- */
#ifdef WIN32
DWORD WINAPI Communication::SendThreadFunc(mp_void* pThis)
#else
mp_void* Communication::SendThreadFunc(mp_void* pThis)
#endif
{
    // CodeDex误报，UNUSED_VALUE
    LOGGUARD("");
    Communication* pInstance = static_cast<Communication*>(pThis);

    pInstance->SetSendThreadStatus(THREAD_STATUS_RUNNING);
    while (!pInstance->NeedExit()) {
        message_pair_t rspMsg;
        mp_int32 iRet = Communication::GetInstance().PopRspMsgQueue(rspMsg);
        if (iRet != 0) {
            DoSleep(RCV_SLEEP_TIME);
            continue;
        }

        mp_bool bCheck = rspMsg.pReqMsg == NULL || rspMsg.pRspMsg == NULL;
        if (bCheck) {
            COMMLOG(
                OS_LOG_INFO, "NULL pointer, rspMsg.pReqMsg = %d, rspMsg.pRspMsg = %d", rspMsg.pReqMsg, rspMsg.pRspMsg);
            continue;
        }

        CRequestMsg* requestMsg = static_cast<CRequestMsg*>(rspMsg.pReqMsg);
        CResponseMsg* responseMsg = static_cast<CResponseMsg*>(rspMsg.pRspMsg);
        bCheck = (requestMsg == NULL) || (responseMsg == NULL);
        if (bCheck) {
            COMMLOG(OS_LOG_ERROR, "rspMsg.pReqMsg is not CRequestMsg or rspMsg.pRspMsg is not CResponseMsg.");
            continue;
        }
        // 对冻结解冻请求进行过滤
        FTExceptionHandle::GetInstance().UpdateFreezeOper(*requestMsg, *responseMsg);

        // 发送返回消息
        (mp_void) responseMsg->Send();

        // 释放request
        Communication::GetInstance().ReleaseRequest(*requestMsg->GetHttpReq().GetFcgxReq());

        // 释放内存
        if (rspMsg.pReqMsg) {
            delete rspMsg.pReqMsg;
            rspMsg.pReqMsg = NULL;
        }
        if (rspMsg.pRspMsg) {
            delete rspMsg.pRspMsg;
            rspMsg.pRspMsg = NULL;
        }
    }

    pInstance->SetSendThreadStatus(THREAD_STATUS_EXITED);
#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

/* ------------------------------------------------------------
Function Name: PopReqMsgQueue
Description  : 从请求队列中获取排在最前面的消息请求
Others       :-------------------------------------------------------- */
mp_int32 Communication::PopReqMsgQueue(message_pair_t& msgPair)
{
    // 获取互斥锁
    CThreadAutoLock lock(&m_reqMsgQueueMutex);
    if (!m_reqMsgQueue.empty()) {
        LOGGUARD("");
        vector<message_pair_t>::iterator it = m_reqMsgQueue.begin();
        msgPair = *it;
        (mp_void) m_reqMsgQueue.erase(it);
        COMMLOG(OS_LOG_DEBUG,
            "pop message from request queue success, req=0x%x, rsp=0x%x!",
            msgPair.pReqMsg,
            msgPair.pRspMsg);
        return MP_SUCCESS;
    } else {
        return MP_FAILED;
    }
}

/* ------------------------------------------------------------
Function Name: GetFrontReqMsgQueue
Description  : 从请求队列中获取排在最前面的消息请求
Others       :-------------------------------------------------------- */
mp_int32 Communication::GetFrontReqMsgQueue(message_pair_t& msgPair)
{
    // 获取互斥锁
    CThreadAutoLock lock(&m_reqMsgQueueMutex);
    if (!m_reqMsgQueue.empty()) {
        LOGGUARD("");
        vector<message_pair_t>::iterator it = m_reqMsgQueue.begin();
        msgPair = *it;
        COMMLOG(OS_LOG_DEBUG,
            "get message from request queue success, req=0x%x, rsp=0x%x!",
            msgPair.pReqMsg,
            msgPair.pRspMsg);
        return MP_SUCCESS;
    } else {
        return MP_FAILED;
    }
}

/* ------------------------------------------------------------
Function Name: PushReqMsgQueue
Description  : 将消息加入到请求消息队列
Others       :-------------------------------------------------------- */
mp_void Communication::PushReqMsgQueue(const message_pair_t& msgPair)
{
    LOGGUARD("");
    // 获取互斥锁
    CThreadAutoLock lock(&m_reqMsgQueueMutex);
    m_reqMsgQueue.push_back(msgPair);
    COMMLOG(
        OS_LOG_DEBUG, "push message to request queue success, req=0x%x, rsp=0x%x!", msgPair.pReqMsg, msgPair.pRspMsg);
}

/* ------------------------------------------------------------
Function Name: PopRspMsgQueue
Description  : 从消息响应队列中取出排在最前面的响应消息
Others       :-------------------------------------------------------- */
mp_int32 Communication::PopRspMsgQueue(message_pair_t& msgPair)
{
    // 获取互斥锁
    CThreadAutoLock lock(&m_rspMsgQueueMutex);
    if (!m_rspMsgQueue.empty()) {
        vector<message_pair_t>::iterator it = m_rspMsgQueue.begin();

        CResponseMsg* responseMsg = static_cast<CResponseMsg*>(it->pRspMsg);
        if (responseMsg == NULL) {
            (mp_void) m_rspMsgQueue.erase(it);
            COMMLOG(OS_LOG_ERROR, "responseMsg is not CResponseMsg.");
            return MP_FAILED;
        }

        while (it != m_rspMsgQueue.end() && responseMsg->IsInternalMsg()) {
            ++it;
        }

        if (it == m_rspMsgQueue.end()) {
            return MP_FAILED;
        }
        msgPair = *it;
        (mp_void) m_rspMsgQueue.erase(it);
        COMMLOG(OS_LOG_DEBUG,
            "pop message from response queue success, req=0x%x, rsp=0x%x!",
            msgPair.pReqMsg,
            msgPair.pRspMsg);
        return MP_SUCCESS;
    } else {
        return MP_FAILED;
    }
}

/* ------------------------------------------------------------
Function Name: PopRspInternalMsgQueue
Description  : 从消息响应队列中取出排在最前面的内部响应消息。所谓内部消息是指agent内部触发的消息请求，
               处理完后不通过fcgi向外返还
Others       :-------------------------------------------------------- */
mp_int32 Communication::PopRspInternalMsgQueue(message_pair_t& msgPair)
{
    // 获取互斥锁
    CThreadAutoLock lock(&m_rspMsgQueueMutex);
    if (!m_rspMsgQueue.empty()) {
        LOGGUARD("");
        vector<message_pair_t>::iterator it = m_rspMsgQueue.begin();

        CResponseMsg* responseMsg = static_cast<CResponseMsg*>(it->pRspMsg);
        if (responseMsg == NULL) {
            (mp_void) m_rspMsgQueue.erase(it);
            COMMLOG(OS_LOG_ERROR, "responseMsg is not CResponseMsg.");
            return MP_FAILED;
        }

        while (it != m_rspMsgQueue.end() && !responseMsg->IsInternalMsg()) {
            ++it;
        }

        if (it == m_rspMsgQueue.end()) {
            return MP_FAILED;
        }
        msgPair = *it;
        (mp_void) m_rspMsgQueue.erase(it);
        COMMLOG(OS_LOG_DEBUG,
            "pop internal message from response queue success, req=0x%x, rsp=0x%x!",
            msgPair.pReqMsg,
            msgPair.pRspMsg);
        return MP_SUCCESS;
    } else {
        return MP_FAILED;
    }
}

/* ------------------------------------------------------------
Function Name: PushRspMsgQueue
Description  : 将响应消息加入到消息响应队列
Others       :-------------------------------------------------------- */
mp_void Communication::PushRspMsgQueue(const message_pair_t& msgPair)
{
    LOGGUARD("");
    // 获取互斥锁
    CThreadAutoLock lock(&m_rspMsgQueueMutex);
    m_rspMsgQueue.push_back(msgPair);
    COMMLOG(
        OS_LOG_DEBUG, "push message to response queue success, req=0x%x, rsp=0x%x!", msgPair.pReqMsg, msgPair.pRspMsg);
}

mp_int32 Communication::CheckIPandPort(mp_string& strPort)
{
    mp_socket sock = MP_INVALID_SOCKET;
    CSocket::CreateTcpSocket(sock);
    if (CSocket::Bind(sock, DEFAULT_LISTEN_IP, atoi(strPort.c_str())) != MP_SUCCESS) {
        CSocket::Close(sock);
        for (mp_uint16 port = DEFAULT_FCGI_START_PORT; port < DEFAULT_FCGI_END_PORT; port++) {
            mp_socket sock = MP_INVALID_SOCKET;
            CSocket::CreateTcpSocket(sock);
            if (CSocket::Bind(sock, DEFAULT_LISTEN_IP, port) != MP_SUCCESS) {
                CSocket::Close(sock);
                WARNLOG("Port %d is occupied, try next.", port);
            } else {
                CSocket::Close(sock);
                INFOLOG("FCGI server port is %d.", port);
                strPort = std::to_string(port);
                return MP_SUCCESS;
            }
        }
        ERRLOG("Get FCGI port failed use default port 8091");
        strPort = "8091";
        return MP_FAILED;
    }
    CSocket::Close(sock);
    return MP_SUCCESS;
}
