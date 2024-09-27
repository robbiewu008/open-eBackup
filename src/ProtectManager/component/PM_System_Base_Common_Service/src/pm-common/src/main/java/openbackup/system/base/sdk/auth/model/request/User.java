/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model.request;

import openbackup.system.base.sdk.auth.model.Domain;

import lombok.Getter;
import lombok.Setter;

/**
 * 功能描述
 *
 * @author l30044826
 * @since 2023-07-07
 */
@Getter
@Setter
public class User {
    private String name;

    private String password;

    private Domain domain;
}
