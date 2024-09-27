/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

import lombok.Data;

/**
 * Role
 *
 * @author y30021475
 * @since 2023-07-08
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
public class Role {
    private String name;
    private String id;
}
