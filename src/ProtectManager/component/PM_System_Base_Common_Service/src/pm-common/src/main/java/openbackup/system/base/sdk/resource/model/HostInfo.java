/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * host信息
 *
 * @author t00482481
 * @since 2020-11-04
 */
@Data
public class HostInfo extends ResourceEntity {
    @JsonProperty("endpoint")
    private String endpoint;

    @JsonProperty("user_name")
    private String userName;

    @JsonProperty("password")
    private int password;

    @JsonProperty("port")
    private int port;

    @JsonProperty("os_type")
    private String osType;

    @JsonProperty("is_cluster")
    private boolean isCluster;

    @JsonProperty("sla_id")
    private String slaId;

    @JsonProperty("sla_name")
    private String slaName;

    @JsonProperty("sla_status")
    private String slaStatus;

    @JsonProperty("sla_compliance")
    private String slaCompliance;

    @JsonProperty("alias_type")
    private String aliasType;

    @JsonProperty("alias_value")
    private String aliasValue;
}
