#include "dataprocess/datamessage/DataMessage.h"

#include <sstream>

#include "common/CMpThread.h"
#include "common/ConfigXmlParse.h"
#include "common/ErrorCode.h"
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/TimeOut.h"
#include "common/Types.h"
#include "common/Utils.h"
#include "common/Ip.h"
#include "common/Path.h"
#include "message/tcp/CConnection.h"
#include "message/tcp/CDppMessage.h"

using std::ostringstream;
namespace {
const mp_int32 MAX_NAME_SIZE = 128;
const mp_int32 MAX_BUFF_SIZE = 256;
const mp_int32 SYSTEM_PREALLOCATED_PORT = 1024;
}  // namespace
DataMessage::DataMessage()
{
    recvdMsg = NULL;
}

DataMessage::~DataMessage()
{}

CConnection &DataMessage::GetConnection()
{
    return conn;
}

mp_int32 DataMessage::Init(mp_socket sock, const mp_string& ip, mp_uint16 port)
{
    mp_uint32 ipAddr;
    mp_int32 ret = MP_SUCCESS;

    ipAddr = inet_addr(ip.c_str());
    conn.SetClientSocket(sock);
    conn.SetClientIpAddr(ipAddr);
    conn.SetClientPort(port);
    return ret;
}

#ifdef WIN32
mp_int32 DataMessage::GetPid()
{
    return GetCurrentProcessId();
}
#else
mp_int32 DataMessage::GetPid()
{
    return getpid();
}
#endif

mp_int32 DataMessage::SetSockTimeOut(mp_int32 secondTimeout)
{
    if (secondTimeout == -1) {
        return MP_SUCCESS;
    }

    if (conn.GetClientSocket() == MP_INVALID_HANDLE) {
        return MP_SUCCESS;
    }

    struct timeval timeout = {secondTimeout, 0};
    if (setsockopt(conn.GetClientSocket(), SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        COMMLOG(OS_LOG_ERROR, "setsockopt socket(%d) failed.", conn.GetClientSocket());
        return MP_FAILED;
    } else {
        COMMLOG(OS_LOG_INFO, "setsockopt socket(%d) recv timeout %d succ.", conn.GetClientSocket(), secondTimeout);
        return MP_SUCCESS;
    }
}

mp_int32 DataMessage::CreateClient(mp_socket &clientSock)
{
    mp_int32 iRet = MP_SUCCESS;

    iRet = CSocket::CreateClientSocket(clientSock);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create client socket failed, ret: '%d'.", iRet);
        return iRet;
    }
    conn.SetClientSocket(clientSock);
    conn.SetSockNoBlock(MP_FALSE);
    DBGLOG("Create client success, ret: '%d'.", iRet);

    mp_string strPidFilePath = CPath::GetInstance().GetLogFilePath(DATAPROCESS_PID_FILE);
    mp_char fname[MAX_NAME_SIZE];
    memset_s(&fname, sizeof(fname), 0, sizeof(fname));

    mp_int32 pid = GetPid();
    iRet = sprintf_s(fname, sizeof(fname), "%s_%d", strPidFilePath.c_str(), pid);
    if (-1 == iRet) {
        DBGLOG("Exec function sprintf_s failure, ret: '%d'.", iRet);
        return iRet;
    }

    iRet = CSocket::Connect(clientSock, conn.GetClientIpAddr(), conn.GetClientPort());
    if (iRet != MP_SUCCESS) {
        ERRLOG("Connect server failed, ret: '%d', server addr '%s', port '%u'.",
            iRet,
            conn.GetClientIpAddrStr().c_str(),
            conn.GetClientPort());
        return iRet;
    }
    iRet = CSocket::SetFdCloexec(clientSock);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Set Fd failed, ret: '%d', server addr '%s', port '%u'.",
            iRet,
            conn.GetClientIpAddrStr().c_str(),
            conn.GetClientPort());
        return iRet;
    }
    DBGLOG("Connection client success, server addr '%s', port '%u', ret: '%d'.",
        conn.GetClientIpAddrStr().c_str(),
        conn.GetClientPort(),
        iRet);

    return iRet;
}

mp_uint16 DataMessage::GetRandomPort(mp_socket servSock, const mp_string& ip)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 attempts = 0;
    mp_int32 maxattempts = 100;
    mp_uint16 uiPort = 0;
    mp_uint16 upper = 59520;
    mp_uint16 lower = 59510;
    mp_bool portFound = MP_FALSE;

    for (attempts = 0; attempts < maxattempts; attempts++) {
        uiPort = (GetRandom() % (upper - lower)) + lower;
        if (uiPort <= SYSTEM_PREALLOCATED_PORT) {
            continue;
        }
        iRet = CSocket::Bind(servSock, ip, uiPort);
        if (MP_SUCCESS != iRet)
            continue;
        else {
            portFound = MP_TRUE;
            break;
        }
    }

    if (!portFound) {
        uiPort = 0;
        COMMLOG(OS_LOG_ERROR, "Failed to get random port number");
    }

    return uiPort;
}

// this function only generate preudo random number, do not use to handle secret msg
mp_int32 DataMessage::GetRandom()
{
    COMMLOG(OS_LOG_INFO, "Start to Get GetRandom num");
    mp_int32 iFd = 0;
    iFd = open("/dev/urandom", O_RDONLY);
    if (iFd < 0) {
        COMMLOG(OS_LOG_ERROR, "create random failed.");
        return MP_FAILED;
    }
    unsigned long result = 0;
    read(iFd, &result, sizeof(result));
    close(iFd);
    COMMLOG(OS_LOG_INFO, "GetRandom num(%d) success", result);
    return result;
}

mp_int32 DataMessage::CreateRandomPortServer(
    mp_socket &servSock, const mp_string& ip, mp_int32 serviceType, const mp_string &dpParam)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_uint16 uiPort = 0;

    iRet = CSocket::CreateTcpSocket(servSock);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create tcp socket failed, ret: '%d'.", iRet);
        return (iRet);
    }

    iRet = CSocket::SetReuseAddr(servSock);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Set resue addr failed, ret: '%d'.", iRet);
        return iRet;
    }

    uiPort = DataMessage::GetRandomPort(servSock, ip);
    if (uiPort == 0 || uiPort <= SYSTEM_PREALLOCATED_PORT) {
        ERRLOG("Invalid communication port '%d'", uiPort);
        return (MP_FAILED);
    }

    iRet = CSocket::StartListening(servSock);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Start listening failed, ret: '%d'.", iRet);
        return iRet;
    }

    mp_uint32 ipAddr = inet_addr(ip.c_str());
    conn.SetClientIpAddr(ipAddr);
    conn.SetClientPort(uiPort);
    COMMLOG(OS_LOG_DEBUG, "Create server socket success. ip: '%s' port: '%u', ret: '%d'.", ip.c_str(), uiPort, iRet);

    mp_string strPidFilePath = CPath::GetInstance().GetConfFilePath(DATAPROCESS_PID_FILE);
    ostringstream oss;
    oss << strPidFilePath << "_" << serviceType << "_" << dpParam;
    iRet = DataMessage::WritePort(oss.str(), uiPort);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    return iRet;
}

mp_void DataMessage::CloseDppConnection()
{
    COMMLOG(OS_LOG_DEBUG, "Close connection");
    CloseConnect(conn);
}

CConnection *DataMessage::GetConnByMessage()
{
    return &conn;
}

mp_int32 DataMessage::SendDpp(CDppMessage &message)
{
    CConnection *connection = GetConnByMessage();
    if (!connection) {
        ERRLOG("Can't find connection by ip '%u'.", message.GetIpAddr());
        return MP_FAILED;
    }

    mp_int32 iRet = message.ReinitMsgBody();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Reinit message buffer failed, ret: '%d'.", iRet);
        return iRet;
    }

    INFOLOG("dest addr:%s, port:%u, cmd=0x%x, seq=%llu.",
        connection->GetClientIpAddrStr().c_str(),
        connection->GetClientPort(),
        message.GetManageCmd(),
        message.GetOrgSeqNo());
    return SendMsg(message, *connection);
}

mp_int32 DataMessage::StartReceiveDpp()
{
    conn.SetReceivingState(STATE_RECV, MP_TRUE);
    conn.SetReceivingState(STATE_RECV_PART1, MP_TRUE);
    conn.GetControlStat()->uiBytesRecv = 0;
    conn.GetControlStat()->uiBytesToRecv = conn.GetDppMessage()->GetSize1();
    while (!recvdMsg) {
        mp_int32 iRet = RecvMsg(conn);
        if (iRet == MP_EAGAIN) {
            continue;
        }

        if (iRet != MP_SUCCESS) {
            ERRLOG("Receive message from client node failed.");
            return iRet;
        }
    }

    recvdMsg->SetLinkInfo(conn.GetClientIpAddrStr(), conn.GetClientPort());
    return MP_SUCCESS;
}

mp_int32 DataMessage::ConvertDppBody2Json(CDppMessage &message, Json::Value &bodyMsg)
{
    mp_string strBody = message.GetBuffer();
    mp_int32 iRet = CJsonUtils::ConvertStringtoJson(strBody, bodyMsg);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    if (!bodyMsg.isObject() || !bodyMsg.isMember(MANAGECMD_KEY_BODY)) {
        COMMLOG(OS_LOG_ERROR, "Message have no key '%s'.", MANAGECMD_KEY_BODY.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

    return MP_SUCCESS;
}

mp_int32 DataMessage::HandleRecvDppMessage(CDppMessage &message)
{
    recvdMsg = &message;
    DBGLOG("Received message, cmd=0x%x, len=%u, seq=%llu.",
        recvdMsg->GetManageCmd(),
        recvdMsg->GetSize2(),
        recvdMsg->GetOrgSeqNo());

    return MP_SUCCESS;
}

mp_int32 DataMessage::WritePort(const mp_string &file, mp_uint16 uiPort)
{
    FILE *fptr = NULL;

    fptr = fopen(file.c_str(), "w");
    if (!fptr) {
        mp_char szErr[MAX_BUFF_SIZE] = {0};
        mp_int32 iErr = 0;
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Failed to open file %s, errno[%d]:%s.",
            file.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    ChmodFile(file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    fprintf(fptr, "%hu", uiPort);
    fflush(fptr);
    fclose(fptr);

    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_DEBUG, "Write port '%u' to file '%s'.", uiPort, file.c_str());
    return (iRet);
}

mp_uint16 DataMessage::ReadPort(const mp_string &file)
{
    mp_int32 iErr = 0;
    FILE *fptr = NULL;

    fptr = fopen(file.c_str(), "r");
    if (!fptr) {
        mp_char szErr[MAX_BUFF_SIZE] = {0};
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Failed to open file %s, errno[%d]:%s.",
            file.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    mp_uint16 uiPort = 0;
    iErr = fscanf_s(fptr, "%hu", &uiPort);
    if (-1 == iErr) {
        COMMLOG(OS_LOG_DEBUG, "Exec function fscanf_s failure, errno: '%d'.", iErr);
        fclose(fptr);
        return MP_FAILED;
    }

    mp_char buff[MAX_BUFF_SIZE] = {0};
    fgets(buff, MAX_BUFF_SIZE, fptr);

    COMMLOG(OS_LOG_DEBUG, "Read port '%u' to file '%s'.", uiPort, file.c_str());

    fclose(fptr);
    return uiPort;
}
