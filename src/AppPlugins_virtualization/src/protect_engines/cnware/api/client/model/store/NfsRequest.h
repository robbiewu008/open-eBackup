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
#ifndef NFS_REQUEST_H
#define NFS_REQUEST_H

#include <string>
#include "../CNwareRequest.h"
#include "common/Constants.h"

namespace CNwarePlugin {

class NfsRequest : public CNwareRequest {
public:
    NfsRequest() {}
    ~NfsRequest() {}

    bool SetNfsReq(const std::string &name)
    {
        NfsReq nfsReq;
        nfsReq.m_name = name;
        if (!Module::JsonHelper::StructToJsonString(nfsReq, m_body)) {
            ERRLOG("Convert snapinfo to json string failed!");
            return false;
        }
        return true;
    }
};
};
#endif