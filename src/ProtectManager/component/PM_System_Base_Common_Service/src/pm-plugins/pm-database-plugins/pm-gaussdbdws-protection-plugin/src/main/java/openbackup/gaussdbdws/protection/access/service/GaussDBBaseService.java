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
package openbackup.gaussdbdws.protection.access.service;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;

import com.alibaba.fastjson.JSON;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.util.DwsBuildRepositoryUtil;
import openbackup.gaussdbdws.protection.access.util.DwsTaskEnvironmentUtil;
import openbackup.gaussdbdws.protection.access.util.DwsValidator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.service.hostagent.AgentQueryService;
import openbackup.system.base.service.hostagent.model.AgentInfo;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.OpServiceUtil;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * DWS 应用基本的Service
 *
 */
@Slf4j
@Service
public class GaussDBBaseService {
    private static final String VPC_INFO = "vpc_info";

    private final ResourceService resourceService;

    private final ProtectedResourceChecker protectedResourceChecker;

    private final ProviderManager providerManager;

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    private final TaskRepositoryManager taskRepositoryManager;

    @Autowired
    private AgentQueryService agentQueryService;

    @Autowired
    private MemberClusterService memberClusterService;

    @Autowired
    private IVpcService iVpcService;

    /**
     * DWS 应用基本的Service有参构造方法
     *
     * @param resourceService 资源服务接口
     * @param protectedResourceChecker protectedResourceChecker
     * @param providerManager DataMover provider registry
     * @param resourceConnectionCheckProvider 资源连通性检查provider
     * @param taskRepositoryManager repositoryManager
     */
    public GaussDBBaseService(ResourceService resourceService,
        @Qualifier("unifiedResourceConnectionChecker") ProtectedResourceChecker protectedResourceChecker,
        ProviderManager providerManager,
        @Qualifier("unifiedConnectionCheckProvider") ResourceConnectionCheckProvider resourceConnectionCheckProvider,
        TaskRepositoryManager taskRepositoryManager) {
        this.resourceService = resourceService;
        this.protectedResourceChecker = protectedResourceChecker;
        this.providerManager = providerManager;
        this.resourceConnectionCheckProvider = resourceConnectionCheckProvider;
        this.taskRepositoryManager = taskRepositoryManager;
    }

    /**
     * 校验集群 是否在线
     *
     * @param uuid 集群的uuid
     */
    public void checkClusterLinkStatus(String uuid) {
        PageListResponse<ProtectedResource> environments = resourceService.query(LegoNumberConstant.ZERO,
            LegoNumberConstant.ONE, Collections.singletonMap(DatabaseConstants.UUID, uuid));
        DwsValidator.checkEnvironment(environments);
    }

    /**
     * 获取集群对象
     *
     * @param envId 集群的uuid
     * @return 查询资源对象
     */
    public ProtectedResource getResourceById(String envId) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceByIdIgnoreOwner(envId);
        if (resOptional.isPresent()) {
            return resOptional.get();
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!");
    }

    /**
     * 获取集群对象
     *
     * @param envId 集群的uuid
     * @return 查询资源对象
     */
    public ProtectedEnvironment getEnvironmentById(String envId) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceByIdIgnoreOwner(envId);
        if (resOptional.isPresent() && resOptional.get() instanceof ProtectedEnvironment) {
            return (ProtectedEnvironment) resOptional.get();
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!");
    }

    /**
     * 检查连通性
     *
     * @param protectedResource protectedResource
     */
    public void checkConnention(ProtectedResource protectedResource) {
        ResourceConnectionCheckProvider provider = providerManager.findProviderOrDefault(
            ResourceConnectionCheckProvider.class, protectedResource, resourceConnectionCheckProvider);
        provider.checkConnection(protectedResource);
    }

    /**
     * 校验 表/schema 集的对象是否相同
     *
     * @param resource 受保护资源
     * @param tableInfo 表列表String
     * @param subType 子资源类型
     */
    public void checkSameTableInfo(ProtectedResource resource, String tableInfo, String subType) {
        PageListResponse<ProtectedResource> records = resourceService.query(LegoNumberConstant.ZERO,
            LegoNumberConstant.ONE,
            ImmutableMap.of(DatabaseConstants.ROOT_UUID, resource.getRootUuid(), "subType", subType));
        int count = records.getTotalCount() / DwsConstant.QUERY_QUANTITY_SPECIFICATIONS;
        for (int size = 0; size <= count; size++) {
            PageListResponse<ProtectedResource> environments = resourceService.query(size,
                DwsConstant.QUERY_QUANTITY_SPECIFICATIONS,
                ImmutableMap.of(DatabaseConstants.ROOT_UUID, resource.getRootUuid(), "subType", subType));
            environments.getRecords()
                .stream()
                .filter(protectedResource -> !protectedResource.getUuid().equals(resource.getUuid()))
                .map(ProtectedResource::getExtendInfo)
                .map(extendInfo -> extendInfo.get(DwsConstant.EXTEND_INFO_KEY_TABLE))
                .forEach(extendInfoValue -> DwsValidator.checkSameTable(Arrays.asList(extendInfoValue.split(",")),
                    tableInfo.split(","), subType));
        }
    }

    /**
     * 填充agent信息
     *
     * @param agentSelectParam agentSelectParam
     * @return  List<Endpoint> Endpoint列表
     */
    public List<Endpoint> supplyAgent(AgentSelectParam agentSelectParam) {
        String rootUuid = agentSelectParam.getResource().getRootUuid();
        Optional<ProtectedResource> resOptional = resourceService.getResourceByIdIgnoreOwner(rootUuid);
        if (!resOptional.isPresent()) {
            log.error("rootUuid: {}, no exist protectedResource", rootUuid);
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "rootUuid: " + rootUuid + ", no exist protectedResource");
        }
        ProtectedResourceChecker checker = providerManager.findProviderOrDefault(ProtectedResourceChecker.class,
            resOptional.get(), this.protectedResourceChecker);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = checker.collectConnectableResources(
            resOptional.get());
        List<Endpoint> endpointList = new ArrayList<>();
        protectedResourceMap.forEach(
            (protectedResource, protectedEnvironments) -> addPointList(endpointList, protectedEnvironments));
        return endpointList;
    }

    private void addPointList(List<Endpoint> endpointList, List<ProtectedEnvironment> protectedEnvironments) {
        for (ProtectedEnvironment protectedEnvironment : protectedEnvironments) {
            Endpoint endpoint = new Endpoint();
            endpoint.setId(protectedEnvironment.getUuid());
            endpoint.setIp(protectedEnvironment.getEndpoint());
            endpoint.setPort(protectedEnvironment.getPort());
            endpointList.add(endpoint);
        }
    }

    /**
     * 增加本地存储的容器阈值判断
     *
     * @param storageRepository 本地存储repository
     * @param storageId 存储库id
     * @param storageType 存储库类型
     */
    public void addRepositoryCapacity(StorageRepository storageRepository, String storageId, String storageType) {
        log.info("storageType:{},storageId:{}", storageType, storageId);
        if (!BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE.equals(storageType) || StringUtils.isEmpty(
            storageId)) {
            log.info("Add repository capacity. type: {}, id: {}.", storageType, storageId);
            return;
        }
        taskRepositoryManager.addRepositoryCapacity(storageRepository, storageId);
    }

    /**
     * 修改task采纳数信息
     *
     * @param backupTask backupTask
     */
    public void modifyBackupTaskParam(BackupTask backupTask) {
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        // 添加存储集群的X8000型号; esn role角色;
        List<StorageRepository> repositories = backupTask.getRepositories();
        for (StorageRepository repository : repositories) {
            repository.setRole(DwsConstant.SLAVE_ROLE);
        }
        // 将本机对应的节点移到最前面，并且把第一个设为主节点
        moveCurrentEsnRepositoryToIndex0(repositories);
        repositories.add(DwsBuildRepositoryUtil.getCacheRepository(repositories.get(0)));
        Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams()).orElse(new HashMap<>());
        // 默认支持多文件系统
        // 1.元数据存放路径（"metadataPath":"/opt/meta/"）、备份方式("backupToolType":0-Roach,1-GDS)放在advanceParams里;
        advanceParams.put(DwsConstant.REPOSITORIES_KEY_MULTI_FILE_SYSTEM,
            DatabaseConstants.MULTI_FILE_SYSTEM_VALUE_ENABLE);
        advanceParams.put(DatabaseConstants.FORBID_WORM_FILE_SYSTEM, Boolean.TRUE.toString());
        DwsBuildRepositoryUtil.modifyAdvanceParam(advanceParams, DwsConstant.ADVANCE_PARAMS_KEY_BACKUP_METADATA_PATH,
            DwsConstant.ADVANCE_PARAMS_KEY_METADATA_PATH);
        DwsBuildRepositoryUtil.modifyAdvanceParam(advanceParams, DwsConstant.ADVANCE_PARAMS_KEY_BACKUP_TOOL_TYPE,
            DwsConstant.ADVANCE_PARAMS_KEY_TOOL_TYPE);
        DwsBuildRepositoryUtil.addSpeedStatisticsAdvanceParam(advanceParams, DwsConstant.ADVANCE_PARAMS_KEY_TOOL_TYPE,
            TaskUtil.SPEED_STATISTICS);
        addRepositoryCapacity(repositories.get(0), advanceParams.get(DwsConstant.ADVANCE_PARAMS_KEY_STORAGE_ID),
            advanceParams.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY));
        // storage_unit场景下storage_id为空
        if (BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_VALUE.equals(
            advanceParams.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY))) {
            log.info("storageId set form {} to null.", DwsConstant.ADVANCE_PARAMS_KEY_STORAGE_ID);
            advanceParams.put(DwsConstant.ADVANCE_PARAMS_KEY_STORAGE_ID, "");
        }
        ProtectedEnvironment environment = getEnvironmentById(backupTask.getProtectObject().getRootUuid());
        TaskEnvironment protectEnv = backupTask.getProtectEnv();
        // 2.主机类型("nodeType":0-集群节点,1-代理主机)放在protectEnv->nodes->extendInfo里;
        DwsTaskEnvironmentUtil.initNodeType(protectEnv, environment);
        // 3.用户名("DwsUser":"omm")、环境变量路径("envFile":"/home/env")放在protectEnv->extendInfo里;
        DwsTaskEnvironmentUtil.initProtectEnvOfDwsUser(protectEnv, environment.getAuth().getAuthKey());
        DwsTaskEnvironmentUtil.initProtectEnvOfEnvFile(protectEnv,
            environment.getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_ENV_FILE));
        protectEnv.getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
    }

    private void moveCurrentEsnRepositoryToIndex0(List<StorageRepository> repositories) {
        String currentClusterEsn = memberClusterService.getCurrentClusterEsn();
        repositories.sort(((o1, o2) -> {
            if (currentClusterEsn.equals(o1.getId())) {
                return -1;
            } else if (currentClusterEsn.equals(o2.getId())) {
                return 1;
            } else {
                return 0;
            }
        }));
        repositories.get(0).setRole(DwsConstant.MASTER_ROLE);
    }

    /**
     * 返回节点信息列表
     *
     * @param uuid 集群uuid
     * @return 节点信息列表
     */
    public List<TaskEnvironment> supplyNodes(String uuid) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(uuid);
        if (!resOptional.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
        List<ProtectedEnvironment> dwsClusterAgent = getDwsClusterAgent(resOptional.get());
        List<ProtectedEnvironment> hostAgent = getHostClusterAgent(resOptional.get());

        checkAgent(dwsClusterAgent, hostAgent, null);

        List<TaskEnvironment> nodes = new ArrayList<>();
        nodes.addAll(dwsClusterAgent.stream()
            .filter(protectedEnvironment -> LinkStatusEnum.ONLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedEnvironment)))
            .map(environment -> BeanTools.copy(environment, TaskEnvironment::new))
            .collect(Collectors.toList()));

        nodes.addAll(hostAgent.stream()
            .filter(protectedEnvironment -> LinkStatusEnum.ONLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedEnvironment)))
            .map(environment -> BeanTools.copy(environment, TaskEnvironment::new))
            .collect(Collectors.toList()));
        return nodes;
    }

    private List<ProtectedEnvironment> getDwsClusterAgent(ProtectedResource resource) {
        return Optional.ofNullable(
                resource.getDependencies().get(DwsConstant.DWS_CLUSTER_AGENT))
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Dws cluster not exists."))
                .stream()
                .map(ProtectedResource::getUuid)
                .map(this::getEnvironmentById)
                .collect(Collectors.toList());
    }

    private List<ProtectedEnvironment> getHostClusterAgent(ProtectedResource resource) {
        return Optional.ofNullable(
                resource.getDependencies().get(DwsConstant.HOST_AGENT))
                .orElse(new ArrayList<>())
                .stream()
                .map(ProtectedResource::getUuid)
                .map(this::getEnvironmentById)
                .collect(Collectors.toList());
    }

    private void checkAgent(List<ProtectedEnvironment> dwsClusterAgent,
                            List<ProtectedEnvironment> hostAgent,
                            String targetEsn) {
        // 1. 如果集群没有选择代理主机，你就判断是否所有集群节点在线，如果有一个不在线，就不下发
        if (hostAgent.size() == 0 && dwsClusterAgent.stream()
            .anyMatch(protectedEnvironment -> LinkStatusEnum.OFFLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedEnvironment, targetEsn)))) {
            log.error("Not agent and dws cluster exists offline.");
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }

        // 2. 如果集群有选择代理主机，当所有代理主机都不在线，或者所有集群节点都不在线时，就不下发
        if (dwsClusterAgent.stream()
            .allMatch(protectedEnvironment -> LinkStatusEnum.OFFLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedEnvironment, targetEsn)))) {
            log.error("All dws cluster offline.");
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }

        if (!hostAgent.isEmpty() && hostAgent.stream()
            .allMatch(protectedEnvironment -> LinkStatusEnum.OFFLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedEnvironment, targetEsn)))) {
            log.error("All agent offline.");
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    /**
     * 多集群任务分发，过滤掉不连通的esn
     *
     * @param resourceId 资源id
     * @param availableEsnList 可用节点
     * @return 连通节点
     */
    public Set<String> availableEsnFilter(String resourceId, Set<String> availableEsnList) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(resourceId);
        if (!resOptional.isPresent()) {
            log.error("ProtectedResource {} is not present. Agent check network error.", resourceId);
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
        List<ProtectedEnvironment> dwsClusterAgentList;
        try {
            dwsClusterAgentList = getDwsClusterAgent(resOptional.get());
        } catch (LegoCheckedException e) {
            log.error("Dws cluster not exists.", ExceptionUtil.getErrorMessage(e));
            return new HashSet<>();
        }
        List<ProtectedEnvironment> hostAgentList = getHostClusterAgent(resOptional.get());
        return availableEsnList.stream()
                .filter(targetEsn -> checkAgentByEsn(dwsClusterAgentList, hostAgentList, targetEsn))
                .collect(Collectors.toSet());
    }

    private boolean checkAgentByEsn(List<ProtectedEnvironment> dwsClusterAgent,
                                    List<ProtectedEnvironment> hostAgent,
                                    String targetEsn) {
        try {
            checkAgent(dwsClusterAgent, hostAgent, targetEsn);
        } catch (LegoCheckedException e) {
            log.error("TargetEsn {} and agent link status check failed.", targetEsn, ExceptionUtil.getErrorMessage(e));
            return false;
        }
        return true;
    }

    /**
     * 校验应用集群是否在线;
     *
     * @param uuid 集群uuid
     * @param resourceSubType 待恢复的子类型
     */
    public void checkLinkStatus(String uuid, String resourceSubType) {
        if (ResourceSubTypeEnum.GAUSSDB_DWS.getType().equals(resourceSubType)) {
            return;
        }
        ProtectedEnvironment clusterEnvironment = getEnvironmentById(uuid);
        if (LinkStatusEnum.ONLINE.getStatus().toString()
            .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(clusterEnvironment))) {
            return;
        }
        log.error("Dws Cluster {} Not Online", clusterEnvironment.getName());
        throw new LegoCheckedException(CommonErrorCode.CLUSTER_LINK_STATUS_ERROR,
            "dws cluster " + clusterEnvironment.getName() + " not online");
    }

    /**
     * 校验应用集群是否在线;
     *
     * @param uuid 集群uuid
     */
    public void checkLinkStatus(String uuid) {
        ProtectedEnvironment clusterEnvironment = getEnvironmentById(uuid);
        if (LinkStatusEnum.ONLINE.getStatus().toString().equals(clusterEnvironment.getLinkStatus())) {
            return;
        }
        log.error("Dws Cluster {} Not Online", clusterEnvironment.getName());
        throw new LegoCheckedException(CommonErrorCode.CLUSTER_LINK_STATUS_ERROR,
            "dws cluster " + clusterEnvironment.getName() + " not online");
    }

    /**
     * 获取已存在的DWS资源信息
     *
     * @param subType 资源子类型
     * @return 已存在的HBase资源信息
     */
    public List<ProtectedEnvironment> getExistingDwsResources(String subType) {
        Map<String, Object> filter = new HashMap<>();
        filter.put("type", ResourceTypeEnum.DATABASE.getType());
        filter.put("subType", subType);
        filter.put("sourceType", "register");
        List<ProtectedResource> existingResources = resourceService.query(0, LegoNumberConstant.TWENTY, filter)
            .getRecords();
        // 转成ProtectedEnvironment环境对象
        return existingResources.stream()
            .filter(existingResource -> existingResource instanceof ProtectedEnvironment)
            .map(existingResource -> (ProtectedEnvironment) existingResource)
            .collect(Collectors.toList());
    }

    /**
     * 获取所有subType和sourceType方式rootUuid为传参的列表
     *
     * @param uuid 资源的RootUuid
     * @param subType 资源的子类型
     * @param sourceType 资源的注册方式
     * @return 查询列表
     */
    public List<ProtectedResource> getEnvResourceList(String uuid, String subType, String sourceType) {
        List<ProtectedResource> protectedResources = new ArrayList<>();
        PageListResponse<ProtectedResource> records = resourceService.query(LegoNumberConstant.ZERO,
            LegoNumberConstant.ONE,
            ImmutableMap.of(DatabaseConstants.ROOT_UUID, uuid, "subType", subType, "sourceType", sourceType));
        int count = records.getTotalCount() / DwsConstant.QUERY_QUANTITY_SPECIFICATIONS;
        for (int size = 0; size <= count; size++) {
            PageListResponse<ProtectedResource> environments = resourceService.query(size,
                DwsConstant.QUERY_QUANTITY_SPECIFICATIONS,
                ImmutableMap.of(DatabaseConstants.ROOT_UUID, uuid, "subType", subType));
            protectedResources.addAll(environments.getRecords());
        }
        return protectedResources;
    }

    /**
     * 下发agent参数对象
     *
     * @param env agent 环境资源
     * @param environment 注册 环境资源
     * @param environmentConditions 过滤条件
     * @return 下发agent参数对象
     */
    public ListResourceV2Req getListResourceReq(ProtectedEnvironment env, ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        AppEnv appEnv = BeanTools.copy(env, AppEnv::new);
        Application application = new Application();
        application.setAuth(environment.getAuth());
        application.setName(environmentConditions.getParentId());
        application.setType(environment.getType());
        application.setSubType(environmentConditions.getResourceType());
        application.setExtendInfo(environment.getExtendInfo());

        ListResourceV2Req req = new ListResourceV2Req();
        req.setPageNo(environmentConditions.getPageNo());
        req.setPageSize(environmentConditions.getPageSize());
        req.setAppEnv(appEnv);
        req.setConditions(environmentConditions.getConditions());
        req.setApplications(Lists.newArrayList(application));
        return req;
    }

    /**
     * 获取schema集和表集的rootUuid集合
     *
     * @return schema集和表集的rootUuid集合
     */
    public List<String> getAllTableAndSchemaUuidList() {
        List<String> list = new ArrayList<>();
        PageListResponse<ProtectedResource> records = resourceService.query(LegoNumberConstant.ZERO,
            LegoNumberConstant.ONE, ImmutableMap.of("subType", ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()));
        int count = records.getTotalCount() / DwsConstant.QUERY_QUANTITY_SPECIFICATIONS;
        for (int size = 0; size <= count; size++) {
            PageListResponse<ProtectedResource> environments = resourceService.query(size,
                DwsConstant.QUERY_QUANTITY_SPECIFICATIONS,
                ImmutableMap.of("subType", ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()));
            list.addAll(environments.getRecords()
                .stream()
                .map(ProtectedResource::getRootUuid)
                .distinct()
                .collect(Collectors.toList()));
        }
        records = resourceService.query(LegoNumberConstant.ZERO, LegoNumberConstant.ONE,
            ImmutableMap.of("subType", ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType()));
        count = records.getTotalCount() / DwsConstant.QUERY_QUANTITY_SPECIFICATIONS;
        for (int size = 0; size <= count; size++) {
            PageListResponse<ProtectedResource> environments = resourceService.query(size,
                DwsConstant.QUERY_QUANTITY_SPECIFICATIONS,
                ImmutableMap.of("subType", ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType()));
            list.addAll(environments.getRecords()
                .stream()
                .map(ProtectedResource::getRootUuid)
                .distinct()
                .collect(Collectors.toList()));
        }
        return list.stream().distinct().collect(Collectors.toList());
    }

    /**
     * 获取第一个在线agent 环境
     *
     * @param clusterResources agent 资源列表
     * @return 在线agent 环境
     */
    public ProtectedEnvironment getClusterUuid(List<ProtectedResource> clusterResources) {
        List<String> clusterUuids = Optional.of(clusterResources)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "name is not exist"))
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        for (String clusterUuid : clusterUuids) {
            ProtectedEnvironment environmentById = getEnvironmentById(clusterUuid);
            if (LinkStatusEnum.OFFLINE.getStatus().toString().equals(environmentById.getLinkStatus())) {
                continue;
            }
            return environmentById;
        }
        throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "name is not exist");
    }

    /**
     * 获取第一个在线agent 环境
     *
     * @param advanceParams agents列表
     * @param uuid pluginType
     */
    public void modifyAdvanceParams(Map<String, String> advanceParams, String uuid) {
        if (!OpServiceUtil.isHcsService()) {
            return;
        }
        ProtectedResource resourceById = getResourceById(uuid);
        if (StringUtils.isEmpty(resourceById.getUserId())) {
            return;
        }
        List<VpcInfoEntity> vpcInfoEntities = iVpcService.getVpcInfoEntityByProjectId(
            resourceById.getUserId());
        advanceParams.put(VPC_INFO, JSON.toJSONString(vpcInfoEntities));
    }

    /**
     * 获取第一个在线agent 环境
     *
     * @param agents agents列表
     * @param uuid envId
     * @param pluginType pluginType
     */
    public void addSupplyAgent(List<Endpoint> agents, String uuid, String pluginType) {
        if (!OpServiceUtil.isHcsService()) {
            return;
        }
        log.info("HCS Environment register start");

        ProtectedEnvironment environmentById = getEnvironmentById(uuid);
        Set<String> resourceUuid = new HashSet<>();
        DwsTaskEnvironmentUtil.getAgentResourcesByKey(environmentById,
            DwsConstant.DWS_CLUSTER_AGENT).forEach(protectedResource -> resourceUuid.add(protectedResource.getUuid()));
        List<ProtectedResource> agentResourcesByKey = DwsTaskEnvironmentUtil.getAgentResourcesByKey(environmentById,
            DwsConstant.HOST_AGENT);

        // 注意！op服务化支持无代理场景
        if (Objects.nonNull(agentResourcesByKey) && !agentResourcesByKey.isEmpty()) {
            log.info("HCS Environment register with agent");
            agentResourcesByKey.forEach(protectedResource -> resourceUuid.add(protectedResource.getUuid()));
        }
        List<Endpoint> shardAgents = getHcsServiceEndPointList(pluginType);
        Set<String> collect = agents.stream().map(Endpoint::getId).collect(Collectors.toSet());
        for (Endpoint shardAgent : shardAgents) {
            if (collect.contains(shardAgent.getId())) {
                continue;
            }
            if (resourceUuid.contains(shardAgent.getId())) {
                agents.add(shardAgent);
            }
        }
    }

    private List<Endpoint> getHcsServiceEndPointList(String pluginType) {
        List<Endpoint> shardAgent = new ArrayList<>();
        openbackup.system.base.common.model.PageListResponse<AgentInfo> detailPageList =
            agentQueryService.querySharedAgents(pluginType);
        detailPageList.getRecords().forEach(agentInfo -> {
            Endpoint endpoint = new Endpoint();
            endpoint.setIp(agentInfo.getEndpoint());
            endpoint.setPort(agentInfo.getPort());
            endpoint.setId(agentInfo.getUuid());
            shardAgent.add(endpoint);
        });
        return shardAgent;
    }
}
