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
package openbackup.data.access.framework.livemount.entity;

import openbackup.system.base.query.PageQueryConfig;

import com.baomidou.mybatisplus.annotation.FieldFill;
import com.baomidou.mybatisplus.annotation.IdType;
import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Data;

import java.sql.Timestamp;

/**
 * UpdatingPolicyEntity
 *
 */
@Data
@PageQueryConfig(conditions = {"%name%"}, orders = {"live_mount_count", "created_time"})
@TableName(value = "live_mount_policy")
public class LiveMountPolicyEntity {
    @TableId(type = IdType.INPUT)
    private String policyId;

    private String name;

    private String copyDataSelectionPolicy;

    private String retentionPolicy;

    private String retentionUnit;

    private Integer retentionValue;

    private String schedulePolicy;

    private Integer scheduleInterval;

    private String scheduleIntervalUnit;

    private Timestamp scheduleStartTime;

    @TableField(fill = FieldFill.INSERT)
    private Timestamp createdTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private Timestamp updatedTime;

    private String latestCopyFor;

    private String afterCopyGenerated;

    @TableField(exist = false)
    private int liveMountCount;

    private String userId;
}


