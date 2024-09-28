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
package openbackup.gaussdbdws.protection.access.interceptor.restore;

import static org.mockito.ArgumentMatchers.anyString;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * GaussDbDws 恢复任务基础数据Provider测试类
 *
 */
public class GaussDBDWSBaseRestoreInterceptorTest {
    private final ProtectedEnvironmentService environmentService = Mockito.mock(ProtectedEnvironmentService.class);

    private final GaussDBBaseService gaussDBBaseService = Mockito.mock(GaussDBBaseService.class);

    private final ProtectedEnvironmentRetrievalsService protectedResourceChecker = Mockito.mock(
        ProtectedEnvironmentRetrievalsService.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final ClusterNativeApi clusterNativeApi = PowerMockito.mock(ClusterNativeApi.class);

    private final EncryptorService encryptorService = PowerMockito.mock(EncryptorService.class);

    private ResourceService resourceService = Mockito.mock(ResourceService.class);

    private JobCenterRestApi jobCenterRestApi = Mockito.mock(JobCenterRestApi.class);

    private final GaussDBDWSBaseRestoreInterceptor restoreInterceptor = new GaussDBDWSBaseRestoreInterceptor(
        environmentService, gaussDBBaseService, protectedResourceChecker, copyRestApi, clusterNativeApi,
        resourceService, jobCenterRestApi);

    @Before
    public void initTest() {
        restoreInterceptor.setEncryptorService(encryptorService);
    }
    /**
     * 用例场景：DWS恢复参数处理
     * 前置条件：1.没有agent信息
     * 检  查  点：返回备份类型是否为快照备份
     */
    @Test
    public void supply_restore_task_not_agents_success() {
        Mockito.when(environmentService.getEnvironmentById("env")).thenReturn(initProtectedEnvironment());
        Mockito.when(gaussDBBaseService.supplyNodes(Mockito.anyString())).thenReturn(new ArrayList<>());
        ProtectedResource protectedResource = initProtectedResource();
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("uuid");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8088);
        List<ProtectedEnvironment> protectedEnvironments = Lists.newArrayList(protectedEnvironment);
        protectedResourceMap.put(protectedResource, protectedEnvironments);
        Mockito.when(protectedResourceChecker.collectConnectableResources(Mockito.anyString()))
            .thenReturn(protectedResourceMap);
        mockLockCluster();
        RestoreTask restoreTask = restoreInterceptor.supplyRestoreTask(initRestoreTask());
        Assert.assertEquals(restoreTask.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.SINGLE.getType());
    }

    /**
     * 用例场景：回填副本恢复类型到恢复任务中成功
     * 前置条件：副本正常，下发任务正常
     * 检查点：回填成功-LocalRestore
     *
     * @throws Exception MethodNotFoundException
     */
    @Test
    public void supply_task_restore_mode_with_LocalRestore_type_success() throws Exception {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setCopyId("test_copy");
        Whitebox.invokeMethod(restoreInterceptor, "supplyRestoreMode", restoreTask);
        Assert.assertEquals(RestoreModeEnum.LOCAL_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：回填副本恢复类型到恢复任务中成功
     * 前置条件：副本正常，下发任务正常
     * 检查点：回填成功-DownloadRestore
     *
     * @throws Exception MethodNotFoundException
     */
    @Test
    public void supply_task_restore_mode_with_DownloadRestore_type_success() throws Exception {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setCopyId("test_copy");
        Whitebox.invokeMethod(restoreInterceptor, "supplyRestoreMode", restoreTask);
        Assert.assertEquals(RestoreModeEnum.DOWNLOAD_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：回填副本恢复类型到恢复任务中成功
     * 前置条件：副本正常，下发任务正常
     * 检查点：回填成功-RemoteRestore
     *
     * @throws Exception MethodNotFoundException
     */
    @Test
    public void supply_task_restore_mode_with_RemoteRestore_type_success() throws Exception {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setCopyId("test_copy");
        Whitebox.invokeMethod(restoreInterceptor, "supplyRestoreMode", restoreTask);
        Assert.assertEquals(RestoreModeEnum.REMOTE_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：DWS恢复参数处理
     * 前置条件：1.细粒度恢复 2.副本正常
     * 检查点：组装subObjects成功
     */
    @Test
    public void supply_restore_task_subObjects_success() throws Exception {
        RestoreTask restoreTask = initRestoreTask();
        restoreTask.setRestoreType(RestoreTypeEnum.FLR.getType());
        TaskResource taskResource = new TaskResource();
        taskResource.setName("{\"name\":\"database/schema/table\",\"type\":\"Database\",\"subType\":\"DWS-table\"}");
        restoreTask.setSubObjects(Collections.singletonList(taskResource));
        Whitebox.invokeMethod(restoreInterceptor, "supplySubObjects", restoreTask);
        Assert.assertEquals("database/schema/table", restoreTask.getSubObjects().get(0).getName());
        Assert.assertEquals(ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType(),
            restoreTask.getSubObjects().get(0).getSubType());
        Assert.assertEquals(ResourceTypeEnum.DATABASE.getType(), restoreTask.getSubObjects().get(0).getType());
    }

    /**
     * 用例场景：DWS GDS新位置恢复参数处理
     * 前置条件：1.GDS新位置恢复 2.副本正常
     * 检查点：组装targetObject成功
     */
    @Test
    public void supply_restore_task_targetObject_success() throws Exception {
        RestoreTask restoreTask = initRestoreTask();
        restoreTask.setRestoreType(RestoreTypeEnum.CR.getType());
        restoreTask.setTargetLocation(RestoreLocationEnum.NEW);
        restoreTask.setTargetObject(new TaskResource());
        Map<String, String> map = new HashMap<>();
        map.put(DwsConstant.EXTEND_INFO_KEY_DATABASE, "postgre");
        restoreTask.setAdvanceParams(map);
        Whitebox.invokeMethod(restoreInterceptor, "supplyTargetObject", restoreTask);
        Assert.assertEquals("postgre",
            restoreTask.getTargetObject().getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_DATABASE));
    }

    /**
     * 用例场景：DWS 新位置恢复参数下发
     * 前置条件：1.新位置恢复 2.副本正常
     * 检查点：下发新位置高级参数成功
     */
    @Test
    public void supply_restore_task_advance_params_success() throws Exception {
        RestoreTask restoreTask = initRestoreTask();
        restoreTask.setTargetLocation(RestoreLocationEnum.NEW);
        restoreTask.setCopyId("2222");
        restoreTask.setAdvanceParams(new HashMap<>());
        Mockito.when(copyRestApi.queryCopyByID(restoreTask.getCopyId())).thenReturn(new Copy());
        Whitebox.invokeMethod(restoreInterceptor, "supplyAdvancedParams", restoreTask);
        Assert.assertEquals(RestoreLocationEnum.NEW.getLocation(),
            restoreTask.getAdvanceParams().get(DwsConstant.EXTEND_INFO_KEY_TARGET_LOCATION));
    }

    /**
     * 用例场景：DWS 恢复参数下发
     * 前置条件：1.副本正常
     * 检查点：下发认证参数成功
     */
    @Test
    public void supply_restore_task_auth_success() throws Exception {
        RestoreTask restoreTask = initRestoreTask();
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setId("22222222");
        restoreTask.setRepositories(Lists.newArrayList(storageRepository));
        storageRepository.setProtocol(100);
        mockLockCluster();
        Whitebox.invokeMethod(restoreInterceptor, "supplyAuth", restoreTask);
        Assert.assertEquals("127.0.0.1", restoreTask.getRepositories().get(0).getEndpoint().getIp());
    }

    private void mockLockCluster() {
        ClusterDetailInfo localCluster = new ClusterDetailInfo();
        SourceClustersParams sourceClustersParams = new SourceClustersParams();
        sourceClustersParams.setMgrIpList(Lists.newArrayList("127.0.0.1"));
        localCluster.setSourceClusters(sourceClustersParams);
        StorageSystemInfo storageSystemInfo = new StorageSystemInfo();
        storageSystemInfo.setStorageEsn("22222222");
        localCluster.setStorageSystem(storageSystemInfo);
        Mockito.when(clusterNativeApi.queryCurrentGroupClusterDetails()).thenReturn(localCluster);
    }

    /**
     * 用例场景：DWS恢复参数处理
     * 前置条件：1.类型判断
     * 检  查  点：是否匹配
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(restoreInterceptor.applicable(ResourceSubTypeEnum.GAUSSDB_DWS.getType()));
        Assert.assertTrue(restoreInterceptor.applicable(ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType()));
        Assert.assertTrue(restoreInterceptor.applicable(ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()));
        Assert.assertTrue(restoreInterceptor.applicable(ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType()));
    }

    private RestoreTask initRestoreTask() {
        RestoreTask restoreTask = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("env");
        restoreTask.setCopyId("copy_id");
        restoreTask.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        restoreTask.setTargetObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("env");
        taskEnvironment.setExtendInfo(new HashMap<>());
        restoreTask.setTargetEnv(taskEnvironment);
        restoreTask.setAdvanceParams(new HashMap<>());
        restoreTask.getAdvanceParams()
            .put(DwsConstant.EXTEND_INFO_KEY_SUB_TYPE, ResourceSubTypeEnum.GAUSSDB_DWS.getType());
        restoreTask.setRepositories(new ArrayList<>());
        return restoreTask;
    }

    private ProtectedEnvironment initProtectedEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Authentication authentication = new Authentication();
        authentication.setAuthKey("auth");
        environment.setSubType(ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType());
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        environment.setRootUuid("env");
        environment.setAuth(authentication);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> protectedResources = new ArrayList<>();
        protectedResources.add(initProtectedResource());
        dependencies.put(DwsConstant.DWS_CLUSTER_AGENT, protectedResources);
        environment.setDependencies(dependencies);
        environment.setExtendInfo(new HashMap<>());
        return environment;
    }

    private ProtectedResource initProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("1");
        return protectedResource;
    }

    private HostDto initHostDto() {
        HostDto hostDto = new HostDto();
        hostDto.setEndpoint("127.0.0.1");
        hostDto.setUuid("1");
        return hostDto;
    }
}
