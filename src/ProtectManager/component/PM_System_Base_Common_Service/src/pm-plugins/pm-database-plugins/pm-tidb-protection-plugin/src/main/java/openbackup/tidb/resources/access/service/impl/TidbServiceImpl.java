/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tidb.resources.access.service.impl;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.enums.LockedValueEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.model.ClusterInfo;
import openbackup.tidb.resources.access.service.TidbService;

import com.fasterxml.jackson.core.type.TypeReference;
import com.google.common.collect.Lists;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * TidbService 实现类
 *
 * @author w00426202
 * @since 2023-07-06
 */
@Slf4j
@Service
public class TidbServiceImpl implements TidbService {
    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * TidbServiceImpl
     *
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     */
    public TidbServiceImpl(ResourceService resourceService, AgentUnifiedService agentUnifiedService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
    }

    /**
     * 更新资源的状态
     *
     * @param resourceId resourceId
     * @param status status
     */
    @Override
    public void updateResourceLinkStatus(String resourceId, String status) {
        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(resourceId);
        updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, status);
        resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
    }

    @Override
    public PageListResponse<ProtectedResource> getBrowseResult(
        BrowseEnvironmentResourceConditions environmentConditions, ProtectedResource endpointResource,
        boolean isCluster) {
        log.info("start to browse tidb dataNode,ip is {},resource type is {}, agentPort is {}",
            endpointResource.getEndpoint(), environmentConditions.getResourceSubType(), endpointResource.getPort());
        ListResourceV2Req listResourceReq = getListResourceReq(environmentConditions, endpointResource, isCluster);
        PageListResponse<ProtectedResource> detailPageList = agentUnifiedService.getDetailPageListNoRetry(
            environmentConditions.getResourceSubType(), endpointResource.getEndpoint(), endpointResource.getPort(),
            listResourceReq, false);
        log.info("Tidb browser list size is {}.", detailPageList.getRecords().size());

        // 表分页查询场景
        Map<String, String> conditionMap = JsonUtil.read(environmentConditions.getConditions(), Map.class);
        log.info("condition-action map is {}", conditionMap.get(TidbConstants.ACTION_TYPE));
        if (StringUtils.equals(conditionMap.get(TidbConstants.ACTION_TYPE), TidbConstants.ACTION_LIST_TABLE)) {
            ProtectedResource record = detailPageList.getRecords().get(0);
            String tableListStr = record.getExtendInfo().get(TidbConstants.TABLE_LIST);
            List<String> tableNameList = JsonUtil.read(tableListStr, List.class);
            ArrayList<ProtectedResource> pageList = new ArrayList<>();
            for (String tableName : tableNameList) {
                ProtectedResource protectedResource = new ProtectedResource();
                protectedResource.setName(tableName);
                pageList.add(protectedResource);
            }
            detailPageList.setRecords(pageList);
            setTableLockedStatus(environmentConditions.getParentId(), detailPageList);
            detailPageList.setTotalCount(Integer.parseInt(record.getExtendInfo().get(TidbConstants.TABLE_NUM)));
        }
        return detailPageList;
    }

    @Override
    public void setTableLockedStatus(String databaseId, PageListResponse<ProtectedResource> tablespaceList) {
        List<ProtectedResource> registeredTablespace = getTableSpaceList(databaseId);
        List<String> lockedTables = getSubTables(registeredTablespace);
        tablespaceList.getRecords().forEach(tablespace -> {
            if (lockedTables.contains(tablespace.getName())) {
                tablespace.setExtendInfoByKey(DatabaseConstants.EXTEND_INFO_KEY_IS_LOCKED,
                    LockedValueEnum.OPTIONAL.getLocked());
            }
        });
    }

    private List<ProtectedResource> getTableSpaceList(String databaseId) {
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.TIDB_TABLE.getType());
        filter.put(DatabaseConstants.PARENT_UUID, databaseId);
        PageListResponse<ProtectedResource> data;
        int pageNo = 0;
        List<ProtectedResource> resources = new ArrayList<>();
        do {
            data = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, filter);
            resources.addAll(data.getRecords());
            pageNo++;
        } while (data.getRecords().size() >= IsmNumberConstant.HUNDRED);
        return resources;
    }

    private List<String> getSubTables(List<ProtectedResource> registeredTablespace) {
        return registeredTablespace.stream()
            .map(tablespace -> JsonUtil.read(tablespace.getExtendInfo().get(TidbConstants.TABLE_NAME),
                new TypeReference<List<String>>() {
                }))
            .flatMap(Collection::stream)
            .collect(Collectors.toList());
    }

    private ListResourceV2Req getListResourceReq(BrowseEnvironmentResourceConditions environmentConditions,
        ProtectedResource endpointResource, boolean isCluster) {
        log.info("conditions is {}", environmentConditions.getConditions());
        Application application = null;
        ListResourceV2Req req = new ListResourceV2Req();
        if (isCluster) {
            application = BeanTools.copy(environmentConditions, Application::new);
            req.setAppEnv(BeanTools.copy(environmentConditions, AppEnv::new));
        } else {
            ProtectedResource resourceById = getResourceByCondition(environmentConditions.getAgentId());
            application = BeanTools.copy(resourceById, Application::new);
            req.setAppEnv(BeanTools.copy(resourceById, AppEnv::new));
        }

        application.setSubType(ResourceSubTypeEnum.TIDB_CLUSTER.getType());
        req.setApplications(Lists.newArrayList(application));

        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put("tiupNode", endpointResource.getEndpoint());
        req.getAppEnv().setExtendInfo(extendInfo);
        req.setConditions(environmentConditions.getConditions());
        req.setPageNo(environmentConditions.getPageNo());
        req.setPageSize(environmentConditions.getPageSize());
        log.info("browser cluster db table, request param is {}", req);
        return req;
    }

    private ListResourceV2Req getListResourceReqForRegister(ProtectedEnvironment environment) {
        ListResourceV2Req req = new ListResourceV2Req();
        req.setAppEnv(BeanTools.copy(environment, AppEnv::new));

        HashMap<Object, Object> condition = new HashMap<>();
        condition.put("action_type", "check_cluster");
        req.setConditions(JsonUtil.json(condition));

        Application application = new Application();
        BeanTools.copy(environment, application);
        List<Application> applications = new ArrayList<>();
        applications.add(application);
        req.setApplications(applications);
        return req;
    }

    /**
     * 检查集群信息
     *
     * @param environment environment
     * @param type type
     * @param endpoint endpoint
     */
    @Override
    public void checkClusterInfo(ProtectedEnvironment environment, String type, Endpoint endpoint) {
        agentUnifiedService.getDetailPageListNoRetry(ResourceSubTypeEnum.TIDB_CLUSTER.getType(), endpoint.getIp(),
            endpoint.getPort(), getListResourceReqForRegister(environment), false);
    }

    /**
     * 根据资源uuid，获取应该存在的资源信息
     *
     * @param uuid 资源uuid
     * @return ProtectedResource 资源信息
     */
    @Override
    public ProtectedResource getResourceByCondition(String uuid) {
        return resourceService.getResourceById(uuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
    }

    @Override
    public PageListResponse<ProtectedResource> getResourceByCondition(Map<String, Object> conditions) {
        return resourceService.query(0, TidbConstants.TIDB_RESOURCE_MAX_COUNT, conditions);
    }

    @Override
    public ProtectedResource getEndpointResource(BrowseEnvironmentResourceConditions environmentConditions) {
        ProtectedResource clusterResource = getResourceByCondition(environmentConditions.getAgentId());
        Map<String, List<ProtectedResource>> dependencies = clusterResource.getDependencies();
        List<ProtectedResource> protectedResources = dependencies.get(DatabaseConstants.AGENTS);
        ProtectedResource agentInfo = protectedResources.get(0);
        ProtectedResource resourceById = getResourceByCondition(agentInfo.getUuid());
        clusterResource.setEndpoint(resourceById.getEndpoint());
        clusterResource.setPort(resourceById.getPort());
        log.info("getEndpointResource is ,ip {}, port is{} ", clusterResource.getEndpoint(), clusterResource.getPort());
        return clusterResource;
    }

    @Override
    public void checkHealth(ProtectedResource clusterResource, ProtectedResource agentResource, String resourceSubType,
        String actionType) {
        AppEnv appEnv = new AppEnv();
        BeanTools.copy(clusterResource, appEnv);
        Application application = new Application();
        BeanTools.copy(clusterResource, application);
        appEnv.getExtendInfo().put(TidbConstants.ACTION_TYPE, actionType);
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(appEnv);
        checkAppReq.setApplication(application);
        AgentBaseDto agentBaseDto;
        try {
            log.info("tidb health check request .resourceSubType is {},  ", resourceSubType);
            agentBaseDto = agentUnifiedService.check(resourceSubType, agentResource.getEndpoint(),
                agentResource.getPort(), checkAppReq);
            log.info("agentBaseDto is {}", JsonUtil.json(agentBaseDto));
            parseRs(agentBaseDto);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("tidb health check failed.", e);
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "tidb health check failed!");
        } catch (Exception e) {
            log.error("check tidb health exception.");
            long errorCode = CommonErrorCode.AGENT_NETWORK_ERROR;
            ActionResult actionResult = new ActionResult();
            if (StringUtils.isNotEmpty(e.getMessage())) {
                actionResult = JsonUtil.read(e.getMessage(), ActionResult.class);
                errorCode = Long.parseLong(actionResult.getBodyErr());
            }
            throw new LegoCheckedException(errorCode, actionResult.getMessage());
        }
    }

    private void parseRs(AgentBaseDto agentBaseDto) {
        if (agentBaseDto == null) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "tidb health check failed, check rs is null!");
        }

        if (!agentBaseDto.isAgentBaseDtoReturnSuccess()) {
            log.info("check health error, errorCode: {}, errorMessage: {}", agentBaseDto.getErrorCode(),
                agentBaseDto.getErrorMessage());
            throw new LegoCheckedException(Long.parseLong(agentBaseDto.getErrorCode()), agentBaseDto.getErrorMessage());
        }
    }

    @Override
    public List<ProtectedResource> getClusterList(String resourceSubType) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.TIDB_CLUSTER.getType());
        List<ProtectedResource> environments = new ArrayList<>();
        int num = TidbConstants.TIDB_RESOURCE_MAX_COUNT / DatabaseConstants.PAGE_SIZE;
        for (int size = 0; size < num; size++) {
            List<ProtectedResource> resources = resourceService.query(size, DatabaseConstants.PAGE_SIZE, conditions)
                .getRecords();
            environments.addAll(resources);
        }
        if (environments.size() >= TidbConstants.TIDB_RESOURCE_MAX_COUNT) {
            log.error("tidb resource register exceed max count: {}.", TidbConstants.TIDB_RESOURCE_MAX_COUNT);
            throw new LegoCheckedException(DatabaseErrorCode.RESOURCE_REACHED_THE_UPPER_LIMIT,
                new String[] {String.valueOf(TidbConstants.TIDB_RESOURCE_MAX_COUNT)},
                "tidb resource register exceed max count.");
        }
        return environments;
    }

    @Override
    public void checkDuplicateResource(ProtectedResource resource, String resourceName) {
        // 修改时跳过检查
        // save_type 0:注册/1:修改
        String saveType = resource.getExtendInfo().get(TidbConstants.SAVE_TYPE);
        if (StringUtils.equalsIgnoreCase(saveType, TidbConstants.SAVE_TYPE_UPDATE)) {
            return;
        }

        // 检查当前集群下的所有资源
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("parentUuid", resource.getParentUuid());
        PageListResponse<ProtectedResource> resourceByCondition = getResourceByCondition(conditions);
        List<ProtectedResource> records = resourceByCondition.getRecords();
        if (CollectionUtils.isEmpty(records)) {
            return;
        }

        for (ProtectedResource protectedResource : records) {
            String savedDatabaseName = protectedResource.getExtendInfo().get(resourceName);
            if (StringUtils.equals(savedDatabaseName, resource.getExtendInfo().get(resourceName))) {
                log.error("tidb resource {} exists ,can not register .", resource.getExtendInfo().get(resourceName));
                throw new LegoCheckedException(CommonErrorCode.RESOURCE_IS_REGISTERED, "The tidb resource exists");
            }
        }
    }

    @Override
    public void checkDuplicateResource(List<String> tablespaceList, String databaseId) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.PARENT_UUID, databaseId);
        int pageNo = 0;
        PageListResponse<ProtectedResource> data;
        do {
            data = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, conditions);
            data.getRecords().forEach(resource -> checkTablespace(resource, tablespaceList));
            pageNo++;
        } while (data.getRecords().size() >= IsmNumberConstant.HUNDRED);
    }

    private void checkTablespace(ProtectedResource resource, List<String> tablespaceList) {
        String registeredTablespace = resource.getExtendInfoByKey(TidbConstants.TABLE_NAME);
        List<String> registeredTablespaceList = JsonUtil.read(registeredTablespace, List.class);
        log.info("register-table-name is {}", JsonUtil.json(registeredTablespaceList));
        if (!Collections.disjoint(tablespaceList, registeredTablespaceList)) {
            log.error("The select tablespace has been registered. uuids: {}", tablespaceList);
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "The tablespace is registered");
        }
    }

    @Override
    public void checkResourceStatus(ProtectedResource protectedResource) {
        ProtectedEnvironment protectedEnvironment = resourceService.getResourceById(protectedResource.getUuid())
            .filter(env -> env instanceof ProtectedEnvironment)
            .map(env -> (ProtectedEnvironment) env)
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Protected resource is not exists!"));

        if (!StringUtils.equalsIgnoreCase(protectedEnvironment.getLinkStatus(),
            LinkStatusEnum.ONLINE.getStatus().toString())) {
            log.error("protectedResource link status is not online. ");
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "protectedResource link status is not online. ");
        }
    }

    @Override
    public void updateResourceLinkStatus(List<ProtectedResource> resourceList, String status) {
        List<ProtectedResource> updateResourceList = new ArrayList<>();
        for (ProtectedResource protectedResource : resourceList) {
            ProtectedResource updateResource = new ProtectedResource();
            updateResource.setUuid(protectedResource.getUuid());
            updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, status);
            updateResourceList.add(updateResource);
        }
        resourceService.updateSourceDirectly(updateResourceList);
    }

    /**
     * 集群资源
     *
     * @param clusterResource clusterResource
     * @return List Endpoint
     */
    @Override
    public List<Endpoint> getTaskEndpoint(ProtectedResource clusterResource) {
        List<Endpoint> supplyAgent = getSupplyAgent(clusterResource.getDependencies());
        String clusterInfoListStr = clusterResource.getExtendInfo().get(TidbConstants.CLUSTER_INFO_LIST);
        List<ClusterInfo> nodeInfos = JsonUtil.read(clusterInfoListStr, new TypeReference<List<ClusterInfo>>() {
        });
        Set<String> uuidSet = nodeInfos.stream()
            .map(ClusterInfo::getHostManagerResourceUuid)
            .collect(Collectors.toSet());
        supplyAgent.forEach(agent -> uuidSet.add(agent.getId()));

        List<Endpoint> endpointList = new ArrayList<>();
        for (String uuid : uuidSet) {
            ProtectedEnvironment protectedEnvironment = resourceService.getResourceById(uuid)
                .filter(env -> env instanceof ProtectedEnvironment)
                .map(env -> (ProtectedEnvironment) env)
                .orElseThrow(
                    () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Protected resource is not exists!"));
            if (LinkStatusEnum.ONLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedEnvironment))) {
                endpointList.add(new Endpoint(protectedEnvironment.getUuid(), protectedEnvironment.getEndpoint(),
                    protectedEnvironment.getPort()));
            }
        }
        return endpointList;
    }

    @Override
    public List<Endpoint> getSupplyAgent(Map<String, List<ProtectedResource>> dependencies) {
        List<ProtectedResource> resourceList = dependencies.get(DatabaseConstants.AGENTS);
        return resourceList.stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(this::applyAgentEndpoint)
            .collect(Collectors.toList());
    }

    private Endpoint applyAgentEndpoint(ProtectedEnvironment agentProtectedEnv) {
        Endpoint endpoint = new Endpoint();
        endpoint.setIp(agentProtectedEnv.getEndpoint());
        endpoint.setPort(agentProtectedEnv.getPort());
        endpoint.setId(agentProtectedEnv.getUuid());
        return endpoint;
    }

    /**
     * 根据集群信息获取agent 信息
     *
     * @param clusterResource clusterResource
     * @return agent信息
     */
    @Override
    public ProtectedResource getAgentResource(ProtectedResource clusterResource) {
        List<ProtectedResource> agentList = clusterResource.getDependencies().get(DatabaseConstants.AGENTS);
        String agentUuid = agentList.get(0).getUuid();
        return getResourceByCondition(agentUuid);
    }
}

