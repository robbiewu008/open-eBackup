#include "message/tcp/BusinessClient.h"
#include <sstream>
#include "common/Log.h"
#include "common/Ip.h"
#include "common/Utils.h"
#include "common/DB.h"
#include "message/tcp/MessageHandler.h"
#include "message/tcp/CConnection.h"
#include "message/tcp/CDppMessage.h"

using std::ostringstream;
namespace {
const int SLEEP_1000_MS = 1000;
const int RETRY_CONNECT_NUM = 5;
}  // namespace
BusinessClient::BusinessClient()
{
    seqNum = 0;
    conn.UpdateHBTime();
}

BusinessClient::~BusinessClient()
{}

#ifdef SUPPORT_SSL
mp_int32 BusinessClient::Init(mp_string &serverIp, mp_uint16 serverPort, MESSAGE_ROLE role, SSL_CTX *psslCtx)
#else
mp_int32 BusinessClient::Init(mp_string &serverIp, mp_uint16 serverPort, MESSAGE_ROLE role)
#endif
{
    mp_string temp = CheckParamInvalidReplace(serverIp);
    INFOLOG("BusinessClient ip=%s, port=%d.", temp.c_str(), serverPort);
    if (serverPort <= INVALID_PORT) {
        ERRLOG("tcp port %u is invalid.", serverPort);
        return MP_FAILED;
    }

    mp_uint32 ipv4Addr;
    mp_uint32 ipv6Addr[4];
    mp_int32 iRet = MP_FAILED;
    if (CIP::IsIPV4(serverIp)) {
        iRet = CIP::IPV4StrToUInt(serverIp, ipv4Addr);
    } else if (CIP::IsIPv6(serverIp)) {
        iRet = CIP::IPV6StrToUInt(serverIp, ipv6Addr, IPV6_NUMERIC_LEN);
    }

    if (MP_SUCCESS != iRet) {
        return MP_FAILED;
    }

    conn.SetClientPort(serverPort);
    if (CIP::IsIPV4(serverIp)) {
        conn.SetClientIpAddr(ipv4Addr);
    } else {
        mp_uint32 ipVecLen = 4;
        conn.SetClientIpv6Addr(ipv6Addr, ipVecLen);
    }
    this->role = role;

#ifdef SUPPORT_SSL
    conn.SetSslCtx(psslCtx);
#endif
    // save connection to db
    return InsertBusiClientToDB();
}

mp_string BusinessClient::GetLisetenIP()
{
    mp_uint32 uAddr = conn.GetClientIpAddr();
    mp_uint16 uPort = conn.GetClientPort();
    if (uAddr == 0 || uPort == 0) {
        ERRLOG("BusinessClient haven't initail server information.");
        return "";
    }
    mp_string ipStr = conn.GetClientIpAddrStr();
    mp_string temp = CheckParamInvalidReplace(ipStr);
    mp_string listenIp;
    mp_int32 listenPort = 0;
    bool flag = GetNginxListenIP(listenIp, listenPort);
    if (flag == false) {
        ERRLOG("Get Nginx Listen ip Fail .");
        return "";
    }
    INFOLOG("Nginx listen ip is %s", listenIp.c_str());
    static const mp_int32 checkTimeOut = 200;
    mp_int32 iRet = CSocket::CheckHostLinkStatus(listenIp, ipStr, uPort, checkTimeOut);
    if (MP_SUCCESS != iRet) {
        ERRLOG("Check server %s:%u failed, iRet %d.", temp.c_str(), uPort, iRet);
        return "";
    }
    return listenIp;
}

mp_int32 BusinessClient::Connect()
{
    INFOLOG("Begin to establish businessclient connection.");
    mp_string listenIp = GetLisetenIP();
    if (listenIp.empty()) {
        return MP_FAILED;
    }
    conn.SetListenIp(listenIp);
    mp_int32 iRet = conn.Connect();
    if (iRet != MP_SUCCESS) {
        ERRLOG("BusinessClient connect remote server failed.");
        return iRet;
    }

    // begin to recve ebk app node message
    iRet = conn.StartRecvMsg();
    if (iRet != MP_SUCCESS) {
        conn.DisConnect();
        ERRLOG("BusinessClient start recv message failed.");
        return iRet;
    }
    return MP_SUCCESS;
}

mp_void BusinessClient::DisConnect()
{
    conn.DisConnect();
}

mp_void BusinessClient::Remove()
{
    DelBusiClientFromDB();
    conn.DisConnect();
}

mp_void BusinessClient::HeartBeat()
{
    DBGLOG("BusinessClient info:%d-%d.", conn.GetLinkState(), conn.CheckHBTimeOut());
    mp_string ipStr = conn.GetClientIpAddrStr();
    mp_uint16 uPort = conn.GetClientPort();
    if (conn.CheckHBTimeOut() == 1 || (conn.GetLinkState() == LINK_STATE_NO_LINKED) ||
        conn.GetSendMsgFlag() == MP_FALSE) {
        INFOLOG("Reconnect ip:%s, business:%d-%d.", ipStr.c_str(), conn.GetLinkState(), conn.CheckHBTimeOut());
        conn.DisConnect();

        // relink, ignor error
        INFOLOG("BusinessClient(%s:%d_%d) state:%d-%d.",
            ipStr.c_str(),
            uPort,
            conn.GetClientSocket(),
            conn.GetLinkState(),
            conn.CheckHBTimeOut());
        mp_int32 iRet = Connect();
        if (iRet != MP_SUCCESS) {
            ERRLOG("Connect and login business node failed, iRet %d.", iRet);
        } else {
            // update heartbeat time
            conn.UpdateHBTime();
        }
        return;
    }
    CDppMessage *reqBusiMsg = NULL;
    CDppMessage *rspBusiMsg = NULL;
    mp_int32 iRet = NewMsgPair(reqBusiMsg, rspBusiMsg, GetSeqNo());
    if (iRet != MP_SUCCESS) {
        return;
    }

    // message list only handle response message
    Json::Value hbMsg;
    hbMsg[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_HEARTBEATE;
    rspBusiMsg->InitMsgHead(MSG_DATA_TYPE_MANAGE, 0, 0);
    rspBusiMsg->SetLinkInfo(ipStr, conn.GetClientPort());
    rspBusiMsg->SetMsgSrc(ROLE_HOST_AGENT);
    rspBusiMsg->SetMsgTgt(role);
    rspBusiMsg->SetMsgBody(hbMsg);

    message_pair_t busiPair(*reqBusiMsg, *rspBusiMsg);
    MessageHandler::GetInstance().PushRspMsg(busiPair);
    DBGLOG("Client(%s:%d) heartbeat.", ipStr.c_str(), conn.GetClientPort());
}

CConnection &BusinessClient::GetConnection()
{
    return conn;
}

MESSAGE_ROLE BusinessClient::GetRole()
{
    return this->role;
}

mp_uint64 BusinessClient::GetSeqNo()
{
    return this->seqNum++;
}

mp_socket BusinessClient::GetClientSocket()
{
    return conn.GetClientSocket();
}

mp_int32 BusinessClient::InsertBusiClientToDB()
{
    ostringstream buff;
    DbParamStream dps;
    DBReader readBuff;
    mp_string ipStr = conn.GetClientIpAddrStr();
    mp_uint16 uPort = conn.GetClientPort();
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;

    buff << "select count(*) from " << g_BusinessClient << " where " << g_BusiClientRole << " = " << role << " and "
         << g_BusiClientIP << " = '" << ipStr << "' and " << g_BusiClientPort << " = " << uPort;
    mp_string strSql = buff.str();

    mp_int32 iRet = CDB::GetInstance().QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query business clients failed, iRet %d.", iRet);
        return iRet;
    }
    mp_string temp = CheckParamInvalidReplace(ipStr);
    mp_int32 busiNum = 0;
    for (mp_int32 iRowCurr = 0; iRowCurr < iRowCount; ++iRowCurr) {
        mp_string strBusiNum;
        readBuff >> strBusiNum;
        busiNum = static_cast<mp_int32>(atoi(strBusiNum.c_str()));
    }

    if (busiNum >= 1) {
        DBGLOG("Business link info r=%d,ip=%s,port=%u is exists.", role, temp.c_str(), uPort);
        return MP_SUCCESS;
    }

    INFOLOG("Begin add business client info to db.");
    ostringstream insertBuff;
    insertBuff << "insert into " << g_BusinessClient << "(" << g_BusiClientRole << "," << g_BusiClientIP << ","
               << g_BusiClientPort << ") values(?,?,?);";
    strSql = insertBuff.str();

    DbParam dp = role;
    dps << std::move(dp);
    dp = ipStr;
    dps << std::move(dp);
    dp = uPort;
    dps << std::move(dp);

    iRet = CDB::GetInstance().ExecSql(strSql, dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Add business link info failed, iRet=%d.", iRet);
        return iRet;
    }

    DBGLOG("Add business link info role=%d,ip=%s,port=%u to db succs.", role, temp.c_str(), uPort);
    return iRet;
}

mp_void BusinessClient::DelBusiClientFromDB()
{
    COMMLOG(OS_LOG_INFO,
        "Begin to delete business client info from DB, %s:%u, role:%d.",
        conn.GetClientIpAddrStr().c_str(),
        conn.GetClientPort(),
        role);

    ostringstream buff;
    buff << "delete from " << g_BusinessClient << " where " << g_BusiClientRole << "== ? and " << g_BusiClientIP
         << " == ? and " << g_BusiClientPort << " == ?";
    mp_string sql = buff.str();

    DbParamStream dps;
    DbParam dp = role;
    dps << std::move(dp);
    dp = conn.GetClientIpAddrStr();
    dps << std::move(dp);
    dp = conn.GetClientPort();
    dps << std::move(dp);
    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    // don't return when delete from database failed
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN,
            "delete from business client failed, iRet=%d, %s:%u:%d.",
            iRet,
            conn.GetClientIpAddrStr().c_str(),
            conn.GetClientPort(),
            role);
    }
}

mp_int32 BusinessClient::NewMsgPair(CDppMessage *&reqMsg, CDppMessage *&rspMsg, mp_uint64 seqNo)
{
    NEW_CATCH_RETURN_FAILED(reqMsg, CDppMessage);
    NEW_CATCH(rspMsg, CDppMessage);
    if (!rspMsg) {
        delete reqMsg;
        COMMLOG(OS_LOG_ERROR, "New CDppMessage failed");
        return MP_FAILED;
    }

    reqMsg->SetOrgSeqNo(seqNo);
    rspMsg->SetOrgSeqNo(seqNo);
    return MP_SUCCESS;
}
