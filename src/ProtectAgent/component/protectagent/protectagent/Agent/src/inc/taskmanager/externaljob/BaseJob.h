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
#ifndef BASE_JOB_HEADER
#define BASE_JOB_HEADER
#include <memory>
#include <common/Types.h>

namespace  AppProtect {

class BaseJob : public std::enable_shared_from_this<BaseJob> {
public:
    BaseJob() {}
    virtual ~BaseJob() {}
    virtual mp_string GetJobId()
    {
        return m_jobId;
    }
    virtual mp_int32 Exec()=0;

protected:
    mp_string m_jobId;
};
}

#endif