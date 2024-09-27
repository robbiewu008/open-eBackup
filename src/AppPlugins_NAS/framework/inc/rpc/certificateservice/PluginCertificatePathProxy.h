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
#ifndef PLUGINCERTIFICATEPATHPROXY_H
#define PLUGINCERTIFICATEPATHPROXY_H
#include <map>
#include "ICertificatePathProxy.h"

namespace certificateservice {
namespace detail {
class PluginCertificatePathProxy : public ICertificatePathProxy {

public:
    PluginCertificatePathProxy() = default;
    virtual ~PluginCertificatePathProxy() = default;
    std::string GetCertificateRootPath() override;
    std::string GetCertificateFileName(CertificateType type) override;
    bool GetCertificateConfig(CertificateConfig config, std::string& value) override;

private:
    bool GetPassword(std::string& pw);
    bool GetAlgorithmSuite(std::string& suite);
    bool GetHostName(std::string& hostName);
};
}
}

#endif