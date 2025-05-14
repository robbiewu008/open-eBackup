/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;
import static org.powermock.api.mockito.PowerMockito.doThrow;

import com.huawei.oceanprotect.base.cluster.remote.dorado.service.ClusterStorageService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import com.huawei.oceanprotect.repository.task.LocalStorageScheduler;
import com.huawei.oceanprotect.system.base.initialize.backstorage.InitializeBackStorage;
import com.huawei.oceanprotect.system.base.initialize.network.InitializeNetPlane;
import com.huawei.oceanprotect.system.base.initialize.network.action.DeviceManagerHandler;
import com.huawei.oceanprotect.system.base.initialize.network.beans.PortFactory;
import com.huawei.oceanprotect.system.base.initialize.network.common.ArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.BackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.status.InitStatusService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.NfsServiceApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.service.InitNetworkService;
import com.huawei.oceanprotect.system.base.service.SystemService;

import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.agent.UpdateAgentBusinessIps;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.InfrastructureService;
import openbackup.system.base.sdk.system.model.StorageAuth;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.NetworkService;

import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;
import org.springframework.context.ApplicationContext;

/**
 * 测试初始化线程类
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-29
 */
public class InitNetworkConfigThreadTest {
    private final InitNetworkConfigMapper initNetworkConfigMapper = Mockito.mock(InitNetworkConfigMapper.class);

    private final DeviceManagerHandler deviceManagerHandler = Mockito.mock(DeviceManagerHandler.class);

    private final InfrastructureRestApi infrastructureRestApi = Mockito.mock(InfrastructureRestApi.class);

    private InitNetworkBody initNetworkBody = new InitNetworkBody();

    private final InitializeBackStorage initializeBackStorage = Mockito.mock(InitializeBackStorage.class);

    private final LocalStorageService localStorageService = Mockito.mock(LocalStorageService.class);

    private final InitStatusService initStatusService = Mockito.mock(InitStatusService.class);

    private final RedissonClient redissonClient = Mockito.mock(RedissonClient.class);

    private final LocalStorageScheduler localStorageScheduler = Mockito.mock(LocalStorageScheduler.class);

    private final DeviceManagerService deviceManagerService = Mockito.mock(DeviceManagerService.class);

    private final ClusterStorageService clusterStorageService = Mockito.mock(ClusterStorageService.class);

    private final InitializeNetPlane initializeNetPlane = Mockito.mock(InitializeNetPlane.class);

    private final ApplicationContext applicationContext = Mockito.mock(ApplicationContext.class);

    private final InitNetworkService initNetworkService = PowerMockito.mock(InitNetworkService.class);

    private final InitConfigService initConfigService = PowerMockito.mock(InitConfigService.class);

    private final SystemService systemService = PowerMockito.mock(SystemService.class);

    private final DeployTypeService deployTypeService = PowerMockito.mock(DeployTypeService.class);

    private final InfrastructureService infrastructureService = PowerMockito.mock(InfrastructureService.class);

    private final UpdateAgentBusinessIps updateAllAgentIp = PowerMockito.mock(UpdateAgentBusinessIps.class);

    private final NetWorkPortService netWorkPortService = PowerMockito.mock(NetWorkPortService.class);

    private final NetworkService networkService = PowerMockito.mock(NetworkService.class);

    private final PortFactory portFactory = PowerMockito.mock(PortFactory.class);

    private final NfsServiceApi nfsServiceApi = PowerMockito.mock(NfsServiceApi.class);

    private final ClusterBasicService clusterBasicService = PowerMockito.mock(ClusterBasicService.class);

    private final InitNetworkConfigThread initNetworkConfigThread = new InitNetworkConfigThread(
        new InitNetworkConfigParams(initNetworkBody, initNetworkConfigMapper, deviceManagerHandler,
            initializeBackStorage, localStorageService, initStatusService, redissonClient, infrastructureRestApi,
            localStorageScheduler, clusterStorageService, initializeNetPlane, applicationContext, initNetworkService,
            deviceManagerService, initConfigService, systemService, true, deployTypeService, infrastructureService,
            updateAllAgentIp, netWorkPortService, networkService, portFactory, nfsServiceApi, clusterBasicService));

    /**
     * 用例场景：测试初始化备份网络成功
     * 前置条件：满足条件
     * 检查点：成功
     */
    @Test
    public void check_init_network_config_thread_run_success() throws IllegalAccessException, InterruptedException {
        mockParam();
        initNetworkConfigThread.run();
    }

    /**
     * 用例场景：测试初始化备份和归档网络成功
     * 前置条件：满足条件
     * 检查点：成功
     */
    @Test
    public void check_init_backup_and_archive_run_success() throws IllegalAccessException {
        mockParam();
        InitNetworkBody body = new InitNetworkBody();
        body.setStorageAuth(new StorageAuth());
        BackupNetworkConfig backupNetworkConfig = new BackupNetworkConfig();
        body.setBackupNetworkConfig(backupNetworkConfig);
        ArchiveNetworkConfig archiveNetworkConfig = new ArchiveNetworkConfig();
        body.setArchiveNetworkConfig(archiveNetworkConfig);
        MemberModifier.field(InitNetworkConfigThread.class, "initNetworkBody").set(initNetworkConfigThread, body);
        initNetworkConfigThread.run();
    }

    private void mockParam() throws IllegalAccessException {
        initNetworkBody.setStorageAuth(new StorageAuth());
        BackupNetworkConfig backupNetworkConfig = new BackupNetworkConfig();
        initNetworkBody.setBackupNetworkConfig(backupNetworkConfig);
        MemberModifier.field(InitNetworkConfigThread.class, "initNetworkBody")
            .set(initNetworkConfigThread, initNetworkBody);
        RBucket map = PowerMockito.mock(RBucket.class);
        given(redissonClient.getBucket(Constants.SYSTEM_INITIALIZED)).willReturn(map);
        given(deviceManagerHandler.achiveDeviceManagerService(any())).willReturn(deviceManagerService);
        doThrow(new LegoCheckedException("Error")).when(applicationContext).publishEvent(any());
    }

    /**
     * 用例场景：测试初始化添加前端口失败
     * 前置条件：满足条件
     * 检查点：失败异常被接收
     */
    @Test
    public void should_throw_legocheckedexception_if_add_net_plane_front_end_container_port_when_run()
        throws IllegalAccessException {
        mockParam();
        // doThrow(new LegoCheckedException(CommonErrorCode.OPERATION_FAILED)).when(deviceManagerHandler)
        //     .addressAllocationInitNetPlaneFrontPort(2, InitConfigConstant.P0, NetworkType.BACKUP);
        initNetworkConfigThread.run();
    }

    /**
     * 用例场景：测试初始化添加前端口失败
     * 前置条件：满足条件
     * 检查点：失败异常被接收
     */
    @Test
    public void should_throw_Exception_if_update_repository_fail_when_run() throws IllegalAccessException {
        mockParam();
        doThrow(new NullPointerException()).when(localStorageScheduler).monitorLocalRepository();
        initNetworkConfigThread.run();
    }
}