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
#ifndef CNWARE_QUERY_VM_TASK_RESPONSE_H
#define CNWARE_QUERY_VM_TASK_RESPONSE_H

#include <string>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
struct VMTask {
    std::string mId;
    std::string mCode;
    std::string mName;
    std::string mDescription;
    int mStatus;
    std::string mTargetName;
    std::string mDomainId;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mId, id);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCode, code);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mName, name);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mDescription, description);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mStatus, status);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTargetName, targetName);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mDomainId, domainId);
    END_SERIAL_MEMEBER;
};

struct VMTaskInfo {
    std::vector<VMTask> mData;
    int32_t mTotal = 0;
    int32_t mStart = 0;
    int32_t mSize = 0;
    std::string mSort;
    std::string mOrder;
 
    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mData, data);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotal, total);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mStart, start);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSize, size);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSort, sort);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mOrder, order);
    END_SERIAL_MEMEBER;
};

class QueryVMTaskResponse : public VirtPlugin::ResponseModel {
public:
    QueryVMTaskResponse() {}
    ~QueryVMTaskResponse() {}
 
    bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_vmTaskInfo)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }
 
    VMTaskInfo GetVMTaskInfo()
    {
        return m_vmTaskInfo;
    }

private:
    VMTaskInfo m_vmTaskInfo;
};
};

#endif