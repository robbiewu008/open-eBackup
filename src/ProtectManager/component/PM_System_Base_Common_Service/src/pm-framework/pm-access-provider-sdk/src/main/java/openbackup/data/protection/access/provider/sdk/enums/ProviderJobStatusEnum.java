/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.enums;

import java.util.Arrays;
import java.util.List;

/**
 * SDK中使用的JobType枚举类
 *
 * @author y00559272
 * @since 2021-10-11
 */
public enum ProviderJobStatusEnum {
    /**
     * be queuing
     */
    PENDING(0),

    /**
     * ready
     */
    READY(1),

    /**
     * running
     */
    RUNNING(2),

    /**
     * abnormal
     */
    ABNORMAL(3),

    /**
     * failed
     */
    FAIL(4),

    /**
     * cancelled
     */
    CANCELLED(5),

    /**
     * success
     */
    SUCCESS(5),

    /**
     * partial success
     */
    PARTIAL_SUCCESS(5),

    /**
     * aborting
     */
    ABORTING(5),

    /**
     * aborted
     */
    ABORTED(6),

    /**
     * abort failed
     */
    ABORT_FAILED(6);

    /**
     * 运行状态列表
     */
    public static final List<ProviderJobStatusEnum> RUNNING_STATUS_LIST =
            Arrays.asList(ProviderJobStatusEnum.RUNNING, ProviderJobStatusEnum.ABORTING);

    /**
     * 完结状态列表
     */
    public static final List<ProviderJobStatusEnum> FINISHED_STATUS_LIST =
            Arrays.asList(
                    ProviderJobStatusEnum.FAIL,
                    ProviderJobStatusEnum.SUCCESS,
                    ProviderJobStatusEnum.ABORTED,
                    ProviderJobStatusEnum.PARTIAL_SUCCESS,
                    ProviderJobStatusEnum.CANCELLED,
                    ProviderJobStatusEnum.ABORT_FAILED);

    /**
     * 不带终止的完结状态
     */
    public static final List<ProviderJobStatusEnum> FINISHED_STATUS_WITHOUT_ABORT_LIST =
            Arrays.asList(
                    ProviderJobStatusEnum.FAIL,
                    ProviderJobStatusEnum.SUCCESS,
                    ProviderJobStatusEnum.PARTIAL_SUCCESS,
                    ProviderJobStatusEnum.CANCELLED);

    /**
     * 成功状态列表
     */
    private static final List<ProviderJobStatusEnum> SUCCESS_STATUS_LIST =
            Arrays.asList(ProviderJobStatusEnum.SUCCESS, ProviderJobStatusEnum.PARTIAL_SUCCESS);

    private final int status;

    ProviderJobStatusEnum(int status) {
        this.status = status;
    }

    /**
     * get job status enum by str
     *
     * @param jobStatus status
     * @return job status enum
     */
    public static ProviderJobStatusEnum getByStatus(int jobStatus) {
        return Arrays.stream(ProviderJobStatusEnum.values())
                .filter(status -> status.status == jobStatus)
                .findFirst()
                .orElseThrow(IllegalArgumentException::new);
    }

    public int getStatus() {
        return status;
    }

    /**
     * check job status
     *
     * @return check result
     */
    public boolean checkSuccess() {
        return checkSuccess(null);
    }

    /**
     * check job status
     *
     * @param exception exception
     * @return check result
     */
    public boolean checkSuccess(RuntimeException exception) {
        boolean isSuccess = SUCCESS_STATUS_LIST.contains(this);
        if (exception == null) {
            return isSuccess;
        }
        if (!isSuccess) {
            throw exception;
        }
        return true;
    }

    /**
     * 判断当前状态是否为完结状态
     *
     * @return 是否完结状态
     */
    public Boolean finishedStatus() {
        return FINISHED_STATUS_LIST.contains(this);
    }

    /**
     * 判断当前状态是否为不带终止的完结状态
     *
     * @return 是否完结
     */
    public Boolean finishedStatusWithoutAbort() {
        return FINISHED_STATUS_WITHOUT_ABORT_LIST.contains(this);
    }

    /**
     * 判断当前状态是否为运行中状态
     *
     * @return 是否运行状态
     */
    public Boolean runningStatus() {
        return RUNNING_STATUS_LIST.contains(this);
    }
}
