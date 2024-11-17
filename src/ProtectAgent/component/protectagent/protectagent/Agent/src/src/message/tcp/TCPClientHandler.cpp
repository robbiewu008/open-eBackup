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
#include "message/tcp/TCPClientHandler.h"
#include <sstream>
#include <vector>
#include <map>
#include "openssl/ssl.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/CMpTime.h"
#include "common/DB.h"
#include "common/Path.h"
#include "securecom/CryptAlg.h"
#include "common/ConfigXmlParse.h"
#include "message/tcp/MessageHandler.h"

namespace {
// use the cer to connect dme_vmare(oracle and vmare)
const mp_string CAINFO = "pmca.pem";
const mp_string SSLCERT = "server.pem";
const mp_string SSLKEY = "server.key";
const mp_int32 VERIFY_DEPTH = 10;
}  // namespace

using std::map;
using std::ostringstream;
using std::vector;

TCPClientHandler TCPClientHandler::tcpHandler;

TCPClientHandler& TCPClientHandler::GetInstance()
{
    COMMLOG(OS_LOG_INFO, "constructer TCPClientHandler, this pointer:%p.", &tcpHandler);
    return tcpHandler;
}

TCPClientHandler::TCPClientHandler()
{
    CMpThread::InitLock(&busiCliensMutex);
    CMpThread::InitLock(&msgListMutex);
    CMpThread::InitLock(&busiConnectMutex);
    CMpThread::InitLock(&rwConnLock);

    hbExitFlag = MP_FALSE;
    recvExitFlag = MP_FALSE;
    sendExitFlag = MP_FALSE;
    msgHandlerExitFlag = MP_FALSE;
    m_secureCh = MP_FALSE;
#ifdef SUPPORT_SSL
    pSslCtx = NULL;
#endif

    (mp_void) memset_s(&hbTid, sizeof(hbTid), 0, sizeof(hbTid));
    (mp_void) memset_s(&recvTid, sizeof(recvTid), 0, sizeof(recvTid));
    (mp_void) memset_s(&sendTid, sizeof(sendTid), 0, sizeof(sendTid));
    (mp_void) memset_s(&msgHandlerTid, sizeof(msgHandlerTid), 0, sizeof(msgHandlerTid));

    // define handler function by the cmd number
    dppHandlers.emplace(
        map<mp_uint32, DppAction>::value_type(MANAGE_CMD_NO_HEARTBEATE_ACK, &TCPClientHandler::HandleHBAck));
    dppHandlers.emplace(
        map<mp_uint32, DppAction>::value_type(MANAGE_CMD_NO_TASK_COMPLETED, &TCPClientHandler::HandleTaskCompleted));
}

TCPClientHandler::~TCPClientHandler()
{
    hbExitFlag = MP_TRUE;
    recvExitFlag = MP_TRUE;
    sendExitFlag = MP_TRUE;
    msgHandlerExitFlag = MP_TRUE;
    if (hbTid.os_id != 0) {
        CMpThread::WaitForEnd(&hbTid, NULL);
    }

    if (recvTid.os_id != 0) {
        CMpThread::WaitForEnd(&recvTid, NULL);
    }

    if (sendTid.os_id != 0) {
        CMpThread::WaitForEnd(&sendTid, NULL);
    }

    if (msgHandlerTid.os_id != 0) {
        CMpThread::WaitForEnd(&msgHandlerTid, NULL);
    }

    // remove business client, lock isn't need, while all threads have exit
    for (vector<BusinessClient *>::iterator iter = busiClients.begin(); iter != busiClients.end();) {
        BusinessClient *busiClient = *iter;
        if (busiClient == NULL) {
            ++iter;
        } else {
            busiClient->DisConnect();
            delete busiClient;
            iter = busiClients.erase(iter);
        }
    }
    busiClients.clear();

    // remove message list
    while (msgList.size() > 0) {
        CDppMessage *dppMsg = msgList.front();
        msgList.erase(msgList.begin());
        delete dppMsg;
    }
    CMpThread::DestroyLock(&busiCliensMutex);
    CMpThread::DestroyLock(&msgListMutex);
    CMpThread::DestroyLock(&busiConnectMutex);
    CMpThread::DestroyLock(&rwConnLock);
    dppHandlers.clear();
#ifdef SUPPORT_SSL
    if (pSslCtx != NULL) {
        SSL_CTX_free(pSslCtx);
        pSslCtx = NULL;
    }
#endif
}

mp_int32 TCPClientHandler::Init()
{
#ifdef SUPPORT_SSL
    InitSsl();
#endif
    mp_int32 iRet = InitBusinessClients();
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "InitBusinessClients failed, ret %d.", iRet);
        return MP_FAILED;
    }
    // start hb thread
    iRet = CMpThread::Create(&hbTid, HeartBeat, this);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Init heartbeat task worker failed, ret %d.", iRet);
        return MP_FAILED;
    }

    // start receive thread
    iRet = CMpThread::Create(&recvTid, RecvThread, this);
    if (MP_SUCCESS != iRet) {
        // close heartbeat thread only, there is no business socket, no need to close it
        hbExitFlag = MP_TRUE;
        CMpThread::WaitForEnd(&hbTid, NULL);
        COMMLOG(OS_LOG_ERROR, "Init recv task worker failed, ret %d.", iRet);
        return MP_FAILED;
    }

    // start send thread
    iRet = CMpThread::Create(&sendTid, SendThread, this);
    if (MP_SUCCESS != iRet) {
        // close heartbeat thread only, there is no business socket, no need to close it
        hbExitFlag = MP_TRUE;
        CMpThread::WaitForEnd(&hbTid, NULL);
        CMpThread::WaitForEnd(&recvTid, NULL);
        COMMLOG(OS_LOG_ERROR, "Init send task worker failed, ret %d.", iRet);
        return MP_FAILED;
    }

    // start handler message thread
    iRet = CMpThread::Create(&msgHandlerTid, HandleMsgThread, this);
    if (MP_SUCCESS != iRet) {
        // close heartbeat thread only, there is no business socket, no need to close it
        hbExitFlag = MP_TRUE;
        CMpThread::WaitForEnd(&hbTid, NULL);
        CMpThread::WaitForEnd(&recvTid, NULL);
        CMpThread::WaitForEnd(&sendTid, NULL);
        COMMLOG(OS_LOG_ERROR, "Init message handler task worker failed, ret %d.", iRet);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 TCPClientHandler::Connect(mp_string &busiIp, mp_uint16 busiPort, MESSAGE_ROLE role)
{
    if (busiIp.empty()) {
        ERRLOG("business client ip address is null.");
        return MP_FAILED;
    }

    // 防止DME同时让Agent连接同一个IP，加锁保证只要有连接就会复用当前连接
    CThreadAutoLock busiLock(&busiConnectMutex);
    mp_string tempip = CheckParamInvalidReplace(busiIp);
    mp_bool bExists = MP_FALSE;
    BusinessClient *busiClient = GetBusiClientByIpPort(busiIp, busiPort);
    if (busiClient == NULL) {
        NEW_CATCH_RETURN_FAILED(busiClient, BusinessClient);
    } else {
        if (busiClient->GetConnection().GetLinkState() == LINK_STATE_LINKED) {
            INFOLOG("business %s:%u have already connected.", tempip.c_str(), busiPort);
            return MP_SUCCESS;
        }
        bExists = MP_TRUE;
    }

    DBGLOG("business %s:%u is added into business clients, size:%d.", tempip.c_str(), busiPort, busiClients.size());
    mp_int32 iRet = MP_FAILED;
#ifdef SUPPORT_SSL
    if (m_secureCh == MP_TRUE) {
        iRet = busiClient->Init(busiIp, busiPort, role, pSslCtx);
    } else {
        iRet = busiClient->Init(busiIp, busiPort, role);
    }
#else
    iRet = busiClient->Init(busiIp, busiPort, role);
#endif
    if (iRet != MP_SUCCESS) {
        RemoveBusiClient(busiClient);
        ERRLOG("Init business connection failed, business %s:%u, iRet %d.", tempip.c_str(), busiPort, iRet);
        return iRet;
    }

    iRet = busiClient->Connect();
    if (iRet != MP_SUCCESS) {
        RemoveBusiClient(busiClient);
        ERRLOG("Connect business connection failed, business %s:%u, iRet %d.", tempip.c_str(), busiPort, iRet);
        return iRet;
    }

    if (bExists == MP_FALSE) {
        AddBusiClient(busiClient);
    }
    INFOLOG("Connect business succ, business %s:%u, size:%u.", tempip.c_str(), busiPort, busiClients.size());
    return MP_SUCCESS;
}

mp_bool TCPClientHandler::GetHBExitFlag()
{
    return hbExitFlag;
}

mp_bool TCPClientHandler::GetRecvExitFlag()
{
    return recvExitFlag;
}

mp_bool TCPClientHandler::GetSendExitFlag()
{
    return sendExitFlag;
}

mp_bool TCPClientHandler::GetMsgHandlerExitFlag()
{
    return msgHandlerExitFlag;
}

#ifdef WIN32
DWORD WINAPI TCPClientHandler::HeartBeat(mp_void *client)
#else
void *TCPClientHandler::HeartBeat(mp_void *client)
#endif
{
    mp_int32 iRet;
    TCPClientHandler *tcpClient = static_cast<TCPClientHandler *>(client);
    if (!tcpClient) {
        COMMLOG(OS_LOG_ERROR, "convert to TCPClientHandler failed.");
#ifdef WIN32
        return MP_SUCCESS;
#else
        return NULL;
#endif
    }

    COMMLOG(OS_LOG_INFO, "Start heartbeat thread.");
    while (!tcpClient->GetHBExitFlag()) {
        // construct heartbeat command
        iRet = tcpClient->PushHBMsg();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "PushHBMsg failed, iRet %d.", iRet);
            continue;
        }

        // first sleep
        CMpTime::DoSleep(PERIOD_HEARTBEAT);
    }
    COMMLOG(OS_LOG_INFO, "Finish heartbeat thread.");
#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

#ifdef WIN32
DWORD WINAPI TCPClientHandler::RecvThread(mp_void *client)
#else
void *TCPClientHandler::RecvThread(mp_void *client)
#endif
{
    TCPClientHandler *tcpClient = static_cast<TCPClientHandler *>(client);
    if (!tcpClient) {
        COMMLOG(OS_LOG_ERROR, "convert to TCPClientHandler failed.");
#ifdef WIN32
        return MP_SUCCESS;
#else
        return NULL;
#endif
    }

    COMMLOG(OS_LOG_INFO, "Start receivce thread.");
    while (!tcpClient->GetRecvExitFlag()) {
        mp_int32 iRet = tcpClient->RecvMessage();
        if (iRet == MP_SUCCESS) {
            continue;
        }

        // when result isn't successfully, have no business client, need to sleep
        if (iRet != MP_NOEXISTS && iRet != MP_TIMEOUT) {
            COMMLOG(OS_LOG_ERROR, "RecvMessage failed, iRet %d.", iRet);
            continue;
        }

        if (iRet == MP_NOEXISTS) {
            CMpTime::DoSleep(THREAD_SLEEP_TIME);
        }
    }

#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

#ifdef WIN32
DWORD WINAPI TCPClientHandler::SendThread(mp_void *client)
#else
void *TCPClientHandler::SendThread(mp_void *client)
#endif
{
    static const mp_uint32 SEND_SLEEP_TIME = 100;  // unit millisecond
    mp_int32 iRet;
    TCPClientHandler *tcpClient = static_cast<TCPClientHandler *>(client);
    if (!tcpClient) {
        COMMLOG(OS_LOG_ERROR, "convert to TCPClientHandler failed.");
#ifdef WIN32
        return MP_SUCCESS;
#else
        return NULL;
#endif
    }

    COMMLOG(OS_LOG_INFO, "Start send thread.");
    while (!tcpClient->GetSendExitFlag()) {
        // try to get send message
        message_pair_t msgPair;
        iRet = MessageHandler::GetInstance().PopRspMsg(msgPair);
        if (iRet != MP_SUCCESS) {
            DoSleep(SEND_SLEEP_TIME);
            continue;
        }
        CDppMessage *rspMsg = static_cast<CDppMessage *>(msgPair.pRspMsg);
        COMMLOG(OS_LOG_DEBUG, "Get a new response message.");
        if (rspMsg == NULL) {
            COMMLOG(OS_LOG_ERROR, "message is not DPP message");
            continue;
        }
        iRet = tcpClient->SendDppMsg(*rspMsg);
        if (MP_SUCCESS != iRet) {
            COMMLOG(OS_LOG_ERROR, "send dpp message failed, iRet %d. cmd=%d, sqno=%d",
                iRet, rspMsg->GetManageCmd(), rspMsg->GetOrgSeqNo());
        }

        COMMLOG(OS_LOG_DEBUG, "dppmessage send sucess, rspMsg.src=%d, rspMsg.tgt=%d. cmd=%d, sqno=%d",
            rspMsg->GetMsgSrc(), rspMsg->GetMsgTgt(), rspMsg->GetManageCmd(), rspMsg->GetOrgSeqNo());
        if (msgPair.pRspMsg) {
            delete msgPair.pRspMsg;
            msgPair.pRspMsg = NULL;
        }
        if (msgPair.pReqMsg) {
            delete msgPair.pReqMsg;
            msgPair.pReqMsg = NULL;
        }
    }

#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

#ifdef WIN32
DWORD WINAPI TCPClientHandler::HandleMsgThread(mp_void *client)
#else
void *TCPClientHandler::HandleMsgThread(mp_void *client)
#endif
{
    TCPClientHandler *tcpClient = static_cast<TCPClientHandler *>(client);
    if (!tcpClient) {
        COMMLOG(OS_LOG_ERROR, "convert to TCPClientHandler failed.");
#ifdef WIN32
        return MP_SUCCESS;
#else
        return NULL;
#endif
    }

    COMMLOG(OS_LOG_INFO, "Start handle message thread.");
    while (!tcpClient->GetMsgHandlerExitFlag()) {
        tcpClient->HandleMessage();
    }

#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

mp_int32 TCPClientHandler::RecvMessage()
{
    mp_int32 iRet = WaitTcpEvents();
    // retry when timeout, other try to recvMsg, when receive message failed, close connection
    if (iRet == MP_TIMEOUT || iRet == MP_NOEXISTS) {
        return iRet;
    }

    vector<BusinessClient *> recvClients;
    {
        CThreadAutoLock busiLock(&busiCliensMutex);
        for (vector<BusinessClient *>::iterator iter = busiClients.begin(); iter != busiClients.end(); ++iter) {
            // receive business message
            if (CSocket::FdIsSet((*iter)->GetClientSocket(), fdsRead)) {
                recvClients.push_back(*iter);
            }
        }
    }

    for (vector<BusinessClient *>::iterator iter = recvClients.begin(); iter != recvClients.end(); ++iter) {
        {
            CThreadAutoLock rwLock(&rwConnLock);
            mp_int32 iRet = RecvMsg((*iter)->GetConnection());
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR,
                    "Receive message from business node(%s:%d) failed.",
                    (*iter)->GetConnection().GetClientIpAddrStr().c_str(),
                    (*iter)->GetConnection().GetClientPort());
            }
        }
        // if receive failed, remove close connection when receive only, send and heartbeat don't remove client
        if ((*iter)->GetConnection().GetLinkState() == LINK_STATE_NO_LINKED) {
            COMMLOG(OS_LOG_WARN,
                "begin to remove business node(%s:%d).",
                (*iter)->GetConnection().GetClientIpAddrStr().c_str(),
                (*iter)->GetConnection().GetClientPort());
            (*iter)->GetConnection().SetSendMsgFlag(MP_FALSE);
            RemoveBusiClient(*iter);
        }
    }

    return MP_SUCCESS;
}

mp_int32 TCPClientHandler::SendDppMsg(CDppMessage &message)
{
    CConnection *connection = GetConnectionByMessage(message);
    if (!connection) {
        return MP_FAILED;
    }

    mp_int32 iRet = message.ReinitMsgBody();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "reintial message body failed, IP %u.", message.GetIpAddr());
        return iRet;
    }
    CThreadAutoLock rwLock(&rwConnLock);
    return SendMsg(message, *connection);
}

mp_void TCPClientHandler::HandleMessage()
{
    // 如果消息队列中没有消息，等待100ms继续处理
    CDppMessage *message = NULL;
    {
        CThreadAutoLock busiLock(&msgListMutex);
        if (msgList.size() > 0) {
            message = msgList.front();
            msgList.erase(msgList.begin());
        }
    }
    if (message == NULL) {
        static mp_uint32 SLEEP_TIME = 100;
        DoSleep(SLEEP_TIME);
        return;
    }

    COMMLOG(OS_LOG_DEBUG, "handler message number=%u.", msgList.size());
    mp_uint32 manageCmd = message->GetManageCmd();
    map<mp_uint32, DppAction>::iterator iter = dppHandlers.find(manageCmd);
    if (iter != dppHandlers.end()) {
        mp_int32 iRet = (this->*iter->second)(*message);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "handler message 0x%.8x failed.", manageCmd);
        }
    } else {
        COMMLOG(OS_LOG_ERROR, "find message hander 0x%.8x failed.", manageCmd);
    }

    delete message;
}

mp_int32 TCPClientHandler::WaitTcpEvents()
{
    CSocket::FdInit(fdsRead);

    mp_uint16 maxFd = MP_INVALID_SOCKET;
    {
        CThreadAutoLock busiLock(&busiCliensMutex);
        if (busiClients.size() == 0) {
            return MP_NOEXISTS;
        }

        // config business client
        for (vector<BusinessClient *>::iterator iter = busiClients.begin(); iter != busiClients.end(); ++iter) {
            CSocket::FdSet((*iter)->GetClientSocket(), fdsRead);
            maxFd = (*iter)->GetClientSocket() > maxFd ? (*iter)->GetClientSocket() : maxFd;
        }
    }

    mp_int32 iRet = CSocket::WaitEvents(fdsRead, fdsWrite, maxFd);
    if (iRet < 0) {
        COMMLOG(OS_LOG_ERROR, "Wait events failed, iRet %d.", iRet);
        return iRet;
    } else if (iRet == 0) {
        // select timeout
        return MP_TIMEOUT;
    }

    return MP_SUCCESS;
}

mp_int32 TCPClientHandler::PushHBMsg()
{
    DBGLOG("begin to hearbeat, businessclient size:%u, this pointer:%p.", busiClients.size(), this);
    CThreadAutoLock busiLock(&busiCliensMutex);
    // ebk_app hearbeat message
    for (vector<BusinessClient *>::iterator iter = busiClients.begin(); iter != busiClients.end(); ++iter) {
        (mp_void)(*iter)->HeartBeat();
    }

    return MP_SUCCESS;
}

mp_int32 TCPClientHandler::NewMsgPair(CDppMessage *&reqMsg, CDppMessage *&rspMsg, mp_uint64 seqNo)
{
    NEW_CATCH_RETURN_FAILED(reqMsg, CDppMessage);
    NEW_CATCH(rspMsg, CDppMessage);
    if (!rspMsg) {
        delete reqMsg;
        reqMsg = NULL;
        ERRLOG("New CDppMessage failed");
        return MP_FAILED;
    }

    reqMsg->SetOrgSeqNo(seqNo);
    rspMsg->SetOrgSeqNo(seqNo);
    return MP_SUCCESS;
}

BusinessClient *TCPClientHandler::GetBusiClientByIpPort(const mp_string &busiIp, mp_uint16 busiPort)
{
    if (busiIp.empty()) {
        COMMLOG(OS_LOG_ERROR, "business client ip address is null.");
        return NULL;
    }

    COMMLOG(OS_LOG_DEBUG, "business client number %d, this pointer:%p.", busiClients.size(), this);
    CThreadAutoLock busiLock(&busiCliensMutex);
    for (vector<BusinessClient *>::iterator iter = busiClients.begin(); iter != busiClients.end(); ++iter) {
        if ((*iter)->GetConnection().GetClientIpAddrStr().compare(busiIp) == 0 &&
            (*iter)->GetConnection().GetClientPort() == busiPort) {
            return *iter;
        }
    }
    mp_string tempip = busiIp;
    mp_string temp = CheckParamInvalidReplace(tempip);
    WARNLOG("Can't get business client with invalid client ip:%s, port:%u, client %d.",
        temp.c_str(),
        busiPort,
        busiClients.size());
    return NULL;
}

mp_void TCPClientHandler::AddBusiClient(BusinessClient *busiClient)
{
    CThreadAutoLock busiLock(&busiCliensMutex);
    for (vector<BusinessClient *>::iterator iter = busiClients.begin(); iter != busiClients.end(); ++iter) {
        if ((*iter) == busiClient) {
            return;
        }
    }
    busiClients.push_back(busiClient);
}

mp_void TCPClientHandler::RemoveBusiClient(BusinessClient *&busiClient)
{
    CThreadAutoLock busiLock(&busiCliensMutex);
    vector<BusinessClient *>::iterator iter = busiClients.begin();
    for (; iter != busiClients.end(); ++iter) {
        if ((*iter) == busiClient) {
            break;
        }
    }

    if (iter == busiClients.end()) {
        WARNLOG("Can't get business client with business client pointer.");
    } else {
        INFOLOG("Remove business client %s:%d.",
            busiClient->GetConnection().GetClientIpAddrStr().c_str(),
            busiClient->GetConnection().GetClientPort());
        busiClients.erase(iter);

        busiClient->Remove();
        delete busiClient;
        busiClient = NULL;
    }

    DBGLOG("business client number %d.", busiClients.size());
}

// 根据消息内容获取connection，除了消息上报，都是connection指针获取
CConnection *TCPClientHandler::GetConnectionByMessage(CDppMessage &message)
{
    {
        CThreadAutoLock busiLock(&busiCliensMutex);
        for (vector<BusinessClient *>::iterator iter = busiClients.begin(); iter != busiClients.end(); ++iter) {
            COMMLOG(OS_LOG_DEBUG, "business client %s:%u",
                (*iter)->GetConnection().GetClientIpAddrStr().c_str(), (*iter)->GetConnection().GetClientPort());
            if ((*iter)->GetConnection().GetClientIpAddrStr() == message.GetIpAddr() &&
                (*iter)->GetConnection().GetClientPort() == message.GetPort()) {
                return &(*iter)->GetConnection();
            }
        }
    }

    COMMLOG(OS_LOG_ERROR,
        "Can't find server IP %s, server Port %u, cmd %u.",
        message.GetIpAddrStr().c_str(),
        message.GetPort(),
        message.GetManageCmd());
    return NULL;
}

mp_int32 TCPClientHandler::HandleRecvDppMessage(CDppMessage &message)
{
    LOGGUARD("");
    // if cmdno is register to AdminClient, need to handler by register function
    mp_uint32 manageCmd = message.GetManageCmd();
    mp_string strbuffer;
    // filter vmware feature's request with cmd 0x0408(1032) and 0x0414(1044)
    if (!CmdFilter(manageCmd) &&
        WipeSensitiveForJsonData(message.GetBuffer(), strbuffer) != MP_SUCCESS) {
        strbuffer = message.GetBuffer();
    }
    COMMLOG(OS_LOG_DEBUG,
        "receive dppmessage cmd is 0x%.8x, SeqNo:%u",
        manageCmd,
        message.GetOrgSeqNo());

    map<mp_uint32, DppAction>::iterator iter = dppHandlers.find(manageCmd);
    if (iter != dppHandlers.end()) {
        CThreadAutoLock busiLock(&msgListMutex);
        msgList.push_back(&message);
        COMMLOG(OS_LOG_DEBUG, "Receive message number=%d.", msgList.size());
    } else {
        // dispatch message
        CDppMessage *rspMsg = NULL;
        NEW_CATCH(rspMsg, CDppMessage);
        if (!rspMsg) {
            COMMLOG(OS_LOG_ERROR, "New CDppMessage failed");
            return MP_FAILED;
        }
        rspMsg->CloneMsg(message);
        message_pair_t msgPair(message, *rspMsg);
        // after deal, release message
        mp_int32 iRet = MessageHandler::GetInstance().PushReqMsg(msgPair);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "push request message failed.");
        }
        COMMLOG(OS_LOG_INFO, "push dppmessage SeqNo is:%u", message.GetOrgSeqNo());
    }

    return MP_SUCCESS;
}

mp_int32 TCPClientHandler::InitBusinessClients()
{
    mp_string strSql;
    DbParamStream dps;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    COMMLOG(OS_LOG_INFO, "Begin to query all business clients info.");
    ostringstream buff;
    buff << "select " << g_BusiClientRole << "," << g_BusiClientIP << "," << g_BusiClientPort << " from "
         << g_BusinessClient;
    strSql = buff.str();

    mp_int32 iRet = CDB::GetInstance().QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query all business clients failed, iRet %d.", iRet);
        return iRet;
    }

    for (mp_int32 iRowCurr = 0; iRowCurr < iRowCount; ++iRowCurr) {
        mp_string strRole;
        mp_string busiIp;
        mp_string strPort;
        readBuff >> strRole;
        readBuff >> busiIp;
        readBuff >> strPort;

        MESSAGE_ROLE busiRole = static_cast<MESSAGE_ROLE>(atoi(strRole.c_str()));
        mp_uint16 busiPort = static_cast<mp_uint16>(atoi(strPort.c_str()));
        mp_string tempip = CheckParamInvalidReplace(busiIp);
        iRet = Connect(busiIp, busiPort, busiRole);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "restore business %s:%u link failed, iRet %d.", tempip.c_str(), busiPort, iRet);
        }
    }
    COMMLOG(OS_LOG_INFO, "Connect all business client successfully.");
    return MP_SUCCESS;
}

// refresh heartbeat time
mp_int32 TCPClientHandler::HandleHBAck(CDppMessage &message)
{
    LOGGUARD("");
    CConnection *msgConn = GetConnectionByMessage(message);
    if (!msgConn) {
        return MP_FAILED;
    }

    msgConn->UpdateHBTime();
    return MP_SUCCESS;
}

mp_int32 TCPClientHandler::HandleTaskCompleted(CDppMessage &message)
{
    LOGGUARD("");
    mp_string busiIp = message.GetIpAddrStr();
    mp_string tempip = CheckParamInvalidReplace(busiIp);
    mp_uint16 busiPort = message.GetPort();
    COMMLOG(OS_LOG_INFO, "Begin to disconnect business clients info, %s:%u, now do nothing.", tempip.c_str(), busiPort);
    return MP_SUCCESS;
}

#ifdef SUPPORT_SSL
EXTER_ATTACK mp_void TCPClientHandler::InitSsl()
{
    INFOLOG("Initialization openssl.");
    if (pSslCtx != NULL) {
        INFOLOG("agent was haved sslctx.");
        return;
    }

    mp_int32 sercureChannelFlag = 1;
    mp_int32 secure_channel;
    CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_SECURE_CHANNEL, secure_channel);
    if (secure_channel != sercureChannelFlag) {
        INFOLOG("don't use secure channel : %d", secure_channel);
        m_secureCh = MP_FALSE;
        return;
    }

    m_secureCh = MP_TRUE;
    // SSL_library_init();           // 初始化OpenSSL
    // OpenSSL_add_all_algorithms(); // 载入所有SSL算法
    // SSL_load_error_strings();     // 载入所有SSL错误信息

    // 设置安全会话环境
    pSslCtx = SSL_CTX_new(TLSv1_2_client_method());
    if (!pSslCtx) {
        COMMLOG(OS_LOG_ERROR, "Init client ssl context failed.");
        return;
    }

    SSL_CTX_set_options(pSslCtx, SSL_OP_NO_RENEGOTIATION | SSL_OP_CIPHER_SERVER_PREFERENCE);
    INFOLOG("Init client ssl context succ.");
    // 设置算法套件
    mp_string Algorithm_Suite;
    if (CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_ALGORITHM_SUITE, Algorithm_Suite) !=
        MP_SUCCESS) {
        ERRLOG("get Algorithm suite failed.");
        return;
    }
    SSL_CTX_set_cipher_list(pSslCtx, Algorithm_Suite.c_str());

    SSL_CTX_set_verify_depth(pSslCtx, VERIFY_DEPTH);
    SSL_CTX_set_options(pSslCtx,
        SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_COMPRESSION |
            SSL_OP_CIPHER_SERVER_PREFERENCE);
    SSL_CTX_set_verify(pSslCtx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_mode(pSslCtx, SSL_MODE_AUTO_RETRY);

    if (InitVerityCert() == MP_FALSE) {
        ERRLOG("Init verity Cert failed.");
    }
}

mp_bool TCPClientHandler::InitVerityCert()
{
    INFOLOG("Certificate verification initialization.");
    mp_string caInfoPath = CPath::GetInstance().GetNginxConfFilePath(CAINFO);
    mp_string sslKeyPath = CPath::GetInstance().GetNginxConfFilePath(SSLKEY);
    mp_string sslCertPath = CPath::GetInstance().GetNginxConfFilePath(SSLCERT);

    /* 设置信任根证书 */
    mp_int32 ret = SSL_CTX_load_verify_locations(pSslCtx, caInfoPath.c_str(), NULL);
    if (ret <= 0) {
        COMMLOG(OS_LOG_ERROR, "Failed to set the trust root certificate.");
        return MP_FALSE;
    }

    /* 载入用户的数字证书 */
    ret = SSL_CTX_use_certificate_file(pSslCtx, sslCertPath.c_str(), SSL_FILETYPE_PEM);
    if (ret <= 0) {
        COMMLOG(OS_LOG_ERROR, "Load the user's digital certificate failed.");
        return MP_FALSE;
    }

    mp_string CipherStr;
    ret = CConfigXmlParser::GetInstance().GetValueString(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, CipherStr);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "get GetValueString of ssl_key_password failed.");
        return MP_FALSE;
    }

    mp_string outStr;
    DecryptStr(CipherStr, outStr);
    if (outStr.empty()) {
        COMMLOG(OS_LOG_ERROR, "DecryptStr private key password failed.");
        return MP_FALSE;
    }

    /* 设置证书密码 */
    SSL_CTX_set_default_passwd_cb_userdata(pSslCtx, (void *)outStr.c_str());

    /* 载入用户的私钥文件 */
    ret = SSL_CTX_use_PrivateKey_file(pSslCtx, sslKeyPath.c_str(), SSL_FILETYPE_PEM);
    if (ret <= 0) {
        COMMLOG(OS_LOG_ERROR, "Load the user private key failed.");
        ClearString(outStr);
        return MP_FALSE;
    }

    /* 检查用户私钥是否正确 */
    if (!SSL_CTX_check_private_key(pSslCtx)) {
        COMMLOG(OS_LOG_ERROR, "The user private key is incorrect.");
        ClearString(outStr);
        return MP_FALSE;
    }
    ClearString(outStr);
    return MP_TRUE;
}
#endif
