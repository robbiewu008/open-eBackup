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

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;

import com.fasterxml.jackson.core.type.TypeReference;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectObjectConsistentService;
import openbackup.access.framework.resource.service.ProtectedResourceRepository;
import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.dao.CopyMapper;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.protection.access.provider.sdk.base.v2.RemotePath;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.system.base.common.enums.ConsistentStatusEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.storage.service.LocalDoradoFileSystemService;
import openbackup.system.base.util.SQLDistributeLock;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

/**
 * 功能描述: ProtectObjectConsistentService
 *
 */
@Slf4j
@Service
public class ProtectObjectConsistentServiceImpl implements ProtectObjectConsistentService {
    private static final List<String> UNSUPPORTED_SUB_TYPE_LIST = Arrays.asList(
            ResourceSubTypeEnum.NAS_FILESYSTEM.getType(),
            ResourceSubTypeEnum.GAUSSDB_DWS.getType(),
            ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType(),
            ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType(),
            ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType(),
            ResourceSubTypeEnum.VCENTER.getType(),
            ResourceSubTypeEnum.ESX.getType(),
            ResourceSubTypeEnum.ESXI.getType(),
            ResourceSubTypeEnum.VMWARE.getType(),
            ResourceSubTypeEnum.ORACLE.getType());

    private static final String ZK_PROTECT_OBJECT_CONSISTENT_REFRESH_LOCK = "/protectObjectConsistentRefreshLock";

    private static final String ZK_PROTECT_OBJECT_CONSISTENT_CHECK_LOCK = "/protectObjectConsistentCheckLock";

    private static final String ZK_CONSISTENT_CHECK_JOB_RUNNING_LOCK = "/protectObjectConsistentCheckJobRunningLock";

    private static final long TEN_MINUTES = 1000 * 60 * 10L;

    private static final String DEVICE_ESN = "device_esn";

    private final ProtectedResourceRepository repository;

    private final LocalDoradoFileSystemService fileSystemService;

    private final CopyRestApi copyRestApi;

    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    private final LockService lockService;

    @Autowired
    private MemberClusterService memberClusterService;

    @Autowired
    private CopyMapper copyMapper;

    /**
     * 构造器注入
     *
     * @param repository repository
     * @param fileSystemService fileSystemService
     * @param copyRestApi copyRestApi
     * @param dmeUnifiedRestApi dmeUnifiedRestApi
     * @param lockService lockService
     */
    public ProtectObjectConsistentServiceImpl(ProtectedResourceRepository repository,
            LocalDoradoFileSystemService fileSystemService, CopyRestApi copyRestApi,
            DmeUnifiedRestApi dmeUnifiedRestApi, LockService lockService) {
        this.repository = repository;
        this.fileSystemService = fileSystemService;
        this.copyRestApi = copyRestApi;
        this.dmeUnifiedRestApi = dmeUnifiedRestApi;
        this.lockService = lockService;
    }

    @Override
    @SQLDistributeLock(lockName = ZK_PROTECT_OBJECT_CONSISTENT_REFRESH_LOCK, tryLockTime = 0)
    public synchronized void refreshProtectObjectConsistentStatus() {
        int undetectedCount = repository.queryProtectObjectCountByConsistentStatus(ConsistentStatusEnum.UNDETECTED);
        log.info("Refresh protect object start, undetected count: {}.", undetectedCount);
        if (undetectedCount <= 0) {
            repository.updateAllProtectObjectConsistentStatus(ConsistentStatusEnum.UNDETECTED);
            log.info("Refresh all protect object to undetected success");
        }
    }

    @Override
    @SQLDistributeLock(lockName = ZK_PROTECT_OBJECT_CONSISTENT_CHECK_LOCK, tryLockTime = 0)
    public synchronized void checkProtectObjectConsistentStatus(boolean isInit) {
        ThreadPoolExecutor threadPool = new ThreadPoolExecutor(1, 1, 1, TimeUnit.SECONDS, new ArrayBlockingQueue<>(1));
        threadPool.execute(() -> lockAndCheck(isInit));
        threadPool.allowCoreThreadTimeOut(true);
    }

    private void lockAndCheck(boolean isInit) {
        if (isInit) {
            waitProcessInitFinish();
        }
        Lock lock = null;
        boolean isLockSuccess = false;
        try {
            lock = lockService.createSQLDistributeLock(ZK_CONSISTENT_CHECK_JOB_RUNNING_LOCK);
            isLockSuccess = lock.tryLock(0, TimeUnit.SECONDS);
            log.info("Check consistent status lock result: {}.", isLockSuccess);
            if (!isLockSuccess) {
                return;
            }
            doCheck();
        } catch (LegoCheckedException e) {
            log.error("Check consistent status failed.", e);
        } finally {
            if (lock != null && isLockSuccess) {
                lock.unlock();
            }
            log.info("Check all undetected object consistent status finish.");
        }
    }

    private void doCheck() {
        List<String> undetectedIdList = repository.queryProtectObjectIdListByConsistentStatus(
                ConsistentStatusEnum.UNDETECTED);
        log.info("Check all undetected object start, all undetected count: {}.", undetectedIdList.size());
        undetectedIdList.forEach(this::checkProtectObjectConsistentStatusById);
    }

    private void checkProtectObjectConsistentStatusById(String uuid) {
        ProtectedObjectPo protectedObjectPo = repository.queryProtectObjectById(uuid);
        if (VerifyUtil.isEmpty(protectedObjectPo)) {
            log.warn("Protect object not exist, uuid: {}.", uuid);
            return;
        }
        if (UNSUPPORTED_SUB_TYPE_LIST.contains(protectedObjectPo.getSubType())) {
            log.warn("Protect object is unsupported, uuid: {}, subType: {}.", uuid, protectedObjectPo.getSubType());
            repository.updateProtectObjectConsistentById(uuid, ConsistentStatusEnum.CONSISTENT, StringUtils.EMPTY);
            return;
        }
        String resourceId = protectedObjectPo.getResourceId();
        log.info("Check protect object start, uuid: {}, resourceId: {}.", uuid, resourceId);
        Copy latestCopy = copyRestApi.queryLatestBackupCopy(resourceId, null, null);
        if (VerifyUtil.isEmpty(latestCopy)) {
            log.info("Resource has no backup copy, resourceId: {}.", resourceId);
            repository.updateProtectObjectConsistentById(uuid, ConsistentStatusEnum.CONSISTENT, StringUtils.EMPTY);
            return;
        }
        if (StringUtils.isBlank(latestCopy.getDeviceEsn())) {
            // 升级场景，device esn为空
            checkProtectConsistentStatusSingle(protectedObjectPo, latestCopy);
        } else {
            // 当前版本场景
            checkProtectConsistentStatusCluster(protectedObjectPo);
        }
        log.info("Check protect object finish, uuid: {}.", uuid);
    }

    private void checkProtectConsistentStatusSingle(ProtectedObjectPo protectedObject, Copy latestCopy) {
        ConsistentStatusEnum status = checkCopyConsistent(latestCopy.getUuid());
        repository.updateProtectObjectConsistentById(protectedObject.getUuid(), status, StringUtils.EMPTY);
        log.info("Check protect object finish, uuid: {}, status: {}.", protectedObject.getUuid(), status);
    }

    private void checkProtectConsistentStatusCluster(ProtectedObjectPo protectedObject) {
        List<String> deviceEsnList = copyMapper.selectCopyDeviceEsnByResourceId(protectedObject.getResourceId());
        if (VerifyUtil.isEmpty(deviceEsnList)) {
            log.info("Resource has no backup copy, resourceId: {}.", protectedObject.getResourceId());
            repository.updateProtectObjectConsistentById(protectedObject.getUuid(), ConsistentStatusEnum.CONSISTENT,
                    StringUtils.EMPTY);
            return;
        }
        deviceEsnList = deviceEsnList.stream().filter(StringUtils::isNotBlank).collect(Collectors.toList());
        // 获取当前集群的device-esn
        String deviceId = memberClusterService.getCurrentClusterEsn();
        Copy latestDeviceCopy = queryLatestBackupCopyByDeviceEsn(protectedObject.getResourceId(), deviceId);
        if (VerifyUtil.isEmpty(latestDeviceCopy)) {
            log.info("Resource has no backup copy, resourceId: {}.", protectedObject.getResourceId());
            return;
        }
        ConsistentStatusEnum status = checkCopyConsistent(latestDeviceCopy.getUuid());
        Map<String, ConsistentStatusEnum> consistentResults = parseConsistentResults(
                protectedObject.getConsistentResults());
        consistentResults.put(deviceId, status);
        log.info("Check protect object finish by device, uuid: {}, status: {}, deviceEsn: {}.",
                protectedObject.getUuid(), status, deviceId);

        if (deviceEsnList.size() == consistentResults.size()) {
            // 所有device检测完成， 更新检测状态
            int inconsistentSize = (int) consistentResults.values()
                    .stream()
                    .filter(v -> v == ConsistentStatusEnum.INCONSISTENT)
                    .count();
            repository.updateProtectObjectConsistentById(protectedObject.getUuid(),
                    inconsistentSize > 0 ? ConsistentStatusEnum.INCONSISTENT : ConsistentStatusEnum.CONSISTENT,
                    JsonUtil.json(consistentResults));
        } else {
            // 只有部分device检测完成，暂时不更新检测状态
            repository.updateProtectObjectConsistentById(protectedObject.getUuid(), ConsistentStatusEnum.UNDETECTED,
                    JsonUtil.json(consistentResults));
        }
    }

    private Copy queryLatestBackupCopyByDeviceEsn(String resourceId, String deviceEsn) {
        Map<String, Object> condition = new HashMap<>();
        condition.put(DEVICE_ESN, deviceEsn);
        return copyRestApi.queryLatestBackupCopy(resourceId, null, condition);
    }

    private ConsistentStatusEnum checkCopyConsistent(String copyId) {
        String fsId = queryFsIdByCopyId(copyId);
        if (VerifyUtil.isEmpty(fsId)) {
            log.warn("Protect object fsId is empty, copyId: {}, fsId: {}.", copyId, fsId);
            return ConsistentStatusEnum.CONSISTENT;
        }
        return fileSystemService.checkFsConsistentStatus(fsId);
    }

    private String queryFsIdByCopyId(String copyId) {
        try {
            DmeCopyInfo copyInfo = dmeUnifiedRestApi.getCopyInfo(copyId);
            return parseFsIdFromDmeCopyInfo(copyInfo);
        } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
            log.error("Query fsId failed, copyId: {}.", copyId, ExceptionUtil.getErrorMessage(e));
            return StringUtils.EMPTY;
        }
    }

    private Map<String, ConsistentStatusEnum> parseConsistentResults(String consistentResults) {
        if (StringUtils.isBlank(consistentResults)) {
            return new HashMap<>();
        }
        return JsonUtil.read(consistentResults, new TypeReference<Map<String, ConsistentStatusEnum>>() {});
    }

    private String parseFsIdFromDmeCopyInfo(DmeCopyInfo copyInfo) {
        List<StorageRepository> repositories = Optional.ofNullable(copyInfo)
                .map(DmeCopyInfo::getRepositories)
                .orElse(Collections.emptyList());
        List<RemotePath> remotePaths = repositories.stream()
                .filter(StorageRepository::getLocal)
                .filter(repository -> repository.getType().equals(RepositoryTypeEnum.DATA.getType()))
                .findFirst()
                .map(StorageRepository::getRemotePath)
                .orElse(Collections.emptyList());
        return remotePaths.stream()
                .filter(remotePath -> remotePath.getType() == RepositoryTypeEnum.DATA.getType())
                .findFirst()
                .map(RemotePath::getId)
                .orElse(StringUtils.EMPTY);
    }

    private void waitProcessInitFinish() {
        try {
            log.info("Wait 10 minutes to ensure process init finish.");
            Thread.sleep(TEN_MINUTES);
        } catch (InterruptedException e) {
            log.error("Fail to wait process init finish.", e);
        }
    }
}