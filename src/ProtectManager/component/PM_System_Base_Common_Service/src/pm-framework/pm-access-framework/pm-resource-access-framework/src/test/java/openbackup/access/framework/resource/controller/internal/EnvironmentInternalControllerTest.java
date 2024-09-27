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
package openbackup.access.framework.resource.controller.internal;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.*;
import static org.mockito.ArgumentMatchers.anyString;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.delete;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;

import openbackup.access.framework.resource.dto.ProtectedEnvironmentDto;
import openbackup.access.framework.resource.service.ProtectedResourceRepository;
import openbackup.access.framework.resource.testdata.MockEntity;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.auth.api.HcsTokenAPi;
import openbackup.system.base.sdk.resource.enums.LinuxOsTypeEnum;

import openbackup.system.base.sdk.auth.model.RoleBo;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import com.huawei.oceanprotect.system.base.user.service.UserService;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.MvcResult;
import org.springframework.test.web.servlet.result.MockMvcResultHandlers;
import org.springframework.test.web.servlet.result.MockMvcResultMatchers;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.util.NestedServletException;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * EnvironmentInternalController测试类
 *
 * @author w00616953
 * @since 2021-12-17
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {EnvironmentInternalController.class, SqlInitializationAutoConfiguration.class})
@AutoConfigureMockMvc
@AutoConfigureWebMvc
public class EnvironmentInternalControllerTest {
    @Autowired
    private MockMvc mvc;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private ProtectedEnvironmentService environmentService;

    @MockBean
    private UserService userService;

    @MockBean
    private HcsTokenAPi hcsTokenAPi;

    @MockBean(name = "hcsUserRestTemplate")
    private RestTemplate restTemplate;

    @MockBean
    private ProtectedResourceRepository protectedResourceRepository;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * 用例名称：验证环境注册接口注册时uuid不合法是否会抛异常。<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：不合法的uuid无法注册。<br/>
     */
    @Test
    public void test_should_throw_LegoCheckedException_when_register_if_duplicate_not_valid() throws Exception {
        ProtectedEnvironmentDto protectedEnvironmentDto = MockEntity.mockProtectedEnvironmentDto();
        protectedEnvironmentDto.setUuid("ca4da9cf62d34bd5970fce853f5109c2");

        String content = JSONObject.fromObject(protectedEnvironmentDto).toString();

        try {
            mvc.perform(post("/v2/internal/environments").content(content).contentType(MediaType.APPLICATION_JSON));
        } catch (NestedServletException e) {
            assertThat(e.getMessage().contains("uuid is invalid."), equalTo(true));
        }
    }

    /**
     * 用例名称：验证环境注册接口注册时SanClient agent uuid不合法是否会抛异常。<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：不合法的uuid无法注册。<br/>
     */
    @Test
    public void test_should_throw_LegoCheckedException_when_register_if_sanclient_uuid_not_valid() throws Exception {
        ProtectedEnvironmentDto protectedEnvironmentDto = MockEntity.mockProtectedEnvironmentDto();
        protectedEnvironmentDto.setSubType("SBackupAgent");
        protectedEnvironmentDto.setUuid("363ca5cf-e365-498b-b906-1a8cf6981205_Sanclient");

        String content = JSONObject.fromObject(protectedEnvironmentDto).toString();

        try {
            mvc.perform(post("/v2/internal/environments").content(content).contentType(MediaType.APPLICATION_JSON));
        } catch (NestedServletException e) {
            assertThat(e.getMessage().contains("uuid is invalid."), equalTo(true));
        }
    }

    /**
     * 用例名称：验证注册时，如果数据库中存在相同的uuid，则只是更新环境信息。<br/>
     * 前置条件：spring mvc框架正常运行，数据库运行正常。<br/>
     * check点：不合法的uuid无法注册。<br/>
     */
    @Test
    public void test_register_not_invoked_when_register_agent_if_has_same_uuid_in_db() throws Exception {
        ProtectedEnvironmentDto protectedEnvironmentDto = MockEntity.mockProtectedEnvironmentDto();
        ProtectedEnvironment resource = MockEntity.mockProtectedEnvironment();
        resource.setUuid(protectedEnvironmentDto.getUuid());

        String content = JSONObject.fromObject(protectedEnvironmentDto).toString();

        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(resource));

        EnvironmentInternalController spy =
                PowerMockito.spy(
                        new EnvironmentInternalController(
                                environmentService, resourceService, userService));
        UserInnerResponse userInnerResponse = new UserInnerResponse();
        Mockito.when(userService.getUserInfoByUserId(anyString())).thenReturn(userInnerResponse);

        ProtectedEnvironment protectedEnvironment =
                ReflectionTestUtils.invokeMethod(spy, "convertToProtectedEnvironment", protectedEnvironmentDto);

        MvcResult result =
                mvc.perform(post("/v2/internal/environments").content(content).contentType(MediaType.APPLICATION_JSON))
                        .andDo(MockMvcResultHandlers.print())
                        .andExpect(MockMvcResultMatchers.status().isOk())
                        .andReturn();
        Mockito.verify(environmentService, Mockito.times(0)).register(protectedEnvironment);
    }

    /**
     * 用例名称：验证注册时，注册新agent成功。<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：能够成功注册agent。<br/>
     */
    @Test
    public void test_register_invoked_once_when_register_a_new_agent() throws Exception {
        ProtectedEnvironmentDto protectedEnvironmentDto = MockEntity.mockProtectedEnvironmentDto();

        String content = JSONObject.fromObject(protectedEnvironmentDto).toString();

        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        UserInnerResponse userInnerResponse = new UserInnerResponse();
        Mockito.when(userService.getUserInfoByUserId(anyString())).thenReturn(userInnerResponse);

        MvcResult result =
                mvc.perform(post("/v2/internal/environments").content(content).contentType(MediaType.APPLICATION_JSON))
                        .andDo(MockMvcResultHandlers.print())
                        .andExpect(MockMvcResultMatchers.status().isOk())
                        .andReturn();
        Mockito.verify(environmentService, Mockito.times(1)).register(Mockito.any());
    }

    /**
     * 用例名称：验证注册时，注册新agent成功。<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：能够成功注册agent。<br/>
     */
    @Test
    public void test_register_sanclient_once_when_register_a_new_agent() throws Exception {
        ProtectedEnvironmentDto protectedEnvironmentDto = MockEntity.mockProtectedEnvironmentDto();
        protectedEnvironmentDto.setSubType("SBackupAgent");
        protectedEnvironmentDto.setUuid("decf8540-dbeb-42c3-870c-fe377b415485_sanclient");

        String content = JSONObject.fromObject(protectedEnvironmentDto).toString();

        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        UserInnerResponse userInnerResponse = new UserInnerResponse();
        Mockito.when(userService.getUserInfoByUserId(anyString())).thenReturn(userInnerResponse);

        MvcResult result =
                mvc.perform(post("/v2/internal/environments").content(content).contentType(MediaType.APPLICATION_JSON))
                        .andDo(MockMvcResultHandlers.print())
                        .andExpect(MockMvcResultMatchers.status().isOk())
                        .andReturn();
        Mockito.verify(environmentService, Mockito.times(1)).register(Mockito.any());
    }

    /**
     * 用例名称：验证删除agent成功。<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：能够成功注册agent。<br/>
     */
    @Test
    public void test_delete_invoked_once_when_delete_agent() throws Exception {
        ProtectedEnvironmentDto protectedEnvironmentDto = MockEntity.mockProtectedEnvironmentDto();

        String content = JSONObject.fromObject(protectedEnvironmentDto).toString();

        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        UserInnerResponse userInnerResponse = new UserInnerResponse();
        Mockito.when(userService.getUserInfoByUserId(anyString())).thenReturn(userInnerResponse);

        MvcResult result =
                mvc.perform(delete("/v2/internal/environments/delete/1").content(content).contentType(MediaType.APPLICATION_JSON))
                        .andDo(MockMvcResultHandlers.print())
                        .andExpect(MockMvcResultMatchers.status().isOk())
                        .andReturn();
        Mockito.verify(environmentService, Mockito.times(1)).deleteEnvironmentById(Mockito.any());
    }

    /**
     * 用例名称：验证注册时，linux大类的osType统一展示为linux<br/>
     * 前置条件：agent正常注册<br/>
     * check点：属于linux的操作系统类型，统一展示为linux<br/>
     */
    @Test
    public void test_environment_os_type_should_be_linux_if_belongs_to_linux() {
        ProtectedEnvironmentDto protectedEnvironmentDto = MockEntity.mockProtectedEnvironmentDto();
        EnvironmentInternalController spy =
                PowerMockito.spy(
                        new EnvironmentInternalController(
                                environmentService, resourceService, userService));
        List<String> osTypes = new ArrayList<>();
        for (LinuxOsTypeEnum type : LinuxOsTypeEnum.values()) {
            osTypes.add(type.getName());
        }
        for (String osType : osTypes) {
            protectedEnvironmentDto.setOsType(osType);
            ProtectedEnvironment protectedEnvironment =
                    ReflectionTestUtils.invokeMethod(spy, "convertToProtectedEnvironment", protectedEnvironmentDto);
            if (!"SOLARIS".equals(osType))
                Assert.assertEquals("linux", protectedEnvironment.getOsType());
        }
    }

    /**
     * 用例名称：验证注册时，Unix大类的osType统一展示为aix<br/>
     * 前置条件：agent正常注册<br/>
     * check点：属于AIX的操作系统类型，统一展示为aix<br/>
     */
    @Test
    public void test_environment_os_type_should_be_aix_if_belongs_to_aix() {
        ProtectedEnvironmentDto protectedEnvironmentDto = MockEntity.mockProtectedEnvironmentDto();
        EnvironmentInternalController spy =
            PowerMockito.spy(
                new EnvironmentInternalController(
                    environmentService, resourceService, userService));
        protectedEnvironmentDto.setOsType("AIX");
        ProtectedEnvironment protectedEnvironment =
            ReflectionTestUtils.invokeMethod(spy, "convertToProtectedEnvironment", protectedEnvironmentDto);
        Assert.assertEquals("aix", protectedEnvironment.getOsType());
    }

    /**
     * 用例名称：验证注册时，如果userId为系统管理员，则userId置为null<br/>
     * 前置条件：agent正常注册<br/>
     * check点：系统管理员无需设置userId<br/>
     */
    @Test
    public void test_environment_user_id_null_if_is_sys_admin() {
        String userId = UUIDGenerator.getUUID();

        ProtectedEnvironmentDto protectedEnvironmentDto = MockEntity.mockProtectedEnvironmentDto();
        EnvironmentInternalController spy =
                PowerMockito.spy(
                        new EnvironmentInternalController(
                                environmentService, resourceService, userService));
        List<String> osTypes = new ArrayList<>();
        for (LinuxOsTypeEnum type : LinuxOsTypeEnum.values()) {
            osTypes.add(type.getName());
        }
        UserInnerResponse userInnerResponse = new UserInnerResponse();
        RoleBo roleBo = new RoleBo();
        roleBo.setRoleId("1");
        userInnerResponse.setRolesSet(Collections.singleton(roleBo));
        for (String osType : osTypes) {
            protectedEnvironmentDto.setOsType(osType);
            protectedEnvironmentDto.setUserId(userId);
            Mockito.when(userService.getUserInfoByUserId(userId)).thenReturn(userInnerResponse);
            ProtectedEnvironment protectedEnvironment =
                    ReflectionTestUtils.invokeMethod(spy, "convertToProtectedEnvironment", protectedEnvironmentDto);
            if (!"SOLARIS".equals(osType))
                Assert.assertEquals("linux", protectedEnvironment.getOsType());
            Assert.assertNull(protectedEnvironment.getUserId());
        }
    }

    /**
     * 用例名称：验证注册时，如果不为系统管理员，userId有值，且authorizedName为userName<br/>
     * 前置条件：agent正常注册<br/>
     * check点：非系统管理员，userId和authorizedName有值<br/>
     */
    @Test
    public void test_environment_user_id_not_null_if_not_sys_admin() {
        String userId = UUIDGenerator.getUUID();
        ProtectedEnvironmentDto protectedEnvironmentDto = MockEntity.mockProtectedEnvironmentDto();
        EnvironmentInternalController spy =
                PowerMockito.spy(
                        new EnvironmentInternalController(
                                environmentService, resourceService, userService));
        List<String> osTypes = new ArrayList<>();
        for (LinuxOsTypeEnum type : LinuxOsTypeEnum.values()) {
            osTypes.add(type.getName());
        }
        UserInnerResponse userInnerResponse = new UserInnerResponse();
        RoleBo roleBo = new RoleBo();
        roleBo.setRoleId("2");
        userInnerResponse.setRolesSet(Collections.singleton(roleBo));
        for (String osType : osTypes) {
            protectedEnvironmentDto.setOsType(osType);
            protectedEnvironmentDto.setUserId(userId);
            Mockito.when(userService.getUserInfoByUserId(userId)).thenReturn(userInnerResponse);
            ProtectedEnvironment protectedEnvironment =
                    ReflectionTestUtils.invokeMethod(spy, "convertToProtectedEnvironment", protectedEnvironmentDto);
            if (!"SOLARIS".equals(osType))
                Assert.assertEquals("linux", protectedEnvironment.getOsType());
            Assert.assertEquals(userId, protectedEnvironment.getUserId());
            Assert.assertEquals(protectedEnvironmentDto.getUsername(), protectedEnvironment.getUsername());
        }
    }

    /**
     * 用例名称：验证设置agent(内置 + 外置)注册后延迟5min再去开始执行扫描<br/>
     * 前置条件：相应方法变量已mock<br/>
     * check点：成功设置触发定时扫描任务的开始时间<br/>
     */
    @Test
    public void test_set_delay_time_to_Scan() {
        EnvironmentInternalController environmentInternalController = new EnvironmentInternalController(
                environmentService, resourceService, userService);
        EnvironmentInternalController spy = Mockito.spy(environmentInternalController);
        ProtectedEnvironment environment = MockEntity.mockProtectedEnvironment();
        environment.setUuid("01234");
        Assert.assertNull(environment.getStartDate());

        ReflectionTestUtils.invokeMethod(spy,"setDelayTimeToScan", environment);
        Assert.assertNotNull(environment.getStartDate());
        Assert.assertEquals("01234", environment.getUuid());
    }

    /**
     * 用例名称：验证注册时，注册新agent成功。
     * 前置条件：spring mvc框架正常运行。
     * check点：能够成功注册agent。
     */
    @Test
    public void test_when_register_a_new_agent() throws Exception {
        ProtectedEnvironmentDto protectedEnvironmentDto = MockEntity.mockProtectedEnvironmentDto();

        String content = JSONObject.fromObject(protectedEnvironmentDto).toString();

        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        UserInnerResponse userInnerResponse = new UserInnerResponse();
        Mockito.when(userService.getUserInfoByUserId(anyString())).thenReturn(userInnerResponse);

        MvcResult result =
            mvc.perform(post("/v2/internal/environments").content(content).contentType(MediaType.APPLICATION_JSON))
                .andDo(MockMvcResultHandlers.print())
                .andExpect(MockMvcResultMatchers.status().isOk())
                .andReturn();
        Mockito.verify(environmentService, Mockito.times(1)).register(Mockito.any());
    }
}
