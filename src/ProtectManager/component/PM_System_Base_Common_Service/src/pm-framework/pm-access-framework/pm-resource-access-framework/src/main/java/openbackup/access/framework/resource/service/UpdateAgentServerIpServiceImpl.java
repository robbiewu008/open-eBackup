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
package openbackup.access.framework.resource.service;

import com.huawei.oceanprotect.system.base.cert.service.CertPushUpdateService;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.UpdateAgentServerIpService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.host.ManagementIp;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * 内部接口 更新AgentServerIp
 *
 */
@Slf4j
@Service
public class UpdateAgentServerIpServiceImpl implements UpdateAgentServerIpService {
    /**
     * 线程池
     */
    private static final BlockingQueue<Runnable> BLOCKING_QUEUE = new LinkedBlockingQueue<>(IsmNumberConstant.HUNDRED);

    /**
     * 业务线程池
     */
    private static final ThreadPoolExecutor THREAD_POOL_EXECUTOR = new ThreadPoolExecutor(IsmNumberConstant.TWO,
        IsmNumberConstant.SIXTY_FOUR, IsmNumberConstant.SIXTY, TimeUnit.SECONDS, BLOCKING_QUEUE);

    @Autowired
    private CertPushUpdateService certPushUpdateService;

    @Autowired
    private AgentUnifiedService agentUnifiedService;

    @Override
    public void updateAgentServer(ManagementIp managementIp) {
        log.info("update managerServerList: {}", managementIp);
        THREAD_POOL_EXECUTOR.execute(() -> doUpdateAgentServer(managementIp));
    }

    private void doUpdateAgentServer(ManagementIp managementIp) {
        // 所有agent信息， 有管控面的，也有agent的
        List<ProtectedEnvironment> protectedEnvironments = certPushUpdateService.getAllAgent();
        for (ProtectedEnvironment protectedEnvironment : protectedEnvironments) {
            updateAgentIps(protectedEnvironment, managementIp);
        }
    }

    /**
     * 更新agent ip
     *
     * @param protectedResource protectedResource
     * @param managementIp 业务ip
     */
    private void updateAgentIps(ProtectedEnvironment protectedResource, ManagementIp managementIp) {
        try {
            agentUnifiedService.updateAgentServer(protectedResource, managementIp);
        } catch (FeignException | LegoCheckedException e) {
            // 发送失败 ,添加事件
            log.error("Limit agent fail.");
        }
    }
}
