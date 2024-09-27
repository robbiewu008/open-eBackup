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
#ifndef REMOVE_ASSOCIATE_HOST_RES_H
#define REMOVE_ASSOCIATE_HOST_RES_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
struct RemoveHostData {
    int64_t mCapacity;
    int32_t mStoreType;
    int32_t mResourceNum;
    int32_t mHostNum;
    std::string mIp;
    std::string mId;
    std::string mRemark;
    std::string mStorageName;
    std::string mCreateTime;
    std::string mLastScnTime;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCapacity, capacity);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mStoreType, storeType);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mResourceNum, resourceNum);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mHostNum, hostNum);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIp, ip);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mId, id);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mRemark, remark);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mStorageName, storageName);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCreateTime, createTime);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mLastScnTime, lastScnTime);
    END_SERIAL_MEMEBER;
};

struct RemoveHosttobeAssociated {
    std::vector<struct RemoveHostData> mData;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mData, data);
    END_SERIAL_MEMEBER;
};

class RemoveAssociateHostResponse : public VirtPlugin::ResponseModel {
public:
    RemoveAssociateHostResponse() {}
    ~RemoveAssociateHostResponse() {}

    bool Serial()
    {
        if (m_body.empty()) {
            ERRLOG("Body is empty");
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_Data)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }

    RemoveHosttobeAssociated GetData()
    {
        return m_Data;
    }

private:
    RemoveHosttobeAssociated m_Data;
};
};

#endif