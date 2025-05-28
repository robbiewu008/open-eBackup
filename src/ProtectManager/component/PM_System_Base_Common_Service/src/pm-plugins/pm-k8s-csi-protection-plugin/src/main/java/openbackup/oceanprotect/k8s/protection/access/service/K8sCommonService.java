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
package openbackup.oceanprotect.k8s.protection.access.service;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;

import openbackup.access.framework.resource.persistence.dao.ProtectedResourceMapper;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import openbackup.oceanprotect.k8s.protection.access.common.K8sQueryParam;
import openbackup.oceanprotect.k8s.protection.access.constant.K8sConstant;
import openbackup.oceanprotect.k8s.protection.access.util.K8sUtil;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.NetworkServiceApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.request.IpRulePolicyUpdateRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.response.IpRulePolicyUpdateResponse;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.bean.DeviceNetworkInfo;
import openbackup.system.base.bean.NetworkConnectRequest;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.NetworkTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.host.AgentManagementDomain;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.AvailableAgentManagementDomainService;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.NetworkService;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.OpServiceUtil;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.net.URI;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述: K8sCommonService
 *
 */
@Slf4j
@Service
@AllArgsConstructor
public class K8sCommonService {
    private static final String VPC_INFO = "vpc_info";

    private final AgentBusinessService agentBusinessService;
    private final AgentUnifiedService agentUnifiedService;
    private final MemberClusterService memberClusterService;
    private final ResourceService resourceService;
    private final IVpcService iVpcService;
    private final NetworkServiceApi networkServiceApi;
    private final AvailableAgentManagementDomainService domainService;
    private final ProtectedEnvironmentService environmentService;
    private final ProtectedResourceMapper protectedResourceMapper;

    @Autowired
    private NetworkService networkService;

    @Autowired
    private DeployTypeService deployTypeService;

    /**
     * 检查K8S集群连通性
     *
     * @param k8sCluster K8S 集群
     */
    public void checkConnectivity(ProtectedEnvironment k8sCluster) {
        List<ProtectedEnvironment> agents = getAgentEnvironment(k8sCluster);
        AgentBaseDto response = null;
        try {
            addIpRule(k8sCluster);
            for (ProtectedEnvironment internalAgent : agents) {
                response = agentUnifiedService.checkApplication(k8sCluster, internalAgent);
                if (!VerifyUtil.isEmpty(response) && K8sConstant.SUCCESS.equals(response.getErrorCode())) {
                    log.info("K8s cluster check success, uuid: {}, name: {}.",
                        k8sCluster.getUuid(), k8sCluster.getName());
                    k8sCluster.setExtendInfoByKey(K8sUtil.getInternalAgentConnectionKey(internalAgent.getUuid()),
                        String.valueOf(true));
                    return;
                }
            }
        } finally {
            deleteIpRule(k8sCluster);
        }
        log.error("K8s cluster check failed, uuid: {}, name: {}.", k8sCluster.getUuid(), k8sCluster.getName());
        if (VerifyUtil.isEmpty(response) || VerifyUtil.isEmpty(response.getErrorMessage())) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent is empty");
        }
        ActionResult result = JsonUtil.read(response.getErrorMessage(), ActionResult.class);
        throw new LegoCheckedException(Long.parseLong(result.getBodyErr()), result.getMessage());
    }

    /**
     * 过滤本节点的内置agent
     *
     * @return 过滤后本节点的内置agent
     */
    public List<ProtectedEnvironment> filterCurrentNodeInternalAgents() {
        List<ProtectedEnvironment> agents = agentBusinessService.queryInternalAgentEnv();
        if (!memberClusterService.clusterEstablished()) {
            return agents;
        }
        String esn = memberClusterService.getCurrentClusterEsn();
        return agents.stream()
                .filter(agent -> StringUtils.equals(esn, agent.getExtendInfoByKey("internal_agent_esn")))
                .collect(Collectors.toList());
    }

    /**
     * 向agent查询资源
     *
     * @param pageNo pageNo
     * @param pageSize pageSize
     * @param param 查询参数
     * @param k8sCluster k8s集群环境
     * @return 查询的资源
     */
    public PageListResponse<ProtectedResource> queryResource(int pageNo, int pageSize, K8sQueryParam param,
            ProtectedEnvironment k8sCluster) {
        List<ProtectedEnvironment> agents = getConnectiveInternalAgent(k8sCluster, true);
        ProtectedEnvironment internalAgent = agents.get(0);
        ListResourceV2Req listResourceV2Req = new ListResourceV2Req();
        listResourceV2Req.setPageNo(pageNo);
        listResourceV2Req.setPageSize(pageSize);
        listResourceV2Req.setConditions(JsonUtil.json(param));
        listResourceV2Req.setAppEnv(toAppEnv(k8sCluster));
        listResourceV2Req.setApplications(new ArrayList<>());
        listResourceV2Req.getApplications().add(toApplication(k8sCluster));
        try {
            addIpRule(k8sCluster);
            return agentUnifiedService.getDetailPageList(ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.getType(),
                internalAgent.getEndpoint(), internalAgent.getPort(), listResourceV2Req);
        } finally {
            deleteIpRule(k8sCluster);
        }
    }

    /**
     * 查询集群信息
     *
     * @param k8sCluster k8s集群
     * @return 集群信息
     */
    public AppEnvResponse queryClusterInfo(ProtectedEnvironment k8sCluster) {
        List<ProtectedEnvironment> agents = getConnectiveInternalAgent(k8sCluster, true);
        try {
            addIpRule(k8sCluster);
            return agentUnifiedService.getClusterInfo(k8sCluster, agents.get(0));
        } finally {
            deleteIpRule(k8sCluster);
        }
    }

    private AppEnv toAppEnv(ProtectedEnvironment environment) {
        AppEnv appEnv = new AppEnv();
        BeanUtils.copyProperties(environment, appEnv);
        return appEnv;
    }

    private Application toApplication(ProtectedResource resource) {
        Application application = new Application();
        BeanUtils.copyProperties(resource, application);
        return application;
    }

    /**
     * 获取能连通k8s集群的内置agent
     *
     * @param k8sCluster k8s集群
     * @param isStrict 严格模式，为true时未找到连通的agent则报错
     * @return 能连通k8s集群的内置agent
     */
    public List<ProtectedEnvironment> getConnectiveInternalAgent(ProtectedResource k8sCluster, boolean isStrict) {
        return getConnectiveInternalAgentByParams(k8sCluster, isStrict);
    }

    /**
     * 根据env的扩展参数得到能连通k8s的agent
     *
     * @param k8sCluster k8s集群
     * @param isStrict 严格模式，为true时未找到连通的agent则报错
     * @return 能连通k8s集群的内置agent
     */
    public List<ProtectedEnvironment> getConnectiveInternalAgentByParams(ProtectedResource k8sCluster,
        boolean isStrict) {
        Map<String, String> params = k8sCluster.getExtendInfo();
        Map<String, Boolean> agentConnectionMap = new HashMap<>();
        if (params == null) {
            return Collections.emptyList();
        }
        params.forEach((key, value) -> {
            Optional<String> agentId = K8sUtil.getAgentIdFromExtendInfoKey(key);
            if (agentId.isPresent() && !VerifyUtil.isEmpty(agentId.get())) {
                agentConnectionMap.put(agentId.get(), Boolean.valueOf(value));
            }
        });
        ProtectedEnvironment k8sClusterEnv = BeanTools.copy(k8sCluster, ProtectedEnvironment::new);
        List<ProtectedEnvironment> agents = getAgentEnvironment(k8sClusterEnv);
        List<String> agentIds = agents.stream()
                .map(ProtectedEnvironment::getUuid).collect(Collectors.toList());
        log.info("After filter agent, agents are {}, agentConnectionMap is {}",
            String.join(",", agentIds), agentConnectionMap);
        agents.removeIf(next -> !agentConnectionMap.getOrDefault(next.getUuid(), false));

        if (VerifyUtil.isEmpty(agents) && isMultiClusterAgents(agents, k8sCluster)) {
            log.warn("K8s can not get connective agent, k8s cluster id is {}", k8sCluster.getUuid());
            if (isStrict) {
                throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR,
                        "there is no internal agent connective");
            }
        }
        return agents;
    }

    private boolean isMultiClusterAgents(List<ProtectedEnvironment> agents, ProtectedResource k8sCluster) {
        if (!VerifyUtil.isEmpty(agents) || !memberClusterService.clusterEstablished()) {
            return true;
        }
        AgentBaseDto response;
        ProtectedEnvironment k8sClusterEnv = BeanTools.copy(k8sCluster, ProtectedEnvironment::new);
        List<ProtectedEnvironment> currentNodeInternalAgents = getAgentEnvironment(k8sClusterEnv);
        try {
            addIpRule(k8sClusterEnv);
            for (ProtectedEnvironment internalAgent : currentNodeInternalAgents) {
                response = agentUnifiedService.checkApplication(k8sCluster, internalAgent);
                if (!VerifyUtil.isEmpty(response) && K8sConstant.SUCCESS.equals(response.getErrorCode())) {
                    log.info("K8s cluster check success, uuid: {}, name: {}.", k8sCluster.getUuid(),
                        k8sCluster.getName());
                    k8sCluster.setExtendInfoByKey(K8sUtil.getInternalAgentConnectionKey(internalAgent.getUuid()),
                        String.valueOf(true));
                    agents.add(internalAgent);
                    return false;
                }
            }
        } finally {
            deleteIpRule(k8sClusterEnv);
        }
        return true;
    }

    private List<ProtectedEnvironment> getAgentEnvironment(ProtectedEnvironment environment) {
        log.info("Start to get k8s csi agent info of environment: {}", environment.getUuid());
        List<ProtectedEnvironment> agentsEnvList = new ArrayList<>();
        if (VerifyUtil.isEmpty(environment.getDependencies())
            || VerifyUtil.isEmpty(environment.getDependencies().get(K8sConstant.AGENTS))) {
            // 未指定代理，获取环境中在线的K8S CSI类型的代理主机信息，包括内置代理
            List<ProtectedResource> agentList =
                protectedResourceMapper.queryOnlineAgentListByAppLabel(K8sConstant.APP_LABEL_CSI);
            agentsEnvList = agentList.stream()
                .map(o -> BeanTools.copy(o, ProtectedEnvironment::new))
                .collect(Collectors.toList());
        } else {
            List<ProtectedResource> agents = environment.getDependencies().get(K8sConstant.AGENTS);
            for (ProtectedResource agent : agents) {
                try {
                    ProtectedEnvironment agentEnv = environmentService.getEnvironmentById(agent.getUuid());
                    if (VerifyUtil.isEmpty(agentEnv)) {
                        throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "Get agent environment failed");
                    }
                    agentsEnvList.add(agentEnv);
                } catch (LegoCheckedException e) {
                    log.error("Query k8s csi agent environment error, agentId:{}.", agent.getUuid(), e);
                }
            }
        }
        if (!memberClusterService.clusterEstablished()) {
            return agentsEnvList;
        }
        String esn = memberClusterService.getCurrentClusterEsn();
        List<ProtectedEnvironment> agentsEnvFiltered = agentsEnvList.stream()
            .filter(agent -> StringUtils.equals(esn, agent.getExtendInfoByKey(K8sConstant.INTERNAL_AGENT_ESN))
                || StringUtils.equals(
                AgentTypeEnum.INTERNAL_AGENT.getValue(), agent.getExtendInfo().get(K8sConstant.INTERNAL_AGENT_KEY)))
            .collect(Collectors.toList());
        return agentsEnvFiltered;
    }

    /**
     * 适配OP服务化,高级参数传入所有的vpc信息
     *
     * @param advanceParams 任务高级参数
     * @param resourceId 资源uuid
     */
    public void fillVpcInfo(Map<String, String> advanceParams, String resourceId) {
        if (!OpServiceUtil.isHcsService()) {
            return;
        }
        String userId = resourceService.getResourceByIdIgnoreOwner(resourceId)
                .map(ProtectedResource::getUserId)
                .orElse("");
        if (VerifyUtil.isEmpty(userId)) {
            return;
        }
        List<VpcInfoEntity> vpcInfoEntities = iVpcService.getVpcInfoEntityByProjectId(userId);
        advanceParams.put(VPC_INFO, JsonUtil.json(vpcInfoEntities));
    }

    /**
     * 给所有备份ip添加到集群的路由
     *
     * @param environment 环境信息 destinationIp 集群ip port 集群端口
     */

    public void addIpRule(ProtectedEnvironment environment) {
        String destinationIp = environment.getEndpoint();
        int port = environment.getPort();
        if (!isNeedUpdateIpRule(destinationIp, port)) {
            log.info("K8s no need to add ip rule.");
            return;
        }
        List<AgentManagementDomain> agentManagementDomains = new ArrayList<>();
        List<ProtectedEnvironment> agentList = getAgentEnvironment(environment);
        agentList.forEach(agent ->
            agentManagementDomains.addAll(domainService.getAllAvailableManagementInfo(agent.getUuid())));
        IpRulePolicyUpdateRequest request = getIpRulePolicyRequest(destinationIp, "backup", port);
        int size = getUpdateIpRuleSuccessDomainNum("add", request, agentManagementDomains);
        if (size == 0) {
            log.error("Add ip rule failed, destination ip: {}", destinationIp);
            throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT, "Add ip rule failed");
        }
        log.debug("Add ip rule success, destination ip: {}", destinationIp);
    }

    /**
     * 删除所有备份ip到集群的路由
     *
     * @param environment 环境信息 destinationIp 集群ip port 集群端口
     */
    public void deleteIpRule(ProtectedEnvironment environment) {
        String destinationIp = environment.getEndpoint();
        int port = environment.getPort();
        if (!isNeedUpdateIpRule(destinationIp, port)) {
            log.info("K8s no need to delete ip rule.");
            return;
        }
        List<AgentManagementDomain> agentManagementDomains = new ArrayList<>();
        List<ProtectedEnvironment> agentList = getAgentEnvironment(environment);
        agentList.forEach(agent ->
            agentManagementDomains.addAll(domainService.getAllAvailableManagementInfo(agent.getUuid())));
        IpRulePolicyUpdateRequest request = getIpRulePolicyRequest(destinationIp, "backup", 0);
        int size = getUpdateIpRuleSuccessDomainNum("delete", request, agentManagementDomains);
        if (size == 0) {
            log.error("Delete ip rule failed, destination ip: {}", destinationIp);
            throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT, "Delete ip rule failed");
        }
        log.debug("Delete ip rule success, destination ip: {}", destinationIp);
    }

    /**
     * 通过副本查询集群信息
     *
     * @param copy 副本
     * @return 集群信息
     */
    public Optional<ProtectedResource> queryEnvByCopy(Copy copy) {
        Object tmp = JSONObject.fromObject(copy.getResourceProperties()).get("root_uuid");
        String originEnvId = "";
        if (tmp instanceof String) {
            originEnvId = (String) tmp;
        }
        return resourceService.getResourceById(originEnvId);
    }

    private boolean isNeedUpdateIpRule(String destinationIp, int port) {
        NetworkConnectRequest request = new NetworkConnectRequest();
        request.setIp(destinationIp);
        request.setPort(port);
        request.setNetworkType(NetworkTypeEnum.MANAGEMENT);
        return isOnlyXSeries() && !networkService.isNetworkConnectivity(request);
    }

    private boolean isOnlyXSeries() {
        return deployTypeService.isXSeries() && !OpServiceUtil.isHcsService();
    }

    private int getUpdateIpRuleSuccessDomainNum(String action, IpRulePolicyUpdateRequest request,
        List<AgentManagementDomain> agentManagementDomains) {
        int result = agentManagementDomains.size();
        String openStorageUrl = "https://%s:30173/v1/internal";
        for (AgentManagementDomain agentManagementDomain : agentManagementDomains) {
            String openStorageApiUri = String.format(openStorageUrl, agentManagementDomain.getDomain());
            URI uri = URI.create(openStorageApiUri);
            IpRulePolicyUpdateResponse response = networkServiceApi.updateIpRulePolicy(uri, action, request);
            boolean isResponseEmpty = VerifyUtil.isEmpty(response) || VerifyUtil.isEmpty(response.getError());
            if (isResponseEmpty || response.getError().getCode() == 1) {
                log.error("Update ip rule failed, domain: {}, isResponseEmpty: {}",
                    agentManagementDomain.getDomain(), isResponseEmpty);
                result--;
            }
            log.debug("Update ip rule success, domain: {}", agentManagementDomain.getDomain());
        }
        return result;
    }

    private IpRulePolicyUpdateRequest getIpRulePolicyRequest(String destinationIp, String taskType, int port) {
        IpRulePolicyUpdateRequest request = new IpRulePolicyUpdateRequest();
        request.setDestinationIp(destinationIp);
        request.setTaskType(taskType);
        request.setPort(String.valueOf(port));
        return request;
    }

    /**
     * 查出所有的备份网络，将其添加到备份agent的高级参数中
     *
     * @param backUpEndPoints 备份agent参数对象
     */
    public void fillBackUpAgentConnectedIps(List<Endpoint> backUpEndPoints) {
        if (VerifyUtil.isEmpty(backUpEndPoints)) {
            return;
        }
        DeviceNetworkInfo deviceNetworkInfo = networkService.getDeviceNetworkInfo();
        List<String> result = new ArrayList<>(networkService.getNetPlaneIp(deviceNetworkInfo.getBackupConfig()));
        String agentConnectedIps = JsonUtil.json(result);
        for (Endpoint endpoint : backUpEndPoints) {
            endpoint.setAdvanceParamsByKey(BackupConstant.AGENT_CONNECTED_IPS, agentConnectedIps);
        }
    }

    /**
     * 查出所有的备份网络，将其添加到恢复高级参数中
     *
     * @param restoreEndPoints 恢复参数对象
     */
    public void fillReStoreAgentConnectedIps(List<Endpoint> restoreEndPoints) {
        if (VerifyUtil.isEmpty(restoreEndPoints)) {
            return;
        }
        DeviceNetworkInfo deviceNetworkInfo = networkService.getDeviceNetworkInfo();
        List<String> result = new ArrayList<>(networkService.getNetPlaneIp(deviceNetworkInfo.getBackupConfig()));
        String agentConnectedIps = JsonUtil.json(result);
        for (Endpoint endpoint : restoreEndPoints) {
            endpoint.setAdvanceParamsByKey(BackupConstant.AGENT_CONNECTED_IPS, agentConnectedIps);
        }
    }
}