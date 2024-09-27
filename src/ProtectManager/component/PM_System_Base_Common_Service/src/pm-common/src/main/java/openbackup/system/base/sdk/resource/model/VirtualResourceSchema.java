/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 虚拟机资源类
 *
 * @author t00482481
 * @since 2020-10-29
 */
@Data
public class VirtualResourceSchema extends ResourceEntity {
    @JsonProperty("vm_ip")
    private String vmIp;

    @JsonProperty("env_ip")
    private String envIp;

    @JsonProperty("link_status")
    private String linkStatus;

    @JsonProperty("mo_id")
    private String moId;

    @JsonProperty("alias_type")
    private String aliasType;

    @JsonProperty("alias_value")
    private String aliasValue;

    @JsonProperty("capacity")
    private String capacity;

    @JsonProperty("free_space")
    private String freeSpace;

    @JsonProperty("uncommitted")
    private String uncommitted;

    @JsonProperty("children")
    private String children;

    @JsonProperty("is_template")
    private String isTemplate;

    @JsonProperty("os_type")
    private String osType;

    @JsonProperty("partitions")
    private List<String> partitions;
}
