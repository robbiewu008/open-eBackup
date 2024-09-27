/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.plugin;

import lombok.Data;

/**
 *  收集的配置
 *
 * @since 2022-05-23
 */
@Data
public class CollectableConfig {
    private String resource;

    private String environment;
}
