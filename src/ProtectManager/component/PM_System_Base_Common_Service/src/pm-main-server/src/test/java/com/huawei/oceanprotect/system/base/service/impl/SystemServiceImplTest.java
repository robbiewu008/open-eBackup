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
package com.huawei.oceanprotect.system.base.service.impl;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.system.base.bean.DeviceUser;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.storage.StorageError;
import openbackup.system.base.common.model.storage.StorageResponse;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.dto.dorado.ManualInitPortDto;
import com.huawei.oceanprotect.system.base.dto.pacific.NetworkInfoDto;
import com.huawei.oceanprotect.system.base.dto.pacific.NodeNetworkInfoDto;
import com.huawei.oceanprotect.system.base.initialize.network.common.DependentBackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.DependentInitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.common.LldBackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.LldInitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.common.ManualBackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.ManualInitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.common.PacificBackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.PacificInitNetworkBody;
import openbackup.system.base.sdk.cluster.NodeRestApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.api.StorageArraySessionResponse;
import openbackup.system.base.sdk.devicemanager.entity.ZoneDto;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.EdsDnsServiceApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.LoginAuthRestApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.OpenStorageService;
import openbackup.system.base.sdk.devicemanager.request.IpInfo;
import openbackup.system.base.sdk.devicemanager.request.NodeNetworkInfoRequest;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.system.model.StorageAuth;
import openbackup.system.base.service.DeployTypeService;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.service.InitNetworkService;
import com.huawei.oceanprotect.system.base.service.PacificService;
import com.huawei.oceanprotect.system.base.service.impl.dorado.DoradoInitNetworkServiceImpl;
import openbackup.system.base.service.secret.DeviceSecretService;
import openbackup.system.base.util.ProviderRegistry;

import feign.FeignException;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * SystemServiceImplTest
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-06
 */
@RunWith(PowerMockRunner.class)
public class SystemServiceImplTest {
    @InjectMocks
    private SystemServiceImpl systemService;

    @Mock
    private InitConfigService initConfigService;

    @Mock
    private DeviceSecretService deviceSecretService;

    @Mock
    private DeployTypeService deployTypeService;

    @Mock
    private LoginAuthRestApi authRestApi;

    @Mock
    private EdsDnsServiceApi edsDnsServiceApi;

    @Mock
    private InfrastructureRestApi infrastructureRestApi;

    @Mock
    private NodeRestApi nodeRestApi;

    @Mock
    private ProviderRegistry registry;

    @Mock
    private EncryptorService encryptorService;

    @Mock
    private DoradoInitNetworkServiceImpl doradoInitNetworkService;

    @Mock
    private PacificService pacificService;

    @Mock
    private OpenStorageService openStorageService;

    @Before
    public void init() {
        InitNetworkServiceImpl initNetworkService = Mockito.mock(InitNetworkServiceImpl.class);
        Mockito.when(registry.findProvider(any(), any())).thenReturn(initNetworkService);
        Mockito.when(initNetworkService.init(any())).thenReturn("1");
    }

    /**
     * 用例场景：查询所有节点的配置信息 未做筛选
     * 前置条件：无
     * 检查点：查询成功
     */
    @Test
    public void test_getNetworkInfo_when_manageIp_null_then_success() {
        // mock storageAuth
        StorageAuth storageAuth = PowerMockito.mock(StorageAuth.class);
        PowerMockito.when(initConfigService.getLocalStorageAuth()).thenReturn(storageAuth);

        // mock zoneDto
        StorageResponse<List<ZoneDto>> storageResponse = PowerMockito.mock(StorageResponse.class);
        PowerMockito.when(edsDnsServiceApi.queryEdsDnsServiceZones(any(), any(), any())).thenReturn(storageResponse);
        List<ZoneDto> zoneDtoList = new ArrayList<>();
        ZoneDto zoneDto = PowerMockito.mock(ZoneDto.class);
        zoneDtoList.add(zoneDto);
        PowerMockito.when(storageResponse.getData()).thenReturn(zoneDtoList);

        // run
        systemService.getNetworkInfo(null);

        // check
        Assert.assertTrue(true);
    }

    @Test
    public void test_getNetworkInfo_throws_exception() {
        // mock storageAuth
        StorageAuth storageAuth = PowerMockito.mock(StorageAuth.class);
        PowerMockito.when(initConfigService.getLocalStorageAuth()).thenThrow(new LegoCheckedException("message"));

        Assert.assertThrows(LegoCheckedException.class,()->systemService.getNetworkInfo(null));
    }

    /**
     * 用例场景：查询指定节点的配置信息 未做筛选
     * 前置条件：无
     * 检查点：查询成功
     */
    @Test
    public void test_getNodeNetworkInfo_when_no_filter_then_success() {
        // mock NetworkInfo
        NetworkInfoDto networkInfo = PowerMockito.mock(NetworkInfoDto.class);
        List<NodeNetworkInfoDto> nodeNetworkInfoList = new ArrayList<>();
        NodeNetworkInfoDto nodeNetworkInfo = PowerMockito.mock(NodeNetworkInfoDto.class);
        nodeNetworkInfoList.add(nodeNetworkInfo);
        PowerMockito.when(networkInfo.getNodeNetworkInfoList()).thenReturn(nodeNetworkInfoList);
        PowerMockito.when(pacificService.getNetworkInfo(any(), any(), any())).thenReturn(networkInfo);

        // mock storageAuthDto
        StorageAuth storageAuth = PowerMockito.mock(StorageAuth.class);
        PowerMockito.when(initConfigService.getLocalStorageAuth()).thenReturn(storageAuth);

        // mock zoneDto
        StorageResponse<List<ZoneDto>> storageResponse = PowerMockito.mock(StorageResponse.class);
        PowerMockito.when(edsDnsServiceApi.queryEdsDnsServiceZones(any(), any(), any())).thenReturn(storageResponse);
        List<ZoneDto> zoneDtoList = new ArrayList<>();
        ZoneDto zoneDto = PowerMockito.mock(ZoneDto.class);
        zoneDtoList.add(zoneDto);
        PowerMockito.when(storageResponse.getData()).thenReturn(zoneDtoList);

        // run
        String manageIp = "8.40.102.101";
        systemService.getNodeNetworkInfo(manageIp, null, null);

        // check
        Assert.assertTrue(true);
    }

    @Test
    public void test_getNodeNetworkInfo_when_empty_then_success() {
        // mock NetworkInfo
        NetworkInfoDto networkInfo = PowerMockito.mock(NetworkInfoDto.class);
        List<NodeNetworkInfoDto> nodeNetworkInfoList = new ArrayList<>();
        PowerMockito.when(networkInfo.getNodeNetworkInfoList()).thenReturn(nodeNetworkInfoList);
        PowerMockito.when(pacificService.getNetworkInfo(any(), any(), any())).thenReturn(networkInfo);

        // mock storageAuthDto
        StorageAuth storageAuth = PowerMockito.mock(StorageAuth.class);
        PowerMockito.when(initConfigService.getLocalStorageAuth()).thenReturn(storageAuth);

        // mock zoneDto
        StorageResponse<List<ZoneDto>> storageResponse = PowerMockito.mock(StorageResponse.class);
        PowerMockito.when(edsDnsServiceApi.queryEdsDnsServiceZones(any(), any(), any())).thenReturn(storageResponse);
        List<ZoneDto> zoneDtoList = new ArrayList<>();
        ZoneDto zoneDto = PowerMockito.mock(ZoneDto.class);
        zoneDtoList.add(zoneDto);
        PowerMockito.when(storageResponse.getData()).thenReturn(zoneDtoList);

        // run
        String manageIp = "8.40.102.101";
        systemService.getNodeNetworkInfo(manageIp, null, null);

        // check
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：配置存储认证
     * 前置条件：无
     * 检查点：认证成功
     */
    @Test
    public void test_checkAuth_when_find_float_ip_then_fail() {
        // mock storageAuth
        DeviceUser deviceUser = PowerMockito.mock(DeviceUser.class);

        // mock endpoints
        InfraResponseWithError<List<String>> infraResponseWithError = PowerMockito.mock(InfraResponseWithError.class);
        PowerMockito.when(infrastructureRestApi.getEndpoints("pm-system-base"))
            .thenReturn(infraResponseWithError);
        List<String> endpoints = new ArrayList<>();
        endpoints.add("172.16.192.1");
        endpoints.add("172.16.192.2");
        PowerMockito.when(infraResponseWithError.getData()).thenReturn(endpoints);

        // mock floatIp
        String floatIp = "192.168.10.220";
        PowerMockito.when(nodeRestApi.getFloatIp(any())).thenReturn(floatIp);

        // mock deploy type
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.E6000);

        // mock create secret
        PowerMockito.when(deviceSecretService.createSecret(any())).thenReturn(true);

        // mock login
        StorageResponse<StorageArraySessionResponse> login = PowerMockito.mock(StorageResponse.class);
        PowerMockito.when(authRestApi.login(any(), any())).thenReturn(login);
        StorageArraySessionResponse data = PowerMockito.mock(StorageArraySessionResponse.class);
        PowerMockito.when(login.getData()).thenReturn(data);

        // mock initNetworkService
        InitNetworkService initNetworkService = PowerMockito.mock(InitNetworkService.class);
        PowerMockito.when(initNetworkService.getDeviceIp()).thenReturn(floatIp);
        PowerMockito.when(registry.findProviderOrDefault(any(), any(), any())).thenReturn(initNetworkService);

        // check
        Assert.assertThrows(LegoCheckedException.class,() ->
            systemService.configStorageAuth(deviceUser));
    }

    @Test
    public void test_checkAuth_when_create_secret_fail_then_fail() {
        // mock storageAuth
        DeviceUser deviceUser = PowerMockito.mock(DeviceUser.class);

        // mock endpoints
        InfraResponseWithError<List<String>> infraResponseWithError = PowerMockito.mock(InfraResponseWithError.class);
        PowerMockito.when(infrastructureRestApi.getEndpoints("pm-system-base"))
            .thenReturn(infraResponseWithError);
        List<String> endpoints = new ArrayList<>();
        endpoints.add("172.16.192.1");
        endpoints.add("172.16.192.2");
        PowerMockito.when(infraResponseWithError.getData()).thenReturn(endpoints);
        InitNetworkService initNetworkService = PowerMockito.mock(InitNetworkService.class);
        String floatIp = "192.168.10.220";
        PowerMockito.when(initNetworkService.getDeviceIp()).thenReturn(floatIp);
        // mock floatIp
        PowerMockito.when(nodeRestApi.getFloatIp(any())).thenReturn(floatIp);

        // mock deploy type
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.E6000);
        PowerMockito.when(registry.findProviderOrDefault(any(), any(), any())).thenReturn(initNetworkService);

        // mock create secret
        PowerMockito.when(deviceSecretService.createSecret(any())).thenThrow(new LegoCheckedException("message"));
        // check
        Assert.assertThrows(LegoCheckedException.class,() -> systemService.configStorageAuth(deviceUser));
    }

    @Test
    public void test_checkAuth_when_login_fail_then_fail()
        throws InstantiationException, IllegalAccessException, NoSuchMethodException, InvocationTargetException {
        // mock storageAuth
        DeviceUser deviceUser = PowerMockito.mock(DeviceUser.class);

        // mock endpoints
        InfraResponseWithError<List<String>> infraResponseWithError = PowerMockito.mock(InfraResponseWithError.class);
        PowerMockito.when(infrastructureRestApi.getEndpoints("pm-system-base"))
            .thenReturn(infraResponseWithError);
        List<String> endpoints = new ArrayList<>();
        endpoints.add("172.16.192.1");
        endpoints.add("172.16.192.2");
        PowerMockito.when(infraResponseWithError.getData()).thenReturn(endpoints);

        // mock floatIp
        String floatIp = "192.168.10.220";
        PowerMockito.when(nodeRestApi.getFloatIp(any())).thenReturn(floatIp);

        // mock deploy type
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.E6000);

        // mock create secret
        PowerMockito.when(deviceSecretService.createSecret(any())).thenReturn(true);
        InitNetworkService initNetworkService = PowerMockito.mock(InitNetworkService.class);
        PowerMockito.when(initNetworkService.getDeviceIp()).thenReturn(floatIp);
        PowerMockito.when(registry.findProviderOrDefault(any(), any(), any())).thenReturn(initNetworkService);

        // mock login
        Constructor<FeignException> declaredConstructor = FeignException.class.getDeclaredConstructor(int.class,
            String.class);
        declaredConstructor.setAccessible(true);

        PowerMockito.when(authRestApi.login(any(), any())).thenThrow(declaredConstructor.newInstance(0,"message"));

        // check
        Assert.assertThrows(LegoCheckedException.class,() ->
            systemService.configStorageAuth(deviceUser));
    }

    @Test
    public void test_checkAuth_when_handle_error_then_fail() {
        // mock storageAuth
        DeviceUser deviceUser = PowerMockito.mock(DeviceUser.class);

        // mock endpoints
        InfraResponseWithError<List<String>> infraResponseWithError = PowerMockito.mock(InfraResponseWithError.class);
        PowerMockito.when(infrastructureRestApi.getEndpoints("pm-system-base"))
            .thenReturn(infraResponseWithError);
        List<String> endpoints = new ArrayList<>();
        endpoints.add("172.16.192.1");
        endpoints.add("172.16.192.2");
        PowerMockito.when(infraResponseWithError.getData()).thenReturn(endpoints);

        // mock floatIp
        String floatIp = "192.168.10.220";
        PowerMockito.when(nodeRestApi.getFloatIp(any())).thenReturn(floatIp);

        // mock deploy type
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.E6000);

        // mock create secret
        PowerMockito.when(deviceSecretService.createSecret(any())).thenReturn(true);

        // mock login
        StorageResponse<StorageArraySessionResponse> login = PowerMockito.mock(StorageResponse.class);
        PowerMockito.when(authRestApi.login(any(), any())).thenReturn(login);
        StorageArraySessionResponse data = PowerMockito.mock(StorageArraySessionResponse.class);
        PowerMockito.when(login.getData()).thenReturn(data);
        PowerMockito.when(login.getError()).thenReturn(new StorageError(){{
            setCode("1");
            setDescription("desc");
        }});

        // mock initNetworkService
        InitNetworkService initNetworkService = PowerMockito.mock(InitNetworkService.class);
        PowerMockito.when(initNetworkService.getDeviceIp()).thenReturn(floatIp);
        PowerMockito.when(registry.findProviderOrDefault(any(), any(), any())).thenReturn(initNetworkService);

        // check
        Assert.assertThrows(LegoCheckedException.class,() ->
            systemService.configStorageAuth(deviceUser));
    }

    @Test
    public void test_checkAuth_when_handle_device_not_equal_then_fail() {
        // mock storageAuth
        String deviceId = UUID.randomUUID().toString();
        DeviceUser deviceUser = PowerMockito.mock(DeviceUser.class);
        // mock endpoints
        InfraResponseWithError<List<String>> infraResponseWithError = PowerMockito.mock(InfraResponseWithError.class);
        PowerMockito.when(infrastructureRestApi.getEndpoints("pm-system-base"))
            .thenReturn(infraResponseWithError);
        List<String> endpoints = new ArrayList<>();
        endpoints.add("172.16.192.1");
        endpoints.add("172.16.192.2");
        PowerMockito.when(infraResponseWithError.getData()).thenReturn(endpoints);

        // mock floatIp
        String floatIp = "192.168.10.220";
        PowerMockito.when(nodeRestApi.getFloatIp(any())).thenReturn(floatIp);

        // mock deploy type
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.E6000);

        // mock create secret
        PowerMockito.when(deviceSecretService.createSecret(any())).thenReturn(true,false);

        // mock login
        StorageResponse<StorageArraySessionResponse> login = PowerMockito.mock(StorageResponse.class);
        PowerMockito.when(authRestApi.login(any(), any())).thenReturn(login);
        StorageArraySessionResponse data = PowerMockito.mock(StorageArraySessionResponse.class);
        data.setDeviceId(deviceId);
        PowerMockito.when(login.getData()).thenReturn(data);
        PowerMockito.when(login.getError()).thenReturn(new StorageError(){{
            setCode("0");
            setDescription("desc");
        }});

        // mock initNetworkService
        InitNetworkService initNetworkService = PowerMockito.mock(InitNetworkService.class);
        PowerMockito.when(initNetworkService.getDeviceIp()).thenReturn(floatIp);
        PowerMockito.when(registry.findProviderOrDefault(any(), any(), any())).thenReturn(initNetworkService);

        // run
        // check
        Assert.assertThrows(LegoCheckedException.class,() ->
            systemService.configStorageAuth(deviceUser));
    }

    @Test
    public void test_checkAuth_when_handle_permission_then_fail() {
        // mock storageAuth
        String deviceId = UUID.randomUUID().toString();
        DeviceUser deviceUser = PowerMockito.mock(DeviceUser.class);
        // mock endpoints
        InfraResponseWithError<List<String>> infraResponseWithError = PowerMockito.mock(InfraResponseWithError.class);
        PowerMockito.when(infrastructureRestApi.getEndpoints("pm-system-base"))
            .thenReturn(infraResponseWithError);
        List<String> endpoints = new ArrayList<>();
        endpoints.add("172.16.192.1");
        endpoints.add("172.16.192.2");
        PowerMockito.when(infraResponseWithError.getData()).thenReturn(endpoints);

        // mock floatIp
        String floatIp = "192.168.10.220";
        PowerMockito.when(nodeRestApi.getFloatIp(any())).thenReturn(floatIp);

        // mock deploy type
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.E6000);

        // mock create secret
        PowerMockito.when(deviceSecretService.createSecret(any())).thenReturn(true,false);

        // mock login
        StorageResponse<StorageArraySessionResponse> login = PowerMockito.mock(StorageResponse.class);
        PowerMockito.when(authRestApi.login(any(), any())).thenReturn(login);
        StorageArraySessionResponse data = PowerMockito.mock(StorageArraySessionResponse.class);
        data.setDeviceId(deviceId);
        data.setRoleId("0");
        PowerMockito.when(login.getData()).thenReturn(data);
        PowerMockito.when(login.getError()).thenReturn(new StorageError(){{
            setCode("0");
            setDescription("desc");
        }});

        // mock initNetworkService
        InitNetworkService initNetworkService = PowerMockito.mock(InitNetworkService.class);
        PowerMockito.when(initNetworkService.getDeviceIp()).thenReturn(floatIp);
        PowerMockito.when(initNetworkService.getSuperAdminRoleId()).thenReturn(1);
        PowerMockito.when(registry.findProviderOrDefault(any(), any(), any())).thenReturn(initNetworkService);

        // check
        Assert.assertThrows(LegoCheckedException.class,() ->
            systemService.configStorageAuth(deviceUser));
    }

    /**
     * 用例场景：测试手动初始化成功
     * 前置条件：满足条件
     * 检查点：findProvider调用次数
     */
    @Test
    public void test_create_manual_init_config_success() {
        ManualInitNetworkBody manualInitNetworkBody = new ManualInitNetworkBody();
        StorageAuth storageAuth = new StorageAuth();
        storageAuth.setUsername("wuyanzu");
        storageAuth.setPassword("zhendeshuai");
        manualInitNetworkBody.setStorageAuth(storageAuth);

        ManualBackupNetworkConfig backupNetworkConfig = new ManualBackupNetworkConfig();
        List<ManualInitPortDto> logicPorts = new ArrayList<>();
        ManualInitPortDto p = new ManualInitPortDto();
        p.setName("huge");
        logicPorts.add(p);
        backupNetworkConfig.setLogicPorts(logicPorts);
        manualInitNetworkBody.setBackupNetworkConfig(backupNetworkConfig);
        systemService.createManualInitConfig(manualInitNetworkBody);
        Mockito.verify(registry, Mockito.times(1)).findProvider(any(), any());
    }

    /**
     * 用例场景：lld初始化成功
     * 前置条件：满足条件
     * 检查点：findProvider调用次数
     */
    @Test
    public void test_create_lld_init_config_success() {
        LldInitNetworkBody manualInitNetworkBody = new LldInitNetworkBody();
        StorageAuth storageAuth = new StorageAuth();
        storageAuth.setUsername("wuyanzu");
        storageAuth.setPassword("zhendeshuai");
        manualInitNetworkBody.setStorageAuth(storageAuth);

        LldBackupNetworkConfig backupNetworkConfig = new LldBackupNetworkConfig();
        List<LogicPortDto> logicPorts = new ArrayList<>();
        LogicPortDto p = new LogicPortDto();
        p.setName("huge");
        logicPorts.add(p);
        backupNetworkConfig.setLogicPorts(logicPorts);
        manualInitNetworkBody.setBackupNetworkConfig(backupNetworkConfig);
        systemService.createLldInitConfig(manualInitNetworkBody);
        Mockito.verify(registry, Mockito.times(1)).findProvider(any(), any());
    }

    /**
     * 用例场景：分布式一体机初始化成功
     * 前置条件：满足条件
     * 检查点：findProvider调用次数
     */
    @Test
    public void test_create_pacific_init_config_success() {
        PacificInitNetworkBody manualInitNetworkBody = new PacificInitNetworkBody();
        StorageAuth storageAuth = new StorageAuth();
        storageAuth.setUsername("wuyanzu");
        storageAuth.setPassword("zhendeshuai");
        manualInitNetworkBody.setStorageAuth(storageAuth);

        PacificBackupNetworkConfig backupNetworkConfig = new PacificBackupNetworkConfig();
        List<NodeNetworkInfoRequest> logicPorts = new ArrayList<>();
        List<IpInfo> ipPoolDtoList = new ArrayList<>();
        IpInfo ipPoolDto = new IpInfo();
        ipPoolDto.setIfaceName("wuyanzu");
        ipPoolDto.setIpAddress("2.2.2.2");
        ipPoolDtoList.add(ipPoolDto);
        NodeNetworkInfoRequest p = new NodeNetworkInfoRequest();
        p.setManageIp("1.1.1.1");
        p.setIpInfoList(ipPoolDtoList);
        logicPorts.add(p);
        backupNetworkConfig.setPacificInitNetWorkInfoList(logicPorts);
        manualInitNetworkBody.setBackupNetworkConfig(backupNetworkConfig);
        systemService.createPacificInitConfig(manualInitNetworkBody);
        Mockito.verify(registry, Mockito.times(1)).findProvider(any(), any());
    }

    /**
     * 用例场景：软硬解耦初始化成功
     * 前置条件：满足条件
     * 检查点：findProvider调用次数
     */
    @Test
    public void test_create_dependent_init_config_success() {
        DependentInitNetworkBody manualInitNetworkBody = new DependentInitNetworkBody();
        StorageAuth storageAuth = new StorageAuth();
        storageAuth.setUsername("wuyanzu");
        storageAuth.setPassword("zhendeshuai");
        manualInitNetworkBody.setStorageAuth(storageAuth);

        DependentBackupNetworkConfig backupNetworkConfig = new DependentBackupNetworkConfig();
        manualInitNetworkBody.setBackupNetworkConfig(backupNetworkConfig);
        systemService.createDependentInitConfig(manualInitNetworkBody);
        Mockito.verify(registry, Mockito.times(1)).findProvider(any(), any());
    }
}