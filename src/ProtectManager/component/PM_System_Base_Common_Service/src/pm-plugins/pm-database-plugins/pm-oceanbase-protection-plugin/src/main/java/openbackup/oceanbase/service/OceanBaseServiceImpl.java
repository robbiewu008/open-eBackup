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
package openbackup.oceanbase.service;

import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.NfsServiceApi;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.common.dto.OBAgentInfo;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.oceanbase.common.dto.OBTenantInfo;
import openbackup.oceanbase.common.util.OceanBaseUtils;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UserUtils;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 */
@Slf4j
@Service
public class OceanBaseServiceImpl implements OceanBaseService {
    private static final String SUPPORTV41 = "SUPPORTV41";

    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    private final NfsServiceApi nfsServiceApi;

    private final ClusterBasicService clusterBasicService;

    private final JobService jobService;

    @Autowired
    private ClusterQueryService clusterQueryService;

    /**
     * 应用基本的Service有参构造方法
     *
     * @param jobService jobService
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     * @param dmeUnifiedRestApi dmeUnifiedRestApi
     * @param nfsServiceApi NFS相关API
     * @param clusterBasicService clusterBasicService
     */
    public OceanBaseServiceImpl(JobService jobService, ResourceService resourceService,
        AgentUnifiedService agentUnifiedService, DmeUnifiedRestApi dmeUnifiedRestApi, NfsServiceApi nfsServiceApi,
        ClusterBasicService clusterBasicService) {
        this.jobService = jobService;
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.dmeUnifiedRestApi = dmeUnifiedRestApi;
        this.nfsServiceApi = nfsServiceApi;
        this.clusterBasicService = clusterBasicService;
    }

    @Override
    public ProtectedEnvironment getEnvironmentById(String agentUuid) {
        log.info("get environment by id[{}]", agentUuid);
        ProtectedEnvironment environment = resourceService.getResourceById(agentUuid)
            .filter(env -> env instanceof ProtectedEnvironment)
            .map(env -> (ProtectedEnvironment) env)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST,
                "Protected environment is not exists!"));

        log.info("the environment name: {}, ", environment.getName());
        return environment;
    }

    @Override
    public Optional<ProtectedResource> getResourceById(String uuid) {
        return resourceService.getResourceById(uuid);
    }

    @Override
    public OBClusterInfo queryClusterInfo(ProtectedEnvironment environment, String conditions) {
        log.info("begin query cluster info of environment[name={}, uuid={}]", environment.getName(),
            environment.getUuid());
        ProtectedEnvironment obClientAgentEnv = getFirstOnlineOBClientAgent(environment);
        log.info("query from OBClient, client uuid:{}, client name:{}", obClientAgentEnv.getUuid(),
            obClientAgentEnv.getName());

        ListResourceV2Req req = OceanBaseUtils.generateListResourceV2Req(environment, conditions);
        PageListResponse<ProtectedResource> response = agentUnifiedService.getDetailPageListNoRetry(
            environment.getSubType(), obClientAgentEnv.getEndpoint(), obClientAgentEnv.getPort(), req, false);
        List<ProtectedResource> resources = Optional.ofNullable(response)
            .map(PageListResponse::getRecords)
            .orElse(new ArrayList<>());
        if (CollectionUtils.isNotEmpty(resources)) {
            OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(resources.get(0));
            log.info("query cluster info success! env name: {}, subType: {}. cluster name:{} version:{}, tenant:[{}]",
                environment.getName(), environment.getSubType(), clusterInfo.getClusterName(), clusterInfo.getVersion(),
                clusterInfo.getTenantInfos());
            return clusterInfo;
        } else {
            log.error("query cluster info failed!env name: {}, subType: {}.", environment.getName(),
                environment.getSubType());
            throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_NOT_EXIST, "cluster is not exist!");
        }
    }

    /**
     * 获取已存在的OceanBase资源信息
     *
     * @param excludeUuid 排除的uuid
     * @return 已存在的OceanBase资源信息
     */
    @Override
    public List<String> getExistingOceanBaseCluster(String excludeUuid) {
        List<ProtectedResource> existingResources = getProtectedEnvironments(ResourceSubTypeEnum.OCEAN_BASE_CLUSTER,
            null);
        // 转成ProtectedEnvironment环境对象
        List<ProtectedEnvironment> existEnvironments = existingResources.stream()
            .filter(item -> !Objects.equals(item.getUuid(), excludeUuid))
            .filter(existingResource -> existingResource instanceof ProtectedEnvironment)
            .map(existingResource -> (ProtectedEnvironment) existingResource)
            .collect(Collectors.toList());
        log.info("get exist OceanBase cluster count: {}", existEnvironments.size());

        return existEnvironments.stream()
            .map(OceanBaseUtils::readExtendClusterInfo)
            .flatMap(cluster -> cluster.getObServerAgents().stream())
            .map(OBAgentInfo::getParentUuid)
            .collect(Collectors.toList());
    }

    @Override
    public List<String> getExistingOceanBaseTenant(String parentUuid, String excludeUuid) {
        List<ProtectedResource> resources = getProtectedEnvironments(ResourceSubTypeEnum.OCEAN_BASE_TENANT, parentUuid);
        log.info("get exist OceanBase tenant set count: {}", resources.size());
        return resources.stream()
            .filter(item -> !item.getUuid().equals(excludeUuid))
            .map(OceanBaseUtils::readExtendClusterInfo)
            .flatMap(item -> item.getTenantInfos().stream())
            .map(OBTenantInfo::getName)
            .collect(Collectors.toList());
    }

    @Override
    public List<ProtectedResource> getProtectedEnvironments(ResourceSubTypeEnum type, String parentUuid) {
        log.info("get exist env by type: {}", type.getType());
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        filter.put(DatabaseConstants.SUB_TYPE, type.getType());
        if (Objects.equals(ResourceSubTypeEnum.OCEAN_BASE_TENANT, type)) {
            filter.put(DatabaseConstants.PARENT_UUID, parentUuid);
        }
        return resourceService.query(0, OBConstants.OB_CLUSTER_MAX_COUNT, filter).getRecords();
    }

    @Override
    public void updateExtendInfo(List<ProtectedResource> resourceList) {
        // 只更新extendInfo
        List<ProtectedResource> collect = resourceList.stream().map(this::copyExtendInfo).collect(Collectors.toList());
        resourceService.updateSourceDirectly(collect);
    }

    @Override
    public void updateSourceDirectly(ProtectedEnvironment environment) {
        resourceService.updateSourceDirectly(Lists.newArrayList(environment));
    }

    private ProtectedResource copyExtendInfo(ProtectedResource protectedResource) {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(protectedResource.getUuid());
        resource.setExtendInfo(protectedResource.getExtendInfo());
        return resource;
    }

    @Override
    public List<String> checkTenantSetConnect(ProtectedResource resource) {
        log.info("begin check tenant set health. name={}, uuid={}, tenants=[{}]", resource.getUuid(),
            resource.getName(), resource.getExtendInfoByKey(OBConstants.KEY_CLUSTER_INFO));

        ProtectedEnvironment environment = getEnvironmentById(resource.getParentUuid());
        OBClusterInfo oceanBaseInfo = queryClusterInfo(environment);
        List<String> tenantInCluster = Optional.ofNullable(oceanBaseInfo)
            .map(OBClusterInfo::getTenantInfos)
            .orElse(new ArrayList<>())
            .stream()
            .map(OBTenantInfo::getName)
            .collect(Collectors.toList());

        OBClusterInfo tenantInRequest = OceanBaseUtils.readExtendClusterInfo(resource);
        List<OBTenantInfo> tenantNeedCheckList = Optional.ofNullable(tenantInRequest)
            .map(OBClusterInfo::getTenantInfos)
            .orElse(new ArrayList<>());

        List<String> notExistTenants = updateLinkStatus(resource, tenantInCluster, tenantNeedCheckList);
        resource.setExtendInfoByKey(OBConstants.KEY_CLUSTER_INFO, JsonUtil.json(tenantInRequest));

        log.info("end check tenant set health. name={}, uuid={}", resource.getUuid(), resource.getName());
        return notExistTenants;
    }

    /**
     * 获取ObClient agent节点的uuid
     *
     * @param environment 环境
     * @return ObClient agent节点的uuid
     */
    private ProtectedEnvironment getFirstOnlineOBClientAgent(ProtectedEnvironment environment) {
        List<OBAgentInfo> obAgentInfos = Optional.ofNullable(OceanBaseUtils.readExtendClusterInfo(environment))
            .map(OBClusterInfo::getObClientAgents)
            .orElse(new ArrayList<>());

        // 优先查找agent中OBClient服务在线的agent
        for (OBAgentInfo agentInfo : obAgentInfos) {
            if (Objects.equals(agentInfo.getLinkStatus(), LinkStatusEnum.ONLINE.getStatus().toString())) {
                return getEnvironmentById(agentInfo.getParentUuid());
            }
        }

        // 如果没有，则查找agent的host连通性在线的agent
        for (OBAgentInfo agentInfo : obAgentInfos) {
            ProtectedEnvironment agentEnv = getEnvironmentById(agentInfo.getParentUuid());
            if (EnvironmentLinkStatusHelper.isOnlineAdaptMultiCluster(agentEnv)) {
                return agentEnv;
            }
        }

        throw new LegoCheckedException(CommonErrorCode.CLUSTER_LINK_STATUS_ERROR,
            "OBClient node is empty or offline all");
    }

    private static List<String> updateLinkStatus(ProtectedResource resource, List<String> tenantInCluster,
        List<OBTenantInfo> tenantNeedCheckList) {
        List<String> notExistTenants = new ArrayList<>();
        int onlineResult = 0;
        for (OBTenantInfo tenantInfo : tenantNeedCheckList) {
            if (tenantInCluster.contains(tenantInfo.getName())) {
                tenantInfo.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
                onlineResult++;
            } else {
                tenantInfo.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
                notExistTenants.add(tenantInfo.getName());
            }
        }
        if (onlineResult < tenantNeedCheckList.size()) {
            resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                LinkStatusEnum.OFFLINE.getStatus().toString());
        } else {
            resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                LinkStatusEnum.ONLINE.getStatus().toString());
        }
        return notExistTenants;
    }

    @Override
    public void setTenantSetStatue(ProtectedEnvironment env) {
        log.info("start update link status of tenant set under cluster[uuid={}]", env.getUuid());
        // 集群健康检查失败，所有的租户集都设置为离线
        List<ProtectedResource> children = Optional.ofNullable(
            getProtectedEnvironments(ResourceSubTypeEnum.OCEAN_BASE_TENANT, env.getUuid())).orElseGet(ArrayList::new);

        if (CollectionUtils.isNotEmpty(children)) {
            // 更新集群下所有租户集及租户集中租户状态
            children.forEach(resource -> {
                // 更新租户集状态
                resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                    LinkStatusEnum.OFFLINE.getStatus().toString());
                // 租户集中租户状态
                OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(resource);
                clusterInfo.getTenantInfos()
                    .forEach(item -> item.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString()));
                resource.setExtendInfoByKey(OBConstants.KEY_CLUSTER_INFO, JsonUtil.json(clusterInfo));
            });
            updateExtendInfo(children);
        }
        log.info("already update link status of {} tenant sets under cluster[uuid={}]", children.size(), env.getUuid());
    }

    @Override
    public void removeDataRepoWhiteListOfResource(String resourceId) {
        Map<String, Object> paramsMap = new HashMap<>();
        ArrayList<String> actionList = new ArrayList<>();
        actionList.add("removeDataRepoWhiteList");
        Integer count = jobService.getJobCount(Lists.newArrayList(JobTypeEnum.BACKUP.getValue()),
            Lists.newArrayList(JobStatusEnum.SUCCESS.name()), Lists.newArrayList(resourceId));
        if (count > 0) {
            actionList.add("removeRepository");
        }
        paramsMap.put("resourceId", resourceId);
        paramsMap.put("actions", actionList);
        log.info("Removing data repository white list of resource(uuid={}), body: {}.", resourceId, paramsMap);
        dmeUnifiedRestApi.removeRepoWhiteListOfResource(paramsMap);
    }

    @Override
    public void umountDataRepo(OBClusterInfo obClusterInfo, ProtectedResource resource) {
        List<OBAgentInfo> obClientAgents = obClusterInfo.getObClientAgents();
        // 给obClientAgent下发请求
        log.info("umount data repository");
        for (OBAgentInfo obClientAgent : obClientAgents) {
            ProtectedEnvironment agentEnv = getEnvironmentById(obClientAgent.getParentUuid());
            if (agentEnv == null || StringUtils.equals(agentEnv.getLinkStatus(),
                LinkStatusEnum.OFFLINE.getStatus().toString())) {
                log.warn("agent {} not exist or is offline", obClientAgent.getParentUuid());
                continue;
            }
            CheckAppReq reqBody = buildCheckAppReq(obClusterInfo, resource, "1");
            agentUnifiedService.removeProtectUnmountRepoNoRetry(agentEnv.getEndpoint(), agentEnv.getPort(),
                OBConstants.OCEANBASE_CLUSTER, JSONObject.fromObject(reqBody).toString());
        }

        // 给obServerAgent下发请求
        List<OBAgentInfo> obServerAgents = obClusterInfo.getObServerAgents();
        for (OBAgentInfo obServerAgent : obServerAgents) {
            ProtectedEnvironment agentEnv = getEnvironmentById(obServerAgent.getParentUuid());
            if (agentEnv == null || StringUtils.equals(agentEnv.getLinkStatus(),
                LinkStatusEnum.OFFLINE.getStatus().toString())) {
                log.warn("agent {} not exist or is offline", obServerAgent.getParentUuid());
                continue;
            }
            CheckAppReq reqBody = buildCheckAppReq(obClusterInfo, resource, "0");
            agentUnifiedService.removeProtectUnmountRepoNoRetry(agentEnv.getEndpoint(), agentEnv.getPort(),
                OBConstants.OCEANBASE_CLUSTER, JSONObject.fromObject(reqBody).toString());
        }
    }

    @Override
    public void checkSupportNFSV41() {
        checkSupportNFSV41(clusterBasicService.getCurrentClusterEsn(), UserUtils.getBusinessUsername());
    }

    @Override
    public void checkSupportNFSV41Dependent(List<StorageRepository> repositories) {
        if (CollectionUtils.isEmpty(repositories)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "param illegal.");
        }
        String esn = MapUtils.getString(repositories.get(0).getExtendInfo(), StorageRepository.REPOSITORIES_KEY_ENS);
        if (StringUtils.isEmpty(esn)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "param illegal.");
        }
        TargetCluster targetCluster = clusterQueryService.getTargetClusterByEsn(esn);
        checkSupportNFSV41(esn, targetCluster.getUsername());
    }

    private void checkSupportNFSV41(String deviceId, String username) {
        log.info("CheckSupportNFSV4.1, deviceId:{}", deviceId);
        Object response = nfsServiceApi.getNfsServiceConfig(deviceId, username);
        JSONObject responseObject = JSONObject.fromObject(response);
        if (responseObject.isEmpty()) {
            log.error("Check NFSV4.1 service error.");
            throw new LegoCheckedException(CommonErrorCode.NFS_V41_SERVICE_NOT_OPEN, "Check NFSV4.1 service error.");
        } else {
            JSONObject data = responseObject.getJSONObject("data");
            if (data.containsKey(SUPPORTV41)) {
                if (!data.getBoolean(SUPPORTV41)) {
                    log.error("The config of NFSV4.1 service is false.");
                    throw new LegoCheckedException(CommonErrorCode.NFS_V41_SERVICE_NOT_OPEN,
                        "The config of NFSV4.1 service is false.");
                }
            } else {
                log.error("Check NFSV4.1 service error.");
                throw new LegoCheckedException(CommonErrorCode.NFS_V41_SERVICE_NOT_OPEN,
                    "Check NFSV4.1 service error.");
            }
        }
    }

    private CheckAppReq buildCheckAppReq(OBClusterInfo obClusterInfo, ProtectedResource resource,
        String needStopBackup) {
        Application application = new Application();
        application.setSubType(ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType());
        application.setAuth(resource.getAuth());
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(OBConstants.KEY_CLUSTER_INFO, JsonUtil.json(obClusterInfo));
        extendInfo.put("needStopBackup", needStopBackup);
        application.setExtendInfo(extendInfo);

        AppEnv appEnv = BeanTools.copy(resource, AppEnv::new);
        return new CheckAppReq(appEnv, application);
    }
}
