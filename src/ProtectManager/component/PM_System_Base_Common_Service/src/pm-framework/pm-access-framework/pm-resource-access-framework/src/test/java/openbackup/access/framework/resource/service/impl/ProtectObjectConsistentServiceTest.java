/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.access.framework.resource.service.impl;

import openbackup.access.framework.resource.service.impl.ProtectObjectConsistentServiceImpl;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.access.framework.resource.service.ProtectObjectConsistentService;
import openbackup.access.framework.resource.service.ProtectedResourceRepository;
import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.protection.access.provider.sdk.base.v2.RemotePath;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.system.base.common.enums.ConsistentStatusEnum;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.storage.service.LocalDoradoFileSystemService;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.reflect.Whitebox;

import java.util.Collections;
import java.util.concurrent.TimeUnit;

import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

/**
 * 功能描述: ProtectObjectConsistentServiceTest
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-07
 */
public class ProtectObjectConsistentServiceTest {
    private static ProtectedResourceRepository repository;

    private static LocalDoradoFileSystemService fileSystemService;

    private static CopyRestApi copyRestApi;

    private static DmeUnifiedRestApi dmeUnifiedRestApi;

    private static LockService lockService;

    private static ProtectObjectConsistentService consistentService;

    @BeforeClass
    public static void init() {
        repository = mock(ProtectedResourceRepository.class);
        fileSystemService = mock(LocalDoradoFileSystemService.class);
        copyRestApi = mock(CopyRestApi.class);
        dmeUnifiedRestApi = mock(DmeUnifiedRestApi.class);
        lockService = mock(LockService.class);
        consistentService = new ProtectObjectConsistentServiceImpl(repository, fileSystemService, copyRestApi,
                dmeUnifiedRestApi, lockService);
    }

    /**
     * 用例名称：验证定时刷新保护对象数据一致性标记
     * 前置条件：无
     * 检查点：数据一致性标记是否刷新
     */
    @Test
    public void test_refresh_protect_object_consistent_status_success() {
        when(repository.queryProtectObjectCountByConsistentStatus(ConsistentStatusEnum.UNDETECTED)).thenReturn(1);
        consistentService.refreshProtectObjectConsistentStatus();
        verify(repository, Mockito.times(0)).updateAllProtectObjectConsistentStatus(ConsistentStatusEnum.UNDETECTED);

        when(repository.queryProtectObjectCountByConsistentStatus(ConsistentStatusEnum.UNDETECTED)).thenReturn(0);
        consistentService.refreshProtectObjectConsistentStatus();
        verify(repository, Mockito.times(1)).updateAllProtectObjectConsistentStatus(ConsistentStatusEnum.UNDETECTED);
    }

    /**
     * 用例名称：验证定时检测保护对象数据一致性标记
     * 前置条件：无
     * 检查点：保护对象数据一致性标记是否检测成功
     */
    @Test
    public void test_check_protect_object_consistent_status_success() throws Exception {
        Lock lock = mock(Lock.class);
        when(lockService.createDistributeLock(anyString())).thenReturn(lock);
        when(lock.tryLock(0, TimeUnit.SECONDS)).thenReturn(true);
        when(repository.queryProtectObjectIdListByConsistentStatus(ConsistentStatusEnum.UNDETECTED))
                .thenReturn(Collections.singletonList("test-id"));
        ProtectedObjectPo protectedObject = mockProtectedObject();
        when(repository.queryProtectObjectById("test-id")).thenReturn(protectedObject);
        Copy copy = mockCopy();
        when(copyRestApi.queryLatestBackupCopy(protectedObject.getResourceId(), null, null)).thenReturn(copy);
        when(dmeUnifiedRestApi.getCopyInfo(copy.getUuid())).thenReturn(mockDmeCopyInfo());
        when(fileSystemService.checkFsConsistentStatus("test-fs-id")).thenReturn(ConsistentStatusEnum.CONSISTENT);
        consistentService.checkProtectObjectConsistentStatus(false);
        Whitebox.invokeMethod(consistentService, "doCheck");
        Assert.assertTrue(true);
    }

    private ProtectedObjectPo mockProtectedObject() {
        ProtectedObjectPo protectedObject = new ProtectedObjectPo();
        protectedObject.setSubType(ResourceSubTypeEnum.FUSION_COMPUTE.getType());
        protectedObject.setUuid("test-id");
        protectedObject.setResourceId("test-id");
        return protectedObject;
    }

    private Copy mockCopy() {
        Copy copy = new Copy();
        copy.setUuid("test-copy-id");
        return copy;
    }

    private DmeCopyInfo mockDmeCopyInfo() {
        DmeCopyInfo dmeCopyInfo = new DmeCopyInfo();
        StorageRepository repository = new StorageRepository();
        repository.setLocal(true);
        repository.setType(RepositoryTypeEnum.DATA.getType());
        dmeCopyInfo.setRepositories(Collections.singletonList(repository));
        RemotePath remotePath = new RemotePath();
        remotePath.setType(RepositoryTypeEnum.DATA.getType());
        remotePath.setId("test-fs-id");
        repository.setRemotePath(Collections.singletonList(remotePath));
        return dmeCopyInfo;
    }
}