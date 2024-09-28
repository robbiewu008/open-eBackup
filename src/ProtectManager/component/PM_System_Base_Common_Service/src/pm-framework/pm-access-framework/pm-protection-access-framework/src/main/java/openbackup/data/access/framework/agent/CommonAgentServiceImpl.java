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
package openbackup.data.access.framework.agent;

import com.huawei.oceanprotect.base.cluster.sdk.enums.StorageUnitTypeEnum;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import com.huawei.oceanprotect.client.resource.manager.service.dto.AgentConnectedIps;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.model.ProtectedResourceExtendInfo;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.ResExtendConstant;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;

import com.fasterxml.jackson.core.type.TypeReference;

import org.springframework.stereotype.Service;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Deque;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.Stack;
import java.util.stream.Collectors;

/**
 * CommonAgentService实现类
 *
 */
@Service
public class CommonAgentServiceImpl implements CommonAgentService {
    private final Set<String> needDeliverKeys = new HashSet<>(Collections.singletonList("subNetFixedIp"));

    private final ResourceExtendInfoService resourceExtendInfoService;

    private final MemberClusterService memberClusterService;

    private final JobService jobService;

    private final StorageUnitService storageUnitService;

    public CommonAgentServiceImpl(ResourceExtendInfoService resourceExtendInfoService,
        MemberClusterService memberClusterService, JobService jobService, StorageUnitService storageUnitService) {
        this.resourceExtendInfoService = resourceExtendInfoService;
        this.memberClusterService = memberClusterService;
        this.jobService = jobService;
        this.storageUnitService = storageUnitService;
    }

    @Override
    public void supplyAgentCommonInfo(List<Endpoint> endpoints) {
        if (VerifyUtil.isEmpty(endpoints)) {
            return;
        }
        List<String> agentIds = endpoints.stream().map(Endpoint::getId).distinct().collect(Collectors.toList());
        Map<String, List<ProtectedResourceExtendInfo>> extendInfoMap =
            resourceExtendInfoService.queryExtendInfoByResourceIds(agentIds);
        for (Endpoint endpoint : endpoints) {
            List<ProtectedResourceExtendInfo> extendInfoList = extendInfoMap.computeIfAbsent(endpoint.getId(),
                elem -> new ArrayList<>());
            handleAgentExtendInfo(endpoint, extendInfoList);
        }
    }

    @Override
    public AgentMountTypeEnum getJobAgentMountTypeByJob(String jobId) {
        JobBo jobBo = jobService.queryJob(jobId);
        if (jobBo == null || VerifyUtil.isEmpty(jobBo.getStorageUnitId())) {
            return AgentMountTypeEnum.MOUNT;
        }
        return getJobAgentMountTypeByUnitId(jobBo.getStorageUnitId());
    }

    @Override
    public AgentMountTypeEnum getJobAgentMountTypeByUnitId(String unitId) {
        if (VerifyUtil.isEmpty(unitId)) {
            return AgentMountTypeEnum.MOUNT;
        }
        Optional<StorageUnitVo> storageUnit = storageUnitService.getStorageUnitById(unitId);
        if (!storageUnit.isPresent()) {
            return AgentMountTypeEnum.MOUNT;
        }
        if (StorageUnitTypeEnum.BASIC_DISK.getType().equals(storageUnit.get().getDeviceType())) {
            return AgentMountTypeEnum.FUSE;
        }
        return AgentMountTypeEnum.MOUNT;
    }


    private void handleAgentExtendInfo(Endpoint endpoint, List<ProtectedResourceExtendInfo> extendInfoList) {
        for (ProtectedResourceExtendInfo extendInfo : extendInfoList) {
            if (needDeliverKeys.contains(extendInfo.getKey())) {
                endpoint.setAdvanceParamsByKey(extendInfo.getKey(), extendInfo.getValue());
            }
            if (ResExtendConstant.AGENT_CONNECTED_IP.equals(extendInfo.getKey())
                && !endpoint.getAdvanceParams().containsKey(BackupConstant.AGENT_CONNECTED_IPS)) {
                handleAvailableIp(endpoint, extendInfo.getValue());
            }
        }
    }

    private void handleAvailableIp(Endpoint endpoint, String availableIpString) {
        List<AgentConnectedIps> agentConnectedIps = JsonUtil.read(availableIpString,
            new TypeReference<List<AgentConnectedIps>>() {});
        Map<String, Map<String, Stack<String>>> ipMap = AgentConnectedIps.listToMap(agentConnectedIps);
        Map<String, Stack<String>> ipMapInCurCluster = ipMap.get(memberClusterService.getCurrentClusterEsn());
        List<String> sortedIpList = new ArrayList<>();
        if (!VerifyUtil.isEmpty(ipMapInCurCluster)) {
            Deque<Stack<String>> nodeIpDeque = new ArrayDeque<>(ipMapInCurCluster.values());
            while (!nodeIpDeque.isEmpty()) {
                Stack<String> ipStack = nodeIpDeque.pollFirst();
                if (!ipStack.isEmpty()) {
                    sortedIpList.add(ipStack.pop());
                    nodeIpDeque.addLast(ipStack);
                }
            }
        }
        endpoint.setAdvanceParamsByKey(BackupConstant.AGENT_CONNECTED_IPS, JsonUtil.json(sortedIpList));
    }
}
