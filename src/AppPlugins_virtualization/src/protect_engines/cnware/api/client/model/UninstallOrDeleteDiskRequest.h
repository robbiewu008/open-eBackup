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
#ifndef UNINSTALL_OR_DELETE_DISK_REQUEST_H
#define UNINSTALL_OR_DELETE_DISK_REQUEST_H

#include <string>
#include <common/JsonHelper.h>
#include "common/Constants.h"
#include "CNwareRequest.h"

namespace CNwarePlugin {

enum class DeleteType {
    TYPE_DELETE,
    TYPE_UNINSTALL
};

class UninstallOrDeleteDiskRequest : public CNwareRequest {
public:
    UninstallOrDeleteDiskRequest() {}
    ~UninstallOrDeleteDiskRequest() {}

    int32_t SetParam(std::string domainId, std::string volId, int32_t deleteType)
    {
        if (deleteType != static_cast<int32_t>(DeleteType::TYPE_DELETE) &&
            deleteType != static_cast<int32_t>(DeleteType::TYPE_UNINSTALL)) {
            ERRLOG("DeleteType is invalid, deleteType is: %d", deleteType);
            return VirtPlugin::FAILED;
        }

        m_domainId = domainId;
        m_volId = volId;
        m_deleteType = deleteType;
        return VirtPlugin::SUCCESS;
    }

    std::string GetDomainId()
    {
        return m_domainId;
    }
    std::string GetVolId()
    {
        return m_volId;
    }
    int32_t GetDeleteType()
    {
        return m_deleteType;
    }

private:
    std::string m_domainId;
    std::string m_volId;
    int32_t m_deleteType = 0;
};
};

#endif