/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

/**
 * 手动备份请求参数
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-23
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class ManualBackupReq {
    /**
     * 绑定的SLA的id
     */
    private String slaId;

    /**
     * 备份动作
     */
    private String action;

    /**
     * 副本名称
     */
    private String copyName;

    /**
     * 是否是资源组
     */
    private Boolean isResourceGroup;
}
