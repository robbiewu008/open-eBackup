/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * hcs资源集
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/7/26
 */
@Getter
@Setter
public class HcsProject {
    private String id;
    private String name;
    private String description;
    @JsonProperty("domain_id")
    private String domainId;
    private boolean enabled;
    @JsonProperty("tenant_id")
    private String tenantId;
    @JsonProperty("is_shared")
    private boolean isShared;
    @JsonProperty("tenant_name")
    private String tenantName;
    @JsonProperty("create_user_id")
    private String createUserId;
    @JsonProperty("create_user_name")
    private String createUserName;
    private List<Region> regions;
}
