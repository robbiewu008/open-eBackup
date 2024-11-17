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
#ifndef NGINX_CERTIFTCATE_PATH_PROXY_H_
#define NGINX_CERTIFTCATE_PATH_PROXY_H_
#include "servicecenter/certificateservice/include/ICertificatePathProxy.h"
#include "common/Defines.h"
#include <map>

namespace certificateservice {
namespace detail {
class NginxCertificatePathProxy : public ICertificatePathProxy {
public:
    NginxCertificatePathProxy() = default;
    virtual ~NginxCertificatePathProxy() = default;
    virtual std::string GetCertificateRootPath();
    virtual std::string GetCertificateFileName(CertificateType type);
    EXTER_ATTACK virtual bool GetCertificateConfig(CertificateConfig config, std::string& value);

private:
    bool GetPassword(std::string& pw);
    bool GetAlgorithmSuite(std::string& pw);
    mp_void GetHostName(std::string& pw);
    bool GetCertificateFileNameFromXml(CertificateType type, const std::string& config, std::string& value);

private:
    using FilePair = std::pair<std::string, std::string>;
    static std::map<CertificateType, FilePair> g_certificateFileName;
};
}
}
#endif