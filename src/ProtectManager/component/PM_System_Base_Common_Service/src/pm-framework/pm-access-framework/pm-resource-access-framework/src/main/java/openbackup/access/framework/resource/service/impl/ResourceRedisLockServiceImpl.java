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
package openbackup.access.framework.resource.service.impl;

import com.huawei.oceanprotect.base.cluster.sdk.dto.MemberClusterBo;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.job.sdk.JobService;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.lock.ResourceLockConstant;
import openbackup.access.framework.resource.service.ResourceRedisLockService;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.sdk.cluster.BackupClusterJobClient;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceLockEntity;
import openbackup.system.base.sdk.resource.model.ResourceLockRequest;

import org.redisson.api.RBucket;
import org.redisson.api.RSet;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.util.UriComponentsBuilder;

import java.net.URI;
import java.time.Duration;
import java.time.temporal.ChronoUnit;
import java.util.List;
import java.util.stream.Collectors;

/**
 * 资源Redis冗余锁
 *
 */
@Service
@Slf4j
public class ResourceRedisLockServiceImpl implements ResourceRedisLockService {
    private static final String REQUEST_SCHEME = "https";

    // 默认锁过期时间一天
    private static final long DEFAULT_EXPIRE_TIME = 24 * 60 * 60 * 1000L;

    // 3秒超时时间
    private static final long TIMEOUT = 3000L;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private MemberClusterService memberClusterService;

    @Autowired
    private BackupClusterJobClient backupClusterJobClient;

    @Autowired
    private AuthNativeApi authNativeApi;

    @Autowired
    private JobService jobService;

    @Autowired
    private CopyRestApi copyRestApi;

    @Override
    public boolean acquireLock(String lockId, List<ResourceLockEntity> resources, long expireTime) {
        if (!lockId.startsWith(ResourceLockConstant.REDIS_LOCK_ID_PREFIX)) {
            log.info("Lock id: {} format is abnormal", lockId);
            return false;
        }

        String jobId = lockId.replaceFirst(ResourceLockConstant.REDIS_LOCK_ID_PREFIX, "")
                .replaceFirst(ResourceLockConstant.VMWARE_RESTORE_LOCK_SUFFIX, "");
        if (!jobService.isJobPresent(jobId)) {
            log.info("Lock id: {} related job is not exist", lockId);
            // 如果查询不到任务ID,则校验复制场景。如果无法根据资源id查询到副本，则不加锁
            filterAbnormalResourcesInRepCase(resources, lockId);
            if (resources.isEmpty()) {
                log.info("No resource need to lock");
                return true;
            }
        }

        // 任务ID不存在时可能是复制场景,需要检验资源ID是否存在
        RBucket<String> bucket = redissonClient.getBucket(lockId);
        RSet<String> set = redissonClient.getSet(ResourceLockConstant.LOCK_ID_REDIS_SET_KEY);
        long startTime = System.currentTimeMillis();

        // 自旋尝试获取锁
        while (true) {
            boolean isSuccess = Boolean.TRUE.equals(bucket.setIfAbsent(JsonUtil.json(resources),
                Duration.of(expireTime, ChronoUnit.MILLIS)));
            if (isSuccess) {
                // redis冗余锁加锁成功后，将lock_id保存在redis中。如果任务丢失，可以根据redis中lock_id解锁对应资源
                set.add(lockId);
                log.info("Redis lock isSuccess,lock_id:{}", lockId);
                return true;
            }

            // 检查锁是否超时
            if (System.currentTimeMillis() - startTime > TIMEOUT) {
                log.info("Redis lock fail,lock_id:{}", lockId);
                return false;
            }

            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                log.warn("Redis resource lock sleep interrupt.lock_id:{}", lockId);
            }
        }
    }

    @Override
    public boolean acquireLock(String lockId, List<ResourceLockEntity> resources) {
        return acquireLock(lockId, resources, DEFAULT_EXPIRE_TIME);
    }

    @Override
    public boolean unlock(String lockId) {
        RSet<String> fstCodeSet = redissonClient.getSet(ResourceLockConstant.LOCK_ID_REDIS_SET_KEY);
        fstCodeSet.remove(lockId);
        // redis升级到3.23.5后，使用redisson替换redisTemplate后，不指定编码格式取值会报错，使用str的编码格式来取值
        RSet<String> stringCodeSet = redissonClient.getSet(ResourceLockConstant.LOCK_ID_REDIS_SET_KEY,
                new StringCodec());
        stringCodeSet.remove(lockId);
        redissonClient.getBucket(lockId).delete();
        log.info("Redis resource unlock success.lock id:{}", lockId);
        return true;
    }

    @Override
    public boolean lockAndMultiClusterSync(String lockId, List<ResourceLockEntity> resources) {
        // 本地redis入库
        boolean isLocked = acquireLock(lockId, resources);
        log.info("Current node redis resource lock {},lock_id :{}", isLocked ? "success" : "fail", lockId);
        // 未组建集群，不同步
        if (!memberClusterService.clusterEstablished()) {
            return isLocked;
        }
        // 查询所有的节点，开始同步资源锁
        log.info("Start sync redis resource lock while multi cluster");
        List<MemberClusterBo> allMemberClusters = memberClusterService.getAllMemberClusters();
        String currentClusterEsn = memberClusterService.getCurrentClusterEsn();
        String token = authNativeApi.generateClusterAdminToken();
        List<Boolean> lockResult = allMemberClusters.stream()
            .filter(memberClusterBo -> !currentClusterEsn.equals(memberClusterBo.getRemoteEsn()))
            .map(memberClusterBo -> {
                URI uri = buildUri(memberClusterBo);
                try {
                    boolean isClusterLocked = backupClusterJobClient.acquireLock(uri, token,
                        ResourceLockRequest.builder().lockId(lockId).resources(resources).build());
                    log.info("Redis lock {},cluster ip:{},lock id:{}", isClusterLocked ? "success" : "fail",
                        memberClusterBo.getClusterIp(), lockId);
                    return isClusterLocked;
                } catch (FeignException e) {
                    log.error("Invoke api fail redis lock,lockId={},msg:{}", lockId, ExceptionUtil.getErrorMessage(e));
                    return false;
                } catch (Exception e) {
                    log.error("Unexpected error when redis lock ,lockId={},msg:{}", lockId,
                            ExceptionUtil.getErrorMessage(e));
                    return false;
                }
            })
            .collect(Collectors.toList());
        StringUtil.clean(token);
        return lockResult.stream().anyMatch(Boolean.TRUE::equals);
    }

    @Override
    public boolean unlockAndMultiClusterSync(String lockId) {
        // 本地redis删锁
        boolean isUnLocked = unlock(lockId);
        log.info("Current node redis resource unlock {},lockId :{}", isUnLocked ? "success" : "fail", lockId);
        // 未组建集群，不同步
        if (!memberClusterService.clusterEstablished()) {
            return isUnLocked;
        }
        // 同步所有节点，删锁
        log.info("Start sync redis resource unlock while multi cluster");
        List<MemberClusterBo> allMemberClusters = memberClusterService.getAllMemberClusters();
        String currentClusterEsn = memberClusterService.getCurrentClusterEsn();
        String token = authNativeApi.generateClusterAdminToken();
        List<Boolean> lockResult = allMemberClusters.stream()
            .filter(memberClusterBo -> !currentClusterEsn.equals(memberClusterBo.getRemoteEsn()))
            .map(memberClusterBo -> {
                URI uri = buildUri(memberClusterBo);
                try {
                    boolean isClusterUnlock =
                        backupClusterJobClient.unlock(uri, token, ResourceLockRequest.builder().lockId(lockId).build());
                    log.info("Redis unlock {},cluster ip:{},lock id:{}", isClusterUnlock ? "success" : "fail",
                        memberClusterBo.getClusterIp(), lockId);
                    return isClusterUnlock;
                } catch (FeignException e) {
                    log.error("Invoke api fail redis unlock,lockId={},msg:{}", lockId,
                        ExceptionUtil.getErrorMessage(e));
                    return false;
                } catch (Exception e) {
                    log.error("Unexpected error when redis unlock ,lockId={},msg:{}", lockId,
                        ExceptionUtil.getErrorMessage(e));
                    return false;
                }
            })
            .collect(Collectors.toList());
        StringUtil.clean(token);
        return lockResult.stream().anyMatch(Boolean.TRUE::equals);
    }

    private void filterAbnormalResourcesInRepCase(List<ResourceLockEntity> resources, String lockId) {
        resources.removeIf(resourceLockEntity -> {
            boolean shouldDelete = isResIdAbnormalWhenJobNotExist(resourceLockEntity);
            if (shouldDelete) {
                log.info("Resource(id: {}) in lock request(lock id: {}) is not related any copy,will not be lock.",
                    resourceLockEntity.getResourceId(), lockId);
            }
            return shouldDelete;
        });
    }

    private boolean isResIdAbnormalWhenJobNotExist(ResourceLockEntity resourceLockEntity) {
        return !resourceLockEntity.getResourceId().startsWith(ResourceLockConstant.LOCK_REPLICATION_TASK_FLAG)
            || copyRestApi
                .queryCopiesByResourceId(resourceLockEntity.getResourceId()
                    .replaceFirst(ResourceLockConstant.LOCK_REPLICATION_TASK_FLAG, ""))
                .isEmpty();
    }

    private URI buildUri(MemberClusterBo targetCluster) {
        String targetIp = targetCluster.getClusterIp().split(",")[0];
        return UriComponentsBuilder.newInstance()
            .scheme(REQUEST_SCHEME)
            .host(targetIp)
            .port(targetCluster.getClusterPort())
            .build()
            .toUri();
    }
}
