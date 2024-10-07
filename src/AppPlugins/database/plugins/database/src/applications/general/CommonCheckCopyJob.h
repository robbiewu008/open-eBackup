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
#ifndef COMMON_CHECKCOPY_JOB_H
#define COMMON_CHECKCOPY_JOB_H

#include "CommonDBJob.h"
#include "trjsontostruct.h"
#include "trstructtojson.h"

namespace GeneralDB {
class CommonCheckCopyJob : public GeneralDB::CommonDBJob, public std::enable_shared_from_this<CommonCheckCopyJob> {
public:
    CommonCheckCopyJob() {}
    ~CommonCheckCopyJob() {}

    int PrerequisiteJob() override;
    int GenerateSubJob() override;
    int ExecuteSubJob() override;
    int PostJob() override;
};
}
#endif // COMMON_CHECKCOPY_JOB_H