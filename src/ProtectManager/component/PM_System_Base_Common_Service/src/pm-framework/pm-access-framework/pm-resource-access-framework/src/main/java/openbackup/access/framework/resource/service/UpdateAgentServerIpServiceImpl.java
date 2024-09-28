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

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.UpdateAgentServerIpService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.host.ManagementIp;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

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

    // 内置agent的value
    private static final String INTERNAL_AGENT_VALUE = "1";

    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    public UpdateAgentServerIpServiceImpl(ResourceService resourceService, AgentUnifiedService agentUnifiedService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public void updateAgentServer(ManagementIp managementIp) {
        log.info("update managerServerList: {}", managementIp);
        THREAD_POOL_EXECUTOR.execute(() -> doUpdateAgentServer(managementIp));
    }

    private void doUpdateAgentServer(ManagementIp managementIp) {
        // 所有agent信息， 有管控面的，也有agent的
        List<ProtectedEnvironment> protectedEnvironments = queryAgentList();
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
    @Async
    public void updateAgentIps(ProtectedEnvironment protectedResource, ManagementIp managementIp) {
        try {
            agentUnifiedService.updateAgentServer(protectedResource, managementIp);
        } catch (FeignException | LegoCheckedException e) {
            // 发送失败 ,添加事件
            log.error("Limit agent fail.");
        }
    }

    private List<ProtectedEnvironment> queryAgentList() {
        Map<String, Object> filter = new HashMap<>();
        filter.put("type", ResourceTypeEnum.HOST.getType());
        filter.put("scenario", Arrays.asList(Arrays.asList("!="), INTERNAL_AGENT_VALUE));
        filter.put("isCluster", false);
        filter.put("subType",
            Arrays.asList(ResourceSubTypeEnum.DB_BACKUP_AGENT.getType(), ResourceSubTypeEnum.VM_BACKUP_AGENT.getType(),
                ResourceSubTypeEnum.U_BACKUP_AGENT.getType()));
        PageListResponse<ProtectedResource> totalList = resourceService.query(IsmNumberConstant.ZERO,
            IsmNumberConstant.ONE, filter);
        List<ProtectedEnvironment> protectedResourceList = new ArrayList<>();
        int size = totalList.getTotalCount() / IsmNumberConstant.THOUSAND;
        for (int count = 0; count <= size; count++) {
            protectedResourceList.addAll(saveProtectedResource(filter, count));
        }
        return protectedResourceList;
    }

    private List<ProtectedEnvironment> saveProtectedResource(Map<String, Object> filter, int count) {
        return resourceService.query(count, IsmNumberConstant.THOUSAND, filter)
            .getRecords()
            .stream()
            .filter(existingResource -> existingResource instanceof ProtectedEnvironment)
            .map(existingResource -> (ProtectedEnvironment) existingResource)
            .filter(protectedResource -> LinkStatusEnum.ONLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedResource)))
            .collect(Collectors.toList());
    }
}
