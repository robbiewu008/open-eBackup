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
package openbackup.data.access.framework.core.agent.impl;

import static org.assertj.core.api.Assertions.assertThatNoException;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentIqnValidateRequest;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentWwpnInfo;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CleanAgentLogReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CollectAgentLogRsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.GetAgentLogCollectStatusRsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.GetClusterEsnReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Rsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.PluginsDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.SupportApplicationDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.SupportPluginDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.UpdateAgentLevelReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.UpdateAgentPluginTypeReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.model.AgentUpdatePluginTypeResult;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.agent.mock.MockEntity;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.agent.model.AgentUpdateResponse;
import openbackup.system.base.sdk.cert.request.PushUpdateCertToAgentReq;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.service.AvailableAgentManagementDomainService;
import openbackup.system.base.service.DeployTypeService;

import feign.FeignException;
import feign.Request;
import feign.RequestTemplate;
import feign.Response;
import feign.codec.Encoder;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.junit.jupiter.api.Assertions;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.net.InetSocketAddress;
import java.net.Proxy;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

/**
 * AgentUnifiedServiceImplTest 测试类
 *
 */
@SpringBootTest(classes = {AgentUnifiedServiceImpl.class, FeignBuilder.class})
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@PrepareForTest({FeignBuilder.class, Response.class})
public class AgentUnifiedServiceImplTest {
    @Mock
    private static AgentUnifiedRestApi agentUnifiedRestApi;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Autowired
    private AgentUnifiedService agentUnifiedService;

    @MockBean
    private AvailableAgentManagementDomainService domainService;

    @MockBean
    private Encoder encoder;

    @MockBean
    private DeployTypeService deployTypeService;

    @Before
    public void init() throws Exception {
        PowerMockito.mockStatic(FeignBuilder.class);
        PowerMockito.when(
                FeignBuilder.buildHttpsTarget(any(), any(), anyBoolean(), anyBoolean(), any()))
            .thenReturn(agentUnifiedRestApi);
        PowerMockito.when(domainService.getDmeProxy(any()))
            .thenReturn(Optional.of(new Proxy(Proxy.Type.HTTP,
                new InetSocketAddress("127.0.0.1", 8088))));
    }

    /**
     * 用例场景：agent正常返回数据
     * 前置条件：agent正常返回数据(无数据的返回, )
     * 检查点：agent正常返回数据，接收正常、转换正常
     */
    @Test
    public void test_getDetailPageList() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        PowerMockito.when(deployTypeService.isHyperDetectDeployType()).thenReturn(true);
        // 无数据的返回
        PowerMockito.when(agentUnifiedRestApi.listResourceDetailV2(any(), any(), any())).thenReturn(null);

        PageListResponse<ProtectedResource> response = agentUnifiedService.getDetailPageList("", "127.0.0.1", 25080,
            new ListResourceV2Req());
        Assert.assertEquals(response.getTotalCount(), 0);

        // 无数据的返回：有total
        String resultJson2
            = "{\"errorCode\":\"0\",\"errorMessage\":\"\",\"resourceList\":{\"items\":,\"pageNo\":0,\"pageSize\":100,\"pages\":0,\"total\":23}}";
        ListResourceV2Rsp resourceV2Rsp2 = JSONObject.toBean(resultJson2, ListResourceV2Rsp.class);
        PowerMockito.when(agentUnifiedRestApi.listResourceDetailV2(any(), any(), any())).thenReturn(resourceV2Rsp2);

        PageListResponse<ProtectedResource> response2 = agentUnifiedService.getDetailPageList("", "127.0.0.1", 25080,
            new ListResourceV2Req());
        Assert.assertEquals(response2.getTotalCount(), 23);

        // 有数据的返回
        String resultJson
            = "{\"errorCode\": \"0\",\"errorMessage\": \"\",\"resourceList\": {\"items\": [{\"extendInfo\": {\"hasChildren\": true,\"modifyTime\": \"2020-01-19 09:31:15\",\"path\": \"/lost+found\",\"size\": 16384,\"type\": \"d\"},\"name\": \"\",\"parentName\": \"\",\"parentUuid\": \"\",\"subType\": \"\",\"type\": \"\",\"uuid\": \"23\"}],\"pageNo\": 0,\"pageSize\": 100,\"pages\": 0,\"total\": 23}}";
        ListResourceV2Rsp resourceV2Rsp = JSONObject.toBean(resultJson, ListResourceV2Rsp.class);
        PowerMockito.when(agentUnifiedRestApi.listResourceDetailV2(any(), any(), any())).thenReturn(resourceV2Rsp);

        PageListResponse<ProtectedResource> response3 = agentUnifiedService.getDetailPageList("", "127.0.0.1", 25080,
            new ListResourceV2Req());
        Assert.assertEquals(response3.getTotalCount(), 23);
        Assert.assertEquals(response3.getRecords().get(0).getExtendInfo().get("path"), "/lost+found");
    }

    /**
     * 用例场景：agent返回数据，返回错误码
     * 前置条件：agent获取数据失败，并上报了错误码
     * 检查点：正常接收，抛出错误码
     */
    @Test
    public void test_getDetailPageList_error() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        PowerMockito.when(deployTypeService.isHyperDetectDeployType()).thenReturn(false);
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("error test");

        String resultJson2 = "{\"errorCode\": 1577209919,\"errorMessage\": \"error test\"}";
        ListResourceV2Rsp resourceV2Rsp = JSONObject.toBean(resultJson2, ListResourceV2Rsp.class);
        PowerMockito.when(agentUnifiedRestApi.listResourceDetailV2(any(), any(), any())).thenReturn(resourceV2Rsp);
        PageListResponse<ProtectedResource> response = agentUnifiedService.getDetailPageList("", "127.0.0.1", 25080,
            new ListResourceV2Req());
        System.out.println(JSONObject.writeValueAsString(response));
    }

    /**
     * 用例场景：测试能够访问agent接口
     * 前置条件：agent进程运行正常，dme代理功能正常
     * 检查 点： 通过rest接口能够访问agent接口
     */
    @Test
    public void test_get_plugins_success() throws Exception {
        PluginsDto pluginsDto = MockEntity.mockPluginsDto();
        List<ProtectedResource> protectedResources = new ArrayList<>();
        protectedResources.add(new ProtectedResource());
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        String endpoint = "127.0.0.1";
        Integer port = 8080;
        PowerMockito.when(agentUnifiedRestApi.getPlugins(any())).thenReturn(pluginsDto).thenReturn(pluginsDto);
        List<ProtectedResource> plugins = agentUnifiedService.getPlugins(endpoint, port);
        Assert.assertEquals(5, plugins.size());
        endpoint = "2017:8:40:96:c12::51";
        List<ProtectedResource> ipv6Plugins = agentUnifiedService.getPlugins(endpoint, port);
        Assert.assertEquals(5, ipv6Plugins.size());
    }

    /**
     * 用例场景：测试能成功将Agent返回的插件信息转换成ProtectedResource
     * 前置条件：agent进程运行正常，能够返回插件信息；dme代理运行正常
     * 检查 点：成功将插件信息转换为受保护资源
     */
    @Test
    public void test_convert_plugins_to_protected_resource_success() {
        PluginsDto plugins = MockEntity.mockPluginsDto();

        Object protectedResources = ReflectionTestUtils.invokeMethod(agentUnifiedService,
            "convertPluginsToProtectedResources", plugins);

        Assert.assertTrue(protectedResources instanceof List);
        List resources = (List) protectedResources;
        Assert.assertEquals(5, resources.size());
    }

    /**
     * 用例场景：测试将Agent返回的插件对象，转换为受保护对象
     * 前置条件：agent成功响应
     * 检查 点：当agent返回的对象中，uuid为空时抛出异常
     */
    @Test
    public void test_should_raise_LegoCheckedException_when_convert_plugins_to_protected_resources_if_uuid_empty() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Uuid can not be empty.");

        PluginsDto pluginsDto = MockEntity.mockPluginsDto();
        pluginsDto.setUuid(null);
        Object protectedResources = ReflectionTestUtils.invokeMethod(agentUnifiedService,
            "convertPluginsToProtectedResources", pluginsDto);
    }

    /**
     * 用例场景：测试能够访问agent接口
     * 前置条件：agent进程运行正常，dme代理功能正常
     * 检查 点： 通过rest接口能够访问agent接口
     */
    @Test
    public void test_get_agent_host_success() throws Exception {
        HostDto hostDto = MockEntity.mockHostDto();

        PowerMockito.when(agentUnifiedRestApi.getHost(any())).thenReturn(hostDto);
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        HostDto host = agentUnifiedService.getHost("127.0.0.1", 8080);
        Assert.assertEquals("host dto", host.getName());
    }

    /**
     * 用例场景：测试能够访问agent接口
     * 前置条件：agent进程运行正常，dme代理功能正常
     * 检查 点： 通过rest接口能够访问agent接口
     */
    @Test
    public void test_get_agent_host_new_success() throws Exception {
        HostDto hostDto = MockEntity.mockHostDto();

        PowerMockito.when(agentUnifiedRestApi.getAgentHost(any())).thenReturn(hostDto);
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        HostDto host = agentUnifiedService.getAgentHost("127.0.0.1", 8080);
        Assert.assertEquals("host dto", host.getName());
    }

    /**
     * 用例场景：调用agent接口测试环境的连通性
     * 前置条件：agent环境网络畅通
     * 检查 点：1、调用的参数与预期相同，2、响应码为成功码
     */
    @Test
    public void check_application_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(UUID.randomUUID().toString());
        protectedResource.setName("test_name");
        protectedResource.setType(ResourceTypeEnum.BIG_DATA.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.HBASE.getType());
        protectedResource.setParentUuid(UUID.randomUUID().toString());
        protectedResource.setParentName("test_parent_name");

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);
        protectedResource.setAuth(authentication);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedRestApi.check(any(), any(), any())).thenReturn(mockAgentBaseDto);

        AgentBaseDto agentBaseDto = agentUnifiedService.checkApplication(protectedResource, protectedEnvironment);

        Assert.assertEquals("0", agentBaseDto.getErrorCode());
    }

    /**
     * 用例场景：连通性测试失败
     * 前置条件：1、agent环境网络畅通 2、agent API抛出错误
     * 检查 点：返回与预期相同的action result
     */
    @Test
    public void check_application_fail_when_agent_api_throwing_excepetion() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(UUID.randomUUID().toString());
        protectedResource.setName("test_name");
        protectedResource.setType(ResourceTypeEnum.BIG_DATA.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.HBASE.getType());
        protectedResource.setParentUuid(UUID.randomUUID().toString());
        protectedResource.setParentName("test_parent_name");

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);
        protectedResource.setAuth(authentication);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");

        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1);
        PowerMockito.when(agentUnifiedRestApi.check(any(), any(), any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                JSONObject.writeValueAsString(actionResult)));

        AgentBaseDto agentBaseDto = agentUnifiedService.checkApplication(protectedResource, protectedEnvironment);

        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, Long.parseLong(agentBaseDto.getErrorCode()));
        Assert.assertEquals(1, JSONObject.fromObject(agentBaseDto.getErrorMessage()).getLong("code"));
    }

    /**
     * 用例场景：连通性检查失败
     * 前置条件：1、agent环境网络畅通 2、agent API超时
     * 检查 点：返回与预期相同的action result
     */
    @Test
    public void check_application_fail_when_agent_api_connection_timeout() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(UUID.randomUUID().toString());
        protectedResource.setName("test_name");
        protectedResource.setType(ResourceTypeEnum.BIG_DATA.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.HBASE.getType());
        protectedResource.setParentUuid(UUID.randomUUID().toString());
        protectedResource.setParentName("test_parent_name");

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);
        protectedResource.setAuth(authentication);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");

        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1);
        FeignException.InternalServerError connectTimeoutException = new FeignException.InternalServerError(
            "Connect Timeout",
            Request.create(Request.HttpMethod.GET, "", Collections.emptyMap(), new byte[0], StandardCharsets.UTF_8,
                new RequestTemplate()), new byte[0], new HashMap<>());
        PowerMockito.when(agentUnifiedRestApi.check(any(), any(), any())).thenThrow(connectTimeoutException);

        AgentBaseDto agentBaseDto = agentUnifiedService.checkApplication(protectedResource, protectedEnvironment);

        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, Long.parseLong(agentBaseDto.getErrorCode()));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR,
            JSONObject.fromObject(agentBaseDto.getErrorMessage()).getLong("code"));
        Assert.assertEquals("Connect Timeout",
            JSONObject.fromObject(agentBaseDto.getErrorMessage()).getString("message"));
    }

    /**
     * 用例场景：收集agent日志
     * 前置条件：1、agent环境网络畅通
     * 检查 点：返回成功
     */
    @Test
    public void test_collect_agent_log_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        CollectAgentLogRsp mockRsp = new CollectAgentLogRsp();
        mockRsp.setId("123");
        PowerMockito.when(agentUnifiedRestApi.collectAgentLog(any())).thenReturn(mockRsp);
        CollectAgentLogRsp rsp = agentUnifiedService.collectAgentLog("1.1.1.1", 430);
        Assert.assertEquals("123", rsp.getId());
    }

    /**
     * 用例场景：收集agent日志
     * 前置条件：1、agent环境网络不同 2、agent离线
     * 检查 点：收集失败，抛出异常
     */
    @Test
    @Ignore
    public void test_collect_agent_log_failed_when_offline() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        FeignException.InternalServerError connectTimeoutException = new FeignException.InternalServerError(
            "Connect Timeout",
            Request.create(Request.HttpMethod.GET, "", Collections.emptyMap(), new byte[0], StandardCharsets.UTF_8,
                new RequestTemplate()), new byte[0], new HashMap<>());
        PowerMockito.when(agentUnifiedRestApi.collectAgentLog(any())).thenThrow(connectTimeoutException);
        Assertions.assertThrows(LegoCheckedException.class, () -> agentUnifiedService.collectAgentLog("1.1.1.1", 430));
    }

    /**
     * 用例场景：查询agent日志收集状态
     * 前置条件：1、agent环境网络畅通
     * 检查 点：返回成功
     */
    @Test
    public void test_get_collect_agent_log_status_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        GetAgentLogCollectStatusRsp mockRsp = new GetAgentLogCollectStatusRsp();
        mockRsp.setStatus("completed");
        PowerMockito.when(agentUnifiedRestApi.getCollectAgentLogStatus(any())).thenReturn(mockRsp);
        GetAgentLogCollectStatusRsp rsp = agentUnifiedService.getCollectAgentLogStatus("1.1.1.1", 430);
        Assert.assertEquals("completed", rsp.getStatus());
    }

    /**
     * 用例场景：查询agent日志收集状态
     * 前置条件：1、agent环境网络不同 2、agent离线
     * 检查 点：收集失败，抛出异常
     */
    @Test
    @Ignore
    public void test_get_collect_agent_log_status_failed_when_offline() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        FeignException.InternalServerError connectTimeoutException = new FeignException.InternalServerError(
            "Connect Timeout",
            Request.create(Request.HttpMethod.GET, "", Collections.emptyMap(), new byte[0], StandardCharsets.UTF_8,
                new RequestTemplate()), new byte[0], new HashMap<>());
        PowerMockito.when(agentUnifiedRestApi.getCollectAgentLogStatus(any())).thenThrow(connectTimeoutException);
        Assertions.assertThrows(LegoCheckedException.class,
            () -> agentUnifiedService.getCollectAgentLogStatus("1.1.1.1", 430));
    }

    /**
     * 用例场景：导出agent日志
     * 前置条件：1、agent环境网络畅通
     * 检查 点：返回成功
     */
    @Test
    public void test_export_agent_log_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        Response response = PowerMockito.mock(Response.class);
        PowerMockito.when(agentUnifiedRestApi.exportAgentLog(any(), anyString(), anyLong())).thenReturn(response);
        Response rsp = agentUnifiedService.exportAgentLog("1.1.1.1", 430, "123", 300);
        Assert.assertNotNull(rsp);
    }

    /**
     * 用例场景：更新日志级别
     * 前置条件：1、agent环境网络畅通
     * 检查 点：返回成功
     */
    @Test
    public void test_update_agent_log_level_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        PowerMockito.doNothing().when(agentUnifiedRestApi).updateAgentLogLevel(any(), any());
        agentUnifiedService.updateAgentLogLevel("1.1.1.1", 430, UpdateAgentLevelReq.builder().level(1).build());
        Mockito.verify(agentUnifiedRestApi, Mockito.times(1)).updateAgentLogLevel(any(), any());
    }

    /**
     * 用例场景：更新日志级别
     * 前置条件：1、agent环境网络不同 2、agent离线
     * 检查 点：更新日志级别失败，抛出异常
     */
    @Test
    @Ignore
    public void test_update_agent_log_level_failed_when_offline() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        FeignException.InternalServerError connectTimeoutException = new FeignException.InternalServerError(
            "Connect Timeout",
            Request.create(Request.HttpMethod.GET, "", Collections.emptyMap(), new byte[0], StandardCharsets.UTF_8,
                new RequestTemplate()), new byte[0], new HashMap<>());
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        PowerMockito.doThrow(connectTimeoutException).when(agentUnifiedRestApi).updateAgentLogLevel(any(), any());
        Assertions.assertThrows(LegoCheckedException.class,
            () -> agentUnifiedService.updateAgentLogLevel("1.1.1.1", 430,
                UpdateAgentLevelReq.builder().level(0).build()));
    }

    /**
     * 用例场景：通知Agent清理日志
     * 前置条件：agent环境网络连通
     * 检查 点：通知成功
     */
    @Test
    public void test_clean_agent_log_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        PowerMockito.doNothing().when(agentUnifiedRestApi).cleanAgentLog(any(), any());
        try {
            agentUnifiedService.cleanAgentLog("1.1.1.1", 430, CleanAgentLogReq.builder().id("1").build());
        } catch (Exception e) {
            Assert.fail();
        }
        Mockito.verify(agentUnifiedRestApi, Mockito.times(1)).cleanAgentLog(any(), any());
    }

    /**
     * 用例场景：清理Agent失败
     * 前置条件：1、agent环境网络不同 2、agent离线
     * 检查 点：清理Agent失败，抛出异常
     */
    @Test
    @Ignore
    public void test_clean_agent_log_when_offline() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        FeignException.InternalServerError connectTimeoutException = new FeignException.InternalServerError(
            "Connect Timeout",
            Request.create(Request.HttpMethod.GET, "", Collections.emptyMap(), new byte[0], StandardCharsets.UTF_8,
                new RequestTemplate()), new byte[0], new HashMap<>());
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        PowerMockito.doThrow(connectTimeoutException).when(agentUnifiedRestApi).cleanAgentLog(any(), any());
        Assertions.assertThrows(LegoCheckedException.class,
            () -> agentUnifiedService.cleanAgentLog("1.1.1.1", 430, CleanAgentLogReq.builder().id("1").build()));
    }

    /**
     * 用例场景：查询agent wwpn
     * 前置条件：1、agent环境网络畅通
     * 检查 点：返回成功
     */
    @Test
    public void test_list_wwpn_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        AgentWwpnInfo wwpnInfo = new AgentWwpnInfo();
        wwpnInfo.setUuid("12313");
        PowerMockito.when(agentUnifiedRestApi.listWwpn(any())).thenReturn(wwpnInfo);
        AgentWwpnInfo res = agentUnifiedService.listWwpn("1.1.1.1", 430, null);
        Assert.assertEquals("12313", res.getUuid());
    }

    /**
     * 用例场景：查询agent wwpn
     * 前置条件：1、查询agent接口异常
     * 检查 点：返回失败
     */
    @Test
    @Ignore
    public void test_list_wwpn_exception() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        PowerMockito.doThrow(new LegoUncheckedException("111")).when(agentUnifiedRestApi).listWwpn(any());
        try {
            agentUnifiedService.listWwpn("1.1.1.1", 430, null);
            Assert.fail();
        } catch (LegoCheckedException e) {
            Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, e.getErrorCode());
        }
    }

    /**
     * 用例场景：获取iqn验证信息
     * 前置条件：1、agent环境网络畅通
     * 检查 点：返回成功
     */
    @Test
    public void test_get_iqn_info_list_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        AgentWwpnInfo wwpnInfo = new AgentWwpnInfo();
        wwpnInfo.setUuid("123123");
        AgentIqnValidateRequest agentIqnValidateRequest = new AgentIqnValidateRequest();
        PowerMockito.when(agentUnifiedRestApi.getIqnInfoList(any(), any())).thenReturn(wwpnInfo);
        AgentWwpnInfo res = agentUnifiedService.getIqnInfoList("1.1.1.1", 430, agentIqnValidateRequest);
        Assert.assertEquals("123123", res.getUuid());
    }

    /**
     * 用例场景：扫描SanClient IQN信息
     * 前置条件：1、agent环境网络畅通
     * 检查 点：返回成功
     */
    @Test
    public void test_scan_san_client_iqn_info_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        AgentWwpnInfo agentWwpnInfo = new AgentWwpnInfo();
        List<String> iqns = new ArrayList<>();
        iqns.add("iqn");
        agentWwpnInfo.setWwpns(iqns);
        PowerMockito.when(agentUnifiedRestApi.scanSanClientIqnInfo(any())).thenReturn(agentWwpnInfo);
        AgentWwpnInfo res = agentUnifiedService.scanSanClientIqnInfo("1.1.1.1", 430);
        Assert.assertEquals("iqn", res.getWwpns().get(0));
    }

    /**
     * 用例场景：查询agent wwpn
     * 前置条件：1、agent环境网络畅通
     * 检查 点：返回成功
     */
    @Test
    public void test_reScan_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("111");
        PowerMockito.when(agentUnifiedRestApi.reScan(any())).thenReturn(agentBaseDto);
        AgentBaseDto res = agentUnifiedService.reScan("1.1.1.1", 430);
        Assert.assertEquals("111", res.getErrorCode());
    }

    /**
     * 用例场景：查询agent wwpn
     * 前置条件：1、agent扫盘异常
     * 检查 点：返回失败
     */
    @Test
    @Ignore
    public void test_reScan_exception() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        PowerMockito.doThrow(new LegoUncheckedException("111")).when(agentUnifiedRestApi).reScan(any());
        try {
            agentUnifiedService.reScan("1.1.1.1", 430);
            Assert.fail();
        } catch (LegoCheckedException e) {
            Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, e.getErrorCode());
        }
    }

    /**
     * 用例场景：通过agent查询集群esn
     * 前置条件：1、agent环境网络畅通
     * 检查 点：返回成功
     */
    @Test
    public void test_get_cluster_esn_by_agent_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        GetClusterEsnReq mockRsp = new GetClusterEsnReq();
        mockRsp.setEsn("esn");
        PowerMockito.when(agentUnifiedRestApi.getClusterEsn(any())).thenReturn(mockRsp);
        String esn = agentUnifiedService.queryRelatedClusterEsnByAgent("1.1.1.1", 430);
        Assert.assertEquals("esn", esn);
    }

    /**
     * 用例场景：通过agent查询集群esn
     * 前置条件：1、agent环境网络不同 2、agent离线
     * 检查 点：收集失败，抛出异常
     */
    @Test
    @Ignore
    public void test_get_cluster_esn_by_agent_failed_when_offline() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        FeignException.InternalServerError connectTimeoutException = new FeignException.InternalServerError(
                "Connect Timeout",
                Request.create(Request.HttpMethod.GET, "", Collections.emptyMap(), new byte[0],
                        StandardCharsets.UTF_8, new RequestTemplate()), new byte[0], new HashMap<>());
        PowerMockito.when(agentUnifiedRestApi.getClusterEsn(any())).thenThrow(connectTimeoutException);
        Assertions.assertThrows(LegoCheckedException.class,
                () -> agentUnifiedService.queryRelatedClusterEsnByAgent("1.1.1.1", 430));
    }

    /**
     * 用例场景：修改agent插件类型任务下发成功
     * 前置条件：agent环境网络连通
     * 检查 点：下发成功
     */
    @Test
    public void test_modify_plugin_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        AgentUpdateResponse response = new AgentUpdateResponse();
        response.setRevStatus(1);
        PowerMockito.when(agentUnifiedRestApi.modifyPlugin(any(), any())).thenReturn(response);
        UpdateAgentPluginTypeReq req = buildUpdatePluginTypeRequest();
        assertThatNoException().isThrownBy(() -> agentUnifiedService.modifyPluginType("1.1.1.1", 430, req));
    }

    private UpdateAgentPluginTypeReq buildUpdatePluginTypeRequest() {
        UpdateAgentPluginTypeReq req = new UpdateAgentPluginTypeReq();
        req.setAgentId("1686922785995112450");
        req.setJobId("baae3496-35c1-4095-8881-c5ae3de4845b");
        req.setDownloadLink(
            "https://[192.168.97.144,192.168.97.145]:25082/v1/host-agent/update/download?uuid=crmdownloadlinkd833e431c");
        return req;
    }

    /**
     * 用例场景：修改agent插件类型异常
     * 前置条件：1、修改agent插件类型异常
     * 检查 点：返回失败
     */
    @Test
    @Ignore
    public void test_modify_plugin_exception() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        String errorMsg = "update plugin type error";
        PowerMockito.doThrow(new LegoCheckedException(errorMsg)).when(agentUnifiedRestApi).modifyPlugin(any(), any());
        UpdateAgentPluginTypeReq req = buildUpdatePluginTypeRequest();
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> agentUnifiedService.modifyPluginType("1.1.1.1", 430, req));
        Assert.assertEquals(exception.getMessage(), errorMsg);
        PowerMockito.doThrow(FeignException.errorStatus("modifyPluginType", Response.builder()
            .status(500)
            .reason("message error")
            .request(
                Request.create(Request.HttpMethod.PUT, "/v1/agent/host/action/modifyPlugin", Collections.emptyMap(),
                    null, null, null))
            .body(new byte[] {})
            .build())).when(agentUnifiedRestApi).modifyPlugin(any(), any());
        exception = Assert.assertThrows(LegoCheckedException.class,
            () -> agentUnifiedService.modifyPluginType("1.1.1.1", 430, req));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, exception.getErrorCode());
    }

    /**
     * 用例场景：查询agent插件类型任务状态成功
     * 前置条件：agent环境网络连通
     * 检查 点：查询状态成功
     */
    @Test
    public void test_get_modify_plugin_result_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        AgentUpdatePluginTypeResult response = new AgentUpdatePluginTypeResult();
        response.setModifyStatus(1);
        PowerMockito.when(agentUnifiedRestApi.getModifyPluginTypeResult(any())).thenReturn(response);
        assertThatNoException().isThrownBy(() -> agentUnifiedService.getModifyPluginTypeResult("1.1.1.1", 430));
    }

    /**
     * 用例场景：查询agent插件类型任务状态异常
     * 前置条件：1、查询agent插件类型任务状态异常
     * 检查 点：返回失败
     */
    @Test
    @Ignore
    public void test_get_modify_plugin_result_exception() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        String errorMsg = "update plugin type error";
        PowerMockito.doThrow(new LegoCheckedException(errorMsg)).when(agentUnifiedRestApi).getModifyPluginTypeResult(any());
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> agentUnifiedService.getModifyPluginTypeResult("1.1.1.1", 430));
        Assert.assertEquals(exception.getMessage(), errorMsg);
        PowerMockito.doThrow(FeignException.errorStatus("modifyPluginType", Response.builder()
            .status(500)
            .reason("message error")
            .request(
                Request.create(Request.HttpMethod.PUT, "/v1/agent/host/action/modifyPlugin", Collections.emptyMap(),
                    null, null, null))
            .body(new byte[] {})
            .build())).when(agentUnifiedRestApi).getModifyPluginTypeResult(any());
        exception = Assert.assertThrows(LegoCheckedException.class,
            () -> agentUnifiedService.getModifyPluginTypeResult("1.1.1.1", 430));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, exception.getErrorCode());
    }

    /**
     * 用例场景：仓库解挂载
     * 前置条件：输入ip、端口、类型、请求体
     * 检查 点：返回失败
     */
    @Test
    @Ignore
    public void test_remove_protect_unmount_repo_exception() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        PowerMockito.doThrow(new LegoUncheckedException("111"))
            .when(agentUnifiedRestApi)
            .removeProtectUnmountRepo(any(), anyString(), anyString());
        Assert.assertThrows(LegoCheckedException.class,
            () -> agentUnifiedService.removeProtectUnmountRepo("1.1.1.1", 430, "OceanBase-cluster", ""));
    }

    @Test
    public void test_getPlugins() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);
        PluginsDto plugins = new PluginsDto();
        plugins.setUuid("fe387935-571e-447e-bee5-e50d949810d7");
        List<SupportPluginDto> supportPlugins = new ArrayList<>();
        plugins.setSupportPlugins(supportPlugins);
        SupportPluginDto supportPluginDto = new SupportPluginDto();
        supportPluginDto.setPluginName("FusionComputePlugin");
        supportPluginDto.setPluginVersion("1.0.0");
        List<SupportApplicationDto> supportApplications = new ArrayList<>();
        SupportApplicationDto supportApplicationDto = new SupportApplicationDto();
        supportApplicationDto.setApplication("FusionCompute");
        supportApplications.add(supportApplicationDto);
        supportPluginDto.setSupportApplications(supportApplications);
        supportPlugins.add(supportPluginDto);
        supportPluginDto = new SupportPluginDto();
        supportPluginDto.setPluginName("VirtualizationPlugin");
        supportPluginDto.setPluginVersion("1.0.0");
        supportPlugins.add(supportPluginDto);
        PowerMockito.when(agentUnifiedRestApi.getPlugins(any())).thenReturn(plugins);
        List<ProtectedResource> resources = agentUnifiedService.getPlugins("127.0.0.1", 5985);
        Assert.assertEquals(resources.size(), 1);
    }

    /**
     * 用例场景：给agent推送证书成功
     * 前置条件：无
     * 检查点：推送成功
     */
    @Test
    public void test_pushCertToAgent_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedRestApi.pushCertToAgent(any(), any())).thenReturn(mockAgentBaseDto);

        AgentBaseDto agentBaseDto = agentUnifiedService.pushCertToAgent(protectedEnvironment, any());

        Assert.assertEquals("0", agentBaseDto.getErrorCode());
    }

    /**
     * 用例场景：给agent推送证书，agent失败，正常返回失败信息
     * 前置条件：无
     * 检查点：推送失败，agent返回异常信息处理成功
     */
    @Test
    public void test_pushCertToAgent_agent_api_fail() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");

        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1);
        PowerMockito.when(agentUnifiedRestApi.pushCertToAgent(any(), any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
            JSONObject.writeValueAsString(actionResult)));

        AgentBaseDto agentBaseDto = agentUnifiedService.pushCertToAgent(protectedEnvironment, any());

        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, Long.parseLong(agentBaseDto.getErrorCode()));
        Assert.assertEquals(1, JSONObject.fromObject(agentBaseDto.getErrorMessage()).getLong("code"));
    }

    /**
     * 用例场景：给agent推送证书，agent连接超时，推送失败
     * 前置条件：无
     * 检查点：推送失败，连接超时异常信息处理成功
     */
    @Test
    public void test_pushCertToAgent_agent_connect_timeout_fail() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");

        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1);

        FeignException.InternalServerError connectTimeoutException = new FeignException.InternalServerError(
            "Connect Timeout",
            Request.create(Request.HttpMethod.GET, "", Collections.emptyMap(), new byte[0], StandardCharsets.UTF_8,
                new RequestTemplate()), new byte[0], new HashMap<>());

        PowerMockito.when(agentUnifiedRestApi.pushCertToAgent(any(), any()))
            .thenThrow(connectTimeoutException);

        AgentBaseDto agentBaseDto = agentUnifiedService.pushCertToAgent(protectedEnvironment, any());

        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, Long.parseLong(agentBaseDto.getErrorCode()));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR,
            JSONObject.fromObject(agentBaseDto.getErrorMessage()).getLong("code"));
        Assert.assertEquals("Connect Timeout",
            JSONObject.fromObject(agentBaseDto.getErrorMessage()).getString("message"));
    }

    /**
     * 用例场景：通知agent更新证书成功
     * 前置条件：无
     * 检查点：成功通知
     */
    @Test
    public void test_notifyAgentUpdateCert_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedRestApi.notifyAgentUpdateCert(any(), any())).thenReturn(mockAgentBaseDto);

        AgentBaseDto agentBaseDto = agentUnifiedService.notifyAgentUpdateCert(protectedEnvironment,
            new PushUpdateCertToAgentReq());

        Assert.assertEquals("0", agentBaseDto.getErrorCode());
    }

    /**
     * 用例场景：通知agent更新证书，agent失败，正常返回失败信息
     * 前置条件：无
     * 检查点：通知失败，agent返回异常信息处理成功
     */
    @Test
    public void test_notifyAgentUpdateCert_agent_api_fail() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");

        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1);
        PowerMockito.when(agentUnifiedRestApi.notifyAgentUpdateCert(any(), any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                JSONObject.writeValueAsString(actionResult)));

        AgentBaseDto agentBaseDto = agentUnifiedService.notifyAgentUpdateCert(protectedEnvironment,
            new PushUpdateCertToAgentReq());

        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, Long.parseLong(agentBaseDto.getErrorCode()));
        Assert.assertEquals(1, JSONObject.fromObject(agentBaseDto.getErrorMessage()).getLong("code"));
    }

    /**
     * 用例场景：通知agent更新证书，agent连接超时，通知失败
     * 前置条件：无
     * 检查点：通知失败，连接超时异常信息处理成功
     */
    @Test
    public void test_notifyAgentUpdateCert_agent_connect_timeout_fail() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");

        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1);

        FeignException.InternalServerError connectTimeoutException = new FeignException.InternalServerError(
            "Connect Timeout",
            Request.create(Request.HttpMethod.GET, "", Collections.emptyMap(), new byte[0], StandardCharsets.UTF_8,
                new RequestTemplate()), new byte[0], new HashMap<>());

        PowerMockito.when(agentUnifiedRestApi.notifyAgentUpdateCert(any(), any()))
            .thenThrow(connectTimeoutException);

        AgentBaseDto agentBaseDto = agentUnifiedService.notifyAgentUpdateCert(protectedEnvironment,
            new PushUpdateCertToAgentReq());

        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, Long.parseLong(agentBaseDto.getErrorCode()));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR,
            JSONObject.fromObject(agentBaseDto.getErrorMessage()).getLong("code"));
        Assert.assertEquals("Connect Timeout",
            JSONObject.fromObject(agentBaseDto.getErrorMessage()).getString("message"));
    }

    /**
     * 用例场景：通知agent删除旧证书成功
     * 前置条件：无
     * 检查点：成功通知
     */
    @Test
    public void test_notifyAgentDeleteOldCert_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedRestApi.notifyAgentDeleteOldCert(any(), any())).thenReturn(mockAgentBaseDto);

        AgentBaseDto agentBaseDto = agentUnifiedService.notifyAgentDeleteOldCert(protectedEnvironment,
            new PushUpdateCertToAgentReq());

        Assert.assertEquals("0", agentBaseDto.getErrorCode());
    }

    /**
     * 用例场景：通知agent删除旧证书，agent失败，正常返回失败信息
     * 前置条件：无
     * 检查点：通知失败，agent返回异常信息处理成功
     */
    @Test
    public void test_notifyAgentDeleteOldCert_agent_api_fail() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");

        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1);
        PowerMockito.when(agentUnifiedRestApi.notifyAgentDeleteOldCert(any(), any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                JSONObject.writeValueAsString(actionResult)));

        AgentBaseDto agentBaseDto = agentUnifiedService.notifyAgentDeleteOldCert(protectedEnvironment,
            new PushUpdateCertToAgentReq());

        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, Long.parseLong(agentBaseDto.getErrorCode()));
        Assert.assertEquals(1, JSONObject.fromObject(agentBaseDto.getErrorMessage()).getLong("code"));
    }

    /**
     * 用例场景：通知agent删除旧证书，agent连接超时，通知失败
     * 前置条件：无
     * 检查点：通知失败，连接超时异常信息处理成功
     */
    @Test
    public void test_notifyAgentDeleteOldCert_agent_connect_timeout_fail() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");

        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1);

        FeignException.InternalServerError connectTimeoutException = new FeignException.InternalServerError(
            "Connect Timeout",
            Request.create(Request.HttpMethod.GET, "", Collections.emptyMap(), new byte[0], StandardCharsets.UTF_8,
                new RequestTemplate()), new byte[0], new HashMap<>());

        PowerMockito.when(agentUnifiedRestApi.notifyAgentDeleteOldCert(any(), any()))
            .thenThrow(connectTimeoutException);

        AgentBaseDto agentBaseDto = agentUnifiedService.notifyAgentDeleteOldCert(protectedEnvironment,
            new PushUpdateCertToAgentReq());

        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, Long.parseLong(agentBaseDto.getErrorCode()));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR,
            JSONObject.fromObject(agentBaseDto.getErrorMessage()).getLong("code"));
        Assert.assertEquals("Connect Timeout",
            JSONObject.fromObject(agentBaseDto.getErrorMessage()).getString("message"));
    }

    /**
     * 用例场景：通知agent回退证书成功
     * 前置条件：无
     * 检查点：成功通知
     */
    @Test
    public void test_notifyAgentFallbackCert_success() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedRestApi.notifyAgentFallbackCert(any(), any())).thenReturn(mockAgentBaseDto);

        AgentBaseDto agentBaseDto = agentUnifiedService.notifyAgentFallbackCert(protectedEnvironment,
            new PushUpdateCertToAgentReq());

        Assert.assertEquals("0", agentBaseDto.getErrorCode());
    }

    /**
     * 用例场景：通知agent回退证书，agent失败，正常返回失败信息
     * 前置条件：无
     * 检查点：通知失败，agent返回异常信息处理成功
     */
    @Test
    public void test_notifyAgentFallbackCert_agent_api_fail() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");

        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1);
        PowerMockito.when(agentUnifiedRestApi.notifyAgentFallbackCert(any(), any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                JSONObject.writeValueAsString(actionResult)));

        AgentBaseDto agentBaseDto = agentUnifiedService.notifyAgentFallbackCert(protectedEnvironment,
            new PushUpdateCertToAgentReq());

        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, Long.parseLong(agentBaseDto.getErrorCode()));
        Assert.assertEquals(1, JSONObject.fromObject(agentBaseDto.getErrorMessage()).getLong("code"));
    }

    /**
     * 用例场景：通知agent回退证书，agent连接超时，通知失败
     * 前置条件：无
     * 检查点：通知失败，连接超时异常信息处理成功
     */
    @Test
    public void test_notifyAgentFallbackCert_agent_connect_timeout_fail() {
        ReflectionTestUtils.setField(agentUnifiedService, "agentUnifiedRestApi", agentUnifiedRestApi);

        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setName("host_01");
        protectedEnvironment.setType(ResourceTypeEnum.HOST.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.PROTECT_AGENT.getType());
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        protectedEnvironment.setAuth(authentication);

        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");

        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1);

        FeignException.InternalServerError connectTimeoutException = new FeignException.InternalServerError(
            "Connect Timeout",
            Request.create(Request.HttpMethod.GET, "", Collections.emptyMap(), new byte[0], StandardCharsets.UTF_8,
                new RequestTemplate()), new byte[0], new HashMap<>());

        PowerMockito.when(agentUnifiedRestApi.notifyAgentFallbackCert(any(), any()))
            .thenThrow(connectTimeoutException);

        AgentBaseDto agentBaseDto = agentUnifiedService.notifyAgentFallbackCert(protectedEnvironment,
            new PushUpdateCertToAgentReq());

        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, Long.parseLong(agentBaseDto.getErrorCode()));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR,
            JSONObject.fromObject(agentBaseDto.getErrorMessage()).getLong("code"));
        Assert.assertEquals("Connect Timeout",
            JSONObject.fromObject(agentBaseDto.getErrorMessage()).getString("message"));
    }
}
