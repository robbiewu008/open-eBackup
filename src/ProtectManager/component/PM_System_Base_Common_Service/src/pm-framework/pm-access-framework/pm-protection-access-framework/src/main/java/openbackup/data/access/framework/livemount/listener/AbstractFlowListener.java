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

import openbackup.data.access.framework.livemount.common.LiveMountRestApi;
import openbackup.data.access.framework.livemount.common.enums.RetentionType;
import openbackup.data.access.framework.livemount.common.enums.RetentionUnit;
import openbackup.data.access.framework.livemount.common.model.LiveMountRefreshParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountUnmountParam;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyRetentionPolicy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.copy.model.CopyStatusUpdateParam;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.livemount.model.Identity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.util.MessageTemplate;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;

import java.time.temporal.ChronoUnit;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * Abstract Live Mount Flow Listener
 *
 * @author l00272247
 * @since 2020-11-05
 */
@Slf4j
public abstract class AbstractFlowListener {
    /**
     * the copy retention type is permanent retention
     */
    protected static final int PERMANENT_RETENTION = 1;

    /**
     * the copy retention type is temp retention
     */
    protected static final int TEMP_RETENTION = 2;

    /**
     * job complete topic TaskCompleteMessage
     */
    protected static final String TASK_COMPLETE_MESSAGE = "TaskCompleteMessage";

    /**
     * TASK_LEVEL_STATUS_LOG
     */
    protected static final String TASK_LEVEL_STATUS_LOG = "job_status_{context.job_status|status}_label";

    /**
     * STEP_LEVEL_STATUS_LOG
     */
    protected static final String STEP_LEVEL_STATUS_LOG = "job_status_{status}_label";

    /**
     * job status log
     */
    protected static final String JOB_STATUS_LOG = "job_status_{payload.job_status|status}_label";

    /**
     * job id
     */
    protected static final String JOB_ID = "job_id";

    /**
     * source copy
     */
    protected static final String SOURCE_COPY = "source_copy";

    /**
     * live mount
     */
    protected static final String LIVE_MOUNT = "live_mount";

    /**
     * mounted copy
     */
    protected static final String MOUNTED_COPY = "mounted_copy";

    /**
     * policy
     */
    protected static final String POLICY = "policy";

    /**
     * request id
     */
    protected static final String REQUEST_ID = "request_id";

    /**
     * reserve app
     */
    protected static final String RESERVE_APP = "reserve_app";

    /**
     * reserve copy
     */
    protected static final String RESERVE_COPY = "reserve_copy";

    /**
     * job status
     */
    protected static final String JOB_STATUS = "job_status";

    /**
     * unmount jog id live mount suffix
     */
    protected static final String UNMOUNT_JOB_ID_LIVE_MOUNT_SUFFIX = "_unmount";

    /**
     * mount job id live mount suffix
     */
    protected static final String MOUNT_JOB_ID_LIVE_MOUNT_SUFFIX = "_live_mount";

    /**
     * clone copy
     */
    protected static final String CLONE_COPY = "clone_copy";

    /**
     * current operate user id
     */
    protected static final String CURRENT_OPERATE_USER_ID = "current_operate_user_id";

    /**
     * resource lock
     */
    protected static final String LOCK = "lock";

    /**
     * resource lock status
     */
    protected static final String LOCK_STATUS = "status";

    /**
     * resource lock status fail
     */
    protected static final String LOCK_STATUS_FAIL = "fail";

    /**
     * RETENTION_UNITS
     */
    protected static final Map<RetentionUnit, String> RETENTION_UNITS = new HashMap<>();

    /**
     * RETENTION_TYPES
     */
    protected static final Map<RetentionUnit, ChronoUnit> RETENTION_TYPES = new HashMap<>();

    /**
     * copyRestApi
     */
    @Autowired
    protected CopyRestApi copyRestApi;

    /**
     * liveMountClientRestApi
     */
    @Autowired
    protected LiveMountRestApi liveMountClientRestApi;

    /**
     * jobCenterRestApi
     */
    @Autowired
    protected JobCenterRestApi jobCenterRestApi;

    /**
     * liveMountService
     */
    @Autowired
    protected LiveMountService liveMountService;

    /**
     * messageTemplate
     */
    @Autowired
    protected MessageTemplate<String> messageTemplate;

    /**
     * liveMountPolicyEntityDao
     */
    @Autowired
    protected LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    @Autowired
    private ScheduleRestApi scheduleRestApi;

    @Autowired
    private RedissonClient redissonClient;

    /**
     * get source copy
     *
     * @param json json
     * @return copy
     */
    protected Copy getSourceCopy(JSONObject json) {
        return json.getBean(SOURCE_COPY, Copy.class);
    }

    /**
     * get clone copy
     *
     * @param json json
     * @return copy
     */
    protected Copy getCloneCopy(JSONObject json) {
        return json.getBean(CLONE_COPY, Copy.class);
    }

    /**
     * update copy status
     *
     * @param copy copy
     * @param status status
     */
    protected void updateCopyStatus(Copy copy, CopyStatus status) {
        updateCopyStatus(copy, status, null, null, null);
    }

    /**
     * update copy status
     *
     * @param copy copy
     * @param status status
     * @param isDeletable deletable
     */
    protected void updateCopyStatus(Copy copy, CopyStatus status, Boolean isDeletable) {
        updateCopyStatus(copy, status, isDeletable, null, null);
    }

    /**
     * update copy status
     *
     * @param copy copy
     * @param status status
     * @param isDeletable deletable
     * @param timestamp timestamp
     * @param displayTimestamp display time
     */
    protected void updateCopyStatus(Copy copy, CopyStatus status, Boolean isDeletable, String timestamp,
        String displayTimestamp) {
        if (copy == null) {
            return;
        }
        CopyStatusUpdateParam param = new CopyStatusUpdateParam();
        param.setStatus(status);
        param.setIsDeletable(isDeletable);
        param.setTimestamp(timestamp);
        param.setDisplayTimestamp(displayTimestamp);
        param.setGeneratedTime(displayTimestamp);
        copyRestApi.updateCopyStatus(copy.getUuid(), param);
    }

    /**
     * set request cache
     *
     * @param requestId request id
     * @param key key
     * @param value value
     */
    @ExterAttack
    protected void setRequestCache(String requestId, String key, String value) {
        RMap<Object, Object> map = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        map.put(key, value);
    }

    /**
     * get request cache
     *
     * @param requestId request id
     * @param key key
     * @return the value of key
     */
    @ExterAttack
    protected Object getRequestCache(String requestId, String key) {
        RMap<Object, Object> map = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        return map.get(key);
    }

    /**
     * request live mount unmount
     *
     * @param param param
     * @param mountedCopy mounted copy
     */
    protected void requestLiveMountUnmount(JSONObject param, Copy mountedCopy) {
        LiveMountUnmountParam destroyParam = param.toBean(LiveMountUnmountParam.class);
        destroyParam.setMountedCopy(mountedCopy);
        destroyParam.setJobId(destroyParam.getJobId() + UNMOUNT_JOB_ID_LIVE_MOUNT_SUFFIX);
        Identity<LiveMountUnmountParam> identity = new Identity<>(mountedCopy.getResourceSubType(), destroyParam);
        liveMountClientRestApi.unmountLiveMount(identity);
    }

    /**
     * update job status
     *
     * @param json json
     * @param status status
     */
    protected void updateJobStatus(JSONObject json, JobStatusEnum status) {
        String jobId = json.getString(JOB_ID);
        UpdateJobRequest request = new UpdateJobRequest();
        request.setStatus(status);
        jobCenterRestApi.updateJob(jobId, request);
    }

    /**
     * update job log
     *
     * @param jobId job id
     * @param jobLogBoList job log bo list
     */
    protected void updateJobLog(String jobId, List<JobLogBo> jobLogBoList) {
        UpdateJobRequest request = new UpdateJobRequest();
        request.setJobLogs(jobLogBoList);
        jobCenterRestApi.updateJob(jobId, request);
    }

    /**
     * get live mount entity
     *
     * @param json json
     * @return live mount entity
     */
    protected LiveMountEntity getLiveMountEntity(JSONObject json) {
        return json.getBean(LIVE_MOUNT, LiveMountEntity.class);
    }

    /**
     * refresh target resource
     *
     * @param jobId job id
     * @param liveMount live mount
     * @param count count
     * @param hasClearProtection  has clear protection object
     * @return status
     */
    protected Optional<JobStatusEnum> refreshTargetResource(String jobId, LiveMountEntity liveMount, int count,
        boolean hasClearProtection) {
        LiveMountRefreshParam liveMountRefreshParam = new LiveMountRefreshParam();
        liveMountRefreshParam.setLiveMount(liveMount);
        // 是否清除对象对应的保护对象
        liveMountRefreshParam.setHasCleanProtection(hasClearProtection);

        List<String> targetResourceUuidList = liveMountClientRestApi.refreshTargetResource(
            new Identity<>(liveMount.getResourceSubType(), liveMountRefreshParam));
        int size = targetResourceUuidList != null ? targetResourceUuidList.size() : 0;
        if (size == count) {
            String targetResourceUuid = size > 0 ? targetResourceUuidList.get(0) : null;
            liveMountService.updateLiveMountMountedResource(liveMount, targetResourceUuid);
            return Optional.empty();
        } else {
            return recordFailedJobLog(jobId, liveMount.getResourceSubType());
        }
    }

    private Optional<JobStatusEnum> recordFailedJobLog(String jobId, String resourceSubType) {
        if (resourceSubType.equals(ResourceSubTypeEnum.ORACLE.getType())) {
            return Optional.of(JobStatusEnum.SUCCESS);
        }
        recordJobLog(jobId, "job_log_live_mount_update_target_resource_failed_label",
                JobLogLevelEnum.WARNING.getValue());
        return Optional.of(JobStatusEnum.FAIL);
    }

    /**
     * refresh target resource
     *
     * @param jobId job id
     * @param liveMount live mount
     * @param count count
     * @return status
     */
    protected Optional<JobStatusEnum> refreshTargetResource(String jobId, LiveMountEntity liveMount, int count) {
        return refreshTargetResource(jobId, liveMount, count, false);
    }

    private void recordJobLog(String jobId, String logInfo, String level) {
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setJobId(jobId);
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setLogInfo(logInfo);
        jobLogBo.setLogInfoParam(Collections.emptyList());
        jobLogBo.setLevel(level);
        UpdateJobRequest request = new UpdateJobRequest();
        request.setJobLogs(Collections.singletonList(jobLogBo));
        jobCenterRestApi.updateJob(jobId, request);
    }

    /**
     * clean old mounted copy
     *
     * @param liveMountEntity live mount entity
     * @param mountedCopy mounted copy
     * @param policy policy
     * @param requestId request id
     * @return has clean old copy
     */
    protected boolean cleanOldMountedCopy(LiveMountEntity liveMountEntity, Copy mountedCopy,
        LiveMountPolicyEntity policy, String requestId) {
        if (mountedCopy == null) {
            // 没有副本则认为清除成功
            return true;
        }
        return cleanCopyByPolicy(liveMountEntity, mountedCopy, policy, requestId, false);
    }

    /**
     * clean just unmounted copy
     *
     * @param liveMountEntity live mount entity
     * @param mountedCopy mounted copy
     * @param requestId request id
     * @param policy policy
     * @param hasLastedGn has contain lasted gn
     * @return has clean old copy
     */
    protected boolean cleanCopyByPolicy(LiveMountEntity liveMountEntity, Copy mountedCopy, LiveMountPolicyEntity policy,
        String requestId, boolean hasLastedGn) {
        if (VerifyUtil.isEmpty(liveMountEntity.getPolicyId())) {
            CopyRetentionPolicy retentionPolicy = new CopyRetentionPolicy();
            retentionPolicy.setRetentionType(PERMANENT_RETENTION); // 1:永久保留
            copyRestApi.updateCopyRetention(mountedCopy.getUuid(), retentionPolicy);
        } else {
            String retentionPolicy = policy.getRetentionPolicy();
            RetentionType retentionType = RetentionType.get(retentionPolicy);
            if (retentionType == RetentionType.LATEST_ONE) {
                String currentOperateUserId = String.valueOf(getRequestCache(requestId, CURRENT_OPERATE_USER_ID));
                deleteOldMountedCopy(mountedCopy, currentOperateUserId, hasLastedGn);
            } else if (retentionType == RetentionType.FIXED_TIME) {
                RetentionUnit retentionUnit = RetentionUnit.get(policy.getRetentionUnit());
                int retentionValue = policy.getRetentionValue();
                updateCopyRetention(mountedCopy, retentionUnit, retentionValue);
            } else {
                log.warn("Unexpected scene happen.");
            }
        }
        return true;
    }

    private void deleteOldMountedCopy(Copy mountedCopy, String userId, boolean hasLastedGn) {
        BasePage<Copy> data = getCopyBasePage(mountedCopy, hasLastedGn, null);
        for (Copy copy : data.getItems()) {
            copyRestApi.deleteCopy(copy.getUuid(), userId);
        }
    }

    /**
     * get copies by condition
     *
     * @param mountedCopy mounted copy
     * @param hasLastedGn has contain lasted gn
     * @param gnStart gn
     * @return copy list
     */
    public BasePage<Copy> getCopyBasePage(Copy mountedCopy, boolean hasLastedGn, Integer gnStart) {
        Map<String, Object> condition = new HashMap<>();
        condition.put("resource_id", mountedCopy.getResourceId());
        condition.put("chain_id", mountedCopy.getChainId());
        int gn = mountedCopy.getGn();
        if (!hasLastedGn && gn > 0) {
            gn = gn - 1;
        }

        condition.put("gn_range", Arrays.asList(gnStart, gn));
        return BasePage.queryAll((page, size) -> copyRestApi.queryCopies(page, size, condition));
    }

    private void updateCopyRetention(Copy mountedCopy, RetentionUnit retentionUnit, int retentionValue) {
        CopyRetentionPolicy retentionPolicy = new CopyRetentionPolicy();
        retentionPolicy.setRetentionType(TEMP_RETENTION); // 2:临时保留
        String unit = RETENTION_UNITS.get(retentionUnit);
        retentionPolicy.setDurationUnit(unit);
        retentionPolicy.setRetentionDuration(retentionValue);
        copyRestApi.updateCopyRetention(mountedCopy.getUuid(), retentionPolicy);
    }

    /**
     * get live mount policy
     *
     * @param liveMountEntity live mount entity
     * @return live mount policy entity
     */
    protected LiveMountPolicyEntity getLiveMountPolicy(LiveMountEntity liveMountEntity) {
        if (VerifyUtil.isEmpty(liveMountEntity.getPolicyId())) {
            return null;
        }
        return liveMountPolicyEntityDao.selectPolicy(liveMountEntity.getPolicyId());
    }

    /**
     * delete clone copy
     *
     * @param cloneCopyId clone copy
     * @param requestId request id
     */
    protected void deleteCloneCopy(String cloneCopyId, String requestId) {
        try {
            String currentOperateUserId = String.valueOf(getRequestCache(requestId, CURRENT_OPERATE_USER_ID));
            copyRestApi.deleteCopy(cloneCopyId, currentOperateUserId);
        } catch (Throwable e) {
            log.error("delete clone copy failed. copy id: {}", cloneCopyId, ExceptionUtil.getErrorMessage(e));
            throw LegoCheckedException.cast(e);
        }
    }

    /**
     * delete live mount schedule
     *
     * @param liveMountEntity live mount entity
     */
    public void deleteSchedule(LiveMountEntity liveMountEntity) {
        String scheduleId = liveMountEntity.getScheduleId();
        if (!VerifyUtil.isEmpty(scheduleId)) {
            scheduleRestApi.deleteSchedule(scheduleId);
        }
    }

    /**
     * Check whether the resource lock fails to be obtained.
     *
     * @param requestId request id
     * @return check result
     */
    public boolean isLockFail(String requestId) {
        JSONObject lockObject = JSONObject.fromObject(getRequestCache(requestId, LOCK));
        if (lockObject.getString(LOCK_STATUS) != null) {
            return LOCK_STATUS_FAIL.equals(lockObject.get(LOCK_STATUS).toString());
        }
        return false;
    }
}
