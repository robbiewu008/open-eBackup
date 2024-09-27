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
package openbackup.access.framework.resource.lock;

import openbackup.access.framework.resource.lock.dao.ResourceLockMapper;
import openbackup.access.framework.resource.lock.entity.ResourceLock;
import openbackup.access.framework.resource.service.ResourceRedisLockService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobStatusEnum;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RBucket;
import org.redisson.api.RSet;
import org.redisson.api.RedissonClient;
import org.redisson.client.RedisException;
import org.redisson.client.codec.StringCodec;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 功能描述: LockResourceReleaseScheduler
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-07-07
 */
@Slf4j
@Component
@AllArgsConstructor
public class ResourceLockReleaseScheduler {
    // 初始化完成5分钟后开始定时清理
    private static final long INITIAL_DELAY = 1000 * 60 * 5;

    // 定时清理频率
    private static final long FIXED_RATE = 1000 * 60 * 5;

    private static final int FIVE_MINUTE = 1000 * 60 * 5;

    private static final StringCodec STRING_REDISSON_CODE = new StringCodec();

    // redisson以默认编码格式取值时，存放的value编码格式不一致时会抛异常
    private static final String REDIS_EXCEPTION_MESSAGE = "Unexpected exception while processing command";

    private final ResourceLockMapper resourceLockMapper;

    private final JobService jobService;

    private RedissonClient redissonClient;

    private ResourceRedisLockService resourceRedisLockService;

    /**
     * 定时清理任务已经结束，但是未释放的资源锁
     */
    @Scheduled(initialDelay = INITIAL_DELAY, fixedRate = FIXED_RATE)
    public void releaseOverTimeResourceLock() {
        List<String> lockIdList = queryAllResourceLockId();
        lockIdList.forEach(this::releaseAbnormalResourceLock);
    }

    private List<String> queryAllResourceLockId() {
        // Redis中放了资源冗余锁，清理的时候也要一并清除。防止HA同步后，任务丢失，锁还存在的情况。
        Set<String> members = new HashSet<>();
        // 1.6升级redis开源软件到3.23.5时，由于spring版本和Redis版本不一致，用到spring调用redis能力需要替换为redission
        // 由于spring操作redis的str类型数据时默认使用str的编码格式，而redission使用的sft的编码格式。
        // 用redission获取spring存放的数据会报错，修改方案通过捕获指定异常区分spring保存的数据还是redission保存的数据
        try {
            RSet<String> set = redissonClient.getSet(ResourceLockConstant.LOCK_ID_REDIS_SET_KEY);
            members = set.readAll();
        } catch (RedisException e) {
            log.error("Get redis set:{} error.", ResourceLockConstant.LOCK_ID_REDIS_SET_KEY,
                    ExceptionUtil.getErrorMessage(e));
            if (!VerifyUtil.isEmpty(e.getMessage())
                    && e.getMessage().contains(REDIS_EXCEPTION_MESSAGE)) {
                RSet<String> set = redissonClient.getSet(ResourceLockConstant.LOCK_ID_REDIS_SET_KEY,
                        STRING_REDISSON_CODE);
                members = set.readAll();
            }
        }
        LambdaQueryWrapper<ResourceLock> wrapper = new LambdaQueryWrapper<ResourceLock>().eq(ResourceLock::getLockState,
                "LOCKED");
        List<ResourceLock> resourceLocks = resourceLockMapper.selectList(wrapper);
        Set<String> lockIdSet = Optional.ofNullable(resourceLocks).orElse(Collections.emptyList()).stream()
                .filter(this::shouldReleaseLock).map(ResourceLock::getLockId).collect(Collectors.toSet());

        if (!VerifyUtil.isEmpty(members)) {
            lockIdSet.addAll(members.stream().filter(this::shouldReleaseRedisLock)
                    .map(lockId -> lockId.replaceFirst(ResourceLockConstant.REDIS_LOCK_ID_PREFIX, ""))
                    .collect(Collectors.toList()));
        }
        return new ArrayList<>(lockIdSet);
    }

    private boolean shouldReleaseRedisLock(String lockId) {
        // 复制添加的redis锁不由PM做定时清理。由复制侧做清理动作
        String resources = null;
        try {
            RBucket<String> bucket = redissonClient.getBucket(lockId);
            resources = bucket.get();
        } catch (RedisException e) {
            log.error("Get redis key:{} error.", lockId, ExceptionUtil.getErrorMessage(e));
            if (!VerifyUtil.isEmpty(e.getMessage())
                    && e.getMessage().contains("Unexpected exception while processing" + " command")) {
                RBucket<String> bucket = redissonClient.getBucket(lockId, STRING_REDISSON_CODE);
                resources = bucket.get();
            }
        }
        if (VerifyUtil.isEmpty(resources)) {
            return true;
        }
        return !resources.contains(ResourceLockConstant.LOCK_REPLICATION_TASK_FLAG);
    }

    private boolean shouldReleaseLock(ResourceLock resourceLock) {
        // 复制添加的锁不由P做定时清理。由复制侧做清理动作
        return !resourceLock.getResourceId().contains(ResourceLockConstant.LOCK_REPLICATION_TASK_FLAG);
    }

    private void releaseAbnormalResourceLock(String lockId) {
        String tmpLockId = lockId;
        if (lockId.endsWith("@log")) {
            tmpLockId = lockId.substring(0, lockId.length() - 4);
        }

        if (!isJobFinished(tmpLockId)) {
            return;
        }

        LambdaQueryWrapper<ResourceLock> wrapper =
            new LambdaQueryWrapper<ResourceLock>().eq(ResourceLock::getLockId, lockId);
        resourceLockMapper.delete(wrapper);
        resourceRedisLockService.unlock(ResourceLockConstant.REDIS_LOCK_ID_PREFIX + lockId);
        log.warn("Release abnormal lock when Job has finished, lockId and jobId: {}", lockId);
    }

    private boolean isJobFinished(String jobId) {
        JobBo jobBo;
        try {
            jobBo = jobService.queryJob(jobId);
        } catch (LegoCheckedException e) {
            log.error("Query job failed, jobId: {}.", jobId, ExceptionUtil.getErrorMessage(e));
            if (e.getErrorCode() == CommonErrorCode.OBJ_NOT_EXIST) {
                return true;
            } else {
                return false;
            }
        }

        JobStatusEnum statusEnum = JobStatusEnum.get(jobBo.getStatus());
        if (!JobStatusEnum.FINISHED_STATUS_LIST.contains(statusEnum)) {
            return false;
        }
        if (VerifyUtil.isEmpty(jobBo.getEndTime())) {
            return true;
        }
        return System.currentTimeMillis() - jobBo.getEndTime() > FIVE_MINUTE;
    }
}