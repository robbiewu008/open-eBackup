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
package openbackup.data.access.framework.protection.service.archive;

import static org.mockito.BDDMockito.given;

import com.huawei.oceanprotect.base.cluster.sdk.dto.ClusterAuthInfo;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import openbackup.data.access.framework.protection.mocks.RepositoryMocker;

import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.access.framework.protection.service.repository.strategies.NativeNfsRepositoryStrategy;
import openbackup.data.access.framework.protection.service.repository.strategies.S3RepositoryStrategy;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.repository.s3.entity.S3Storage;
import com.huawei.oceanprotect.repository.s3.service.S3StorageService;
import com.huawei.oceanprotect.system.base.cert.service.ObjectCertDependencyService;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.session.IStorageDeviceRepository;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.StorageDevice;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.BackupClusterVo;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.List;

/**
 * 归档认证测试类
 *
 */
public class ArchiveAuthServiceTest {
    private final ClusterNativeApi clusterNativeApi = Mockito.mock(ClusterNativeApi.class);

    private final S3StorageService s3StorageService = Mockito.mock(S3StorageService.class);

    private final RepositoryStrategyManager repositoryStrategyManager = Mockito.mock(RepositoryStrategyManager.class);

    private final EncryptorService encryptorService = Mockito.mock(EncryptorService.class);

    private final BackupStorageApi storageService = Mockito.mock(BackupStorageApi.class);

    private final IStorageDeviceRepository repository =  Mockito.mock(IStorageDeviceRepository.class);

    private final ObjectCertDependencyService objectCertDependencyService = Mockito.mock(
        ObjectCertDependencyService.class);

    private final ClusterQueryService clusterQueryService = Mockito.mock(ClusterQueryService.class);

    private final ArchiveRepositoryService archiveAuthService = new ArchiveRepositoryService(repositoryStrategyManager,
        clusterNativeApi, clusterQueryService, encryptorService, storageService);

    /**
     * 用例名称：获得本地存储信息
     * 前置条件：无
     * check点：1.获得本地存储信息
     */
    @Test
    public void build_local_storage_repository_success() {
        final ClusterDetailInfo localCluster = RepositoryMocker.mockLocalClusterInfo();
        given(clusterNativeApi.queryDecryptCurrentGroupClusterDetails()).willReturn(localCluster);
        given(repositoryStrategyManager.getStrategy(Mockito.any())).willReturn(
            new NativeNfsRepositoryStrategy(clusterNativeApi, repository));
        StorageDevice storageDevice = new StorageDevice();
        storageDevice.setUserName("name");
        storageDevice.setPassword("pwd");
        given(repository.findLocalStorage(true)).willReturn(storageDevice);
        StorageRepository result = archiveAuthService.buildLocalStorageRepository();
        Assert.assertEquals("XLFDI303821JND-1823", result.getId());
    }

    /**
     * 用例名称：通过esn获取存储信息
     * 前置条件：无
     * check点：1.获得存储信息
     */
    @Test
    public void build_storage_repository_by_copy_esn_success() {
        TaskRepositoryManager taskRepositoryManager = Mockito.mock(TaskRepositoryManager.class);
        archiveAuthService.setTaskRepositoryManager(taskRepositoryManager);
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setExtendAuth(new Authentication());
        storageRepository.getExtendAuth().setAuthPwd("depwd");
        Mockito.when(taskRepositoryManager.buildStorageRepository(Mockito.anyString(),Mockito.any(),Mockito.any()))
            .thenReturn(storageRepository);

        ClusterAuthInfo clusterAuthInfo = new ClusterAuthInfo();
        clusterAuthInfo.setPassword("pwd");
        clusterAuthInfo.setUserName("name");
        clusterAuthInfo.setIp("8.40.x.x");
        clusterAuthInfo.setPort(8088);
        PowerMockito.when(clusterQueryService.getClusterAuthInfoByEsn(Mockito.anyString())).thenReturn(clusterAuthInfo);
        PowerMockito.when(encryptorService.decrypt(Mockito.anyString())).thenReturn("depwd");
        StorageRepository result = archiveAuthService.buildStorageRepositoryByCopyEsn("esn");
        Assert.assertEquals(result.getExtendAuth().getAuthPwd(), "depwd");
    }

    /**
     * 用例名称：获取归档任务对应存储库信息
     * 前置条件：无
     * check点：1.获得存储信息
     */
    @Test
    public void query_repository_success() {
        given(repositoryStrategyManager.getStrategy(Mockito.any())).willReturn(
            new S3RepositoryStrategy(s3StorageService, objectCertDependencyService));
        given(s3StorageService.queryS3Storage(Mockito.any())).willReturn(new S3Storage());
        StorageRepository result = archiveAuthService.queryRepository("adgadg", RepositoryProtocolEnum.S3);
        Assert.assertEquals("adgadg", result.getId());
    }

    /**
     * 用例名称：获得存储信息列表
     * 前置条件：无
     * check点：1.获得存储信息
     */
    @Test
    public void build_sub_repositoryList_success() {
        Mockito.when(storageService.getDetail(Mockito.anyString())).thenReturn(nasDistributionStorageDetailInit());
        Mockito.when(clusterNativeApi.queryTargetClusterListDetails(Mockito.any()))
            .thenReturn(Lists.newArrayList(clusterDetailInit()));
        Mockito.when(encryptorService.decrypt(Mockito.anyString())).thenReturn("222");
        List<StorageRepository> result = archiveAuthService.buildSubRepositoryList("22222");
        Assert.assertEquals(1, result.size());
    }

    private NasDistributionStorageDetail nasDistributionStorageDetailInit() {
        NasDistributionStorageDetail storageDetail = new NasDistributionStorageDetail();
        storageDetail.setUnitList(Lists.newArrayList(backupClusterVoInit()));
        return storageDetail;
    }

    private BackupUnitVo backupClusterVoInit() {
        BackupUnitVo backupUnitVo = new BackupUnitVo();
        BackupClusterVo backupClusterVo = new BackupClusterVo();
        backupClusterVo.setStatus(ClusterEnum.StatusEnum.ONLINE.getStatus());
        backupClusterVo.setClusterId(2);
        backupClusterVo.setStorageEsn("esn");
        backupClusterVo.setClusterIp("127.0.0.1");
        backupClusterVo.setPort(8088);
        backupUnitVo.setBackupClusterVo(backupClusterVo);
        return backupUnitVo;
    }

    private ClusterDetailInfo clusterDetailInit() {
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        StorageSystemInfo storageSystem = new StorageSystemInfo();
        storageSystem.setPassword("222");
        clusterDetailInfo.setStorageSystem(storageSystem);
        return clusterDetailInfo;
    }
}
