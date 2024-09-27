package openbackup.data.access.framework.backup.service.impl;

import static openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum.SANCLIENT_DISABLED;
import static openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum.SANCLIENT_ENABLED;

import com.huawei.oceanprotect.base.cluster.sdk.dto.StorageUnitQueryParam;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.sla.sdk.enums.BackupStorageTypeEnum;
import com.huawei.oceanprotect.system.base.user.service.UserService;
import com.huawei.oceanprotect.system.sdk.enums.SwitchNameEnum;
import com.huawei.oceanprotect.system.sdk.enums.SwitchStatusEnum;
import com.huawei.oceanprotect.system.sdk.service.SystemSwitchInternalService;

import com.fasterxml.jackson.databind.JsonNode;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dee.DeeCopiesManagementRestApi;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.access.framework.backup.dto.StorageInfoDto;
import openbackup.data.access.framework.backup.service.IBackupService;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.copy.mng.service.DeeCopyService;
import openbackup.data.access.framework.copy.mng.service.FsSnapshotService;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TaskParamConstants;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.access.framework.protection.common.util.FibreUtil;
import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.access.framework.servitization.util.OpServiceHelper;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum;
import openbackup.data.protection.access.provider.sdk.backup.PredefineBackupParameters;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.backup.ResourceExtendInfoConstants;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupScript;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.DataLayout;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryProvider;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.Qos;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.ClientProtocolTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.resource.NextBackupParams;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.SelectorManager;
import openbackup.data.protection.access.provider.sdk.sla.Policy;
import openbackup.data.protection.access.provider.sdk.sla.Sla;
import openbackup.data.protection.access.provider.sdk.util.AgentApiUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.job.util.JobUpdateUtil;
import openbackup.system.base.sdk.protection.QosCommonRestApi;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.QosBo;
import openbackup.system.base.sdk.protection.model.SlaBo;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.AvailableAgentManagementDomainService;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.EnumUtil;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;
import org.springframework.util.StringUtils;

import java.net.URI;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 具体处理备份请求的实现
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-25
 */
@Service
@Slf4j
public class BackupServiceImpl implements IBackupService {
    private static final String HCS_TOKEN = "hcs_token";

    private static final List<String> EXCLUDE_KEYS = Arrays.stream(PredefineBackupParameters.values())
            .map(PredefineBackupParameters::getKey).collect(Collectors.toList());

    private final ProviderManager providerManager;

    private final ResourceService resourceService;

    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final QosCommonRestApi qosCommonRestApi;

    private final DmeUnifiedRestApi unifiedRestApi;

    private DeeCopiesManagementRestApi deeCopiesManagementRestApi;

    private RepositoryStrategyManager repositoryStrategyManager;

    private CopyRestApi copyRestApi;

    private TaskRepositoryManager taskRepositoryManager;

    private SystemSwitchInternalService systemSwitchInternalService;

    private RedissonClient redissonClient;

    private JobService jobService;

    private UserService userService;

    private DeployTypeService deployTypeService;

    private AvailableAgentManagementDomainService domainService;

    private OpServiceHelper opServiceHelper;

    private SanClientService sanClientService;

    private DeeCopyService deeCopyService;

    private MemberClusterService memberClusterService;

    private CommonAgentService commonAgentService;
    private SelectorManager selectorManager;

    private StorageUnitService storageUnitService;

    private BackupStorageApi backupStorageApi;

    private DefaultStorageRepositoryProvider defaultStorageRepositoryProvider;

    private FsSnapshotService fsSnapshotService;

    /**
     * 构造函数
     *
     * @param providerManager Provider管理器{@link ProviderManager}
     * @param resourceService 资源服务接口{@link ResourceService}
     * @param protectedEnvironmentService 受保护环境服务接口{@link ProtectedEnvironmentService}
     * @param qosCommonRestApi QOS API
     * @param unifiedRestApi DME 统一备份API
     */
    public BackupServiceImpl(ProviderManager providerManager, ResourceService resourceService,
            ProtectedEnvironmentService protectedEnvironmentService, QosCommonRestApi qosCommonRestApi,
            DmeUnifiedRestApi unifiedRestApi) {
        this.providerManager = providerManager;
        this.resourceService = resourceService;
        this.qosCommonRestApi = qosCommonRestApi;
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.unifiedRestApi = unifiedRestApi;
    }

    @Autowired
    public void setBackupStorageApi(BackupStorageApi backupStorageApi) {
        this.backupStorageApi = backupStorageApi;
    }

    @Autowired
    public void setCommonAgentService(CommonAgentService commonAgentService) {
        this.commonAgentService = commonAgentService;
    }

    @Autowired
    public void setStorageUnitService(StorageUnitService storageUnitService) {
        this.storageUnitService = storageUnitService;
    }

    @Autowired
    public void setRepositoryStrategyManager(RepositoryStrategyManager repositoryStrategyManager) {
        this.repositoryStrategyManager = repositoryStrategyManager;
    }

    @Autowired
    public void setDeployTypeService(DeployTypeService deployTypeService) {
        this.deployTypeService = deployTypeService;
    }

    @Autowired
    public void setRedissonClient(RedissonClient redissonClient) {
        this.redissonClient = redissonClient;
    }

    @Autowired
    public void setJobService(final JobService jobService) {
        this.jobService = jobService;
    }

    @Autowired
    public void setUserService(final UserService userService) {
        this.userService = userService;
    }

    @Autowired
    public void setCopyRestApi(CopyRestApi copyRestApi) {
        this.copyRestApi = copyRestApi;
    }

    @Autowired
    public void setTaskRepositoryManager(TaskRepositoryManager taskRepositoryManager) {
        this.taskRepositoryManager = taskRepositoryManager;
    }

    @Autowired
    public void setSystemSwitchInternalService(SystemSwitchInternalService systemSwitchInternalService) {
        this.systemSwitchInternalService = systemSwitchInternalService;
    }

    @Autowired
    public void setOpServiceHelper(OpServiceHelper opServiceHelper) {
        this.opServiceHelper = opServiceHelper;
    }

    @Autowired
    public void setAvailableAgentManagementDomainService(AvailableAgentManagementDomainService domainService) {
        this.domainService = domainService;
    }

    @Autowired
    public void setDeeCopiesManagementRestApi(DeeCopiesManagementRestApi deeCopiesManagementRestApi) {
        this.deeCopiesManagementRestApi = deeCopiesManagementRestApi;
    }

    @Autowired
    public void setDeeCopyService(DeeCopyService deeCopyService) {
        this.deeCopyService = deeCopyService;
    }

    @Autowired
    public void setSanClientService(SanClientService sanClientService) {
        this.sanClientService = sanClientService;
    }

    @Autowired
    public void setMemberClusterService(MemberClusterService memberClusterService) {
        this.memberClusterService = memberClusterService;
    }

    @Autowired
    public void setStorageRepositoryProvider(DefaultStorageRepositoryProvider defaultStorageRepositoryProvider) {
        this.defaultStorageRepositoryProvider = defaultStorageRepositoryProvider;
    }

    @Autowired
    public void setSelectorManager(SelectorManager selectorManager) {
        this.selectorManager = selectorManager;
    }

    @Autowired
    public void setFsSnapshotService(FsSnapshotService fsSnapshotService) {
        this.fsSnapshotService = fsSnapshotService;
    }

    @ExterAttack
    @Override
    public BackupTask backup(BackupObject backupObject) {
        log.info("Begin to prepare backup task parameter for {}. requestId is {}",
                backupObject.getProtectedObject().getName(), backupObject.getRequestId());

        if (deployTypeService.isHyperDetectDeployType()) {
            return deeCopyService.hyperDetectBackup(backupObject);
        }

        if (deployTypeService.isCyberEngine()) {
            ProtectedResource protectedResource = getProtectedResource(backupObject.getProtectedObject()
                .getResourceId());
            return fsSnapshotService.oceanCyberDetectBackup(backupObject, protectedResource);
        }

        if (isAgentLessBackup(backupObject)) {
            return agentLessBackup(backupObject);
        }

        return commonBackup(backupObject);
    }

    private boolean isAgentLessBackup(BackupObject backupObject) {
        return ResourceSubTypeEnum.COMMON_SHARE.equalsSubType(backupObject.getProtectedObject().getSubType());
    }

    private BackupTask agentLessBackup(BackupObject backupObject) {
        // 框架负责构造通用的备份请求
        BackupTask task = new BackupTask();
        try {
            task = buildBackupRequest(backupObject);
            // agentLess 不传agent
            task.setAgents(Collections.emptyList());
            interceptor(task, backupObject.getProtectedObject().getSubType());
            // 根据requestId取出缓存，设置备份任务的COPY_FORMAT
            RMap<String, String> redis = redissonClient.getMap(task.getRequestId(), StringCodec.INSTANCE);
            redis.put(ContextConstants.COPY_FORMAT, String.valueOf(task.getCopyFormat()));
            jobService.updateJob(task.getRequestId(), JobUpdateUtil.getDeliverReq());
        } finally {
            cleanAuthPwd(task);
        }
        return task;
    }

    private void interceptor(BackupTask task, String subType) {
        // 获取资源的备份请求拦截器，如果插件提供了相应的拦截器，则应用拦截器
        BackupInterceptorProvider interceptor =
                providerManager.findProvider(BackupInterceptorProvider.class, subType, null);
        setAgentMountType(task, interceptor);
        if (interceptor != null) {
            log.debug("Backup interceptor for {} is provided, and the interceptor will be used!", subType);
            interceptor.initialize(task);
        }
        paramCorrection(task);
    }

    private void paramCorrection(BackupTask task) {
        if (deployTypeService.isE1000()) {
            task.getRepositories().forEach(repo -> {
                if (repo.getEndpoint() != null && repo.getEndpoint().getPort() < 0) {
                    repo.getEndpoint().setPort(0);
                }
            });
        }
    }

    private void setAgentMountType(BackupTask task, BackupInterceptorProvider interceptor) {
        if (interceptor != null) {
            Optional<AgentMountTypeEnum> mountTypeOp = interceptor.getMountType(task);
            if (mountTypeOp.isPresent()) {
                task.addParameter(TaskParamConstants.AGENT_MOUNT_TYPE, mountTypeOp.get().getValue());
                return;
            }
        }
        task.addParameter(TaskParamConstants.AGENT_MOUNT_TYPE,
            commonAgentService.getJobAgentMountTypeByJob(task.getTaskId()).getValue());
    }

    private BackupTask commonBackup(BackupObject backupObject) {
        // 框架负责构造通用的备份请求
        BackupTask task = new BackupTask();
        try {
            task = buildBackupRequest(backupObject);
            interceptor(task, backupObject.getProtectedObject().getSubType());
            // 校验是否可以下发到dme
            checkBackupTask(task);
            // 根据requestId取出缓存，设置备份任务的COPY_FORMAT
            RMap<String, String> redis = redissonClient.getMap(task.getRequestId(), StringCodec.INSTANCE);
            redis.put(ContextConstants.COPY_FORMAT, String.valueOf(task.getCopyFormat()));
            // 添加LanFree配置
            setLanFreeConfig(task, backupObject.getSla());
            commonAgentService.supplyAgentCommonInfo(task.getAgents());

            URI uri = domainService.getUrlByAgents(AgentApiUtil.getAgentIds(task.getAgents()));
            log.info("backup uri:{}, task id:{}", JSONObject.stringify(uri), task.getRequestId());
            opServiceHelper.injectVpcInfo(task);
            log.info("start to send task to dme, requestId is {}", task.getRequestId());
            unifiedRestApi.createBackupTask(uri, task);
            log.info("Send backup task to data mover engine successful! requestId is {}", task.getRequestId());

            jobService.updateJob(task.getRequestId(), JobUpdateUtil.getDeliverReq());
        } finally {
            // 加在这里 清理 protectObject protectEnv auth(), repositories auth() extendAuth()
            cleanAuthPwd(task);
        }
        return task;
    }

    private void setLanFreeConfig(BackupTask task, Sla sla) {
        String subType = task.getProtectObject().getSubType();
        List<Endpoint> agents = task.getAgents();
        Map<String, String> fcMap = resourceService.getLanFreeConfig(subType, FibreUtil.getAgentIds(agents));
        task.getDataLayout().setClientProtocolType(FibreUtil.getClientProtocol(fcMap));
        task.addParameters(FibreUtil.getLanFreeAgents(fcMap));
        log.debug("backup task:{}, protocol Type:{}", task.getRequestId(),
                task.getDataLayout().getClientProtocolType());
        log.info("Start config sanClient, backup task: {}", task.getTaskId());
        String[] notConfiguredSanClientAgents = sanClientService.getAgentsNotConfiguredSanclient(agents);

        HashMap<String, String> sanClientConfigMap = new HashMap<>();
        // 备份任务代理主机未配置sanClient
        if (!VerifyUtil.isEmpty(notConfiguredSanClientAgents) && notConfiguredSanClientAgents.length == agents.size()) {
            sanClientConfigMap.put(sanClientService.IS_SANCLIENT, Boolean.FALSE.toString());
            task.addParameters(sanClientConfigMap);
            // 判断是否为切换sanClient状态场景，需要转全量副本
            switchSanClientCopy(task, false, sla);
            return;
        }

        // 备份任务代理主机，部分配置sanClient
        if (!VerifyUtil.isEmpty(notConfiguredSanClientAgents)) {
            throw new LegoCheckedException(CommonErrorCode.AIX_PARTIAL_ASSOCIATED_SANCLIENT_ERROR,
                    notConfiguredSanClientAgents, "Aix agents {0} not config sanClient");
        }
        // 备份任务代理主机，所有都配置了sanClient。下发sanClient备份任务
        sanClientConfigMap.put(sanClientService.IS_SANCLIENT, Boolean.TRUE.toString());
        sanClientConfigMap.put(sanClientService.ADVANCE_PARAMS_KEY_MULTI_POST_JOB, Boolean.TRUE.toString());
        task.addParameters(sanClientConfigMap);
        // 如果下发的是san任务，需要将协议类型改为DATA_TURBO
        task.getDataLayout().setClientProtocolType(ClientProtocolTypeEnum.DATA_TURBO.getClientProtocolType());
        agents.forEach(sanClientService::fillAgentParams);
        // 判断是否为切换sanClient状态场景，需要转全量副本
        switchSanClientCopy(task, true, sla);
    }

    private void checkBackupTask(BackupTask task) {
        List<Endpoint> onlineAgents = Optional.ofNullable(task.getAgents()).orElse(Collections.emptyList()).stream()
                .filter(agent -> isAgentOnline(agent.getId())).collect(Collectors.toList());
        log.info("Backup task: {}, find agent count: {}, online agent count: {}.", task.getTaskId(),
                task.getAgents().size(), onlineAgents.size());
        if (CollectionUtils.isEmpty(onlineAgents)) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "No agent found");
        }
        task.setAgents(onlineAgents);
    }

    // 如果插件没有返回uuid，则默认其在线
    private boolean isAgentOnline(String agentId) {
        if (VerifyUtil.isEmpty(agentId)) {
            log.warn("Agent uuid is empty, agentId: {}.", agentId);
            return true;
        }
        return resourceService.getBasicResourceById(false, agentId)
                .filter(resource -> resource instanceof ProtectedEnvironment)
                .map(resource -> (ProtectedEnvironment) resource)
                .filter(env -> LinkStatusEnum.ONLINE.getStatus().toString().equals(env.getLinkStatus())).isPresent();
    }

    private void cleanAuthPwd(BackupTask task) {
        if (task.getProtectEnv() != null && task.getProtectEnv().getAuth() != null) {
            StringUtil.clean(task.getProtectEnv().getAuth().getAuthPwd());
            // 清理OP服务化HCS Token
            if (task.getProtectEnv().getExtendInfo() != null) {
                StringUtil.clean(task.getProtectEnv().getExtendInfo().get(HCS_TOKEN));
            }
        }
        if (task.getProtectObject() != null && task.getProtectObject().getAuth() != null) {
            StringUtil.clean(task.getProtectObject().getAuth().getAuthPwd());
        }
        Optional.ofNullable(task.getRepositories()).orElse(new ArrayList<>()).forEach(this::cleanRepositoryAuthPwd);
    }

    private void cleanRepositoryAuthPwd(StorageRepository repository) {
        if (repository.getAuth() != null) {
            StringUtil.clean(repository.getAuth().getAuthPwd());
        }
        if (repository.getExtendAuth() != null) {
            StringUtil.clean(repository.getExtendAuth().getAuthPwd());
        }
    }

    private BackupTask buildBackupRequest(BackupObject backupObject) {
        BackupTask request = new BackupTask();
        request.setRequestId(backupObject.getRequestId());
        request.setTaskId(backupObject.getTaskId());
        request.setCopyId(backupObject.getRequestId()); // 副本ID和RequestID保持一致
        request.setBackupType(BackupTypeConstants.convert2DmeBackType(backupObject.getBackupType()));

        // 设置受保护的资源
        String resourceId = backupObject.getProtectedObject().getResourceId();
        ProtectedResource protectedResource = this.getProtectedResource(resourceId);
        TaskResource taskResource = new TaskResource();
        BeanUtils.copyProperties(protectedResource, taskResource);
        request.setProtectObject(taskResource);

        // 设置受保护的环境
        String envId = backupObject.getProtectedObject().getEnvUuid();
        ProtectedEnvironment env = protectedEnvironmentService.getEnvironmentById(envId);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(env, taskEnvironment);
        request.setProtectEnv(taskEnvironment);

        // 设置脚本，Qos、高级备份参数， 数据布局
        request.setScripts(extractScripts(backupObject));
        request.setQos(extractQos(backupObject).orElse(null));
        Map<String, String> parameters = extractBackupParameters(backupObject);

        request.setDataLayout(extractDataLayout(backupObject));
        if (request.getDataLayout().isSrcDeduption()) {
            parameters.put(AgentKeyConstant.CLUSTER_ESN, memberClusterService.getCurrentClusterEsn());
        }
        request.setAdvanceParams(parameters);
        // 设置保护代理
        List<Endpoint> endpointList = selectorManager.selectAgentByResource(protectedResource,
                JobTypeEnum.BACKUP.getValue(), parameters);
        request.setAgents(endpointList);

        buildDataRepository(backupObject, request);

        // 设置下次备份变更原因，如果有的话
        buildNextBackupCause(backupObject, request);

        // 抽取前后两次备份参数的差异
        extractChangedParameter(backupObject, request);

        return request;
    }

    private void buildDataRepository(BackupObject backupObject, BackupTask request) {
        /**
         * 1。 框架提供默认构造数据仓的方法
         * 2. 插件可以重写实现自己方法
         */
        StorageRepositoryProvider storageRepositoryProvider = providerManager
            .findProviderOrDefault(StorageRepositoryProvider.class, backupObject, defaultStorageRepositoryProvider);
        request.addAllRepository(storageRepositoryProvider.buildBackupDataRepository(backupObject));
    }

    private StorageInfoDto buildStorageInfo(BackupObject backupObject) {
        // 设置备份存储单元参数
        String jobId = backupObject.getRequestId();
        String storageUnitId = jobService.queryJob(jobId).getStorageUnitId();
        log.info("Current backup job linked storage unit id :{} ,job id :{}", storageUnitId, jobId);
        StorageUnitQueryParam queryParam = new StorageUnitQueryParam();
        if (VerifyUtil.isEmpty(storageUnitId)) { // 没有指定存储单元，取自动添加映射的第0号存储池
            String currentClusterEsn = memberClusterService.getCurrentClusterEsn();
            queryParam.setDeviceId(currentClusterEsn);
            queryParam.setIsAutoAdded(true);
        } else {
            // 如果指定了存储单元，取存储单元id，查T_STORAGE_UNIT表
            // 存储单元组的场景待适配
            queryParam.setId(storageUnitId);
        }
        PageListResponse<StorageUnitVo> storageUnitVoPageListResponse = storageUnitService
                .pageQueryStorageUnits(queryParam, 0, 1);
        StorageInfoDto storageInfoDto = new StorageInfoDto();
        storageInfoDto.setStorageDevice(storageUnitVoPageListResponse.getRecords().get(0).getDeviceId());
        log.info("storage_device_id:{}", storageUnitVoPageListResponse.getRecords().get(0).getDeviceId());
        storageInfoDto.setStoragePool(storageUnitVoPageListResponse.getRecords().get(0).getPoolId());
        log.info("storage_pool_id:{}", storageUnitVoPageListResponse.getRecords().get(0).getPoolId());
        return storageInfoDto;
    }


    private boolean isStorageUnit(BackupObject backupObject) {
        JsonNode storageInfo =
                backupObject.getPolicy().getExtParameters().get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY);
        if (VerifyUtil.isEmpty(storageInfo)) {
            return false;
        }
        JsonNode storageTypeNode = storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY);
        String storageType = (VerifyUtil.isEmpty(storageTypeNode) || VerifyUtil.isNone(storageTypeNode.asText())) ? ""
                : storageTypeNode.asText();
        return BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_VALUE.equals(storageType);
    }

    private boolean isDwsParallel(BackupObject backupObject) {
        // 是否为DWS并行存储
        JsonNode storageInfo =
                backupObject.getPolicy().getExtParameters().get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY);
        if (VerifyUtil.isEmpty(storageInfo)) {
            return false;
        }
        JsonNode storageIdNode = storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_ID_KEY);
        JsonNode storageTypeNode = storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY);
        String storageType = (VerifyUtil.isEmpty(storageTypeNode) || VerifyUtil.isNone(storageTypeNode.asText())) ? ""
                : storageTypeNode.asText();
        if (!BackupStorageTypeEnum.BACKUP_STORAGE_UNIT_GROUP.getValue().equals(storageType)) {
            return false;
        }
        String storageId = (VerifyUtil.isEmpty(storageIdNode) || VerifyUtil.isNone(storageIdNode.asText())) ? ""
                : storageIdNode.asText();
        return backupStorageApi.getDetail(storageId).isHasEnableParallelStorage();
    }

    private void buildNextBackupCause(BackupObject backupObject, BackupTask request) {
        ProtectedObject protectedObject = backupObject.getProtectedObject();
        String resourceId = protectedObject.getResourceId();
        NextBackupParams nextBackupParams = resourceService.queryNextBackupTypeAndCause(resourceId);
        String nextCauseLabel = nextBackupParams.getNextBackupChangeCause();
        if (!StringUtils.hasText(nextCauseLabel)) {
            nextCauseLabel = Optional.of(protectedObject).map(ProtectedObject::getExtParameters)
                    .map(e -> e.get(ResourceExtendInfoConstants.NEXT_BACKUP_CHANGE_CAUSE_EXT_KEY)).map(Object::toString)
                    .orElse(null);
        }
        log.debug("backup next cause label is: {}, request is: {}", nextCauseLabel, request.getRequestId());
        NextBackupChangeCauseEnum nextBackupChangeCauseEnum = EnumUtil.get(NextBackupChangeCauseEnum.class,
                NextBackupChangeCauseEnum::getLabel, nextCauseLabel, false, true);
        String nextCause = Optional.ofNullable(nextBackupChangeCauseEnum).map(e -> String.valueOf(e.getIndex()))
                .orElse(BackupConstant.BACKUP_EXT_PARAM_NEXT_CAUSE_DEFAULT_VALUE);
        request.addParameter(BackupConstant.BACKUP_EXT_PARAM_NEXT_CAUSE_KEY, nextCause);
    }

    private List<StorageRepository> buildStorageRepository(Policy policy, boolean isNeedAllRep) {
        if (policy == null) {
            return new ArrayList<>();
        }
        JsonNode node = policy.getExtParameters().get(CopyPropertiesKeyConstant.KEY_BACKUP_REPOSITORY_ID);

        String storageId = (node == null || StringUtils.isEmpty(node.asText())) ? "" : node.asText();
        if (!VerifyUtil.isEmpty(storageId)) {
            return taskRepositoryManager.buildTargetRepositories(storageId, policy.getType(), isNeedAllRep);
        }
        return buildStorageUnitGroupGRepositories(policy, isNeedAllRep);
    }

    private List<StorageRepository> buildStorageUnitGroupGRepositories(Policy policy, boolean isNeedAllRep) {
        JsonNode storageInfo = policy.getExtParameters().get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY);
        if (VerifyUtil.isEmpty(storageInfo)) {
            return Lists.newArrayList();
        }
        JsonNode storageTypeNode = storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY);
        JsonNode storageIdNode = storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_ID_KEY);
        String storageType = (VerifyUtil.isEmpty(storageTypeNode) || VerifyUtil.isNone(storageTypeNode.asText()))
                ? ""
                : storageTypeNode.asText();
        String storageId = (VerifyUtil.isEmpty(storageIdNode) || VerifyUtil.isNone(storageIdNode.asText()))
                ? ""
                : storageIdNode.asText();
        return taskRepositoryManager.buildExternalRepositories(storageType, storageId, isNeedAllRep);
    }

    private void extractChangedParameter(BackupObject backupObject, BackupTask backupTask) {
        Copy copy = copyRestApi.queryLatestBackupCopy(backupObject.getProtectedObject().getResourceId(), null, null);
        // 首次备份没有副本
        if (copy == null) {
            log.info("Last copy is not exists!");
            return;
        }

        // 当前备份参数
        Map<String, String> curBackupParameter = new HashMap<>();
        Policy policy = backupObject.getPolicy();
        if (policy == null) {
            return;
        }
        extractCurrentBackupParameters(curBackupParameter, policy);

        // 上一次备份参数
        Map<String, String> lastBackupParameter = extractLastBackupParameter(backupObject, copy);

        // 合并变化的备份参数
        mergeChangedParameters(backupTask, curBackupParameter, lastBackupParameter);
    }

    private void extractCurrentBackupParameters(Map<String, String> curBackupParameter, Policy policy) {
        JsonNode jsonNode = policy.getExtParameters();
        jsonNode.fieldNames().forEachRemaining(fieldName -> {
            String fileValue = jsonNode.get(fieldName).asText();
            if (jsonNode.get(fieldName) != null && StringUtils.hasText(fileValue)) {
                curBackupParameter.put(fieldName, fileValue);
            }
        });
    }

    private Map<String, String> extractLastBackupParameter(BackupObject backupObject, Copy copy) {
        Map<String, String> lastBackupParameter = new HashMap<>();
        String slaJsonString = copy.getSlaProperties();
        SlaBo slaBo = JSONObject.fromObject(slaJsonString).toBean(SlaBo.class);
        String backupType = backupObject.getBackupType();
        PolicyBo lastPolicy = Optional.ofNullable(slaBo.getPolicyList()).orElse(Collections.emptyList()).stream()
                .filter(policy -> "backup".equals(policy.getType()) && backupType.equals(policy.getAction()))
                .findFirst().orElse(null);
        if (lastPolicy == null) {
            return lastBackupParameter;
        }
        JsonNode jsonNode = lastPolicy.getExtParameters();
        jsonNode.fieldNames().forEachRemaining(fieldName -> {
            String fieldValue = jsonNode.get(fieldName).asText();
            if (jsonNode.get(fieldName) != null && StringUtils.hasText(fieldValue)) {
                lastBackupParameter.put(fieldName, fieldValue);
            }
        });
        return lastBackupParameter;
    }

    /**
     * 合并备份参数，提取出变化的参数
     *
     * @param backupTask 本次备份任务
     * @param current 当前备份参数
     * @param last 上一次备份参数
     */
    private void mergeChangedParameters(BackupTask backupTask, Map<String, String> current, Map<String, String> last) {
        // 处理当前的备份参数
        current.forEach((key, value) -> {
            if (last.containsKey(key) && !last.get(key).equals(value)) {
                backupTask.addChangedParameter(key, new String[]{last.get(key), value});
            } else {
                backupTask.addChangedParameter(key, new String[]{null, value});
            }
        });

        // 处理上一次备份参数
        last.forEach((key, value) -> {
            // 上次有，本次没有
            if (!current.containsKey(key)) {
                backupTask.addChangedParameter(key, new String[]{last.get(key), null});
            } else {
                backupTask.addChangedParameter(key, new String[]{value, null});
            }
        });
    }

    private ProtectedResource getProtectedResource(String resourceId) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(resourceId);
        if (resOptional.isPresent()) {
            return resOptional.get();
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected Resource is not exists!");
    }

    /**
     * 从备份策略中中抽取Qos信息
     *
     * @param backupObject 备份对象{@link BackupObject}
     * @return Qos
     */
    private Optional<Qos> extractQos(BackupObject backupObject) {
        Policy policy = backupObject.getPolicy();
        if (policy == null) {
            return Optional.empty();
        }

        JsonNode jsonNode = policy.getExtParameters().get(PredefineBackupParameters.QOS_ID.getKey());
        if (jsonNode == null || StringUtils.isEmpty(jsonNode.asText())) {
            return Optional.empty();
        }
        String qosId = jsonNode.asText();
        QosBo qosBo = qosCommonRestApi.queryQos(qosId);
        if (qosBo == null) {
            log.info("Task id: {}, backup parameter is null", backupObject.getTaskId());
            return Optional.empty();
        }

        Qos qos = new Qos();
        qos.setBandwidth(qosBo.getSpeedLimit());
        log.info("Task id: {}, set qos, backup parameters is: {}", backupObject.getTaskId(),
                JSONObject.fromObject(qos));
        return Optional.of(qos);
    }

    /**
     * 保护对象的扩展参数中抽取备份的脚本参数
     *
     * @param backupObject 备份对象{@link BackupObject}
     * @return 备份脚本
     */
    private BackupScript extractScripts(BackupObject backupObject) {
        BackupScript script = new BackupScript();
        JSONObject ext = JSONObject.fromObject(backupObject.getProtectedObject().getExtParameters());
        script.setPreScript(ext.getString(PredefineBackupParameters.PRE_SCRIPT.getKey()));
        script.setPostScript(ext.getString(PredefineBackupParameters.POST_SCRIPT.getKey()));
        script.setFailPostScript(ext.getString(PredefineBackupParameters.FAILED_SCRIPT.getKey()));
        log.info("Backup scripts is : {}", JSONObject.fromObject(script));
        return script;
    }

    /**
     * 从保护对象和策略中抽取备份高级参数
     *
     * @param backupObject 备份对象{@link BackupObject}
     * @return 备份参数
     */
    private Map<String, String> extractBackupParameters(BackupObject backupObject) {
        Map<String, String> params = new HashMap<>();

        // 将保护对象中的扩展参数抽取出来
        ProtectedObject protectedObject = backupObject.getProtectedObject();
        Map ext = protectedObject.getExtParameters();
        if (ext != null) {
            extractParameter(params, ext);
        }

        // 将策略中扩展参数抽取出来
        Policy policy = backupObject.getPolicy();
        if (policy != null) {
            extractParameter(params, policy);
        }
        log.info("Backup taskId: {}, advance params: {}.", backupObject.getTaskId(), JSONObject.fromObject(params));
        JobBo jobBo = jobService.queryJob(backupObject.getTaskId());
        if (jobBo != null && !VerifyUtil.isEmpty(jobBo.getUserId())) {
            UserInnerResponse userInnerResponse = userService.getUserInfoByUserId(jobBo.getUserId());
            params.put(AgentKeyConstant.USER_INFO, JSONObject.writeValueAsString(userInnerResponse));
        }
        return params;
    }

    private void extractParameter(Map<String, String> params, Map ext) {
        ext.forEach((key, value) -> {
            if (!EXCLUDE_KEYS.contains(key) && key != null && value != null) {
                params.put(key.toString(), value.toString());
            }
        });
    }

    private void extractParameter(Map<String, String> params, Policy policy) {
        JsonNode jsonNode = policy.getExtParameters();
        jsonNode.fieldNames().forEachRemaining(fieldName -> {
            if (!EXCLUDE_KEYS.contains(fieldName) && jsonNode.get(fieldName) != null) {
                JsonNode itemsNode = jsonNode.get(fieldName);
                if (itemsNode.isContainerNode()) {
                    itemsNode.fieldNames().forEachRemaining(itemName -> {
                        if (itemsNode.get(itemName) != null) {
                            params.put(itemName, itemsNode.get(itemName).asText());
                        }
                    });
                } else {
                    params.put(fieldName, jsonNode.get(fieldName).asText());
                }
            }
        });
    }

    private DataLayout extractDataLayout(BackupObject backupObject) {
        DataLayout dataLayout = new DataLayout();
        Policy policy = backupObject.getPolicy();
        if (policy == null) {
            return new DataLayout();
        }
        dataLayout.setSrcEncryption(
                getBooleanValueFromExtParameters(policy,
                    PredefineBackupParameters.ENCRYPTION, false));
        dataLayout.setDstDeduption(
                getBooleanValueFromExtParameters(policy,
                    PredefineBackupParameters.DEDUPLICATION, true));
        dataLayout.setSrcDeduption(
                getBooleanValueFromExtParameters(policy,
                    PredefineBackupParameters.SOURCE_DEDUPLICATION, false));
        dataLayout.setDstCompression(
            getBooleanValueFromExtParameters(policy,
                PredefineBackupParameters.ENABLE_DEDUPTION_COMPRESSION, true));
        // 设置备份链路加密
        dataLayout.setLinkEncryption(getSwitchByName(SwitchNameEnum.BACKUP_LINK_ENCRYPTION));
        String dataLayoutStr = getDataLayoutStr(dataLayout);
        log.info("DataLayout is : {}", dataLayoutStr);
        return dataLayout;
    }

    private String getDataLayoutStr(DataLayout dataLayout) {
        Map<String, Object> datalLayoutMap = JSONObject.toMap(JSONObject.fromObject(dataLayout), Object.class);
        StringBuilder dataLayoutStr = new StringBuilder();
        for (Map.Entry<String, Object> entry : datalLayoutMap.entrySet()) {
            dataLayoutStr.append(entry.getKey() + ":" + entry.getValue() + ";");
        }
        return dataLayoutStr.toString();
    }

    private boolean getBooleanValueFromExtParameters(Policy policy, PredefineBackupParameters param,
            boolean shouldBeWhenDefault) {
        final JsonNode extParameters = policy.getExtParameters();
        return Optional.ofNullable(extParameters.get(param.getKey())).map(JsonNode::asBoolean)
                .orElse(shouldBeWhenDefault);
    }

    private boolean getSwitchByName(SwitchNameEnum switchNameEnum) {
        return SwitchStatusEnum.ON.equals(systemSwitchInternalService.queryByName(switchNameEnum).getStatus());
    }

    /**
     * 创建默认的数据存储库
     *
     * @param backupObject 备份对象
     * @return 默认的数据存储库
     */
    private StorageRepository buildDefaultDataRepository(BackupObject backupObject) {
        Policy policy = backupObject.getPolicy();
        String storageId = null;
        if (policy == null) {
            log.debug("Unset backup storage, local backup storage will be used. ");
            return buildStorageRepository(storageId, RepositoryProtocolEnum.NATIVE_NFS, backupObject);
        }

        JsonNode node = policy.getExtParameters().get("storageId");
        log.debug("Backup storage repository id is {}.", storageId);

        // 未设置存储ID，则为备份存储库为本地A8000存储
        if (node == null || StringUtils.isEmpty(node.asText())) {
            log.debug("Unset backup storage, local backup storage will be used. ");
            return buildStorageRepository(storageId, RepositoryProtocolEnum.NATIVE_NFS, backupObject);
        }
        storageId = node.asText();
        JsonNode storageTypeNode = policy.getExtParameters().get("storageType");
        log.info("Backup storage repository id is {} and storage type is {}", storageId, storageTypeNode);

        // 设置了存储库的类型，默认使用S3
        int storageType = storageTypeNode.asInt(RepositoryProtocolEnum.S3.getProtocol());
        return buildStorageRepository(storageId, RepositoryProtocolEnum.getByProtocol(storageType), backupObject);
    }

    private StorageRepository buildStorageRepository(String storageId, RepositoryProtocolEnum protocol,
        BackupObject backupObject) {
        StorageRepository strategyRepository = taskRepositoryManager.buildStorageRepository(storageId, protocol,
            buildStorageInfo(backupObject));
        strategyRepository.setLocalCluster(true);
        return strategyRepository;
    }

    private void switchSanClientCopy(BackupTask task, boolean isSanClient, Sla sla) {
        log.info("Check switch sanClient copy, backupType:{}, isSanClient: {}, taskId: {}", task.getBackupType(),
                isSanClient, task.getTaskId());
        // 本次备份为全量则直接返回
        if (BackupTypeConstants.DME_BACKUP_FULL.equals(task.getBackupType())) {
            log.info("This is full backup, taskId: {}", task.getTaskId());
            return;
        }
        // 根据上次副本是否为sanClient判断本次是否需要转全量
        Copy copy = copyRestApi.queryLatestBackupCopy(task.getProtectObject().getUuid(), null, null);
        if (copy == null) {
            log.info("Latest copy is null, taskId: {}", task.getTaskId());
            return;
        }
        if (CopyUtil.checkIsSanCopy(copy) == isSanClient) {
            log.info("Same sanclient status, isSanClient: {}, taskId: {}", isSanClient, task.getTaskId());
            return;
        }

        log.info("Start switch sanClient copy, last copy is sanClient: {}, taskId: {}", !isSanClient, task.getTaskId());
        // 任务类型转为全量
        task.setBackupType(BackupTypeConstants.DME_BACKUP_FULL);
        // sla策略改为全量策略
        List<Policy> policies = sla.getPolicyList();
        for (Policy policy : policies) {
            if (BackupTypeConstants.FULL.getBackupType().equals(policy.getAction())) {
                // 根据requestId(copyId)取出缓存的保护对象信息，更新policy为全量policy
                RMap<String, String> redis = redissonClient.getMap(task.getCopyId(), StringCodec.INSTANCE);
                String s = JSONObject.fromObject(policy).toString();
                redis.put("policy", s);
                log.info("Set policy in redis to: {}, taskId: {}", policy.getUuid(), task.getTaskId());
                break;
            }
        }
        // 上报任务label
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        JobLogBo jobLog = new JobLogBo();
        jobLog.setJobId(task.getRequestId());
        jobLog.setStartTime(System.currentTimeMillis());
        jobLog.setLevel(JobLogLevelEnum.INFO.getValue());
        if (isSanClient) {
            // 普通备份切换为SANClient备份，转为全量备份
            jobLog.setLogInfo(SANCLIENT_ENABLED.getLabel());
        } else {
            // SANClient备份切换为普通备份，转为全量备份
            jobLog.setLogInfo(SANCLIENT_DISABLED.getLabel());
        }
        updateJobRequest.setJobLogs(Collections.singletonList(jobLog));
        jobService.updateJob(task.getRequestId(), updateJobRequest);
    }
}
