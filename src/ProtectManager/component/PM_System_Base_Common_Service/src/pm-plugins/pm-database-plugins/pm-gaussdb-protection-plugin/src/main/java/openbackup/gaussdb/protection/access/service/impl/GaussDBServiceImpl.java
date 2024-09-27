package openbackup.gaussdb.protection.access.service.impl;

import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupModifyReq;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.gaussdb.protection.access.constant.GaussDBConstant;
import openbackup.gaussdb.protection.access.service.GaussDBService;
import openbackup.gaussdb.protection.access.util.GaussDBClusterUtils;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.ObjectUtils;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * GaussDB 应用基本的Service
 *
 * @author t30021437
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-02-09
 */
@Slf4j
@Service
public class GaussDBServiceImpl implements GaussDBService {
    private final ResourceService resourceService;

    private final ProviderManager providerManager;

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    private final UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    private final TaskRepositoryManager taskRepositoryManager;

    /**
     * GaussDB 应用基本的Service有参构造方法
     *
     * @param resourceService 资源服务接口
     * @param providerManager DataMover provider registry
     * @param resourceConnectionCheckProvider 资源连通性检查provider
     * @param clusterIntegrityChecker 资源连通性检查provider
     * @param taskRepositoryManager repositoryManager
     */
    public GaussDBServiceImpl(ResourceService resourceService, ProviderManager providerManager,
        @Qualifier("unifiedConnectionCheckProvider") ResourceConnectionCheckProvider resourceConnectionCheckProvider,
        UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker, TaskRepositoryManager taskRepositoryManager) {
        this.resourceService = resourceService;
        this.providerManager = providerManager;
        this.resourceConnectionCheckProvider = resourceConnectionCheckProvider;
        this.clusterIntegrityChecker = clusterIntegrityChecker;
        this.taskRepositoryManager = taskRepositoryManager;
    }

    /**
     * 获取environment 对象
     *
     * @param envId environment的uuid
     * @return 查询资源对象
     */
    @Override
    public ProtectedEnvironment getEnvironmentById(String envId) {
        log.info("start to query Environment by id");
        Optional<ProtectedResource> resOptional = resourceService.getResourceByIdIgnoreOwner(envId);
        if (resOptional.isPresent() && resOptional.get() instanceof ProtectedEnvironment) {
            log.info("start to query getEnvironmentById :{}", resOptional.get());
            return (ProtectedEnvironment) resOptional.get();
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!");
    }

    /**
     * 检查连通性
     *
     * @param protectedResource protectedResource
     */
    @Override
    public void checkConnention(ProtectedResource protectedResource) {
        log.info("check connection");
        ResourceConnectionCheckProvider provider = providerManager.findProviderOrDefault(
            ResourceConnectionCheckProvider.class, protectedResource, resourceConnectionCheckProvider);
        provider.checkConnection(protectedResource);
    }

    /**
     * 检查连通性
     *
     * @param agentResources agentResources agent 列表
     * @param protectedResource protectedResource
     * @return AppEnvResponse AppEnvResponse agent 信息
     */
    @Override
    public AppEnvResponse getAppEnvResponse(List<ProtectedResource> agentResources,
        ProtectedResource protectedResource) {
        log.info("query agent environment");
        ProtectedEnvironment protectedEnvironment = getEnvironmentById(Optional.of(agentResources)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "name is not exist"))
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList())
            .get(GaussDBConstant.INT_ZERO));
        protectedResource.setEnvironment(protectedEnvironment);
        CheckResult<AppEnvResponse> appEnvResponseCheckResult = clusterIntegrityChecker.generateCheckResult(
            protectedResource);
        AppEnvResponse appEnv = appEnvResponseCheckResult.getData();
        if (ObjectUtils.isEmpty(appEnvResponseCheckResult.getData())) {
            log.error("The GaussDb instance nodes query failed.");
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "The GaussDb instance nodes query failed.");
        }
        return appEnv;
    }

    /**
     * 获取已存在的GaussDb资源信息
     *
     * @param subType 资源类型
     * @param filter 查询条件
     * @return 已存在的GaussDb资源信息
     */
    @Override
    public List<ProtectedResource> getExistingGaussDbResources(String subType, Map<String, Object> filter) {
        log.info("query the existing gaussDb resources");
        filter.put("type", ResourceTypeEnum.DATABASE.getType());
        filter.put("subType", subType);
        List<ProtectedResource> existingResources = new ArrayList<>();
        PageListResponse<ProtectedResource> response;
        int count = GaussDBConstant.INT_ZERO;
        do {
            response = resourceService.query(count++, GaussDBConstant.QUERY_SIZE, filter);
            if (!response.getRecords().isEmpty()) {
                existingResources.addAll(response.getRecords());
            }
        } while (response.getRecords().size() == GaussDBConstant.QUERY_SIZE);
        return Optional.ofNullable(existingResources).orElse(Collections.emptyList());
    }

    /**
     * 修改task采纳数信息
     *
     * @param backupTask backupTask
     */
    @Override
    public void modifyBackupTaskParam(BackupTask backupTask) {
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());

        // 添加存储集群的X8000型号; esn role角色;
        List<StorageRepository> repositories = backupTask.getRepositories();
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            backupTask.addRepository(logRepository);
            repositories.remove(0);
        }
        repositories.add(GaussDBClusterUtils.getCacheRepository(repositories.get(0)));
        Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams()).orElse(new HashMap<>());

        // 默认支持多文件系统
        // 1.元数据存放路径（"metadataPath":"/opt/meta/"）、备份方式("backupToolType":Roach)放在advanceParams里;
        advanceParams.put(GaussDBConstant.REPOSITORIES_KEY_MULTI_FILE_SYSTEM,
            DatabaseConstants.MULTI_FILE_SYSTEM_VALUE_ENABLE);
        advanceParams.put(GaussDBConstant.SPEED_STATISTICS, SpeedStatisticsEnum.APPLICATION.getType());

        advanceParams.put(DatabaseConstants.FORBID_WORM_FILE_SYSTEM, Boolean.TRUE.toString());
        GaussDBClusterUtils.modifyAdvanceParam(advanceParams, GaussDBConstant.ADVANCE_PARAMS_KEY_BACKUP_METADATA_PATH,
            GaussDBConstant.ADVANCE_PARAMS_KEY_METADATA_PATH);
        GaussDBClusterUtils.modifyAdvanceParam(advanceParams, GaussDBConstant.ADVANCE_PARAMS_KEY_BACKUP_TOOL_TYPE,
            GaussDBConstant.ADVANCE_PARAMS_KEY_TOOL_TYPE);

        ProtectedEnvironment environment = getEnvironmentById(backupTask.getProtectObject().getRootUuid());
        TaskEnvironment protectEnv = backupTask.getProtectEnv();

        // 3.用户名("gaussDbUser":"omm"))放在protectEnv->extendInfo里;
        GaussDBClusterUtils.initProtectEnvOfGaussDbUser(protectEnv, environment.getAuth().getAuthKey());
        protectEnv.getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        buildEnvironmentNodes(protectEnv);
    }

    private TaskEnvironment buildEnvironmentNodes(TaskEnvironment taskEnvironment) {
        ProtectedEnvironment protectedEnvironment = getEnvironmentById(taskEnvironment.getUuid());

        List<ProtectedEnvironment> agentsEnvironments = protectedEnvironment.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .collect(Collectors.toList());

        List<TaskEnvironment> newNodes = new ArrayList<>();

        List<TaskEnvironment> nodes = taskEnvironment.getNodes();
        for (TaskEnvironment node : nodes) {
            Optional<ProtectedEnvironment> protectedEnv = agentsEnvironments.stream()
                .filter(agent -> agent.getName().equals(node.getName()))
                .findFirst();
            protectedEnv.ifPresent(environment -> {
                node.setUuid(environment.getUuid());
                node.setPort(environment.getPort());
            });
            newNodes.add(node);
        }
        taskEnvironment.setNodes(newNodes);
        return taskEnvironment;
    }

    /**
     * 返回节点信息列表
     *
     * @param uuid 集群uuid
     * @return 节点信息列表
     */
    @Override
    public List<TaskEnvironment> supplyNodes(String uuid) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(uuid);
        if (!resOptional.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }

        List<ProtectedEnvironment> gaussDbClusterAgent = Optional.ofNullable(
                resOptional.get().getDependencies().get(GaussDBConstant.GAUSSDB_AGENTS))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "GaussDB cluster not exists."))
            .stream()
            .map(ProtectedResource::getUuid)
            .map(this::getEnvironmentById)
            .collect(Collectors.toList());

        List<TaskEnvironment> nodes = new ArrayList<>();
        nodes.addAll(gaussDbClusterAgent.stream()
            .filter(EnvironmentLinkStatusHelper::isOnlineAdaptMultiCluster)
            .map(environment -> BeanTools.copy(environment, TaskEnvironment::new))
            .collect(Collectors.toList()));
        return nodes;
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
        updateResource.setExtendInfoByKey(DatabaseConstants.STATUS, status);
        resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
    }

    /**
     * 若上一次日志备份失败, 设置下一次备份为全量备份
     *
     * @param postBackupTask 任务参数
     */
    @Override
    public void setNextBackupTypeWhenLogBackFail(PostBackupTask postBackupTask) {
        // 如若上一次日志备份失败, 设置下一次备份为全量备份
        if (BackupTypeEnum.LOG.getAbbreviation() == postBackupTask.getBackupType().getAbbreviation()
            && ProviderJobStatusEnum.FAIL.equals(postBackupTask.getJobStatus())) {
            log.info("Hcs gauss db last log backup task failed! Start to set next backup type");
            NextBackupModifyReq nextBackupModifyReq = NextBackupModifyReq.build(
                postBackupTask.getProtectedObject().getResourceId(),
                NextBackupChangeCauseEnum.VERIFY_FAILED_TO_FULL_LABEL);
            resourceService.modifyNextBackup(nextBackupModifyReq);
        }
    }
}
