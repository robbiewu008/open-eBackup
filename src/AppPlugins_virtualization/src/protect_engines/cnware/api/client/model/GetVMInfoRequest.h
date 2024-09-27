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
#ifndef GET_VM_INFO_REQUEST_H
#define GET_VM_INFO_REQUEST_H

#include <string>
#include "CNwareRequest.h"

namespace CNwarePlugin {
class GetVMInfoRequest : public CNwareRequest {
public:
    GetVMInfoRequest() {}
    ~GetVMInfoRequest() {}

    void SetDomainId(const std::string &domainId)
    {
        m_domainId = domainId;
    }

    std::string GetDomainId()
    {
        return m_domainId;
    }

private:
    std::string m_domainId;
};
};

#endif