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
#ifndef CNWARE_DELETE_DISK_ON_STORAGE_REQUEST_H
#define CNWARE_DELETE_DISK_ON_STORAGE_REQUEST_H

#include <string>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
struct DeleteStorageVolumeReq {
    std::string m_deleteAffirm = "YES";

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_deleteAffirm, deleteAffirm)
    END_SERIAL_MEMEBER
};

class DelDiskOnStorageRequest : public CNwareRequest {
public:
    DelDiskOnStorageRequest() {}
    ~DelDiskOnStorageRequest() {}

    void SetVolId(const std::string &volId)
    {
        m_volId = volId;
    }

    void SetDelDiskOnStorageRequest(const DeleteStorageVolumeReq &req)
    {
        m_deleteStorageVolumeReq = req;
    }

    std::string GetVolId()
    {
        return m_volId;
    }

    int32_t DelDiskOnStorageRequestToJson()
    {
        if (!Module::JsonHelper::StructToJsonString(m_deleteStorageVolumeReq, m_body)) {
            ERRLOG("Convert DelDiskOnStorageRequest to json string failed!");
            return VirtPlugin::FAILED;
        }
        return VirtPlugin::SUCCESS;
    }

private:
    std::string m_volId;
    DeleteStorageVolumeReq m_deleteStorageVolumeReq;
};
};

#endif