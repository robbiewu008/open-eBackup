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
#ifndef CLOUD_SERVICE_ERROR_CODE_H
#define CLOUD_SERVICE_ERROR_CODE_H

#include <cstdint>
#include <string>
#include <map>
#include "common/CloudServiceCommonStruct.h"
#include "errno.h"
namespace Module {
constexpr int64_t OBS_INTERNAL_ERROR_CODE = 200;                                // 内部错误
constexpr int64_t ERROR_OBS_RESOURCE_ENDPOINT_CONNECT_FAILED = 1577213547;      // 备份软件与对象存储服务的Endpoint不连通
constexpr int64_t ERROR_OBS_RESOURCE_AK_OR_SK_ERROR = 1577213549;               // AK或SK填写不正确
constexpr int64_t ERROR_OBS_RESOURCE_PROXY_SERVER_CONNECT_FAILED = 1577213551;  // 连接代理服务器失败
constexpr int64_t ERROR_OBS_RESOURCE_CERT_VERIFY_FAILED = 1577213552;           // 证书验证失败
constexpr int64_t ERROR_OBS_RESOURCE_TIME_VERIFY_FAILED = 1577213553;           // 备份软件与对象存储环境时间不一致

class CloudServiceErrorCode {
public:
    static int64_t Transform2OMRP(const StorageType &storageType, const std::string &errorCode);
    static int64_t Transform2Linux(const StorageType &storageType, const std::string &errorCode);

public:
    static std::map<std::string, int64_t> m_errCodeALI;
    static std::map<std::string, int64_t> m_errCodeALI2Linux;

private:
    static int64_t GetFromHCS(const std::string &errorCode);
    static int64_t GetFromHCS2Linux(const std::string &errorCode);
    static int64_t GetFromALI(const std::string &errorCode);
    static int64_t GetFromALI2Linux(const std::string &errorCode);
};
}  // namespace Module

#endif  // CLOUD_SERVICE_ERROR_CODE_H
