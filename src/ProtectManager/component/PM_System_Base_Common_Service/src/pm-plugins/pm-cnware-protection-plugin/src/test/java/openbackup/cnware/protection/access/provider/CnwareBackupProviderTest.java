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
package openbackup.cnware.protection.access.provider;

import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.cnware.protection.access.service.CnwareCommonService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.cluster.model.ClustersInfoVo;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述 CnwareBackupProviderTest
 *
 */
public class CnwareBackupProviderTest {
    private static CnwareBackupProvider backupProvider;

    private static final CnwareCommonService cnwareCommonService = Mockito.mock(CnwareCommonService.class);

    private static final ResourceService resourceService = Mockito.mock(ResourceService.class);

    @BeforeClass
    public static void init() {
        backupProvider = new CnwareBackupProvider(resourceService, cnwareCommonService);
    }

    /**
     * 用例场景：Cnware备份插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        boolean cNwareVm = backupProvider.applicable("CNwareVm");
        Assert.assertTrue(cNwareVm);
    }

    /**
     * 用例场景：Cnware备份对象参数转换 <br/>
     * 前置条件：Cnware备份对象参数正确 <br/>
     * 检查点：无异常，参数补充正确
     */
    @Test
    public void test_backup_intercept_success() {
        BackupTask backupTask = mockCnWareBackupTask();
        ProtectedResource domain = MockFactory.mockProtectedResource();
        domain.setAuth(new Authentication());
        Mockito.when(resourceService.getResourceById(ArgumentMatchers.anyBoolean(), ArgumentMatchers.any()))
            .thenReturn(Optional.of(domain));

        BackupTask interceptTask = backupProvider.initialize(backupTask);
        Assert.assertEquals(0, interceptTask.getProtectSubObjects().size());
        Assert.assertNull(interceptTask.getProtectObject().getAuth());
        Assert.assertEquals(2, interceptTask.getRepositories().size());
    }

    /**
     * 用例场景：Openstack备份下发全部磁盘 <br/>
     * 前置条件：保护对象高级参数全部磁盘参数为true <br/>
     * 检查点：下发的磁盘信息为空，其余参数正常
     */
    @Test
    public void test_backup_intercept_success_if_all_disk_is_true() {
        BackupTask backupTask = mockCnWareBackupTask();
        backupTask.getAdvanceParams().put(CnwareConstant.ALL_DISK, "true");
        ClustersInfoVo localClusterVoInfo = new ClustersInfoVo();
        localClusterVoInfo.setStorageEsn("esn_01");
        Mockito.when(resourceService.getResourceById(ArgumentMatchers.anyBoolean(), ArgumentMatchers.any()))
            .thenReturn(Optional.of(MockFactory.mockProtectedResource()));

        BackupTask interceptTask = backupProvider.initialize(backupTask);
        Assert.assertEquals(2, interceptTask.getRepositories().size());
    }

    private BackupTask mockCnWareBackupTask() {
        BackupTask backupTask = new BackupTask();

        TaskResource protectObject = new TaskResource();
        protectObject.setUuid(UUIDGenerator.getUUID());
        protectObject.setParentUuid(UUIDGenerator.getUUID());
        Map<String, String> proExtendInfo = new HashMap<>();
        proExtendInfo.put(CnwareConstant.DOMAIN_ID_KEY, "default");
        protectObject.setExtendInfo(proExtendInfo);
        backupTask.setProtectObject(protectObject);

        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put(CnwareConstant.DISK_INFO, "[]");
        advanceParams.put(CnwareConstant.ALL_DISK, "ture");
        backupTask.setAdvanceParams(advanceParams);

        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        TaskEnvironment taskEnvironment = BeanTools.copy(environment, TaskEnvironment::new);
        backupTask.setProtectEnv(taskEnvironment);

        StorageRepository dataRepository = new StorageRepository();
        dataRepository.setType(RepositoryTypeEnum.DATA.getType());
        List<StorageRepository> repositoryList = new ArrayList<>();
        repositoryList.add(dataRepository);
        backupTask.setRepositories(repositoryList);
        return backupTask;
    }



    /**
     * 用例场景：测试当sla中高级配置中的生产存储剩余容量阈值，在备份参数中的值是否正确
     * 前置条件：生产存储剩余容量阈值的范围为0-100
     * 检查点：备份参数中的值正确
     */
    @Test
    public void testFillAvailableCapacityThresholdShouldSuccess()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        Class<CnwareBackupProvider> providerClass = CnwareBackupProvider.class;
        BackupTask backupTask = new BackupTask();
        backupTask.setTaskId("123");
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("456");
        backupTask.setProtectObject(taskResource);
        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put("available_capacity_threshold", "10");
        backupTask.setAdvanceParams(advanceParams);
        Method privateMethod = providerClass.getDeclaredMethod("fillAvailableCapacityThreshold", BackupTask.class);
        privateMethod.setAccessible(true);
        privateMethod.invoke(backupProvider, backupTask);
        Assert.assertEquals(backupTask.getAdvanceParams().get("available_capacity_threshold"), "10");
    }

    /**
     * 用例场景：测试当sla中高级配置中的生产存储剩余容量阈值，在备份参数中的值是否正确
     * 前置条件：生产存储剩余容量阈值的范围不为0-100
     * 检查点：备份参数中的值为默认值20
     */
    @Test
    public void testFillAvailableCapacityThresholdShouldSetTwenty()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        Class<CnwareBackupProvider> providerClass = CnwareBackupProvider.class;
        BackupTask backupTask = new BackupTask();
        backupTask.setTaskId("123");
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("456");
        backupTask.setProtectObject(taskResource);
        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put("available_capacity_threshold", "101");
        backupTask.setAdvanceParams(advanceParams);
        Method privateMethod = providerClass.getDeclaredMethod("fillAvailableCapacityThreshold", BackupTask.class);
        privateMethod.setAccessible(true);
        privateMethod.invoke(backupProvider, backupTask);
        Assert.assertEquals(backupTask.getAdvanceParams().get("available_capacity_threshold"), "20");
    }

    /**
     * 用例场景：测试当sla中高级配置中的生产存储剩余容量阈值，在备份参数中的值是否正确
     * 前置条件：生产存储剩余容量阈值未填
     * 检查点：备份参数中的值为默认值20
     */
    @Test
    public void testFillAvailableCapacityThresholdShouldSetDefaultTwenty()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        Class<CnwareBackupProvider> providerClass = CnwareBackupProvider.class;
        BackupTask backupTask = new BackupTask();
        backupTask.setTaskId("123");
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("456");
        backupTask.setProtectObject(taskResource);
        Map<String, String> advanceParams = new HashMap<>();
        backupTask.setAdvanceParams(advanceParams);
        Method privateMethod = providerClass.getDeclaredMethod("fillAvailableCapacityThreshold", BackupTask.class);
        privateMethod.setAccessible(true);
        privateMethod.invoke(backupProvider, backupTask);
        Assert.assertEquals(backupTask.getAdvanceParams().get("available_capacity_threshold"), "20");
    }

}
