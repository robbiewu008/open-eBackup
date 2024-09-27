/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * token
 *
 * @author l30044826
 * @since 2023-07-07
 */
@Getter
@Setter
public class HcsToken {
    private String tokenStr;

    private List<String> methods;

    @JsonProperty("expires_at")
    private String expiresAt;

    private String issuedAt;

    private TokenUser user;

    private Domain domain;

    private List<CataLog> catalog;

    private List<Role> roles;

    private Project project;
}
