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
#include "AppService.h"
#include "PluginNasTypes.h"
#include "ApplicationServiceDataType.h"
#include "log/Log.h"
#include "PluginConstants.h"
#include "JsonTransUtil.h"
#include "ProtectPluginFactory.h"
#include "ErrorCode.h"
#include "utils/PluginUtilities.h"

using namespace AppProtect;
using namespace FilePlugin;

namespace {
    constexpr auto MODULE = "AppService";
    constexpr auto INTERNAL_ERROR = 200;
    constexpr auto HOMOVALUE = "HOMOGENEOUS";
    constexpr auto HETROVALUE = "HETEROGENEOUS";
    constexpr auto PROTOCOL_CIFS = "CIFS";
    constexpr int NUM_64 = 64;
    const std::string NFS_MOUNT_OPTION = "retry=0,timeo=3";
    constexpr int DEFAULT_PAGE_SIZE = 200;
}


namespace AppServiceExport {
    EXTER_ATTACK void DiscoverApplications(std::vector<Application>& returnValue, const std::string& appType)
    {
        returnValue;
        appType;
        return;
    }

    EXTER_ATTACK void CheckApplication(ActionResult& returnValue, const ApplicationEnvironment& appEnv,
        const Application& application)
    {
        return;
    }

    EXTER_ATTACK void ListApplicationResource(std::vector<ApplicationResource>& returnValue,
        const ApplicationEnvironment& appEnv, const Application& application,
        const ApplicationResource& parentResource)
    {
        return;
    }

    bool ConstructResourceParam(const ListResourceRequest& request, ListResourceParam& listResourceParam)
    {
        listResourceParam.applicationId = request.applications[0].id;
        listResourceParam.sharePath = request.applications[0].name;
        listResourceParam.pageNo = request.condition.pageNo;
        listResourceParam.pageSize = (request.condition.pageSize == 0) ?
                                     DEFAULT_PAGE_SIZE : request.condition.pageSize;
        listResourceParam.nasShareAuthInfo.authType = request.applications[0].auth.authType;
        listResourceParam.nasShareAuthInfo.authKey = request.applications[0].auth.authkey;
        listResourceParam.nasShareAuthInfo.authPwd = request.applications[0].auth.authPwd;
        if (!Module::JsonHelper::JsonStringToStruct(request.applications[0].extendInfo,
            listResourceParam.resourceExtendInfo)) {
            HCP_Log(ERR, MODULE) << "JsonStringToStruct failed,application.extendInfo:"<<
                                 request.applications[0].extendInfo << HCPENDLOG;
            return false;
        }
        listResourceParam.path = listResourceParam.resourceExtendInfo.directory;
        if (listResourceParam.nasShareAuthInfo.authType == KRB5AUTHMODE) {
            if (!Module::JsonHelper::JsonStringToStruct(request.applications[0].auth.extendInfo,
                listResourceParam.nasAuthExtentInfo)) {
                HCP_Log(ERR, MODULE) << "JsonStringToStruct failed."<< HCPENDLOG;
                return false;
            }
        }
        return true;
    }

    EXTER_ATTACK void ListApplicationResourceV2(ResourceResultByPage& returnValue, const ListResourceRequest& request)
    {
        if (request.applications.empty()) {
            HCP_Log(ERR, MODULE) << "List application resource failed,applications is empty" << HCPENDLOG;
            return;
        }
        Json::Value appEnvStr;
        StructToJson(request.appEnv, appEnvStr);
        HCP_Log(INFO, MODULE) << "ListApplicationResource, parameter:" <<
                              appEnvStr.toStyledString() << HCPENDLOG;
        Json::Value applicationStr;
        StructToJson(request.applications[0], applicationStr);
        HCP_Log(INFO, MODULE) << "ListApplicationResource, parameter:" <<
                              applicationStr.toStyledString() << HCPENDLOG;

        ListResourceParam listResourceParam;
        if (!ConstructResourceParam(request, listResourceParam)) {
            return;
        }
        PrintListResourceParam(listResourceParam);
        std::string resourceType = listResourceParam.resourceExtendInfo.protocol;
        auto startTime = std::chrono::steady_clock::now();
        auto appManagerPtr = ProtectPluginFactory::GetInstance().Create(resourceType);
        if (appManagerPtr != nullptr) {
            appManagerPtr->ListApplicationResource(returnValue, listResourceParam);
            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            HCP_Log(DEBUG, MODULE) << "ListApplicationResource, cost=" << duration.count() <<"ms" << HCPENDLOG;
            return;
        }
        HCP_Log(ERR, MODULE) << "List application resource failed,resourceType:"<< resourceType << HCPENDLOG;
        AppProtect::AppProtectPluginException appException;
        appException.__set_code(E_RESOURCE_TYPE_INVALID);
        appException.__set_message("invalid resourceType");
        throw appException;
    }

    void PrintListResourceParam(const ListResourceParam& listResourceParam)
    {
        HCP_Log(INFO, MODULE) << "PrintListResourceParam, parameter:" <<
                              "applicationId:"<<listResourceParam.applicationId <<
                              ",sharePath:"<< listResourceParam.sharePath <<
                              ",path:"<< listResourceParam.path <<
                              ",fileType:"<< listResourceParam.resourceExtendInfo.fileType <<
                              ",protocol:"<< listResourceParam.resourceExtendInfo.protocol <<
                              ",authType:"<< listResourceParam.nasShareAuthInfo.authType <<
                              ",pageNo:"<< listResourceParam.pageNo <<
                              ",pageSize:"<< listResourceParam.pageSize << HCPENDLOG;
    }
}

