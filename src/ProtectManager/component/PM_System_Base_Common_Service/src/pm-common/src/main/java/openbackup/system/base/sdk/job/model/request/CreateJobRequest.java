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
import openbackup.system.base.sdk.job.model.JobStatusEnum;

import lombok.Data;

import java.util.List;

import javax.validation.constraints.NotNull;

/**
 * 功能描述
 *
 */
@Data
public class CreateJobRequest {
    private String jobId;

    private String userId;

    @NotNull
    private String type;

    private String detail;

    private String[] detailPara;

    /**
     * 任务是否具备中止能力。具备：true 不具备：false
     */
    private boolean enableStop = false;

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
     * 是否为系统任务
     */
    private Boolean isSystem = false;

    /**
     * 副本ID
     */
    private String copyId;

    /**
     * 副本生成时间
     */
    private Long copyTime;

    /**
     * 请求ID
     */
    private String requestId;

    /**
     * 任务是否显示
     */
    private Boolean isVisible = true;

    /**
     * 任务消息，存放kafka消息topic、任务限流配置数据等信息
     */
    private JobMessage message;

    /**
     * 任务状态
     */
    private JobStatusEnum status;

    /**
     * 存放任务运行过程中会用到的临时数据
     */
    private JSONObject data;

    /**
     * 扩展参数，存放任务类型强相关的数据
     */
    private JSONObject extendField;

    /**
     * 任务详情是否包含特定级别事件标志位
     */
    private Integer logLevels;

    /**
     * 设备esn
     */
    private String deviceEsn;

    /**
     * 演练id
     */
    private String exerciseId;

    /**
     * 设备esn
     */
    private String exerciseJobId;

    /**
     * 域id列表
     */
    private List<String> domainIdList;
}
