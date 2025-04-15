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
#ifndef REST_CLIENT_H
#define REST_CLIENT_H

#include <map>
#include <string>
#include "common/Constants.h"
#include "common/Macros.h"
#include "common/httpclient/HttpClient.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

const int32_t RETRY_GET_TOKEN_TIMES = 3;

class RestClient {
public:
    RestClient() {}
    virtual ~RestClient() {}
    // 系统管理员信息
    virtual bool CheckParams(ModelBase &model) = 0;
    int32_t CallApi(RequestInfo &requestInfo, std::shared_ptr<ResponseModel> response, ModelBase &model,
        bool isOpService = false);
    void SetRetryTimes(const int &retryTimes);

protected:
    int32_t m_retryTimes = RETRY_GET_TOKEN_TIMES;

protected:
    int32_t DoHttpRequestSync(Module::HttpRequest &request, std::shared_ptr<ResponseModel> response, ModelBase &model);
    std::string GetResourcePath(const std::string &uri, const std::map<std::string, std::string> &pathParams);
    std::string GetQueryParams(const std::map<std::string, std::string> &queryParams);
    void AddHeaderParams(Module::HttpRequest &request, const std::map<std::string, std::string> &headerParams);
    virtual bool UpdateToken(ModelBase &model, std::string &tokenStr)
    {
        return false;
    };
};

VIRT_PLUGIN_NAMESPACE_END

#endif