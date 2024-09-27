/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.openstack.adapter.dto;

import openbackup.openstack.adapter.enums.OpenStackJobType;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;

/**
 * 云核OpenStack恢复任务Dto
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-16
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class OpenStackRestoreJobDto {
    /**
     * 恢复任务id
     */
    private String id;

    /**
     * 恢复任务名称
     */
    @NotEmpty
    private String name;

    /**
     * 恢复任务描述
     */
    @NotEmpty
    private String description;

    /**
     * 恢复对象实例
     */
    @NotNull
    private OpenStackJobType type;

    /**
     * 恢复对象实例id，虚拟机id或卷id
     */
    @NotEmpty
    private String instanceId;

    /**
     * 待恢复的备份副本id
     */
    @NotEmpty
    private String copyId;

    /**
     * 任务执行结果
     */
    private String result;

    /**
     * 任务状态
     */
    private String status;
}
