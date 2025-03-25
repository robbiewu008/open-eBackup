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
#ifndef THRIFT_CLIENT_H
#define THRIFT_CLIENT_H

#include "xbsaclientcomm/BSAService.h"
#include "xbsa/xbsa.h"
#include "File.h"
#include "common/Log.h"

#include <thrift/transport/THttpClient.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/TZlibTransport.h>
#include <thrift/async/TEvhttpClientChannel.h>
#include "TSSLSocketFactoryPassword.h"

#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <map>
#include <mutex>
#include <atomic>

enum protection_type {
    BACKUP = 0,
    RECOVERY = 1
};

struct ThriftClientInfo {
    std::shared_ptr<apache::thrift::transport::TTransport> m_transport;
    std::shared_ptr<BSAServiceClient> m_client;
};

class ThriftClientMgr {
public:
    static ThriftClientMgr& GetInstance()
    {
        static ThriftClientMgr m_Instance;
        return m_Instance;
    }
    ~ThriftClientMgr();
    void Init();
    /* thrift 透传接口 */
    int32_t BSABeginTxnMgr(long bsaHandle);
    int32_t BSACreateObjectMgr(
        long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr, BSA_DataBlock32 *dataBlockPtr);
    int32_t BSADeleteObjectMgr(long bsaHandle, BSA_UInt64 copyId);
    int32_t BSAEndDataMgr(long bsaHandle);
    int32_t BSAEndTxnMgr(long bsaHandle, BSA_Vote vote);
    int32_t BSAGetDataMgr(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);
    int32_t BSAGetEnvironmentMgr(long bsaHandle, BSA_ObjectOwner *objectOwner, char **ptr);
    int32_t BSAGetLastErrorMgr(BSA_UInt32 *sizePtr, char *errorCodePtr);
    int32_t BSAGetNextQueryObjectMgr(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr);
    int32_t BSAGetObjectMgr(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr, BSA_DataBlock32 *dataBlockPtr);
    int32_t BSAInitMgr(long *bsaHandlePtr, BSA_SecurityToken *tokenPtr,
                    BSA_ObjectOwner *objectOwnerPtr, char **environmentPtr, const int32_t appType);
    int32_t BSAQueryApiVersionMgr(BSA_ApiVersion *apiVersionPtr);
    int32_t BSAQueryObjectMgr(
        long bsaHandle, BSA_QueryDescriptor *queryDescriptorPtr, BSA_ObjectDescriptor *objectDescriptorPtr);
    int32_t BSAQueryServiceProviderMgr(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr);
    int32_t IifBSAQueryServiceProviderMgr(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr);
    int32_t BSASendDataMgr(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);
    int32_t BSATerminateMgr(long bsaHandle);

    /* 数据落盘 */
    int32_t SendData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);
    int32_t GetData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);
    void SetThriftRetryInterval(uint32_t interval);
    void SetThriftRetryTimes(int32_t times);

    bool IsInfomrix11();

private:
    ThriftClientMgr();
    int32_t InitModule();
    EXTER_ATTACK int32_t Init(bool ssl);
    std::shared_ptr<BSAServiceClient> GetClient();
    std::shared_ptr<apache::thrift::transport::TSocket> CreateSock(bool ssl);
    bool SetWorkingObj(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr);
    template<typename T>
    void SaveObjIntoReadMap(long bsaHandle, const T &objectDescriptor);
    bool GetEstimatedSize(long bsaHandle, BsaUInt64 &estimatedSize);
    int32_t GetInformixVersion();

    template<typename T, typename ... ARGS1, typename ... ARGS2>
    bool RetryThriftCall(std::shared_ptr<T> ptr, void (T::*MemFunc)(ARGS1 ...), long bsaHandle, ARGS2 &&... args);
    template<typename T, typename ... ARGS1, typename ... ARGS2>
    bool ProtectServiceCall(void (T::*MemFunc)(ARGS1...), long bsaHandle, ARGS2 &&... args);
    void CheckAppIsUsedSockFile();
private:
    int32_t m_thriftRetryTimes{600}; // thrift接口调用最大重试次数，方便LLT修改提升执行效率
    uint32_t m_thriftRetryInterval{1000*1000}; // thrift接口调用重试间隔时间，单位us,方便LLT修改提升执行效率
    std::unique_ptr<BSAServiceClient> m_serClient;
    File m_reader;  // 读数据
    File m_writer;  // 写数据
    std::mutex m_Mutex;              // 插入map时加锁
    std::map<long, BsaHandleContext> m_readMap;
    std::map<long, BsaHandleContext> m_writeMap;
    std::atomic<uint64_t> m_totalDataSize{0};
    std::shared_ptr<TSSLSocketFactoryPassword> m_socketFactory;
    int32_t m_appType = {0};
    bool m_isInformix11 = false;
    bool m_isUseSockFile = false;
};

#endif

