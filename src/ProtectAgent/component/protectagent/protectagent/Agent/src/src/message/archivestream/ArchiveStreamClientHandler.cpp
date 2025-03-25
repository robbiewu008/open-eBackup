#include "message/archivestream/ArchiveStreamClientHandler.h"
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>
#include <algorithm>
#include <random>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/CMpTime.h"
#include "common/DB.h"
#include "common/Path.h"
#include "securecom/CryptAlg.h"
#include "common/ConfigXmlParse.h"
#include "servicecenter/timerservice/detail/Timer.h"
namespace {
// use the cer to connect dme_archive
const mp_string CAINFO = "pmca.pem";
const mp_string SSLCERT = "server.pem";
const mp_string SSLKEY = "server.key";

const mp_int32 VERIFY_DEPTH = 10;
const mp_int32 NOCONNECTED_SLEEP = 200;
const mp_uint32 RECONNECT_SLEEP_TIME = 2000; // unit millisecond
const mp_uint32 DEFULAT_SEND_TIMEOUT = 3;    // send time out, default 3 seconds
const mp_uint32 PERIOD_HEARTBEAT = 30000;    // heartbeat period, unit millisecond
const mp_uint32 RECV_SLEEP_TIME = 500;
} // namespace

using std::map;
using std::ostringstream;
using std::vector;

ArchiveStreamClientHandler::ArchiveStreamClientHandler()
{
    INFOLOG("Enter construct.");
    CMpThread::InitLock(&m_reqMsgMutext);
    CMpThread::InitLock(&m_rspMsgMutext);

    m_serverList.clear();

    m_recvExitFlag = MP_FALSE;
    m_sendExitFlag = MP_FALSE;
    m_client.GetConnection().SetRecvExitFlag(MP_FALSE);

    m_inited = MP_FALSE;
    m_dissconnected = MP_FALSE;
#ifdef SUPPORT_SSL
    m_pSslCtx = NULL;
#endif
    m_secureCh = MP_FALSE;

    (mp_void)memset_s(&m_recvTid, sizeof(m_recvTid), 0, sizeof(m_recvTid));
    (mp_void)memset_s(&m_sendTid, sizeof(m_sendTid), 0, sizeof(m_sendTid));
}

ArchiveStreamClientHandler::~ArchiveStreamClientHandler()
{
    INFOLOG("Enter destruct.");
    m_recvExitFlag = MP_TRUE;
    m_sendExitFlag = MP_TRUE;
    m_client.GetConnection().SetRecvExitFlag(MP_TRUE);

    if (m_recvTid.os_id != 0) {
        (mp_void)CMpThread::WaitForEnd(&m_recvTid, NULL);
    }

    if (m_sendTid.os_id != 0) {
        (mp_void)CMpThread::WaitForEnd(&m_sendTid, NULL);
    }
    INFOLOG("recv thread join.");
#ifdef SUPPORT_SSL
    if (m_pSslCtx != NULL) {
        SSL_CTX_free(m_pSslCtx);
        m_pSslCtx = NULL;
    }
#endif
    CMpThread::DestroyLock(&m_reqMsgMutext);
    CMpThread::DestroyLock(&m_rspMsgMutext);
    INFOLOG("thread destroy.");

    CDppMessage *msg = NULL;
    for (std::vector<CDppMessage *>::iterator iter = m_requestMsgs.begin(); iter != m_requestMsgs.end(); ++iter) {
        msg = *iter;
        if (msg != NULL) {
            delete msg;
            msg = NULL;
        }
    }
    m_requestMsgs.clear();

    for (RspMsgIter iter = m_responseMsgs.begin(); iter != m_responseMsgs.end(); ++iter) {
        for (std::vector<CDppMessage *>::iterator it = iter->second.begin(); it != iter->second.end(); ++it) {
            msg = *it;
            if (msg != NULL) {
                delete msg;
                msg = NULL;
            }
        }
    }

    m_responseMsgs.clear();
    INFOLOG("Clear msg finish.");
}
mp_void ArchiveStreamClientHandler::handleRecevMsg(CDppMessage *msg)
{
    if (msg == NULL) {
        ERRLOG("Msg is NULL");
        return;
    }

    Json::Value msgBody;
    mp_uint32 cmd = msg->GetCmd();
    mp_uint64 seq = msg->GetOrgSeqNo();
    DBGLOG("Recieve message cmd=0x%x, seq=%llu.", cmd, seq);

    mp_string taskId;
    if (cmd == CMD_ARCHIVE_GET_FILE_DATA_BIN_ACK) {
        std::ostringstream strBuf;
        strBuf << seq;
        taskId = strBuf.str();
    } else {
        if (msg->GetManageBody(msgBody) != MP_SUCCESS) {
            ERRLOG("Unable to push message cmd=0x%x, seq=%llu into received message list.", cmd, seq);
            delete msg;
            msg = NULL;
            return;
        }
        if (cmd == CMD_ARCHIVE_GET_FILE_DATA_JSON_ACK) {
            std::ostringstream strBuf;
            strBuf << seq;
            taskId = strBuf.str();
        } else {
            if (msgBody[MANAGECMD_KEY_BODY].isObject() &&
                msgBody[MANAGECMD_KEY_BODY][MANAGECMD_KEY_TASKID].isString()) {
                taskId = msgBody[MANAGECMD_KEY_BODY][MANAGECMD_KEY_TASKID].asString();
            }
        }
    }
    msg->UpdateTime();

    CThreadAutoLock lock(&m_rspMsgMutext);
    RspMsgIter iter = m_responseMsgs.find(taskId);
    if (iter == m_responseMsgs.end()) {
        std::vector<CDppMessage *> vecMsgs;
        vecMsgs.push_back(msg);
        m_responseMsgs[taskId] = std::move(vecMsgs);
    } else {
        iter->second.push_back(msg);
    }
}

mp_int32 ArchiveStreamClientHandler::Init()
{
    mp_int32 iRet = MP_SUCCESS;
    if (m_inited) {
        return MP_SUCCESS;
    }
    // start receive thread
    iRet = CMpThread::Create(&m_recvTid, RecvThread, this);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init recv thread failed, ret %d.", iRet);
        return MP_FAILED;
    }

    // start send thread
    iRet = CMpThread::Create(&m_sendTid, SendThread, this);
    if (iRet != MP_SUCCESS) {
        (mp_void)CMpThread::WaitForEnd(&m_recvTid, NULL);
        ERRLOG("Init send thread failed, ret %d.", iRet);
        return MP_FAILED;
    }

#ifdef SUPPORT_SSL
    InitSsl();
#endif

    m_inited = true;
    return MP_SUCCESS;
}

mp_int32 ArchiveStreamClientHandler::Connect(const mp_string &busiIP,
    mp_uint16 busiPort, MESSAGE_ROLE role, bool useSSL)
{
    if (busiIP.empty()) {
        ERRLOG("Server client ip address is null.");
        return MP_FAILED;
    }
    mp_string busiIp = busiIP;
    mp_int32 iRet = MP_SUCCESS;
#ifdef SUPPORT_SSL
    if (useSSL) { // 是否开启ssl
        if (m_secureCh && m_pSslCtx != NULL) {
            iRet = m_client.Init(busiIp, busiPort, role, m_pSslCtx);
        } else {
            ERRLOG("Do not use secure channel, Init server init failed, business %s:%u, iRet %d.",
                busiIp.c_str(), busiPort, iRet);
            return MP_FAILED;
        }
    } else {
        iRet = m_client.Init(busiIp, busiPort, role, NULL);
    }
#else
    iRet = m_client.Init(busiIp, busiPort, role);
#endif
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init server init failed, business %s:%u, iRet %d.", busiIp.c_str(), busiPort, iRet);
        return iRet;
    }

    iRet = m_client.Connect();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Connect DME_Archive failed, business %s:%u, iRet %d.", busiIp.c_str(), busiPort, iRet);
        return iRet;
    }

    INFOLOG("Connect DME_Archive succ, business %s:%u.", busiIp.c_str(), busiPort);
    return MP_SUCCESS;
}

mp_int32 ArchiveStreamClientHandler::Connect(mp_string &busiIp, mp_uint16 busiPort, bool openSsl)
{
    std::vector<mp_string> serverList;
    CMpString::StrSplit(serverList, busiIp, ',');
    if (serverList.empty()) {
        ERRLOG("Split ip failed, PM ip list is empty(%s).", busiIp.c_str());
        return MP_FAILED;
    }
    // 随机打乱server列表，负荷分担
    std::random_device rd;
    std::shuffle(std::begin(serverList), std::end(serverList), rd);
    mp_int32 iRet = MP_SUCCESS;

    CConnection &conn = m_client.GetConnection();
    mp_bool connectFlg = MP_FALSE;
    mp_bool connectTooMuch = MP_FALSE;
    for (int i = 0; i < serverList.size(); ++i) {
        iRet = Connect(serverList[i], busiPort, ROLE_HOST_AGENT, openSsl);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Current server [%s:%u] link failed, will connect next server, iRet %d.", serverList[i].c_str(),
                busiPort, iRet);
            if (iRet == MP_ARCHIVE_TOO_MUCH_CONNECTION) {
                connectTooMuch = MP_TRUE;
            }
            continue;
        } else {
            busiIp = serverList[i];
            connectFlg = MP_TRUE;
            break;
        }
    }
    if (!connectFlg) {
        ERRLOG("Connect all servers failed.");
        if (connectTooMuch) {
            return MP_ARCHIVE_TOO_MUCH_CONNECTION;
        }
        return iRet;
    }

    m_OpenSSl = openSsl;
    m_busiPort = busiPort;
    m_serverList = std::move(serverList);

    if (m_dissconnected == MP_TRUE) {
        if (CreateThreadIfReconnect() != MP_SUCCESS) {
            ERRLOG("Reconnect Create Thread fail.");
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}
mp_int32 ArchiveStreamClientHandler::Disconnect()
{
    INFOLOG("ArchiveStreamClientHandler::Disconnect");
    CConnection &conn = m_client.GetConnection();
    EndThreadIfDisConnect();
    if (conn.GetLinkState() == LINK_STATE_LINKED) {
        conn.DisConnect();
    }
    m_dissconnected = MP_TRUE;
    return MP_SUCCESS;
}

mp_int32 ArchiveStreamClientHandler::Connect()
{
    mp_bool connectFlg = MP_FALSE;
    mp_int32 iRet = MP_SUCCESS;

    for (int i = 0; i < m_serverList.size(); ++i) {
        iRet = Connect(m_serverList[i], m_busiPort, ROLE_HOST_AGENT, m_OpenSSl);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Current server [%s:%u] link failed, will connect next server, iRet %d.", m_serverList[i].c_str(),
                m_busiPort, iRet);
            continue;
        } else {
            connectFlg = MP_TRUE;
            m_currentServerIP = m_serverList[i];
            break;
        }
    }

    if (!connectFlg) {
        ERRLOG("Connect all servers failed.");
        return MP_FAILED;
    }
    if (m_dissconnected == MP_TRUE) {
        if (CreateThreadIfReconnect() != MP_SUCCESS) {
            ERRLOG("Reconnect Create Thread fail.");
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}
mp_int32 ArchiveStreamClientHandler::SwitchConnect()
{
    mp_bool connectFlg = MP_FALSE;
    mp_int32 iRet = MP_SUCCESS;
    
    CConnection &conn = m_client.GetConnection();
    if (conn.GetLinkState() == LINK_STATE_LINKED) {
        conn.DisConnect();
    }
    for (int i = 0; i < m_serverList.size(); ++i) {
        if (m_currentServerIP == m_serverList[i]) {
            continue;
        }
        iRet = Connect(m_serverList[i], m_busiPort, ROLE_HOST_AGENT, m_OpenSSl);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Current server [%s:%u] link failed, will connect next server, iRet %d.", m_serverList[i].c_str(),
                m_busiPort, iRet);
            continue;
        } else {
            connectFlg = MP_TRUE;
            break;
        }
    }

    if (!connectFlg) {
        ERRLOG("Connect all servers failed.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}
mp_int32 ArchiveStreamClientHandler::GetConnectState()
{
    CConnection &conn = m_client.GetConnection();
    DBGLOG("HeartBeat info:%d-%d.", conn.GetLinkState(), conn.CheckHBTimeOut());
    if (conn.GetLinkState() == LINK_STATE_LINKED) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_bool ArchiveStreamClientHandler::GetRecvExitFlag()
{
    return m_recvExitFlag;
}

mp_bool ArchiveStreamClientHandler::GetSendExitFlag()
{
    return m_sendExitFlag;
}

#ifdef WIN32
DWORD WINAPI ArchiveStreamClientHandler::RecvThread(mp_void *client)
#else
void *ArchiveStreamClientHandler::RecvThread(mp_void *client)
#endif
{
    ArchiveStreamClientHandler *tcpClient = static_cast<ArchiveStreamClientHandler *>(client);
    if (tcpClient == NULL) {
        ERRLOG("Convert to ArchiveStreamClientHandler failed.");
#ifdef WIN32
        return MP_SUCCESS;
#else
        return NULL;
#endif
    }

    tcpClient->RecvMsgFromSvr();

#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

mp_uint64 ArchiveStreamClientHandler::GetSeqNo()
{
    return m_client.GetSeqNo();
}

mp_string ArchiveStreamClientHandler::GetConnectedServerAddr()
{
    CConnection &conn = m_client.GetConnection();
    return conn.GetClientIpAddrStr();
}

mp_void ArchiveStreamClientHandler::RecvMsgFromSvr()
{
    INFOLOG("Start receive message process.");

    CConnection &conn = m_client.GetConnection();

    while ((!GetRecvExitFlag()) && (!conn.GetRecvExitFlag())) {
        if (conn.GetLinkState() != LINK_STATE_LINKED) {
            CMpTime::DoSleep(NOCONNECTED_SLEEP);
            continue;
        }

        if (!m_client.RecvEventsReady()) {
            CMpTime::DoSleep(NOCONNECTED_SLEEP);
            continue;
        }

        mp_int32 iRet = m_client.RecvMessage();
        if (iRet == MP_ABORTED) {
            WARNLOG("Recv existing...");
            break;
        }
        if (iRet != MP_SUCCESS) {
            ERRLOG("Unable to receive message from service.");
            CMpTime::DoSleep(NOCONNECTED_SLEEP);
            continue;
        }

        CDppMessage *msg = m_client.GetRecvMsg();
        if (msg == NULL) {
            WARNLOG("Recv null message from server.");
            continue;
        }

        m_client.setRecvMsg(NULL);

        mp_uint32 cmd = msg->GetCmd();
        mp_uint64 seq = msg->GetOrgSeqNo();

        conn.UpdateHBTime();
        handleRecevMsg(msg);
    }

    WARNLOG("Exit recv message thread.");
}

#ifdef WIN32
DWORD WINAPI ArchiveStreamClientHandler::SendThread(mp_void *client)
#else
void *ArchiveStreamClientHandler::SendThread(mp_void *client)
#endif
{
    mp_int32 iRet;
    ArchiveStreamClientHandler *tcpClient = static_cast<ArchiveStreamClientHandler *>(client);
    if (tcpClient == NULL) {
        ERRLOG("Convert to ArchiveStreamClientHandler failed.");
#ifdef WIN32
        return MP_SUCCESS;
#else
        return NULL;
#endif
    }

    tcpClient->SendMsq2Svr();

#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

mp_void ArchiveStreamClientHandler::SendMsq2Svr()
{
    static const mp_uint32 sendInterTime = 200;
    INFOLOG("Start Send message process.");
    mp_int32 iRet = MP_FAILED;

    uint32_t reportInterval = sendInterTime * sendInterTime;
    uint32_t checkCount = reportInterval;
    while (!GetSendExitFlag()) {
        CConnection &conn = m_client.GetConnection();
        if (conn.GetLinkState() != LINK_STATE_LINKED) {
            CMpTime::DoSleep(NOCONNECTED_SLEEP);
            if (checkCount < reportInterval) {
                checkCount++;
            } else {
                DBGLOG("Not state linked, wait.");
                checkCount = 0;
            }
            continue;
        }

        CDppMessage *msg = NULL;
        {
            CThreadAutoLock lock(&m_reqMsgMutext);
            if (m_requestMsgs.size() > 0) {
                std::vector<CDppMessage *>::iterator iter = m_requestMsgs.begin();
                msg = *iter;
                m_requestMsgs.erase(iter);
                DBGLOG("Get wait msg.");
            }
        }

        if (msg != NULL) {
            msg->SetLinkInfo(conn.GetClientIpAddrStr(), conn.GetClientPort());
            iRet = msg->ReinitMsgBody();
            if (iRet != MP_SUCCESS) {
                ERRLOG("Reintial message body failed.");
                continue;
            }

            iRet = m_client.SendMsg(*msg, conn);
            if (iRet != MP_SUCCESS) {
                ERRLOG("Send message failed, cmd:%u, seq=%llu.", msg->GetCmd(), msg->GetOrgSeqNo());
            } else {
                DBGLOG("Send message success, cmd:%u, seq=%llu.", msg->GetCmd(), msg->GetOrgSeqNo());
            }
            delete msg;
            msg = NULL;
        }
        if (m_requestMsgs.empty()) {
            CMpTime::DoSleep(sendInterTime);
        }
    }

    INFOLOG("Exit send message thread.");
}

mp_void ArchiveStreamClientHandler::PushMsg2Queue(CDppMessage *reqMsg)
{
    CThreadAutoLock lock(&m_reqMsgMutext);
    m_requestMsgs.push_back(reqMsg);
}

mp_int32 ArchiveStreamClientHandler::CreateThreadIfReconnect()
{
    m_recvExitFlag = MP_FALSE;
    m_sendExitFlag = MP_FALSE;
    m_dissconnected = MP_FALSE;
    m_client.GetConnection().SetRecvExitFlag(MP_FALSE);

    mp_int32 iRet = MP_SUCCESS;
    // start receive thread
    iRet = CMpThread::Create(&m_recvTid, RecvThread, this);
    INFOLOG("Recv thread Create.");
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init recv thread failed, ret %d.", iRet);
        return MP_FAILED;
    }

    // start send thread
    iRet = CMpThread::Create(&m_sendTid, SendThread, this);
    INFOLOG("Send thread Create.");
    if (iRet != MP_SUCCESS) {
        (mp_void)CMpThread::WaitForEnd(&m_recvTid, nullptr);
        ERRLOG("Init send thread failed, ret %d.", iRet);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_void ArchiveStreamClientHandler::EndThreadIfDisConnect()
{
    m_recvExitFlag = MP_TRUE;
    m_sendExitFlag = MP_TRUE;
    m_client.GetConnection().SetRecvExitFlag(MP_TRUE);

    if (m_recvTid.os_id != 0) {
        (mp_void)CMpThread::WaitForEnd(&m_recvTid, nullptr);
    }

    if (m_sendTid.os_id != 0) {
        (mp_void)CMpThread::WaitForEnd(&m_sendTid, nullptr);
    }

    (mp_void)memset_s(&m_recvTid, sizeof(m_recvTid), 0, sizeof(m_recvTid));
    (mp_void)memset_s(&m_sendTid, sizeof(m_sendTid), 0, sizeof(m_sendTid));
}

mp_void ArchiveStreamClientHandler::SendDPMessage(const mp_string &taskId, CDppMessage *reqMsg)
{
    // 10 min message have not been dispatch, release message
    if (reqMsg == NULL) {
        ERRLOG("Request message is NULL or response message isn't NULL, taskid=%s.", taskId.c_str());
        return;
    }

    PushMsg2Queue(reqMsg);
}

mp_bool ArchiveStreamClientHandler::GetResponseMessage(const mp_string &taskId, CDppMessage *&rspMsg,
    mp_uint64 seqNo, mp_uint32 timeout)
{
    // 10 min message have not been dispatch, release message
    static const double timeoutMsg = 600;
    static const mp_uint32 interTime = 10;
    if (rspMsg != NULL) {
        ERRLOG("Response message isn't NULL, taskid=%s.", taskId.c_str());
        return MP_FALSE;
    }

    mp_time sendTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    while (true) {
        CMpTime::DoSleep(interTime);
        mp_time nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (CMpTime::Difftime(nowTime, sendTime) > timeout) {
            ERRLOG("TimeOut receive message, taskid=%s, seq=%llu.", taskId.c_str(), seqNo);
            return MP_FALSE;
        }
        CThreadAutoLock lock(&m_rspMsgMutext);
        RspMsgIter iter = m_responseMsgs.find(taskId);
        if (iter == m_responseMsgs.end()) {
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
                WARNLOG("Msg timeout, cmd=0x%x, seq=%llu.", (*it)->GetManageCmd(), (*it)->GetOrgSeqNo());
                delete *it;
                it = iter->second.erase(it);
            } else {
                ++it;
            }
        }

        if (iter->second.size() == 0) {
            m_responseMsgs.erase(iter);
        }

        // found message
        if (rspMsg != NULL) {
            if (rspMsg->GetCmd() != CMD_ARCHIVE_GET_FILE_DATA_BIN_ACK) {
                Json::Value dppBody;
                rspMsg->GetManageBody(dppBody);
            }
            break;
        }
    }

    return MP_TRUE;
}

EXTER_ATTACK mp_void ArchiveStreamClientHandler::InitSsl()
{
#ifdef SUPPORT_SSL
    DBGLOG("Initialization openssl.");
    if (m_pSslCtx != NULL) {
        INFOLOG("Agent was haved sslctx.");
        return;
    }

    mp_int32 sercureChannelFlag = 1;
    mp_int32 secure_channel;
    CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_SECURE_CHANNEL, secure_channel);
    if (secure_channel != sercureChannelFlag) {
        INFOLOG("Do not use secure channel : %d", secure_channel);
        m_secureCh = MP_FALSE;
        return;
    }

    m_secureCh = MP_TRUE;

    // 设置安全会话环境
    m_pSslCtx = SSL_CTX_new(TLSv1_2_client_method());
    if (m_pSslCtx == NULL) {
        ERRLOG("Init client ssl context failed.");
        return;
    }

    /* 设置算法套件 */
    mp_string Algorithm_Suite;
    if (CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_ALGORITHM_SUITE, Algorithm_Suite) !=
        MP_SUCCESS) {
        ERRLOG("Get Algorithm suite failed.");
        SSL_CTX_free(m_pSslCtx);
        m_pSslCtx = NULL;
        return;
    }

    SSL_CTX_set_cipher_list(m_pSslCtx, Algorithm_Suite.c_str());
    SSL_CTX_set_verify_depth(m_pSslCtx, VERIFY_DEPTH);
    SSL_CTX_set_options(m_pSslCtx, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 |
        SSL_OP_NO_COMPRESSION | SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_NO_RENEGOTIATION);
    SSL_CTX_set_verify(m_pSslCtx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_mode(m_pSslCtx, SSL_MODE_AUTO_RETRY);

    if (InitVerityCert() == MP_FALSE) {
        ERRLOG("Init verity Cert failed.");
        SSL_CTX_free(m_pSslCtx);
        m_pSslCtx = NULL;
    }
#endif
}

mp_bool ArchiveStreamClientHandler::InitVerityCert()
{
#ifdef SUPPORT_SSL
    DBGLOG("Certificate verification initialization.");

    /* 设置信任根证书 */
    mp_string caInfoPath = CPath::GetInstance().GetNginxConfFilePath(CAINFO);
    int ret = SSL_CTX_load_verify_locations(m_pSslCtx, caInfoPath.c_str(), NULL);
    if (ret <= 0) {
        ERRLOG("Failed to set the trust root certificate. errcode:%d", ret);
        return MP_FALSE;
    }

    /* 载入用户的数字证书 */
    mp_string sslCertPath = CPath::GetInstance().GetNginxConfFilePath(SSLCERT);
    ret = SSL_CTX_use_certificate_file(m_pSslCtx, sslCertPath.c_str(), SSL_FILETYPE_PEM);
    if (ret <= 0) {
        ERRLOG("Load the user's digital certificate failed. errcode:%d", ret);
        return MP_FALSE;
    }

    mp_string CipherStr;
    if (CConfigXmlParser::GetInstance().GetValueString(CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD,
        CipherStr) != MP_SUCCESS) {
        ERRLOG("Get GetValueString of ssl_key_password failed.");
        return MP_FALSE;
    }

    mp_string outStr;
    DecryptStr(CipherStr, outStr);
    if (outStr.empty()) {
        ERRLOG("DecryptStr private key password failed.");
        return MP_FALSE;
    }

    /* 设置证书密码 */
    SSL_CTX_set_default_passwd_cb_userdata(m_pSslCtx, (void *)outStr.c_str());

    /* 载入用户的私钥文件 */
    mp_string sslKeyPath = CPath::GetInstance().GetNginxConfFilePath(SSLKEY);
    ret = SSL_CTX_use_PrivateKey_file(m_pSslCtx, sslKeyPath.c_str(), SSL_FILETYPE_PEM);
    if (ret <= 0) {
        ERRLOG("Load the user private key failed. errcode:%d", ret);
        ClearString(outStr); // clear memory passwd
        return MP_FALSE;
    }

    /* 检查用户私钥是否正确 */
    if (!SSL_CTX_check_private_key(m_pSslCtx)) {
        ERRLOG("The user private key is incorrect.");
        ClearString(outStr); // clear memory passwd
        return MP_FALSE;
    }
    ClearString(outStr); // clear memory passwd
    DBGLOG("Certificate verification initialization success.");
#endif

    return MP_TRUE;
}
