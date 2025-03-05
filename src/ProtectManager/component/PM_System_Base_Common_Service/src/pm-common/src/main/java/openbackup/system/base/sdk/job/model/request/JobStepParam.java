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
package openbackup.system.base.sdk.job.model.request;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;
import openbackup.system.base.sdk.job.model.JobStatusEnum;

import java.util.List;

/**
 * 任务步骤参数
 *
 */
@Getter
@Setter
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class JobStepParam {
    /**
     * 任务id
     */
    private String jobId;

    /**
     * 任务描述
     */
    private String logInfo;

    /**
     * 步骤描述
     */
    private List<String> params;

    /**
     * job日志级别
     */
    private String level;

    /**
     * 错误描述
     */
    private String detail;

    /**
     * 错误详细描述
     */
    private List<String> detailParam;

    /**
     * 任务状态
     */
    private JobStatusEnum status;
}
