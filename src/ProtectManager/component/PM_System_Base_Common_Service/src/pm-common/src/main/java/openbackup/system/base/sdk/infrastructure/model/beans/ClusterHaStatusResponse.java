/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.infrastructure.model.beans;

import openbackup.system.base.sdk.cluster.model.JobLog;

import lombok.Data;

import java.util.List;

/**
 * HA操作结果
 *
 * @author w00607005
 * @since 2023-05-22
 */
@Data
public class ClusterHaStatusResponse {
    /**
     * 任务状态
     */
    private Integer status;

    /**
     * 任务日志
     */
    private List<JobLog> jobLogs;
}
