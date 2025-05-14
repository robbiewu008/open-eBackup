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
package com.huawei.oceanprotect.system.base.initialize;

import com.huawei.oceanprotect.base.cluster.remote.dorado.service.ClusterStorageService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import com.huawei.oceanprotect.repository.task.LocalStorageScheduler;
import com.huawei.oceanprotect.system.base.initialize.backstorage.InitializeBackStorage;
import com.huawei.oceanprotect.system.base.initialize.network.InitializeNetPlane;
import com.huawei.oceanprotect.system.base.initialize.network.action.DeviceManagerHandler;
import com.huawei.oceanprotect.system.base.initialize.network.beans.PortFactory;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.status.InitStatusService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.NfsServiceApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.service.InitNetworkService;
import com.huawei.oceanprotect.system.base.service.SystemService;

import lombok.AllArgsConstructor;
import lombok.Data;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.system.base.sdk.agent.UpdateAgentBusinessIps;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.InfrastructureService;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.NetworkService;

import org.redisson.api.RedissonClient;
import org.springframework.context.ApplicationContext;

/**
 * 功能描述
 *
 * @since 2021-03-19
 */
@Data
@AllArgsConstructor
public class InitNetworkConfigParams {
    private InitNetworkBody initNetworkBody;

    private InitNetworkConfigMapper initNetworkConfigMapper;

    private DeviceManagerHandler deviceManagerHandler;

    private InitializeBackStorage initializeBackStorage;

    private LocalStorageService localStorageService;

    private InitStatusService initStatusService;

    private RedissonClient redissonClient;

    private InfrastructureRestApi infrastructureRestApi;

    private LocalStorageScheduler localStorageScheduler;

    private ClusterStorageService clusterStorageService;

    private InitializeNetPlane initializeNetPlane;

    private ApplicationContext applicationContext;

    private InitNetworkService initNetworkService;

    private DeviceManagerService deviceManagerService;

    private InitConfigService initConfigService;

    private SystemService systemService;

    private boolean isFirstInit;

    private DeployTypeService deployTypeService;

    private InfrastructureService infrastructureService;

    private UpdateAgentBusinessIps updateAgentBusinessIps;

    private NetWorkPortService netWorkPortService;

    private NetworkService networkService;

    private PortFactory portFactory;

    private NfsServiceApi nfsServiceApi;

    private ClusterBasicService clusterBasicService;
}
