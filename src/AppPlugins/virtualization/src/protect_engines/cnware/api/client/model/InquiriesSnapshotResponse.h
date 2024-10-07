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
#ifndef INQUIRIES_SNAPSHOT_RES_H
#define INQUIRIES_SNAPSHOT_RES_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
struct SnapshotDiskInfo {
    std::string m_storagePoolId;
    std::string m_storagePoolName;
    std::string m_volName;
    std::string m_volId;
    std::string m_originVolId;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storagePoolId, storagePoolId);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storagePoolName, storagePoolName);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volName, volName);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volId, volId);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_originVolId, originVolId);
    END_SERIAL_MEMEBER;
};

struct InquiriesSnapshotInfo {
    std::string m_id;
    std::string m_domainId;
    std::string m_hostId;
    std::string m_remark;
    std::vector<SnapshotDiskInfo> m_diskInfoList;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainId, domainId);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostId, hostId);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_remark, remark);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_diskInfoList, diskInfoList);
    END_SERIAL_MEMEBER;
};

class InquiriesSnapshotResponse : public VirtPlugin::ResponseModel {
public:
    InquiriesSnapshotResponse() {}
    ~InquiriesSnapshotResponse() {}

    bool Serial()
    {
        if (m_body.empty()) {
            ERRLOG("Body is empty");
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_content)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }

    InquiriesSnapshotInfo GetInquiriesSnapshotInfo()
    {
        return m_content;
    }

private:
    InquiriesSnapshotInfo m_content;
};
};

#endif