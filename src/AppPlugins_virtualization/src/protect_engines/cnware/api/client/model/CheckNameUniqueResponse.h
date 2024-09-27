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
#ifndef CHECH_NAME_UNIQUE_RESPONSE_H
#define CHECH_NAME_UNIQUE_RESPONSE_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
struct ResOfNameCheck {
    bool mName;
    bool mDomainName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mName, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mDomainName, domainname)
    END_SERIAL_MEMEBER
};

class CheckNameUniqueResponse : public VirtPlugin::ResponseModel {
public:
    CheckNameUniqueResponse() {};
    ~CheckNameUniqueResponse() {};

    bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_checkRes)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }

    ResOfNameCheck GetDevices()
    {
        return m_checkRes;
    }

private:
    ResOfNameCheck m_checkRes;
};
};

#endif