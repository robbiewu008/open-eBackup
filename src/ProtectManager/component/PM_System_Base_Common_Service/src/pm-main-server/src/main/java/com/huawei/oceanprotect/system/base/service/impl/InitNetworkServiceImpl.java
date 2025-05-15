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

import com.huawei.oceanprotect.base.cluster.remote.dorado.service.ClusterStorageService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.repository.dao.ProductStorageDao;
import com.huawei.oceanprotect.repository.entity.ProductStorage;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import com.huawei.oceanprotect.repository.task.LocalStorageScheduler;
import com.huawei.oceanprotect.system.base.initialize.InitNetworkConfigParams;
import com.huawei.oceanprotect.system.base.initialize.InitNetworkConfigThread;
import com.huawei.oceanprotect.system.base.initialize.backstorage.InitializeBackStorage;
import com.huawei.oceanprotect.system.base.initialize.network.InitializeNetPlane;
import com.huawei.oceanprotect.system.base.initialize.network.action.DeviceManagerHandler;
import com.huawei.oceanprotect.system.base.initialize.network.beans.PortFactory;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigStatus;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitServiceType;
import com.huawei.oceanprotect.system.base.initialize.network.util.SystemTime;
import com.huawei.oceanprotect.system.base.initialize.status.InitStatusService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.NfsServiceApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.service.InitNetworkService;
import com.huawei.oceanprotect.system.base.service.InitService;
import com.huawei.oceanprotect.system.base.service.SystemService;
import com.huawei.oceanprotect.system.base.service.impl.dorado.DoradoInitNetworkServiceImpl;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.enums.AuthTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.agent.UpdateAgentBusinessIps;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.InfrastructureService;
import openbackup.system.base.sdk.system.model.StorageAuth;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.NetworkService;
import openbackup.system.base.task.UpsertClusterESNEvent;
import openbackup.system.base.util.ProviderRegistry;

import org.redisson.api.RLock;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.RedisException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Component;

import java.time.Duration;
import java.util.List;
import java.util.Optional;

/**
 * 功能描述 标准备份服务启动service
 *
 */
@Slf4j
@Component
public class InitNetworkServiceImpl implements InitService<InitNetworkBody> {
    @Autowired
    private InitNetworkConfigMapper initNetworkConfigMapper;

    @Autowired
    private InitializeBackStorage initializeBackStorage;

    @Autowired
    private DeviceManagerHandler deviceManagerHandler;

    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    @Autowired
    private LocalStorageService localStorageService;

    @Autowired
    private InitStatusService initStatusService;

    @Autowired
    private EncryptorService encryptorService;

    @Autowired
    private ClusterBasicService clusterBasicService;

    @Autowired
    private LocalStorageScheduler localStorageScheduler;

    @Autowired
    private ClusterStorageService clusterStorageService;

    @Autowired
    private InitializeNetPlane initializeNetPlane;

    @Autowired
    private ProviderRegistry registry;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private DoradoInitNetworkServiceImpl doradoInitNetworkService;

    @Autowired
    private ProductStorageDao productStorageDao;

    /**
     * Redis客户端
     */
    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private ApplicationContext applicationContext;

    /**
     * 临时变量，和上下文无关
     */
    private DeviceManagerService deviceManagerService;

    @Autowired
    private InitConfigService initConfigService;

    @Autowired
    private SystemService systemService;

    @Autowired
    private JobService jobService;

    @Autowired
    private InfrastructureService infrastructureService;

    @Autowired
    private NetWorkPortService netWorkPortService;

    @Autowired
    private NetworkService networkService;

    @Autowired
    private PortFactory portFactory;

    @Autowired
    private NfsServiceApi nfsServiceApi;

    @Autowired
    private UpdateAgentBusinessIps updateAgentBusinessIps;

    /**
     * 开启初始化网络配置服务 线程
     *
     * @param initNetworkBody 参数
     * @return 执行结果
     */
    @ExterAttack
    public String init(InitNetworkBody initNetworkBody) {
        fillStorageAuth(initNetworkBody);
        xSeriesAuth(initNetworkBody);
        InitNetworkService initNetworkService = registry.findProviderOrDefault(InitNetworkService.class,
            deployTypeService.getDeployType().getValue(), doradoInitNetworkService);
        if (!deployTypeService.isPacific()) {
            initNetworkService.addLogicPort(initNetworkBody);
            initNetworkService.unifiedCheck("", "", initNetworkBody);
        }
        // 获取Redis锁
        RLock redisLock = redissonClient.getLock(InitConfigConstant.INIT_SYSTEM_LOCK_NAME);

        // 默认未锁定
        boolean isLocked = false;
        try {
            ConfigStatus configStatus = initStatusService.queryInitStatus();
            boolean isFirstInit = true;
            if (configStatus.getStatus() == Constants.ERROR_CODE_OK) {
                log.info("network has been initialized");
                if (deployTypeService.isPacific()) {
                    return InitConfigConstant.INIT_READY_SUCCESS;
                }
                isFirstInit = false;
            }

            // 尝试锁定
            isLocked = redisLock.tryLock(InitConfigConstant.INIT_SYSTEM_LOCK_WAIT_TIMES,
                InitConfigConstant.INIT_SYSTEM_LOCK_RELEASE_TIMES, InitConfigConstant.INIT_SYSTEM_LOCK_TIME_UNIT);

            // 如果能够锁定则执行
            if (!isLocked) {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_INITIALIZING_EXCEPTION);
            }
            if (configStatus.getStatus() == InitConfigConstant.ERROR_CODE_RUNNING) {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_INITIALIZING_EXCEPTION);
            }
            if (!deployTypeService.isPacific()) {
                buildTimeOutCheckMap();
            }
            updateInitStatusToRunning(isFirstInit);
            new InitNetworkConfigThread(
                new InitNetworkConfigParams(initNetworkBody, initNetworkConfigMapper, deviceManagerHandler,
                    initializeBackStorage, localStorageService, initStatusService, redissonClient,
                    infrastructureRestApi, localStorageScheduler, clusterStorageService, initializeNetPlane,
                    applicationContext, initNetworkService, deviceManagerService, initConfigService, systemService,
                    isFirstInit, deployTypeService, infrastructureService, updateAgentBusinessIps, netWorkPortService,
                    networkService, portFactory, nfsServiceApi, clusterBasicService)).start();
        } catch (InterruptedException exception) {
            log.error("Lock[{}] failed, cause: {}", InitConfigConstant.INIT_SYSTEM_LOCK_NAME, exception.getMessage());
        } finally {
            releaseInitLock(isLocked, redisLock);
        }
        return InitConfigConstant.INIT_SUCCESS;
    }

    private void releaseInitLock(boolean isLocked, RLock redisLock) {
        // 如果已经锁定则释放
        if (isLocked) {
            try {
                redisLock.unlock();
            } catch (RedisException exception) {
                log.error("Unlock[{}] failed, cause: {}", InitConfigConstant.INIT_SYSTEM_LOCK_NAME,
                    exception.getMessage());
            }
        }
    }

    private void fillStorageAuth(InitNetworkBody initNetworkBody) {
        // 如果不是X系列直接跳过此方法，X系列需要账号密码（前端已不下发）来进行后续的鉴权
        if (!deployTypeService.isXSeries()) {
            return;
        }
        // 如果网络已经初始化.则为修改场景设置账号密码
        ConfigStatus configStatus = initStatusService.queryInitStatus();
        if (configStatus.getStatus() == Constants.ERROR_CODE_OK) {
            String esn = clusterBasicService.getCurrentClusterEsn();
            List<ProductStorage> productStorageList = Optional.ofNullable(
                productStorageDao.findProductStoragesById(esn)).orElseThrow(() -> {
                log.error("System error, operating accounts lost.");
                return new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "System error, operating accounts lost.");
            });
            ProductStorage mangerProductStorage = productStorageList.stream()
                .filter(productStorage -> AuthTypeEnum.MANAGER_AUTH.getEntryAuthType() == productStorage.getAuthType())
                .findFirst().orElseThrow(() -> {
                    log.error("System error, operating accounts lost.");
                    return new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                        "System error, operating accounts lost.");
                });
            StorageAuth storageAuth = new StorageAuth();
            storageAuth.setPassword(encryptorService.decrypt(mangerProductStorage.getPassword()));
            storageAuth.setUsername(mangerProductStorage.getUserName());
            initNetworkBody.setStorageAuth(storageAuth);
        }
    }

    private void xSeriesAuth(InitNetworkBody initNetworkBody) {
        if (deployTypeService.isXSeries()) {
            authentication(initNetworkBody);
        }
    }

    private void buildTimeOutCheckMap() {
        RMap<Object, Object> initTimeOutCheckMap = redissonClient.getMap(InitConfigConstant.INIT_TIMEOUT_CHECK);
        initTimeOutCheckMap.put(InitConfigConstant.INIT_TIMEOUT_CHECK_KEY, InitConfigConstant.INIT_TIMEOUT_CHECK_VALUE);
        initTimeOutCheckMap.expire(Duration.ofMinutes(InitConfigConstant.INIT_TIMEOUT_PERIOD));
    }

    private void updateInitStatusToRunning(boolean isFirstInit) {
        if (!isFirstInit) {
            initNetworkConfigMapper.deleteInitConfigByEsnAndType(Constants.MODIFY_STATUS_FLAG,
                clusterBasicService.getCurrentClusterEsn());
            initNetworkConfigMapper.insertInitConfig(new InitConfigInfo(Constants.MODIFY_STATUS_FLAG,
                String.valueOf(InitConfigConstant.MODIFY_NETWORK_RUNNING), System.currentTimeMillis(),
                clusterBasicService.getCurrentClusterEsn()));
        } else {
            initNetworkConfigMapper.deleteInitConfigByEsnAndType(Constants.INIT_ERROR_FLAG,
                clusterBasicService.getCurrentClusterEsn());
            initNetworkConfigMapper.insertInitConfig(
                new InitConfigInfo(Constants.INIT_ERROR_FLAG, String.valueOf(InitConfigConstant.ERROR_CODE_RUNNING),
                    System.currentTimeMillis(), clusterBasicService.getCurrentClusterEsn()));
        }
    }

    /**
     * detect object applicable
     *
     * @param object RestoreObject
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return InitServiceType.INIT_NETWORK.getType().equals(object);
    }

    private void authentication(InitNetworkBody initNetworkBody) {
        SystemTime systemTime = new SystemTime();
        deviceManagerService = null;
        log.info("Create device manager service start ... Time: {}",
            systemTime.getStartDateTime(SystemTime.PART_TIME).getTime());
        StorageAuth storageAuth = initNetworkBody.getStorageAuth();
        deviceManagerService = deviceManagerHandler.achiveDeviceManagerService(storageAuth);
        String deviceId = deviceManagerService.getDeviceId();
        log.info("authentication success, ESN:{}", deviceId);
        if (org.apache.commons.lang3.StringUtils.isNotBlank(deviceId)) {
            applicationContext.publishEvent(new UpsertClusterESNEvent(applicationContext, deviceId));
        }
        initStatusService.setInitProgressRate(InitConfigConstant.PROGRESS_RATE_05, deviceId);
        log.info("Create device manager service OK!...Time: {}",
            systemTime.getEndDateTime(SystemTime.PART_TIME).getTime());
        log.warn("init time report, Auth User info : {} seconds", systemTime.updateDateTime());
    }
}
