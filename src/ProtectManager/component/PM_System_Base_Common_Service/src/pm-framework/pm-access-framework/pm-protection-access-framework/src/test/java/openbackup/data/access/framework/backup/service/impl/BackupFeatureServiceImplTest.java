/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.access.framework.backup.service.impl;

import openbackup.data.access.framework.backup.service.impl.BackupFeatureServiceImpl;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Optional;

/**
 * BackupFeatureServiceImpl测试类
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/6/6
 */
@RunWith(SpringRunner.class)
@PrepareForTest({BackupFeatureServiceImpl.class})
@SpringBootTest(classes = BackupFeatureServiceImpl.class)
public class BackupFeatureServiceImplTest {
    @Autowired
    private BackupFeatureServiceImpl backupFeatureService;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private ProviderManager providerManager;

    /**
     * interceptor为空时，不支持并行备份
     */
    @Test
    public void check_false_when_interceptor_is_empty() {
        ProtectedResource protectedResource = new ProtectedResource();

        Mockito.when(providerManager.findProvider(Mockito.any(), Mockito.any(), Mockito.eq(null))).thenReturn(null);
        boolean supportDataAndLogParallelBackup =
            backupFeatureService.isSupportDataAndLogParallelBackup(protectedResource);

        Assert.assertFalse(supportDataAndLogParallelBackup);
    }

    /**
     * interceptor不为空，且配置为true时，支持并行备份
     */
    @Test
    public void check_true_when_interceptor_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("11");
        Mockito.when(resourceService.getBasicResourceById("11")).thenReturn(Optional.of(protectedResource));

        BackupInterceptorProvider backupInterceptorProvider = Mockito.mock(BackupInterceptorProvider.class);
        Mockito.when(backupInterceptorProvider.isSupportDataAndLogParallelBackup(protectedResource)).thenReturn(true);

        Mockito.when(providerManager.findProvider(Mockito.any(), Mockito.any(), Mockito.eq(null)))
            .thenReturn(backupInterceptorProvider);
        boolean supportDataAndLogParallelBackup = backupFeatureService.isSupportDataAndLogParallelBackup("11");

        Assert.assertTrue(supportDataAndLogParallelBackup);
    }

    /**
     * interceptor为空时，备份类型不转化
     */
    @Test
    public void transfer_the_same_when_interceptor_is_empty() {
        ProtectedResource protectedResource = new ProtectedResource();

        Mockito.when(providerManager.findProvider(Mockito.any(), Mockito.any(), Mockito.eq(null))).thenReturn(null);

        BackupTypeConstants backupTypeConstants =
            backupFeatureService.transferBackupType(BackupTypeConstants.LOG, protectedResource);

        Assert.assertEquals(backupTypeConstants, BackupTypeConstants.LOG);
    }

    /**
     * interceptor不为空时，备份类型转化为插件定义的类型
     */
    @Test
    public void transfer_the_other_when_interceptor_is_not_empty() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("11");
        Mockito.when(resourceService.getBasicResourceById("11")).thenReturn(Optional.of(protectedResource));

        BackupInterceptorProvider backupInterceptorProvider = Mockito.mock(BackupInterceptorProvider.class);
        Mockito.when(backupInterceptorProvider.transferBackupType(BackupTypeConstants.LOG, protectedResource))
            .thenReturn(BackupTypeConstants.FULL);

        Mockito.when(providerManager.findProvider(Mockito.any(), Mockito.any(), Mockito.eq(null)))
            .thenReturn(backupInterceptorProvider);
        BackupTypeConstants backupTypeConstants =
            backupFeatureService.transferBackupType(BackupTypeConstants.LOG, "11");

        Assert.assertEquals(backupTypeConstants, BackupTypeConstants.FULL);
    }
}
