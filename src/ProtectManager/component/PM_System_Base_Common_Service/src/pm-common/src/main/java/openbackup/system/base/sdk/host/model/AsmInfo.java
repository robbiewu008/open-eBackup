/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.host.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * asm信息
 *
 * @author l00272247
 * @since 2020-10-09
 */
@Data
public class AsmInfo {
    @JsonProperty("auth_type")
    private String authType;

    @JsonProperty("inst_name")
    private String instName;

    @JsonProperty("is_cluster")
    private String isCluster;

    @JsonProperty("disk_string")
    private String diskString;

    @JsonProperty("disk_groups")
    private String diskGroups;
}
