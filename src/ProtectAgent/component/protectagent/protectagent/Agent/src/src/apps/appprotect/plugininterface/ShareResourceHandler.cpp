/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ExternalPluginParse.h
 * @brief  The implemention about ExternalPluginParse.h
 * @version 1.1.0
 * @date 2021-11-8
 * @author jwx966562
 */

#include "apps/appprotect/plugininterface/ShareResourceHandler.h"
#include "common/Log.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "pluginfx/ExternalPluginManager.h"
#include "message/rest/interfaces.h"
#include "message/curlclient/RestClientCommon.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "common/ErrorCode.h"
#include "host/host.h"

namespace AppProtect {
ShareResourceHandler::ShareResourceHandler()
{
    m_nodeId = GetNodeId();
}

void ShareResourceHandler::CommonSendRequestHandle(ActionResult& _return, const DmeRestClient::HttpReqParam& req)
{
    HttpResponse response;
    mp_int32 ret = DmeRestClient::GetInstance()->SendRequest(req, response);
    if ((ret == MP_SUCCESS) && (response.statusCode == SC_OK)) {
        _return.code = MP_SUCCESS;
        return;
    }
    ERRLOG("Send req or service handle with url(%s) to dme failed, ret=%d, statusCode=%d.",
        req.url.c_str(), ret, response.statusCode);
    _return.code = RPC_ACTION_EXECUTIVE_INTERNAL_ERROR;
    RestClientCommon::RspMsg rspSt;
    ret = RestClientCommon::ConvertStrToRspMsg(response.body, rspSt);
    if (ret != MP_SUCCESS) {
        ERRLOG("Parse rsp failed.");
        return;
    }

    if (rspSt.errorCode != "0") {
        _return.bodyErr = atoi(rspSt.errorCode.c_str());
        _return.message = rspSt.errorMessage;
    }
}

void ShareResourceHandler::SendRequestToPMHandle(ActionResult& _return, HttpReqCommonParam& req)
{
    DBGLOG("Start send reuqest to pm...");
    HttpResponse response;
    mp_int32 ret = PmRestClient::GetInstance().SendRequest(req, response);
    if ((ret == MP_SUCCESS) && (response.statusCode == SC_OK)) {
        _return.code = MP_SUCCESS;
        _return.__set_message(response.body);
        DBGLOG("Send reuqest to pm success, response is [%s]", response.body.c_str());
        return;
    }
    ERRLOG("Send req or service handle with url(%s) to pm failed, ret=%d, statusCode=%d.",
        req.url.c_str(), ret, response.statusCode);
    _return.code = RPC_ACTION_EXECUTIVE_INTERNAL_ERROR;
    RestClientCommon::RspMsg rspSt;
    ret = RestClientCommon::ConvertStrToRspMsg(response.body, rspSt);
    if (ret != MP_SUCCESS) {
        ERRLOG("Parse rsp failed.");
        return;
    }

    if (rspSt.errorCode != "0") {
        _return.bodyErr = atoi(rspSt.errorCode.c_str());
        _return.__set_message(rspSt.errorMessage);
    }
}
mp_int32 ShareResourceHandler::SwitchResourceToJson(const Resource& resource, std::string &output)
{
    Json::Value body;
    body["scope"] = resource.scope;
    mp_string tempScopeKey = resource.scopeKey;
    if (AssignScopeKey(resource.scope, tempScopeKey) != MP_SUCCESS) {
        ERRLOG("Node id is empty.");
        return RPC_ACTION_EXECUTIVE_INTERNAL_ERROR;
    }
    body["scopeKey"] = tempScopeKey;
    body["resourceKey"] = resource.resourceKey;
    body["resourceValue"] = resource.resourceValue;
    body["sharedNum"] = resource.sharedNum;
    output = body.toStyledString();
    return MP_SUCCESS;
}

EXTER_ATTACK void ShareResourceHandler::CreateResource(
    ActionResult& _return, const Resource& resource, const std::string& mainJobId)
{
    PrintLog(__FUNCTION__, resource);
    DmeRestClient::HttpReqParam req;
    req.method = REST_URL_METHOD_POST;
    req.url = "/v1/dme-unified/shared-resources";
    _return.code = SwitchResourceToJson(resource, req.body);
    if (_return.code != MP_SUCCESS) {
        ERRLOG("Fail to create resource.");
        return;
    }
    req.mainJobId = mainJobId;
    CommonSendRequestHandle(_return, req);
}

mp_int32 ShareResourceHandler::AssignScopeKey(ResourceScope::type scope, std::string &scopeKey)
{
    if (scope == ResourceScope::type::SINGLE_NODE) {
        if (m_nodeId.empty()) {
            return MP_FAILED;
        }
        scopeKey = m_nodeId;
    }
    return MP_SUCCESS;
}

mp_string ShareResourceHandler::GetNodeId()
{
    CHost host;
    mp_string strSN;
    mp_int32 iRet = host.GetHostSN(strSN);
    if (iRet != MP_SUCCESS) {
        ERRLOG("GetHostSN failed, iRet %d.", iRet);
        return mp_string("");
    }
    return strSN;
}

EXTER_ATTACK void ShareResourceHandler::QueryResource(
    ResourceStatus& _return, const Resource& resource, const std::string& mainJobId)
{
    PrintLog(__FUNCTION__, resource);
    DmeRestClient::HttpReqParam req;
    req.method = REST_URL_METHOD_GET;
    mp_string tempScopeKey = resource.resourceKey;
    if (AssignScopeKey(resource.scope, tempScopeKey) != MP_SUCCESS) {
        ERRLOG("Fail to query resource for empty node id.");
        ThrowAppProtectException(MP_FAILED, "Fail to get node id.");
    }

    req.url = "/v1/dme-unified/shared-resources/" + resource.resourceKey + "?scope=" + std::to_string(resource.scope)
        + "&scopeKey=" + tempScopeKey;
    req.mainJobId = mainJobId;
    HttpResponse response;
    mp_int32 ret = DmeRestClient::GetInstance()->SendRequest(req, response);
    if (ret != MP_SUCCESS) {
        ERRLOG("Send Resource(%s) to dme failed.", resource.resourceKey.c_str());
        ThrowAppProtectException(ret, "Fail to access to dme. error code:" + std::to_string(ret));
    }

    Json::Value value;
    if (CJsonUtils::ConvertStringtoJson(response.body, value) != MP_SUCCESS) {
        ERRLOG("Convert resource to json value failed.");
        ThrowAppProtectException(MP_FAILED, "Quired resource message is illegal. error code:" + std::to_string(ret));
        return;
    }
    if (value.isObject() && value.isMember("errorCode") && value["errorCode"].isString()) {
        std::string errCodeStr = value["errorCode"].asString();
        ERRLOG("Rsp body have error code: %s.", errCodeStr.c_str());
        mp_int32 erroCode;
        try {
            erroCode = std::stoi(errCodeStr);
        } catch (const std::exception& erro) {
            ERRLOG("Invalid errCodeStr , erro: %s.", erro.what());
            erroCode = MP_FAILED;
        }
        ThrowAppProtectException(erroCode, "Quired resource message is illegal. error code:" + errCodeStr);
        return;
    }
    HandleQueryResourceResponse(_return, value);
}

void ShareResourceHandler::HandleQueryResourceResponse(ResourceStatus& _return, const Json::Value& value)
{
    mp_int32 ret = MP_FAILED;
    if (!value.isObject() || !value.isMember("resource") || !CheckResourceJsonValid(value["resource"])) {
        ERRLOG("Quired resource message is illegal");
        ThrowAppProtectException(MP_FAILED, "Quired resource message is illegal. error code:" + std::to_string(ret));
        return;
    }
    if (value.isMember("lockNum") && value["lockNum"].isInt()) {
        _return.lockNum = value["lockNum"].asInt();
    }

    int tempScope = -1;
    if (value["resource"]["scope"].isInt()) {
        tempScope = value["resource"]["scope"].asInt();
    }
    if ((tempScope > ResourceScope::type::SINGLE_JOB || tempScope < ResourceScope::type::ENTIRE_SYSTEM)) {
        ERRLOG("Scope is illegal : %d", tempScope);
        ThrowAppProtectException(MP_FAILED, "Quired resource message is illegal. error code:" + std::to_string(ret));
        return;
    }
    _return.resource.scope = (ResourceScope::type)tempScope;
    if (value["resource"]["scopeKey"].isString()) {
        _return.resource.scopeKey = value["resource"]["scopeKey"].asString();
    }
    if (value["resource"]["resourceKey"].isString()) {
        _return.resource.resourceKey = value["resource"]["resourceKey"].asString();
    }
    if (value["resource"]["resourceValue"].isString()) {
        _return.resource.resourceValue = value["resource"]["resourceValue"].asString();
    }
    if (value["resource"].isMember("sharedNum") && value["resource"]["sharedNum"].isInt()) {
        _return.resource.sharedNum = value["resource"]["sharedNum"].asInt();
    }
}

bool ShareResourceHandler::CheckResourceJsonValid(const Json::Value& resourceInfo)
{
    if (!resourceInfo.isObject()) {
        return false;
    }
    if (!resourceInfo.isMember("scope") || !resourceInfo.isMember("scopeKey") ||
        !resourceInfo.isMember("resourceKey") || !resourceInfo.isMember("resourceValue")) {
        return false;
    }

    return true;
}

EXTER_ATTACK void ShareResourceHandler::UpdateResource(
    ActionResult& _return, const Resource& resource, const std::string& mainJobId)
{
    PrintLog(__FUNCTION__, resource);
    DmeRestClient::HttpReqParam req;
    req.method = REST_URL_METHOD_PUT;
    req.url = "/v1/dme-unified/shared-resources/update";
    _return.code = SwitchResourceToJson(resource, req.body);
    if (_return.code != MP_SUCCESS) {
        ERRLOG("Fail to create resource.");
        return;
    }
    req.mainJobId = mainJobId;
    CommonSendRequestHandle(_return, req);
}

EXTER_ATTACK void ShareResourceHandler::DeleteResource(
    ActionResult& _return, const Resource& resource, const std::string& mainJobId)
{
    PrintLog(__FUNCTION__, resource);
    DmeRestClient::HttpReqParam req;
    req.method = REST_URL_METHOD_DELETE;
    mp_string tempScopeKey = resource.resourceKey;
    if (AssignScopeKey(resource.scope, tempScopeKey) != MP_SUCCESS) {
        ERRLOG("Fail to delete resource for empty node id.");
        _return.code = RPC_ACTION_EXECUTIVE_INTERNAL_ERROR;
        return;
    }
    req.url = "/v1/dme-unified/shared-resources/" + resource.resourceKey + "?scope=" + std::to_string(resource.scope)
        + "&scopeKey=" + tempScopeKey;
    req.mainJobId = mainJobId;
    CommonSendRequestHandle(_return, req);
}

EXTER_ATTACK void ShareResourceHandler::LockResource(
    ActionResult& _return, const Resource& resource, const std::string& mainJobId)
{
    PrintLog(__FUNCTION__, resource);
    DmeRestClient::HttpReqParam req;
    req.method = REST_URL_METHOD_PUT;
    req.url = "/v1/dme-unified/shared-resources/lock";
    req.nTimeOut = HTTP_TIME_OUT_ONE_MIN;
    _return.code = SwitchResourceToJson(resource, req.body);
    if (_return.code != MP_SUCCESS) {
        ERRLOG("Fail to create resource.");
        return;
    }
    req.mainJobId = mainJobId;
    CommonSendRequestHandle(_return, req);
}

EXTER_ATTACK void ShareResourceHandler::UnLockResource(
    ActionResult& _return, const Resource& resource, const std::string& mainJobId)
{
    PrintLog(__FUNCTION__, resource);
    DmeRestClient::HttpReqParam req;
    req.method = REST_URL_METHOD_PUT;
    req.url = "/v1/dme-unified/shared-resources/release";
    req.nTimeOut = HTTP_TIME_OUT_ONE_MIN;
    _return.code = SwitchResourceToJson(resource, req.body);
    if (_return.code != MP_SUCCESS) {
        ERRLOG("Fail to create resource.");
        return;
    }
    req.mainJobId = mainJobId;
    CommonSendRequestHandle(_return, req);
}

EXTER_ATTACK void ShareResourceHandler::LockJobResource(
    ActionResult& _return, const Resource& resource, const std::string& mainJobId)
{
    PrintLog(__FUNCTION__, resource);
    HttpReqCommonParam req;
    req.method = REST_URL_METHOD_POST;
    req.url = "/v1/internal/locks";
    req.nTimeOut = HTTP_TIME_OUT_ONE_MIN;

    Json::Value bodyResource;
    bodyResource["id"] = resource.resourceKey;
    DBGLOG("MainJobId is %s, resourceId is %s", mainJobId.c_str(), resource.resourceKey);
    if (resource.lockType.empty()) {
        _return.code = MP_FAILED;
        ERRLOG("The lock type is empty.");
        return;
    }
    bodyResource["lockType"] = resource.lockType;

    Json::Value body;
    body["requestId"] = mainJobId;
    body["lockId"] = mainJobId;
    body["resources"].append(bodyResource);
    req.body = body.toStyledString();
    DBGLOG("Req body is [%s]", req.body.c_str());

    SendRequestToPMHandle(_return, req);
}

void ShareResourceHandler::Update(std::shared_ptr<messageservice::RpcPublishEvent> event)
{
    m_processorName = "ShareResource";
    std::shared_ptr<servicecenter::IService> handler = shared_from_this();
    std::shared_ptr<ShareResourceHandler> srHandler = std::dynamic_pointer_cast<ShareResourceHandler>(handler);
    m_processor = std::make_shared<ShareResourceProcessor>(srHandler);

    if (event->GetThriftServer().get() == nullptr) {
        COMMLOG(OS_LOG_ERROR, "ShareResourceHandler receives a null event");
        return;
    }
    if (!event->GetThriftServer()->RegisterProcessor(m_processorName, m_processor)) {
        COMMLOG(OS_LOG_ERROR, "ShareResourceHandler register processor failed.");
    }
}

bool ShareResourceHandler::Initailize()
{
    std::shared_ptr<servicecenter::IService> handler = shared_from_this();
    std::shared_ptr<ShareResourceHandler> srHandler = std::dynamic_pointer_cast<ShareResourceHandler>(handler);
    ExternalPluginManager::GetInstance().RegisterObserver(messageservice::EVENT_TYPE::RPC_PUBLISH_TYPE, srHandler);
    return true;
}

bool ShareResourceHandler::Uninitailize()
{
    return true;
}

void ShareResourceHandler::ThrowAppProtectException(int32_t errCode, const std::string& message)
{
    AppProtectFrameworkException resException;
    resException.code = errCode;
    resException.message = message;
    throw resException;
}

void ShareResourceHandler::PrintLog(const std::string &funName, const Resource &resource)
{
    std::stringstream resourceStr;
    resourceStr << resource;
    INFOLOG("Receive %s.", funName.c_str());
}
}  // namespace AppProtect