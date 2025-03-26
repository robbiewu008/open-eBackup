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
#include "ApplicationServiceImp.h"
#include <vector>
#include "ApplicationProtectFramework_types.h"
#include "AppService.h"
#include "OpenLibMgr.h"
#include "AsyncListJob.h"
#include "JobExecution.h"
#include "PluginTypes.h"
#include "log/Log.h"
#include "param_checker/ParamChecker.h"
#include "JsonTransUtil.h"
#include "Utils.h"

using namespace AppProtect;

namespace {
    const auto MODULE = "ApplicationServiceImp";
    const auto LIST_RESOURCE_REQUEST = "ListResourceRequest";
    constexpr int MAX_ERR_MSG_LEN = 256;
    constexpr int CLEAR_CHAR = 0;
    constexpr int DEFAULT_ERROR_CODE = -1;
    constexpr int INVOKE_PLUGIN_SUCCESS = 0;
    constexpr int INVOKE_PLUGIN_FAILED = -1;
}

using DiscoverApplicationsFun = void(std::vector<Application>& returnValue, const std::string& appType);

using OracleCheckArchiveAreaFun = void(ActionResult& _return,
    const std::string& appType, const std::vector<AppProtect::OracleDBInfo>& dbInfoList);

using CheckApplicationFun = void(
    ActionResult& returnValue, const ApplicationEnvironment& appEnv, const Application& application);

using ListApplicationResourceFun = void(std::vector<ApplicationResource>& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application,
    const ApplicationResource& parentResource);

using ListApplicationResourceV2Fun = void(ResourceResultByPage& page, const ListResourceRequest& request);
using DiscoverHostClusterFun = void(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv);
using DiscoverAppClusterFun = void(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv,
        const Application& application);
using ListApplicationConfigFun = void(std::map<std::string, std::string>& resources, const std::string& script);
using RemoveProtectFun = void(
        ActionResult& returnValue, const ApplicationEnvironment& appEnv, const Application& application);
using FinalizeClearFun = void(ActionResult& returnValue, const ApplicationEnvironment& appEnv,
    const Application& application, const std::map<std::string, std::string>& extendInfo);

ApplicationServiceImp::ApplicationServiceImp()
{}

ApplicationServiceImp::~ApplicationServiceImp()
{}

EXTER_ATTACK void ApplicationServiceImp::DiscoverApplications(std::vector<Application>& returnValue,
    const std::string& appType)
{
    HCP_Log(INFO, MODULE) << "Enter Discoverapplications" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<DiscoverApplicationsFun>("DiscoverApplications");
    if (fun == nullptr) {
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get Discoverapplications function failed error" <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }
    fun(returnValue, appType);
}

EXTER_ATTACK void ApplicationServiceImp::CheckApplication(
    ActionResult& returnValue, const ApplicationEnvironment& appEnv, const Application& application)
{
    HCP_Log(INFO, MODULE) << "Enter CheckApplication" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<CheckApplicationFun>("CheckApplication");
    if (fun == nullptr) {
        returnValue.__set_code(INNER_ERROR);
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get CheckApplication function failed" <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }

    std::unordered_map<std::string, Json::Value> params = {
        {"ApplicationEnvironment", StructToJson(appEnv)},
        {"Application", StructToJson(application)},
    };
    if (!ParamCheck(params, returnValue)) {
        return;
    }
    fun(returnValue, appEnv, application);
}

EXTER_ATTACK void ApplicationServiceImp::ListApplicationResource(
    std::vector<ApplicationResource>& returnValue, const ApplicationEnvironment& appEnv,
    const Application& application, const ApplicationResource& parentResource)
{
    HCP_Log(INFO, MODULE) << "Enter ListApplicationResource" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<ListApplicationResourceFun>("ListApplicationResource");
    if (fun == nullptr) {
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get ListApplicationResource function failed" <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        AppProtectPluginException exception;
        exception.code = -1;
        exception.message = "Invoke ListApplicationResource error";
        throw exception;
        return;
    }

    std::unordered_map<std::string, Json::Value> params = {
        {"ApplicationEnvironment", StructToJson(appEnv)},
        {"Application", StructToJson(application)},
        {"ApplicationResource", StructToJson(parentResource)}
    };
    ParamCheck(params);

    fun(returnValue, appEnv, application, parentResource);
}

EXTER_ATTACK void ApplicationServiceImp::ListApplicationResourceV2(
    ResourceResultByPage& page, const ListResourceRequest& request)
{
    HCP_Log(INFO, MODULE) << "Enter ListApplicationResourceV2" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<ListApplicationResourceV2Fun>("ListApplicationResourceV2");
    if (fun == nullptr) {
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get ListApplicationResourceV2 function failed" <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        AppProtectFrameworkException exception;
        exception.code = -1;
        exception.message = "Invoke ListApplicationResourceV2 error";
        throw exception;
    }

    ParamCheck("ListResourceRequest", StructToJson(request));
    fun(page, request);
}

EXTER_ATTACK void ApplicationServiceImp::AsyncListApplicationResource(
    ActionResult& returnValue, const ListResourceRequest& request)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncListApplicationResourceV2." << HCPENDLOG;
    ParamCheck("ListResourceRequest", StructToJson(request));
    std::shared_ptr<BasicJob> jobPtr = std::make_shared<AsyncListJob>(request);
    if (jobPtr == nullptr || request.id.empty()) {
        HCP_Log(ERR, MODULE) << "Get AsyncListApplicationResourceV2 id:" <<
            request.id << " BasicJob failed" << HCPENDLOG;
        AppProtectFrameworkException exception;
        exception.code = -1;
        exception.message = "AsyncListApplicationResourceV2 BasicJob nullptr error";
        throw exception;
    }
    jobPtr->SetParentJobId(request.id);
    jobPtr->SetJobId(request.id);
    // 执行异步任务，创建线程，JobMgr纳入管理
    HCP_Log(DEBUG, MODULE) << "Enter excute job" << HCPENDLOG;
    common::jobmanager::JobExecution jobExecution;
    int ret = jobExecution.ExecuteJob(returnValue, jobPtr, request.id, OperType::RESOURCE);
    if (ret != Module::SUCCESS) {
        returnValue.__set_code(INVOKE_PLUGIN_FAILED);
        AppProtectFrameworkException exception;
        exception.code = -1;
        exception.message = "AsyncListApplicationResourceV2 BasicJob nullptr error";
        throw exception;
    }
    returnValue.__set_code(INVOKE_PLUGIN_SUCCESS);  // 0 成功 ； 500 失败
    HCP_Log(INFO, MODULE) << "Exit excute job success" HCPENDLOG;
    return;
}

EXTER_ATTACK void ApplicationServiceImp::DiscoverHostCluster(ApplicationEnvironment& returnEnv,
    const ApplicationEnvironment& appEnv)
{
    HCP_Log(INFO, MODULE) << "Enter DiscoverHostCluster" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<DiscoverHostClusterFun>("DiscoverHostCluster");
    if (fun == nullptr) {
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get DiscoverHostCluster function failed" <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        AppProtectFrameworkException exception;
        exception.code = -1;
        exception.message = "Invoke DiscoverHostCluster error";
        throw exception;
    }

    ParamCheck("ApplicationEnvironment", StructToJson(appEnv));
    fun(returnEnv, appEnv);
}

EXTER_ATTACK void ApplicationServiceImp::DiscoverAppCluster(ApplicationEnvironment& returnEnv,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    HCP_Log(INFO, MODULE) << "Enter DiscoverAppCluster" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<DiscoverAppClusterFun>("DiscoverAppCluster");
    if (fun == nullptr) {
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get DiscoverAppCluster function failed" <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        AppProtectFrameworkException exception;
        exception.code = -1;
        exception.message = "Invoke DiscoverAppCluster error";
        throw exception;
    }

    ParamCheck({{"ApplicationEnvironment", StructToJson(appEnv)}, {"Application", StructToJson(application)}});
    fun(returnEnv, appEnv, application);
}

EXTER_ATTACK void ApplicationServiceImp::ListApplicationConfig(
    std::map<std::string, std::string>& resources, const std::string& script)
{
    HCP_Log(INFO, MODULE) << "Enter ListApplicationConfig" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<ListApplicationConfigFun>("ListApplicationConfig");
    if (fun == nullptr) {
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get ListApplicationConfig function failed" <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        AppProtectPluginException exception;
        exception.code = -1;
        exception.message = "Invoke ListApplicationConfig error";
        throw exception;
        return;
    }
    fun(resources, script);
}

EXTER_ATTACK void ApplicationServiceImp::OracleCheckArchiveArea(ActionResult& _return,
    const std::string& appType, const std::vector<AppProtect::OracleDBInfo>& dbInfoList)
{
    HCP_Log(INFO, MODULE) << "Enter OracleCheckArchiveArea" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<OracleCheckArchiveAreaFun>("OracleCheckArchiveArea");
    if (fun == nullptr) {
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get OracleCheckArchiveArea function failed error" <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }

    ParamCheck("OracleDBInfo", StructToJson(dbInfoList), _return);
    fun(_return, appType, dbInfoList);
}

EXTER_ATTACK void ApplicationServiceImp::RemoveProtect(ActionResult& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    HCP_Log(INFO, MODULE) << "Enter RemoveProtect" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<RemoveProtectFun>("RemoveProtect");
    if (fun == nullptr) {
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get RemoveProtect function failed" <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        AppProtectPluginException exception;
        exception.code = -1;
        exception.message = "Invoke RemoveProtect error";
        throw exception;
    }

    ParamCheck({{"ApplicationEnvironment", StructToJson(appEnv)}, {"Application", StructToJson(application)}});
    fun(returnValue, appEnv, application);
}

EXTER_ATTACK void ApplicationServiceImp::FinalizeClear(ActionResult& _return, const ApplicationEnvironment& appEnv,
    const Application& application, const std::map<std::string, std::string>& extendInfo)
{
    HCP_Log(INFO, MODULE) << "Enter FinalizeClear" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<FinalizeClearFun>("FinalizeClear");
    if (fun == nullptr) {
        // 初始化字符串数组为空
        char errMsg[MAX_ERR_MSG_LEN] = {CLEAR_CHAR};
        HCP_Log(ERR, MODULE) << "Get FinalizeClear function failed error" <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        AppProtectPluginException exception;
        // 未实现对应的函数 将错误码置为默认错误码
        exception.code = DEFAULT_ERROR_CODE;
        exception.message = "Invoke FinalizeClear error";
        throw exception;
    }
    fun(_return, appEnv, application, extendInfo);
}
