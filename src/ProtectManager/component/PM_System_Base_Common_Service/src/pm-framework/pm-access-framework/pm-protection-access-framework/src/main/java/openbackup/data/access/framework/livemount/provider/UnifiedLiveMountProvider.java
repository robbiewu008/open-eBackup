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
package openbackup.data.access.framework.livemount.provider;

import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dee.DeeLiveMountRestApi;
import openbackup.data.access.client.sdk.api.framework.dee.model.OcLiveMountFsShareInfo;
import openbackup.data.access.client.sdk.api.framework.dee.model.OcLiveMountFsShareReq;
import openbackup.data.access.client.sdk.api.framework.dme.DmeBackupClone;
import openbackup.data.access.client.sdk.api.framework.dme.DmeMountQos;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedService;
import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.access.framework.agent.ProtectAgentSelector;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.livemount.common.constants.LiveMountConstants;
import openbackup.data.access.framework.livemount.common.enums.OperationEnums;
import openbackup.data.access.framework.livemount.common.model.LiveMountCreateCheckParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountExecuteParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.access.framework.livemount.common.model.LiveMountMigrateParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.common.model.LiveMountRefreshParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountUnmountParam;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.data.access.framework.livemount.service.impl.PerformanceValidator;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.access.framework.protection.common.util.FibreUtil;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.protection.service.repository.strategies.RepositoryStrategy;
import openbackup.data.access.framework.servitization.util.OpServiceHelper;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.anti.ransomware.CopyRansomwareService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.exception.NotImplementedException;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountModifyParam;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountPerformance;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountRemoveQosParam;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.AgentApiUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import openbackup.system.base.sdk.job.util.JobUpdateUtil;
import openbackup.system.base.sdk.livemount.model.Performance;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.AvailableAgentManagementDomainService;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.util.Assert;

import java.net.URI;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * Unified Live Mount Provider
 *
 */
@Slf4j
@Component
public class UnifiedLiveMountProvider extends AbstractLiveMountProvider {
    private static final String FILE_SYSTEM_NAME_PREFIX = "mount_";

    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    private final List<LiveMountInterceptorProvider> liveMountInterceptorProviders;

    private final ProviderManager providerManager;

    private final ResourceService resourceService;

    private final ProtectAgentSelector defaultSelector;

    private final RepositoryStrategyManager repositoryStrategyManager;

    private final PerformanceValidator performanceValidator;

    private final JobService jobService;

    private final UserService userService;

    private final DefaultLiveMountServiceProvider defaultLiveMountServiceProvider;

    private AvailableAgentManagementDomainService domainService;

    private DmeUnifiedService dmeUnifiedService;

    private OpServiceHelper opServiceHelper;

    private CommonAgentService commonAgentService;

    @Autowired
    private CopyService copyService;

    @Autowired
    private LiveMountEntityDao liveMountEntityDao;

    @Autowired
    private DeeLiveMountRestApi deeLiveMountRestApi;

    @Autowired
    private CopyRansomwareService copyRansomwareService;

    public UnifiedLiveMountProvider(DmeUnifiedRestApi dmeUnifiedRestApi,
            List<LiveMountInterceptorProvider> liveMountInterceptorProviders, ProviderManager providerManager,
            ResourceService resourceService, RepositoryStrategyManager repositoryStrategyManager,
            PerformanceValidator performanceValidator, JobService jobService, UserService userService,
            DefaultLiveMountServiceProvider defaultLiveMountServiceProvider,
            DefaultProtectAgentSelector defaultSelector) {
        this.dmeUnifiedRestApi = dmeUnifiedRestApi;
        this.liveMountInterceptorProviders = liveMountInterceptorProviders;
        this.providerManager = providerManager;
        this.resourceService = resourceService;
        this.defaultSelector = defaultSelector;
        this.repositoryStrategyManager = repositoryStrategyManager;
        this.performanceValidator = performanceValidator;
        this.jobService = jobService;
        this.userService = userService;
        this.defaultLiveMountServiceProvider = defaultLiveMountServiceProvider;
    }

    @Autowired
    public void setCommonAgentService(CommonAgentService commonAgentService) {
        this.commonAgentService = commonAgentService;
    }

    @Autowired
    public void setAvailableAgentManagementDomainService(AvailableAgentManagementDomainService domainService) {
        this.domainService = domainService;
    }

    @Autowired
    public void setDmeUnifiedService(DmeUnifiedService dmeUnifiedService) {
        this.dmeUnifiedService = dmeUnifiedService;
    }

    @Autowired
    public void setOpServiceHelper(OpServiceHelper opServiceHelper) {
        this.opServiceHelper = opServiceHelper;
    }

    /**
     * clone backup
     *
     * @param cloneCopyParam backup clone param
     */
    @Override
    protected void cloneBackup(CloneCopyParam cloneCopyParam) {
        LiveMountServiceProvider serviceProvider = providerManager.findProviderOrDefault(LiveMountServiceProvider.class,
                cloneCopyParam.getResourceSubType(), defaultLiveMountServiceProvider);
        DmeBackupClone clone = serviceProvider.buildDmeCloneCopyRequest(cloneCopyParam);
        if (deployTypeService.isCyberEngine()) {
            clone.setRepositories(cloneCopyParam.getRepositories());
            clone.setSnapShotName(cloneCopyParam.getBackupId());
        }
        dmeUnifiedRestApi.cloneBackup(clone);

        LiveMountPerformance performance = cloneCopyParam.getPerformance();
        if (!deployTypeService.isCyberEngine() && performance != null) {
            // 修改副本QoS
            modifyQos(cloneCopyParam.getCloneBackupId(), performance);
        }
    }

    /**
     * remove live mount qos
     *
     * @param param param
     */
    @Override
    protected void removeLiveMountQos(LiveMountRemoveQosParam param) {
        dmeUnifiedRestApi.deleteQos(param.getBackupId());
    }

    /**
     * modify live mount qos
     *
     * @param param param
     */
    @Override
    protected void modifyLiveMountQos(LiveMountModifyParam param) {
        modifyQos(param.getBackupId(), param.getPerformance());
    }

    private void modifyQos(String copyId, LiveMountPerformance performance) {
        dmeUnifiedRestApi.modifyQos(copyId, BeanTools.copy(performance, DmeMountQos::new));
    }

    @Override
    public void createLiveMountPreCheck(LiveMountCreateCheckParam liveMountCreateCheckParam) {
        CopyResourceSummary resource = liveMountCreateCheckParam.getResource();
        LiveMountInterceptorProvider provider = getLiveMountInterceptorProvider(resource.getResourceSubType());
        checkPerformance(liveMountCreateCheckParam.getLiveMountObject().getParameters());
        if (CollectionUtils.isEmpty(liveMountCreateCheckParam.getTargetResources())) {
            if (OperationEnums.MODIFY.equals(liveMountCreateCheckParam.getOperationEnums())) {
                checkPerformance(liveMountCreateCheckParam.getLiveMountObject().getParameters());
                return;
            }
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Target resource is empty.");
        }
        if (!provider.isSupportRepeatable()) {
            createLiveMountDuplicatedCheck(liveMountCreateCheckParam.getLiveMountObject());
        }
        if (provider.isSupportCheckCopyOperation()) {
            if (Objects.equals(liveMountCreateCheckParam.getOperationEnums(), OperationEnums.CREATE)) {
                copyRansomwareService.checkCopyOperationValid(liveMountCreateCheckParam.getCopy().getUuid(),
                    "LIVE_MOUNT");
            }
        }
    }

    private void createLiveMountDuplicatedCheck(LiveMountObject liveMountObject) {
        String resourceId = liveMountObject.getSourceResourceId();
        CopyResourceSummary resource = copyService.queryCopyResourceSummary(resourceId);
        LiveMountInterceptorProvider provider = getLiveMountInterceptorProvider(resource.getResourceSubType());
        LambdaQueryWrapper<LiveMountEntity> wrapper = new LambdaQueryWrapper<LiveMountEntity>().eq(
                LiveMountEntity::getResourceId, resourceId)
            .in(LiveMountEntity::getTargetResourceId, liveMountObject.getTargetResourceUuidList());
        int count = liveMountEntityDao.selectCount(wrapper).intValue();
        if (count >= provider.getLiveMountNumLimit()) {
            throw new LegoCheckedException(CommonErrorCode.DUPLICATE_MOUNT_ERROR,
                "not allow create duplicated live mount");
        }
    }

    /**
     * check performance params
     *
     * @param params performance params
     */
    @Override
    public void checkPerformance(Map<String, Object> params) {
        Map<String, Object> performanceParams = JSONObject.fromObject(params.get(PERFORMANCE)).toMap(Object.class);
        Performance performance = performanceValidator.loadPerformance(performanceParams);

        // 过滤掉无效和0的输入
        JSONObject performanceJsonObject = JSONObject.fromObject(performance);
        Map<String, Object> performanceParamsMap = JSONObject.toMap(performanceJsonObject, Object.class);
        if (performanceParams.size() != performanceParamsMap.size()) {
            throw new LegoCheckedException(ErrorCodeConstant.ERR_PARAM, "Performance parameters is invalid.");
        }

        if (performance != null) {
            performanceValidator.validatePerformance(performance);
        }
    }

    /**
     * execute live mount
     *
     * @param liveMountExecuteParam live mount execute param
     */
    @Override
    public void executeLiveMount(LiveMountExecuteParam liveMountExecuteParam) {
        LiveMountCreateTask task = new LiveMountCreateTask();
        try {
            // 初始化框架通用参数
            buildGeneralData(task, liveMountExecuteParam);
            // 初始化特性参数
            String subType = liveMountExecuteParam.getLiveMount().getResourceSubType();
            LiveMountInterceptorProvider provider = providerManager.findProvider(LiveMountInterceptorProvider.class,
                    subType);
            provider.initialize(task);

            log.info("Call dme exec live mount rest api begin, jobId: {}.", task.getRequestId());
            if (deployTypeService.isCyberEngine()) {
                task.getTargetObject().setSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
                task.getTargetEnv().setSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
                // 安全一体机适配共享路径恢复：调用dee内部接口创建共享，共享创建成功后更新任务 deeLiveMountRestApi
                OcLiveMountFsShareReq liveMountFsShareReq = buildOcLiveMountFsShareReq(liveMountExecuteParam);
                log.info("start to create fsShare task[{}].", liveMountFsShareReq.getTaskId());
                deeLiveMountRestApi.createFilesystemShare(liveMountFsShareReq);
                log.info("task[{}] create filesystem share success.", liveMountFsShareReq.getTaskId());
                jobService.updateJob(task.getRequestId(), JobUpdateUtil.getDeliverReq());
                return;
            } else {
                // 设置LanFree
                setMountLanfree(task, subType);
                opServiceHelper.injectVpcInfoForLiveMount(task);
            }

            commonAgentService.supplyAgentCommonInfo(task.getAgents());
            URI uri = domainService.getUrlByAgents(AgentApiUtil.getAgentIds(task.getAgents()));
            log.info("live mount uri:{}, task id:{}", JSONObject.stringify(uri), task.getRequestId());

            log.info("start to send live mount task to dme, requestId is {}", task.getRequestId());
            dmeUnifiedRestApi.createLiveMount(uri, task);
            log.info("Call dme exec live mount rest api success, jobId: {}.", task.getRequestId());

            jobService.updateJob(task.getRequestId(), JobUpdateUtil.getDeliverReq());
        } finally {
            // 清理 targetObject targetEnv auth(), repositories auth() extendAuth()
            task.cleanBaseTaskAuthPwd();
        }
    }

    private OcLiveMountFsShareReq buildOcLiveMountFsShareReq(LiveMountExecuteParam liveMountExecuteParam) {
        OcLiveMountFsShareReq liveMountFsShareReq = new OcLiveMountFsShareReq();
        liveMountFsShareReq.setRequestId(liveMountExecuteParam.getRequestId());
        liveMountFsShareReq.setTaskId(liveMountExecuteParam.getRequestId());
        liveMountFsShareReq.setLiveMountFsShareInfos(Collections.emptyList());
        LiveMountEntity liveMountEntity = liveMountExecuteParam.getLiveMount();
        if (liveMountEntity != null && liveMountEntity.getFileSystemShareInfo() != null) {
            List<OcLiveMountFsShareInfo> fsShareList = JSONArray.fromObject(
                liveMountEntity.getFileSystemShareInfo()).toBean(OcLiveMountFsShareInfo.class);
            liveMountFsShareReq.setLiveMountFsShareInfos(fsShareList);
        }
        return liveMountFsShareReq;
    }

    private void buildGeneralData(LiveMountCreateTask task, LiveMountExecuteParam liveMountExecuteParam) {
        task.setRequestId(liveMountExecuteParam.getRequestId());
        task.setTaskId(liveMountExecuteParam.getJobId());
        // 初始化数据面副本ID
        Copy mountedCopy = liveMountExecuteParam.getCloneCopy();
        JSONObject properties = JSONObject.fromObject(mountedCopy.getProperties());
        String backupId = Optional.ofNullable(properties.getString(BACKUP_ID)).orElse(mountedCopy.getUuid());
        task.setCopyId(backupId);

        // 初始化高级参数
        LiveMountEntity liveMountEntity = liveMountExecuteParam.getLiveMount();
        Map<String, Object> parameters = JSONObject.fromObject(liveMountEntity.getParameters()).toMap(Object.class);

        JSONArray jsonArray = JSONArray.fromObject(liveMountEntity.getFileSystemShareInfo());
        List<LiveMountFileSystemShareInfo> fileSystemShareInfo = JSONArray.toCollection(jsonArray,
            LiveMountFileSystemShareInfo.class);
        parameters.put(LiveMountConstants.FILE_SYSTEM_SHARE_INFO, fileSystemShareInfo);
        task.setAdvanceParams(parameters);
        // 初始化目标环境信息
        ProtectedResource targetResource = buildResource(liveMountEntity.getTargetResourceId(), mountedCopy);
        task.setTargetEnv(BeanTools.copy(targetResource, TaskEnvironment::new));
        // 初始化agent信息，像nas这类不做挂载的需要填充agent为空
        String sourceResourceId = mountedCopy.getResourceId();
        ProtectedResource sourceResource = buildResource(sourceResourceId, mountedCopy);
        // 初始化存储库信息
        if (deployTypeService.isCyberEngine()) {
            List<StorageRepository> storageRepositories = getCyberEngineStorageRepositories(
                liveMountExecuteParam.getCloneCopy().getProperties());
            // 添加认证存储设备认证信息
            Optional<ProtectedResource> resOptional = resourceService.getResourceById(mountedCopy.getResourceId());
            resOptional.ifPresent(
                protectedResource -> storageRepositories.get(0).setExtendAuth(protectedResource.getAuth()));
            task.setRepositories(storageRepositories);
        } else {
            task.setAgents(buildAgents(task.getRequestId(), sourceResource, parameters));
            task.setRepositories(getStorageRepositories(liveMountExecuteParam.getSourceCopy().getProperties()));
        }
        // 设置目标资源信息
        task.setTargetObject(BeanTools.copy(targetResource, TaskResource::new));
    }

    /**
     * unmount live mount
     *
     * @param unmountParam unmount param
     */
    @Override
    public void unmountLiveMount(LiveMountUnmountParam unmountParam) {
        LiveMountCancelTask task = new LiveMountCancelTask();
        try {
            task.setRequestId(unmountParam.getRequestId());
            task.setTaskId(unmountParam.getJobId());
            Copy mountedCopy = unmountParam.getMountedCopy();
            JSONObject properties = JSONObject.fromObject(mountedCopy.getProperties());
            String backupId = Optional.ofNullable(properties.getString(BACKUP_ID)).orElse(mountedCopy.getUuid());
            task.setCopyId(backupId);

            ProtectedResource resource = resourceService.getResourceById(false,
                    unmountParam.getLiveMount().getResourceId())
                    .orElse(constructResources(unmountParam.getMountedCopy()));
            Map<String, Object> parameters = JSONObject.fromObject(unmountParam.getLiveMount().getParameters())
                    .toMap(Object.class);
            // 初始化agent信息
            task.setAgents(buildAgents(task.getRequestId(), resource, parameters));
            // 初始化存储库信息
            task.setRepositories(getStorageRepositories(mountedCopy.getProperties()));
            // 初始化目标受保护的资源
            Optional<ProtectedResource> targetResource = resourceService.getResourceById(
                    unmountParam.getLiveMount().getTargetResourceId());
            PowerAssert.state(targetResource.isPresent(),
                    () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "target resource not exist"));
            task.setTargetObject(BeanTools.copy(targetResource.get(), TaskResource::new));
            // 设置即时挂载的任务id
            task.getTargetObject()
                    .setExtendInfo(Optional.ofNullable(task.getTargetObject().getExtendInfo()).orElse(new HashMap<>()));
            task.getTargetObject()
                    .getExtendInfo()
                    .put(LiveMountConstants.MOUNT_JOB_ID, unmountParam.getLiveMount().getMountJobId());
            task.setAdvanceParams(Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>()));
            task.getAdvanceParams().put(LiveMountConstants.RESERVE_COPY, unmountParam.getReserveCopy());
            task.getAdvanceParams().put(LiveMountConstants.FORCE_DELETE, unmountParam.getForceDelete());
            // 初始化特性卸载参数
            LiveMountEntity liveMountEntityBo = unmountParam.getLiveMount();
            String subType = liveMountEntityBo.getResourceSubType();
            LiveMountInterceptorProvider provider = providerManager.findProvider(LiveMountInterceptorProvider.class,
                    subType);
            provider.finalize(task);
            opServiceHelper.injectVpcInfoForUnLiveMount(task);

            commonAgentService.supplyAgentCommonInfo(task.getAgents());
            dmeUnifiedService.cancelLiveMount(task);
            jobService.updateJob(task.getRequestId(), JobUpdateUtil.getDeliverReq());
        } finally {
            // 清理 targetObject targetEnv auth(), repositories auth() extendAuth()
            task.cleanBaseTaskAuthPwd();
        }
    }

    private void setMountLanfree(LiveMountCreateTask task, String subType) {
        Map<String, String> fcMap = resourceService.getLanFreeConfig(subType, FibreUtil.getAgentIds(task.getAgents()));
        task.getDataLayout().setClientProtocolType(FibreUtil.getClientProtocol(fcMap));
        task.addParameters(FibreUtil.getLanFreeAgents(fcMap));
        log.debug("mount task:{}, protocol Type:{}",
                task.getRequestId(), task.getDataLayout().getClientProtocolType());
    }

    /**
     * refresh target resource
     *
     * @param liveMountRefreshParam live mount refresh param
     * @return resource uuid
     */
    @Override
    public List<String> refreshTargetResource(LiveMountRefreshParam liveMountRefreshParam) {
        LiveMountEntity liveMountEntity = liveMountRefreshParam.getLiveMount();
        String subType = liveMountEntity.getResourceSubType();
        LiveMountInterceptorProvider provider =
            providerManager.findProvider(LiveMountInterceptorProvider.class, subType);
        if (!provider.isRefreshTargetEnvironment()) {
            if (liveMountRefreshParam.getHasCleanProtection()) {
                return Collections.emptyList();
            } else {
                return Collections.singletonList(null);
            }
        }
        return refreshTargetResource(liveMountEntity);
    }

    private List<String> refreshTargetResource(LiveMountEntity liveMountEntity) {
        String targetResourceId = liveMountEntity.getTargetResourceId();
        List<ProtectedResource> resources =
            resourceService.query(0, 1, Collections.singletonMap("uuid", targetResourceId)).getRecords();
        if (resources.isEmpty()) {
            log.error("not found target resource({})", targetResourceId);
            return Collections.emptyList();
        }
        ProtectedResource resource = resources.get(0);
        ProtectedEnvironment environment;
        if (resource instanceof ProtectedEnvironment) {
            environment = (ProtectedEnvironment) resource;
        } else {
            environment = resource.getEnvironment();
        }
        List<String> increasedResourceUuidList = resourceService.scanProtectedResource(environment);
        if (increasedResourceUuidList.isEmpty()) {
            return Collections.emptyList();
        }

        // 根据扫描新增资源查询LiveMount挂载资源
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("uuid", increasedResourceUuidList);
        conditions.put("name", liveMountEntity.getTargetResourceName());
        List<ProtectedResource> increasedResources =
            resourceService.query(0, increasedResourceUuidList.size(), conditions).getRecords();
        return increasedResources.stream().map(ProtectedResource::getUuid).collect(Collectors.toList());
    }

    @Override
    public void migrateLiveMount(LiveMountMigrateParam migrateParam) {
        throw new NotImplementedException("not implemented");
    }

    @Override
    public LiveMountEntity addLiveMountFileSystemName(LiveMountEntity liveMountEntity) {
        if (!VerifyUtil.isEmpty(liveMountEntity)) {
            String fileSystemShareInfo = liveMountEntity.getFileSystemShareInfo();
            List<LiveMountFileSystemShareInfo> liveMountFileSystemShareInfos = JSONArray.fromObject(fileSystemShareInfo)
                .toBean(LiveMountFileSystemShareInfo.class);
            String filesystemName = FILE_SYSTEM_NAME_PREFIX + UUID.randomUUID();
            liveMountFileSystemShareInfos.forEach(info -> info.setFileSystemName(filesystemName));
            log.debug(
                "When add name,live mount filesystem share info size is: {}, live mount id is :{}, filesystem name: {}",
                liveMountFileSystemShareInfos.size(), liveMountEntity.getId(), filesystemName);
            liveMountEntity.setFileSystemShareInfo(JSONArray.fromObject(liveMountFileSystemShareInfos).toString());
        }
        return liveMountEntity;
    }

    /**
     * detect live mount provider applicable
     *
     * @param resourceSubType resource sub type
     * @return detect result
     */
    @Override
    public boolean applicable(String resourceSubType) {
        if (resourceSubType == null) {
            return false;
        }
        return findLiveMountInterceptorProvider(resourceSubType).isPresent();
    }

    private Optional<LiveMountInterceptorProvider> findLiveMountInterceptorProvider(String resourceSubType) {
        return liveMountInterceptorProviders.stream()
            .filter(liveMountInterceptorProvider -> liveMountInterceptorProvider.applicable(resourceSubType))
            .findFirst();
    }

    private LiveMountInterceptorProvider getLiveMountInterceptorProvider(String resourceSubType) {
        return findLiveMountInterceptorProvider(resourceSubType)
            .orElseThrow(() -> new LegoCheckedException("not supported"));
    }

    private ProtectedResource buildResource(String targetResourceId, Copy mountedCopy) {
        List<ProtectedResource> resources = resourceService.query(0, 1,
            Collections.singletonMap("uuid", targetResourceId)).getRecords();
        if (CollectionUtils.isEmpty(resources)) {
            return constructResources(mountedCopy);
        }
        return resources.get(0);
    }

    private ProtectedEnvironment constructResources(Copy mountedCopy) {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(mountedCopy.getResourceId());
        protectedEnvironment.setName(mountedCopy.getResourceName());
        protectedEnvironment.setType(mountedCopy.getResourceType());
        protectedEnvironment.setSubType(mountedCopy.getResourceSubType());
        return protectedEnvironment;
    }

    private List<Endpoint> buildAgents(String jobId, ProtectedResource targetResource, Map<String, Object> parameters) {
        // 此处最好走内置agent,先暂时使用此selector
        Map<String, String> parametersString = new HashMap<>();
        for (Map.Entry<String, Object> entry : parameters.entrySet()) {
            if (!VerifyUtil.isEmpty(entry.getValue())) {
                parametersString.put(entry.getKey(), entry.getValue().toString());
            }
        }
        JobBo jobBo = jobService.queryJob(jobId);
        if (jobBo != null && !VerifyUtil.isEmpty(jobBo.getUserId())) {
            UserInnerResponse userInnerResponse = userService.getUserInfoByUserId(jobBo.getUserId());
            parametersString.put(AgentKeyConstant.USER_INFO, JSONObject.writeValueAsString(userInnerResponse));
        }

        // 设置保护代理
        ProtectAgentSelector selector = providerManager.findProviderOrDefault(ProtectAgentSelector.class,
            targetResource.getSubType(), defaultSelector);
        List<Endpoint> agentList = selector.select(targetResource, parametersString);
        if (VerifyUtil.isEmpty(agentList)) {
            long errorCode = deployTypeService.isCyberEngine()
                ? CommonErrorCode.AGENT_NOT_EXIST_CYBER
                : CommonErrorCode.AGENT_NOT_EXIST;
            throw new LegoCheckedException(errorCode, "No available agent.");
        }
        return agentList;
    }

    private List<StorageRepository> getCyberEngineStorageRepositories(String copyProperties) {
        // 副本中的扩展参数数据，json字符串形式存储
        JSONObject extendParam = JSONObject.fromObject(copyProperties);
        JSONArray repositories = extendParam.getJSONArray(CopyPropertiesKeyConstant.KEY_REPOSITORIES);
        Assert.notEmpty(repositories, "Live mount copy properties not contains repositories info.");
        return repositories.map(repository -> JSONObject.fromObject(repository).toBean(StorageRepository.class));
    }

    private List<StorageRepository> getStorageRepositories(String copyProperties) {
        // 副本中的扩展参数数据，json字符串形式存储
        final JSONObject extendParam = JSONObject.fromObject(copyProperties);
        final JSONArray repositories = extendParam.getJSONArray(CopyPropertiesKeyConstant.KEY_REPOSITORIES);
        Assert.notEmpty(repositories, "Live mount copy properties not contains repositories info.");
        return repositories.map(repository -> this.buildStorageRepository(
            JSONObject.fromObject(repository).toBean(BaseStorageRepository.class)));
    }

    private StorageRepository buildStorageRepository(BaseStorageRepository base) {
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(
            RepositoryProtocolEnum.getByProtocol(base.getProtocol()));
        return strategy.getRepository(base);
    }
}
