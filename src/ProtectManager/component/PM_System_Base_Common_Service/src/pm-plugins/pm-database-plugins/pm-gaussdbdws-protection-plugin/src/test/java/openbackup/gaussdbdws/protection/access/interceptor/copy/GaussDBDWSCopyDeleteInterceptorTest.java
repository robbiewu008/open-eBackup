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
package openbackup.gaussdbdws.protection.access.interceptor.copy;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.ClustersInfoVo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * GaussDBDWS副本删除测试类
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-01
 */
public class GaussDBDWSCopyDeleteInterceptorTest {
    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);
    private final ClusterNativeApi clusterNativeApi = PowerMockito.mock(ClusterNativeApi.class);
    private final EncryptorService encryptorService = PowerMockito.mock(EncryptorService.class);
    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);
    private final GaussDBBaseService gaussDBBaseService = PowerMockito.mock(GaussDBBaseService.class);
    private final ProtectedResourceChecker checker = PowerMockito.mock(ProtectedResourceChecker.class);
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);
    private final GaussDBDWSCopyDeleteInterceptor copyDeleteInterceptor = new GaussDBDWSCopyDeleteInterceptor(
        copyRestApi, clusterNativeApi, encryptorService, providerManager,  resourceService);

    @Before
    public void init() {
        copyDeleteInterceptor.setProtectedResourceChecker(checker);
        copyDeleteInterceptor.setGaussDBBaseService(gaussDBBaseService);
        Mockito.when(clusterNativeApi.queryTargetClusterListDetails(Mockito.any()))
            .thenReturn(Lists.newArrayList(initClusterDetailInfo()));
        ClustersInfoVo clustersInfoVo = new ClustersInfoVo();
        clustersInfoVo.setStorageEsn("esn");
        PowerMockito.when(clusterNativeApi.getCurrentClusterVoInfo()).thenReturn(clustersInfoVo);
    }

    /**
     * 用例场景：DWS副本删除策略适配
     * 前置条件：类型判断
     * 检查点：是否匹配
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(copyDeleteInterceptor.applicable(ResourceSubTypeEnum.GAUSSDB_DWS.getType()));
        Assert.assertTrue(copyDeleteInterceptor.applicable(ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType()));
        Assert.assertTrue(copyDeleteInterceptor.applicable(ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()));
        Assert.assertTrue(copyDeleteInterceptor.applicable(ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType()));
    }

    /**
     * 用例场景：GaussDBDWS副本级联删除(删除增量副本时返回依赖副本）
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：是否添加成功需要级联删除的副本
     */
    @Test
    public void get_copies_copy_type_is_difference_increment_success() {
        List<Copy> copies = generateBackupCopies();
        Copy thisCopy = copies.stream().filter(copy -> copy.getGn() == 6).findFirst().get();
        Copy nextFullCopy = copies.stream().filter(copy -> copy.getGn() == 10).findFirst().get();
        List<String> processedCopies = copyDeleteInterceptor.getCopiesCopyTypeIsDifferenceIncrement(copies, thisCopy, nextFullCopy);
        Assert.assertEquals(1, processedCopies.size());
    }

    /**
     * 用例场景：GaussDBDWS副本删除下发agent
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：去除非集群节点成功
     */
    @Test
    public void supply_agent_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setRootUuid("root_uuid");
        PowerMockito.when(gaussDBBaseService.getResourceById(anyString())).thenReturn(protectedResource);
        PowerMockito.when(providerManager.findProviderOrDefault(any(), any(), any())).thenReturn(checker);
        Map<ProtectedResource, List<ProtectedEnvironment>> map = new HashMap<>();
        ProtectedEnvironment environment1 = new ProtectedEnvironment();
        environment1.setUuid("env1");
        environment1.setEndpoint("127.0.0.1");
        environment1.setPort(0);
        ProtectedEnvironment environment2 = new ProtectedEnvironment();
        environment2.setUuid("env2");
        environment2.setEndpoint("127.0.0.2");
        environment2.setPort(0);
        map.put(new ProtectedResource(), Arrays.asList(environment1, environment2));
        PowerMockito.when(checker.collectConnectableResources(any())).thenReturn(map);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DwsConstant.DWS_CLUSTER_AGENT, "127.0.0.1");
        environment.setExtendInfo(extendInfo);
        PowerMockito.when(gaussDBBaseService.getEnvironmentById(anyString())).thenReturn(environment);
        DeleteCopyTask task = new DeleteCopyTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("test");
        task.setProtectObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("test");
        task.setProtectEnv(taskEnvironment);
        copyDeleteInterceptor.supplyAgent(task, new CopyInfoBo());
        Assert.assertEquals(1, task.getAgents().size());
    }

    /**
     * 用例场景：DWS 副本删除参数下发组装
     * 前置条件：1.副本正常，2.资源存在
     * 检查点：组装参数成功
     */
    @Test
    public void hand_copy_delete_task_success() {
        DeleteCopyTask task = new DeleteCopyTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setExtendInfo(new HashMap<>());
        task.setProtectEnv(taskEnvironment);
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("test");
        task.setProtectObject(taskResource);
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setId("22222222");
        storageRepository.setExtendAuth(new Authentication());
        task.setRepositories(Lists.newArrayList(storageRepository));
        ClusterDetailInfo localCluster = new ClusterDetailInfo();
        SourceClustersParams sourceClustersParams = new SourceClustersParams();
        sourceClustersParams.setMgrIpList(Lists.newArrayList("127.0.0.1"));
        localCluster.setSourceClusters(sourceClustersParams);
        StorageSystemInfo storageSystemInfo = new StorageSystemInfo();
        storageSystemInfo.setStorageEsn("22222222");
        storageRepository.setProtocol(100);
        localCluster.setStorageSystem(storageSystemInfo);
        Mockito.when(clusterNativeApi.queryCurrentGroupClusterDetails()).thenReturn(localCluster);
        Mockito.when(encryptorService.decrypt(anyString())).thenReturn("test");
        ProtectedResource resource = new ProtectedResource();
        resource.setRootUuid("root_uuid");
        PowerMockito.when(resourceService.getBasicResourceById(anyString())).thenReturn(Optional.of(resource));
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Map<String, String> map = new HashMap<>();
        map.put(DwsConstant.EXTEND_INFO_KEY_ENV_FILE, "env/file");
        environment.setExtendInfo(map);
        Authentication authentication = new Authentication();
        authentication.setAuthKey("omm");
        environment.setAuth(authentication);
        PowerMockito.when(gaussDBBaseService.getResourceById(anyString())).thenReturn(resource);
        PowerMockito.when(gaussDBBaseService.getEnvironmentById(anyString())).thenReturn(environment);
        copyDeleteInterceptor.handleTask(task, new CopyInfoBo());
        Assert.assertEquals("127.0.0.1", task.getRepositories().get(0).getEndpoint().getIp());
        Assert.assertEquals("env/file", task.getProtectEnv().getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_ENV_FILE));
        Assert.assertEquals("omm", task.getProtectEnv().getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_DWS_USER));
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(), task.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    private List<Copy> generateBackupCopies() {
        // 副本顺序：全差增增全增差增差全
        Copy fullCopy1 = generateResourceCopy();
        fullCopy1.setGn(1);
        fullCopy1.setUuid("full_01");
        fullCopy1.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        fullCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy cumulativeCopy1 = generateResourceCopy();
        cumulativeCopy1.setGn(2);
        cumulativeCopy1.setUuid("cumulative_01");
        cumulativeCopy1.setBackupType(BackupTypeConstants.CUMULATIVE_INCREMENT.getAbBackupType());
        cumulativeCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy differenceCopy1 = generateResourceCopy();
        differenceCopy1.setGn(3);
        differenceCopy1.setUuid("difference_01");
        differenceCopy1.setBackupType(BackupTypeConstants.DIFFERENCE_INCREMENT.getAbBackupType());
        differenceCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy differenceCopy2 = BeanTools.copy(differenceCopy1, Copy::new);
        differenceCopy2.setGn(4);
        differenceCopy2.setUuid("difference_02");
        Copy fullCopy2 = BeanTools.copy(fullCopy1, Copy::new);
        fullCopy2.setGn(5);
        fullCopy2.setUuid("full_01");
        Copy differenceCopy3 = BeanTools.copy(differenceCopy1, Copy::new);
        differenceCopy3.setGn(6);
        differenceCopy3.setUuid("difference_03");
        Copy cumulativeCopy2 = BeanTools.copy(cumulativeCopy1, Copy::new);
        cumulativeCopy2.setGn(7);
        cumulativeCopy2.setUuid("cumulative_02");
        Copy differenceCopy4 = BeanTools.copy(differenceCopy1, Copy::new);
        differenceCopy4.setGn(8);
        differenceCopy4.setUuid("difference_04");
        Copy cumulativeCopy3 = BeanTools.copy(cumulativeCopy1, Copy::new);
        cumulativeCopy3.setGn(9);
        cumulativeCopy3.setUuid("cumulative_03");
        Copy fullCopy3 = BeanTools.copy(fullCopy1, Copy::new);
        fullCopy3.setGn(10);
        fullCopy3.setUuid("full_03");
        return Arrays.asList(fullCopy1, fullCopy2, fullCopy3, differenceCopy1, differenceCopy2, differenceCopy3,
            cumulativeCopy1, cumulativeCopy2, cumulativeCopy3, differenceCopy4);
    }

    private Copy generateResourceCopy() {
        Copy copy = new Copy();
        copy.setResourceId("this_resource_id");
        return copy;
    }

    private ClusterDetailInfo initClusterDetailInfo() {
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        StorageSystemInfo storageSystem = new StorageSystemInfo();
        storageSystem.setPassword("123456");
        clusterDetailInfo.setStorageSystem(storageSystem);
        return clusterDetailInfo;
    }
}
