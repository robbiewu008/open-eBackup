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
#include <certificateservice/detail/NginxCertificatePathProxy.h>
#include <certificateservice/include/ICertificateComm.h>
#include <map>
#include "common/Path.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "securecom/SecureUtils.h"
#include "securecom/CryptAlg.h"

namespace certificateservice {
namespace detail {
const std::string CAINFO = "agentca.pem";
const std::string SSLCERT = "server.pem";
const std::string SSLKEY = "server.key";
std::map<CertificateType, NginxCertificatePathProxy::FilePair> NginxCertificatePathProxy::g_certificateFileName = {
    {CertificateType::TRUSTE_CRETIFICATE_FILE, {CFG_AGENT_CA_INFO, CAINFO}},
    {CertificateType::USE_CRETIFICATE_FILE, {CFG_SSL_CERT, SSLCERT}},
    {CertificateType::KEY_FILE, {CFG_SSL_KEY, SSLKEY}}
};

std::string NginxCertificatePathProxy::GetCertificateRootPath()
{
    return CPath::GetInstance().GetNginxConfFilePath("");
}

std::string NginxCertificatePathProxy::GetCertificateFileName(CertificateType type)
{
    std::string ret;
    if (GetCertificateFileNameFromXml(type, g_certificateFileName[type].first, ret)) {
        return ret;
    }
    WARNLOG("Found file name from config file failed, return default file name");
    return g_certificateFileName[type].second;
}

EXTER_ATTACK bool NginxCertificatePathProxy::GetCertificateConfig(CertificateConfig config, std::string& value)
{
    switch (config) {
        case CertificateConfig::PASSWORD:
            return GetPassword(value);
        case CertificateConfig::ALGORITEHM_SUITE:
            return GetAlgorithmSuite(value);
        case CertificateConfig::HOST_NAME:
            GetHostName(value);
            return true;
        default:
            return false;
    }
}

bool NginxCertificatePathProxy::GetPassword(std::string& pw)
{
    std::string CipherStr;
    auto ret = CConfigXmlParser::GetInstance().GetValueString(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, CipherStr);
    if (ret != MP_SUCCESS) {
        return false;
    }
    DecryptStr(CipherStr, pw);
    if (pw.empty()) {
        WARNLOG("DecryptStr password output is empty");
        return false;
    }
    return true;
}

bool NginxCertificatePathProxy::GetAlgorithmSuite(std::string& suite)
{
    auto ret = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION,
        CFG_ALGORITHM_SUITE, suite);
    if (ret != MP_SUCCESS) {
        return false;
    }
    return true;
}
mp_void NginxCertificatePathProxy::GetHostName(std::string& pw)
{
    pw = "DataBackup-AGENT";
    SecureCom::GetHostFromCert(GetCertificateRootPath() +
        "/" + GetCertificateFileName(CertificateType::USE_CRETIFICATE_FILE), pw);
}

bool NginxCertificatePathProxy::GetCertificateFileNameFromXml(CertificateType type, const std::string& config,
    std::string& value)
{
    auto ret = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION,
        config, value);
    if (ret != MP_SUCCESS || value.empty()) {
        return false;
    }
    return true;
}
}
}