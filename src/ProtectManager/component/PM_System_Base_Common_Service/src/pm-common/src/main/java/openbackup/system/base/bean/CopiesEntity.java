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
package openbackup.system.base.bean;

import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import openbackup.system.base.common.typehandler.TimestampTypeHandler;

import java.sql.Timestamp;

/**
 * 副本信息
 *
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
@TableName("COPIES")
public class CopiesEntity {
    /**
     * 副本ID
     */
    @TableId
    private String uuid;

    /**
     * 副本链ID
     */
    @TableField("chain_id")
    private String chainId;

    /**
     * 副本时间戳
     */
    private String timestamp;

    /**
     * 副本时间戳
     */
    @TableField(value = "display_timestamp",
                typeHandler = TimestampTypeHandler.class)
    private Timestamp displayTimestamp;

    /**
     * 副本是否可删除
     */
    @TableField("deletable")
    private boolean isDeletable;

    /**
     * 副本有效性
     */
    private String status;

    /**
     * 副本位置
     */
    private String location;

    /**
     * 备份类型
     */
    @TableField("backup_type")
    private int backupType;

    /**
     * 副本生成类型:0 Imported, 1 Replicated, 2 Backup, 3 Archive, 4 Live Mount
     */
    @TableField("generated_by")
    private String generatedBy;

    /**
     * 原副本ID，复制前
     */
    @TableField("origin_backup_id")
    private String originBackupId;

    /**
     * 副本时间戳
     */
    @TableField(value = "generated_time",
                typeHandler = TimestampTypeHandler.class)
    private Timestamp generatedTime;

    /**
     * 副本支持的特性
     */
    private int features;

    /**
     * 副本建立索引状态
     */
    private String indexed;

    /**
     * 副本代数
     */
    private int generation;

    /**
     * 父副本ID
     */
    @TableField("parent_copy_uuid")
    private String parentCopyUuid;

    /**
     * 副本保留类型：1（永久保留）2（指定时间保留）
     */
    @TableField("retention_type")
    private int retentionType;

    /**
     * 副本保留时间
     */
    @TableField("retention_duration")
    private int retentionDuration;

    /**
     * 副本保留时间单位（天、周、月、年）
     */
    @TableField("duration_unit")
    private String durationUnit;

    /**
     * 副本过期时间
     */
    @TableField(value = "expiration_time",
                typeHandler = TimestampTypeHandler.class)
    private Timestamp expirationTime;

    /**
     * 副本扩展属性
     */
    private String properties;

    /**
     * 资源ID
     */
    @TableField("resource_id")
    private String resourceId;

    /**
     * 资源名称
     */
    @TableField("resource_name")
    private String resourceName;

    /**
     * 资源类型
     */
    @TableField("resource_type")
    private String resourceType;

    /**
     * 资源子类型
     */
    @TableField("resource_sub_type")
    private String resourceSubType;

    /**
     * 资源位置
     */
    @TableField("resource_location")
    private String resourceLocation;

    /**
     * 资源状态
     */
    @TableField("resource_status")
    private String resourceStatus;

    /**
     * 资源属性（JSON格式）
     */
    @TableField("resource_properties")
    private String resourceProperties;

    /**
     * 资源环境名称
     */
    @TableField("resource_environment_name")
    private String resourceEnvironmentName;

    /**
     * 资源环境IP
     */
    @TableField("resource_environment_ip")
    private String resourceEnvironmentIp;

    /**
     * SLA名称
     */
    @TableField("sla_name")
    private String slaName;

    /**
     * SLA属性（JSON格式）
     */
    @TableField("sla_properties")
    private String slaProperties;

    /**
     * 副本资源的用户id
     */
    @TableField("user_id")
    private String userId;

    /**
     * 副本是否归档
     */
    @TableField("is_archived")
    private Boolean isArchived;

    /**
     * 副本是否复制
     */
    @TableField("is_replicated")
    private Boolean isReplicated;

    /**
     * 副本详情
     */
    private String detail;

    /**
     * 副本名字
     */
    private String name;

    /**
     * 存储库id
     */
    @TableField("storage_id")
    private String storageId;

    /**
     * 原始副本类型
     */
    @TableField("source_copy_type")
    private String sourceCopyType;

    /**
     * 副本worm状态
     */
    @TableField("worm_status")
    private String wormStatus;

    /**
     * worm过期时间
     */
    @TableField(value = "worm_expiration_time", typeHandler = TimestampTypeHandler.class)
    private Timestamp wormExpirationTime;

    /**
     * 设备Esn
     */
    @TableField("device_esn")
    private String deviceEsn;

    /**
     * gn
     */
    @TableField("gn")
    private String gn;

    /**
     * copy storage unit status
     */
    @TableField("storage_unit_status")
    private Integer storageUnitStatus;
}
