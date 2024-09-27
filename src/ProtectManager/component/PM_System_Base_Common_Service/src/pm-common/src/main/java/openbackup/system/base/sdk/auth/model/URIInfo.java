/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Data;

import java.net.URI;

/**
 * uri和reginId信息
 *
 * @author l30044826
 * @since 2023-07-11
 */
@Data
public class URIInfo {
    private URI uri;

    private String regionId;
}
