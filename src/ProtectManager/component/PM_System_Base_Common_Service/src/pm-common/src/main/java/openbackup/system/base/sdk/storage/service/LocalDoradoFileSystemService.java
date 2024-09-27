/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.system.base.sdk.storage.service;

import openbackup.system.base.common.enums.ConsistentStatusEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.storage.StorageService;
import openbackup.system.base.sdk.storage.api.LocalDoradoRestApi;
import openbackup.system.base.sdk.storage.model.DoradoResponse;
import openbackup.system.base.sdk.storage.model.FileSystemScrubRequest;
import openbackup.system.base.sdk.storage.model.FileSystemScrubResponse;
import openbackup.system.base.sdk.storage.model.Result;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Objects;
import java.util.function.Supplier;

/**
 * 功能描述: 访问本地 dorado 存储文件系统相关接口统一 Service
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-26
 */
@Slf4j
@Component
public class LocalDoradoFileSystemService {
    /**
     * 指定的文件系统中存在扫描任务。
     */
    private static final long SCRUB_TASK_ALREADY_EXIST = 1073844240L;

    /**
     * 指定文件系统扫描任务不存在。
     */
    private static final long SCRUB_TASK_NOT_EXIST = 1073844234L;

    private static final long ONE_MINUTE = 1000 * 60L;

    private static final String START = "1";

    private static final String STOP = "0";

    private static final String JOB_FINISHED = "1";

    private static final String JOB_FAILED = "3";

    private final LocalDoradoRestApi localDoradoRestApi;

    private final StorageService storageService;

    /**
     * 构造器注入
     *
     * @param localDoradoRestApi localDoradoRestApi
     * @param storageService storageService
     */
    public LocalDoradoFileSystemService(LocalDoradoRestApi localDoradoRestApi, StorageService storageService) {
        this.localDoradoRestApi = localDoradoRestApi;
        this.storageService = storageService;
    }

    /**
     * 校验文件系统的数据一致性状态
     *
     * @param fsId 文件系统 ID
     * @return 文件系统的数据一致性状态
     */
    public ConsistentStatusEnum checkFsConsistentStatus(String fsId) {
        ConsistentStatusEnum consistentStatus = null;
        try {
            changeFileSystemScrub(fsId, START);
            consistentStatus = this.getFsConsistentCheckResult(fsId);
            changeFileSystemScrub(fsId, STOP);
            log.info("Check file system consistent status success, fsId: {}, status: {}.", fsId, consistentStatus);
        } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
            if (consistentStatus == null) {
                consistentStatus = ConsistentStatusEnum.INCONSISTENT;
            }
            log.error("Check file system consistent status failed, fsId: {}, status: {}.", fsId, consistentStatus, e);
        }
        return consistentStatus;
    }

    private void changeFileSystemScrub(String fsId, String action) {
        FileSystemScrubRequest request = new FileSystemScrubRequest(fsId, action);
        try {
            String deviceId = storageService.getStorageSession().getData().getDeviceid();
            invoke("changeFileSystemScrub", () -> localDoradoRestApi.changeFileSystemScrub(deviceId, request));
        } catch (LegoCheckedException e) {
            long code = e.getErrorCode();
            log.error("Invoke storage api failed, fsId: {}, action: {}, code: {}.", fsId, action, code);
            if (code != SCRUB_TASK_ALREADY_EXIST && code != SCRUB_TASK_NOT_EXIST) {
                throw LegoCheckedException.cast(e);
            }
        }
        log.info("Change file system scrub success, fsId: {}, action: {}.", fsId, action);
    }

    private ConsistentStatusEnum getFsConsistentCheckResult(String fsId) {
        DoradoResponse<FileSystemScrubResponse> doradoResponse;
        while (true) {
            String deviceId = storageService.getStorageSession().getData().getDeviceid();
            doradoResponse = invoke("queryFileSystemScrub", () ->
                localDoradoRestApi.queryFileSystemScrub(deviceId, fsId));
            FileSystemScrubResponse response = doradoResponse.getData();
            String runningStatus = response.getRunningStatus();
            log.info("Query file system scrub, status: {}, fsId: {}, mediumErrors: {}, otherErrors: {}.",
                    runningStatus, fsId, response.getMediumErrors(), response.getOtherErrors());
            if (JOB_FAILED.equals(runningStatus)) {
                return ConsistentStatusEnum.INCONSISTENT;
            }
            if (JOB_FINISHED.equals(runningStatus)) {
                return (response.getMediumErrors() == 0 && response.getOtherErrors() == 0)
                        ? ConsistentStatusEnum.CONSISTENT
                        : ConsistentStatusEnum.INCONSISTENT;
            }
            try {
                Thread.sleep(ONE_MINUTE);
            } catch (InterruptedException e) {
                log.error("Sleep failed.", ExceptionUtil.getErrorMessage(e));
            }
        }
    }

    private DoradoResponse invoke(String operator, Supplier<DoradoResponse> supplier) {
        DoradoResponse data = supplier.get();
        if (data == null) {
            log.error("{} fail: data is null", operator);
            return data;
        }
        if (Objects.nonNull(data.getResult()) && Objects.nonNull(data.getResult().getCode())
            && !data.getResult().getCode().equals("0")) {
            Result error = data.getResult();
            log.error("{} fail: {}, {}", operator, error.getCode(), error.getDescription());
        }
        return data;
    }
}