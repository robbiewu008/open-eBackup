/**
* Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
* @file RestClientCommon.h
* @brief Implement for RestClientCommon
* @version 1.1.0
* @date 2021-11-22
* @author jwx966562
*/

#include "message/curlclient/RestClientCommon.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "common/JsonUtils.h"

std::map<mp_string, mp_string> RestClientCommon::m_secureConfigMap;
mp_string RestClientCommon::SecurityConfiguration(const mp_string& actionType)
{
    if (m_secureConfigMap.find(actionType) != m_secureConfigMap.end()) {
        return m_secureConfigMap[actionType];
    }
    mp_string value("");
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, actionType, value);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get %s value.", actionType.c_str());
    }
    m_secureConfigMap[actionType] = value;
    return value;
}

mp_int32 RestClientCommon::Send(IHttpClient* httpClient, const HttpRequest& req, HttpResponse& httpResponse)
{
    IHttpResponse* dpaHttpRespone = httpClient->SendRequest(req, req.nTimeOut);
    DBGLOG("Method: %s\n url: %s\n", req.method.c_str(), req.url.c_str());
    if (dpaHttpRespone == nullptr) {
        ERRLOG("Curl http(url:%s) initialization response failed.", req.url.c_str());
        return MP_FAILED;
    }
    mp_int32 iRet = MP_SUCCESS;
    httpResponse.statusCode = dpaHttpRespone->GetHttpStatusCode();
    httpResponse.curlErrCode = dpaHttpRespone->GetErrCode();
    if (dpaHttpRespone->Success()) {
        // 发送成功，对方返回状态码200
        httpResponse.body = dpaHttpRespone->GetBody();
        iRet = MP_SUCCESS;
    } else if (!IsNetworkError(httpResponse.statusCode)) {
        // 发送成功，对方返回状态码非200
        WARNLOG("Curl http(url: %s, hostinfo: %s) server handle failed.Status code:%d.",
            req.url.c_str(), req.hostinfo.c_str(), httpResponse.statusCode);
        httpResponse.body = dpaHttpRespone->GetBody();
        iRet = MP_SUCCESS;
    } else {
        // 发送失败
        ERRLOG("Send rest(url: %s, hostinfo: %s) failed.ErrCode:%d, statusCode: %d.",
            req.url.c_str(), req.hostinfo.c_str(), httpResponse.curlErrCode, httpResponse.statusCode);
        iRet = MP_FAILED;
    }
    if (dpaHttpRespone) {
        delete dpaHttpRespone;
        dpaHttpRespone = nullptr;
    }
    return iRet;
}

bool RestClientCommon::IsNetworkError(mp_int32 statusCode)
{
    switch (statusCode) {
        case SC_BAD_GATEWAY:
        case SC_SERVICE_UNAVAILABLE:
        case SC_GATEWAY_TIMEOUT:
        case SC_INTERNAL_SERVER_ERROR:
        case 0: {
            return MP_TRUE;
        }
        default:
            return MP_FALSE;
    }
}

mp_int32 RestClientCommon::ConvertStrToRspMsg(const std::string &rspstr, RspMsg &rspSt)
{
    Json::Value value;
    if (CJsonUtils::ConvertStringtoJson(rspstr, value) != MP_SUCCESS) {
        ERRLOG("Convert %s to json value failed.", rspstr.c_str());
        return MP_FAILED;
    }
    if (value.type() != Json::objectValue) {
        rspSt.errorCode = std::to_string(ERR_INVALID_PARAM);
        rspSt.errorMessage = "The format of response message is illegal.";
        ERRLOG("Failed to parse param.");
        return MP_FAILED;
    }
    if (!value.isMember("errorCode") && !value["errorCode"].isString()) {
        ERRLOG("The format of response message is illegal.");
        return MP_FAILED;
    }
    rspSt.errorCode = value["errorCode"].asString();

    if (value.isMember("errorMessage") && value["errorMessage"].isString()) {
        rspSt.errorMessage = value["errorMessage"].asString();
    }

    if (value.isMember("detailParams")) {
        GET_ARRAY_STRING_WITHOUT_BRACES(value["detailParams"], rspSt.detailParams);
    }
    return MP_SUCCESS;
}
