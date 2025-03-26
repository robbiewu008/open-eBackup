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
#ifndef APPPLUGINS_VIRTUALIZATION_NUTANIXCLIENT_H
#define APPPLUGINS_VIRTUALIZATION_NUTANIXCLIENT_H

#include "log/Log.h"
#include "common/client/RestClient.h"
#include "common/cert_mgr/CertMgr.h"
#include "common/Structs.h"
#include "curl_http/HttpStatus.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "protect_engines/nutanix/api/client/NutanixSession.h"
#include "NutanixRetryCodeStruct.h"
#include "model/NutanixRequest.h"
#include "model/NutanixGetClusterListResponse.h"
#include "model/GetHostListRequest.h"
#include "model/NutanixGetVMListResponse.h"
#include "model/NutanixVMResponse.h"
#include "model/NutanixResponse.h"
#include "model/NutanixCreateSnapshotRequest.h"
#include "model/AttachDiskRequest.h"
#include "model/GetHostInfoRequest.h"
#include "model/GetSnapshotRequest.h"
#include "model/GetStorageContainerRequest.h"
#include "model/GetVirtualDiskInfoRequest.h"
#include "model/GetVMInfoRequest.h"
#include "model/QueryTaskRequest.h"
#include "model/NutanixResponse.h"
#include "model/NutanixRequest.h"
#include "model/DelSnapshotRequest.h"
#include "model/DetachDiskRequest.h"
#include "model/GetContainerResponse.h"
#include "model/GetNetworkResponse.h"
#include "model/DeleteVmRequest.h"

using VirtPlugin::RestClient;
using VirtPlugin::RequestInfo;
using VirtPlugin::SUCCESS;
using VirtPlugin::ResponseModel;
using VirtPlugin::CertManger;
using AppProtect::QueryByPage;
using AppProtect::ApplicationEnvironment;

namespace NutanixPlugin {
    struct NutanixErrorCodeS {
        int64_t code;
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMEBER(code)
        END_SERIAL_MEMEBER
    };

    struct NutanixErrorMsg {
        std::string message;
        std::string detailedMessage;
        struct NutanixErrorCodeS errorCode;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMEBER(message)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(detailedMessage, Detailed_message)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(errorCode, error_code)
        END_SERIAL_MEMEBER
    };

class NutanixClient : public RestClient {
public:
    explicit NutanixClient(Authentication auth);

    ~NutanixClient(void)
    {
        // 既然要session缓存，就不需要登出;
    };

    template<typename rspT, typename reqT>
    std::shared_ptr<rspT> ExecuteAPI(reqT &req)
    {
        if (!CheckParams(req)) {
            ERRLOG("action[%s]: Failed to check param. Ip: %s", typeid(reqT).name(), req.GetEndpoint().c_str());
            return nullptr;
        }
        INFOLOG("action[%s]: %s", typeid(reqT).name(), req.GetEndpoint().c_str());
        RequestInfo requestInfo;
        req.FillRequest(requestInfo);
        std::string errorDes;
        int64_t errorCode = 0;
        std::shared_ptr<rspT> response = std::make_shared<rspT>();
        if (response == nullptr) {
            ERRLOG("action[%s]: Failed to create response handler.", typeid(reqT).name());
            return nullptr;
        }
        if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
            ERRLOG("action[%s]: Failed to send request. errorCode: %d, errorDes: %s", typeid(reqT).name(),
                errorCode, errorDes.c_str());
            return nullptr;
        }
        if (response->GetStatusCode() == Module::SC_OK || response->GetStatusCode() == Module::SC_CREATED) {
            if (!response->Serial()) {
                ERRLOG("action[%s]: Serialize response body failed!", typeid(reqT).name());
                return nullptr;
            }
            INFOLOG("action[%s]: success!", typeid(reqT).name());
            return response;
        }
        ERRLOG("response status: %d", response->GetStatusCode());
        (void)ParseErrorBody(response);
        return nullptr;
    }
    int32_t Init(const ApplicationEnvironment& appEnv);
    NutanixErrorMsg GetErrorCode(void);
    int32_t CheckAuth(GetClusterListRequest &req, int64_t &errorCode, std::string &errorDes);

private:
    Authentication m_auth;
    std::string m_taskId;
    NutanixErrorMsg m_rspErrMsg;
    int32_t m_retryIntervalTime;
    int32_t m_retryTimes;

    bool CheckParams(ModelBase &model) override;
    bool SetSession(const NutanixRequest &request, RequestInfo &requestInfo);
    int32_t ParseErrorBody(const std::shared_ptr<ResponseModel> &response);
    void DelayTimeSendRequest();
    int32_t SendRequest(std::shared_ptr<ResponseModel> response, NutanixRequest &req, RequestInfo &requestInfo,
        std::string &errorDes, int64_t &errorCode);
    int32_t DoSendRequest(std::shared_ptr<ResponseModel> response, NutanixRequest &req, RequestInfo &requestInfo,
        std::string &errorDes, int64_t &errorCode);
    void RefreshSession(RequestInfo &requestInfo, const std::shared_ptr<ResponseModel> &response,
        NutanixRequest &req);
    void CleanSession(RequestInfo &requestInfo, NutanixRequest &req);
    bool IsPasswordAccess(RequestInfo &requestInfo);
};
}

#endif