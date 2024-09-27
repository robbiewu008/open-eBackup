/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.job.model;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.util.EnumUtil;

import com.google.common.collect.Lists;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * 功能描述
 *
 * @author w00448845
 * @version [BCManager 8.0.0]
 * @since 2020-04-15
 */
public enum JobStatusEnum {
    /**
     * be queuing
     */
    PENDING(0),

    /**
     * dispatching
     */
    DISPATCHING(0),

    /**
     * redispatch
     */
    REDISPATCH(0),

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
    ABORT_FAILED(6),

    /**
     * dispatch failed
     */
    DISPATCH_FAILED(6),

    /**
     * unknown
     */
    UNKNOWN(6);

    /**
     * 运行状态列表
     */
    public static final List<JobStatusEnum> LONG_TIME_JOB_STOP_STATUS_LIST =
            Arrays.asList(JobStatusEnum.RUNNING, JobStatusEnum.ABORTING, JobStatusEnum.READY);

    /**
     * 运行状态列表
     */
    public static final List<JobStatusEnum> RUNNING_STATUS_LIST =
            Arrays.asList(JobStatusEnum.RUNNING, JobStatusEnum.ABORTING);

    /**
     * 运行状态+就绪状态列表
     */
    public static final List<JobStatusEnum> READY_AND_RUNNING_STATUS_LIST =
            Collections.unmodifiableList(Arrays.asList(JobStatusEnum.READY,
                    JobStatusEnum.RUNNING, JobStatusEnum.ABORTING));

    /**
     * 完结状态列表
     */
    public static final List<JobStatusEnum> FINISHED_STATUS_LIST =
            Lists.newArrayList(
                    JobStatusEnum.FAIL,
                    JobStatusEnum.SUCCESS,
                    JobStatusEnum.ABORTED,
                    JobStatusEnum.PARTIAL_SUCCESS,
                    JobStatusEnum.CANCELLED,
                    JobStatusEnum.DISPATCH_FAILED,
                    JobStatusEnum.ABORT_FAILED);

    /**
     * 获取所有处于未完结状态的列表
     *
     * @return 处于未完结状态的列表
     */
    public static List<JobStatusEnum> getUnfinishedStatusList() {
        List<JobStatusEnum> unfinishedStatusList = new ArrayList<>();
        for (JobStatusEnum jobStatusEnum : JobStatusEnum.values()) {
            if (!FINISHED_STATUS_LIST.contains(jobStatusEnum)) {
                unfinishedStatusList.add(jobStatusEnum);
            }
        }
        return unfinishedStatusList;
    }

    /**
     * 不带终止的完结状态
     */
    public static final List<JobStatusEnum> FINISHED_STATUS_WITHOUT_ABORT_LIST =
            Lists.newArrayList(
                    JobStatusEnum.FAIL, JobStatusEnum.SUCCESS, JobStatusEnum.PARTIAL_SUCCESS, JobStatusEnum.CANCELLED);

    /**
     * 成功状态列表
     */
    private static final List<JobStatusEnum> SUCCESS_STATUS_LIST =
            Arrays.asList(JobStatusEnum.SUCCESS, JobStatusEnum.PARTIAL_SUCCESS);

    private final int index;

    JobStatusEnum(int index) {
        this.index = index;
    }

    /**
     * get job status enum by str
     *
     * @param str str
     * @return job status enum
     */
    public static JobStatusEnum get(String str) {
        return get(str, false);
    }

    /**
     * get job status enum by str
     *
     * @param str str
     * @param isSilent silent mode
     * @return job status enum
     */
    public static JobStatusEnum get(String str, boolean isSilent) {
        return EnumUtil.get(JobStatusEnum.class, JobStatusEnum::name, str, false, isSilent);
    }

    public int getIndex() {
        return index;
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
    public boolean checkSuccess(LegoCheckedException exception) {
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
