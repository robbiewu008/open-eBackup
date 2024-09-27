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
package openbackup.data.access.framework.copy.verify.service;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.client.sdk.api.framework.dme.CopyVerifyStatusEnum;
import openbackup.data.access.framework.agent.AgentSelectorManager;
import openbackup.data.access.framework.copy.verify.constant.CopyVerifyJobLabelConstant;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.converters.JobDataConverter;
import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.access.framework.protection.service.job.JobLogRecorder;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.access.framework.restore.service.RestoreResourceService;
import openbackup.data.access.framework.servitization.util.OpServiceHelper;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.copy.CopyVerifyInterceptor;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.util.JobUpdateUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Retryable;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;
import java.util.Optional;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * 副本校验任务管理器
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/28
 **/
@Slf4j
@Component
public class CopyVerifyTaskManager {
    // 内置agent的key
    private static final String INTERNAL_AGENT_KEY = "scenario";

    private static final String INTERNAL_AGENT_ESN = "internal_agent_esn";

    private final CopyVerifyService copyVerifyService;

    private final JobLogRecorder jobLogRecorder;

    private final AgentSelectorManager agentSelectorManager;

    private final ProviderManager providerManager;

    private final ResourceService resourceService;

    @Autowired
    private OpServiceHelper opServiceHelper;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private RestoreResourceService restoreResourceService;

    @Autowired
    private MemberClusterService memberClusterService;

    @Autowired
    private TaskRepositoryManager taskRepositoryManager;

    private CommonAgentService commonAgentService;

    private SanClientService sanClientService;

    /**
     * 副本校验任务管理器构造函数
     *
     * @param copyVerifyService 副本校验服务
     * @param jobLogRecorder 任务步骤记录器
     * @param agentSelectorManager agent选择管理器
     * @param providerManager provider管理器
     * @param resourceService 资源服务
     */
    public CopyVerifyTaskManager(CopyVerifyService copyVerifyService, JobLogRecorder jobLogRecorder,
            AgentSelectorManager agentSelectorManager, ProviderManager providerManager,
            ResourceService resourceService) {
        this.jobLogRecorder = jobLogRecorder;
        this.copyVerifyService = copyVerifyService;
        this.agentSelectorManager = agentSelectorManager;
        this.providerManager = providerManager;
        this.resourceService = resourceService;
    }

    @Autowired
    public void setCommonAgentService(CommonAgentService commonAgentService) {
        this.commonAgentService = commonAgentService;
    }

    @Autowired
    public void setSanClientService(SanClientService sanClientService) {
        this.sanClientService = sanClientService;
    }

    /**
     * 任务初始化步骤.
     *
     * @param copyId 副本id
     * @param agents 代理主机id
     * @return 任务id
     */
    public String init(String copyId, String agents) {
        String requestId = UUID.randomUUID().toString();
        log.info("Copy Verify init, requestId={}", requestId);
        CopyVerifyManagerContext context = buildContext(copyId, requestId);
        checkTaskCanBeExecuted(context.getCopy());
        CopyVerifyTask task = buildCopyCheckTask(context, agents);
        commonAgentService.supplyAgentCommonInfo(task.getAgents());
        context.setTask(task);
        return copyVerifyService.createJob(context);
    }

    /**
     * 校验是否能够执行副本校验任务
     *
     * @param copy 副本信息
     */
    public void checkTaskCanBeExecuted(Copy copy) {
        CopyVerifyHelper.copyHasVerificationFile(copy);
        CopyVerifyHelper.checkCopyStatus(copy);
        CopyVerifyHelper.copyIsNotGeneratedByArchive(copy);
        CopyVerifyInterceptor copyVerifyInterceptor = providerManager.findProvider(CopyVerifyInterceptor.class,
                copy.getResourceSubType(), null);
        if (copyVerifyInterceptor != null) {
            copyVerifyInterceptor.checkIsSupportVerify(copy);
        }
    }

    private String generateTaskId(String requestId, boolean isSubTask) {
        if (isSubTask) {
            log.info("Copy Verify init, is subTask, requestId={}", requestId);
            // 作为子任务执行时，重新生成任务id
            return UUID.randomUUID().toString();
        } else {
            // 作为主任务执行时，使用requestId作为任务id
            return requestId;
        }
    }

    private CopyVerifyManagerContext buildContext(String copyId, String requestId) {
        Copy copyInfo = copyVerifyService.getCopyDetail(copyId);
        final CopyVerifyManagerContext context = new CopyVerifyManagerContext();
        context.setCopy(copyInfo);
        context.setRequestId(requestId);
        context.setSubTask(false);
        return context;
    }

    private CopyVerifyTask buildCopyCheckTask(CopyVerifyManagerContext context, String agents) {
        final CopyVerifyTask task = new CopyVerifyTask();
        task.setRequestId(context.getRequestId());
        task.setTaskId(generateTaskId(context.getRequestId(), context.isSubTask()));
        if (StringUtils.isNotBlank(agents)) {
            log.info("copy verify selected agents: {}", agents);
            List<Endpoint> agentList = getAgentEndpoints(agents);
            task.setAgents(agentList);
        }
        if (VerifyUtil.isEmpty(task.getAgents())) {
            task.setAgents(agentSelectorManager.selectAgentsByCopy(context.getCopy()));
        }
        if (VerifyUtil.isEmpty(task.getAgents())) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "No available agent");
        }
        task.setCopyId(context.getCopy().getUuid());
        return task;
    }

    private List<Endpoint> getAgentEndpoints(String agents) {
        return Arrays.stream(agents.split(";"))
            .map(resourceService::getResourceById)
            .filter(Optional::isPresent)
            .map(Optional::get)
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .filter(env -> LinkStatusEnum.ONLINE.getStatus().toString()
            .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env)))
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort(), env.getOsType()))
            .collect(Collectors.toList());
    }

    /**
     * 任务执行步骤.
     *
     * @param task 副本检查任务对象
     */
    @Retryable(label = "copyVerifyStart", exclude = {
        LegoCheckedException.class
    }, maxAttemptsExpression = "${system.specs.retry.maxAttempts}",
        backoff = @Backoff(delayExpression = "${system.specs.retry.maxDelay}"))
    public void execute(CopyVerifyTask task) {
        final String requestId = task.getRequestId();
        final String taskId = task.getTaskId();
        log.info("Copy Verify execute, requestId={}, taskId={}", requestId, taskId);
        jobLogRecorder.recordJobStep(requestId, JobLogLevelEnum.INFO, CopyVerifyJobLabelConstant.COPY_CHECK_INIT, null);
        if (!lockResourceSuccess(task)) {
            log.info("Copy Verify execute, lock resource failed, requestId={}, taskId={}", requestId, taskId);
            return;
        }
        if (task.isSubTask()) {
            copyVerifyService.modifyJobProgressRange(requestId);
        }
        Copy copy = copyVerifyService.getCopyDetail(task.getCopyId());
        fillAgents(task, copy);
        CopyVerifyInterceptor copyVerifyInterceptor = providerManager.findProvider(CopyVerifyInterceptor.class,
                copy.getResourceSubType(), null);
        if (copyVerifyInterceptor != null) {
            copyVerifyInterceptor.interceptor(task);
        }
        opServiceHelper.injectVpcInfoForCopyVerify(task);
        commonAgentService.supplyAgentCommonInfo(task.getAgents());
        // agent配置了sanclient，并且副本不是san副本以及agent没有配置sanclient，且副本为san副本，不能执行恢复
        if (!sanClientService.checkSanCopyAndLanFree(task)) {
            log.error("Inconsistent copy type, recovery not allowed, taskId={}", taskId);
            throw new LegoCheckedException(CommonErrorCode.INCONSISTENT_COPY_TYPE, "recovery not allowed");
        }
        // 从副本中解析是否是san，若是则添加san标记
        log.info("Start config san, taskID is {}", task.getTaskId());
        sanClientService.configSanClient(task.getCopyId(), task.getAgents(), task.getAdvanceParams(),
            task.getDataLayout());
        if (Boolean.parseBoolean(task.getAdvanceParams().get(SanClientService.IS_SANCLIENT))) {
            log.info("Sanclient copy multiPostJob set false, taskID is {}", task.getTaskId());
            task.getAdvanceParams().put(SanClientService.ADVANCE_PARAMS_KEY_MULTI_POST_JOB, Boolean.FALSE.toString());
        }
        // 填充repositories
        task.setRepositories(
            taskRepositoryManager.getStorageRepositories(copy.getProperties(), copy.getStorageUnitId()));
        copyVerifyService.start(task);
        sanClientService.cleanAgentIqns(task.getAgents(), task.getCopyId());
        log.info("Copy Verify execute, task send to ubc successful, requestId={}, taskId={}", requestId, taskId);
        try {
            // 更新任务为运行中
            copyVerifyService.modifyJobStatusRunning(requestId);
            copyVerifyService.modifyCopyStatus(task.getCopyId(), CopyStatus.VERIFYING);
            // 更新任务为不可以强制停止
            copyVerifyService.modifyJobCanNotForceStop(requestId);
            jobLogRecorder.recordJobStep(requestId, JobLogLevelEnum.INFO, CopyVerifyJobLabelConstant.COPY_CHECK_START,
                    null);
            jobLogRecorder.updateJob(requestId, JobUpdateUtil.getDeliverReq());
        } catch (LegoCheckedException e) {
            jobLogRecorder.recordJobStepWithError(requestId, CopyVerifyJobLabelConstant.COPY_CHECK_START,
                    e.getErrorCode(), null);
            throw e;
        }
    }

    private void fillAgents(CopyVerifyTask task, Copy copy) {
        if (!VerifyUtil.isEmpty(task.getAgents())) {
            List<Endpoint> restEndPointList = filterAvailableAgent(task.getAgents());
            task.setAgents(restEndPointList);
        }
        if (CollectionUtils.isNotEmpty(task.getAgents())) {
            log.info("Rest EndPoint List is not empty, task id:{}, agents:{}", task.getTaskId(),
                task.getAgents().stream().map(Endpoint::getId).collect(Collectors.joining(",")));
            return;
        }
        log.info("start fill agents on copy verify task,task id:{}", task.getTaskId());
        // 安全一体机适配，Nas文件系统的快照数据被标记为CloudBackup，恢复时需修正
        if (deployTypeService.isCyberEngine()
            && ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType().equals(copy.getResourceSubType())) {
            copy.setResourceSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        }
        task.setAgents(agentSelectorManager.selectAgentsByCopy(copy));
        log.info("finish fill agents on copy verify task,task id:{} , agents:{}", task.getTaskId(),
            task.getAgents().stream().map(Endpoint::getId).collect(Collectors.joining(",")));
    }

    // 多集群场景过滤掉非本节点的内置代理
    private List<Endpoint> filterAvailableAgent(List<Endpoint> endpointList) {
        return endpointList.stream().filter(this::isAvailableAgent).collect(Collectors.toList());
    }

    private boolean isAvailableAgent(Endpoint endpoint) {
        Optional<ProtectedEnvironment> agentOpt = resourceService.getResourceById(false, endpoint.getId())
            .filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource);
        if (!agentOpt.isPresent()) {
            log.error("Agent is not found, uuid: {}.", endpoint.getId());
            return false;
        }
        ProtectedEnvironment agent = agentOpt.get();
        String status = EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(agent);
        if (!LinkStatusEnum.ONLINE.getStatus().toString().equals(status)) {
            log.error("Agent status is offline, uuid: {}, status: {}.", endpoint.getId(), status);
            return false;
        }
        if (!memberClusterService.clusterEstablished()) {
            return true;
        }
        boolean isInternalAgent = AgentTypeEnum.INTERNAL_AGENT.getValue()
            .equals(agent.getExtendInfoByKey(INTERNAL_AGENT_KEY));
        if (!isInternalAgent) {
            return true;
        }
        return memberClusterService.getCurrentClusterEsn().equals(agent.getExtendInfoByKey(INTERNAL_AGENT_ESN));
    }

    private boolean lockResourceSuccess(CopyVerifyTask task) {
        final boolean isSuccess = copyVerifyService.lockResource(task.getRequestId(), task.getTaskId(),
                task.getCopyId());
        if (!isSuccess) {
            handleFailed(task, ProviderJobStatusEnum.FAIL, false);
            return Boolean.FALSE;
        }
        return Boolean.TRUE;
    }

    /**
     * 任务完成步骤.
     * <p>
     * 当副本校验任务作为子任务时，requestId和taskId不相同。</br>
     * 作为主任务时，requestId和taskId相同 </br>
     * </p>
     *
     * @param taskCompleteMessage 任务完成信息
     * @param task 副本校验任务新
     */
    @Retryable(label = "copyVerifyComplete")
    public void complete(TaskCompleteMessageBo taskCompleteMessage, CopyVerifyTask task) {
        ProviderJobStatusEnum jobStatus = JobDataConverter
                .convertToProviderJobStatus(taskCompleteMessage.getJobStatus());
        jobLogRecorder.recordJobStep(taskCompleteMessage.getJobRequestId(),
                jobLogRecorder.getLevelByJobStatus(jobStatus), CopyVerifyJobLabelConstant.COPY_CHECK_COMPLETE,
                JobLogRecorder.buildStepParam(jobStatus));
        if (jobStatus.checkSuccess()) {
            handleSuccess(task, jobStatus);
        } else {
            handleFailed(task, jobStatus,
                    taskCompleteMessage.getBooleanFromExtendsInfo(TaskCompleteMessageBo.ExtendsInfoKeys.IS_DAMAGED));
        }
    }

    /**
     * 任务完成步骤.
     * <p>
     * 当副本校验任务作为子任务时，requestId和taskId不相同。</br>
     * 作为主任务时，requestId和taskId相同 </br>
     * </p>
     *
     * @param task 副本校验任务信息
     * @param status 任务状态
     */
    private void handleSuccess(CopyVerifyTask task, ProviderJobStatusEnum status) {
        final String requestId = task.getRequestId();
        final String taskId = task.getTaskId();
        log.info("Copy Verify successful. requestId={}, taskId={}", requestId, taskId);
        if (!task.isSubTask()) {
            // 子任务不结束
            copyVerifyService.completeJob(requestId, status);
        }
        // 更新副本校验状态为校验成功
        copyVerifyService.modifyCopyStatus(task.getCopyId(), CopyStatus.NORMAL);
        copyVerifyService.modifyCopyVerifyStatus(task.getCopyId(), CopyVerifyStatusEnum.VERIFY_SUCCESS);
        // 更新最后校验时间
        copyVerifyService.modifyLastVerifyTime(task.getCopyId());
        copyVerifyService.unlockResource(requestId, taskId, !task.isSubTask());
        if (!task.isSubTask()) {
            return;
        }
        // 子任务执行完需要回到主任务继续执行
        log.info("Copy Verify successful. continue to main task. requestId={}, taskId={}", requestId, taskId);
        copyVerifyService.executeRestoreTask(requestId);
    }

    private void handleFailed(CopyVerifyTask task, ProviderJobStatusEnum status, boolean isDamaged) {
        // 任务失败直接结束，无论是否是子任务
        final String requestId = task.getRequestId();
        final String taskId = task.getTaskId();
        log.info("Copy Verify failed. requestId={}, taskId={}", requestId, taskId);
        copyVerifyService.completeJob(requestId, status);
        // 副本文件损坏时，更新副本状态为无效
        copyVerifyService.modifyCopyStatus(task.getCopyId(), isDamaged ? CopyStatus.INVALID : CopyStatus.NORMAL);
        copyVerifyService.modifyCopyVerifyStatus(task.getCopyId(), CopyVerifyStatusEnum.VERIFY_FAILED);
        // 更新最后校验时间
        copyVerifyService.modifyLastVerifyTime(task.getCopyId());
        copyVerifyService.unlockResource(requestId, task.getTaskId(), true);
        // 后置处理
        copyVerifyService.postProcess(task, isDamaged);
    }
}
