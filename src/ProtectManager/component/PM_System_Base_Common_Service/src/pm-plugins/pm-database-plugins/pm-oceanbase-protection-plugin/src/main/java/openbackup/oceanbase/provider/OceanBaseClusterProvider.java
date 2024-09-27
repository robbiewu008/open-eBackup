/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oceanbase.provider;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.common.dto.OBAgentInfo;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.oceanbase.common.dto.OBTenantInfo;
import openbackup.oceanbase.common.util.OceanBaseUtils;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import com.huawei.oceanprotect.system.base.user.common.utils.IpCheckUtil;

import com.alibaba.fastjson.JSONObject;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-03
 */
@Slf4j
@Component
public class OceanBaseClusterProvider extends DatabaseEnvironmentProvider {
    private final OceanBaseService oceanBaseService;

    private final DeployTypeService deployTypeService;

    /**
     * DatabaseResourceProvider
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     * @param oceanBaseService oceanBaseService
     * @param deployTypeService deployTypeService
     */
    public OceanBaseClusterProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        OceanBaseService oceanBaseService, DeployTypeService deployTypeService) {
        super(providerManager, pluginConfigManager);
        this.oceanBaseService = oceanBaseService;
        this.deployTypeService = deployTypeService;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType().equals(resourceSubType);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("start to check OceanBase cluster: {}, uuid: {}.", environment.getName(), environment.getUuid());
        checkName(environment);
        if (!deployTypeService.isE1000()) {
            oceanBaseService.checkSupportNFSV41();
        }
        log.info("start check cluster info.");
        OBClusterInfo obClusterInfo = Optional.ofNullable(OceanBaseUtils.readExtendClusterInfo(environment))
            .orElse(new OBClusterInfo());

        Set<String> agentEndpoints = new HashSet<>();
        Set<String> serverAgentUuids = new HashSet<>();
        Set<String> serverAgentIps = new HashSet<>();
        checkObServerNode(environment, obClusterInfo, serverAgentUuids, serverAgentIps, agentEndpoints);

        // 检查OBClient参数
        List<OBAgentInfo> obClientAgents = Optional.ofNullable(obClusterInfo.getObClientAgents())
            .orElse(new ArrayList<>());

        Set<String> clientAgentUuid = checkObClientNode(agentEndpoints, obClientAgents);
        String endpoints = agentEndpoints.stream().sorted().collect(Collectors.joining(OBConstants.COMMA));
        environment.setEndpoint(endpoints);
        environment.setPath(endpoints);
        // 生成Uuid
        if (environment.getUuid() == null) {
            // 生成环境资源唯一UUID,检查uuid是否已经存在
            String uuid = UUIDGenerator.getUUID();
            log.info("start to register new OceanBase environment, uuid: {}", uuid);
            environment.setUuid(uuid);
        }

        //  用户名  为空 拦截报错;
        Authentication auth = environment.getAuth();
        if (auth == null || StringUtils.isAnyBlank(auth.getAuthKey(), auth.getAuthPwd())) {
            log.error("auth is empty.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Auth param is empty");
        }

        // 需要检查连通性
        checkConnectWhenRegister(environment);
        obClusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);
        queryClusterVersion(environment, obClusterInfo);
        environment.setExtendInfoByKey(OBConstants.KEY_CLUSTER_INFO, JsonUtil.json(obClusterInfo));
    }

    private void queryClusterVersion(ProtectedEnvironment environment, OBClusterInfo obClusterInfo) {
        OBClusterInfo oceanBaseInfo = oceanBaseService.queryClusterInfo(environment);
        String version = Optional.ofNullable(oceanBaseInfo)
            .map(OBClusterInfo::getVersion)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_ADD_FAILED,
                "get cluster version failed."));
        environment.setVersion(version);

        obClusterInfo.setClusterId(oceanBaseInfo.getClusterId());
        obClusterInfo.setClusterName(oceanBaseInfo.getClusterName());
        obClusterInfo.setClusterStatus(oceanBaseInfo.getClusterStatus());
    }

    private void checkConnectWhenRegister(ProtectedEnvironment environment) {
        environment.setExtendInfoByKey(OBConstants.KEY_CHECK_SCENE, OBConstants.CLUSTER_REGISTER);
        // 连通性检查， 任一agent检查失败都返回失败。抛出异常
        ResourceCheckContext resourceCheckContext = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            environment).checkConnection(environment);
        initLinkStatus(environment);
        List<CheckReport<Object>> contextCheckReport = OceanBaseUtils.getContextCheckReport(resourceCheckContext);
        OceanBaseUtils.setLinkStatusBaseCheckResult(contextCheckReport, environment);
        OceanBaseUtils.clearExtendInfo(environment);
    }

    private void initLinkStatus(ProtectedEnvironment environment) {
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);
        // 先将所有linkStatus默认设置为ONLINE
        OceanBaseUtils.updateAllLinkStatusOnline(clusterInfo);
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        environment.setExtendInfoByKey(OBConstants.KEY_CLUSTER_INFO, JsonUtil.json(clusterInfo));
    }

    private Set<String> checkObClientNode(Set<String> agentEndpoints, List<OBAgentInfo> obClientAgents) {
        // OBClient节点必填
        if (obClientAgents.isEmpty()) {
            log.error("OBClientAgents is empty.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "OBClient node is empty");
        }
        // OBClient中参数必填
        Set<String> clientAgentUuid = new HashSet<>();
        obClientAgents.forEach(agent -> {
            if (StringUtils.isAnyBlank(agent.getNodeType(), agent.getParentUuid())) {
                log.error("OBClient param is empty.");
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "OBClient param is empty");
            }
            clientAgentUuid.add(agent.getParentUuid());
            agentEndpoints.add(oceanBaseService.getEnvironmentById(agent.getParentUuid()).getEndpoint());
        });

        // OBClient节点不能有重复值
        if (clientAgentUuid.size() != obClientAgents.size()) {
            log.error("OBClientAgents is duplicate.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "obServer agents are duplicate");
        }
        return clientAgentUuid;
    }

    private void checkObServerNode(ProtectedEnvironment environment, OBClusterInfo obClusterInfo,
        Set<String> serverAgentUuids, Set<String> serverAgentIps, Set<String> agentEndpoints) {
        // 检查OBServer参数
        List<OBAgentInfo> obServerAgents = Optional.ofNullable(obClusterInfo.getObServerAgents())
            .orElse(new ArrayList<>());

        // OBServer节点必填
        if (obServerAgents.isEmpty()) {
            log.error("OBServerAgents is empty.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "OBServer node is empty");
        }

        // OBServer中参数必填， 且IP合法
        obServerAgents.forEach(agent -> {
            if (StringUtils.isBlank(agent.getNodeType()) || StringUtils.isBlank(agent.getParentUuid())
                || StringUtils.isBlank(agent.getIp()) || StringUtils.isBlank(agent.getPort())) {
                log.error("OBServer param is empty.");
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "OBServer param is empty");
            }
            IpCheckUtil.getIpTypeWithValidCheck(agent.getIp());
            serverAgentUuids.add(agent.getParentUuid());
            serverAgentIps.add(agent.getIp());
            agentEndpoints.add(oceanBaseService.getEnvironmentById(agent.getParentUuid()).getEndpoint());
        });

        // OBServer节点不能有重复值
        if (serverAgentUuids.size() != obServerAgents.size() || serverAgentIps.size() != obServerAgents.size()) {
            log.error("OBServerAgents is duplicate.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "obServer agents are duplicate");
        }

        // 查询出除自己之外的集群中已注册的OBServer节点
        List<String> existOBServerAgentUuids = oceanBaseService.getExistingOceanBaseCluster(environment.getUuid());

        log.info("exist OceanBase agents: [{}].", existOBServerAgentUuids);

        // OBServer节点主机不能是其他集群已经使用过的
        for (String serverAgentUuid : serverAgentUuids) {
            if (existOBServerAgentUuids.contains(serverAgentUuid)) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                    "The OBServer agent id: " + serverAgentUuid + " already exist.");
            }
        }
    }

    private static void checkName(ProtectedEnvironment environment) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(environment.getName());
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(environment.getName());
        Map<String, String> extendInfo = environment.getExtendInfo();
        if (MapUtils.isEmpty(extendInfo)) {
            log.error("OceanBase cluster extendInfo is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "OceanBase cluster extendInfo is null.");
        }
    }

    @Override
    public PageListResponse<ProtectedResource> browse(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions conditions) {
        JSONObject jsonObject = JsonUtil.getJsonObjectFromStr(conditions.getConditions());
        String type = Optional.ofNullable(jsonObject)
            .map(item -> item.get(OBConstants.QUERY_TYPE_KEY))
            .map(Object::toString)
            .orElse(Strings.EMPTY);
        if (Objects.equals(type, OBConstants.QUERY_TYPE_POOL)) {
            return queryPool(environment, conditions.getConditions());
        } else {
            return queryTenantInCluster(environment);
        }
    }

    private PageListResponse<ProtectedResource> queryPool(ProtectedEnvironment environment, String conditions) {
        log.info("begin query pool from cluster. name: {}, uuid: {}", environment.getName(), environment.getUuid());

        OBClusterInfo clusterInfo = oceanBaseService.queryClusterInfo(environment, conditions);
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfoByKey(OBConstants.KEY_CLUSTER_INFO, JsonUtil.json(clusterInfo));

        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(Lists.newArrayList(resource));
        response.setTotalCount(1);
        log.info("success query pool from cluster[{}].", environment.getName());
        return response;
    }

    private PageListResponse<ProtectedResource> queryTenantInCluster(ProtectedEnvironment environment) {
        log.info("begin query tenants from cluster. name: {}, uuid: {}", environment.getName(), environment.getUuid());

        OBClusterInfo clusterInfo = oceanBaseService.queryClusterInfo(environment);

        List<String> existTenants = oceanBaseService.getExistingOceanBaseTenant(environment.getUuid());
        log.info("exist OceanBase tenant: [{}].", existTenants);
        List<ProtectedResource> resourceList = new ArrayList<>();

        for (OBTenantInfo tenantInfo : clusterInfo.getTenantInfos()) {
            String tenantName = tenantInfo.getName();
            ProtectedResource tenant = new ProtectedResource();
            Map<String, String> extendInfo = new HashMap<>();
            extendInfo.put(OBConstants.EXTEND_KEY_IS_USED, String.valueOf(existTenants.contains(tenantName)));
            tenant.setExtendInfo(extendInfo);
            tenant.setName(tenantName);
            resourceList.add(tenant);
        }

        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(resourceList);
        response.setTotalCount(resourceList.size());
        log.info("success query tenants from cluster[{}], counts [{}].", environment.getName(), resourceList.size());
        return response;
    }

    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        log.info("start check OceanBase cluster health. cluster name: {}, uuid: {}.", environment.getName(),
            environment.getUuid());
        // 通过OBClient检查集群的状态
        ResourceCheckContext context = providerManager.findProvider(ResourceConnectionCheckProvider.class, environment)
            .tryCheckConnection(environment);

        boolean isSuccess = ResourceCheckContextUtil.isSuccess(context.getActionResults());
        log.info("end check OceanBase cluster health. cluster name: {}, uuid: {}, and result is {}.",
            environment.getName(), environment.getUuid(), isSuccess ? "success" : "failed");
        // 检查集群健康
        if (isSuccess) {
            // 集群状态ONLINE，继续检查集群下租户集的健康状态（集群状态OFFLINE， 则所有租户集都是OFFLINE，无需检查）。
            checkTenantSetsOfCluster(environment);
            return Optional.of(LinkStatusEnum.ONLINE.getStatus().toString());
        } else {
            return Optional.of(LinkStatusEnum.OFFLINE.getStatus().toString());
        }
    }

    private void checkTenantSetsOfCluster(ProtectedEnvironment environment) {
        log.info("start check OceanBase tenant set health of cluster. cluster name: {}, uuid: {}.",
            environment.getName(), environment.getUuid());
        // 获取集群下的租户集列表
        List<ProtectedResource> tenantSetOfCluster = oceanBaseService.getProtectedEnvironments(
            ResourceSubTypeEnum.OCEAN_BASE_TENANT, environment.getUuid());

        // 如果集群下没有租户集， 则跳过
        if (CollectionUtils.isNotEmpty(tenantSetOfCluster)) {
            // 检查租户集的健康信息
            tenantSetOfCluster.forEach(oceanBaseService::checkTenantSetConnect);

            // 更新租户集健康信息
            oceanBaseService.updateExtendInfo(tenantSetOfCluster);
        }
        log.info("end check OceanBase tenant set health of cluster. cluster name: {}, uuid: {}.", environment.getName(),
            environment.getUuid());
    }
}
