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
#ifndef ICERTIFICATEHANDLER_H
#define ICERTIFICATEHANDLER_H

#include <string>
#include "ICertificateComm.h"

namespace certificateservice {
class ICertificateHandler {
public:
    ICertificateHandler() = default;
    virtual ~ICertificateHandler() = default;
    virtual std::string GetCertificateFile(CertificateType type) = 0;
    virtual bool GetCertificateConfig(CertificateConfig config, std::string& value) = 0;
};
}   // namespace certificateservice
#endif
