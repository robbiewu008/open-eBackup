/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.index.v2;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 副本索引的provider,该类定义资源副本是否支持索引的接口。不用的应用根据需要创建
 *
 * @author lWX776769
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-14
 */
public interface CopyIndexProvider extends DataProtectionProvider<String> {
    /**
     * 资源是否支持对副本创建索引
     *
     * @return 支持返回true，不支持返回false
     */
    boolean isSupportIndex();
}
