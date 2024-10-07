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
#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "define/Types.h"

namespace GeneralDB {
class JobControl {
public:
    JobControl() = default;
    ~JobControl() = default;

    /**
     *  @brief 终止任务
     *  @param returnValue 执行返回的结果
     *  @param jobId 任务ID
     *  @param subJobId 子任务ID
     *  @param appType 应用类型
     * */
    static mp_void AbortJob(ActionResult& returnValue, const Param& prm);

    /**
     *  @brief 暂停任务
     *  @param returnValue 执行返回的结果
     *  @param jobId 任务ID
     *  @param subJobId 子任务ID
     *  @param appType 应用类型
     * */
    static mp_void PauseJob(ActionResult& returnValue, const Param& prm);
};
}
#endif