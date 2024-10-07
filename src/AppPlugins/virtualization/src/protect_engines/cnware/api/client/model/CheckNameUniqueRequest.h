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
#ifndef CHECH_NAME_UNIQUE_REQUEST_H
#define CHECH_NAME_UNIQUE_REQUEST_H

#include <string>
#include "CNwareRequest.h"

namespace CNwarePlugin {
class CheckNameUniqueRequest : public CNwareRequest {
public:
    CheckNameUniqueRequest() {};
    ~CheckNameUniqueRequest() {};

    void SetName(const std::string &name)
    {
        m_name = name;
    }
    void SetDomainName(const std::string &domainName)
    {
        m_domainName = domainName;
    }

    std::string GetName()
    {
        return m_name;
    }
    std::string GetDomainName()
    {
        return m_domainName;
    }

private:
    std::string m_name;
    std::string m_domainName;
};
};

#endif