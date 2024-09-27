/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/5
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
