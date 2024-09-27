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

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobStatusEnum;

import lombok.Data;

import java.util.List;

/**
 * 更新任务时的请求模型
 *
 * @author y00280557
 * @since 2020-09-01
 */
@Data
public class UpdateJobRequest {
    /**
     * 任务进度
     */
    private Integer progress;

    /**
     * 任务状态
     */
    private JobStatusEnum status;

    /**
     * 任务附加状态
     */
    private String additionalStatus;

    /**
     * 任务速度
     */
    private String speed;

    /**
     * 扩展字段，JSON格式，存放与任务类型强关联的字段与值
     */
    private String extendStr;

    /**
     * 关联ID，比如anybackup的实例ID
     */
    private String associativeId;

    /**
     * 副本ID
     */
    private String copyId;

    /**
     * 副本生成时间
     */
    private Long copyTime;

    /**
     * 是否能够停止
     */
    private Boolean enableStop;

    /**
     * 任务日志
     */
    private List<JobLogBo> jobLogs;

    /**
     * 存放任务运行过程中会用到的临时数据
     */
    private JSONObject data;

    /**
     * 存放esn
     */
    private String deviceEsn;

    /**
     * 存储单元id
     */
    private String storageUnitId;

    /**
     * 目标名称
     */
    private String targetName;

    /**
     * 目标对象位置
     */
    private String targetLocation;

    /**
     * mark 用户标记的任务处理意见
     */
    private String mark;

    /**
     * markStatus 任务处理状态
     */
    private String markStatus;
}
