/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.backup;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.Map;

/**
 * Protected Environment
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class ProtectedObject {
    @JsonProperty("resource_id")
    private String resourceId;

    @JsonProperty("sla_id")
    private String slaId;

    @JsonProperty("sla_name")
    private String slaName;

    @JsonProperty("type")
    private String type;

    private String path;

    @JsonProperty("env_id")
    private String envUuid;

    @JsonProperty("env_type")
    private String envType;

    private String name;

    @JsonProperty("sub_type")
    private String subType;

    private int status;

    @JsonProperty("ext_parameters")
    private Map extParameters;
}
