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
#ifndef _REST_CLIENT_COMMON_H
#define _REST_CLIENT_COMMON_H
#include <sstream>
#include "common/Types.h"
#include "common/ErrorCode.h"
#include "message/curlclient/HttpClientInterface.h"
#include "message/rest/http_status.h"
struct HttpReqCommonParam {
    HttpReqCommonParam() : method(""), url(""), body("")
    {}
    HttpReqCommonParam(const mp_string& action, const mp_string& endpoint,
        const mp_string& content)
        : method(action), url(endpoint), body(content)
    {}
    mp_string method;  // POST/GET/PUT/DELETE
    mp_string url;     // 如/v1/dme-unified/shared-resources
    mp_string body;
    mp_uint32 nTimeOut = 0;
    mp_uint32 port = 0;
    std::vector<mp_string> vecIpList;
};

class RestClientCommon {
public:
    struct RspMsg {
        mp_string errorCode;
        mp_string errorMessage;
        std::vector<mp_string> detailParams;
        RspMsg()
        {
            std::stringstream ss;
            ss << ERROR_COMMON_INVALID_PARAM;
            ss >> errorCode;
        }
    };
public:
    /*
    * 功能：获取SSL证书相关信息
    * 入参：actionType: CA信息配置名称, 如CFG_CA_INFO
    * 返回值: 获取到的值
    */
    static mp_string SecurityConfiguration(const mp_string& actionType);
    /*
    * 功能：发送HTTP请求
    * 入参：req: 需要发送的HTTP请求体
    * 出参：responseBody： 返回的消息体
    */
    static mp_int32 Send(IHttpClient* httpClient, const HttpRequest& req, HttpResponse& httpResponse);
    /*
    * 功能：判断HTTP请求返回的状态码是否跟网络相关
    * 入参：statusCode：状态码
    */
    static bool IsNetworkError(mp_int32 statusCode);
    /*
    * 功能：当发送HTTP请求失败后，将DME返回的错误详情由string转成结构体
    */
    static mp_int32 ConvertStrToRspMsg(const std::string &rspstr, RspMsg &rspSt);
private:
    static std::map<mp_string, mp_string> m_secureConfigMap;
};
#endif