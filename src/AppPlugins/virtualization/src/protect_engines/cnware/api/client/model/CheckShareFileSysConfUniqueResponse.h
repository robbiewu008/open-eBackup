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
#ifndef CHECK_SHARE_FILE_SYS_CNOF_UNIQUE_RESPONSE_H
#define CHECK_SHARE_FILE_SYS_CNOF_UNIQUE_RESPONSE_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

struct CheckStorageUniqueRsp {
    bool mDirectory = false;   // 目录是否存在

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mDirectory, directory)
    END_SERIAL_MEMEBER
};
class CheckShareFileSysConfUniqueResponse : public VirtPlugin::ResponseModel {
public:
    CheckShareFileSysConfUniqueResponse() {}
    ~CheckShareFileSysConfUniqueResponse() {}

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
    bool IsDirectoryExist()
    {
        return m_checkRes.mDirectory;
    }
private:
    CheckStorageUniqueRsp m_checkRes;
};
#endif