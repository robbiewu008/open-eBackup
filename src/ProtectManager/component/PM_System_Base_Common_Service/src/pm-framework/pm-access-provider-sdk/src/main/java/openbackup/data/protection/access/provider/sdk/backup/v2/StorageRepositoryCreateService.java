/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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
