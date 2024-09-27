/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.Data;

/**
 * 查询集群任务请求体
 *
 * @author w00607005
 * @since 2023-08-15
 */
@Data
public class ClusterJobRequest {
    private Long startTime;

    private Long endTime;

    private JobTypeEnum type;

    private String esn;
}
