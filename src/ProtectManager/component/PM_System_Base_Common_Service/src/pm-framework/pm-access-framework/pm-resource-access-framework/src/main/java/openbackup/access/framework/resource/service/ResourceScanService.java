/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

package openbackup.access.framework.resource.service;

import openbackup.system.base.common.model.job.JobBo;

import java.util.List;

/**
 * 资源扫描类
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-23
 */
public interface ResourceScanService {
    /**
     * 根据资源ID查询手动扫描的进行状态的任务
     *
     * @param resId 资源ID
     * @return 任务
     */
    List<JobBo> queryManualScanRunningJobByResId(String resId);

    /**
     * 分页查询手动扫描的进行状态的任务
     *
     * @param page page
     * @param size size
     * @return 任务
     */
    List<JobBo> queryManualScanRunningPage(int page, int size);

    /**
     * 任务是否完结
     *
     * @param jobId jobId
     * @return boolean
     */
    boolean jobIsFinished(String jobId);
}
