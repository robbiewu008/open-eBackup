#include "dataprocess/datapath/DataPath.h"
#include <chrono>
#include <iostream>
#ifndef WIN32
#include <pthread.h>
#endif
#include "common/CMpThread.h"
#include "common/Types.h"
#include "common/TimeOut.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "message/tcp/CSocket.h"
#include "dataprocess/datamessage/DataMessage.h"
#include "dataprocess/dataconfig/DataConfig.h"
#include "dataprocess/datareadwrite/DataStream.h"

std::mutex DataPath::m_msgMutex;

DataPath::DataPath(mp_int32 iType, const mp_string &dpParam) : serviceType(iType), dpParam(dpParam)
{
    serviceType = iType;
    sendExitFlag = MP_FALSE;
    (mp_void)memset_s(&handlerTid, sizeof(handlerTid), 0, sizeof(handlerTid));
    responseFlag = MP_FALSE;
    sendThread = std::thread(&DataPath::ResponseMsg, this);
}

DataPath::~DataPath()
{
    sendExitFlag = MP_TRUE;
    if (handlerTid.os_id != 0) {
        CMpThread::WaitForEnd(&handlerTid, NULL);
    }
    responseFlag = MP_TRUE;
    sendThread.join();
}

mp_int32 DataPath::ExtCmdProcess(CDppMessage &message)
{
    mp_uint32 cmdType = message.GetManageCmd();
    if (cmdType == EXT_CMD_CLOSE) {
        sendExitFlag = MP_TRUE;
        INFOLOG("Dataprocess quit.");
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_bool DataPath::GetSendExitFlag()
{
    return sendExitFlag;
}

mp_int32 DataPath::DataInit(mp_socket sockFd, mp_uint16 Port, const mp_string& Ip)
{
    mp_int32 iRet = MP_SUCCESS;
    iRet = dataServer.Init(sockFd, Ip, Port);
    return (iRet);
}

mp_int32 DataPath::RecvMsgProcess(CConnection &conn)
{
    mp_int32 iRet = MP_SUCCESS;
    CDppMessage *msg = NULL;
    iRet = dataServer.StartReceiveDpp();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to receive message, iRet=%d", iRet);
        dataServer.recvdMsg = NULL;
        if (conn.GetLinkState() == LINK_STATE_NO_LINKED) {
            conn.ReleaseMsg();
            return iRet;
        } else {
            INFOLOG("Link is already connected, continue receive.");
            return MP_SUCCESS;
        }
    }

    msg = dataServer.recvdMsg;
    if (msg) {
        iRet = ExtCmdProcess(*msg);
        if (iRet != MP_SUCCESS) {
            INFOLOG("Failed to process message, iRet=%d", iRet);
        }
        delete dataServer.recvdMsg;
        dataServer.recvdMsg = NULL;
        // dispath message failed, continue handle, don't exit data process
        return MP_SUCCESS;
    } else {
        ERRLOG("Receive null message.");
    }

    return MP_FAILED;
}

mp_int32 DataPath::AcceptConnection(mp_socket serverFd)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_socket clientSock = MP_INVALID_SOCKET;
    mp_uint16 clientPort = 0;
    mp_uint32 clientIp = 0;

    CConnection &conn = dataServer.GetConnection();
    while (!sendExitFlag) {
        if (clientSock == MP_INVALID_SOCKET) {
            INFOLOG("Waiting for Connection to be accepted....");
            iRet = CSocket::AcceptClient(serverFd, clientIp, clientPort, clientSock);
            if (iRet != MP_SUCCESS) {
                ERRLOG("Connection accept failed, iRet %d.", iRet);
                break;
            }
            conn.SetSocket(clientSock);
            conn.SetConnectionRole(ConnectRole::SERVER);
            iRet = conn.InitMsgHead();
            if (iRet != MP_SUCCESS) {
                ERRLOG("Init message header failed, iRet %d.", iRet);
                break;
            }
            INFOLOG("Connection accept done");
        } else {
            INFOLOG("Already Connected");
        }

        if (clientSock) {
            iRet = RecvMsgProcess(conn);
            if (iRet != MP_SUCCESS) {
                ERRLOG("RecvMsgProcess failed, close socket %d, wait for new client.", clientSock);
                conn.DisConnect();
                clientSock = MP_INVALID_SOCKET;
                conn.SetClientSocket(MP_INVALID_SOCKET);
            }
        }
    }

    sendExitFlag = MP_TRUE;
    return iRet;
}

#ifdef WIN32
DWORD WINAPI DataPath::ExtCmdHandler(mp_void *arg)
#else
void *DataPath::ExtCmdHandler(mp_void *arg)
#endif
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_socket serverFd = 0;
    mp_string ip = LOCAL_IPADDR;
    mp_uint16 port = 0;

    DataPath *handler = static_cast<DataPath *>(arg);
    if (handler == NULL) {
        goto clean;
    }
    iRet = handler->dataServer.Init(serverFd, ip, port);
    if (iRet != MP_SUCCESS) {
        ERRLOG("datapath init failed.");
        goto clean;
    }

    iRet = handler->dataServer.CreateRandomPortServer(serverFd, ip, handler->GetServiceType(), handler->GetDpParam());
    if (iRet != MP_SUCCESS) {
        ERRLOG("datapath CreateRandomPortServer failed.");
        goto clean;
    }

    INFOLOG("External Command handler Thread created.");
    iRet = handler->AcceptConnection(serverFd);
    if (iRet != MP_SUCCESS) {
        ERRLOG("datapath AcceptConnection failed.");
        goto clean;
    }

    CSocket::Close(serverFd);
clean:
#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

mp_int32 DataPath::StartExtCmdHandler()
{
    INFOLOG("Start External command handler thread.");
    mp_int32 iRet = MP_SUCCESS;

    iRet = CMpThread::Create(&handlerTid, DataPath::ExtCmdHandler, this);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init External handler thread failed, ret %d.", iRet);
        return iRet;
    }

    return (iRet);
}

mp_int32 DataPath::GetServiceType()
{
    return serviceType;
}

mp_string DataPath::GetDpParam()
{
    return dpParam;
}

mp_void DataPath::PushMsg2Queue(std::unique_ptr<CDppMessage> &msg)
{
    std::lock_guard<std::mutex> lock(m_msgMutex);
    m_msgQueue.push_back(std::move(msg));
}

mp_void DataPath::ResponseMsg()
{
    DBGLOG("Start data path response msg.");
    constexpr mp_int32 responseWaitTime = 100;
    while (responseFlag == MP_FALSE) {
        CDppMessage *msg = NULL;
        // get send message
        {
            std::lock_guard<std::mutex> lock(m_msgMutex);
            if (m_msgQueue.size() > 0) {
                auto it = m_msgQueue.begin();
                msg = (*it).release();
                m_msgQueue.erase(it);
            }
        }

        // send message
        if (msg != NULL) {
            mp_int32 iRet = dataServer.SendDpp(*msg);
            if (iRet != MP_SUCCESS) {
                ERRLOG("Unable to send DPP message to host agent, cmd=0x%x, seq=%llu.",
                    msg->GetManageCmd(),
                    msg->GetOrgSeqNo());
            } else {
                INFOLOG("Sent DPP message to host agent successfully, cmd=0x%x, seq=%llu.",
                    msg->GetManageCmd(),
                    msg->GetOrgSeqNo());
            }
            delete msg;
        }

        // waitting for next message
        SleepForMS(responseWaitTime);
    }
    DBGLOG("End data path response msg.");
}
