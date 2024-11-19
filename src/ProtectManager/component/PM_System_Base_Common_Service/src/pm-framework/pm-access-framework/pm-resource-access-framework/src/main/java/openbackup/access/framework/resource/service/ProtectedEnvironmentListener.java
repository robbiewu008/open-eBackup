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
package openbackup.access.framework.resource.service;

import static openbackup.data.protection.access.provider.sdk.backup.ResourceExtendInfoConstants.CONNECTION_RESULT_KEY;

import com.huawei.oceanprotect.base.cluster.sdk.dto.MemberClusterBo;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.system.base.schedule.annotation.MessageDrivenSchedule;

import com.fasterxml.jackson.core.type.TypeReference;
import com.google.common.collect.ImmutableList;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.service.provider.AgentDefaultLinkStatusProvider;
import openbackup.access.framework.resource.util.ResourceConstant;
import openbackup.access.framework.resource.util.ResourceThreadPoolTool;
import openbackup.data.access.framework.core.common.exception.LockNotObtainedException;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.agent.AgentLinkStatusProvider;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentConnectionResult;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.model.ResourceScanDto;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoInternalAlarm;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.DateFormatUtil;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.query.Pagination;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.resource.EnvironmentScanRestApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.AdapterUtils;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.RedisContextService;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.time.Duration;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Protected Environment Listener
 *
 */
@Slf4j
@Component
public class ProtectedEnvironmentListener implements InitializingBean {
    /**
     * 扫描受保护环境Kafka topic
     */
    public static final String SCANNING_ENVIRONMENT_V2 = "Sanning_environment_v2";

    /**
     * 需要进行健康检查的类型(不可变集合)
     */
    public static final ImmutableList<Integer> NEED_TO_HEALTH_CHECK_TYPES = ImmutableList.of(
        LinkStatusEnum.ONLINE.getStatus(), LinkStatusEnum.OFFLINE.getStatus(), LinkStatusEnum.UNAVAILABLE.getStatus(),
        LinkStatusEnum.DEGRADED.getStatus(), LinkStatusEnum.UNSTARTED.getStatus(),
        LinkStatusEnum.PARTLY_ONLING.getStatus());

    /**
     * 需要转发到protection service进行手动扫描的类型(不可变集合)
     */
    public static final ImmutableList<String> NEED_FORWARD_TO_PROTECTION_SERVICE_TYPE = ImmutableList.of(
        ResourceSubTypeEnum.VCENTER.getType(), ResourceSubTypeEnum.ESX.getType(), ResourceSubTypeEnum.ESXI.getType());

    private static final String ENVIRONMENT_HEALTH_CHECK = "environment-health-check";
    private static final int ENVIRONMENT_HEALTH_CHECK_PAGE_SIZE = 500;
    private static final String ENV_ID = "envId";
    private static final String MANUAL_SCAN_JOB_LABEL = "job_log_environment_scan_label";
    private static final String JOB_STATUS_FAIL_LABEL = "job_status_fail_label";
    private static final String JOB_STATUS_SUCCESS_LABEL = "job_status_success_label";
    private static final String JOB_STATUS_PARTIAL_SUCCESS_LABEL = "job_status_partial_success_label";

    private static final int SLEEP_WAIT_LOCK_SHUT_DOWN = 2 * 60 * 1000;

    /**
     * 健康状态异常告警码
     */
    private static final String HEALTH_ALARM_CODE = "0x6403320005";

    /**
     * vmware主机健康状态异常告警码
     */
    private static final String VMWREA_HEALTH_ALARM_CODE = "0x6403320002";

    /**
     * 安全一体机健康状态异常告警码
     */
    private static final String OCEAN_CYBER_HEALTH_ALARM_CODE = "0x6403320008";

    /**
     * 5分钟更新资源的状态的表达式
     */
    private static final String UPDATE_RESOURCE_STATUS_CRON = "0 0/5 * * * ? ";

    /**
     * 5分钟更新单节点的redis锁
     */
    private static final String UPDATE_RESOURCE_STATUS_KEY = "/resource/update_resource_status";

    /**
     * 资源开始扫描任务label
     */
    private static final String START_SCAN_JOB_LABEL = "job_scan_env_start_label";

    /**
     * redis的更新资源状态的过期锁时间 3分钟
     */
    private static final long UPDATE_RESOURCE_STATUS_REDIS_EXPIRED_TIME = 3 * 60;

    private final ProtectedEnvironmentServiceImpl protectedEnvironmentService;
    private final ProtectedResourceRepository protectedResourceRepository;
    private final MessageTemplate<String> messageTemplate;
    private final ProviderManager providerManager;
    private final ResourceService resourceService;

    private JobCenterRestApi jobCenterRestApi;
    private EnvironmentScanRestApi environmentScanRestApi;
    private CommonAlarmService commonAlarmService;
    private RedisContextService redisContextService;
    private LockService lockService;
    private JobService jobService;
    private ResourceScanService resourceScanService;

    private DeployTypeService deployTypeService;

    private MemberClusterService memberClusterService;

    private ResourceExtendInfoService resourceExtendInfoService;

    private AgentDefaultLinkStatusProvider agentDefaultLinkStatusProvider;

    private RedissonClient redissonClient;

    /**
     * constructor
     *
     * @param protectedEnvironmentService protectedEnvironmentService
     * @param protectedResourceRepository protectedResourceRepository
     * @param messageTemplate messageTemplate
     * @param providerManager providerManager
     * @param resourceService resource service
     */
    public ProtectedEnvironmentListener(
        ProtectedEnvironmentServiceImpl protectedEnvironmentService,
        ProtectedResourceRepository protectedResourceRepository,
        MessageTemplate<String> messageTemplate,
        ProviderManager providerManager,
        ResourceService resourceService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.protectedResourceRepository = protectedResourceRepository;
        this.messageTemplate = messageTemplate;
        this.providerManager = providerManager;
        this.resourceService = resourceService;
    }

    @Autowired
    public void setJobCenterRestApi(JobCenterRestApi jobCenterRestApi) {
        this.jobCenterRestApi = jobCenterRestApi;
    }

    @Autowired
    public void setCommonAlarmService(CommonAlarmService commonAlarmService) {
        this.commonAlarmService = commonAlarmService;
    }

    @Autowired
    public void setRedisContextService(RedisContextService redisContextService) {
        this.redisContextService = redisContextService;
    }

    @Autowired
    public void setLockService(LockService lockService) {
        this.lockService = lockService;
    }

    @Autowired
    public void setJobService(JobService jobService) {
        this.jobService = jobService;
    }

    @Autowired
    public void setResourceScanService(ResourceScanService resourceScanService) {
        this.resourceScanService = resourceScanService;
    }

    @Autowired
    public void setDeployTypeService(DeployTypeService deployTypeService) {
        this.deployTypeService = deployTypeService;
    }

    @Autowired
    public void setEnvironmentScanRestApi(EnvironmentScanRestApi environmentScanRestApi) {
        this.environmentScanRestApi = environmentScanRestApi;
    }

    @Autowired
    public void setMemberClusterService(MemberClusterService memberClusterService) {
        this.memberClusterService = memberClusterService;
    }

    @Autowired
    public void setResourceExtendInfoService(ResourceExtendInfoService resourceExtendInfoService) {
        this.resourceExtendInfoService = resourceExtendInfoService;
    }

    @Autowired
    public void setAgentDefaultLinkStatusProvider(AgentDefaultLinkStatusProvider agentDefaultLinkStatusProvider) {
        this.agentDefaultLinkStatusProvider = agentDefaultLinkStatusProvider;
    }

    @Autowired
    public void setRedisTemplate(RedissonClient redissonClient) {
        this.redissonClient = redissonClient;
    }

    /**
     * 处理定时扫描受保护环境消息
     *
     * @param message kafka消息
     * @param acknowledgment Acknowledgment
     */
    @ExterAttack
    @KafkaListener(
        groupId = "scanningConsumeGroup",
        topics = {SCANNING_ENVIRONMENT_V2},
        containerFactory = "batchFactory")
    public void handleScanEnvMessage(String message, Acknowledgment acknowledgment) {
        String envId = JSONObject.fromObject(message).getString("uuid");
        acknowledgment.acknowledge();
        if (envId == null) {
            log.error("Sanning_environment_v2 message has not uuid");
            return;
        }
        log.info("Receive scan environment message! envId:{}", envId);
        ProtectedResource protectedResource;
        try {
            protectedResource = resourceService.getResourceById(envId)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not exist"));
        } catch (LegoCheckedException e) {
            log.error("get environment by id {} failed", envId, e);
            return;
        }
        if (resourceService.checkEnvScanTaskIsRunning(envId)) {
            log.warn("Same rescan task is running, res id: {}", envId);
            return;
        }
        try {
            Consumer<ResourceScanDto> afterScan = (ResourceScanDto resourceScanDto) -> {
                List<JobBo> jobBos = resourceScanService.queryManualScanRunningJobByResId(envId);
                if (jobBos.size() > 0) {
                    JobBo jobBo = jobBos.get(0);
                    JSONObject data = new JSONObject();
                    data.put("job_id", jobBo.getJobId());
                    data.put("request_id", jobBo.getJobId());
                    data.put("job_type", MANUAL_SCAN_JOB_LABEL);

                    handleResourceScanDto(resourceScanDto, data);
                    log.info("schedule scan update job. job id: {}, res id: {}", jobBo.getJobId(), envId);
                }
            };
            ResourceThreadPoolTool.getPool()
                .execute(() -> resourceService.scanProtectedResource(protectedResource, false, afterScan));
        } catch (Throwable exception) {
            log.error("scan protected environment({}) failed", protectedResource.getUuid(), exception);
        }
    }

    /**
     * handle environment scan job
     *
     * @param message message
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(
        topics = {ResourceConstant.JOB_SCHEDULE_PREFIX + ResourceConstant.MANUAL_SCAN_RESOURCE},
        containerFactory = "retryFactory")
    public void handleEnvironmentScanJob(String message, Acknowledgment acknowledgment) {
        JSONObject data = JSONObject.fromObject(message);
        String resId = data.getString(ResourceConstant.RES_ID);
        log.info("receive manual scan message, resId:{}", resId);
        String jobId = data.getString(ResourceConstant.JOB_ID);
        if (!jobService.isJobPresent(jobId)) {
            return;
        }

        // 检查任务是否完成
        if (resourceScanService.jobIsFinished(jobId)) {
            return;
        }
        ProtectedResource resource;
        try {
            resource = resourceService.getResourceById(resId)
                .orElseThrow(
                    () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected resource is not exists!"));
        } catch (LegoCheckedException exception) {
            manualTerminateEnvironmentScanJob(data, JobStatusEnum.FAIL, exception);
            return;
        }

        // vmware手动资源扫描需要转发到protection service进行
        if (NEED_FORWARD_TO_PROTECTION_SERVICE_TYPE.contains(resource.getSubType())) {
            log.info("manual scan need to forward to protection service,jobId: {}", jobId);
            try {
                environmentScanRestApi.doScanResource(resId, jobId, resource.getSubType());
            } catch (FeignException | LegoCheckedException | LegoUncheckedException ex) {
                log.error("request protection service to scan resource failed,jobId: {}", jobId);
                manualTerminateEnvironmentScanJob(data, JobStatusEnum.FAIL, ex instanceof LegoCheckedException
                    ? (LegoCheckedException) ex : new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, ex));
            }
            return;
        }
        updateJobStatus(data);

        // 单独开线程进行消费：将手动扫描逻辑和kafka的提交拆开，避免消费能力瓶颈导致kafka重复消费和堆积
        ResourceThreadPoolTool.getPool().execute(() -> doScanProtectedResource(data, resource));
    }

    private void updateJobStatus(JSONObject data) {
        String requestId = data.getString("request_id");
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setStatus(JobStatusEnum.RUNNING);
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setJobId(requestId);
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setUnique(true);
        jobLogBo.setLogInfo(START_SCAN_JOB_LABEL);
        jobLogBo.setLevel(JobLogLevelEnum.INFO.getValue());
        updateJobRequest.setJobLogs(Collections.singletonList(jobLogBo));
        jobCenterRestApi.updateJob(requestId, updateJobRequest);
    }

    private void doScanProtectedResource(JSONObject data, ProtectedResource resource) throws LegoCheckedException {
        String jobId = data.getString("job_id");
        // 检查任务是否完成
        if (resourceScanService.jobIsFinished(jobId)) {
            return;
        }
        String resId = data.getString(ResourceConstant.RES_ID);
        // 由于开启了异步线程，需要使用jobId来跟踪定位
        log.info("manual scan begin, resource id:{}, jobId:{}", resId, jobId);
        ResourceScanDto resourceScanDto = new ResourceScanDto();
        resourceScanDto.setResource(resource);
        try {
            resourceService.scanProtectedResource(resource, true, null);
        } catch (LockNotObtainedException exception) {
            // 获取不到资源锁,说明资源在定时运行,直接返回
            log.warn("manual scan can not obtain lock. res id: {}", resId);
            return;
        } catch (Exception e) {
            resourceScanDto.setException(e);
        }
        handleResourceScanDto(resourceScanDto, data);
    }

    private void handleResourceScanDto(ResourceScanDto resourceScanDto, JSONObject data) {
        ProtectedResource resource = resourceScanDto.getResource();
        Exception exception = resourceScanDto.getException();
        if (exception == null) {
            // 部分成功
            if (isPartlySuccess(resource)) {
                dealPartlySuccess(data, resource.getUuid(), resource);
                return;
            }
            // 扫描成功
            manualTerminateEnvironmentScanJob(data, JobStatusEnum.SUCCESS, null);
            log.info("scan resource success, resource id:{}", resource.getUuid());
        } else {
            log.error("scan resource failed, resource id:" + resource.getUuid(),
                    ExceptionUtil.getErrorMessage(exception));
            manualTerminateEnvironmentScanJob(data, JobStatusEnum.FAIL, LegoCheckedException.cast(exception));
        }
    }

    /**
     * 受保护环境健康状态检查。间隔时间：5分钟
     *
     */
    @Scheduled(cron = UPDATE_RESOURCE_STATUS_CRON)
    public void handleEnvironmentHealthCheck() {
        boolean isSuccess = redissonClient.getBucket(UPDATE_RESOURCE_STATUS_KEY).setIfAbsent(
                DateFormatUtil.format(Constants.SIMPLE_DATE_FORMAT, new Date()),
                Duration.ofSeconds(UPDATE_RESOURCE_STATUS_REDIS_EXPIRED_TIME));
        if (!isSuccess) {
            log.info("Other Node executed.");
            releaseAbnormalRedisLock();
            return;
        }
        triggerEnvironmentHealthCheck();
    }

    private void releaseAbnormalRedisLock() {
        // Redis锁加固，如果key设置的超时时间大于程序所设定超时时间，此时环境时间可能被修改过，释放此异常的redis锁
        long expire = redissonClient.getBucket(UPDATE_RESOURCE_STATUS_KEY).remainTimeToLive();
        if (expire > UPDATE_RESOURCE_STATUS_REDIS_EXPIRED_TIME) {
            log.info("Environment health check redis expire time is abnormal.Expire time should be: {}, current is :{}",
                UPDATE_RESOURCE_STATUS_REDIS_EXPIRED_TIME, expire);
            redissonClient.getBucket(UPDATE_RESOURCE_STATUS_KEY).delete();
        }
    }

    private void triggerEnvironmentHealthCheck() {
        log.info("resource thread num:{}, queue num:{}",
            ResourceThreadPoolTool.getThreadNum(),
            ResourceThreadPoolTool.getQueueSzie());
        if (ResourceThreadPoolTool.isBusy()) {
            log.info("ResourceThreadPool is too busy, do not trigger environment health check this time!");
            return;
        }

        List<String> items;
        int pageNum = 0;
        // 触发所有环境的健康检查，分页触发，每页最多500条
        do {
            Pagination<JSONObject> pagination =
                new Pagination<>(
                    pageNum,
                    ENVIRONMENT_HEALTH_CHECK_PAGE_SIZE,
                    new JSONObject().set("discriminator", ProtectedResourcePo.ENVIRONMENTS_DISCRIMINATOR),
                    null);
            BasePage<String> page = protectedResourceRepository.queryResourceUuids(pagination);
            items = page.getItems();
            log.info("trigger environment check, size:{}", items.size());
            for (String item : items) {
                messageTemplate.send(ENVIRONMENT_HEALTH_CHECK, new JSONObject().set(ENV_ID, item));
            }
            // 翻页
            pageNum++;
        } while (items.size() == ENVIRONMENT_HEALTH_CHECK_PAGE_SIZE);
    }

    /**
     * handle environment health check
     *
     * @param message message
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = ENVIRONMENT_HEALTH_CHECK, groupId = MessageDrivenSchedule.MESSAGE_DRIVEN_SCHEDULE_GROUP)
    public void handleEnvironmentHealthCheck(String message, Acknowledgment acknowledgment) {
        String envId = JSONObject.fromObject(message).getString(ENV_ID);
        log.info("kafka consumer environment health check, envId:{}", envId);
        ResourceThreadPoolTool.getPool().execute(() -> doEnvironmentHealthCheck(envId));
    }

    private void doEnvironmentHealthCheck(String envId) {
        ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(envId);

        EnvironmentProvider provider =
            providerManager.findProvider(EnvironmentProvider.class, environment.getSubType(), null);
        String linkStatus = EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(environment);
        if (StringUtils.isBlank(linkStatus) || !NEED_TO_HEALTH_CHECK_TYPES.contains(Integer.valueOf(linkStatus))) {
            log.info("Environment({}) status({}) can not health check.", envId, linkStatus);
            return;
        }
        if (provider == null) {
            log.debug("not support handle health check for environment: {}", envId);
            return;
        }
        String currentClusterEsn = memberClusterService.getCurrentClusterEsn();
        try {
            LinkStatusEnum currentNodeLinkStatus = getEnvironmentLinkStatus(environment, provider);
            handleHealthCheckResult(environment, currentNodeLinkStatus, currentClusterEsn);
            triggerHealthCheckAlarm(envId, currentNodeLinkStatus);
        } catch (DataProtectionAccessException e) {
            log.error("check environment({}) health failed", envId, ExceptionUtil.getErrorMessage(e));
            handleHealthCheckResult(environment, LinkStatusEnum.OFFLINE, currentClusterEsn);
            triggerHealthCheckAlarm(envId, LinkStatusEnum.OFFLINE);
        } catch (Throwable e) {
            log.error("check environment({}) health occur an unknown error.", envId, ExceptionUtil.getErrorMessage(e));
            handleHealthCheckResult(environment, LinkStatusEnum.OFFLINE, currentClusterEsn);
            triggerHealthCheckAlarm(envId, LinkStatusEnum.OFFLINE);
        }
    }

    private void handleHealthCheckResult(ProtectedEnvironment environment, LinkStatusEnum status,
                                         String currentClusterEsn) {
        // 更新本地节点与agent联调状态
        String envId = environment.getUuid();
        Map<String, EnvironmentConnectionResult> connectionResultMap = getConnectionResultMapByEnvId(envId);
        connectionResultMap.put(currentClusterEsn, getAgentConnectionResultBy(currentClusterEsn, status.getStatus()));
        resourceExtendInfoService.saveOrUpdateExtendInfo(envId,
                Collections.singletonMap(CONNECTION_RESULT_KEY, JsonUtil.json(connectionResultMap)));

        String connectionResult =
            resourceExtendInfoService.queryExtendInfo(envId, CONNECTION_RESULT_KEY).get(CONNECTION_RESULT_KEY);
        if (!ClusterEnum.BackupRoleTypeEnum.PRIMARY.getBackupRoleType()
                .equals(memberClusterService.getCurrentClusterRole())) {
            return;
        }
        if (!deployTypeService.isE1000()) {
            // 主节点更新Agent状态
            updateEnvironmentStatusOnMasterNode(environment);
        } else {
            Set<Integer> statusSet = new HashSet<>();
            statusSet.add(status.getStatus());
            summaryAndUpdateLinkStatus(environment, statusSet);
        }
        log.info("Update connect status success. envId:{}, status:{}, result:{}.",
                envId, status.name(), connectionResult);
    }

    private EnvironmentConnectionResult getAgentConnectionResultBy(String esn, Integer status) {
        String clusterIp = "";
        String clusterName = "";
        if (!deployTypeService.isE1000()) {
            MemberClusterBo currentCluster = memberClusterService.getMemberClusterByEsn(esn);
            clusterIp = currentCluster.getClusterIp();
            clusterName = currentCluster.getClusterName();
        }
        EnvironmentConnectionResult environmentConnectionResult = new EnvironmentConnectionResult();
        environmentConnectionResult.setLinkStatus(status);
        environmentConnectionResult.setEndPoint(clusterIp);
        environmentConnectionResult.setClusterName(clusterName);
        return environmentConnectionResult;
    }

    private void updateEnvironmentStatusOnMasterNode(ProtectedEnvironment environment) {
        String envId = environment.getUuid();
        // 清除资源扩展表里的垃圾数据
        Map<String, EnvironmentConnectionResult> connectionResultMap = getConnectionResultMapByEnvId(envId);
        Iterator<Map.Entry<String, EnvironmentConnectionResult>> connectionResultIterator =
            connectionResultMap.entrySet().iterator();
        List<MemberClusterBo> allMemberClusters = memberClusterService.getAllMemberClusters();
        Set<String> allMemberEsnSet = allMemberClusters.stream().map(MemberClusterBo::getRemoteEsn)
                .collect(Collectors.toSet());

        // 将当前在线的节点过滤出来。更新资源状态时，仅汇总在线节点的状态数据
        Set<String> onlineMemberEsnSet = allMemberClusters.stream()
                .filter(cluster -> ClusterEnum.StatusEnum.isOnline(cluster.getStatus()))
                .map(MemberClusterBo::getRemoteEsn).collect(Collectors.toSet());
        Set<Integer> agentStatusSet = new HashSet<>();

        // 保存原数据数目，待遍历完成，去掉垃圾数据后做对比。数目发生变化时将新数据重新保存
        int oldConnectionResultSize = connectionResultMap.size();
        while (connectionResultIterator.hasNext()) {
            Map.Entry<String, EnvironmentConnectionResult> nodeStatus = connectionResultIterator.next();
            if (!allMemberEsnSet.contains(nodeStatus.getKey())) {
                log.info("The node(esn : {}) does not exist. The connection status of the node will be removed.",
                        nodeStatus.getKey());
                connectionResultIterator.remove();
                continue;
            }

            // 将联通状态保存到Set，方便后面汇总
            addStatusWhileMemberOnline(onlineMemberEsnSet, agentStatusSet, nodeStatus);
        }
        if (connectionResultMap.size() != oldConnectionResultSize) {
            log.info("When summarizing the Agent health status, the primary node detects junk data "
                    + "and deletes junk data.");
            resourceExtendInfoService.saveOrUpdateExtendInfo(envId,
                    Collections.singletonMap(CONNECTION_RESULT_KEY, JsonUtil.json(connectionResultMap)));
        }

        summaryAndUpdateLinkStatus(environment, agentStatusSet);
    }

    private void summaryAndUpdateLinkStatus(ProtectedEnvironment environment, Set<Integer> agentStatusSet) {
        String envId = environment.getUuid();

        // 汇总Agent健康状态并更新
        AgentLinkStatusProvider provider = providerManager.findProviderOrDefault(AgentLinkStatusProvider.class,
                environment, agentDefaultLinkStatusProvider);

        List<LinkStatusEnum> linkStatusOrderList = provider.getLinkStatusOrderList();
        for (LinkStatusEnum linkStatus : linkStatusOrderList) {
            if (agentStatusSet.contains(linkStatus.getStatus())) {
                log.info("update env link status, envId: {}, status :{}", envId, linkStatus);
                updateEnvironmentLinkStatus(envId, linkStatus);
                return;
            }
        }
    }

    private void addStatusWhileMemberOnline(Set<String> onlineMemberEsnSet, Set<Integer> agentStatusSet,
            Map.Entry<String, EnvironmentConnectionResult> nodeStatus) {
        if (onlineMemberEsnSet.contains(nodeStatus.getKey())) {
            agentStatusSet.add(nodeStatus.getValue().getLinkStatus());
        }
    }

    private Map<String, EnvironmentConnectionResult> getConnectionResultMapByEnvId(String envId) {
        String connectionResult = resourceExtendInfoService.queryExtendInfo(envId, CONNECTION_RESULT_KEY)
                .get(CONNECTION_RESULT_KEY);
        Map<String, EnvironmentConnectionResult> resultMap = JsonUtil.read(connectionResult,
                new TypeReference<Map<String, EnvironmentConnectionResult>>() {
                });
        return VerifyUtil.isEmpty(resultMap) ? new HashMap<>() : resultMap;
    }

    private void triggerHealthCheckAlarm(String envId, LinkStatusEnum currentNodeLinkStatus) {
        ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(envId);
        // 主机资源健康检查时，每个节点需要上报与主机连通性的告警/恢复, 根据当前节点的连通状态
        // 其他应用资源，判断是否上报告警时，根据汇总结果（当前环境状态）上报
        LinkStatusEnum linkStatus = ResourceTypeEnum.HOST.equalsType(environment.getType()) ? currentNodeLinkStatus
            : LinkStatusEnum.getByStatus(Integer.parseInt(environment.getLinkStatus()));
        // 根据连通状态上报告警/恢复
        if (LinkStatusEnum.ONLINE.equals(linkStatus)) {
            clearHealthCheckAlarm(environment);
        } else {
            generateHealthCheckAlarm(environment);
        }
    }

    private LinkStatusEnum getEnvironmentLinkStatus(ProtectedEnvironment environment,
        EnvironmentProvider provider) {
        Optional<String> statusOptional = provider.healthCheckWithResultStatus(environment);
        return statusOptional
            .map(Objects::requireNonNull)
            .map(statusStr -> LinkStatusEnum.getByStatus(Integer.valueOf(statusStr)))
            .orElse(LinkStatusEnum.ONLINE);
    }

    private void clearHealthCheckAlarm(ProtectedEnvironment environment) {
        commonAlarmService.clearAlarm(genHealthAlarm(environment));
    }

    private void generateHealthCheckAlarm(ProtectedEnvironment environment) {
        if (!canSendHealthAlarm(environment)) {
            log.info("Current node won't send alarm to DM. Env id : {}, env type: {}, current node role: {}",
                environment.getUuid(), environment.getType(), memberClusterService.getCurrentClusterRole());
            return;
        }
        commonAlarmService.generateAlarm(genHealthAlarm(environment));
    }

    private boolean canSendHealthAlarm(ProtectedEnvironment environment) {
        // 资源类型为主机，可以在所有节点上报告警
        if (ResourceTypeEnum.HOST.equalsType(environment.getType())) {
            return true;
        }
        // 资源类型为其他，只允许在主节点上报告警
        return ClusterEnum.BackupRoleTypeEnum.PRIMARY.getBackupRoleType()
            .equals(memberClusterService.getCurrentClusterRole());
    }

    private LegoInternalAlarm genHealthAlarm(ProtectedEnvironment environment) {
        LegoInternalAlarm legoInternalAlarm = new LegoInternalAlarm();
        legoInternalAlarm.setAlarmTime(System.currentTimeMillis() / IsmNumberConstant.THOUSAND);
        legoInternalAlarm.setAlarmSequence(IsmNumberConstant.TWO);
        legoInternalAlarm.setMoIp("127.0.0.1");
        legoInternalAlarm.setMoName("Resource");
        String subTypeOrder = null;
        if (deployTypeService.isCyberEngine()) {
            legoInternalAlarm.setAlarmId(OCEAN_CYBER_HEALTH_ALARM_CODE);
            subTypeOrder = AdapterUtils.convertSubType(environment.getSubType());
        } else {
            if (ResourceSubTypeEnum.VM_BACKUP_AGENT.getType().equals(environment.getSubType())) {
                legoInternalAlarm.setAlarmId(VMWREA_HEALTH_ALARM_CODE);
            } else {
                legoInternalAlarm.setAlarmId(HEALTH_ALARM_CODE);
                subTypeOrder = String.valueOf(ResourceSubTypeEnum.getOrderBySubTypeSilent(environment.getSubType()));
            }
        }
        if (ResourceSubTypeEnum.VM_BACKUP_AGENT.getType().equals(environment.getSubType())) {
            legoInternalAlarm.setAlarmParam(new String[]{environment.getEndpoint()});
        } else {
            legoInternalAlarm.setAlarmParam(new String[]{environment.getUuid(), environment.getName(), subTypeOrder});
        }
        legoInternalAlarm.setSourceType(FaultEnum.AlarmResourceType.RESOURCE.getValue());
        legoInternalAlarm.setResourceId(environment.getUuid());
        return legoInternalAlarm;
    }

    private void updateEnvironmentLinkStatus(String envId, LinkStatusEnum status) {
        updateEnvironmentLinkStatus(envId, status.getStatus().toString());
    }

    private void updateEnvironmentLinkStatus(String envId, String status) {
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid(envId);
        resource.setLinkStatus(status);
        // 直接将环境的状态同步到数据库，不走update的重接口，避免两次检查连通性而导致同步数据库失败
        resourceService.updateSourceDirectly(Stream.of(resource).collect(Collectors.toList()));
    }

    private void manualTerminateEnvironmentScanJob(JSONObject data, JobStatusEnum status,
        LegoCheckedException legoCheckedException) {
        String jobId = data.getString("job_id");
        if (resourceScanService.jobIsFinished(jobId)) {
            return;
        }
        String requestId = data.getString("request_id");
        String jobType = data.getString("job_type");
        // 更新日志
        updateLog(legoCheckedException, jobId, status);
        // 手动结束任务
        finishJob(status, jobId, requestId, jobType);
    }

    private void updateLog(LegoCheckedException legoCheckedException, String jobId, JobStatusEnum status) {
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setJobId(jobId);
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setLogInfo(MANUAL_SCAN_JOB_LABEL);
        if (legoCheckedException != null) {
            // 部分成功或者失败
            // 日志级别
            if (JobStatusEnum.PARTIAL_SUCCESS.equals(status)) {
                jobLogBo.setLevel(JobLogLevelEnum.WARNING.getValue());
                jobLogBo.setLogInfoParam(Collections.singletonList(JOB_STATUS_PARTIAL_SUCCESS_LABEL));
            } else {
                jobLogBo.setLevel(JobLogLevelEnum.ERROR.getValue());
                jobLogBo.setLogInfoParam(Collections.singletonList(JOB_STATUS_FAIL_LABEL));
            }
            // 错误提示
            jobLogBo.setLogDetail("" + legoCheckedException.getErrorCode());
            String[] parameters = legoCheckedException.getParameters();
            List<String> params = Optional.ofNullable(parameters).map(Arrays::asList).orElse(Collections.emptyList());
            jobLogBo.setLogDetailParam(params);
        } else {
            // 成功
            jobLogBo.setLevel(JobLogLevelEnum.INFO.getValue());
            jobLogBo.setLogInfoParam(Collections.singletonList(JOB_STATUS_SUCCESS_LABEL));
        }
        UpdateJobRequest request = new UpdateJobRequest();
        request.setJobLogs(Collections.singletonList(jobLogBo));
        jobCenterRestApi.updateJob(jobId, request);
    }

    private void finishJob(JobStatusEnum status, String jobId, String requestId, String jobType) {
        log.info(
            "job {} is finished.(request id: {}, topics: {}, job type: {}, status: {})", jobId, requestId,
            ResourceConstant.JOB_SCHEDULE_PREFIX + ResourceConstant.MANUAL_SCAN_RESOURCE, jobType, status);

        if (jobId != null) {
            jobCenterRestApi.completeJob(jobId, status);
        }
        redisContextService.delete(requestId);
    }

    private boolean isPartlySuccess(ProtectedResource resource) {
        return !VerifyUtil.isEmpty(resource.getExtendInfoByKey(Constants.ERROR_CODE));
    }

    private void dealPartlySuccess(JSONObject data, String resId, ProtectedResource resource) {
        Long errCode = Long.valueOf(resource.getExtendInfoByKey(Constants.ERROR_CODE));
        String errParam = resource.getExtendInfoByKey(Constants.ERROR_PARAM);
        String errMsg = resource.getExtendInfoByKey(Constants.ERROR_MSG);
        LegoCheckedException exception = new LegoCheckedException(errCode, new String[]{errParam}, errMsg);
        log.info("scan resource partly success, resource id:{}", resId, ExceptionUtil.getErrorMessage(exception));
        manualTerminateEnvironmentScanJob(data, JobStatusEnum.PARTIAL_SUCCESS, exception);
    }

    @Override
    public void afterPropertiesSet() throws Exception {
        List<JobBo> jobBos = resourceScanService.queryManualScanRunningPage(0, 1);
        if (VerifyUtil.isEmpty(jobBos)) {
            return;
        }
        // 睡眠等待保证锁被清理
        Thread.sleep(SLEEP_WAIT_LOCK_SHUT_DOWN);
        Lock allLock = lockService.createSQLDistributeLock(
                ResourceConstant.RESOURCE_LOCK_KEY + "afterPropertiesSetScan");
        allLock.tryLockAndRun(5, TimeUnit.SECONDS, this::updateManualScanJobWhenStart);
    }

    private void updateManualScanJobWhenStart() {
        log.info("start up. obtain lock.");
        int page = 0;
        int size = 1000;
        List<JobBo> jobBos;
        do {
            jobBos = resourceScanService.queryManualScanRunningPage(page, size);
            page++;
            for (JobBo jobBo : jobBos) {
                if (VerifyUtil.isEmpty(jobBo.getSourceId())) {
                    log.warn("startup. source id is empty. job id: {}", jobBo.getJobId());
                }
                Lock lock = lockService.createSQLDistributeLock(
                        ResourceConstant.RESOURCE_LOCK_KEY + jobBo.getSourceId());
                lock.tryLockAndRun(3, TimeUnit.SECONDS, () -> updateJobFail(jobBo));
            }
        } while (jobBos.size() == size);
    }

    private void updateJobFail(JobBo jobBo) {
        log.info("before startup update job id:{},res id: {}", jobBo.getJobId(), jobBo.getSourceId());
        if (resourceScanService.jobIsFinished(jobBo.getJobId())) {
            return;
        }

        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setStatus(JobStatusEnum.FAIL);
        JobLogBo jobLogBo = new JobLogBo();
        updateJobRequest.setJobLogs(Collections.singletonList(jobLogBo));
        jobLogBo.setJobId(jobBo.getJobId());
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setLevel(JobLogLevelEnum.ERROR.getValue());
        jobLogBo.setLogInfoParam(Collections.singletonList(JOB_STATUS_FAIL_LABEL));
        jobLogBo.setLogDetail("" + CommonErrorCode.SYSTEM_ERROR);
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setLogInfo(MANUAL_SCAN_JOB_LABEL);

        jobService.updateJob(jobBo.getJobId(), updateJobRequest);
        log.info("startup update job id:{},res id: {}", jobBo.getJobId(), jobBo.getSourceId());
    }
}
