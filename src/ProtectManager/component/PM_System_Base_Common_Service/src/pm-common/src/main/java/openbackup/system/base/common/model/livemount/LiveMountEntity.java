/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.model.livemount;

import openbackup.system.base.query.PageQueryConfig;

import com.baomidou.mybatisplus.annotation.FieldFill;
import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableName;
import com.fasterxml.jackson.annotation.JsonFormat;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import java.sql.Timestamp;

/**
 * Live Mount Entity
 *
 * @author l00272247
 * @since 2020-09-17
 */
@Data
@PageQueryConfig(
        conditions = {
            "%resource_name%",
            "%resource_path%",
            "%resource_ip%",
            "%target_resource_name%",
            "%target_resource_path%",
            "%target_resource_ip%"
        },
        orders = {"created_time", "mounted_copy_display_timestamp"})
@TableName(value = "live_mount")
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class LiveMountEntity {
    private String id;

    private String resourceType;

    private String resourceSubType;

    private String resourceId;

    private String resourceName;

    private String resourcePath;

    private String resourceIp;

    private String policyId;

    private String copyId;

    private String targetLocation;

    private String targetResourceId;

    private String targetResourceName;

    private String targetResourcePath;

    private String targetResourceIp;

    private String parameters;

    private int anonymizationStatus;

    private String status;

    private String enableStatus;

    @JsonFormat(shape = JsonFormat.Shape.STRING, pattern = "yyyy-MM-dd HH:mm:ss")
    @TableField(fill = FieldFill.INSERT)
    private Timestamp createdTime;

    @JsonFormat(shape = JsonFormat.Shape.STRING, pattern = "yyyy-MM-dd HH:mm:ss")
    @TableField(fill = FieldFill.INSERT_UPDATE)
    private Timestamp updatedTime;

    private String mountedCopyId;

    @JsonFormat(shape = JsonFormat.Shape.STRING, pattern = "yyyy-MM-dd HH:mm:ss")
    private Timestamp mountedCopyDisplayTimestamp;

    private String mountedSourceCopyId;

    private String scheduleId;

    private String mountedResourceId;

    private String userId;

    private String fileSystemShareInfo;

    /**
     * 即时挂载的任务id，用于卸载的时候传给Agent
     */
    private String mountJobId;

    @TableField(exist = false)
    private boolean isDeleteOriginalVM;

    /**
     * 演练任务id
     */
    @TableField(exist = false)
    private String exerciseId;

    /**
     * 演练任务id
     */
    @TableField(exist = false)
    private String exerciseJobId;
}
