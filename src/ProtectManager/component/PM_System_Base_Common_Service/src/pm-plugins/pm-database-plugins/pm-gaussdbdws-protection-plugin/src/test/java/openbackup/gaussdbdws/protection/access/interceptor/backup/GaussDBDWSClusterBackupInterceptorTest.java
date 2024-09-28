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
package openbackup.gaussdbdws.protection.access.interceptor.backup;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import openbackup.system.base.common.constants.LocalStorageInfoRes;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequestParm;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.Optional;

/**
 * DWS集群环境测试类
 *
 */
@RunWith(PowerMockRunner.class)
public class GaussDBDWSClusterBackupInterceptorTest {
    private GaussDBDWSClusterBackupInterceptor gaussDBDWSClusterBackupInterceptor;

    private ProtectedResourceChecker protectedResourceChecker;

    private ResourceService resourceService;

    private LocalStorageService localStorageService;

    private BackupStorageApi backupStorageApi;

    private GaussDBBaseService gaussDBBaseService;

    private ProviderManager providerManager;

    private ClusterNativeApi clusterNativeApi;

    private EncryptorService encryptorService;

    private ClusterBasicService clusterBasicService;

    private DeployTypeService deployTypeService;

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = Mockito.mock(
        ResourceConnectionCheckProvider.class);

    private final TaskRepositoryManager taskRepositoryManager = Mockito.mock(TaskRepositoryManager.class);

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() throws IllegalAccessException {
        this.protectedResourceChecker = Mockito.mock(ProtectedResourceChecker.class);
        this.resourceService = Mockito.mock(ResourceService.class);
        this.localStorageService = Mockito.mock(LocalStorageService.class);
        this.backupStorageApi = Mockito.mock(BackupStorageApi.class);
        this.providerManager = Mockito.mock(ProviderManager.class);
        this.clusterNativeApi = Mockito.mock(ClusterNativeApi.class);
        this.encryptorService = Mockito.mock(EncryptorService.class);
        this.gaussDBBaseService = new GaussDBBaseService(resourceService, protectedResourceChecker, providerManager,
            resourceConnectionCheckProvider, taskRepositoryManager);
        this.gaussDBDWSClusterBackupInterceptor = new GaussDBDWSClusterBackupInterceptor(localStorageService,
            resourceService, this.gaussDBBaseService, this.clusterBasicService, this.deployTypeService);

        LocalStorageInfoRes localStorageInfoRes = new LocalStorageInfoRes();
        localStorageInfoRes.setEsn("xxxxxxxxxxxxxxxxx");
        PowerMockito.when(localStorageService.getStorageInfo()).thenReturn(localStorageInfoRes);
        MemberModifier.field(GaussDBBaseService.class, "taskRepositoryManager")
            .set(gaussDBBaseService, taskRepositoryManager);
    }

    /**
     * 用例场景：GaussDB(DWS)集群 下发备份任务 applicable 校验
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(gaussDBDWSClusterBackupInterceptor.applicable(ResourceSubTypeEnum.GAUSSDB_DWS.getType()));
    }

    /**
     * 用例场景：GaussDB(DWS)集群 更新task信息无异常
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void supply_backup_task_success() {
        BackupTask backupTask = MockInterceptorParameter.getBackupTask();
        backupTask.setAdvanceParams(new HashMap<>());
        backupTask.getAdvanceParams().put(DwsConstant.ADVANCE_PARAMS_KEY_STORAGE_ID, "123456");
        backupTask.getAdvanceParams().put(DwsConstant.ADVANCE_PARAMS_KEY_BACKUP_METADATA_PATH, "/opt/share");
        backupTask.getAdvanceParams().put(DwsConstant.ADVANCE_PARAMS_KEY_BACKUP_TOOL_TYPE, "1");
        PowerMockito.when(backupStorageApi.getDetail("123456"))
            .thenReturn(MockInterceptorParameter.getNasDistributionStorageDetail());
        Optional<ProtectedResource> resourceOptional = Optional.of(MockInterceptorParameter.getProtectedEnvironment());
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("cccccccc")).thenReturn(resourceOptional);
        PowerMockito.when(clusterNativeApi.queryTargetClusterListDetails(
                new TargetClusterRequestParm(Collections.singletonList(10000))))
            .thenReturn(MockInterceptorParameter.getClusterDetailInfo());
        PowerMockito.when(encryptorService.decrypt(
                "AAAAAgAAAAAAAAAAAAAAAQAAAAk8jDbAZpwA6b28kggrrvvDVewywTfg+I5MpjL9AAAAAAAAAAAAAAAAAAAAGU/Nb0x/27hS75VsTEIfdqTfxZbrbTyGNms="))
            .thenReturn("Admin@123");
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setLinkStatus("1");
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("aaaaaaaaaaaaaaaaa"))
            .thenReturn(Optional.of(protectedEnvironment));
        gaussDBDWSClusterBackupInterceptor.supplyBackupTask(backupTask);
    }

    /**
     * 用例场景：GaussDB(DWS)集群 备份没有下发 环境uuid
     * 前置条件：无 对应的 backupTask.getProtectEnv().getUuid()
     * 检查点：获取失败
     */
    @Test
    public void no_get_environment_fail() {
        expectedException.expect(LegoCheckedException.class);
        BackupTask backupTask = MockInterceptorParameter.getBackupTask();
        gaussDBDWSClusterBackupInterceptor.supplyBackupTask(backupTask);
    }

    /**
     * 用例场景：GaussDB(DWS)集群 任务无nodes里面无主机的Agent
     * 前置条件：匹配不到
     * 检查点：失败
     */
    @Test
    public void no_find_task_environment_fail() {
        expectedException.expect(LegoCheckedException.class);
        BackupTask backupTask = MockInterceptorParameter.getBackupTask();
        backupTask.getProtectEnv().getNodes().add(new TaskEnvironment());
        Optional<ProtectedResource> resourceOptional = Optional.of(MockInterceptorParameter.getProtectedEnvironment());
        PowerMockito.when(resourceService.getResourceById("aaaaaaaaaaaaaaaaa")).thenReturn(resourceOptional);
        gaussDBDWSClusterBackupInterceptor.supplyBackupTask(backupTask);
    }
}
