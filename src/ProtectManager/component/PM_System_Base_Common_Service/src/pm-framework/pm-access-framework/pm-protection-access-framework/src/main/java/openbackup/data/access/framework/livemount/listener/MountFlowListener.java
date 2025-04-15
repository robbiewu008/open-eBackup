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
package openbackup.data.access.framework.livemount.listener;

import static openbackup.data.access.framework.core.common.constants.TopicConstants.EXECUTE_BACKUP_DONE;
import static openbackup.system.base.kafka.annotations.MessageListener.RETRY_FACTORY;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.livemount.TopicConstants;
import openbackup.data.access.framework.livemount.common.constants.LiveMountConstants;
import openbackup.data.access.framework.livemount.common.enums.RetentionUnit;
import openbackup.data.access.framework.livemount.common.model.LiveMountCloneRequest;
import openbackup.data.access.framework.livemount.common.model.LiveMountExecuteParam;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountStatus;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.data.access.framework.livemount.provider.DefaultLiveMountServiceProvider;
import openbackup.data.access.framework.livemount.provider.LiveMountFlowService;
import openbackup.data.access.framework.livemount.provider.LiveMountServiceProvider;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.DateFormatConstant;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.enums.WormValidityTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.license.LicenseValidateService;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.kafka.MessageObject;
import openbackup.system.base.kafka.annotations.MessageContext;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.copy.model.CopyStorageUnitStatus;
import openbackup.system.base.sdk.copy.model.CopyWormStatus;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.license.enums.FunctionEnum;
import openbackup.system.base.sdk.livemount.model.Identity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.ProviderRegistry;

import org.apache.logging.log4j.util.Strings;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

import java.text.SimpleDateFormat;
import java.time.temporal.ChronoUnit;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

/**
 * Live Mount Listener
 *
 */
@Component
@Slf4j
public class MountFlowListener extends AbstractFlowListener {
    private static final String LIVE_MOUNT_ID = "live_mount_id";

    private static final String COPY_IDS = "copy_ids";

    private static final String COPY_INDEX_UNINDEXED = "Unindexed";

    // 首次执行挂载
    private static final String DEBUTS_TRUE = "true";

    static {
        addRetentionUnit(RetentionUnit.DAY, "d", ChronoUnit.DAYS);
        addRetentionUnit(RetentionUnit.WEEK, "w", ChronoUnit.WEEKS);
        addRetentionUnit(RetentionUnit.MONTH, "MO", ChronoUnit.MONTHS);
        addRetentionUnit(RetentionUnit.YEAR, "y", ChronoUnit.YEARS);
    }

    @Autowired
    private LiveMountEntityDao liveMountEntityDao;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private LicenseValidateService licenseValidateService;

    @Autowired
    private ProviderRegistry providerRegistry;

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private DefaultLiveMountServiceProvider defaultLiveMountServiceProvider;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private JobService jobService;

    @Autowired
    private MemberClusterService memberClusterService;

    @Autowired
    private ResourceSetApi resourceSetApi;

    private static void addRetentionUnit(RetentionUnit retentionUnit, String unit, ChronoUnit chronoUnit) {
        RETENTION_UNITS.put(retentionUnit, unit);
        RETENTION_TYPES.put(retentionUnit, chronoUnit);
    }

    /**
     * execute live mount
     *
     * @param data data
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_EXECUTE_REQUEST, containerFactory = RETRY_FACTORY,
        failures = TopicConstants.LIVE_MOUNT_COMPLETE_PROCESS,
        log = {"job_log_live_mount_request_label", JOB_STATUS_LOG},
        lock = "resources:{'r'=payload.source_copy.uuid,payload.live_mount.mounted_resource_id}")
    public void requestLiveMountExecute(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        String requestId = json.getString(REQUEST_ID);
        String jobId = json.getString(JOB_ID);
        if (!jobService.isJobPresent(jobId)) {
            return;
        }
        log.info("Receive live-mount.execute.request message, jobId: {}, requestId: {}.", jobId, requestId);
        RMap<Object, Object> map = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        boolean isDebuts = Boolean.TRUE.equals(json.get(LiveMountService.LIVE_MOUNT_DEBUTS));
        map.put(LiveMountService.LIVE_MOUNT_DEBUTS, String.valueOf(isDebuts));
        LiveMountEntity liveMount = getLiveMountEntity(json);
        licenseValidateService.validate(liveMount.getResourceSubType(), FunctionEnum.LIVE_MOUNT);
        liveMountService.checkTargetEnvironmentStatus(liveMount);
        liveMountService.updateLiveMountStatus(liveMount, LiveMountStatus.MOUNTING);
        Copy sourceCopy = getSourceCopy(json);
        // 更新副本状态移除，原因要支持恢复挂载并发
        updateJobStatus(json, JobStatusEnum.RUNNING);
        Copy mountedCopy = getMountedCopy(json);

        // 如果是更新挂载，需要先卸载副本后，再做挂载
        if (mountedCopy != null) {
            log.info("Need to unmount before update live mount, jobId: {}.", jobId);
            messageTemplate.send(TopicConstants.LIVE_MOUNT_UNMOUNT_BEFORE_EXECUTE, json);
        } else {
            sendExecuteLiveMountByCopyGeneratedType(json, sourceCopy);
        }
    }

    private void sendExecuteLiveMountByCopyGeneratedType(JSONObject json, Copy sourceCopy) {
        JSONObject param = json.pick(REQUEST_ID, JOB_ID, SOURCE_COPY, LIVE_MOUNT);
        // 如果是liveMount生成的克隆副本，不需要再做克隆，直接做挂载
        if (CopyGeneratedByEnum.BY_LIVE_MOUNTE.value().equals(sourceCopy.getGeneratedBy())) {
            param.set(CLONE_COPY, sourceCopy);
            messageTemplate.send(TopicConstants.LIVE_MOUNT_EXECUTE_PROCESS_WITHOUT_CLONE_COPY, param);
        } else { // 如果是其他副本，需要做克隆后，再做挂载
            messageTemplate.send(TopicConstants.LIVE_MOUNT_COPY_CLONE, param);
        }
    }

    /**
     * unmount Before Execute
     *
     * @param data data
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_UNMOUNT_BEFORE_EXECUTE, messageContexts = {
        // 上下文栈信息
        @MessageContext(topic = TASK_COMPLETE_MESSAGE,
            messages = TopicConstants.LIVE_MOUNT_UNMOUNTED_BEFORE_EXECUTE_PROCESS + MessageContext.PAYLOAD,
            chain = MessageContext.STACK)
    }, failures = TopicConstants.LIVE_MOUNT_UNMOUNTED_BEFORE_EXECUTE_FAILED, containerFactory = RETRY_FACTORY,
        log = {"job_log_live_mount_unmount_before_live_mount_label", JOB_STATUS_LOG})
    public void unmountBeforeExecute(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        log.info("Receive unmount-before-execute message, jobId: {}.", json.getString(JOB_ID));
        if (!jobService.isJobPresent(json.getString(JOB_ID))) {
            return;
        }
        Copy mountedCopy = getMountedCopy(json);
        JSONObject param = json.pick(REQUEST_ID, JOB_ID, MOUNTED_COPY, LIVE_MOUNT, POLICY);
        param.set(RESERVE_APP, true);
        requestLiveMountUnmount(param, mountedCopy);
    }

    private Copy getMountedCopy(JSONObject json) {
        return json.getBean(MOUNTED_COPY, Copy.class);
    }

    /**
     * process live mount unmount before execute
     *
     * @param data data
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_UNMOUNTED_BEFORE_EXECUTE_PROCESS,
        containerFactory = RETRY_FACTORY, failures = TopicConstants.LIVE_MOUNT_UNMOUNTED_BEFORE_EXECUTE_FAILED,
        log = {"job_log_live_mount_unmounted_before_live_mount_label", STEP_LEVEL_STATUS_LOG})
    public void processLiveMountUnmountBeforeExecute(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        String jobId = json.getString(JOB_ID);
        if (!jobService.isJobPresent(jobId)) {
            return;
        }
        String jobStatus = json.getString(JOB_STATUS);
        log.info("Live mount receive unmounted-before-execute.process msg, jobId: {}, status: {}", jobId, jobStatus);
        JobStatusEnum status = JobStatusEnum.valueOf(jobStatus);
        if (status != JobStatusEnum.SUCCESS) {
            throw new LegoCheckedException("unmount failed");
        }

        LiveMountEntity liveMountEntity = getLiveMountEntity(json);
        Copy mountedCopy = getMountedCopy(json);
        LiveMountPolicyEntity policy = json.getBean(POLICY, LiveMountPolicyEntity.class);

        String requestId = json.getString(REQUEST_ID);
        liveMountService.cleanMountedCopyInfo(liveMountEntity);

        // 解挂载副本
        long nowTimestamp = System.currentTimeMillis();
        updateCopyStatus(mountedCopy, CopyStatus.NORMAL, true, String.valueOf(nowTimestamp * 1000),
            new SimpleDateFormat(DateFormatConstant.DATE_TIME_WITH_MILLIS).format(nowTimestamp));
        cleanOldMountedCopy(liveMountEntity, mountedCopy, policy, requestId);

        // 刷新目标资源，oracle会删除掉目标资源， vmware不会删除
        LiveMountFlowService provider = providerRegistry.findProvider(LiveMountFlowService.class,
            mountedCopy.getResourceSubType(), null);
        if (provider != null) {
            provider.checkRefreshTargetResource(jobId, liveMountEntity, 0, true);
        }
        Copy sourceCopy = getSourceCopy(json);

        // 对于更新即时挂载操作需要更新文件系统名称，统一框架业务
        liveMountClientRestApi.addLiveMountFileSystemName(
            new Identity<>(liveMountEntity.getResourceSubType(), liveMountEntity));
        json.put(LIVE_MOUNT, liveMountEntity);
        sendExecuteLiveMountByCopyGeneratedType(json, sourceCopy);
    }

    /**
     * process live mount unmount before execute fail
     *
     * @param data data
     * @param acknowledgment acknowledgment
     * @return MessageObject
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_UNMOUNTED_BEFORE_EXECUTE_FAILED,
        containerFactory = RETRY_FACTORY, failures = TopicConstants.LIVE_MOUNT_COMPLETE_PROCESS)
    public MessageObject processUnmountBeforeExecuteFailed(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        Copy sourceCopy = json.getBean(SOURCE_COPY, Copy.class);
        updateCopyStatus(sourceCopy, CopyStatus.NORMAL);
        return new MessageObject(TopicConstants.LIVE_MOUNT_COMPLETE_PROCESS, json);
    }

    /**
     * clone live mount copy
     *
     * @param data data
     * @param acknowledgment acknowledgment
     * @return topic and message
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_COPY_CLONE, containerFactory = RETRY_FACTORY,
        failures = TopicConstants.LIVE_MOUNT_COMPLETE_PROCESS,
        log = {"job_log_live_mount_copy_clone_label", JOB_STATUS_LOG}, unlock = true)
    public JSONObject processLiveMountCopyClone(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        Copy sourceCopy = getSourceCopy(json);
        LiveMountCloneRequest cloneRequest = new LiveMountCloneRequest();
        cloneRequest.setSourceCopy(sourceCopy);
        String requestId = json.getString("request_id");
        if (!jobService.isJobPresent(requestId)) {
            return new MessageObject();
        }
        log.info("Live mount receive live-mount.copy.clone msg, requestId(jobId): {}.", requestId);
        // 以request id作为克隆副本的uuid，确保消息重复消费的幂等性
        cloneRequest.setTargetCopyUuid(requestId);
        LiveMountEntity liveMountEntity = getLiveMountEntity(json);
        cloneRequest.setLiveMountEntity(liveMountEntity);
        Identity<LiveMountCloneRequest> cloneIdentity = new Identity<>(sourceCopy.getResourceSubType(), cloneRequest);
        // 更新副本状态移除，原因要支持恢复挂载并发
        CopyInfo cloneCopy = liveMountClientRestApi.cloneCopy(cloneIdentity);
        cloneCopy.setWormStatus(CopyWormStatus.UNSET.getStatus());
        LiveMountEntity liveMount = getLiveMountEntity(json);
        cloneCopy.setChainId(cloneCopy.getChainId() + "@" + liveMount.getId());
        buildCloneCopyGeneralParams(cloneCopy, sourceCopy);
        String properties = cloneCopy.getProperties();
        JSONObject cloneCopyProperties = JSONObject.fromObject(properties);
        cloneCopyProperties.put(LiveMountConstants.FILE_SYSTEM_SHARE_INFO, liveMountEntity.getFileSystemShareInfo());
        cloneCopy.setProperties(cloneCopyProperties.toString());
        if (needSetDeviceEsn(cloneCopy.getResourceSubType())) {
            cloneCopy.setDeviceEsn(memberClusterService.getCurrentClusterEsn());
        }
        // 如果是FC，HCS下发的即时挂载任务，副本索引类型设置为"不支持" : Unsupport
        boolean isTargetResourceSubType =
            ResourceSubTypeEnum.FUSION_COMPUTE.getType().equals(cloneCopy.getResourceSubType())
                || ResourceSubTypeEnum.HCS_CLOUD_HOST.getType().equals(cloneCopy.getResourceSubType())
                || ResourceSubTypeEnum.FUSION_ONE_COMPUTE.getType().equals(cloneCopy.getResourceSubType());
        if (isTargetResourceSubType) {
            cloneCopy.setIndexed(CopyIndexStatus.UNSUPPORT.getIndexStaus());
        }
        if (deployTypeService.isCyberEngine()) {
            cleanAuthPassword(cloneCopy, cloneCopyProperties);
        }
        UuidObject uuidObject = copyRestApi.saveCopy(cloneCopy);
        resourceSetApi.createCopyResourceSetRelation(uuidObject.getUuid(), sourceCopy.getUuid(), Strings.EMPTY);
        Copy copy = copyRestApi.queryCopyByID(uuidObject.getUuid());
        setRequestCache(requestId, LiveMountConstants.CLONE_COPY_ID, copy.getUuid());
        log.info("Clone copy success, requestId: {}, cloneCopyId:{}, status: {}.",
                requestId, copy.getUuid(), copy.getStatus());
        return new MessageObject(TopicConstants.LIVE_MOUNT_EXECUTE_PROCESS, json.set(CLONE_COPY, copy));
    }

    private void cleanAuthPassword(CopyInfo cloneCopy, JSONObject cloneCopyProperties) {
        List<StorageRepository> repositories = JSONArray.toCollection(
            JSONArray.fromObject(cloneCopyProperties.get(CopyPropertiesKeyConstant.KEY_REPOSITORIES)),
            StorageRepository.class);
        // 入库前清除敏感信息
        repositories.get(0).setExtendAuth(null);
        cloneCopyProperties.put(CopyPropertiesKeyConstant.KEY_REPOSITORIES, repositories);
        cloneCopy.setProperties(cloneCopyProperties.toString());
    }

    private boolean needSetDeviceEsn(String resourceSubType) {
        // DWS副本不是存储在一台集群上的，不设置设备esn
        return !Arrays.asList(ResourceSubTypeEnum.GAUSSDB_DWS.getType(),
                ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType(), ResourceSubTypeEnum.GAUSSDB_DWS_TABLE,
                ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()).contains(resourceSubType);
    }

    private void buildCloneCopyGeneralParams(CopyInfo cloneCopy, Copy sourceCopy) {
        cloneCopy.setGenerationType(LIVE_MOUNT);
        // 克隆的副本不能用于挂载和索引
        int features = getCloneCopyFeatureByResourceSubType(cloneCopy);
        cloneCopy.setFeatures(features);
        cloneCopy.setDeletable(false);
        cloneCopy.setGeneratedBy(LIVE_MOUNT);
        cloneCopy.setGeneration(cloneCopy.getGeneration() + 1);
        cloneCopy.setParentCopyUuid(sourceCopy.getUuid());
        cloneCopy.setStatus(CopyStatus.MOUNTING.getValue());
        cloneCopy.setStorageUnitStatus(CopyStorageUnitStatus.ONLINE.getValue());
        cloneCopy.setUserId(sourceCopy.getUserId());
        cloneCopy.setIndexed(COPY_INDEX_UNINDEXED);
        // set retention time
        cloneCopy.setRetentionType(PERMANENT_RETENTION);
        cloneCopy.setDurationUnit(null);
        cloneCopy.setExpirationTime(null);
        cloneCopy.setWormValidityType(WormValidityTypeEnum.WORM_NOT_OPEN.getType());
        cloneCopy.setWormDurationUnit(null);
        cloneCopy.setWormExpirationTime(null);
        Long timestamp = System.currentTimeMillis();
        cloneCopy.setTimestamp(timestamp + "000");
        cloneCopy.setGeneratedTime(timestamp + "000");
        cloneCopy.setDisplayTimestamp(Constants.SIMPLE_DATE_FORMAT.format(new Date(timestamp)));
    }

    private int getCloneCopyFeatureByResourceSubType(CopyInfo cloneCopy) {
        LiveMountFlowService provider = providerRegistry.findProvider(LiveMountFlowService.class,
            cloneCopy.getResourceSubType(), null);
        int features;
        if (provider != null) {
            features = provider.getCloneCopyFeatureByResourceSubType(cloneCopy.getFeatures());
        } else {
            features = ~(~cloneCopy.getFeatures() | CopyFeatureEnum.setAndGetFeatures(CopyFeatureEnum.INDEX,
                CopyFeatureEnum.RESTORE));
        }
        return features;
    }

    /**
     * process live mount execute
     *
     * @param data data
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_EXECUTE_PROCESS, messageContexts = {
        // 上下文栈信息
        @MessageContext(chain = MessageContext.STACK, topic = TASK_COMPLETE_MESSAGE,
            messages = TopicConstants.LIVE_MOUNT_COMPLETE_PROCESS + MessageContext.PAYLOAD)
    }, lock = "resources:{payload.clone_copy.uuid}", log = {"job_log_live_mount_execute_label", JOB_STATUS_LOG},
        containerFactory = RETRY_FACTORY, failures = TopicConstants.LIVE_MOUNT_COMPLETE_PROCESS)
    public void processLiveMountExecute(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        log.info("Receive live-mount.execute.process msg, jobId: {}.", json.getString(JOB_ID));
        if (!jobService.isJobPresent(json.getString(JOB_ID))) {
            return;
        }
        executeLiveMount(data);
    }

    /**
     * process live mount execute
     *
     * @param data data
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_EXECUTE_PROCESS_WITHOUT_CLONE_COPY, messageContexts = {
        // 上下文栈信息
        @MessageContext(chain = MessageContext.STACK, topic = TASK_COMPLETE_MESSAGE,
            messages = TopicConstants.LIVE_MOUNT_COMPLETE_PROCESS + MessageContext.PAYLOAD)
    }, log = {"job_log_live_mount_execute_label", JOB_STATUS_LOG}, containerFactory = RETRY_FACTORY,
        failures = TopicConstants.LIVE_MOUNT_COMPLETE_PROCESS)
    public void processLiveMountExecuteWithoutCloneCopy(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        log.info("Receive live-mount.execute.process.without.clone.copy msg, jobId: {}.", json.getString(JOB_ID));
        if (!jobService.isJobPresent(json.getString(JOB_ID))) {
            return;
        }
        executeLiveMount(data);
    }

    private void executeLiveMount(String data) {
        JSONObject json = JSONObject.fromObject(data);
        Copy sourceCopy = getSourceCopy(json);
        JSONObject param = json.pick(REQUEST_ID, JOB_ID, SOURCE_COPY, LIVE_MOUNT, CLONE_COPY, MOUNTED_COPY);
        LiveMountExecuteParam executeParam = param.toBean(LiveMountExecuteParam.class);
        executeParam.setJobId(executeParam.getJobId() + MOUNT_JOB_ID_LIVE_MOUNT_SUFFIX);
        Identity<LiveMountExecuteParam> identity = new Identity<>(sourceCopy.getResourceSubType(), executeParam);
        liveMountClientRestApi.executeLiveMount(identity);
    }

    /**
     * create live mount copy
     *
     * @param data data
     * @param acknowledgment acknowledgment
     * @return message object of next
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_COMPLETE_PROCESS, containerFactory = RETRY_FACTORY,
        log = {"job_log_live_mount_complete_label", TASK_LEVEL_STATUS_LOG}, unlock = true, terminatedMessage = true)
    public MessageObject completeLiveMountExecute(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        String jobStatus = json.getString(JOB_STATUS);
        String jobId = json.getString(JOB_ID);
        JobStatusEnum status = JobStatusEnum.get(jobStatus);
        LiveMountEntity liveMount = getLiveMountEntity(json);
        MessageObject messageObject = new MessageObject();
        JobStatusEnum finalStatus;
        log.info("Receive live-mount.complete.process msg, jobId: {}, jobStatus: {}.", jobId, status);
        try {
            String requestId = json.getString(REQUEST_ID);

            // 判断是否加锁失败，失败后不继续执行下面流程。
            if (isLockFail(requestId)) {
                lockFailResetLiveMountStatus(liveMount);
                return messageObject;
            }

            // 更新副本和挂载信息
            liveMount.setMountJobId(requestId);
            finalStatus = updateRelatedResourceStatus(json, status, liveMount, messageObject);

            // 按挂载策略初始化调度器
            initialSchedule(liveMount, requestId);

            // 即时挂载完结过程中给插件提供处理函数
            LiveMountServiceProvider provider = providerManager.findProviderOrDefault(LiveMountServiceProvider.class,
                liveMount.getResourceSubType(), defaultLiveMountServiceProvider);
            provider.processLiveMountTerminate(liveMount);

            // 更新live mount状态
            if (JobStatusEnum.SUCCESS.name().equals(finalStatus.name())) {
                liveMountService.updateLiveMountStatus(liveMount, LiveMountStatus.AVAILABLE);
            } else {
                liveMountService.updateLiveMountStatus(liveMount, LiveMountStatus.MOUNT_FAILED);
            }
            if (status.checkSuccess() && deployTypeService.isCyberEngine()) {
                // 安全一体机共享路径恢复任务成功后自动删除克隆副本数据
                copyRestApi.deleteBatchCopies(Arrays.asList(liveMount.getMountedCopyId()));
                liveMountService.deleteLiveMount(liveMount.getId());
            }
            if (!status.checkSuccess() && ResourceSubTypeEnum.VMWARE.equalsSubType(liveMount.getResourceSubType())
                && liveMount.isDeleteOriginalVM()) {
                // vmware即时挂载创建同名新虚拟机删除原虚拟机时，失败需要清理记录
                liveMountService.deleteLiveMount(liveMount.getId());
            }
        } catch (RuntimeException e) {
            liveMountService.updateLiveMountStatus(liveMount, LiveMountStatus.MOUNT_FAILED);
            throw e;
        }
        return messageObject.status(finalStatus);
    }

    private void lockFailResetLiveMountStatus(LiveMountEntity liveMount) {
        if (VerifyUtil.isEmpty(liveMount) || VerifyUtil.isEmpty(liveMount.getStatus())
            || LiveMountStatus.READY.getName().equals(liveMount.getStatus())) {
            liveMountService.updateLiveMountStatus(liveMount, LiveMountStatus.MOUNT_FAILED);
        }
    }

    @ExterAttack
    private void initialSchedule(LiveMountEntity liveMount, String requestId) {
        RMap<Object, Object> map = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        boolean isDebuts = DEBUTS_TRUE.equals(map.get(LiveMountService.LIVE_MOUNT_DEBUTS));
        if (isDebuts) {
            liveMountService.initialAndUpdateLiveMountSchedule(liveMount, null);
        }
    }

    private JobStatusEnum updateRelatedResourceStatus(JSONObject json, JobStatusEnum status, LiveMountEntity liveMount,
        MessageObject messageObject) {
        JobStatusEnum finalStatus;
        Copy sourceCopy = getSourceCopy(json);
        Copy mountedCopy = json.getBean(CLONE_COPY, Copy.class);
        if (!VerifyUtil.isEmpty(mountedCopy)) {
            log.debug("Live mount mountedCopyId:{}", mountedCopy.getUuid());
            liveMount.setMountedCopyId(mountedCopy.getUuid());
        }
        String jobId = json.getString(JOB_ID);
        // 如果挂载成功，更新即时挂载信息和刷新资源
        if (status.checkSuccess()) {
            updateCopyStatus(mountedCopy, CopyStatus.MOUNTED);
            liveMountService.updateMountedCopyInfo(liveMount, mountedCopy);
            finalStatus = refreshTargetResource(jobId, liveMount, IsmNumberConstant.ONE).orElse(status);
        } else if (mountedCopy != null) { // 如果挂载失败，并且有已挂载副本。
            finalStatus = status;
            updateCopyStatus(mountedCopy, CopyStatus.NORMAL, true);
            JSONObject message = JSONObject.fromObject(json.toString());
            message.remove("job_status");

            // 如果源副本是克隆副本，任务失败后不需要删除副本
            if (!CopyGeneratedByEnum.BY_LIVE_MOUNTE.value().equals(sourceCopy.getGeneratedBy())) {
                messageObject.init(TopicConstants.LIVE_MOUNT_CLEAN_CLONE_COPY, message, null, null);
            }
        } else { // 如果挂载失败
            finalStatus = status;
            updateCopyStatus(sourceCopy, CopyStatus.NORMAL, true);
        }

        // 如果成功，配置了只保留最新的副本策略，把刚刚解挂载的副本删除掉
        cleanCopyByPolicyOnlyMountSuccess(json, status, liveMount, mountedCopy);
        return finalStatus;
    }

    private void cleanCopyByPolicyOnlyMountSuccess(JSONObject json, JobStatusEnum status, LiveMountEntity liveMount,
        Copy mountedCopy) {
        LiveMountPolicyEntity policy = getLiveMountPolicy(liveMount);
        if (status.checkSuccess() && mountedCopy != null && policy != null) {
            String requestId = json.getString(REQUEST_ID);
            Copy lastMountedCopy = getCopyBasePage(mountedCopy, false, mountedCopy.getGn() - 1).one();
            if (lastMountedCopy == null) {
                return;
            }
            cleanCopyByPolicy(liveMount, lastMountedCopy, policy, requestId, true);
        }
    }

    /**
     * schedule live mount
     *
     * @param data data
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_SCHEDULE, containerFactory = RETRY_FACTORY)
    public void scheduleLiveMount(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        String liveMountId = json.getString(LIVE_MOUNT_ID);
        LiveMountEntity liveMount = liveMountEntityDao.selectById(liveMountId);

        // if live mount is disabled, forbidden schedule live mount
        boolean hasActive = liveMountService.checkHasActive(liveMount.getEnableStatus(), false);
        if (!hasActive) {
            return;
        }
        String policyId = liveMount.getPolicyId();
        if (policyId == null) {
            log.info("live mount {} is not automatic mode", liveMountId);
            return;
        }
        LiveMountPolicyEntity policy = liveMountPolicyEntityDao.selectPolicy(policyId);
        liveMountService.updateLiveMount(liveMount, policy, null, false);
    }

    /**
     * process protection backup done
     *
     * @param data data
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = EXECUTE_BACKUP_DONE, groupId = "liveMountConsumerGroup", containerFactory = RETRY_FACTORY,
        retryable = true)
    public void processProtectionBackupDone(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        Object copyIds = json.get(COPY_IDS);
        if (copyIds == null) {
            return;
        }
        if (copyIds instanceof String) {
            processProtectionBackupDone(copyIds.toString());
        }
        if (copyIds instanceof List) {
            List items = (List) copyIds;
            for (Object item : items) {
                if (item != null) {
                    String copyId = item.toString();
                    processProtectionBackupDone(copyId);
                }
            }
        }
    }

    private void processProtectionBackupDone(String copyId) {
        if (VerifyUtil.isEmpty(copyId)) {
            return;
        }
        Copy copy = copyRestApi.queryCopyByID(copyId);
        String resourceId = copy.getResourceId();
        List<LiveMountEntity> liveMountEntities = liveMountEntityDao.queryAutoUpdateWhenBackupDoneLiveMounts(
            resourceId);
        for (LiveMountEntity liveMountEntity : liveMountEntities) {
            // if live mount is disabled, forbidden schedule live mount
            boolean hasActive = liveMountService.checkHasActive(liveMountEntity.getEnableStatus(), false);
            if (!hasActive) {
                continue;
            }
            String policyId = liveMountEntity.getPolicyId();
            liveMountPolicyEntityDao.selectPolicy(policyId);
            LiveMountPolicyEntity policy = getLiveMountPolicy(liveMountEntity);
            liveMountService.updateLiveMount(liveMountEntity, policy, copyId, false);
        }
    }
}
