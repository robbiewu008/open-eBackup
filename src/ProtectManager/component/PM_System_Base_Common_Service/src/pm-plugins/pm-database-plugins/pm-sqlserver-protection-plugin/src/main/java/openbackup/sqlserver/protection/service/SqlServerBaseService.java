/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.sqlserver.protection.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.protection.service.repository.strategies.RepositoryStrategy;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.AgentDtoUtil;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.RequestUriUtil;
import openbackup.system.base.util.StreamUtil;

import com.baomidou.mybatisplus.core.toolkit.StringUtils;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;
import org.springframework.util.Assert;

import java.net.URI;
import java.util.ArrayList;
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
 * SqlServer应用基本的Service
 *
 * @author xwx950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-19
 */
@Slf4j
@Service("sqlServerBaseService")
public class SqlServerBaseService {
    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final RepositoryStrategyManager repositoryStrategyManager;

    /**
     * sqlserver应用基本的Service有参构造方法
     *
     * @param resourceService 资源服务接口
     * @param agentUnifiedService agent接口调用服务
     * @param repositoryStrategyManager 存储库管理类
     */
    public SqlServerBaseService(ResourceService resourceService, AgentUnifiedService agentUnifiedService,
        RepositoryStrategyManager repositoryStrategyManager) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.repositoryStrategyManager = repositoryStrategyManager;
    }

    /**
     * 检测实例是否存在
     *
     * @param resource 资源信息
     */
    public void checkInstanceExist(ProtectedResource resource) {
        Map<String, Object> condition = new HashMap<>();
        String hostId = resource.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
        condition.put(DatabaseConstants.PARENT_UUID, hostId);
        condition.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        List<ProtectedResource> records = resourceService.query(0, SqlServerConstants.MAX_RESOURCE_COUNT, condition)
            .getRecords();
        long count = records.stream()
            .filter(
                protectedResource -> protectedResource.getName().toUpperCase().equals(resource.getName().toUpperCase()))
            .count();
        log.info("Check sql server instance exist success, hostId: {}, instance size: {}, the same instance size: {}",
            hostId, records.size(), count);
        if (count != 0) {
            throw new DataProtectionAccessException(CommonErrorCode.DB_INSTANCE_HAS_REGISTERED, new String[] {},
                "SQL Server host has been registered.");
        }
    }

    /**
     * 针对单实例uuid，从单实例的dependency里，获取对应的Agent主机
     *
     * @param singleInstanceUuid 单实例uuid
     * @return Agent主机信息
     */
    public ProtectedEnvironment getAgentBySingleInstanceUuid(String singleInstanceUuid) {
        ProtectedResource singleInstanceResources = getResourceByUuid(singleInstanceUuid);
        List<ProtectedResource> agentResources = singleInstanceResources.getDependencies()
            .get(DatabaseConstants.AGENTS);
        if (agentResources == null || agentResources.size() != 1) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                "single instance dependency agent is not one.");
        }
        ProtectedResource agentResource = agentResources.get(0);
        PowerAssert.state(agentResource instanceof ProtectedEnvironment,
            () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "sqlserver agent resource is not env."));
        return (ProtectedEnvironment) agentResource;
    }

    /**
     * 针对集群实例uuid，从dependency里，获取集群实例下面的所有子实例
     *
     * @param clusterInstanceUuid 集群实例uuid
     * @return List ProtectedResource 集群实例依赖的子实例信息
     */
    public List<ProtectedResource> getSingleInstanceByClusterInstance(String clusterInstanceUuid) {
        ProtectedResource clusterInstanceResource = getResourceByUuid(clusterInstanceUuid);
        Map<String, List<ProtectedResource>> dependencies = clusterInstanceResource.getDependencies();
        return dependencies.get(DatabaseConstants.CHILDREN);
    }

    /**
     * 根据Agent主机信息，获取Agent主机的Endpoint
     *
     * @param env Agent环境信息
     * @return Agent对应的Endpoint信息
     */
    public Endpoint getAgentEndpoint(ProtectedEnvironment env) {
        if (VerifyUtil.isEmpty(env.getUuid()) || VerifyUtil.isEmpty(env.getEndpoint()) || VerifyUtil.isEmpty(
            env.getPort())) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "sqlserver agent env lack require msg.");
        }
        return new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort());
    }

    /**
     * 根据资源uuid，获取应该存在的资源信息
     *
     * @param resourceUuid 资源uuid
     * @return ProtectedResource 资源信息
     */
    public ProtectedResource getResourceByUuid(String resourceUuid) {
        return resourceService.getResourceById(resourceUuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource is not exist"));
    }

    /**
     * 设置恢复node
     *
     * @param restoreTask 恢复对象
     */
    public void supplyNodes(RestoreTask restoreTask) {
        List<TaskEnvironment> hostList = restoreTask.getAgents()
            .stream()
            .map(agent -> agentUnifiedService.getHost(agent.getIp(), agent.getPort()))
            .map(AgentDtoUtil::toTaskEnvironment)
            .collect(Collectors.toList());
        restoreTask.getTargetEnv().setNodes(hostList);
    }

    /**
     * 将资源环境信息转换为agents
     *
     * @param uuid 资源uuid
     * @return endpoints
     */
    public List<Endpoint> convertNodeListToAgents(String uuid) {
        return queryNodeList(uuid).stream()
            .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
            .collect(Collectors.toList());
    }

    /**
     * 查询资源集群节点
     *
     * @param uuid 资源uuid
     * @return 资源集群节点
     */
    public List<TaskEnvironment> queryNodeList(String uuid) {
        ProtectedResource resource = getResourceByUuid(uuid);
        switch (ResourceSubTypeEnum.get(resource.getSubType())) {
            case SQL_SERVER_DATABASE:
                return generateTaskEnvironments(getResourceByUuid(resource.getParentUuid()));
            case SQL_SERVER_INSTANCE:
            case SQL_SERVER_CLUSTER_INSTANCE:
                return generateTaskEnvironments(resource);
            case SQL_SERVER_ALWAYS_ON:
                return resource.getDependencies()
                    .get(DatabaseConstants.INSTANCE)
                    .stream()
                    .map(instance -> generateTaskEnvironments(getResourceByUuid(instance.getUuid())))
                    .flatMap(List::stream)
                    .collect(Collectors.toList());
            default:
                log.error("[SQL Server] protect resource subType error. resourceId: {}, subType: {}", uuid,
                    resource.getSubType());
                return Collections.emptyList();
        }
    }

    private List<TaskEnvironment> generateTaskEnvironments(ProtectedResource protectedResource) {
        return protectedResource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(childNode -> {
                childNode.setAuth(protectedResource.getAuth());
                return BeanTools.copy(childNode, TaskEnvironment::new);
            })
            .collect(Collectors.toList());
    }

    /**
     * 查询可用性组或者实例下面的数据库
     *
     * @param alwaysOnUuid 可用性组Uuid，实例uuid
     * @return 保护资源查询对象
     */
    public PageListResponse<ProtectedResource> queryDatabasesInAlwaysOnOrInstance(String alwaysOnUuid) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType());
        conditions.put(DatabaseConstants.PARENT_UUID, alwaysOnUuid);
        PageListResponse<ProtectedResource> pageListResponse = resourceService.query(0,
            SqlServerConstants.MAX_RESOURCE_COUNT, conditions);
        if (pageListResponse.getTotalCount() > pageListResponse.getRecords().size()) {
            pageListResponse = resourceService.query(0, pageListResponse.getTotalCount(), conditions);
        }
        return pageListResponse;
    }

    /**
     * 根据集群id查询集群下集群实例列表
     *
     * @param clusterId 集群ID
     * @param resourceType 资源类型
     * @param isDecrypt 是否需要解密
     * @return 集群实例列表
     */
    public List<ProtectedResource> getResourceOfClusterByType(String clusterId, String resourceType,
        boolean isDecrypt) {
        Map<String, Object> instanceConditions = new HashMap<>();
        instanceConditions.put(DatabaseConstants.ROOT_UUID, clusterId);
        instanceConditions.put(DatabaseConstants.SUB_TYPE, resourceType);
        ResourceQueryParams queryContext = new ResourceQueryParams();
        queryContext.setShouldDecrypt(isDecrypt);
        queryContext.setShouldQueryDependency(true);
        queryContext.setConditions(instanceConditions);
        queryContext.setPage(0);
        queryContext.setSize(SqlServerConstants.MAX_RESOURCE_COUNT);
        return resourceService.query(queryContext).getRecords();
    }

    /**
     * 根据资源id列表查询对应的环境列表
     *
     * @param resourceIds 资源id列表
     * @return 环境列表
     */
    public List<ProtectedEnvironment> getProtectedEnvironmentByResourceList(List<ProtectedResource> resourceIds) {
        if (VerifyUtil.isEmpty(resourceIds)) {
            return new ArrayList<>();
        }
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.UUID,
            resourceIds.stream().map(ProtectedResource::getUuid).collect(Collectors.toList()));
        List<ProtectedEnvironment> result = new ArrayList<>(resourceIds.size());
        resourceService.query(0, resourceIds.size(), conditions, "")
            .getRecords()
            .stream()
            .filter(resource -> resource instanceof ProtectedEnvironment)
            .forEach(resource -> result.add((ProtectedEnvironment) resource));
        return result;
    }

    /**
     * 扫描实例数据库
     *
     * @param environment 环境信息
     * @param endpoint 地址
     * @param port 端口
     * @param instance 实例信息
     * @param subType 资源子类型
     * @return 返回信息
     */
    public List<ProtectedResource> getDatabaseInfoByAgent(ProtectedEnvironment environment, String endpoint,
        Integer port, ProtectedResource instance, String subType) {
        ListResourceReq sqlServerDbsRequest = new ListResourceReq();
        sqlServerDbsRequest.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        sqlServerDbsRequest.setApplication(BeanTools.copy(instance, Application::new));
        AgentDetailDto database;
        try {
            database = agentUnifiedService.getDetail(subType, endpoint, port, sqlServerDbsRequest);
        } catch (LegoCheckedException exception) {
            log.warn("[SQL Server] environment: {}, instance id :{}, database scan error: ", environment.getName(),
                instance.getUuid(), exception);
            throw exception;
        }
        List<AppResource> resourceList = Optional.ofNullable(database.getResourceList()).orElse(new ArrayList<>(0));
        log.info("[SQL Server] get database from agent success, AppResource size: {}, instance id : {}",
            resourceList.size(), instance.getUuid());
        return resourceList.stream()
            .map(appResource -> BeanTools.copy(appResource, ProtectedResource::new))
            .collect(Collectors.toList());
    }

    /**
     * 根据节点信息，检查是否均属于同一集群
     *
     * @param agents 集群节点列表
     */
    public void checkAllNodesBelongToCluster(List<ProtectedEnvironment> agents) {
        String allNodesName = agents.stream()
            .map(ProtectedEnvironment::getName)
            .collect(Collectors.joining(DatabaseConstants.SPLIT_CHAR));
        AppEnv appEnv = new AppEnv();
        Map<String, String> extendInfo = new HashMap<>(1);
        extendInfo.put(DatabaseConstants.ALL_NODES, allNodesName);
        appEnv.setExtendInfo(extendInfo);
        Application application = new Application();
        application.setSubType(ResourceSubTypeEnum.SQL_SERVER_CLUSTER.getType());
        application.setType(ResourceTypeEnum.DATABASE.getType());
        CheckAppReq checkAppReq = new CheckAppReq(appEnv, application);
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        for (ProtectedEnvironment agent : agents) {
            URI uri = RequestUriUtil.getRequestUri(agent.getEndpoint(), agent.getPort());
            try {
                agentBaseDto = agentUnifiedService.check(ResourceSubTypeEnum.SQL_SERVER_CLUSTER.getType(), agent,
                    checkAppReq);
            } catch (LegoCheckedException e) {
                // 处理未配置powershell异常
                log.error("Fail to check application, uri: {}.", uri, e);
                agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
                if (!VerifyUtil.isEmpty(e.getMessage())) {
                    agentBaseDto.setErrorMessage(e.getMessage());
                    ActionResult actionResult = getActionResultFromErrorMessage(agentBaseDto.getErrorMessage());
                    throw new LegoCheckedException(actionResult.getCode(), actionResult.getMessage());
                }
            }
            log.info("[SQL Server] cluster check by uri: {} fail, nodes size: {}, agent return code: {}", uri,
                agents.size(), agentBaseDto.getErrorCode());
        }
        // 所有agent都需要验证属于同一集群
        if (agentBaseDto.isAgentBaseDtoReturnSuccess()) {
            log.info("[SQL Server] cluster check by uri success, all nodes belong to one cluster.");
            return;
        }
        throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_ADD_FAILED,
            "[SQL Server] nodes are not belong to one cluster.");
    }

    private ActionResult getActionResultFromErrorMessage(String errorMessage) {
        JSONObject jsonObject = JSONObject.fromObject(errorMessage);
        String bodyErr = jsonObject.getString("bodyErr");
        ActionResult actionResult;
        if (VerifyUtil.isEmpty(bodyErr)) {
            actionResult = new ActionResult(CommonErrorCode.AGENT_NETWORK_ERROR, "check application failed.");
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        } else {
            actionResult = JSONObject.toBean(errorMessage, ActionResult.class);
        }
        actionResult.setCode(Long.parseLong(actionResult.getBodyErr()));
        return actionResult;
    }

    /**
     * 校验恢复任务相关参数
     *
     * @param restoreTask 恢复任务
     */
    public void checkRestoreTaskParam(RestoreTask restoreTask) {
        // 新位置恢复校验目标路径合法性
        if (RestoreLocationEnum.NEW.getLocation().equals(restoreTask.getTargetLocation().getLocation())) {
            restoreTask.getAdvanceParams().put(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_PATH, "true");
        } else {
            restoreTask.getAdvanceParams().put(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_PATH, "false");
        }
    }

    /**
     * 获取某一个可用性组下的所有关联的实例uuid列表
     *
     * @param agGroup 实例资源
     * @return 可用性组资源关联的实例uuid列表
     */
    public List<String> getInstanceListDependentOnAGroup(ProtectedResource agGroup) {
        return Optional.ofNullable(agGroup.getDependencies())
            .orElse(new HashMap<>())
            .getOrDefault(SqlServerConstants.INSTANCE, new ArrayList<>())
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
    }

    /**
     * 构建锁资源为 LockResourceBo列表
     *
     * @param relatedLockResources relatedLockResources
     * @return List<LockResourceBo>
     */
    public List<LockResourceBo> buildLockResourceList(Set<String> relatedLockResources) {
        return relatedLockResources.stream()
            .filter(Objects::nonNull)
            .map(lockResourceUuid -> new LockResourceBo(lockResourceUuid, LockType.WRITE))
            .collect(Collectors.toList());
    }

    /**
     * 获取与此相关联的资源，用于恢复成功后下次转全量
     *
     * @param task RestoreTask
     * @return 关联资源，若包含自身，也需要返回
     */
    public List<String> findAssociatedResourcesToSetNextFull(RestoreTask task) {
        ProtectedResource resource = getResourceByUuid(task.getTargetObject().getUuid());
        String subType = resource.getSubType();
        if (!StringUtils.equals(task.getTargetLocation().getLocation(), RestoreLocationEnum.ORIGINAL.getLocation())) {
            return Collections.emptyList();
        }
        // 数据库副本恢复到原位置：targetObj是数据库，需要取parentUuid取到对应的实例（单机实例或者集群实例）
        if (ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType().equals(subType)) {
            resource = getResourceByUuid(resource.getParentUuid());
            subType = resource.getSubType();
            log.info("[SQL Server] SQL_SERVER_DATABASE of related resource id is : {}, requestId : {}.",
                resource.getUuid(), task.getRequestId());
        }

        Set<String> allResourcesIds = new HashSet<>();
        // 添加目标对象自身资源id到allResourcesIds
        allResourcesIds.add(resource.getUuid());
        // 添加目标对象关联资源id到allResourcesIds
        return getAllResourcesId(task, resource, subType, allResourcesIds);
    }

    private List<String> getAllResourcesId(RestoreTask task, ProtectedResource resource, String subType,
        Set<String> allResourcesIds) {
        switch (ResourceSubTypeEnum.get(subType)) {
            // targetObj是单实例场景，只需要取出单实例以及单实例下所有数据库
            case SQL_SERVER_INSTANCE:
                allResourcesIds.addAll(resourceService.queryRelatedResourceUuids(resource.getUuid(), new String[] {}));
                log.info("[SQL Server] SQL_SERVER_INSTANCE of related resource id is : {}, requestId : {}, size: {},"
                        + " allResourcesIds: {}.", resource.getUuid(), task.getRequestId(), allResourcesIds.size(),
                    JSONArray.fromObject(allResourcesIds).toString());
                break;
            // targetObj是集群实例场景，需要取出集群实例以及集群实例下所有数据库，相关联可用性组
            case SQL_SERVER_CLUSTER_INSTANCE:
                allResourcesIds.addAll(resourceService.queryRelatedResourceUuids(resource.getUuid(), new String[] {}));
                allResourcesIds.addAll(getAGroupIdListDependentOnInstance(resource));
                log.info(
                    "[SQL Server] SQL_SERVER_CLUSTER_INSTANCE of related resource id is : {}, requestId : {}, size: {},"
                        + " allResourcesIds: {}", resource.getUuid(), task.getRequestId(), allResourcesIds.size(),
                    JSONArray.fromObject(allResourcesIds).toString());
                break;
            // targetObj是可用性组场景，需要取出可用性组，相关联集群实例
            case SQL_SERVER_ALWAYS_ON:
                allResourcesIds.addAll(getInstanceListDependentOnAGroup(resource));
                log.info("[SQL Server] SQL_SERVER_ALWAYS_ON of related resource id is : {}, requestId : {}, size: {},"
                        + " allResourcesIds: {}.", resource.getUuid(), task.getRequestId(), allResourcesIds.size(),
                    JSONArray.fromObject(allResourcesIds).toString());
                break;
            default:
                log.error("[SQL Server] DATABASE of related resource id is : {}, requestId : {}, size: {}, "
                        + "allResourcesIds: {}.", resource.getUuid(), task.getRequestId(), allResourcesIds.size(),
                    JSONArray.fromObject(allResourcesIds).toString());
                break;
        }
        return new ArrayList<>(allResourcesIds);
    }

    /**
     * 获取某一个实例下的所有关联的可用性组，发现到界面上的（不包含未入库的）
     *
     * @param instance 实例资源
     * @return 实例关联的可用性组资源uuid列表
     */
    public List<String> getAGroupIdListDependentOnInstance(ProtectedResource instance) {
        List<ProtectedResource> alwaysOns = getResourceOfClusterByType(instance.getRootUuid(),
            ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType(), false);
        List<String> dependentAlwaysOnIdList = new ArrayList<>();
        if (!VerifyUtil.isEmpty(alwaysOns)) {
            dependentAlwaysOnIdList = alwaysOns.stream()
                .filter(agGroup -> getInstanceListDependentOnAGroup(agGroup).contains(instance.getUuid()))
                .map(ProtectedResource::getUuid)
                .collect(Collectors.toList());
        }
        return dependentAlwaysOnIdList;
    }

    /**
     * 注入日志副本的data仓
     *
     * @param task RestoreTask
     * @param copyRestApi 副本rest api接口
     */
    public void logRestoreAddData(RestoreTask task, CopyRestApi copyRestApi) {
        final Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        if (!BackupTypeConstants.LOG.equals(BackupTypeConstants.getBackupTypeByAbBackupType(copy.getBackupType()))) {
            return;
        }
        Optional<Copy> latestFullBackupCopy = copyRestApi.queryLatestFullBackupCopies(copy.getResourceId(),
            copy.getGn(), BackupTypeEnum.FULL.getAbbreviation());
        if (!latestFullBackupCopy.isPresent()) {
            return;
        }
        List<StorageRepository> storageRepositories = getStorageRepositories(
            latestFullBackupCopy.get().getProperties());
        storageRepositories = storageRepositories.stream()
            .filter(storageRepository -> RepositoryTypeEnum.DATA.getType() == storageRepository.getType())
            .collect(Collectors.toList());
        task.getRepositories().addAll(storageRepositories);
    }

    /**
     * 获取副本信息中的存储库列表
     *
     * @param copyProperties 副本扩展信息
     * @return 存储库列表 {@code List<StorageRepository>}
     */
    public List<StorageRepository> getStorageRepositories(String copyProperties) {
        // 副本中的扩展参数数据，json字符串形式存储
        final JSONObject extendParam = JSONObject.fromObject(copyProperties);
        final JSONArray repositories = extendParam.getJSONArray(CopyPropertiesKeyConstant.KEY_REPOSITORIES);
        Assert.notEmpty(repositories, "Copy properties not contains repositories info");
        return repositories.map(repository -> this.buildStorageRepository(
            JSONObject.fromObject(repository).toBean(BaseStorageRepository.class)));
    }

    private StorageRepository buildStorageRepository(BaseStorageRepository base) {
        log.info("Restore task build repository, id={}, type={}, protocol={}, role={}", base.getId(), base.getType(),
            base.getProtocol(), base.getRole());
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(
            RepositoryProtocolEnum.getByProtocol(base.getProtocol()));
        final StorageRepository repository = strategy.getRepository(base);
        repository.encryptPassword();
        return repository;
    }
}