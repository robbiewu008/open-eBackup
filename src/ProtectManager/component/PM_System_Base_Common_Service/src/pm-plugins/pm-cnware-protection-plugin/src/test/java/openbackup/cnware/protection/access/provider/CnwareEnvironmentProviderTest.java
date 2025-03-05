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
package openbackup.cnware.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;

import openbackup.cnware.protection.access.service.CnwareCommonService;
import openbackup.cnware.protection.access.mock.CnwareMockUtil;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.util.DefaultRoleHelper;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;
import java.util.Optional;

/**
 * CnwareEnvironmentProvider测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({TokenBo.class, DefaultRoleHelper.class})
@Slf4j
public class CnwareEnvironmentProviderTest {
    @Mock
    private CnwareCommonService mockCnwareCommonService;
    @Mock
    private ResourceService mockResourceService;
    @Mock
    private AgentUnifiedService mockAgentUnifiedService;
    @Mock
    private ProtectedEnvironmentRetrievalsService mockEnvRetrievalsService;
    @Mock
    private AgentUnifiedService mockAgentService;
    @Mock
    private CnwareCommonService cnwareCommonService;
    private CnwareEnvironmentProvider cnwareEnvironmentProviderTest;

    @Before
    public void setUp() {
        cnwareEnvironmentProviderTest = new CnwareEnvironmentProvider(mockCnwareCommonService, mockResourceService,
            mockAgentUnifiedService, mockEnvRetrievalsService, mockAgentService);
    }

    /**
     * 用例场景：CNware环境检查类过滤
     * 前置条件：无
     * 检查点：类型为CNware时过滤成功
     */
    @Test
    public void test_application() {
        Assert.assertTrue(cnwareEnvironmentProviderTest.applicable(ResourceSubTypeEnum.CNWARE.getType()));
        Assert.assertFalse(cnwareEnvironmentProviderTest.applicable("object"));
    }

    /**
     * 用例场景：检查环境上的CNware资源是否已达上限
     * 前置条件：环境中已注册最大数量的CNware资源
     * 检查点：检查环境上的CNware资源是否已达上限，当已达上限时注册新的CNware资源则抛出异常
     */
    @Test
    public void test_checkCnwareCount_should_throw_exception_when_cnware_count_exceeds_maximum()
        throws NoSuchMethodException{
        ProtectedEnvironment environment = new ProtectedEnvironment();
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(8);
        PowerMockito.when(mockResourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);
        Class<CnwareEnvironmentProvider> providerClass = CnwareEnvironmentProvider.class;
        Method privateMethod = providerClass.getDeclaredMethod("checkCnwareCount", ProtectedEnvironment.class);
        privateMethod.setAccessible(true);
        Assert.assertThrows(InvocationTargetException.class,
            () -> privateMethod.invoke(cnwareEnvironmentProviderTest, environment));
    }

    /**
     * 用例场景：检查环境信息
     * 前置条件：环境uuid值为空
     * 检查点：检查CNware环境信息，若uuid为空则生成uuid
     */
    @Test
    public void test_checkEnvironment_should_pass_when_env_uuid_is_blank()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        PowerMockito.when(mockResourceService.query(any())).thenReturn(null);
        Class<CnwareEnvironmentProvider> providerClass = CnwareEnvironmentProvider.class;
        Method privateMethod = providerClass.getDeclaredMethod("checkEnvironment", ProtectedEnvironment.class);
        privateMethod.setAccessible(true);
        privateMethod.invoke(cnwareEnvironmentProviderTest, environment);
        Assert.assertNull(environment.getUuid());
    }

    /**
     * 用例场景：检查环境信息是否重复
     * 前置条件：环境uuid值为空
     * 检查点：检查CNware环境信息是否重复，若uuid为空则抛出异常
     */
    @Test
    public void test_checkEnvironmentRepeat_should_throw_exception_when_env_uuid_is_blank()
        throws NoSuchMethodException{
        ProtectedEnvironment environment = new ProtectedEnvironment();
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(1);
        PowerMockito.when(mockResourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);
        Class<CnwareEnvironmentProvider> providerClass = CnwareEnvironmentProvider.class;
        Method privateMethod = providerClass.getDeclaredMethod("checkEnvironmentRepeat", ProtectedEnvironment.class);
        privateMethod.setAccessible(true);
        Assert.assertThrows(InvocationTargetException.class,
            () -> privateMethod.invoke(cnwareEnvironmentProviderTest, environment));
    }

    /**
     * 用例场景：检查环境ip和port信息
     * 前置条件：环境uuid值为空
     * 检查点：检查CNware环境信息，若port为空则应抛出异常
     */
    @Test
    public void test_checkIpAndPort_should_throw_exception_when_port_is_blank()
        throws NoSuchMethodException{
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("www.cnware.com");
        environment.setPort(65536);
        Class<CnwareEnvironmentProvider> providerClass = CnwareEnvironmentProvider.class;
        Method privateMethod = providerClass.getDeclaredMethod("checkEndpointAndPort", ProtectedEnvironment.class);
        privateMethod.setAccessible(true);
        Assert.assertThrows(InvocationTargetException.class,
            () -> privateMethod.invoke(cnwareEnvironmentProviderTest, environment));
    }

    /**
     * 用例场景：检查check时若获取agent信息失败则抛出异常
     * 前置条件：无
     * 检查点：check是否能捕捉agent信息获取失败
     */
    @Test
    public void test_check_should_failed_when_get_agent_info_failed() {
        ProtectedEnvironment environment = CnwareMockUtil.mockEnvironment();
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(0);
        PowerMockito.when(mockResourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);
        CnwareMockUtil.mockTokenBo(UserTypeEnum.COMMON.getValue());
        PowerMockito.when(mockCnwareCommonService.queryClusterInfo(any(), any()))
            .thenReturn(CnwareMockUtil.mockAppEnvResponse());
        mockDefaultRoleHelper();
        when(cnwareCommonService.getEnvironmentById(any())).thenReturn(getEnvironment());
        Assert.assertThrows(LegoCheckedException.class,
            () -> cnwareEnvironmentProviderTest.register(environment));
    }

    public static void mockDefaultRoleHelper() {
        PowerMockito.mockStatic(DefaultRoleHelper.class);
        DefaultRoleHelper mockToken = PowerMockito.mock(DefaultRoleHelper.class);
        PowerMockito.when(DefaultRoleHelper.isAdmin(anyString())).thenReturn(true);
    }

    /**
     * 用例场景：资源扫描场景
     * 前置条件：agent信息扫描失败
     * 检查点：资源扫描时，若无法获取agent环境信息，则抛出异常
     */
    @Test
    public void test_scan_should_throw_exception_when_scan_by_agent_failed() {
        Assert.assertThrows(LegoCheckedException.class,
            () -> cnwareEnvironmentProviderTest.scan(CnwareMockUtil.mockScanEnvironment()));
    }

    /**
     * 用例场景：资源扫描场景
     * 前置条件：根据uuid查询agent信息失败
     * 检查点：资源扫描时，根据uuid查询agent信息失败，则抛出异常
     */
    @Test
    public void test_scan_should_throw_exception_when_get_agent_resource_by_id_failed() {
        PowerMockito.when(mockEnvRetrievalsService.collectConnectableResources(anyString())).thenReturn(
            CnwareMockUtil.mockCollectConnectableResources());
        Assert.assertThrows(LegoCheckedException.class,
            () -> cnwareEnvironmentProviderTest.scan(CnwareMockUtil.mockScanEnvironment()));
    }

    /**
     * 用例场景：资源扫描场景
     * 前置条件：根据uuid查询agent信息失败
     * 检查点：资源扫描时，成功扫描出CNware资源
     */
    @Test
    public void test_scan_should_throw_exception_when_get_agent_info_failed() {
        PowerMockito.when(mockEnvRetrievalsService.collectConnectableResources(anyString())).thenReturn(
            CnwareMockUtil.mockCollectConnectableResources());
        Optional mock = PowerMockito.mock(Optional.class);
        PowerMockito.when(mock.isPresent()).thenReturn(true);
        PowerMockito.when(mockResourceService.getResourceById(anyString()))
            .thenReturn(Optional.of(CnwareMockUtil.mockScanEnvironment()));
        PowerMockito.when(mockAgentService.getDetailPageList(any(), any(), any(), any()))
            .thenReturn(CnwareMockUtil.mockResponseCnwareHostPool());
        ProtectedEnvironment protectedEnvironment = CnwareMockUtil.mockScanEnvironment();
        List<ProtectedResource>  protectedResourceList = cnwareEnvironmentProviderTest.scan(protectedEnvironment);
        Assert.assertNotNull(protectedResourceList);
    }

    private ProtectedEnvironment getEnvironment() {
        String json = "{\"uuid\":\"872a77ba-3d18-4751-91df-812831c86acc\",\"name\":\"8-40-160-62\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"path\":\"\",\"createdTime\":\"2023-06-02 22:31:19.0\",\"rootUuid\":\"872a77ba-3d18-4751-91df-812831c86acc\",\"sourceType\":\"\",\"version\":\"1.5.RC1.027\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.160.62,8.40.160.62,fe80::d143:5c06:d2cf:2287,fe80::41af:571c:a2e4:f061,fe80::9bf9:6e9c:d1a2:f15e,fe80::d094:f372:f7ba:9d2d,fe80::4655:46db:f055:4a5,fe80::5c03:1bd0:4b26:4a95\",\"agentId\":\"1665020602290819074\",\"$citations_agents_aaf26dfe643f40b2acf54cc74cf9d8b6\":\"34a90a3dd9ec48ef9319479ec62a57f8\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"agentUpgradeable\":\"1\",\"agentUpgradeableVersion\":\"1.5.RC1.029\"},\"endpoint\":\"192.168.160.62\",\"port\":59530,\"linkStatus\":\"1\",\"username\":\"\",\"location\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }
}