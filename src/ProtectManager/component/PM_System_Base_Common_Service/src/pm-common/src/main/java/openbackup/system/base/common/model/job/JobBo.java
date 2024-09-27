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

import openbackup.system.base.sdk.job.model.JobLogBo;

import lombok.Data;

import java.util.List;

/**
 * Job对象
 *
 * @author w00448845
 * @version [BCManager 8.0.0]
 * @since 2020-04-15
 */
@Data
public class JobBo {
    private String userId;

    private String jobId;

    private String type;

    private Integer progress;

    private Long startTime;

    private Long endTime;

    private Long lastUpdateTime;

    private String status;

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
    private String sourceType;

    /**
     * 保护对象子类型
     */
    private String sourceSubType;

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
     * 是否为系统任务
     */
    private Boolean isSystem;

    /**
     * 任务中的消息体信息
     */
    private String message;

    /**
     * 任务附加状态
     */
    private String additionalStatus;

    /**
     * 任务详情是否包含特定级别事件标志位
     */
    private Integer logLevels;

    /**
     * 任务所在节点esn
     */
    private String deviceEsn;

    /**
     * 备份存储单元id
     */
    private String storageUnitId;

    /**
     * 备份存储单元名称
     */
    private String unitName;

    /**
     * 任务所在节点名称
     */
    private String clusterName;

    /**
     * 演练计划任务及其子任务存储演练id
     */
    private String exerciseId;

    /**
     * 演练计划的子任务存储父任务id
     */
    private String exerciseJobId;

    /**
     * 资源组id
     */
    private String resourceGroupId;

    /**
     * 资源组备份父任务id
     */
    private String groupBackupJobId;

    /**
     * mark 用户标记的处理意见
     */
    private String mark;

    /**
     * markStatus 任务的处理状态
     */
    private String markStatus;

    /**
     * 目录数量（目前仅用于Nas共享、文件集）
     */
    private Integer dirNum;

    /**
     * 文件数量（目前仅用于Nas共享、文件集）
     */
    private Integer fileNum;
}
