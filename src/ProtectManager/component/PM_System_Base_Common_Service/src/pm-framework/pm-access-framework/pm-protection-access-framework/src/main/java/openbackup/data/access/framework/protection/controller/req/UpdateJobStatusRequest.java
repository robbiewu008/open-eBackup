/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.data.access.framework.protection.controller.req;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import java.util.List;

import javax.validation.constraints.Size;

/**
 * 更新任务状态时数据模型
 *
 * @author w00616953
 * @since 2021-11-29
 */
@Data
public class UpdateJobStatusRequest {
    /**
     * requestId， 任务请求Id
     */
    @Length(max = 256)
    private String jobRequestId;

    /**
     * 任务id
     */
    @Length(max = 256)
    private String taskId;

    /**
     * 任务执行进度
     */
    private Integer progress;

    /**
     * 任务状态
     */
    private Integer status;

    /**
     * 任务执行速度
     */
    private Long speed;

    /**
     * 任务日志，限制最多50条
     */
    @Size(max = 50)
    private List<UpdateJobLogRequest> jobLogs;

    /**
     * 任务执行附加状态
     */
    @Length(max = 256)
    private String additionalStatus;

    /**
     * 扩展字段
     */
    private Object extendField;

    /**
     * 目标名称
     */
    @Length(max = 512)
    private String targetName;

    /**
     * 目标对象位置
     */
    @Length(max = 1024)
    private String targetLocation;
}
