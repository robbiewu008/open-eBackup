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
#ifndef CHECK_SHARE_FILE_SYS_CNOF_UNIQUE_REQUEST_H
#define CHECK_SHARE_FILE_SYS_CNOF_UNIQUE_REQUEST_H

#include <string>
#include "CNwareRequest.h"

namespace CNwarePlugin {
class CheckShareFileSysConfUniqueRequest : public CNwareRequest {
public:
    CheckShareFileSysConfUniqueRequest() {}
    ~CheckShareFileSysConfUniqueRequest() {}

    void SetQueryParam(const std::string directory, const std::string id, int32_t type)
    {
        m_directory = directory;
        m_id = id;
        m_type = type;
    }

    std::string GetDirectory()
    {
        return m_directory;
    }
    std::string GetId()
    {
        return m_id;
    }
    int32_t GetType()
    {
        return m_type;
    }
private:
    std::string m_directory;
    std::string m_id;
    int32_t m_type;
};
};
#endif