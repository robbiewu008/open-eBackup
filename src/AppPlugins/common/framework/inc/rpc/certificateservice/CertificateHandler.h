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
#ifndef CERTIFICATEHANDLER_H
#define CERTIFICATEHANDLER_H
#include <memory>
#include "ICertificateHandler.h"
#include "ICertificatePathProxy.h"

namespace certificateservice {
namespace detail {
class CertificateHandler : public ICertificateHandler {

public:
    friend class CertificateService;
    CertificateHandler() = default;
    virtual ~CertificateHandler() = default;
    std::string GetCertificateFile(CertificateType type) override;
    bool GetCertificateConfig(CertificateConfig config, std::string& value) override;
private:
    std::shared_ptr<ICertificatePathProxy> m_pathProxy;
};
}
}

#endif