/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Vm信息
 *
 * @author t00482481
 * @since 2020-11-04
 */
@Data
public class VmInfo extends ResourceEntity {
    @JsonProperty("mo_id")
    private String moId;

    @JsonProperty("env_ip")
    private String envIp;

    @JsonProperty("vm_ip")
    private String vmIp;

    @JsonProperty("capacity")
    private int capacity;

    @JsonProperty("free_space")
    private int freeSpace;

    @JsonProperty("uncommitted")
    private int uncommitted;

    @JsonProperty("children")
    private int children;

    @JsonProperty("is_template")
    private boolean isTemplate;

    @JsonProperty("alias_type")
    private String aliasType;

    @JsonProperty("alias_value")
    private String aliasValue;
}
