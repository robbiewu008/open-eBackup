/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model.request;

import openbackup.system.base.sdk.auth.model.HcsUserRegion;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * UserProjects对象属性
 *
 * @author y30021475
 * @since 2023-07-27
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class HcsUserProjectInfo {
    /**
     * region信息
     */
    private List<HcsUserRegion> regions;

    private String id;

    private String name;

    private String iamProjectName;

    private String displayName;

    private String description;

    @JsonProperty("enabled")
    private boolean isEnabled;

    private String domainId;

    private String tenantId;

    private String tenantName;

    private String level;

    private String roleId;

    private String roleName;

    private String isShared;
}
