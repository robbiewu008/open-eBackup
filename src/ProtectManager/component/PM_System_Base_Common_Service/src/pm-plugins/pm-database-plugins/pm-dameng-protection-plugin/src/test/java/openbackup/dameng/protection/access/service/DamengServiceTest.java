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
package openbackup.dameng.protection.access.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.dameng.protection.access.DamengTestDataUtil;
import openbackup.dameng.protection.access.constant.DamengConstant;
import openbackup.dameng.protection.access.service.impl.DamengServiceImpl;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;

/**
 * dameng查询、校验接口的测试类
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-16
 */
@RunWith(PowerMockRunner.class)
public class DamengServiceTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final DamengServiceImpl damengService = new DamengServiceImpl(resourceService, agentUnifiedService,
        copyRestApi);

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * 用例场景：dameng查询集群信息
     * 前置条件：无
     * 检查点：返回信息正确
     */
    @Test
    public void check_success() {
        AppEnvResponse envResponse = buildAppEnvResponse();
        ProtectedResource protectedResource = buildProtectedResource();
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.of(
                DamengTestDataUtil.buildProtectedEnvironment("uuid", ResourceSubTypeEnum.DAMENG_CLUSTER.getType())));
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(envResponse);
        damengService.check(protectedResource);
        Assert.assertTrue(envResponse.getNodes()
            .get(0)
            .getEndpoint()
            .equals(protectedResource.getDependencies()
                .get(DatabaseConstants.CHILDREN)
                .get(0)
                .getEnvironment()
                .getEndpoint()));
    }

    /**
     * 用例场景：dameng集群校验
     * 前置条件：查询返回的实例信息和注册的实例信息不一致
     * 检查点：dameng集群校验失败
     */
    @Test
    public void should_throw_LegoCheckedException_when_check() {
        expectedException.expect(LegoCheckedException.class);
        AppEnvResponse envResponse = buildAppEnvResponse();
        ProtectedResource protectedResource = buildProtectedResource();
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment("",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        Map<String, String> extendInfo = envResponse.getNodes().get(0).getExtendInfo();
        extendInfo.put(DamengConstant.INSTANCESTATUS, "0");
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(environment));
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(envResponse);
        damengService.check(protectedResource);
    }

    /**
     * 用例场景：dameng获取已经注册的ip和实例端口集合
     * 前置条件：无
     * 检查点：返回信息正确
     */
    @Test
    public void get_existing_ip_and_port_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(getPageListResponse());
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.of(
                DamengTestDataUtil.buildProtectedEnvironment("", ResourceSubTypeEnum.DAMENG_CLUSTER.getType())));
        Set<String> uuidAndPortSet = damengService.getExistingUuidAndPort(
            DamengTestDataUtil.buildProtectedEnvironment("", ResourceSubTypeEnum.DAMENG_CLUSTER.getType()));
        Assert.assertTrue(uuidAndPortSet.contains("uuid_8080"));
    }

    /**
     * 用例场景：拼接主机IP和实例端口
     * 前置条件：无
     * 检查点：返回信息正确
     */
    @Test
    public void connect_uuid_and_port_success() {
        String uuidAndPort = damengService.connectUuidAndPort("uuid", "8080");
        Assert.assertEquals(uuidAndPort, "uuid_8080");
    }

    /**
     * 用例场景：从单实例的dependency里，获取对应的Agent主机
     * 前置条件：无
     * 检查点：返回信息正确
     */
    @Test
    public void query_agent_environment_success() {
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.of(
                DamengTestDataUtil.buildProtectedEnvironment("", ResourceSubTypeEnum.DAMENG_CLUSTER.getType())));
        ProtectedResource protectedResource = DamengTestDataUtil.getSubProtectedResourceList().get(0);
        ProtectedEnvironment environment = damengService.queryAgentEnvironment(protectedResource);
        Assert.assertEquals(environment.getEndpoint(), protectedResource.getEnvironment().getEndpoint());
    }

    /**
     * 用例场景：从单实例的dependency里，获取对应的Agent主机
     * 前置条件：dependency里没有agents
     * 检查点：获取Agent主机失败
     */
    @Test
    public void should_throw_LegoCheckedException_when_query_agent_environment() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedResource protectedResource = DamengTestDataUtil.getSubProtectedResourceList().get(0);
        protectedResource.getDependencies().remove(DatabaseConstants.AGENTS);
        damengService.queryAgentEnvironment(protectedResource);
    }

    /**
     * 用例场景：校验从agent查询到的集群信息
     * 前置条件：agent返回为空
     * 检查点：查询结果为空
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_cluster_consistence() {
        expectedException.expect(LegoCheckedException.class);
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(new AppEnvResponse());
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.of(
                DamengTestDataUtil.buildProtectedEnvironment("", ResourceSubTypeEnum.DAMENG_CLUSTER.getType())));
        ProtectedResource protectedResource = buildProtectedResource();
        damengService.check(protectedResource);
    }

    /**
     * 用例场景：校验从agent查询到的集群信息
     * 前置条件：agent返回集群有2个nodes，注册只有一个nodes
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_not_include_all_nodes() {
        expectedException.expect(LegoCheckedException.class);
        AppEnvResponse appEnvResponse = buildAppEnvResponse();
        List<NodeInfo> nodeInfoList = appEnvResponse.getNodes();
        nodeInfoList.addAll(DamengTestDataUtil.buildNodeInfo());
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(appEnvResponse);
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.of(
                DamengTestDataUtil.buildProtectedEnvironment("", ResourceSubTypeEnum.DAMENG_CLUSTER.getType())));
        ProtectedResource protectedResource = buildProtectedResource();
        damengService.check(protectedResource);
    }

    /**
     * 用例场景：根据Agent主机信息，获取Agent主机的Endpoint
     * 前置条件：无
     * 检查点：返回信息正确
     */
    @Test
    public void get_agent_endpoint_success() {
        Endpoint endpoint = damengService.getAgentEndpoint(
            DamengTestDataUtil.buildProtectedEnvironment(UUIDGenerator.getUUID(),
                ResourceSubTypeEnum.DAMENG_CLUSTER.getType()));
        Assert.assertEquals(endpoint.getIp(), "127.0.0.1");
    }

    /**
     * 用例场景：根据Agent主机信息，获取Agent主机的Endpoint
     * 前置条件：Agent主机信息没有端口
     * 检查点：获取Endpoint失败
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_agent_endpoint() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("127.0.0.1");
        damengService.getAgentEndpoint(environment);
    }

    /**
     * 用例场景：将nodes转换为NodeInfo
     * 前置条件：无
     * 检查点：返回信息正确
     */
    @Test
    public void get_node_info_from_nodes_success() {
        List<NodeInfo> nodeInfoList = damengService.getNodeInfoFromNodes(
            DamengTestDataUtil.buildProtectedEnvironment("", ResourceSubTypeEnum.DAMENG_CLUSTER.getType()));
        Assert.assertEquals(nodeInfoList.get(0).getEndpoint(), "127.0.0.1");
    }

    /**
     * 用例场景：针对集群实例，将子实例信息中的auth和主备信息设置到nodes中
     * 前置条件：AUTH信息存在
     * 检查点：不报错
     */
    @Test
    public void set_nodes_success() {
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment("uuid",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(environment));
        HostDto hostDto = new HostDto();
        hostDto.setUuid("uuid");
        PowerMockito.when(agentUnifiedService.getHost(any(), any())).thenReturn(hostDto);
        TaskEnvironment nodeEnv = new TaskEnvironment();
        nodeEnv.setUuid("uuid");
        damengService.buildTaskNodes("uuid");
        Assert.assertEquals(environment.getUuid(), nodeEnv.getUuid());
    }

    /**
     * 用例场景：恢复时校验数据库版本是否一致
     * 前置条件：数据库版本一致
     * 检查点：不报错
     */
    @Test
    public void check_db_version_success() {
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment("uuid",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(environment));
        Copy copy = new Copy();
        copy.setResourceProperties("{\"extendInfo\":{\"version\":\"V8\"}}");
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        damengService.checkDbVersion(DamengTestDataUtil.buildRestoreTask());
        Assert.assertTrue(environment.getExtendInfo().get(DamengConstant.VERSION).equals("V8"));
    }

    /**
     * 用例场景：恢复时校验数据库版本是否一致
     * 前置条件：数据库版本不一致
     * 检查点：报错
     */
    @Test
    public void should_throw_LegoCheckedException_if_check_db_version_failed() {
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment("uuid",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(environment));
        Copy copy = new Copy();
        copy.setResourceProperties("{\"extendInfo\":{\"version\":\"V2\"}}");
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        Assert.assertThrows(LegoCheckedException.class,
            () -> damengService.checkDbVersion(DamengTestDataUtil.buildRestoreTask()));
    }

    /**
     * 用例场景：设置恢复模式
     * 前置条件：副本类型为云上副本
     * 检查点：恢复模式设置正确
     */
    @Test
    public void set_restore_mode_success_if_copy_generated_by_cloud() {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        RestoreTask restoreTask = DamengTestDataUtil.buildRestoreTask();
        damengService.setRestoreMode(restoreTask);
        Assert.assertTrue(restoreTask.getRestoreMode().equals(RestoreModeEnum.DOWNLOAD_RESTORE.getMode()));
    }

    /**
     * 用例场景：设置恢复模式
     * 前置条件：副本类型为本地副本
     * 检查点：恢复模式设置正确
     */
    @Test
    public void set_restore_mode_success_if_copy_generated_by_local() {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        RestoreTask restoreTask = DamengTestDataUtil.buildRestoreTask();
        damengService.setRestoreMode(restoreTask);
        Assert.assertTrue(restoreTask.getRestoreMode().equals(RestoreModeEnum.LOCAL_RESTORE.getMode()));
    }

    /**
     * 用例场景：获取agent列表
     * 前置条件：无
     * 检查点：返回正确
     */
    @Test
    public void get_endpoint_list_success() {
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment("uuid",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(environment));
        List<Endpoint> endpointList = damengService.getEndpointList("uuid");
        Assert.assertTrue(endpointList.get(0).getId().equals("uuid"));
    }

    /**
     * 用例场景：填充auth信息成功
     * 前置条件：实例存在
     * 检查点：authPwd不为空
     */
    @Test
    public void update_auth_success() throws Exception {
        ProtectedResource resource = buildProtectedResource();
        resource.setUuid("uuid");
        ProtectedResource oldResource = buildProtectedResource();
        Authentication authentication = new Authentication();
        authentication.setAuthPwd("testPwd");
        oldResource.setAuth(authentication);
        PowerMockito.when(resourceService.getResourceById("uuid")).thenReturn(Optional.of(oldResource));
        Whitebox.invokeMethod(damengService, "updateAuth", Collections.singletonList(resource));
        Assert.assertEquals("testPwd", resource.getAuth().getAuthPwd());
    }

    /**
     * 用例场景：设置恢复的高级参数
     * 前置条件：无
     * 检查点：设置成功
     */
    @Test
    public void set_restore_advance_params_success() {
        RestoreTask restoreTask = DamengTestDataUtil.buildRestoreTask();
        damengService.setRestoreAdvanceParams(restoreTask);
        Assert.assertTrue(restoreTask.getAdvanceParams()
            .get(DamengConstant.TARGET_LOCATION_KEY)
            .equals(RestoreLocationEnum.ORIGINAL.getLocation()));
    }

    private AppEnvResponse buildAppEnvResponse() {
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setUuid("uuid");
        nodeInfo.setName("实例1");
        nodeInfo.setType("Database");
        nodeInfo.setSubType("Dameng-cluster");
        nodeInfo.setEndpoint("127.0.0.1");
        nodeInfo.setRole(1);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DamengConstant.INSTANCESTATUS, "1");
        extendInfo.put(DatabaseConstants.PORT, "8080");
        nodeInfo.setExtendInfo(extendInfo);
        List<NodeInfo> nodeInfoList = new ArrayList<>();
        nodeInfoList.add(nodeInfo);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setNodes(nodeInfoList);
        return appEnvResponse;
    }

    private ProtectedResource buildProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setAuth(new Authentication());
        protectedResource.setSubType(ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, DamengTestDataUtil.getSubProtectedResourceList());
        protectedResource.setDependencies(dependencies);
        return protectedResource;
    }

    private PageListResponse<ProtectedResource> getPageListResponse() {
        PageListResponse<ProtectedResource> agents = new PageListResponse<>();
        agents.setRecords(Collections.singletonList(
            DamengTestDataUtil.buildProtectedEnvironment("", ResourceSubTypeEnum.DAMENG_CLUSTER.getType())));
        agents.setTotalCount(1);
        return agents;
    }
}
