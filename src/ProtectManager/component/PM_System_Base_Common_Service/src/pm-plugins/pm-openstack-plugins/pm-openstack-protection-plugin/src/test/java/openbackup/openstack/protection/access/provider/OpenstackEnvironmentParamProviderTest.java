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
package openbackup.openstack.protection.access.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.common.OpenstackAgentService;
import openbackup.openstack.protection.access.common.OpenstackCommonService;
import openbackup.openstack.protection.access.common.OpenstackQuotaService;
import openbackup.openstack.protection.access.constant.KeyStoneConstant;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.keystone.KeyStoneService;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.auth.model.RoleBo;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import com.huawei.oceanprotect.system.base.user.entity.UserInfoEntity;
import com.huawei.oceanprotect.system.base.user.service.UserService;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;

/**
 * 功能描述: test OpenstackEnvironmentParamProvider
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(TokenBo.class)
public class OpenstackEnvironmentParamProviderTest {
    private static final KeyStoneService keyStoneService = Mockito.mock(KeyStoneService.class);

    private static final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private static final OpenstackAgentService agentService = Mockito.mock(OpenstackAgentService.class);

    private static final UserService userService = Mockito.mock(UserService.class);

    private static final OpenstackQuotaService quotaService = Mockito.mock(OpenstackQuotaService.class);
    private static final OpenstackCommonService openstackCommonService = Mockito.mock(OpenstackCommonService.class);

    private static OpenstackEnvironmentParamProvider openstackEnvironmentParamProvider;

    @BeforeClass
    public static void init() {
        openstackEnvironmentParamProvider = new OpenstackEnvironmentParamProvider(keyStoneService, resourceService,
                userService, agentService,quotaService);
        openstackEnvironmentParamProvider.setOpenstackCommonService(openstackCommonService);
    }

    /**
     * 测试场景：applicable匹配成功 <br/>
     * 前置条件：传入资源信息subtype类型为OpenStackContainer <br/>
     * 检查点：返回True
     */
    @Test
    public void test_applicable_success() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setSubType(ResourceSubTypeEnum.OPENSTACK_CONTAINER.getType());
        boolean isApplicable = openstackEnvironmentParamProvider.applicable(environment);
        Assert.assertTrue(isApplicable);
    }

    /**
     * 测试场景：环境参数检查和补齐成功 <br/>
     * 前置条件：环境参数正常 <br/>
     * 检查点：无异常抛出
     */
    @Test
    public void test_check_and_prepare_param_success() {
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        environment.setUuid(null);
        environment.getAuth().getExtendInfo().put(OpenstackConstant.CERTIFICATION, "");
        environment.getAuth().getExtendInfo().put(OpenstackConstant.REVOCATION_LIST, "");
        environment.getExtendInfo().put(OpenstackConstant.SERVICE_ID_KEY, "service_id1");
        MockFactory.mockTokenBo();
        ProtectedEnvironment dbEnvironment = MockFactory.mockEnvironment();
        Mockito.when(resourceService.getResourceById(anyBoolean(), any())).thenReturn(Optional.of(dbEnvironment));
        Mockito.when(quotaService.isRegisterOpenstack(any())).thenReturn(false);
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(0);
        registeredEnv.setRecords(new ArrayList<>());
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);

        openstackEnvironmentParamProvider.checkAndPrepareParam(environment);
        Assert.assertNotNull(environment.getUuid());
        Assert.assertNotNull(environment.getUserId());
    }

    /**
     * 测试场景：云核场景环境参数检查和补齐成功 <br/>
     * 前置条件：环境参数正常 <br/>
     * 检查点：无异常抛出
     */
    @Test
    public void test_check_and_prepare_param_success_if_register_openstack() {
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        environment.setUuid(null);
        environment.getExtendInfo().put(OpenstackConstant.REGISTER_SERVICE, OpenstackConstant.REGISTER_OPENSTACK);
        environment.getExtendInfo().put(OpenstackConstant.SERVICE_ID_KEY, "service_id1");
        environment.getDependencies().put(OpenstackConstant.AGENTS, new ArrayList<>());
        MockFactory.mockTokenBo();
        ProtectedEnvironment dbEnvironment = MockFactory.mockEnvironment();
        Mockito.when(resourceService.getResourceById(anyBoolean(), any())).thenReturn(Optional.of(dbEnvironment));
        UserInfoEntity user = new UserInfoEntity();
        user.setUuid("user_id1");
        user.setUserName("user_name1");
        Mockito.when(userService.getUserInfoByName(any())).thenReturn(user);
        UserInnerResponse innerResponse = new UserInnerResponse();
        RoleBo roleBo = new RoleBo();
        roleBo.setRoleName(Constants.Builtin.ROLE_DP_ADMIN);
        innerResponse.setRolesSet(Collections.singleton(roleBo));
        Mockito.when(userService.getUserInfoByUserId(any())).thenReturn(innerResponse);
        Mockito.doNothing().when(keyStoneService).registerOpenstack(any());
        Mockito.when(quotaService.isRegisterOpenstack(any())).thenReturn(true);
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(0);
        registeredEnv.setRecords(new ArrayList<>());
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);

        openstackEnvironmentParamProvider.checkAndPrepareParam(environment);
        Assert.assertNotNull(environment.getUuid());
        Assert.assertEquals(user.getUuid(), environment.getUserId());
    }

    /**
     * 测试场景：agent已被授权检查失败 <br/>
     * 前置条件：环境依赖的agent已被授权 <br/>
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_exception_if_agent_is_authorized_when_check_and_prepare_param() {
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        ProtectedResource agent = new ProtectedResource();
        agent.setUserId("user_id_other");
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        dependency.put(OpenstackConstant.AGENTS, Collections.singletonList(agent));
        environment.setDependencies(dependency);
        MockFactory.mockTokenBo();
        ProtectedEnvironment dbEnvironment = MockFactory.mockEnvironment();
        Mockito.when(resourceService.getResourceById(anyBoolean(), any())).thenReturn(Optional.of(dbEnvironment));
        UserInfoEntity user = new UserInfoEntity();
        user.setUuid("user_id1");
        user.setUserName("user_name1");
        Mockito.when(userService.getUserInfoByName(any())).thenReturn(user);
        UserInnerResponse innerResponse = new UserInnerResponse();
        RoleBo roleBo = new RoleBo();
        roleBo.setRoleName(Constants.Builtin.ROLE_DP_ADMIN);
        innerResponse.setRolesSet(Collections.singleton(roleBo));
        Mockito.when(userService.getUserInfoByUserId(any())).thenReturn(innerResponse);
        Mockito.when(quotaService.isRegisterOpenstack(any())).thenReturn(true);
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(0);
        registeredEnv.setRecords(new ArrayList<>());
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);
        Assert.assertThrows("agent has authorized.", LegoCheckedException.class, () ->
            openstackEnvironmentParamProvider.checkAndPrepareParam(environment));
    }

    /**
     * 测试场景：OpenStack平台数量超过限制 <br/>
     * 前置条件：数据库已有最大限制数量的环境 <br/>
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_exception_if_env_count_is_over_limit_when_check_and_prepare_param() {
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        environment.setUuid(null);
        MockFactory.mockTokenBo();
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        List<ProtectedResource> records = new ArrayList<>();
        for (int i = 0; i < 8; i++) {
            ProtectedEnvironment mockEnvironment = MockFactory.mockEnvironment();
            mockEnvironment.getExtendInfo().put(OpenstackConstant.SERVICE_ID_KEY, "service_"+i);
            records.add(mockEnvironment);
        }
        registeredEnv.setRecords(records);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);
        Assert.assertThrows("Environment count over limit.", LegoCheckedException.class, () ->
            openstackEnvironmentParamProvider.checkAndPrepareParam(environment));
    }


    /**
     * 测试场景：证书开关关闭后内容被清空 <br/>
     * 前置条件：证书开关关闭 <br/>
     * 检查点：证书内容为空
     */
    @Test
    public void test_cert_clear_if_enable_cert_is_off_when_check_and_prepare_param() {
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        environment.setUuid(null);
        environment.getExtendInfo().put(OpenstackConstant.ENABLE_CERT, "0");
        MockFactory.mockTokenBo();
        ProtectedEnvironment dbEnvironment = MockFactory.mockEnvironment();
        Mockito.when(resourceService.getResourceById(anyBoolean(), any())).thenReturn(Optional.of(dbEnvironment));
        Mockito.when(agentService.queryClusterInfo(any()))
            .thenReturn(mockAppEnvResponse("service_id1"));
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(0);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);
        openstackEnvironmentParamProvider.checkEnvironmentRepeat(environment);
        Assert.assertEquals("", environment.getAuth().getExtendInfo().get(OpenstackConstant.CERTIFICATION));
    }

    /**
     * 测试场景：检查环境是否重复注册失败 <br/>
     * 前置条件：agent返回的service_id为空 <br/>
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_exception_if_service_id_is_empty_when_check_environment_repeat() {
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        environment.setUuid(null);
        MockFactory.mockTokenBo();
        Mockito.doNothing().when(agentService).checkConnectivity(any());
        AppEnvResponse envResponse = mockAppEnvResponse("");
        Mockito.when(agentService.queryClusterInfo(any())).thenReturn(envResponse);
        Assert.assertThrows("service id is blank.", LegoCheckedException.class, () ->
            openstackEnvironmentParamProvider.checkEnvironmentRepeat(environment));
    }


    /**
     * 测试场景：检查环境是否重复注册失败 <br/>
     * 前置条件：环境名称为空 <br/>
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_exception_if_name_is_empty_when_check_environment_repeat() {
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        environment.setName("");
        MockFactory.mockTokenBo();
        Assert.assertThrows("env param is empty.", LegoCheckedException.class, () ->
            openstackEnvironmentParamProvider.checkEnvironmentRepeat(environment));
    }

    /**
     * 测试场景：检查环境是否重复注册 <br/>
     * 前置条件：无重复环境 <br/>
     * 检查点：无异常抛出
     */
    @Test
    public void test_check_environment_repeat_success() {
        ProtectedEnvironment env = MockFactory.mockEnvironment();
        env.setUuid(null);
        String serviceId = "endpoint_id1";
        Mockito.doNothing().when(agentService).checkConnectivity(any());
        AppEnvResponse envResponse = mockAppEnvResponse(serviceId);
        Mockito.when(agentService.queryClusterInfo(any())).thenReturn(envResponse);
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(0);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);

        openstackEnvironmentParamProvider.checkEnvironmentRepeat(env);
        Assert.assertEquals(serviceId, env.getExtendInfo().get(OpenstackConstant.SERVICE_ID_KEY));
    }

    /**
     * 测试场景：检查环境是否重复注册时环境已存在 <br/>
     * 前置条件：目标类型环境已存在 <br/>
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_exception_when_check_environment_repeat_if_query_env_exists() {
        ProtectedEnvironment env = MockFactory.mockEnvironment();
        env.setUuid(null);
        String serviceId = "endpoint_id1";
        Mockito.doNothing().when(agentService).checkConnectivity(any());
        AppEnvResponse envResponse = mockAppEnvResponse(serviceId);
        Mockito.when(agentService.queryClusterInfo(any())).thenReturn(envResponse);
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(1);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);

        Assert.assertThrows("Env has registered", LegoCheckedException.class,
            () -> openstackEnvironmentParamProvider.checkEnvironmentRepeat(env));
    }

    private AppEnvResponse mockAppEnvResponse(String serviceId) {
        AppEnvResponse envResponse = new AppEnvResponse();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(OpenstackConstant.SERVICE_ID_KEY, serviceId);
        extendInfo.put(OpenstackConstant.REGISTER_SERVICE, OpenstackConstant.REGISTER_OPENSTACK);
        extendInfo.put(KeyStoneConstant.CPS_IP, "1.1.1.1");
        envResponse.setExtendInfo(extendInfo);
        return envResponse;
    }

}
