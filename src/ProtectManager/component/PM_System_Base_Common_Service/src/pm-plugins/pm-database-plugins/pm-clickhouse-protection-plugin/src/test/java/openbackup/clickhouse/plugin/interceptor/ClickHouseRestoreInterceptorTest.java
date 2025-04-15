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
package openbackup.clickhouse.plugin.interceptor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.eq;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.clickhouse.plugin.provider.ClickHouseAgentProvider;
import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.alibaba.fastjson.JSON;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.mock.mockito.MockBean;

import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * ClickHouseRestoreInterceptor Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {EnvironmentLinkStatusHelper.class,})
public class ClickHouseRestoreInterceptorTest {
    private ClickHouseRestoreInterceptor clickHouseRestoreInterceptor;

    @Mock
    private ResourceService resourceService;

    @Mock
    private DmeUnifiedRestApi dmeUnifiedRestApi;

    @Mock
    private CopyRestApi copyRestApi;

    @Before
    public void init() {
        KerberosService kerberosService = Mockito.mock(KerberosService.class);
        EncryptorService encryptorService = Mockito.mock(EncryptorService.class);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);

        clickHouseRestoreInterceptor = new ClickHouseRestoreInterceptor(resourceService,
            dmeUnifiedRestApi, copyRestApi,
            kerberosService, encryptorService, new ClickHouseAgentProvider(resourceService));
    }

    @MockBean
    private MemberClusterService memberClusterService;

    /**
     * 用例场景：测试clickhouse恢复拦截器参数设置正确
     * 前置条件：无
     * 检查点：task里参数设置正确
     */
    @Test
    public void intercept_task_success() {
        // init data
        PowerMockito.when(EnvironmentLinkStatusHelper.isOnlineAdaptMultiCluster(any(TaskEnvironment.class)))
            .thenReturn(true);
        RestoreTask task = new RestoreTask();
        TaskEnvironment targetEnv = new TaskEnvironment();
        targetEnv.setLinkStatus(String.valueOf(LinkStatusEnum.ONLINE.getStatus()));
        targetEnv.setUuid("targetEnvUuid");
        targetEnv.setExtendInfo(Maps.newHashMap());
        task.setTargetEnv(targetEnv);
        task.setCopyId("copyId");

        DmeCopyInfo copyInfo = new DmeCopyInfo();
        TaskEnvironment protectEnv = new TaskEnvironment();
        TaskEnvironment node1 = new TaskEnvironment();
        Map<String, String> extendInfo1 = Maps.newHashMap();
        extendInfo1.put(DatabaseConstants.CLUSTER_TARGET, "cluster");
        extendInfo1.put(ClickHouseConstant.SHARD_NUM, "1");
        extendInfo1.put(ClickHouseConstant.SHARD_WEIGHT, "1");
        extendInfo1.put(ClickHouseConstant.REPLICA_NUM, "1");
        extendInfo1.put(ClickHouseConstant.AGENT_ID, "agentUuid1");
        node1.setExtendInfo(extendInfo1);
        TaskEnvironment node2 = new TaskEnvironment();
        Map<String, String> extendInfo2 = Maps.newHashMap();
        extendInfo2.put(DatabaseConstants.CLUSTER_TARGET, "cluster");
        extendInfo2.put(ClickHouseConstant.SHARD_NUM, "1");
        extendInfo2.put(ClickHouseConstant.SHARD_WEIGHT, "1");
        extendInfo2.put(ClickHouseConstant.REPLICA_NUM, "2");
        extendInfo2.put(ClickHouseConstant.AGENT_ID, "agentUuid2");
        node2.setExtendInfo(extendInfo2);
        List<TaskEnvironment> nodes = Lists.newArrayList(node1, node2);
        protectEnv.setNodes(nodes);
        protectEnv.setUuid("originalUuid");
        copyInfo.setProtectEnv(protectEnv);

        ProtectedResource targetResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = Maps.newHashMap();

        ProtectedResource targetNode1 = new ProtectedResource();
        Map<String, String> targetNodeExtendInfo1 = Maps.newHashMap();
        targetNodeExtendInfo1.put(DatabaseConstants.CLUSTER_TARGET, "cluster");
        targetNodeExtendInfo1.put(ClickHouseConstant.SHARD_NUM, "1");
        targetNodeExtendInfo1.put(ClickHouseConstant.SHARD_WEIGHT, "1");
        targetNodeExtendInfo1.put(ClickHouseConstant.REPLICA_NUM, "1");
        targetNodeExtendInfo1.put(DatabaseConstants.STATUS, String.valueOf(ClusterEnum.StatusEnum.ONLINE.getStatus()));
        targetNode1.setExtendInfo(targetNodeExtendInfo1);
        targetNode1.setUuid("targetNodeUuid1");
        Authentication auth = new Authentication();
        auth.setExtendInfo(Maps.newHashMap());
        targetNode1.setAuth(auth);
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setLinkStatus("1");
        agent1.setEndpoint("192.168.1.1");
        agent1.setPort(22);
        agent1.setUuid("agentUuid1");
        Map<String, List<ProtectedResource>> nodeDep1 = Maps.newHashMap();
        nodeDep1.put(DatabaseConstants.AGENTS, Lists.newArrayList(agent1));
        targetNode1.setDependencies(nodeDep1);

        ProtectedResource targetNode2 = new ProtectedResource();
        Map<String, String> targetNodeExtendInfo2 = Maps.newHashMap();
        targetNodeExtendInfo2.put(DatabaseConstants.CLUSTER_TARGET, "cluster");
        targetNodeExtendInfo2.put(ClickHouseConstant.SHARD_NUM, "1");
        targetNodeExtendInfo2.put(ClickHouseConstant.SHARD_WEIGHT, "1");
        targetNodeExtendInfo2.put(ClickHouseConstant.REPLICA_NUM, "2");
        targetNodeExtendInfo2.put(DatabaseConstants.STATUS, String.valueOf(ClusterEnum.StatusEnum.ONLINE.getStatus()));
        targetNode2.setExtendInfo(targetNodeExtendInfo2);
        targetNode2.setUuid("targetNodeUuid2");
        ProtectedEnvironment agent2 = new ProtectedEnvironment();
        agent2.setLinkStatus("1");
        agent2.setEndpoint("192.168.1.2");
        agent2.setPort(22);
        agent2.setUuid("agentUuid2");
        Map<String, List<ProtectedResource>> nodeDep2 = Maps.newHashMap();
        nodeDep2.put(DatabaseConstants.AGENTS, Lists.newArrayList(agent2));
        targetNode2.setAuth(auth);
        targetNode2.setDependencies(nodeDep2);

        List<ProtectedResource> targetNodes = Lists.newArrayList(targetNode1, targetNode2);
        dependencies.put(DatabaseConstants.CHILDREN, targetNodes);
        targetResource.setDependencies(dependencies);

        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());

        // mock
        PowerMockito.when(resourceService.getResourceById(anyBoolean(), eq(("targetEnvUuid"))))
            .thenReturn(Optional.of(targetResource));
        PowerMockito.when(dmeUnifiedRestApi.getCopyInfo(any())).thenReturn(copyInfo);
        PowerMockito.when(resourceService.getResourceById(eq("originalUuid"))).thenReturn(Optional.of(targetResource));
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);

        // do invoke
        clickHouseRestoreInterceptor.initialize(task);

        // assert
        assertEquals(
            "[{\"advanceParams\":{},\"id\":\"agentUuid2\",\"ip\":\"192.168.1.2\",\"port\":22},{\"advanceParams\":{},\"id\":\"agentUuid1\",\"ip\":\"192.168.1.1\",\"port\":22}]",
            JSON.toJSONString(task.getAgents()));
        assertEquals("DownloadRestore", task.getRestoreMode());
        assertEquals(
            "{\"extendInfo\":{\"deployType\":\"4\"},\"linkStatus\":\"1\",\"nodes\":[{\"auth\":{\"authType\":0,\"extendInfo\":{}},\"extendInfo\":{\"shard_weight\":\"1\",\"cluster\":\"cluster\",\"replica_num\":\"1\",\"shard_num\":\"1\",\"status\":\"27\",\"agentId\":\"agentUuid1\",\"originalAgentId\":\"agentUuid1\"},\"uuid\":\"targetNodeUuid1\"},{\"auth\":{\"authType\":0,\"extendInfo\":{}},\"extendInfo\":{\"shard_weight\":\"1\",\"cluster\":\"cluster\",\"replica_num\":\"2\",\"shard_num\":\"1\",\"status\":\"27\",\"agentId\":\"agentUuid2\",\"originalAgentId\":\"agentUuid2\"},\"uuid\":\"targetNodeUuid2\"}],\"uuid\":\"targetEnvUuid\"}",
            JSON.toJSONString(task.getTargetEnv()));
    }

    /**
     * 用例场景：测试匹配clickhouse备份拦截器
     * 前置条件：无
     * 检查点：方法返回true
     */
    @Test
    public void test_applicable_click_house() {
        assertTrue(clickHouseRestoreInterceptor.applicable(ResourceSubTypeEnum.CLICK_HOUSE.getType()));
    }

    /**
     * 用例场景：下发恢复任务时 针对资源进行锁定
     * 前置条件：构造restoreTask结构体
     * 检查点: 成功返回加锁列表
     */
    @Test
    public void restore_getLock_resources() {
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setTargetObject(new TaskResource());
        restoreTask.getTargetObject().setUuid("test_uuid");
        List<LockResourceBo> lockResources = clickHouseRestoreInterceptor.getLockResources(restoreTask);
        Assert.assertEquals(lockResources.get(0).getLockType(), LockType.WRITE);
    }
}