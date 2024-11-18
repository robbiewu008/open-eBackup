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
#include "apps/dws/XBSAServer/BSAServiceHandler.h"
#include "apps/dws/XBSAServer/BsaSessionManager.h"
#include "apps/dws/XBSAServer/BsaIntfAdaptor.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

void BSAServiceHandler::HeartBeat(HeartBeatResult& _return)
{
    INFOLOG("BSAServiceHandler");
    _return.__set_strKey("day day up");
    _return.__set_response(BSA_RC_SUCCESS);
    return;
}

void BSAServiceHandler::BSABeginTxn(CallResult &_return, const int64_t handle)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld", handle);
    if (!BsaIntfAdaptor::HandleValid(handle)) {
        _return.response = BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_HANDLE);
    }
    
    _return.response = BsaSessionManager::GetInstance().BeginTxn(handle);
}

void BSAServiceHandler::BSACreateObject(CreateObjectResult& _return, const int64_t handle,
    const BsaObjectDescriptor& objectDescriptor)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld", handle);
    if (!BsaIntfAdaptor::HandleValid(handle)) {
        BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_HANDLE);
        _return.response = BSA_RC_INVALID_HANDLE;
        return;
    }

    BsaSessionManager::GetInstance().CreateObject(handle, objectDescriptor, _return);
}

void BSAServiceHandler::BSADeleteObject(CallResult &_return, const int64_t handle, const BsaUInt64& copyId)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld", handle);
    if (!BsaIntfAdaptor::HandleValid(handle)) {
        _return.response = BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_HANDLE);
    }
    
    _return.response = BsaSessionManager::GetInstance().DeleteObject(handle, copyId);
}

void BSAServiceHandler::BSAEndData(CallResult &_return, const int64_t handle, const BsaUInt64 &estimatedSize,
    const int64_t size)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld,size=%ld", handle, size);
    if (!BsaIntfAdaptor::HandleValid(handle)) {
        _return.response = BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_HANDLE);
    }
    
    _return.response = BsaSessionManager::GetInstance().EndData(handle, estimatedSize, size);
}

void BSAServiceHandler::BSAEndTxn(CallResult &_return, const int64_t handle, const int32_t vote)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld,vote=%d", handle, vote);
    if (!BsaIntfAdaptor::HandleValid(handle)) {
        _return.response = BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_HANDLE);
    }

    _return.response = BsaSessionManager::GetInstance().EndTxn(handle, vote);
}

void BSAServiceHandler::BSAGetData(GetDataResult& _return, const int64_t handle, const BsaDataBlock32& dataBlock)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld", handle);
    if (!BsaIntfAdaptor::HandleValid(handle)) {
        BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_HANDLE);
        _return.response = BSA_RC_INVALID_HANDLE;
        return;
    }
    
    BsaDataBlock32 block(dataBlock);
    _return.response = BsaSessionManager::GetInstance().GetData(handle, block);
}

void BSAServiceHandler::BSAGetEnvironment(GetEnvironmentResult& _return, const int64_t handle,
    const BsaObjectOwner& objectOwner)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld", handle);
    _return.environmentPtr = "BSA_DELIMITER=/;BSA_SERVICE_PROVIDER=";
    _return.environmentPtr.append(BsaSessionManager::GetInstance().GetProvider()).append(";");
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceHandler::BSAGetLastError(GetLastErrorResult& _return, const int64_t bufferSize)
{
    INFOLOG("BSAServiceHandler,bufferSize=%lld", bufferSize);
    mp_string strErr = BsaSessionManager::GetInstance().GetLastErr();

    _return.detailError = strErr;
    _return.bufferSize = strErr.length() + 1;
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceHandler::BSAGetNextQueryObject(GetNextQueryObjectResult& _return, const int64_t handle)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld", handle);
    if (!BsaIntfAdaptor::HandleValid(handle)) {
        BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_HANDLE);
        _return.response = BSA_RC_INVALID_HANDLE;
        return;
    }

    BsaSessionManager::GetInstance().GetNextObj(handle, _return);
}

void BSAServiceHandler::BSAGetObject(GetObjectResult& _return, const int64_t handle,
    const BsaObjectDescriptor& objectDesc)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld", handle);
    if (!BsaIntfAdaptor::HandleValid(handle)) {
        BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_HANDLE);
        _return.response = BSA_RC_INVALID_HANDLE;
        return;
    }
    BsaSessionManager::GetInstance().GetObject(handle, objectDesc, _return);
}

void BSAServiceHandler::BSAInit(BSAInitResult& _return, const BsaObjectOwner& objectOwner, const std::string& envPtr,
                                const int32_t appType)
{
    INFOLOG("BSAServiceHandler");
    BsaSessionManager::GetInstance().NewSession(_return, objectOwner, envPtr, appType);
}

void BSAServiceHandler::BSAQueryApiVersion(QueryApiVersionResult& _return)
{
    INFOLOG("BSAServiceHandler");
    _return.version.issue = 1;
    _return.version.version = 1;
    _return.version.level = 0;
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceHandler::BSAQueryObject(QueryObjectResult& _return, const int64_t handle,
    const BsaQueryDescriptor& queryDesc)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld", handle);
    if (!BsaIntfAdaptor::HandleValid(handle)) {
        BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_HANDLE);
        _return.response = BSA_RC_INVALID_HANDLE;
        return;
    }

    BsaSessionManager::GetInstance().QueryObject(handle, queryDesc, _return);
}

void BSAServiceHandler::BSAQueryServiceProvider(QueryServiceProviderResult& _return, const int64_t retSize)
{
    INFOLOG("BSAServiceHandler,retSize=%lld", retSize);
    _return.delimiter = "/";
    _return.providerPtr = BsaSessionManager::GetInstance().GetProvider();
    _return.retSize = _return.providerPtr.length() + 1;
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceHandler::BSASendData(CallResult &_return, const int64_t handle, const BsaDataBlock32& dataBlock)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld", handle);
    if (!BsaIntfAdaptor::HandleValid(handle)) {
        _return.response = BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_HANDLE);
    }
    
    _return.response = BsaSessionManager::GetInstance().SendData(handle, dataBlock);
}

void BSAServiceHandler::BSATerminate(CallResult &_return, const int64_t handle)
{
    INFOLOG("BSAServiceHandler,bsaHandle=%lld", handle);
    if (!BsaIntfAdaptor::HandleValid(handle)) {
        _return.response = BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_HANDLE);
    }

    _return.response = BsaSessionManager::GetInstance().CloseSession(handle);
}