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
package openbackup.data.access.framework.restore.service;

import com.huawei.oceanprotect.exercise.service.ExerciseQueryService;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.google.common.base.Joiner;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.service.CopyAuthVerifyService;
import openbackup.data.access.framework.copy.verify.service.CopyVerifyTaskManager;
import openbackup.data.access.framework.core.common.constants.TaskParamConstants;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.access.framework.protection.common.constants.JobStatusLabelConstant;
import openbackup.data.access.framework.protection.common.util.FibreUtil;
import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.access.framework.protection.service.job.JobLogRecorder;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.access.framework.restore.constant.RestoreJobLabelConstant;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.converter.RestoreTaskConverter;
import openbackup.data.access.framework.restore.dto.RestoreTaskContext;
import openbackup.data.access.framework.servitization.util.OpServiceHelper;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupFeatureService;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.job.util.JobUpdateUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Recover;
import org.springframework.retry.annotation.Retryable;
import org.springframework.stereotype.Component;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.UUID;

/**
 * 恢复任务工作流管理类，负责恢复任务中各个步骤的执行，控制整个恢复任务流程<br>
 * <p>
 * 一个恢复任务的完成业务流程主要步骤：<br>
 * 1.业务数据填充 <br>
 * <ul>
 *     <li>1.1公共数据填充（框架负责）</li>
 *     <li>1.2应用相关数据填充（插件负责）（可选）</li>
 * </ul>
 * 2.恢复任务业务校验 <br>
 * <ul>
 *     <li>2.1公共业务校验（框架负责）</li>
 *     <li>2.2应用相关的业务校验（插件负责）</li>
 * </ul>
 * 3.创建恢复job <br>
 * 4.JobCenter进行限流排队检查通过后，发出恢复任务开始消息<br>
 * 5.[异步]消费JobCenter消息 <br>
 * 6.校验license,资源锁同步加锁 <br>
 * 7.调用DME统一接口，下发任务到DME <br>
 *  <ul>
 *      <li>7.1 更新恢复任务为运行中</li>
 *      <li>7.2 更新副本状态为恢复中</li>
 *  </ul>
 * 8.数据面上报任务进度（DME及Agent负责） <br>
 * 9.DME调用任务状态上报REST接口上报任务结果 <br>
 * 10.接收任务上报结果，根据结果更新Job状态和副本状态 <br>
 * 11.同步释放资源锁 <br>
 * 12.手动扫描受保护环境 <br>
 * 13.框架调用插件中的后置处理接口（可选）<br>
 * </p>
 *
 **/
@Slf4j
@Component
public class RestoreTaskManager {
    private final ProviderManager providerManager;
    private final RestoreTaskService restoreTaskService;
    private final JobLogRecorder jobLogRecorder;
    private final RestoreValidateService restoreValidateService;
    private final RestoreResourceService restoreResourceService;
    private final CopyVerifyTaskManager copyVerifyTaskManager;
    private final JobService jobService;
    private DeployTypeService deployTypeService;
    private OpServiceHelper opServiceHelper;
    private UserService userService;
    private SanClientService sanClientService;
    private CommonAgentService commonAgentService;
    @Autowired
    private ExerciseQueryService exerciseQueryService;

    private TaskRepositoryManager taskRepositoryManager;

    private CopyAuthVerifyService copyAuthVerifyService;

    private BackupFeatureService backupFeatureService;

    /**
     * 恢复任务流程管理构造函数
     *
     * @param providerManager 插件管理器
     * @param restoreTaskService 恢复任务服务
     * @param jobLogRecorder 任务日志记录器
     * @param restoreValidateService 恢复校验服务
     * @param restoreResourceService 恢复资源服务
     * @param copyVerifyTaskManager 副本校验任务管理器
     * @param jobService 任务管理器
     */
    public RestoreTaskManager(
        ProviderManager providerManager, RestoreTaskService restoreTaskService,
        JobLogRecorder jobLogRecorder, RestoreValidateService restoreValidateService,
        RestoreResourceService restoreResourceService, CopyVerifyTaskManager copyVerifyTaskManager,
        JobService jobService) {
        this.providerManager = providerManager;
        this.restoreTaskService = restoreTaskService;
        this.jobLogRecorder = jobLogRecorder;
        this.restoreValidateService = restoreValidateService;
        this.restoreResourceService = restoreResourceService;
        this.copyVerifyTaskManager = copyVerifyTaskManager;
        this.jobService = jobService;
    }

    @Autowired
    public void setTaskRepositoryManager(TaskRepositoryManager taskRepositoryManager) {
        this.taskRepositoryManager = taskRepositoryManager;
    }

    @Autowired
    public void setCommonAgentService(CommonAgentService commonAgentService) {
        this.commonAgentService = commonAgentService;
    }

    @Autowired
    public void setDeployTypeService(DeployTypeService deployTypeService) {
        this.deployTypeService = deployTypeService;
    }

    @Autowired
    public void setUserService(UserService userService) {
        this.userService = userService;
    }

    @Autowired
    public void setSanClientService(SanClientService sanClientService) {
        this.sanClientService = sanClientService;
    }

    @Autowired
    public void setOpServiceHelper(OpServiceHelper opServiceHelper) {
        this.opServiceHelper = opServiceHelper;
    }

    @Autowired
    public void setBackupFeatureService(BackupFeatureService backupFeatureService) {
        this.backupFeatureService = backupFeatureService;
    }

    @Autowired
    public void setCopyAuthVerifyService(CopyAuthVerifyService copyAuthVerifyService) {
        this.copyAuthVerifyService = copyAuthVerifyService;
    }

    /**
     * 初始化恢复任务流程
     * <p>
     * 主要负责恢复任务步骤1-3的处理
     *
     * @param request 恢复任务创建请求
     * @see CreateRestoreTaskRequest
     * @return jobId 任务id
     */
    public String init(CreateRestoreTaskRequest request) {
        checkSubObjectSize(request.getSubObjects());
        // 初始化恢复上下文
        final RestoreTaskContext context = this.initContext(request);
        // 恢复请求填充通用数据
        this.fillGeneralData(context);
        // 校验通用数据
        this.checkGeneralData(context);
        // 插件任务创建前置校验
        this.pluginPreCheck(context);
        // 创建恢复任务
        return restoreTaskService.createJob(context);
    }

    private void checkSubObjectSize(List<String> subObjects) {
        if (VerifyUtil.isEmpty(subObjects)) {
            return;
        }
        if (subObjects.size() > IsmNumberConstant.THOUSAND) {
            throw new LegoCheckedException(CommonErrorCode.RESTORE_SUB_OBJECT_NUM_MAX_LIMIT,
                new String[]{String.valueOf(IsmNumberConstant.THOUSAND)}, "Total sub object num over 1000");
        }
        long totalSize = 0L;
        for (String subObject : subObjects) {
            totalSize += subObject.getBytes(StandardCharsets.UTF_8).length;
        }
        if (totalSize > IsmNumberConstant.THOUSAND * IsmNumberConstant.THROUND_TWENTY_FOUR) {
            throw new LegoCheckedException(CommonErrorCode.RESTORE_SUB_OBJECT_MAX_BYTES,
                "Total sub object size over 1024 * 1000");
        }
    }

    private RestoreTaskContext initContext(CreateRestoreTaskRequest request) {
        String requestId = UUID.randomUUID().toString();
        log.info("Restore task init context, requestId: {}.", requestId);

        final RestoreTask restoreTask = RestoreTaskConverter.convertToRestoreTask(request);
        log.debug("Restore task init context, query copy: {}, requestId: {}.", request.getCopyId(), requestId);
        final Copy copy = this.restoreTaskService.queryCopyDetail(request.getCopyId());
        log.info("Restore task init context, ResourceSubType: {}.", copy.getResourceSubType());
        // 填充恢复下发的agent和用户信息的扩展字段
        fillRestoreTaskAdvanceParams(restoreTask, request, copy);

        restoreTask.setRequestId(requestId);
        restoreTask.setTaskId(requestId);
        // 安全一体机适配，Nas文件系统的快照数据被标记为CloudBackup，恢复时需修正
        if (deployTypeService.isCyberEngine()
            && ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType().equals(copy.getResourceSubType())) {
            copy.setResourceSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        }
        RestoreTaskContext context = new RestoreTaskContext();
        context.setRequestId(requestId);
        context.setTaskRequest(request);
        context.setCopy(copy);
        context.setRestoreTask(restoreTask);
        RestoreInterceptorProvider provider = getProvider(copy.getResourceSubType());
        context.setInterceptorProvider(provider);
        return context;
    }

    private void encryptExtendInfo(CreateRestoreTaskRequest request, String resourceSubType) {
        RestoreInterceptorProvider provider = getProvider(resourceSubType);
        provider.encryptExtendInfo(request.getExtendInfo());
    }

    private void checkUserRestoreAuthOperation(RestoreTaskContext context) {
        if (deployTypeService.isNotSupportRBACType()) {
            return;
        }
        AuthOperationEnum restoreAuth = getRestoreAuth(context);
        final Copy copy = context.getCopy();
        List<String> authOperationList = Collections.singletonList(restoreAuth.getAuthOperation());
        // 恢复演练功能在恢复演练层已校验恢复演练权限，此处不再校验副本恢复权限
        if (StringUtils.isEmpty(context.getTaskRequest().getExerciseJobId())) {
            copyAuthVerifyService.checkCopyOperationAuth(copy, authOperationList);
        }
    }

    private AuthOperationEnum getRestoreAuth(RestoreTaskContext context) {
        RestoreLocationEnum restoreLocation = context.getRestoreTask().getTargetLocation();
        AuthOperationEnum restoreAuth;
        switch (restoreLocation) {
            case ORIGINAL:
                restoreAuth = AuthOperationEnum.ORIGINAL_RESTORE;
                break;
            case NEW:
                restoreAuth = AuthOperationEnum.NEW_RESTORE;
                break;
            case NATIVE:
                restoreAuth = AuthOperationEnum.NATIVE_RESTORE;
                break;
            default:
                log.error("The restore location {} is illegal.", restoreLocation.getLocation());
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Illegal restore target location.");
        }
        return restoreAuth;
    }

    private void fillRestoreTaskAdvanceParams(RestoreTask restoreTask, CreateRestoreTaskRequest request, Copy copy) {
        // 加密任务扩展信息中的敏感字段
        encryptExtendInfo(request, copy.getResourceSubType());
        Map<String, String> extendInfo = request.getExtendInfo() == null
            ? new HashMap<>()
            : BeanTools.deepClone(request.getExtendInfo());
        if (CollectionUtils.isNotEmpty(request.getAgents())) {
            extendInfo.put(AgentKeyConstant.AGENTS_KEY,
                Joiner.on(AgentKeyConstant.AGENTS_SPLIT).join(request.getAgents()));
        }
        UserInnerResponse userInnerResponse = userService.getUserInfoByUserId(getUserId(request));
        extendInfo.put(AgentKeyConstant.USER_INFO, JSONObject.writeValueAsString(userInnerResponse));
        restoreTask.setAdvanceParams(extendInfo);
    }

    private String getUserId(CreateRestoreTaskRequest request) {
        if (VerifyUtil.isEmpty(request.getExerciseId())) {
            return TokenBo.get().getUser().getId();
        }
        return exerciseQueryService.queryExercise(request.getExerciseId()).getUserId();
    }

    private void pluginPreCheck(RestoreTaskContext context) {
        // 各个插件恢复请求填充业务
        context.getInterceptorProvider().restoreTaskCreationPreCheck(context.getRestoreTask());
    }

    private RestoreInterceptorProvider getProvider(String resourceSubType) {
        return providerManager.findProvider(
            RestoreInterceptorProvider.class,
            resourceSubType,
            new LegoCheckedException("Restore task can not find provider."));
    }

    private void fillGeneralData(RestoreTaskContext context) {
        log.info("Restore task framework fill general data, requestId: {}.", context.getRequestId());
        final CreateRestoreTaskRequest taskRequest = context.getTaskRequest();
        final RestoreTask restoreTask = context.getRestoreTask();
        final Copy copy = context.getCopy();
        ProtectedEnvironment protectedEnvironment = restoreResourceService.queryProtectedEnvironment(taskRequest, copy);
        // 填充恢复目标信息
        log.debug("Restore task query env, env: {},requestId: {}.", taskRequest.getTargetEnv(), context.getRequestId());
        restoreTask.setTargetEnv(covertToTaskEnvironment(protectedEnvironment));

        // 填充恢复目标对象信息
        restoreTask.setTargetObject(buildTargetObject(taskRequest, copy));
        // 快照备份没有存储库信息
        if (copy.getBackupType() == BackupTypeConstants.NATIVE_SNAPSHOT.getAbBackupType()) {
            return;
        }

        // 填充恢复存储库信息
        restoreTask.setRepositories(
            taskRepositoryManager.getStorageRepositories(copy.getProperties(), copy.getStorageUnitId()));

        // 设置恢复模式为普通副本恢复模式
        restoreTask.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
    }

    private void setLanFreeConfig(RestoreTask task, String subType) {
        Map<String, String> fcConfigMap = restoreResourceService.getLanFreeConfig(subType, task);
        task.getDataLayout().setClientProtocolType(FibreUtil.getClientProtocol(fcConfigMap));
        task.addParameters(FibreUtil.getLanFreeAgents(fcConfigMap));
        log.debug("restore task:{}, protocol Type:{}",
            task.getRequestId(), task.getDataLayout().getClientProtocolType());
        // agent配置了sanclient，并且副本不是san副本以及agent没有配置sanclient，且副本为san副本，不能执行校验
        if (!sanClientService.checkSanCopyAndLanFree(task)) {
            log.error("Inconsistent copy type, check not allowed, taskId={}", task.getTaskId());
            throw new LegoCheckedException(CommonErrorCode.INCONSISTENT_COPY_TYPE, "check not allowed");
        }
        // 从副本中解析是否是san，若是则添加san标记
        log.info("Start config san, taskID is {}", task.getTaskId());
        sanClientService.configSanClient(task.getCopyId(), task.getAgents(), task.getAdvanceParams(),
            task.getDataLayout());
    }

    private TaskEnvironment covertToTaskEnvironment(ProtectedEnvironment protectedEnvironment) {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(protectedEnvironment, taskEnvironment);
        return taskEnvironment;
    }

    private TaskResource buildTargetObject(CreateRestoreTaskRequest taskRequest, Copy copy) {
        final TaskResource targetResource = RestoreTaskHelper.buildTaskResourceByCopy(copy);
        if (StringUtils.isNotBlank(taskRequest.getTargetObject())) {
            // 目标对象值非空时，填充目标对象信息
            final TaskResource taskResource =
                restoreResourceService.buildTaskResourceByTargetObject(taskRequest.getTargetObject());
            // id场景: 如果能查询到资源就直接返回
            if (StringUtils.isNotBlank(taskResource.getUuid())) {
                return taskResource;
            }
            // path场景: 去副本里取资源，然后用path覆盖name
            targetResource.setName(taskRequest.getTargetObject());
            return targetResource;
        }
        return targetResource;
    }

    private void checkGeneralData(RestoreTaskContext context) {
        final Copy copy = context.getCopy();
        if (RestoreTaskHelper.isEnableCopyVerify(context.getTaskRequest().getExtendInfo())) {
            // 开启了副本校验，要先校验副本校验子流程是否能够执行
            copyVerifyTaskManager.checkTaskCanBeExecuted(copy);
        }
        // 校验副本状态为正常
        RestoreValidator.checkCopyCanRestore(copy, context.getTaskRequest().getExtendInfo());
        // 校验目标环境是否是在线状态
        RestoreValidator.checkEnvironmentIsOnline(context.getRestoreTask().getTargetEnv(),
            context.getInterceptorProvider().getRestoreFeature());
        // 校验用户恢复权限
        checkUserRestoreAuthOperation(context);
    }

    /**
     * 下发恢复任务到DME
     * <p>
     * 主要负责恢复任务步骤5-7的处理
     * </p>
     * <p>
     * 任务执行出现错误，则会根据任务类型判断是否进行重试，重试依据：<br>
     * 1.如果异常为 {@code LegoCheckedException}认为是程序自己抛出的异常， 不需要进行重试<br>
     * 2.如果是其它异常则进行重试，重试规则： 重试：根据系统规格，最多重试5次，每次间隔2分钟<br>
     * 3.重试超过最大规格之后，会执行回退方法：请见{@link #recoverStart}<br>
     * </p>
     *
     * @param task 恢复任务对象
     */
    @Retryable(
        label = "restoreTaskStart",
        exclude = {LegoCheckedException.class},
        maxAttemptsExpression = "${system.specs.retry.maxAttempts}",
        backoff = @Backoff(delayExpression = "${system.specs.retry.maxDelay}"))
    public void start(RestoreTask task) {
        // 恢复请求填充业务
        RestoreTask restoreTask = task.deepClone();
        if (!jobService.isJobPresent(restoreTask.getRequestId())) {
            return;
        }
        // 填充恢复存储库信息
        fillRestoreRepository(restoreTask);
        // 填充agent信息和各个插件业务参数信息
        final Copy copy = this.restoreTaskService.queryCopyDetail(restoreTask.getCopyId());
        fillRestoreTask(restoreTask.decryptRestoreTask(), copy);
        if (!deployTypeService.isHyperDetectDeployType() && !deployTypeService.isCyberEngine()
            && CollectionUtils.isEmpty(restoreTask.getAgents())) {
            log.info("Agents is empty, taskId: {}", restoreTask.getTaskId());
            // 恢复任务开始的回退处理
            recoverStart(new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "No agent found"), restoreTask);
            return;
        }
        log.info("Restore task start, requestId: {}.", restoreTask.getRequestId());
        JobBo jobBo = jobService.queryJob(restoreTask.getRequestId());
        if (JobStatusEnum.FINISHED_STATUS_LIST.contains(JobStatusEnum.get(jobBo.getStatus()))) {
            log.warn("Restore task has been finished, requestId: {}.", restoreTask.getRequestId());
            return;
        }
        // 记录恢复任务执行
        jobLogRecorder.recordJobStep(
            restoreTask.getRequestId(),
            JobLogLevelEnum.INFO,
            RestoreJobLabelConstant.RESTORE_INIT,
            Collections.singletonList(JobStatusLabelConstant.JOB_SUCCESS_LABEL));
        // 校验license
        restoreValidateService.checkLicense(
            copy.getResourceSubType(), RestoreTypeEnum.getByType(restoreTask.getRestoreType()));
        // 锁定恢复任务资源
        if (lockResourceFailed(restoreTask, copy.getResourceSubType())) {
            return;
        }
        // 将资源锁id更新到任务data字段中
        restoreTaskService.updateJobLockId(restoreTask.getRequestId());
        // 解密auth password
        if (RestoreTaskHelper.isEnableCopyVerify(restoreTask.getAdvanceParams())) {
            restoreTaskService.modifyJobProgressRange(restoreTask.getRequestId());
        }
        // 填充fc LanFree配置
        setLanFreeConfig(restoreTask, copy.getResourceSubType());

        // 下发恢复任务到DME/DEE
        commonAgentService.supplyAgentCommonInfo(restoreTask.getAgents());
        log.info("begin to send restore task({}) to dme.", restoreTask.getRequestId());
        restoreTaskService.startTask(restoreTask);
        log.info("send restore task({}) to dme successfully.", restoreTask.getRequestId());
        jobService.updateJob(task.getRequestId(), JobUpdateUtil.getDeliverReq());

        postHandleStartTask(restoreTask);
    }

    private void fillRestoreRepository(RestoreTask restoreTask) {
        final Copy copy = this.restoreTaskService.queryCopyDetail(restoreTask.getCopyId());
        // 快照备份没有存储库信息
        if (copy.getBackupType() == BackupTypeConstants.NATIVE_SNAPSHOT.getAbBackupType()) {
            return;
        }
        restoreTask.setRepositories(
            taskRepositoryManager.getStorageRepositories(copy.getProperties(), copy.getStorageUnitId()));
    }

    private void fillRestoreTask(RestoreTask restoreTask, Copy copy) {
        ProtectedEnvironment protectedEnvironment =
            BeanTools.copy(restoreTask.getTargetEnv(), ProtectedEnvironment::new);
        log.info("Restore task fill agents, taskId: {}.", restoreTask.getTaskId());
        // 安全一体机适配，Nas文件系统的快照数据被标记为CloudBackup，恢复时需修正
        if (deployTypeService.isCyberEngine()
            && ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType().equals(copy.getResourceSubType())) {
            copy.setResourceSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        }
        restoreTask.setAgents(
            restoreResourceService.queryEndpoints(restoreTask.getAdvanceParams(), copy.getResourceSubType(),
                protectedEnvironment.getType(), protectedEnvironment));
        RestoreInterceptorProvider provider = getProvider(copy.getResourceSubType());
        setAgentMountType(restoreTask, provider, copy);
        // 各个插件恢复请求填充业务
        RestoreTask task = provider.initialize(restoreTask);
        paramCorrection(task);
        opServiceHelper.injectVpcInfoForRestore(restoreTask);
    }

    private void paramCorrection(RestoreTask task) {
        if (task.getAdvanceParams().containsKey("isWormFileSystem")) {
            log.info("is worm file system, start update");
            buildWormUpdateJobRequest(task);
            throw new LegoCheckedException(
                "Invalid resource sub type to start restore task: " + task.getTargetObject().getUuid());
        }
        if (deployTypeService.isE1000()) {
            task.getRepositories().forEach(repo -> {
                if (repo.getEndpoint() != null && repo.getEndpoint().getPort() < 0) {
                    repo.getEndpoint().setPort(0);
                }
            });
        }
        // 本地盘存储单元会切换节点需要更新副本信息
        if (AgentMountTypeEnum.FUSE.getValue()
            .equals(task.getAdvanceParams().get(TaskParamConstants.AGENT_MOUNT_TYPE))) {
            taskRepositoryManager.updateCopyRepoInfo(task.getRepositories());
        }
    }

    private void setAgentMountType(RestoreTask task, RestoreInterceptorProvider interceptor, Copy copy) {
        Map<String, String> tmpMap = new HashMap<>();
        if (interceptor != null) {
            Optional<AgentMountTypeEnum> mountTypeOp = interceptor.getMountType(task);
            mountTypeOp.ifPresent(
                agentMountTypeEnum -> tmpMap.put(TaskParamConstants.AGENT_MOUNT_TYPE, agentMountTypeEnum.getValue()));
        }
        tmpMap.putIfAbsent(TaskParamConstants.AGENT_MOUNT_TYPE,
            commonAgentService.getJobAgentMountTypeByUnitId(copy.getStorageUnitId()).getValue());
        task.addParameters(tmpMap);
    }

    private void buildWormUpdateJobRequest(RestoreTask restoreTask) {
        JobBo jobBo = jobService.queryJob(restoreTask.getRequestId());
        String extendStr = jobBo.getExtendStr();
        ObjectMapper objectMapper = new ObjectMapper();
        try {
            JsonNode jsonNode = objectMapper.readTree(extendStr);
            if (jsonNode instanceof ObjectNode) {
                ObjectNode objectNode = (ObjectNode) jsonNode;
                objectNode.put("errorMessage",
                    "Restoration using this snapshot is not supported because file system  corresponding"
                        + " to the selected snapshot is a WORM file system.");
                UpdateJobRequest request = new UpdateJobRequest();
                request.setExtendStr(objectNode.toString());
                jobService.updateJob(restoreTask.getRequestId(), request);
            }
        } catch (JsonProcessingException e) {
            log.error("jsonParseError, message:{}", e);
        }
    }

    private void postHandleStartTask(RestoreTask task) {
        try {
            // 更新任务状态为运行中
            restoreTaskService.updateJobStatus(task.getRequestId(), ProviderJobStatusEnum.RUNNING);
            // 更新任务enforce_stop为false，任务无法在pm被直接停止
            restoreTaskService.updateJobEnforceStopToFalse(task.getRequestId());
            // 更新副本状态为恢复中，由于支持恢复并发，移除该更新
            // 记录恢复任务运行中
            jobLogRecorder.recordJobStep(
                task.getRequestId(),
                JobLogLevelEnum.INFO,
                RestoreJobLabelConstant.RESTORE_START,
                Collections.singletonList(JobStatusLabelConstant.JOB_SUCCESS_LABEL));
            RestoreInterceptorProvider provider = providerManager.findProvider(
                    RestoreInterceptorProvider.class,
                    task.getTargetObject().getSubType(),
                    null);
            if (provider != null) {
                provider.afterSendTask(task);
            }
        } catch (LegoCheckedException e) {
            jobLogRecorder.recordJobStepWithError(
                task.getRequestId(), RestoreJobLabelConstant.RESTORE_START, e.getErrorCode(), null);
            throw e;
        } finally {
            // 清理 targetObject targetEnv auth(), repositories auth() extendAuth()
            task.cleanBaseTaskAuthPwd();
            sanClientService.cleanAgentIqns(task.getAgents(), task.getCopyId());
        }
    }

    private boolean lockResourceFailed(RestoreTask restoreTask, String resourceSubType) {
        final RestoreInterceptorProvider provider = this.getProvider(resourceSubType);
        List<LockResourceBo> lockResources = provider.getLockResources(restoreTask);
        lockResources = new ArrayList<>(lockResources);
        if (!VerifyUtil.isEmpty(lockResources)) {
            boolean isSupportParallel =
                backupFeatureService.isSupportDataAndLogParallelBackup(restoreTask.getTargetObject().getUuid());
            if (isSupportParallel) {
                int size = lockResources.size();
                for (int i = 0; i < size; i++) {
                    // 支持日志备份的，目标资源@log需要加读锁
                    addOrUpdateResourceLock(lockResources, lockResources.get(i).getId() + "@log", LockType.WRITE);
                }
            }
        }
        if (!restoreTaskService.lockResources(restoreTask.getRequestId(), restoreTask.getCopyId(), lockResources)) {
            this.complete(restoreTask, ProviderJobStatusEnum.FAIL);
            return true;
        }
        return false;
    }

    private void addOrUpdateResourceLock(List<LockResourceBo> lockResources, String resourceId, LockType lockType) {
        LockResourceBo lockResourceBo = null;
        for (LockResourceBo elemLock : lockResources) {
            if (Objects.equals(elemLock.getId(), resourceId)) {
                lockResourceBo = elemLock;
                break;
            }
        }
        if (lockResourceBo == null) {
            lockResourceBo = new LockResourceBo(resourceId, lockType);
            lockResources.add(lockResourceBo);
        } else {
            lockResourceBo.setLockType(lockType);
        }
    }

    /**
     * 恢复任务开始{@link #start}的回退处理。<br>
     * <p>
     * !!!!该方法为重试框架自动调用，不能删除!!!!<br>
     * 当重试超过最大规格后，会调用此方法，进入回退方法后进入兜底处理流程。<br>
     * 按任务失败进行处理：<br>
     * 1.更新任务状态为失败 <br>
     * 2.更新副本状态为正常 <br>
     * 3.释放资源锁 <br>
     * </p>
     *
     * @param exception 需要重试的异常类
     * @param restoreTask 恢复任务下发请求
     */
    @Recover
    public void recoverStart(Exception exception, RestoreTask restoreTask) {
        log.error("Restore task start retry failed, requestId: {}", restoreTask.getRequestId(), exception);
        LegoCheckedException legoCheckedException = LegoCheckedException.cast(exception, CommonErrorCode.SYSTEM_ERROR);
        List<String> errorParams = VerifyUtil.isEmpty(legoCheckedException.getParameters())
            ? null : Arrays.asList(legoCheckedException.getParameters());
        try {
            // 记录恢复任务错误信息
            jobLogRecorder.recordJobStepWithError(
                restoreTask.getRequestId(),
                RestoreJobLabelConstant.RESTORE_START,
                legoCheckedException.getErrorCode(),
                errorParams);
            // 按任务失败来结束处理
            this.complete(restoreTask, ProviderJobStatusEnum.FAIL);
        } catch (Exception ex) {
            log.error("Restore task start recover failed, requestId: {}", restoreTask.getRequestId(), ex);
        } finally {
            restoreTask.cleanBaseTaskAuthPwd();
        }
    }

    /**
     * 任务完成处理。
     *
     * @param restoreTask 恢复任务对象
     * @param jobStatus 任务结果状态
     */
    public void complete(RestoreTask restoreTask, ProviderJobStatusEnum jobStatus) {
        String requestId = restoreTask.getRequestId();
        try {
            log.info("Restore task complete, requestId: {}, status: {}", requestId, jobStatus.name());
            // 更新副本状态移除，原因要支持恢复挂载并发
            // 记录恢复完成日志，hyperdetect不记录恢复完成，任务下发到底座监控状态
            if (!deployTypeService.isHyperDetectDeployType()) {
                jobLogRecorder.recordJobStep(requestId, jobLogRecorder.getLevelByJobStatus(jobStatus),
                    RestoreJobLabelConstant.RESTORE_COMPLETE, JobLogRecorder.buildStepParam(jobStatus));
            }
            // 恢复流程后置处理
            this.postProcess(restoreTask, jobStatus);
        } catch (LegoCheckedException e) {
            log.error("Invoking api to update copy status: {} failed.", restoreTask.getCopyId(),
                ExceptionUtil.getErrorMessage(e));
            throw LegoCheckedException.cast(e);
        } finally {
            // 解锁资源
            restoreTaskService.unlockResources(requestId);
            restoreTaskService.updateJobStatus(requestId, jobStatus);
            // 刷新目标资源
            restoreResourceService.refreshTargetEnv(requestId, restoreTask.getTargetEnv().getUuid(), jobStatus);
        }
    }

    /**
     * 恢复任务后置流程处理
     *
     * @param restoreTask 恢复任务参数
     * @param jobStatus 任务状态
     */
    private void postProcess(RestoreTask restoreTask, ProviderJobStatusEnum jobStatus) {
        log.debug("Restore task complete post process, requestId={}", restoreTask.getRequestId());
        final Copy copy = this.restoreTaskService.queryCopyDetail(restoreTask.getCopyId());
        RestoreInterceptorProvider provider = providerManager.findProvider(RestoreInterceptorProvider.class,
            copy.getResourceSubType(), new LegoCheckedException("Restore task can not find provider."));
        provider.postProcess(restoreTask, jobStatus);
    }
}

