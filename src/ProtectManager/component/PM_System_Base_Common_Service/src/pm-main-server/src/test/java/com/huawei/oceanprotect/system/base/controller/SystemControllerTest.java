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
package com.huawei.oceanprotect.system.base.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.enums.ServiceType;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigStatus;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitServiceType;
import com.huawei.oceanprotect.system.base.initialize.status.InitStandardBackupService;
import com.huawei.oceanprotect.system.base.initialize.status.InitStatusService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponseError;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.storagepool.StoragePool;
import com.huawei.oceanprotect.system.base.service.impl.InitNetworkServiceImpl;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.user.common.utils.CurrentSystemTime;
import openbackup.system.base.util.ProviderRegistry;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;

import java.util.ArrayList;
import java.util.List;

public class SystemControllerTest {
    /**
     * 字符串0
     */
    static final String STRING0 = "0";

    private final SystemController systemController = new SystemController();

    private final InitNetworkConfigMapper initNetworkConfigMapper = Mockito.mock(InitNetworkConfigMapper.class);

    private final InitStatusService initStatusService = Mockito.mock(InitStatusService.class);

    private final InitStandardBackupService initStandardBackupService = Mockito.mock(InitStandardBackupService.class);

    private final ProviderRegistry registry = Mockito.mock(ProviderRegistry.class);

    private final RedissonClient redissonClient = Mockito.mock(RedissonClient.class);

    private final InitConfigService initConfigService = Mockito.mock(InitConfigService.class);

    @Before
    public void init() throws IllegalAccessException {
        MemberModifier.field(SystemController.class, "initNetworkConfigMapper")
            .set(systemController, initNetworkConfigMapper);
        MemberModifier.field(SystemController.class, "initStatusService").set(systemController, initStatusService);
        MemberModifier.field(SystemController.class, "initStandardBackupService")
            .set(systemController, initStandardBackupService);
        MemberModifier.field(SystemController.class, "registry").set(systemController, registry);
        MemberModifier.field(SystemController.class, "redissonClient").set(systemController, redissonClient);
        MemberModifier.field(SystemController.class, "initConfigService").set(systemController, initConfigService);

        given(redissonClient.getMap(InitConfigConstant.INIT_RUNNING_FLAG)).willReturn(Mockito.mock(RMap.class));
    }

    /**
     * 测试 提供后台的查询时间的能力 -001
     */
    @Test
    public void create_system_time_success() {
        CurrentSystemTime systemTime = systemController.getSystemTime();
        System.out.println(systemTime);
        Assert.assertNotNull(systemTime);
    }

    /**
     * 用例场景：查询初始化状态,为需要初始化
     * 前置条件：NA
     * 检查点：返回值为 2
     */
    @Test
    public void get_Init_config_status_is_need_init_success() {
        ConfigStatus initConfig = systemController.getInitConfig();
        Assert.assertEquals(InitConfigConstant.ERROR_CODE_YES, initConfig.getStatus());
    }

    /**
     * 用例场景：查询初始化状态,存在备份录入状态信息; 为无需初始化
     * 前置条件：NA
     * 检查点：返回值为 3
     */
    @Test
    public void get_Init_config_status_exist_backup_network_no_need_init_success() {
        List<InitConfigInfo> backupNetworkFlag = new ArrayList<>();
        backupNetworkFlag.add(new InitConfigInfo());
        given(initNetworkConfigMapper.queryInitConfig(Constants.BACKUP_NETWORK_FLAG)).willReturn(
            backupNetworkFlag);
        given(initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG)).willReturn(
            backupNetworkFlag);
        ConfigStatus initConfig = systemController.getInitConfig();
        Assert.assertEquals(InitConfigConstant.ERROR_CODE_NO, initConfig.getStatus());
    }

    /**
     * 用例场景：查询初始化状态,为无需初始化
     * 前置条件：NA
     * 检查点：返回值为 3
     */
    @Test
    public void get_Init_config_status_no_exist_backup_network_no_need_init_success() {
        List<InitConfigInfo> backupNetworkFlag = new ArrayList<>();
        InitConfigInfo initConfigInfo = new InitConfigInfo();
        initConfigInfo.setInitValue(String.valueOf(InitConfigConstant.ERROR_CODE_NO));
        backupNetworkFlag.add(initConfigInfo);
        given(initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG)).willReturn(
            backupNetworkFlag);
        ConfigStatus initConfig = systemController.getInitConfig();
        Assert.assertEquals(InitConfigConstant.ERROR_CODE_NO, initConfig.getStatus());
    }

    /**
     * 用例场景：查询初始化状态,为不可返回状态
     * 前置条件：NA
     * 检查点：返回值为 6，错误码:INITALIZATION_UNRECOVERABLE_EXCEPTION
     */
    @Test
    public void get_init_config_status_init_network_flag_is_unrecoverable_failed() {
        List<InitConfigInfo> backupNetworkFlag = new ArrayList<>();
        InitConfigInfo initConfigInfo = new InitConfigInfo();
        initConfigInfo.setInitValue(String.valueOf(InitConfigConstant.ERROR_CODE_UNRECOVERABLE_FAILED));
        backupNetworkFlag.add(initConfigInfo);
        given(initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG)).willReturn(
            backupNetworkFlag);
        ConfigStatus initConfig = systemController.getInitConfig();
        Assert.assertEquals(InitConfigConstant.ERROR_CODE_UNRECOVERABLE_FAILED, initConfig.getStatus());
        Assert.assertEquals(String.valueOf(CommonErrorCode.INITALIZATION_UNRECOVERABLE_EXCEPTION),
            initConfig.getCode());
    }

    /**
     * 用例场景：查询初始化状态:为运行中,但是由于redis值参数不存在最终返回;需要初始化
     * 前置条件：NA
     * 检查点：返回值为 2
     */
    @Test
    public void get_init_config_status_init_network_flag_is_running() {
        List<InitConfigInfo> backupNetworkFlag = new ArrayList<>();
        InitConfigInfo initConfigInfo = new InitConfigInfo();
        initConfigInfo.setInitValue(String.valueOf(InitConfigConstant.ERROR_CODE_RUNNING));
        backupNetworkFlag.add(initConfigInfo);
        given(initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG)).willReturn(
            backupNetworkFlag);
        ConfigStatus initConfig = systemController.getInitConfig();
        Assert.assertEquals(InitConfigConstant.ERROR_CODE_YES, initConfig.getStatus());
    }

    /**
     * 用例场景：查询初始化状态
     * 前置条件：NA
     * 检查点：返回值为 0 OK
     */
    @Test
    public void get_init_status_info_success() {
        ConfigStatus configStatus = new ConfigStatus();
        configStatus.setStatus(Constants.ERROR_CODE_OK);
        given(initStatusService.getInitConfigStatus()).willReturn(configStatus);
        ConfigStatus initConfig = systemController.getInitStatusInfo();
        Assert.assertEquals(Constants.ERROR_CODE_OK, initConfig.getStatus());
    }

    /**
     * 用例场景：创建初始化网络成功
     * 前置条件：NA
     * 检查点：返回值为 0 OK
     */
    @Test
    public void create_init_config_success() {
        InitNetworkServiceImpl provider = Mockito.mock(InitNetworkServiceImpl.class);
        given(registry.findProvider(InitNetworkServiceImpl.class, InitServiceType.INIT_NETWORK.getType())).willReturn(
            provider);
        given(provider.init(any())).willReturn(InitConfigConstant.INIT_READY_SUCCESS);
        ConfigStatus initConfig = systemController.createInitConfig(new InitNetworkBody());
        Assert.assertEquals(Constants.ERROR_CODE_OK, initConfig.getStatus());
    }

    /**
     * 用例场景：查询SFTP服务进度成功
     * 前置条件：NA
     * 检查点：返回值为 0 OK
     */
    @Test
    public void init_service_status_success() {
        ConfigStatus configStatus = new ConfigStatus();
        configStatus.setStatus(Constants.ERROR_CODE_OK);
        given(initStandardBackupService.getInitConfigStatus(ServiceType.SFTP)).willReturn(configStatus);
        ConfigStatus initConfig = systemController.initServiceStatus(ServiceType.SFTP);
        Assert.assertEquals(Constants.ERROR_CODE_OK, initConfig.getStatus());
    }

    /**
     * 模拟返回ethPorts
     *
     * @return DeviceManagerResponse
     */
    private DeviceManagerResponse<List<StoragePool>> getDeviceManagerResponseStoragePools() {
        StoragePool storagePool00 = new StoragePool();
        List<StoragePool> storagePools = new ArrayList<>();
        storagePools.add(storagePool00);
        DeviceManagerResponse<List<StoragePool>> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponse.setData(storagePools);
        deviceManagerResponseSetErrorSuccess(deviceManagerResponse);
        return deviceManagerResponse;
    }

    /**
     * 模拟设置成功
     *
     * @param deviceManagerResponse 将其设为成功
     */
    private void deviceManagerResponseSetErrorSuccess(DeviceManagerResponse<?> deviceManagerResponse) {
        DeviceManagerResponseError deviceManagerResponseError = new DeviceManagerResponseError();
        deviceManagerResponseError.setCode(0);
        deviceManagerResponseError.setDescription(STRING0);
        deviceManagerResponse.setError(deviceManagerResponseError);
    }
}
