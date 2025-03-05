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
package openbackup.system.base.common.model.livemount;

import com.baomidou.mybatisplus.annotation.FieldFill;
import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableName;
import com.fasterxml.jackson.annotation.JsonFormat;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;
import openbackup.system.base.common.typehandler.TimestampTypeHandler;
import openbackup.system.base.query.PageQueryConfig;

import java.sql.Timestamp;

/**
 * Live Mount Entity
 *
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
@TableName(value = "live_mount", autoResultMap = true)
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
    @TableField(fill = FieldFill.INSERT, typeHandler = TimestampTypeHandler.class)
    private Timestamp createdTime;

    @JsonFormat(shape = JsonFormat.Shape.STRING, pattern = "yyyy-MM-dd HH:mm:ss")
    @TableField(fill = FieldFill.INSERT_UPDATE, typeHandler = TimestampTypeHandler.class)
    private Timestamp updatedTime;

    private String mountedCopyId;

    @JsonFormat(shape = JsonFormat.Shape.STRING, pattern = "yyyy-MM-dd HH:mm:ss")
    @TableField(typeHandler = TimestampTypeHandler.class)
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
