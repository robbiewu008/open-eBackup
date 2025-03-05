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
package openbackup.system.base.common.model.job;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * JobReport对象
 *
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class JobReportBo {
    private String jobId;

    private String type;

    private String subType;

    private Integer progress;

    private String startTime;

    private String endTime;

    private String status;

    private String speed;

    private String dataBeforeReduction;

    private String userId;

    private String exerciseJobId;

    private String exerciseId;

    private String exerciseName;

    private String sourceId;

    /**
     * 保护对象名称
     */
    private String sourceName;

    /**
     * 保护对象位置
     */
    private String sourceLocation;

    /**
     * 保护对象子类型
     */
    private String sourceSubType;

    /**
     * 目标名称
     */
    private String targetName;

    /**
     * 目标对象位置
     */
    private String targetLocation;

    /**
     * 任务持续时间
     */
    private String duration;

    /**
     * 任务事件失败或告警详情
     */
    private String labelDetailedDescription;
}
