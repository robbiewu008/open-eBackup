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
#ifndef __DELETE_VM_REQUEST_H__
#define __DELETE_VM_REQUEST_H__

#include "CNwareRequest.h"

namespace CNwarePlugin {

enum class DeleteVMType {
    DELETE_VM_KEEP_VOLUMES = 1,
    DELETE_VM_AND_VOLUMES,
    CLEAR_VM_DB_RECORD,
    DELETE_VM_AND_VOLUMES_ERASE
};

class DeleteVMRequest : public CNwareRequest {
public:
    DeleteVMRequest() = default;
    ~DeleteVMRequest() = default;

    void SetDeleteType(const DeleteVMType type)
    {
        m_deleteType = type;
    }

    DeleteVMType GetDeleteType()
    {
        return m_deleteType;
    }

    void SetIsNowDo(const bool deleteNow)
    {
        m_isNowDo = deleteNow;
    }

    bool GetIsNowDo()
    {
        return m_isNowDo;
    }

private:
    DeleteVMType m_deleteType = DeleteVMType::DELETE_VM_AND_VOLUMES;
    bool m_isNowDo = true;
};

};

#endif // __DELETE_VM_REQUEST_H__