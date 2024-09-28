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
import static org.mockito.ArgumentMatchers.eq;

import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.clickhouse.plugin.provider.ClickHouseAgentProvider;
import openbackup.clickhouse.plugin.service.ClickHouseService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * ClickHouseBackupInterceptor Test
 *
 */
@RunWith(PowerMockRunner.class)
public class ClickHouseBackupInterceptorTest {
    private ClickHouseBackupInterceptor clickHouseBackupInterceptor;

    @Mock
    private ResourceService resourceService;

    @Mock
    private ProtectedEnvironmentService protectedEnvironmentService;

    @Mock
    private ClickHouseService clickHouseService;

    /**
     * 预期异常
     */
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() {
        KerberosService kerberosService = Mockito.mock(KerberosService.class);
        EncryptorService encryptorService = Mockito.mock(EncryptorService.class);
        clickHouseBackupInterceptor = new ClickHouseBackupInterceptor(clickHouseService, resourceService,
            protectedEnvironmentService, kerberosService, encryptorService,
            new ClickHouseAgentProvider(resourceService));
    }

    /**
     * 用例场景：测试匹配clickhouse备份拦截器
     * 前置条件：无
     * 检查点：方法返回true
     */
    @Test
    public void test_applicable_click_house() {
        assertTrue(clickHouseBackupInterceptor.applicable(ResourceSubTypeEnum.CLICK_HOUSE.getType()));
    }

    /**
     * 用例场景：测试补充clickhouse应用信息正常
     * 前置条件：无
     * 检查点：clickhouse应用信息正常添加到backupTask对象中
     */
    @Test
    public void test_supplyBackupTask() {
        // init data
        BackupTask backupTask = new BackupTask();
        TaskEnvironment protectEnv = new TaskEnvironment();
        protectEnv.setExtendInfo(Maps.newHashMap());
        backupTask.setProtectEnv(protectEnv);
        backupTask.setRepositories(Lists.newArrayList(new StorageRepository()));

        // do invoke
        clickHouseBackupInterceptor.supplyBackupTask(backupTask);

        // assert
        assertEquals(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat(), backupTask.getCopyFormat());
        assertEquals(DatabaseDeployTypeEnum.SHARDING.getType(),
            backupTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
        assertEquals(Boolean.TRUE.toString(),
            backupTask.getAdvanceParams().get(ClickHouseConstant.ADVANCE_PARAMS_KEY_MULTI_FILE_SYSTEM));
        assertEquals(Boolean.TRUE.toString(),
            backupTask.getAdvanceParams().get(ClickHouseConstant.ADVANCE_PARAMS_KEY_MULTI_POST_JOB));
    }

    /**
     * 用例场景：测试补充agent信息成功
     * 前置条件：无
     * 检查点：agent信息正常添加到backupTask对象中
     */
    @Test
    public void test_supplyAgent() {
        // init data
        BackupTask backupTask = new BackupTask();
        TaskResource protectDatabase = new TaskResource();
        protectDatabase.setRootUuid("clusterUuid");
        backupTask.setProtectObject(protectDatabase);

        ProtectedEnvironment agentEnv1 = new ProtectedEnvironment();
        agentEnv1.setUuid("agentUuid1");
        agentEnv1.setEndpoint("192.168.0.1");
        agentEnv1.setPort(22);
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv1)));

        ProtectedEnvironment agentEnv2 = new ProtectedEnvironment();
        agentEnv2.setUuid("agentUuid2");
        agentEnv2.setEndpoint("192.168.0.2");
        agentEnv2.setPort(22);
        ProtectedResource nodeResource2 = new ProtectedResource();
        nodeResource2.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv2)));

        ProtectedEnvironment clusterEnv = new ProtectedEnvironment();
        clusterEnv.setDependencies(
            ImmutableMap.of(ResourceConstants.CHILDREN, Lists.newArrayList(nodeResource1, nodeResource2)));

        // mock
        PowerMockito.when(resourceService.getResourceById("clusterUuid")).thenReturn(Optional.of(clusterEnv));

        // do invoke
        clickHouseBackupInterceptor.supplyAgent(backupTask);

        // assert
        assertEquals(2, backupTask.getAgents().size());
    }

    /**
     * 用例场景：测试备份数据库，四个节点
     * 前置条件：无
     * 检查点：backupTask设置nodes信息正确
     */
    @Test
    public void test_database_supplyNodes() {
        // init data
        BackupTask backupTask = new BackupTask();
        backupTask.setProtectEnv(new TaskEnvironment());
        TaskResource protectDatabase = new TaskResource();
        protectDatabase.setRootUuid("clusterUuid");
        protectDatabase.setUuid("databaseUuid");
        protectDatabase.setName("databaseName");
        protectDatabase.setType(ClickHouseConstant.DATABASE_TYPE);
        backupTask.setProtectObject(protectDatabase);

        ProtectedEnvironment agentEnv1 = new ProtectedEnvironment();
        agentEnv1.setUuid("agentUuid1");
        agentEnv1.setEndpoint("192.168.0.1");
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setName("nodeResource1");
        Authentication auth = new Authentication();
        auth.setAuthType(0);
        Map<String, String> node1ExtendInfo = Maps.newHashMap();
        node1ExtendInfo.put(ClickHouseConstant.SHARD_NUM, "1");
        auth.setExtendInfo(Maps.newHashMap());
        nodeResource1.setAuth(auth);
        nodeResource1.setExtendInfo(node1ExtendInfo);
        nodeResource1.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv1)));

        ProtectedEnvironment agentEnv2 = new ProtectedEnvironment();
        agentEnv2.setUuid("agentUuid2");
        agentEnv2.setEndpoint("192.168.0.2");
        ProtectedResource nodeResource2 = new ProtectedResource();
        nodeResource2.setName("nodeResource2");
        nodeResource2.setAuth(auth);
        Map<String, String> node2ExtendInfo = Maps.newHashMap();
        node2ExtendInfo.put(ClickHouseConstant.SHARD_NUM, "1");
        nodeResource2.setExtendInfo(node2ExtendInfo);
        nodeResource2.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv2)));

        ProtectedResource nodeResource3 = new ProtectedResource();
        nodeResource3.setName("nodeResource3");
        nodeResource3.setAuth(auth);
        Map<String, String> node3ExtendInfo = Maps.newHashMap();
        node3ExtendInfo.put(ClickHouseConstant.SHARD_NUM, "2");
        nodeResource3.setExtendInfo(node3ExtendInfo);
        ProtectedEnvironment agentEnv3 = new ProtectedEnvironment();
        agentEnv3.setUuid("agentUuid3");
        agentEnv3.setEndpoint("192.168.0.3");
        nodeResource3.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv3)));

        ProtectedResource nodeResource4 = new ProtectedResource();
        nodeResource4.setName("nodeResource4");
        nodeResource4.setAuth(auth);
        Map<String, String> node4ExtendInfo = Maps.newHashMap();
        node4ExtendInfo.put(ClickHouseConstant.SHARD_NUM, "2");
        nodeResource4.setExtendInfo(node4ExtendInfo);
        ProtectedEnvironment agentEnv4 = new ProtectedEnvironment();
        agentEnv4.setUuid("agentUuid4");
        agentEnv4.setEndpoint("192.168.0.4");
        nodeResource4.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv4)));

        ProtectedEnvironment clusterEnv = new ProtectedEnvironment();
        clusterEnv.setUuid("clusterUuid");
        clusterEnv.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN,
            Lists.newArrayList(nodeResource1, nodeResource2, nodeResource3, nodeResource4)));

        ProtectedResource tableDetail1 = new ProtectedResource();
        tableDetail1.setName("tableName1");
        Map<String, String> extendInfo1 = Maps.newHashMap();
        extendInfo1.put(ClickHouseConstant.TABLE_ENGINE, "ReplicatedMergeTree");
        tableDetail1.setExtendInfo(extendInfo1);
        ProtectedResource tableDetail2 = new ProtectedResource();
        Map<String, String> extendInfo2 = Maps.newHashMap();
        tableDetail2.setName("tableName2");
        extendInfo2.put(ClickHouseConstant.TABLE_ENGINE, "ReplicatedMergeTree");
        tableDetail2.setExtendInfo(extendInfo2);
        List<ProtectedResource> queryTablesDetails = Lists.newArrayList(tableDetail1, tableDetail2);

        ProtectedResource databaseDetail = new ProtectedResource();
        Map<String, String> databaseExtendInfo = Maps.newHashMap();
        databaseExtendInfo.put(ClickHouseConstant.DB_NAME, "databaseName");
        databaseDetail.setExtendInfo(databaseExtendInfo);
        List<ProtectedResource> queryDatabaseDetails = Lists.newArrayList(databaseDetail);

        // mock
        PowerMockito.when(resourceService.getResourceById("clusterUuid")).thenReturn(Optional.of(clusterEnv));
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("agentUuid1")).thenReturn(agentEnv1);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("agentUuid2")).thenReturn(agentEnv2);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("agentUuid3")).thenReturn(agentEnv3);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("agentUuid4")).thenReturn(agentEnv4);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("clusterUuid")).thenReturn(clusterEnv);
        PageListResponse<ProtectedResource> databasePageListResponse = new PageListResponse<>();
        databasePageListResponse.setRecords(queryDatabaseDetails);
        PageListResponse<ProtectedResource> tablePageListResponse = new PageListResponse<>();
        tablePageListResponse.setRecords(queryTablesDetails);
        PowerMockito.when(
            clickHouseService.queryClusterDetail(any(), any(), eq(ClickHouseConstant.QUERY_TYPE_VALUE_DATABASE), any(),
                any())).thenReturn(databasePageListResponse);
        PowerMockito.when(
            clickHouseService.queryClusterDetail(any(), any(), eq(ClickHouseConstant.QUERY_TYPE_VALUE_TABLE), any(),
                any())).thenReturn(tablePageListResponse);
        Endpoint endpoint1 = new Endpoint();
        endpoint1.setId(agentEnv1.getUuid());
        Endpoint endpoint2 = new Endpoint();
        endpoint2.setId(agentEnv1.getUuid());
        Endpoint endpoint3 = new Endpoint();
        endpoint3.setId(agentEnv1.getUuid());
        Endpoint endpoint4 = new Endpoint();
        endpoint4.setId(agentEnv1.getUuid());
        PowerMockito.when(clickHouseService.selectAgent(any()))
            .thenReturn(endpoint1)
            .thenReturn(endpoint2)
            .thenReturn(endpoint3)
            .thenReturn(endpoint4);
        // do invoke
        clickHouseBackupInterceptor.supplyNodes(backupTask);
    }

    /**
     * 用例场景：测试备份数据库，四个节点
     * 前置条件：无
     * 检查点：backupTask设置nodes信息正确
     */
    @Test
    public void test_database_supplyNodes2() {
        // init data
        BackupTask backupTask = new BackupTask();
        backupTask.setProtectEnv(new TaskEnvironment());
        TaskResource protectDatabase = new TaskResource();
        protectDatabase.setRootUuid("clusterUuid");
        protectDatabase.setUuid("tableSetUuid");
        protectDatabase.setParentName("databaseName");
        protectDatabase.setType(ClickHouseConstant.TABLE_SET_TYPE);
        backupTask.setProtectObject(protectDatabase);

        ProtectedEnvironment agentEnv1 = new ProtectedEnvironment();
        agentEnv1.setUuid("agentUuid1");
        agentEnv1.setEndpoint("192.168.0.1");
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setName("nodeResource1");
        Authentication auth = new Authentication();
        auth.setAuthType(0);
        Map<String, String> node1ExtendInfo = Maps.newHashMap();
        node1ExtendInfo.put(ClickHouseConstant.SHARD_NUM, "1");
        auth.setExtendInfo(Maps.newHashMap());
        nodeResource1.setAuth(auth);
        nodeResource1.setExtendInfo(node1ExtendInfo);
        nodeResource1.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv1)));

        ProtectedEnvironment agentEnv2 = new ProtectedEnvironment();
        agentEnv2.setUuid("agentUuid2");
        agentEnv2.setEndpoint("192.168.0.2");
        ProtectedResource nodeResource2 = new ProtectedResource();
        nodeResource2.setName("nodeResource2");
        nodeResource2.setAuth(auth);
        Map<String, String> node2ExtendInfo = Maps.newHashMap();
        node2ExtendInfo.put(ClickHouseConstant.SHARD_NUM, "1");
        nodeResource2.setExtendInfo(node2ExtendInfo);
        nodeResource2.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv2)));

        ProtectedResource nodeResource3 = new ProtectedResource();
        nodeResource3.setName("nodeResource3");
        nodeResource3.setAuth(auth);
        Map<String, String> node3ExtendInfo = Maps.newHashMap();
        node3ExtendInfo.put(ClickHouseConstant.SHARD_NUM, "2");
        nodeResource3.setExtendInfo(node3ExtendInfo);
        ProtectedEnvironment agentEnv3 = new ProtectedEnvironment();
        agentEnv3.setUuid("agentUuid3");
        agentEnv3.setEndpoint("192.168.0.3");
        nodeResource3.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv3)));

        ProtectedResource nodeResource4 = new ProtectedResource();
        nodeResource4.setName("nodeResource4");
        nodeResource4.setAuth(auth);
        Map<String, String> node4ExtendInfo = Maps.newHashMap();
        node4ExtendInfo.put(ClickHouseConstant.SHARD_NUM, "2");
        nodeResource4.setExtendInfo(node4ExtendInfo);
        ProtectedEnvironment agentEnv4 = new ProtectedEnvironment();
        agentEnv4.setUuid("agentUuid4");
        agentEnv4.setEndpoint("192.168.0.4");
        nodeResource4.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv4)));

        ProtectedEnvironment clusterEnv = new ProtectedEnvironment();
        clusterEnv.setUuid("clusterUuid");
        clusterEnv.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN,
            Lists.newArrayList(nodeResource1, nodeResource2, nodeResource3, nodeResource4)));

        ProtectedResource tableDetail1 = new ProtectedResource();
        tableDetail1.setName("tableName1");
        Map<String, String> extendInfo1 = Maps.newHashMap();
        extendInfo1.put(ClickHouseConstant.TABLE_ENGINE, "ReplicatedMergeTree");
        tableDetail1.setExtendInfo(extendInfo1);
        ProtectedResource tableDetail2 = new ProtectedResource();
        Map<String, String> extendInfo2 = Maps.newHashMap();
        tableDetail2.setName("tableName2");
        extendInfo2.put(ClickHouseConstant.TABLE_ENGINE, "ReplicatedMergeTree");
        tableDetail2.setExtendInfo(extendInfo2);
        List<ProtectedResource> queryTablesDetails = Lists.newArrayList(tableDetail1, tableDetail2);

        ProtectedResource databaseDetail = new ProtectedResource();
        Map<String, String> databaseExtendInfo = Maps.newHashMap();
        databaseExtendInfo.put(ClickHouseConstant.DB_NAME, "databaseName");
        databaseDetail.setExtendInfo(databaseExtendInfo);
        List<ProtectedResource> queryDatabaseDetails = Lists.newArrayList(databaseDetail);

        // mock

        ProtectedResource tableSet = new ProtectedResource();
        tableSet.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, queryTablesDetails));
        PowerMockito.when(resourceService.getResourceById("tableSetUuid")).thenReturn(Optional.of(tableSet));
        PowerMockito.when(resourceService.getResourceById("clusterUuid")).thenReturn(Optional.of(clusterEnv));
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("agentUuid1")).thenReturn(agentEnv1);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("agentUuid2")).thenReturn(agentEnv2);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("agentUuid3")).thenReturn(agentEnv3);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("agentUuid4")).thenReturn(agentEnv4);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("clusterUuid")).thenReturn(clusterEnv);
        PageListResponse<ProtectedResource> databasePageListResponse = new PageListResponse<>();
        databasePageListResponse.setRecords(queryDatabaseDetails);
        PageListResponse<ProtectedResource> tablePageListResponse = new PageListResponse<>();
        tablePageListResponse.setRecords(queryTablesDetails);
        PowerMockito.when(
            clickHouseService.queryClusterDetail(any(), any(), eq(ClickHouseConstant.QUERY_TYPE_VALUE_DATABASE), any(),
                any())).thenReturn(databasePageListResponse);
        PowerMockito.when(
            clickHouseService.queryClusterDetail(any(), any(), eq(ClickHouseConstant.QUERY_TYPE_VALUE_TABLE), any(),
                any())).thenReturn(tablePageListResponse);
        Endpoint endpoint1 = new Endpoint();
        endpoint1.setId(agentEnv1.getUuid());
        Endpoint endpoint2 = new Endpoint();
        endpoint2.setId(agentEnv1.getUuid());
        Endpoint endpoint3 = new Endpoint();
        endpoint3.setId(agentEnv1.getUuid());
        Endpoint endpoint4 = new Endpoint();
        endpoint4.setId(agentEnv1.getUuid());
        PowerMockito.when(clickHouseService.selectAgent(any()))
            .thenReturn(endpoint1)
            .thenReturn(endpoint2)
            .thenReturn(endpoint3)
            .thenReturn(endpoint4);

        // do invoke
        clickHouseBackupInterceptor.supplyNodes(backupTask);
    }
}
