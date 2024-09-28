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
package openbackup.database.base.plugin.utils;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;

import openbackup.system.base.common.exception.LegoCheckedException;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import java.util.Arrays;
import java.util.List;

/**
 * 存储库相关工具类
 *
 */
public class StorageRepositoryUtilTest {
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * 用例名称：获取data仓
     * 前置条件：有data仓
     * 检查点：获取到的是data仓
     */
    @Test
    public void get_data_storageRepository_success() {
        StorageRepository dataStorageRepository = new StorageRepository();
        dataStorageRepository.setType(RepositoryTypeEnum.DATA.getType());

        StorageRepository metaStorageRepository = new StorageRepository();
        metaStorageRepository.setType(RepositoryTypeEnum.META.getType());

        List<StorageRepository> storageRepositories = Arrays.asList(dataStorageRepository, metaStorageRepository);
        StorageRepository repository = StorageRepositoryUtil.getDataStorageRepository(storageRepositories);
        Assert.assertTrue(repository.getType() == RepositoryTypeEnum.DATA.getType());
    }

    /**
     * 用例名称：获取data仓
     * 前置条件：无data仓
     * 检查点：报错
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_data_storageRepository_when_no_data_storageRepository() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Data repository info is empty!");
        StorageRepository metaStorageRepository = new StorageRepository();
        metaStorageRepository.setType(RepositoryTypeEnum.META.getType());
        List<StorageRepository> storageRepositories = Arrays.asList(metaStorageRepository);
        StorageRepositoryUtil.getDataStorageRepository(storageRepositories);
    }
}
