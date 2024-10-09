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
#ifndef REPORT_JOB_HANDLER_H
#define REPORT_JOB_HANDLER_H

#include "CommonDBJob.h"
#include "trjsontostruct.h"

namespace GeneralDB {
const mp_int32 JOB_INTERNAL_ERROR = 0x5F02573B; // 任务执行过程中，由于内部错误导致任务失败。

enum class LOG_LABEL_TYPE {
    UNDEFIND_LABEL,                     // 不需上报label
    EXEC_PREJOB_FAIL,                   // 执行前置任务失败
    EXEC_GEN_SUBJOB,                    // 正在生成子任务
    EXEC_GENJOB_FAIL,                   // 生成子任务失败
    EXEC_GENJOB_SUCCESS,                // 生成子任务成功
    EXEC_BACKUP_SUBJOB_FAIL,            // 子任务（{0}）备份失败
    EXEC_RESTORE_SUBJOB_FAIL,           // 子任务（{0}）恢复失败
    EXEC_LIVEMOUNT_SUBJOB_FAIL,         // 子任务（{0}）即时挂载失败
    EXEC_CANCELLIVEMOUNT_SUBJOB_FAIL,   // 子任务（{0}）取消即时挂载失败
    EXEC_DELCOPY_SUBJOB_FAIL,           // 子任务（{0}）删除副本失败
    START_EXEC_SUBJOB                   // 数据保护代理主机（{0}）开始执行子任务（{1}）
};

struct JobLogDetail {
    mp_string jobId;
    mp_string subJobId;
    SubJobStatus::type jobStatus;
    LOG_LABEL_TYPE labelType;
    mp_int32 errorCode = -1;
};

struct LogData {
    mp_string label;
    std::vector<mp_string> params;
    AppProtect::JobLogLevel::type level;
};

class ReportJobDetailHandler {
public:
    static std::shared_ptr<ReportJobDetailHandler> GetInstance();

    mp_int32 ReportJobDetailToFrame(const GeneralDB::JobLogDetail &jobDetail);
private:
    std::vector<mp_string> LogDetailParam(const std::vector<mp_string>& parmas,
        const GeneralDB::JobLogDetail &jobDetail);
    mp_string GetAgentIp();
};
}
#endif // REPORT_JOB_HANDLER_H