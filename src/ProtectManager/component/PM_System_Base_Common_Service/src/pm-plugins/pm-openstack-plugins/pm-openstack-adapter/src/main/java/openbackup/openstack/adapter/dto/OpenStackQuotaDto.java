/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.openstack.adapter.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import javax.validation.constraints.Min;

/**
 * OpenStack备份配额
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-19
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class OpenStackQuotaDto {
    /**
     * 备份配额空间大小，单位为GB；只接收，不返回
     */
    @JsonProperty(access = JsonProperty.Access.WRITE_ONLY)
    @Min(-1)
    private int size;

    /**
     * 备份配额总空间，单位为GB，-1表示无限制
     */
    private int all;

    /**
     * 备份配额已使用空间，单位为GB
     */
    private int used;
}
