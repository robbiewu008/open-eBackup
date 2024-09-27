/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.core.entity;

import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Builder;
import lombok.Data;

import java.sql.Timestamp;

/**
 * 副本信息
 *
 * @author w00574036
 * @version [CyberEngine 300 1.0.0]
 * @since 2023/02/13
 */
@Data
@Builder
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
    @TableField("display_timestamp")
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
     * 副本时间戳
     */
    @TableField("generated_time")
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
    @TableField("expiration_time")
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
     * 设备Esn
     */
    @TableField("device_esn")
    private String deviceEsn;

    /**
     * gn
     */
    @TableField("gn")
    private String gn;
}
