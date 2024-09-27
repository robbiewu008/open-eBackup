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
#ifndef DELETE_DISK_REQUEST_H
#define DELETE_DISK_REQUEST_H

#include <string>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
class DeleteDiskRequest : public CNwareRequest {
public:
    DeleteDiskRequest() {}
    ~DeleteDiskRequest() {}

    void SetParam(const std::string &domainId, std::string &busDev)
    {
        m_domainId = domainId;
        m_busDev = m_busDev;
    }

    std::string GetDomainId()
    {
        return m_domainId;
    }

    std::string GetBusDev()
    {
        return m_busDev;
    }

private:
    std::string m_domainId;
    std::string m_busDev;
};
};

#endif