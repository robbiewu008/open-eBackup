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
#include "PluginCertificatePathProxy.h"
#include "log/Log.h"
#include "PluginTypes.h"
#include "ExternalPluginSDK.h"
#include "PluginConfig.h"
#ifdef INTERNAL_PLUGIN_ENABLED
#include "curl_http/CurlHttpClient.h"
#endif


using namespace std;

namespace {
    constexpr auto MODULE = "PluginCertificatePathProxy";
    constexpr auto DOMAINNAME = "DataBackup-AGENT";
}

namespace certificateservice {
namespace detail {
std::string PluginCertificatePathProxy::GetCertificateRootPath()
{
    string result;
    auto ret = GetSecurityItemValue(static_cast<int>(SecurityItemConfig::CRETIFICATE_ROOT_PATH), result);
    if (ret != Module::SUCCESS || result.empty()) {
        result = DOMAINNAME;
    }
    return result;
}

// 优先获取配置中的文件名，获取不到返回默认值
std::string PluginCertificatePathProxy::GetCertificateFileName(CertificateType type)
{
    string result;
    int32_t ret = Module::FAILED;
    switch (type) {
        case CertificateType::USE_CERTIFICATE_FILE:
            ret = GetSecurityItemValue(static_cast<int>(SecurityItemConfig::USE_CRETIFICATE_FILE_NAME), result);
            break;
        case CertificateType::KEY_FILE:
            ret = GetSecurityItemValue(static_cast<int>(SecurityItemConfig::KEY_FILE_NAME), result);
            break;
        case CertificateType::TRUSTE_CERTIFICATE_FILE:
            ret = GetSecurityItemValue(static_cast<int>(SecurityItemConfig::TRUSTE_CRETIFICATE_FILE_NAME), result);
            break;
        default:
            break;
    }
    if (ret != Module::SUCCESS || result.empty()) {
        result = DOMAINNAME;
    }
    return result;
}

bool PluginCertificatePathProxy::GetCertificateConfig(CertificateConfig config, std::string& value)
{
    switch (config) {
        case CertificateConfig::PASSWORD:
            return GetPassword(value);
        case CertificateConfig::ALGORITHM_SUITE:
            return GetAlgorithmSuite(value);
        case CertificateConfig::HOST_NAME:
            return GetHostName(value);
        default:
            return false;
    }
}

bool PluginCertificatePathProxy::GetPassword(std::string& pw)
{
    std::string cipherStr;
    auto ret = GetSecurityItemValue(static_cast<int>(SecurityItemConfig::PASSWORD), cipherStr);
    if (ret != Module::SUCCESS) {
        return false;
    }
#ifdef INTERNAL_PLUGIN_ENABLED
    if (PluginConfig::GetInstance().m_scene != PluginUsageScene::EXTERNAL) {
        if (Module::g_kmcInstance->Decrypt(pw, cipherStr) != Module::SUCCESS) {
            ERRLOG("KMC Decrypt Passwd Failed!");
            return false;
        }
    } else {
        ERRLOG("this is external plugin");
        return false;
    }
#else
    if (PluginConfig::GetInstance().m_scene == PluginUsageScene::EXTERNAL) {
        if (Decrypt(cipherStr, pw) != Module::SUCCESS) {
            HCP_Log(ERR, MODULE) << "KMC Decrypt Passwd Failed!" << HCPENDLOG;
            return Module::FAILED;
        }
    } else {
        ERRLOG("kmc for interal sence is not ready");
        return false;
    }
#endif

    if (pw.empty()) {
        return false;
    }
    return true;
}

bool PluginCertificatePathProxy::GetAlgorithmSuite(std::string& suite)
{
    auto ret = GetSecurityItemValue(static_cast<int>(SecurityItemConfig::ALGORITEHM_SUITE), suite);
    if (ret != Module::SUCCESS || suite.empty()) {
        suite = DOMAINNAME;
    }
    return true;
}

bool PluginCertificatePathProxy::GetHostName(std::string& hostName)
{
    auto ret = GetSecurityItemValue(static_cast<int>(SecurityItemConfig::HOST_NAME), hostName);
    if (ret != Module::SUCCESS || hostName.empty()) {
        return false;
    }
    return true;
}
}
}
