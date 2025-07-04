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
package openbackup.system.base.sdk.job.model;

import lombok.Data;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import java.util.List;

/**
 * Job对象
 *
 */
@Data
public class JobBo {
    private String userId;

    private String jobId;

    private JobTypeEnum type;

    private String name;

    private Integer progress;

    private Long startTime;

    private Long endTime;

    private Long lastUpdateTime;

    private JobStatusEnum status;

    private String speed;

    private String detail;

    private String[] detailPara;

    private List<JobLogBo> jobLogs;

    private String extendStr;

    private String associativeId;

    private String detailParas;

    /**
     * 保护对象ID
     */
    private String sourceId;

    /**
     * 保护对象名称
     */
    private String sourceName;

    /**
     * 保护对象类型
     */
    private ResourceSubTypeEnum sourceSubType;

    /**
     * 保护对象位置
     */
    private String sourceLocation;

    /**
     * 目标名称
     */
    private String targetName;

    /**
     * 目标对象位置
     */
    private String targetLocation;

    /**
     * 副本ID
     */
    private String copyId;

    /**
     * 副本创建时间
     */
    private Long copyTime;

    /**
     * 任务能否被中止
     */
    private Boolean enableStop;

    /**
     * request id
     */
    private String requestId;

    private String message;

    private String data;

    /**
     * 任务所在节点esn
     */
    private String deviceEsn;

    /**
     * 存储单元id
     */
    private String storageUnitId;

    /**
     * mark 用户标记的处理意见
     */
    private String mark;

    /**
     * markStatus 任务的处理状态
     */
    private String markStatus;
}
