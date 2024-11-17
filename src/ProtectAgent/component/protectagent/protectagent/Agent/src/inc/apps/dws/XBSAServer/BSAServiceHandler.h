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
#ifndef _BSA_SERVICE_HANDLER_H_
#define _BSA_SERVICE_HANDLER_H_

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/TNonblockingSSLServerSocket.h>
#include "apps/dws/XBSAServer/BSAService.h"
#include "apps/dws/XBSAServer/xbsa_types.h"
#include "common/Path.h"

class BSAServiceHandler : virtual public BSAServiceIf {
public:
    BSAServiceHandler() {};

    ~BSAServiceHandler() {};

    void HeartBeat(HeartBeatResult& _return);

    void BSABeginTxn(CallResult &_return, const int64_t handle);

    void BSACreateObject(CreateObjectResult &_return, const int64_t handle,
        const BsaObjectDescriptor &objectDescriptor);

    void BSADeleteObject(CallResult &_return, const int64_t handle, const BsaUInt64 &copyId);

    void BSAEndData(CallResult &_return, const int64_t handle, const BsaUInt64 &estimatedSize, const int64_t size);

    void BSAEndTxn(CallResult &_return, const int64_t handle, const int32_t vote);

    void BSAGetData(GetDataResult &_return, const int64_t handle, const BsaDataBlock32 &dataBlock);

    void BSAGetEnvironment(GetEnvironmentResult &_return, const int64_t handle, const BsaObjectOwner &objectOwner);

    void BSAGetLastError(GetLastErrorResult &_return, const int64_t bufferSize);

    void BSAGetNextQueryObject(GetNextQueryObjectResult &_return, const int64_t handle);

    void BSAGetObject(GetObjectResult &_return, const int64_t handle, const BsaObjectDescriptor &objectDesc);

    void BSAInit(BSAInitResult &_return, const BsaObjectOwner &objectOwner, const std::string &envPtr,
                const int32_t appType);

    void BSAQueryApiVersion(QueryApiVersionResult &_return);

    void BSAQueryObject(QueryObjectResult &_return, const int64_t handle, const BsaQueryDescriptor &queryDesc);

    void BSAQueryServiceProvider(QueryServiceProviderResult &_return, const int64_t retSize);

    void BSASendData(CallResult &_return, const int64_t handle, const BsaDataBlock32 &dataBlock);

    void BSATerminate(CallResult &_return, const int64_t handle);
};

#endif // _BSA_SERVICE_HANDLER_H_