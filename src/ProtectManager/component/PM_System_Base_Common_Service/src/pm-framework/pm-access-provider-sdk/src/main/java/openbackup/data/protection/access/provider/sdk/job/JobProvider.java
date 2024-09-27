/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.job;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.exception.NotImplementedException;
import openbackup.system.base.common.model.job.Job;

/**
 * Job provider base class
 *
 * @author l00272247
 * @since 2020-09-01
 */
public interface JobProvider extends DataProtectionProvider<String> {
    /**
     * Querying the Status of a Specified Task by task id, for example backup task
     * or restore task
     *
     * @param associativeId associativeId
     * @param startTime start time
     * @return The JobUpdateBo
     */
    default ProviderJobMessage queryJobMessage(String associativeId, long startTime) {
        throw new NotImplementedException("");
    }

    /**
     * Stop job by ID, for example backup tasks or restore tasks
     *
     * @param associativeId associativeId
     */
    void stopJob(String associativeId);


    /**
     * 检测任务是否能被创建
     *
     * @param userId 用户ID
     */
    default void checkJobCanBeCreate(String userId) {
    }

    /**
     * 对任务参数进行填充
     *
     * @param insertJob 准备入库的任务模型
     */
    default void fillJobInfo(Job insertJob) {
    }
}
