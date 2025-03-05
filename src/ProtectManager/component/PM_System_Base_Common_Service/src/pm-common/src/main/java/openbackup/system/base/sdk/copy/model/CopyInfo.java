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
package openbackup.system.base.sdk.copy.model;

import com.baomidou.mybatisplus.annotation.TableField;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;
import openbackup.system.base.common.typehandler.TimeStringTypeHandler;

import org.apache.commons.lang3.StringUtils;

import java.util.Date;

/**
 * Copy Info
 *
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class CopyInfo {
    private String uuid;

    private String chainId;

    private String timestamp;

    @TableField(typeHandler = TimeStringTypeHandler.class)
    private String displayTimestamp;

    @JsonProperty("deletable")
    private boolean isDeletable;

    private String status; // 副本状态

    private int storageUnitStatus; // 副本所在的存储单元状态

    private String location;

    private int backupType; // 1：完全备份 2：增量备份 3：差异备份 4：日志备份

    private String generatedBy; // 生成方式

    @TableField(typeHandler = TimeStringTypeHandler.class)
    private String generatedTime;

    @TableField(typeHandler = TimeStringTypeHandler.class)
    private String originCopyTimeStamp; // 副本备份时间

    private String generationType;

    private int features;

    private String indexed;

    private int generation;

    private String parentCopyUuid;

    private int retentionType;

    private int retentionDuration;

    private String durationUnit;

    private Date expirationTime;

    private String properties;

    private String resourceId;

    private String resourceName;

    private String resourceType;

    private String resourceSubType;

    private String resourceLocation;

    private String resourceStatus; // 副本所属资源状态

    private String resourceEnvironmentName;

    private String resourceEnvironmentIp;

    private String resourceProperties;

    private String slaName;

    private String slaProperties;

    private String jobType;

    private String userId;

    private Boolean isArchived;

    private Boolean isReplicated;

    private String name;

    private String storageId;

    private int sourceCopyType;

    private int wormStatus = 1;

    /**
     * 副本所在设备的esn号
     */
    private String deviceEsn;

    /**
     * 副本所在存储单元id
     */
    private String storageUnitId;

    /**
     * 原始副本备份id多集群场景复制场景使用
     */
    private String originBackupId;

    /**
     * 是否为存储快照
     */
    @JsonProperty("storage_snapshot_flag")
    private Boolean isStorageSnapshot;

    /**
     * 扩展类型
     */
    @JsonProperty("extend_type")
    private String extendType;

    /**
     * worm有效时间类型
     */
    private int wormValidityType;

    /**
     * worm保留时间
     */
    private int wormRetentionDuration;

    /**
     * worm保留时间单位类型
     */
    private String wormDurationUnit;

    /**
     * worm过期时间
     */
    private Date wormExpirationTime;

    /**
     * 浏览挂载状态
     */
    private String browseMounted;

    // DP中originCopyTimeStamp类型为datetime,不能传空字符串
    public void setOriginCopyTimeStamp(String originCopyTimeStamp) {
        if (StringUtils.isEmpty(originCopyTimeStamp)) {
            this.originCopyTimeStamp = null;
        } else {
            this.originCopyTimeStamp = originCopyTimeStamp;
        }
    }
}
