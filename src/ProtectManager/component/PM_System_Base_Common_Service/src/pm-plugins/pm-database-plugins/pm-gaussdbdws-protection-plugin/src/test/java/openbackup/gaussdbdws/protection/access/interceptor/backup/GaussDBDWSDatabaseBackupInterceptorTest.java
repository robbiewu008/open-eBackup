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
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.provider.GaussDBDWSAgentProvider;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import openbackup.system.base.common.constants.LocalStorageInfoRes;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;

import java.util.HashMap;
import java.util.Optional;

/**
 * DWS集群环境测试类
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-17
 */
public class GaussDBDWSDatabaseBackupInterceptorTest {
    private GaussDBDWSDatabaseBackupInterceptor gaussDBDWSDatabaseBackupInterceptor;

    private ProtectedResourceChecker protectedResourceChecker;

    private ResourceService resourceService;

    private LocalStorageService localStorageService;

    private BackupStorageApi backupStorageApi;

    private GaussDBDWSAgentProvider agentProvider;

    /**
     * providerManager
     */
    private ProviderManager providerManager;

    private GaussDBBaseService gaussDBBaseService;

    private ClusterNativeApi clusterNativeApi;

    private EncryptorService encryptorService;

    private ClusterBasicService clusterBasicService;

    private DeployTypeService deployTypeService;

    private final TaskRepositoryManager taskRepositoryManager = Mockito.mock(TaskRepositoryManager.class);

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = Mockito.mock(
        ResourceConnectionCheckProvider.class);

    @Before
    public void init() throws IllegalAccessException {
        this.protectedResourceChecker = Mockito.mock(ProtectedResourceChecker.class);
        this.resourceService = Mockito.mock(ResourceService.class);
        this.localStorageService = Mockito.mock(LocalStorageService.class);
        this.agentProvider = Mockito.mock(GaussDBDWSAgentProvider.class);
        this.backupStorageApi = Mockito.mock(BackupStorageApi.class);
        this.providerManager = Mockito.mock(ProviderManager.class);
        this.clusterNativeApi = Mockito.mock(ClusterNativeApi.class);
        this.encryptorService = Mockito.mock(EncryptorService.class);
        this.backupStorageApi = Mockito.mock(BackupStorageApi.class);
        this.clusterBasicService = Mockito.mock(ClusterBasicService.class);
        this.deployTypeService = Mockito.mock(DeployTypeService.class);
        this.gaussDBBaseService = new GaussDBBaseService(resourceService, protectedResourceChecker, providerManager,
            resourceConnectionCheckProvider, taskRepositoryManager);
        this.gaussDBDWSDatabaseBackupInterceptor = new GaussDBDWSDatabaseBackupInterceptor(localStorageService,
            agentProvider, gaussDBBaseService, clusterBasicService, deployTypeService);
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
        Assert.assertTrue(
            gaussDBDWSDatabaseBackupInterceptor.applicable(ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType()));
    }

    /**
     * 用例场景：GaussDB(DWS)集群 database 备份 下发更新task信息无异常
     * 前置条件：无
     * 检查点：成功
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
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("cccccccc"))
            .thenReturn(Optional.of(MockInterceptorParameter.getProtectedEnvironment()));
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("aaaaaaaaaaaaaaaaa"))
            .thenReturn(Optional.of(MockInterceptorParameter.getProtectedEnvironment()));
        gaussDBDWSDatabaseBackupInterceptor.supplyBackupTask(backupTask);
    }

    /**
     * 用例场景：GaussDB(DWS)集群 database 更新连通性Agent无异常
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void supply_agent_success() throws IllegalAccessException {
        BackupTask backupTask = MockInterceptorParameter.getBackupTask();
        Optional<ProtectedResource> resourceOptional = Optional.of(MockInterceptorParameter.getProtectedEnvironment());
        PowerMockito.when(resourceService.getResourceById("cccccccc")).thenReturn(resourceOptional);
        PowerMockito.when(providerManager.findProviderOrDefault(ProtectedResourceChecker.class, resourceOptional.get(),
            this.protectedResourceChecker)).thenReturn(protectedResourceChecker);
        PowerMockito.when(protectedResourceChecker.collectConnectableResources(resourceOptional.get()))
            .thenReturn(MockInterceptorParameter.getProtectedResourceListMap());
        MemberModifier.field(GaussDBDWSDatabaseBackupInterceptor.class, "providerManager")
            .set(gaussDBDWSDatabaseBackupInterceptor, providerManager);
        gaussDBDWSDatabaseBackupInterceptor.supplyAgent(backupTask);
    }

    /**
     * 用例场景：GaussDB(DWS)集群 任务无nodes里面无主机的Agent
     * 前置条件：set_node_type_fail
     * 检查点：失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_no_find_cluster_and_host_agent_when_supply_backup_task() {
        BackupTask backupTask = MockInterceptorParameter.getBackupTask();
        backupTask.getProtectEnv().getNodes().add(new TaskEnvironment());
        Assert.assertThrows(LegoCheckedException.class,
            () -> gaussDBDWSDatabaseBackupInterceptor.supplyBackupTask(backupTask));
    }
}
