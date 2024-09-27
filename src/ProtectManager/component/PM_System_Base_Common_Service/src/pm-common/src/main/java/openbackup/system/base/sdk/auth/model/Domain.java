/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

/**
 * HSC token 认证字段
 *
 * @author l30044826
 * @since 2023-07-07
 */
@Getter
@Setter
public class Domain {
    private String name;

    private String id;
}
