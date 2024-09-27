/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Getter;
import lombok.Setter;

/**
 * 标签
 *
 * @author l30057246
 * @since 2024-08-07
 */
@Getter
@Setter
public class Label {
    // 标签UUID
    private String uuid;

    // 标签名称
    private String name;
}
