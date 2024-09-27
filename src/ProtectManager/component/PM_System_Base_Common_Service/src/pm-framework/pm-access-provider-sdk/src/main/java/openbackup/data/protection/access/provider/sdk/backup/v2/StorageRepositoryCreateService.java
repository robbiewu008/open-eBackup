/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.backup.v2;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;

import java.util.List;

/**
 * 构造存储仓（下发给UBC的参数），通过存储单元（组）
 *
 * @author w30044259
 * @since 2024-03-26
 */
public interface StorageRepositoryCreateService {
    /**
     * 构造存储仓，通过存储单元
     * protocol: 默认设置为RepositoryProtocolEnum.NATIVE_NFS
     * type： 默认设置为RepositoryTypeEnum.DATA
     *
     * @param storageUnitId 存储单元ID
     * @return 存储仓
     */
    StorageRepository createRepositoryByStorageUnit(String storageUnitId);

    /**
     * 构造存储仓，通过存储单元
     *
     * @param groupId 存储单元组ID
     * @return 存储仓
     */
    List<StorageRepository> createRepositoryByStorageUnitGroup(String groupId);
}
