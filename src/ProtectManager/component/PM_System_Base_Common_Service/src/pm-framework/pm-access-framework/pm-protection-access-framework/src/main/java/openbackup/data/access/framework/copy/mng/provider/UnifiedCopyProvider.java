/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.copy.mng.provider;

import com.huawei.oceanprotect.job.sdk.JobService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TaskParamConstants;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.util.FibreUtil;
import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyDeleteInterceptor;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyProvider;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.AgentApiUtil;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.util.JobUpdateUtil;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.AvailableAgentManagementDomainService;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.util.StringUtils;

import java.net.URI;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 统一备份框架副本Provider
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-15
 */
@Slf4j
@Component("unifiedCopyProvider")
public class UnifiedCopyProvider implements CopyProvider {
    private final DmeUnifiedRestApi restApi;

    private final RepositoryStrategyManager strategyManager;

    private final RedissonClient redissonClient;

    private final CopyManagerService copyManagerService;

    private ProviderManager providerManager;

    private DefaultCopyDeleteInterceptor defaultCopyDeleteInterceptor;

    private ResourceService resourceService;

    private JobService jobService;

    private AvailableAgentManagementDomainService domainService;

    private SanClientService sanClientService;

    private CommonAgentService commonAgentService;

    /**
     * 构造函数
     *
     * @param restApi DME 统一备份REST API{@link DmeUnifiedRestApi}
     * @param strategyManager 存储库策略管理器{@link RepositoryStrategyManager}
     * @param copyManagerService 副本管理{@link CopyManagerService}
     * @param redissonClient redissonClient
     * @param resourceService resourceService
     */
    public UnifiedCopyProvider(DmeUnifiedRestApi restApi, RepositoryStrategyManager strategyManager,
        RedissonClient redissonClient, CopyManagerService copyManagerService, ResourceService resourceService) {
        this.restApi = restApi;
        this.strategyManager = strategyManager;
        this.redissonClient = redissonClient;
        this.copyManagerService = copyManagerService;
        this.resourceService = resourceService;
    }

    @Autowired
    public void setProviderManager(ProviderManager providerManager) {
        this.providerManager = providerManager;
    }

    @Autowired
    public void setDefaultCopyDeleteInterceptor(DefaultCopyDeleteInterceptor defaultCopyDeleteInterceptor) {
        this.defaultCopyDeleteInterceptor = defaultCopyDeleteInterceptor;
    }

    @Autowired
    public void setJobService(JobService jobService) {
        this.jobService = jobService;
    }

    @Autowired
    public void setAvailableAgentManagementDomainService(AvailableAgentManagementDomainService domainService) {
        this.domainService = domainService;
    }

    @Autowired
    public void setSanClientService(SanClientService sanClientService) {
        this.sanClientService = sanClientService;
    }

    @Autowired
    public void setCommonAgentService(CommonAgentService commonAgentService) {
        this.commonAgentService = commonAgentService;
    }

    @ExterAttack
    @Override
    public void deleteCopy(String requestId, CopyInfoBo copy) {
        DeleteCopyTask task = new DeleteCopyTask();
        task.setTaskId(requestId);
        task.setRequestId(requestId);
        task.setCopyId(copy.getUuid());

        // force delete
        RMap<String, String> jobMap = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String isForcedFromRedis = jobMap.get(ContextConstants.IS_FORCED);
        log.debug("requestId: {}, is forced param from redis value is {}.", requestId, isForcedFromRedis);
        task.setIsForceDeleted(Boolean.valueOf(isForcedFromRedis));

        String isDeleteDataFromRedis = jobMap.get(ContextConstants.IS_DELETE_DATA);
        log.debug("requestId: {}, is delete data param from redis value is {}.", requestId, isDeleteDataFromRedis);
        task.setIsDeleteData(Boolean.valueOf(isDeleteDataFromRedis));
        Copy newCopy = new Copy();
        BeanUtils.copyProperties(copy, newCopy);
        task.setProtectObject(copyManagerService.buildTaskResource(newCopy));
        try {
            task.setProtectEnv(copyManagerService.buildTaskEnvironment(task.getProtectObject().getRootUuid()));
        } catch (LegoCheckedException e) {
            log.warn("requestId: {}, copy delete task environment is not exist.", requestId);
            task.setProtectEnv(null);
        }
        String copyProperties = copy.getProperties();
        JSONObject copyJsonProperties = JSONObject.fromObject(copyProperties);
        JSONArray repositories = copyJsonProperties.getJSONArray(CopyPropertiesKeyConstant.KEY_REPOSITORIES);
        log.debug("requestId: {},Backup storage repository.", requestId);
        List<BaseStorageRepository> repositoryList = JSONArray.toCollection(repositories, BaseStorageRepository.class);
        task.setRepositories(repositoryList.stream()
            .map(this::toStorageRepository)
            .filter(Objects::nonNull)
            .collect(Collectors.toList()));
        try {
            log.debug("requestId: {},Delete copy params.", requestId);
            CopyDeleteInterceptor copyDeleteInterceptor = providerManager.findProviderOrDefault(
                CopyDeleteInterceptor.class, copy.getResourceSubType(), defaultCopyDeleteInterceptor);
            if (copyDeleteInterceptor != null) {
                log.debug("copy delete interceptor for {} is provided, and the interceptor will be used! requestId: {}",
                    copy.getResourceSubType(), requestId);
                copyDeleteInterceptor.initialize(task, copy);
            }
            setAgentMountType(task, copyDeleteInterceptor, copy);
            // 设置lanfree
            setLanFreeConfig(task, copy.getResourceSubType());
            // 下发任务到dme
            sendJobToDme(requestId, copy, task);
            jobService.updateJob(requestId, JobUpdateUtil.getDeliverReq());
        } catch (LegoUncheckedException | LegoCheckedException e) {
            log.error("Delete copy failed! copyId: {}, requestId: {}", copy.getUuid(), requestId);
            throw e;
        } finally {
            task.getRepositories().forEach(StorageRepository::cleanAuth);
        }
    }

    private void setAgentMountType(DeleteCopyTask task, CopyDeleteInterceptor interceptor, CopyInfoBo copy) {
        if (interceptor != null) {
            Optional<AgentMountTypeEnum> mountTypeOp = interceptor.getMountType(task);
            if (mountTypeOp.isPresent()) {
                task.getAdvanceParams().put(TaskParamConstants.AGENT_MOUNT_TYPE, mountTypeOp.get().getValue());
                return;
            }
        }
        task.getAdvanceParams()
            .put(TaskParamConstants.AGENT_MOUNT_TYPE,
                commonAgentService.getJobAgentMountTypeByUnitId(copy.getStorageUnitId()).getValue());
    }

    private void sendJobToDme(String requestId, CopyInfoBo copy, DeleteCopyTask task) {
        if (VerifyUtil.isEmpty(task.getAgents())) {
            log.info("start to send copy delete task to dme, requestId {}, copyId {}", requestId, copy.getUuid());
            restApi.deleteCopy(copy.getUuid(), task);
            log.info("send copy delete task successfully, requestId is {}, copyId is {}", requestId, copy.getUuid());
            return ;
        }
        // 有agent，通过agent id获取能通的dme uri，然后下发任务
        URI uri = domainService.getUrlByAgents(AgentApiUtil.getAgentIds(task.getAgents()));
        log.info("copy delete uri:{}, task id:{}", JSONObject.stringify(uri), task.getRequestId());

        log.info("send copy delete task to dme, requestId is {}, copyId is {}", requestId, copy.getUuid());
        restApi.deleteCopyWithUri(uri, copy.getUuid(), task);
        log.info("send copy delete task successfully, requestId {}, copyId {}", requestId, copy.getUuid());
    }

    private void setLanFreeConfig(DeleteCopyTask task, String subType) {
        Map<String, String> fcMap = resourceService.getLanFreeConfig(subType, FibreUtil.getAgentIds(task.getAgents()));
        task.getDataLayout().setClientProtocolType(FibreUtil.getClientProtocol(fcMap));
        task.addParameters(FibreUtil.getLanFreeAgents(fcMap));
        log.debug("copy delete task:{}, protocol Type:{}", task.getRequestId(),
            task.getDataLayout().getClientProtocolType());
        // 从副本中解析是否是san，若是则添加san标记
        log.info("Start config san, taskID is {}", task.getTaskId());
        sanClientService.configCopyDeleteAgent(task.getCopyId(), task.getAgents());
        sanClientService.configSanClient(task.getCopyId(), task.getAgents(), task.getAdvanceParams(),
            task.getDataLayout());
    }

    private StorageRepository toStorageRepository(BaseStorageRepository baseStorageRepository) {
        // 本地存储不需处理
        String storageId = baseStorageRepository.getId();
        if (StringUtils.isEmpty(storageId)) {
            log.debug("Backup storage repository is local.");
            return null;
        }

        // 外部存储库，必须要获取存储库的信息
        RepositoryProtocolEnum protocolEnum = null;
        RepositoryTypeEnum typeEnum = null;
        Integer protocol = baseStorageRepository.getProtocol();
        Integer type = baseStorageRepository.getType();
        try {
            protocolEnum = RepositoryProtocolEnum.getByProtocol(protocol);
            typeEnum = RepositoryTypeEnum.getByType(type);
        } catch (IllegalArgumentException e) {
            log.error("Backup storage protocol:{} or type:{} error!", protocol, type);
            throw new LegoCheckedException("Backup storage protocol or type error!");
        }
        log.debug("Backup storage protocol is {}, and type is {}", protocol, type);
        baseStorageRepository.setType(typeEnum.getType());
        baseStorageRepository.setId(storageId);
        return strategyManager.getStrategy(protocolEnum).getRepository(baseStorageRepository);
    }

    @Override
    public boolean applicable(CopyBo object) {
        return false;
    }
}
