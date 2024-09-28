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

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.query.PageQueryConfig;
import openbackup.system.base.sdk.job.model.request.JobMessage;

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Data;

import java.util.Optional;

/**
 * job DAO类
 *
 */
@TableName(value = "t_job")
@Data
@PageQueryConfig(
        conditions = {"%source_name%", "%source_location%", "%target_name%", "%target_location%"},
        orders = {"start_time", "copy_time", "end_time"})
public class Job {
    @TableId
    private String jobId;

    private String userId;

    private String type;

    private Integer progress;

    private Long startTime;

    private Long endTime;

    private Long lastUpdateTime;

    private String status;

    private String parentId;

    private String speed;

    private String detail;

    private String detailPara;

    private String extendStr;

    private String associativeId;

    /**
     * 保护对象ID
     */
    private String sourceId;

    /**
     * 保护对象名称
     */
    private String sourceName;

    /**
     * 资源类型
     */
    private String sourceType;

    /**
     * 资源子类型
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
     * 请求ID
     */
    private String requestId;

    /**
     * 是否可见
     */
    private Boolean isVisible;

    /**
     * 附加状态
     */
    private String additionalStatus;

    private String message;

    private String data;

    /**
     * 任务详情是否包含特定级别事件标志位。
     * 使用整数的二进制形式表示多个标志位。
     * 目前表示3个标志位，从高位到低位分别表示 “是否包含警告事件”、“是否包含错误事件”、“是否包含严重事件”。
     * 0表示不包含，1表示包含包含，示例：（0b）110 表示包含警告、错误事件，不包含严重事件，jobLogLevel此时为6。
     */
    private Integer logLevels;

    /**
     * 任务所在节点esn
     */
    private String deviceEsn;

    /**
     * 存储单元id
     */
    private String storageUnitId;

    /**
     * 备份任务slaId
     */
    private String slaId;

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
     * mark，用户标记的处理意见
     */
    private String mark;

    /**
     * markStatus，任务的处理状态
     */
    private String markStatus;

    /**
     * parse message as JobMessage
     *
     * @return JobMessage
     */
    public JobMessage message() {
        return JSONObject.fromObject(message).toBean(JobMessage.class);
    }

    /**
     * get job message payload with traffic data
     *
     * @return job message payload
     */
    public JSONObject resolvePayloadAndTraffic() {
        JSONObject json = JSONObject.fromObject(message);
        JSONObject payload = Optional.ofNullable(json.getJSONObject("payload")).orElseGet(JSONObject::new);
        JSONObject traffic = json.getJSONObject("traffic");
        if (traffic != null) {
            payload.putAll(traffic);
        }
        if (associativeId != null) {
            payload.put("request_id", associativeId);
        }
        return payload;
    }
}
