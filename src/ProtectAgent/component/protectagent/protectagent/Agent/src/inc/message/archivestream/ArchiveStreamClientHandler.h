/* *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file ArchiveStreamClientHandler.h
 * @brief  Contains function declarations for ArchiveStreamClientHandler
 * @version 1.0.0
 * @date 2021-05-17
 * @author lwx1045600
 */
#ifndef GET_FILE_CLIENT_HANDLER_H
#define GET_FILE_CLIENT_HANDLER_H

#include <vector>
#include <map>
#include "common/Types.h"
#include "common/Log.h"
#include "common/CMpThread.h"
#include "message/archivestream/ArchiveStreamClient.h"

class AGENT_API ArchiveStreamClientHandler {
public:

#ifdef WIN32
    static DWORD WINAPI RecvThread(mp_void *client);
    static DWORD WINAPI SendThread(mp_void *client);

#else
    static void *RecvThread(mp_void *client);
    static void *SendThread(mp_void *client);
#endif
    ArchiveStreamClientHandler();
    ~ArchiveStreamClientHandler();
    mp_int32 Init();
    mp_int32 Connect(const mp_string &busiIp, mp_uint16 busiPort, MESSAGE_ROLE role, bool usedSSL);
    mp_int32 Connect(mp_string &busiIp, mp_uint16 busiPort, bool openSsl);
    mp_int32 Connect();
    mp_int32 SwitchConnect();
    mp_int32 Disconnect();
    mp_int32 GetConnectState();
    mp_bool GetRecvExitFlag();
    mp_bool GetSendExitFlag();

    EXTER_ATTACK mp_void InitSsl();
    mp_bool InitVerityCert();

    mp_void SendMsq2Svr();
    mp_void RecvMsgFromSvr();
    mp_void SendDPMessage(const mp_string &taskId, CDppMessage *reqMsg);
    mp_bool GetResponseMessage(const mp_string &taskId, CDppMessage *&rspMsg, mp_uint64 seqNo, mp_uint32 timeout);
    mp_uint64 GetSeqNo();
    mp_string GetConnectedServerAddr();
    
private:
    mp_void PushMsg2Queue(CDppMessage *reqMsg);
    mp_int32 CreateThreadIfReconnect();
    mp_void EndThreadIfDisConnect();
    std::vector<mp_string> GetIP(mp_string &IPList);
    mp_void handleRecevMsg(CDppMessage *msg);

private:

    thread_id_t m_recvTid;
    thread_id_t m_sendTid;

    volatile mp_bool m_recvExitFlag;
    volatile mp_bool m_sendExitFlag;

#ifdef SUPPORT_SSL
    SSL_CTX *m_pSslCtx;
#endif
    mp_bool m_secureCh;

    ArchiveStreamClient m_client;

    thread_lock_t m_reqMsgMutext;
    thread_lock_t m_rspMsgMutext;

    std::vector<CDppMessage *> m_requestMsgs;                       // to send message list
    std::map<mp_string, std::vector<CDppMessage *> > m_responseMsgs; // have received message list
    typedef std::map<mp_string, std::vector<CDppMessage *> >::iterator RspMsgIter;

    std::vector<mp_string> m_serverList;
    mp_string m_currentServerIP;
    mp_uint16 m_busiPort;
    mp_bool m_OpenSSl;
    mp_bool m_inited;
    mp_bool m_dissconnected;
};

#endif
