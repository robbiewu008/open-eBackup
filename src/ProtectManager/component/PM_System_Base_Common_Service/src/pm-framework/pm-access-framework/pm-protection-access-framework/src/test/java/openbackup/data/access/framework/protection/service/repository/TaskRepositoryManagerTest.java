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
package openbackup.data.access.framework.protection.service.repository;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.protection.service.repository.strategies.NativeNfsRepositoryStrategy;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryCreateService;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;

import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.DataProtectionParams;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequestParm;
import openbackup.system.base.sdk.protection.emuns.SlaPolicyTypeEnum;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.BackupClusterVo;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * 目标存储集群的Repository获取Manager 测试类
 *
 **/
public class TaskRepositoryManagerTest {
    private final ClusterNativeApi clusterNativeApi = Mockito.mock(ClusterNativeApi.class);

    private final EncryptorService encryptorService = Mockito.mock(EncryptorService.class);

    private final RepositoryStrategyManager repositoryStrategyManager = Mockito.mock(RepositoryStrategyManager.class);

    private final BackupStorageApi backupStorageApi = Mockito.mock(BackupStorageApi.class);

    private final StorageUnitService storageUnitService = Mockito.mock(StorageUnitService.class);

    private final NativeNfsRepositoryStrategy nativeNfsRepositoryStrategy = Mockito.mock(
        NativeNfsRepositoryStrategy.class);

    private final StorageRepositoryCreateService storageRepositoryCreateService = Mockito.mock(
        StorageRepositoryCreateService.class);

    private final TaskRepositoryManager taskRepositoryManager = new TaskRepositoryManager(clusterNativeApi,
        backupStorageApi, encryptorService, repositoryStrategyManager, storageRepositoryCreateService, storageUnitService);

    /**
     * 用例场景：校验无storageId情况下构造空对象
     * 前置条件：无storageId
     * 检查点：成功
     */
    @Test
    public void check_no_storage_id_build_success() {
        Assert.assertEquals(0,
            taskRepositoryManager.buildTargetRepositories("", SlaPolicyTypeEnum.BACKUP.getName()).size());
    }

    /**
     * 用例场景：校验有storageId情况下正确返回
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void check_exist_storage_id_build_success() throws IllegalAccessException {
        PowerMockito.when(backupStorageApi.getDetail("123456")).thenReturn(getNasDistributionStorageDetail());
        BackupClusterVo backupClusterVo = new BackupClusterVo();
        backupClusterVo.setAvailableCapacityRatio(2);
        backupClusterVo.setUsedCapacity(BigDecimal.ONE);
        backupClusterVo.setCapacity(BigDecimal.TEN);
        PowerMockito.when(backupStorageApi.getClusterStorage(Mockito.anyString(), Mockito.anyInt()))
            .thenReturn(backupClusterVo);
        PowerMockito.when(clusterNativeApi.queryTargetClusterListDetails(
            new TargetClusterRequestParm(Collections.singletonList(10000)))).thenReturn(getClusterDetailInfo());
        PowerMockito.when(clusterNativeApi.getCurrentEsn()).thenReturn("123456");
        PowerMockito.when(repositoryStrategyManager.getStrategy(RepositoryProtocolEnum.NATIVE_NFS))
            .thenReturn(nativeNfsRepositoryStrategy);
        StorageRepository repo = new StorageRepository();
        repo.setType(RepositoryTypeEnum.DATA.getType());
        PowerMockito.when(nativeNfsRepositoryStrategy.getRepository(any())).thenReturn(repo);
        MemberModifier.field(TaskRepositoryManager.class, "repositoryStrategyManager")
            .set(taskRepositoryManager, repositoryStrategyManager);
        PowerMockito.when(encryptorService.decrypt(
            "AAAAAgAAAAAAAAAAAAAAAQAAAAk8jDbAZpwA6b28kggrrvvDVewywTfg+I5MpjL9AAAAAAAAAAAAAAAAAAAAGU/Nb0x/27hS75VsTEIfdqTfxZbrbTyGNms="))
            .thenReturn("Admin@123");
        Assert.assertEquals(1,
            taskRepositoryManager.buildTargetRepositories("123456", SlaPolicyTypeEnum.BACKUP.getName()).size());
    }

    /**
     * 用例场景：容量未超过限制
     * 前置条件：使用率低于90%
     * 检查点：成功
     */
    @Test
    public void testCapacityLimitExceeded()
            throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        BackupClusterVo backupClusterVo = new BackupClusterVo();
        backupClusterVo.setUsedCapacity(new BigDecimal(150));
        backupClusterVo.setCapacity(new BigDecimal(200));
        backupClusterVo.setAvailableCapacityRatio(90);
        Method method = TaskRepositoryManager.class.getDeclaredMethod("capacityLimitExceeded", BackupClusterVo.class);
        method.setAccessible(true);
        boolean result = (Boolean) method.invoke(taskRepositoryManager, backupClusterVo);
        assertTrue(result);
    }

    /**
     * 用例场景：容量超过限制
     * 前置条件：使用率高于90%
     * 检查点：False
     */
    @Test
    public void testCapacityLimitExceededFalse()
            throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        BackupClusterVo backupClusterVo = new BackupClusterVo();
        backupClusterVo.setUsedCapacity(new BigDecimal(190));
        backupClusterVo.setCapacity(new BigDecimal(200));
        backupClusterVo.setAvailableCapacityRatio(90);
        Method method = TaskRepositoryManager.class.getDeclaredMethod("capacityLimitExceeded", BackupClusterVo.class);
        method.setAccessible(true);
        boolean result = (Boolean) method.invoke(taskRepositoryManager, backupClusterVo);
        assertFalse(result);
    }

    /**
     * 用例场景：校验查询离线集群下场景
     * 前置条件：查询集群为离线
     * 检查点：集群都为离线情况下不返回任何集群
     */
    @Test
    public void check_cluster_list_is_offline_success() {
        NasDistributionStorageDetail nasDistributionStorageDetail = getNasDistributionStorageDetail();
        nasDistributionStorageDetail.getUnitList().get(0).getBackupClusterVo()
            .setStatus(ClusterEnum.StatusEnum.OFFLINE.getStatus());
        PowerMockito.when(backupStorageApi.getDetail("123456")).thenReturn(nasDistributionStorageDetail);
        Assert.assertEquals(0,
            taskRepositoryManager.buildTargetRepositories("123456", SlaPolicyTypeEnum.BACKUP.getName()).size());
    }

    /**
     * 用例场景：为本地集群增加容量阈值
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void check_add_repository_capacity_success() {
        NasDistributionStorageDetail nasDistributionStorageDetail = getNasDistributionStorageDetail();
        nasDistributionStorageDetail.getUnitList()
            .get(0)
            .getBackupClusterVo()
            .setRole(ClusterEnum.RoleType.MANAGEMENT.getRoleType());
        PowerMockito.when(backupStorageApi.getDetail("123456")).thenReturn(nasDistributionStorageDetail);
        BackupClusterVo backupClusterVo = new BackupClusterVo();
        backupClusterVo.setAvailableCapacityRatio(2);
        backupClusterVo.setUsedCapacity(BigDecimal.ONE);
        backupClusterVo.setCapacity(BigDecimal.TEN);
        PowerMockito.when(backupStorageApi.getClusterStorage("123456", 10000))
            .thenReturn(backupClusterVo);
        StorageRepository storageRepository = new StorageRepository();
        HashMap<String, Object> objectObjectHashMap = new HashMap<>();
        objectObjectHashMap.put(StorageRepository.REPOSITORIES_KEY_ENS, "2102354DEY10M3000002");
        storageRepository.setExtendInfo(objectObjectHashMap);
        taskRepositoryManager.addRepositoryCapacity(storageRepository, "123456");
        Mockito.verify(backupStorageApi, Mockito.times(1)).getDetail(any());
    }

    /**
     * 用例场景：查询不到本地集群
     * 前置条件：无
     * 检查点：失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_no_find_local_clusterwhen_add_repository_capacity() {
        PowerMockito.when(backupStorageApi.getDetail("123456")).thenReturn(getNasDistributionStorageDetail());
        StorageRepository storageRepository = new StorageRepository();
        HashMap<String, Object> objectObjectHashMap = new HashMap<>();
        objectObjectHashMap.put(StorageRepository.REPOSITORIES_KEY_ENS, "21");
        storageRepository.setExtendInfo(objectObjectHashMap);
        Assert.assertThrows(LegoCheckedException.class,
            () -> taskRepositoryManager.addRepositoryCapacity(storageRepository, "123456"));
    }

    public static NasDistributionStorageDetail getNasDistributionStorageDetail() {
        NasDistributionStorageDetail nasDistributionStorageDetail = new NasDistributionStorageDetail();
        nasDistributionStorageDetail.setUnitList(new ArrayList<>());
        BackupUnitVo backupUnitVo = new BackupUnitVo();
        BackupClusterVo clustersInfoVo = new BackupClusterVo();
        clustersInfoVo.setIp("192.168.100.100");
        clustersInfoVo.setPort(9527);
        clustersInfoVo.setGeneratedType(1);
        clustersInfoVo.setClusterId(10000);
        clustersInfoVo.setStorageEsn("2102354DEY10M3000002");
        clustersInfoVo.setStatus(ClusterEnum.StatusEnum.ONLINE.getStatus());
        backupUnitVo.setBackupClusterVo(clustersInfoVo);
        nasDistributionStorageDetail.getUnitList().add(backupUnitVo);
        return nasDistributionStorageDetail;
    }

    public static List<ClusterDetailInfo> getClusterDetailInfo() {
        List<ClusterDetailInfo> clusterDetailInfos = new ArrayList<>();
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        StorageSystemInfo storageSystemInfo = Optional.ofNullable(clusterDetailInfo.getStorageSystem())
            .orElse(new StorageSystemInfo());
        SourceClustersParams sourceClustersParams = Optional.ofNullable(clusterDetailInfo.getSourceClusters())
            .orElse(new SourceClustersParams());
        List<DataProtectionParams> dataProtectionParams = Optional.ofNullable(
            clusterDetailInfo.getDataProtectionEngines()).orElse(new ArrayList<>());
        sourceClustersParams.setStorageDisplayIps("192.168.100.102");
        storageSystemInfo.setStorageEsn("2102354DEY10M3000002");
        storageSystemInfo.setPassword(
            "AAAAAgAAAAAAAAAAAAAAAQAAAAk8jDbAZpwA6b28kggrrvvDVewywTfg+I5MpjL9AAAAAAAAAAAAAAAAAAAAGU/Nb0x/27hS75VsTEIfdqTfxZbrbTyGNms=");
        storageSystemInfo.setStoragePort(8088);
        storageSystemInfo.setUsername("admin");
        clusterDetailInfo.setStorageSystem(storageSystemInfo);
        clusterDetailInfo.setSourceClusters(sourceClustersParams);
        clusterDetailInfo.setDataProtectionEngines(dataProtectionParams);
        clusterDetailInfos.add(clusterDetailInfo);
        return clusterDetailInfos;
    }
}
