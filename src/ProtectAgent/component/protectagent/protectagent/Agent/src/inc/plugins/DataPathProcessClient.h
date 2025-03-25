/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file DataPathProcessClient.h
 * @brief  Contains function declarations for DataPathProcessClient
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef __AGENT_DATAPATHPROCESS_CLIENT_H__
#define __AGENT_DATAPATHPROCESS_CLIENT_H__

#include <map>
#include <vector>
#include "common/JsonUtils.h"
#include "common/CMpThread.h"
#include "jsoncpp/include/json/value.h"
#include "jsoncpp/include/json/json.h"
#include "dataprocess/datamessage/DataMessage.h"

static const mp_int32 MOBILITY_TYPEOFSERVICE = 1;
static const mp_int32 VMWARENATIVE_TYPEOFSERVICE = 2;  // vmware native type backup
static const mp_uchar MAX_BUF_SIZE = 200;

class DataPathProcessClient {
public:
    DataPathProcessClient(mp_int32 serviceType, const mp_string &dpParam);
    ~DataPathProcessClient();
    mp_int32 Init();
    mp_int32 ResetConnection();
    mp_void SendDPMessage(const mp_string &taskId, CDppMessage *reqMsg, CDppMessage *&rspMsg, mp_uint32 timeout);
    // message sequence number
    mp_uint64 GetSeqNo();
private:
    volatile mp_uint64 seqNum;
    volatile mp_bool m_bWdExitFlag;
    volatile mp_bool m_bReqExitFlag;
    volatile mp_bool m_bRspExitFlag;
    mp_string m_strCurrentDppPid;
    mp_string m_strCurrentDppName;
    thread_id_t m_wdTid;      // watch dog thread
    thread_id_t m_reqTid;     // send message thread
    thread_id_t m_rspTid;     // recv message thread
    mp_int32 m_iServiceType;  // which executable to start
    mp_string m_dpParam;      // dataprocess start parameter
    mp_int32 m_iWdRetries;    // Count of DataProcess Restarted by Watchdog
    Json::Value m_InitParam;  // param that will sent to data process service

    std::vector<CDppMessage *> requestMsgs;                        // to send message list
    std::map<mp_string, std::vector<CDppMessage *> > responseMsgs;  // have received message list
    typedef std::map<mp_string, std::vector<CDppMessage *> >::iterator RspMsgIter;

#ifdef WIN32
    PROCESS_INFORMATION m_stProcessInfo;  // Handle for the data process
#else
    FILE *m_pStream;  // Handle for the data process
#endif

    DataMessage m_dataMsg;
    thread_lock_t reqMsgMutext;
    thread_lock_t rspMsgMutext;
    thread_lock_t seqNoMutext;

private:
#ifdef WIN32
    static DWORD WINAPI WatchDog(mp_void *client);
    static DWORD WINAPI StartSendMsgLoop(mp_void *client);
    static DWORD WINAPI StartRecvMsgLoop(mp_void *client);
#else
    static mp_void *WatchDog(mp_void *client);
    static mp_void *StartSendMsgLoop(mp_void *client);
    static mp_void *StartRecvMsgLoop(mp_void *client);
#endif

#ifndef WIN32
    mp_string GetProcessIDbyProcessName(const mp_string& strCurrentDataProcessName);
#endif
    mp_int32 InitializeClient();
    mp_int32 GetDataPathPort();
    mp_bool IsDataProcessStarted();
    mp_int32 StartLoopThreads();
    mp_void EndDataPathClient();
    mp_int32 StartCommonDataPathProcess();
    mp_void DoWatchDogProcess();
    mp_void SendMsgToDP();
    mp_void RecvMsgFromDP();
    mp_int32 SetUpClient(const mp_string& strIP, mp_uint16 iPort);
    mp_int32 EndDataProcessOnTimeout();
    mp_int32 EstablishClient();
    mp_void PushMsg2Queue(CDppMessage *reqMsg);
    mp_void DiscardReqMsgQueue();

    mp_int32 StartOMDataPathProcess();
    // VMware native data path process
    mp_int32 StartVMwareNativeDataPathProcess();
};
#endif