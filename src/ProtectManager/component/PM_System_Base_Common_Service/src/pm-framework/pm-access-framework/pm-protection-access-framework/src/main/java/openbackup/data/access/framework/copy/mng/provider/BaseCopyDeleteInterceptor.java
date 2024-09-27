/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.copy.mng.provider;

import static openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants.DIFFERENCE_INCREMENT;
import static openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants.LOG;

import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.copy.CopyDeleteInterceptor;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 副本删除抽象类
 *
 * @author h30027154
 * @since 2022-06-15
 * @version OceanProtect X8000 1.2.1
 */
@Slf4j
public abstract class BaseCopyDeleteInterceptor implements CopyDeleteInterceptor {
    private static final String IS_ASSOCIATED = "is_associated";

    private static final String USER_ID = "user_id";

    /**
     * 可以级联删除的类型
     */
    private static final List<CopyGeneratedByEnum> ASSOCIATED_COPY_GENERATED_BY_LIST = new ArrayList<>(
        Arrays.asList(CopyGeneratedByEnum.BY_BACKUP, CopyGeneratedByEnum.BY_REPLICATED,
            CopyGeneratedByEnum.BY_CASCADED_REPLICATION, CopyGeneratedByEnum.BY_REVERSE_REPLICATION));

    /**
     * CopyRestApi copyRestApi
     */
    @Autowired
    protected CopyRestApi copyRestApi;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    @Qualifier("unifiedResourceConnectionChecker")
    private ProtectedResourceChecker protectedResourceChecker;

    @Autowired
    private JobService jobService;

    @ExterAttack
    @Override
    public void initialize(DeleteCopyTask task, CopyInfoBo copy) {
        RMap<String, String> redis = redissonClient.getMap(task.getRequestId(), StringCodec.INSTANCE);
        if (redis == null) {
            log.warn("redis do not find. requestId: {}", task.getRequestId());
            return;
        }
        Boolean isAssociated = Boolean.valueOf(redis.get(IS_ASSOCIATED));
        log.debug("isAssociated is {},requestId is {}", isAssociated, task.getRequestId());
        if (Objects.equals(isAssociated, true)) {
            log.debug("copy({}) will delete associate copies,requestId: {}", copy.getUuid(), task.getRequestId());
            String userId = redis.get(USER_ID);
            log.debug("delete associated copy. userId is {}, requestId is {}", userId, task.getRequestId());
            JobBo jobBo = jobService.queryJob(task.getTaskId());
            if (jobBo == null) {
                log.error("do not find job. job id is {}", task.getTaskId());
                return;
            }
            handleAssociatedCopy(task, copy, task.getIsForceDeleted(), userId, jobBo.getType());
        }
        // agent
        if (shouldSupplyAgent(task, copy)) {
            supplyAgent(task, copy);
        }

        handleTask(task, copy);
    }

    /**
     * 收集与该副本相关联的需要删除的副本（不返回此副本本身），按删除顺序返回
     *
     * @param copyId 副本ID
     * @return 需要删除的关联的副本
     */
    @Override
    public List<String> getAssociatedCopy(String copyId) {
        Copy thisCopy = copyRestApi.queryCopyByID(copyId, false);
        if (thisCopy == null) {
            return Collections.emptyList();
        }
        if (!shouldDeleteAssociatedCopy(thisCopy)) {
            return Collections.emptyList();
        }
        List<String> associatedCopies = Optional.ofNullable(collectDeleteAssociatedCopy(thisCopy))
            .orElse(Collections.emptyList());

        // 防呆，删除此副本本身
        associatedCopies.removeIf(e -> Objects.equals(e, copyId));
        return associatedCopies;
    }

    /**
     * 收集与该副本相关联的需要删除的副本（不返回此副本本身），按删除顺序返回
     *
     * @param thisCopy 本副本对象
     * @return 需要删除的关联的副本
     */
    protected List<String> collectDeleteAssociatedCopy(Copy thisCopy) {
        int nextGn = thisCopy.getNextCopyGn() == 0 ? thisCopy.getGn() + 1 : thisCopy.getNextCopyGn();
        List<Copy> copies = copyRestApi.queryLaterCopiesByGeneratedBy(thisCopy.getResourceId(), nextGn,
            thisCopy.getGeneratedBy())
            .stream()
            .sorted(Comparator.comparing(Copy::getGn))
            .collect(Collectors.toList());
        return getShouldDeleteCopies(copies, thisCopy);
    }

    /**
     * 判断该副本是否要进行关联删除
     *
     * @param thisCopy 副本
     * @return 是否关联删除
     */
    protected boolean shouldDeleteAssociatedCopy(Copy thisCopy) {
        return CopyUtil.match(ASSOCIATED_COPY_GENERATED_BY_LIST, thisCopy.getGeneratedBy());
    }

    /**
     * 各应用处理任务
     *
     * @param task DeleteCopyTask
     * @param copy CopyInfoBo
     */
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        log.info("handle delete task. requestId: {}", task.getRequestId());
    }

    private void handleAssociatedCopy(DeleteCopyTask task, CopyInfoBo copy, Boolean isForced, String userId,
        String jobType) {
        List<String> needDeleteCopies = getAssociatedCopy(copy.getUuid());
        if (needDeleteCopies != null) {
            // 剔除该副本本身，防止重复删除逻辑
            needDeleteCopies.removeAll(Collections.singletonList(copy.getUuid()));
            for (String needDeleteCopy : needDeleteCopies) {
                try {
                    log.debug("need delete copies is {}, requestId is {}", needDeleteCopy,
                            task.getRequestId());
                    copyRestApi.deleteCopy(needDeleteCopy, userId, isForced, false, jobType);
                } catch (LegoUncheckedException | LegoCheckedException e) {
                    log.error(
                        "associated copy call copy delete api occurs error. request id: {}, "
                            + "associated copy id: {}, error message is: {}",
                        task.getRequestId(), needDeleteCopy, e.getMessage());
                }
            }
        }
    }


    /**
     * 是否填充agent
     *
     * @param copy 副本信息
     * @param task 删除任务
     * @return boolean
     */
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        return false;
    }

    /**
     * 填充agent信息
     *
     * @param task RestoreTask
     * @param copy CopyInfoBo
     */
    protected void supplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        if (copy.getResourceId() == null) {
            return;
        }
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(copy.getResourceId());
        if (!resOptional.isPresent()) {
            return;
        }
        ProtectedResourceChecker checker = providerManager.findProviderOrDefault(ProtectedResourceChecker.class,
            resOptional.get(), this.protectedResourceChecker);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = checker.collectConnectableResources(
            resOptional.get());
        List<Endpoint> endpointList = new ArrayList<>();
        protectedResourceMap.forEach(((protectedResource, protectedEnvironments) -> {
            for (ProtectedEnvironment protectedEnvironment : protectedEnvironments) {
                Endpoint endpoint = new Endpoint();
                endpoint.setId(protectedEnvironment.getUuid());
                endpoint.setIp(protectedEnvironment.getEndpoint());
                endpoint.setPort(protectedEnvironment.getPort());
                endpointList.add(endpoint);
            }
        }));
        task.setAgents(endpointList);
    }

    /**
     * 得到应该删除的关联副本
     *
     * @param copies copies
     * @param thisCopy thisCopy
     * @return List<String>
     */
    protected List<String> getShouldDeleteCopies(List<Copy> copies, Copy thisCopy) {
        List<String> copyUuids = new ArrayList<>();
        int backupType = thisCopy.getBackupType();
        Copy nextFullCopy = CopyUtil.getNextFullCopy(copies, thisCopy.getGn());
        switch (BackupTypeConstants.getBackupTypeByAbBackupType(backupType)) {
            case FULL:
                copyUuids = getCopiesCopyTypeIsFull(copies, thisCopy, nextFullCopy);
                break;
            case DIFFERENCE_INCREMENT:
                copyUuids = getCopiesCopyTypeIsDifferenceIncrement(copies, thisCopy, nextFullCopy);
                break;
            case CUMULATIVE_INCREMENT:
                copyUuids = getCopiesCopyTypeIsCumulativeIncrement(copies, thisCopy, nextFullCopy);
                break;
            case LOG:
                copyUuids = getCopiesCopyTypeIsLog(copies, thisCopy, nextFullCopy);
                break;
            case PERMANENT_INCREMENT:
                copyUuids = getCopiesCopyTypeIsPermanentIncrement(copies, thisCopy, nextFullCopy);
                break;
            default:
                break;
        }
        log.info("Should associate delete copies:[{}], this copy id:{}", JSONObject.writeValueAsString(copyUuids),
            thisCopy);
        return copyUuids;
    }

    /**
     * 全量副本时逻辑处理
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        int format = CopyUtil.getFormat(thisCopy).orElse(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        if (Objects.equals(format, CopyFormatEnum.INNER_DIRECTORY.getCopyFormat())) {
            return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        }
        if (Objects.equals(format, CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat())) {
            return getAssociatedTypeCopiesByBackup(copies, thisCopy, nextFullCopy, Collections.singletonList(LOG));
        }
        return Collections.emptyList();
    }

    /**
     * 增量副本时逻辑处理
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    protected List<String> getCopiesCopyTypeIsDifferenceIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        int format = CopyUtil.getFormat(thisCopy).orElse(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        if (!Objects.equals(format, CopyFormatEnum.INNER_DIRECTORY.getCopyFormat())) {
            return Collections.emptyList();
        }
        return getAssociatedTypeCopiesByBackup(copies, thisCopy, nextFullCopy,
            Collections.singletonList(DIFFERENCE_INCREMENT));
    }

    /**
     * 差异副本时逻辑处理
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    protected List<String> getCopiesCopyTypeIsCumulativeIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        return Collections.emptyList();
    }

    /**
     * 日志副本时逻辑处理
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    protected List<String> getCopiesCopyTypeIsLog(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        return Collections.emptyList();
    }

    /**
     * 永久增量时逻辑处理
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    protected List<String> getCopiesCopyTypeIsPermanentIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        return Collections.emptyList();
    }

    /**
     * 按照给定类型，找到下次全量之间的副本
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @param associatedTypes 给定副本类型
     * @return 需要删除的集合
     */
    protected List<String> getAssociatedTypeCopiesByBackup(List<Copy> copies, Copy thisCopy, Copy nextFullCopy,
        List<BackupTypeConstants> associatedTypes) {
        List<Copy> confContainCopies = copies.stream().filter(copy -> {
            BackupTypeConstants copyBackupType = BackupTypeConstants.getBackupTypeByAbBackupType(
                copy.getBackupType());
            return associatedTypes.contains(copyBackupType);
        }).collect(Collectors.toList());

        return CopyUtil.getCopyUuidsBetweenTwoCopy(confContainCopies, thisCopy, nextFullCopy);
    }
}
