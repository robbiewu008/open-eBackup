/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.repository;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 存储单元组修改校验扩展接口
 *
 * @author l00853347
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-04-02
 **/
public interface StorageUnitGroupCheckProvider extends DataProtectionProvider<String> {
    /**
     * 存储单元组修改自定义校验逻辑
     *
     * @return false
     */
    default boolean isSupportParallelStorage() {
        return false;
    }
}
