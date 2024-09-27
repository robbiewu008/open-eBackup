/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 目标集群任务信息查询数据返回模型
 *
 * @author dWX1009286
 * @since 2021-07-20
 */
@Data
public class ClusterJobsInfo {
    private long total;

    private long success;

    private long fail;

    private long running;

    private long aborted;

    private long pending;

    private long ready;

    private long aborting;

    @JsonProperty("partial_success")
    private long partialSuccess;

    private long abnormal;

    @JsonProperty("abort_failed")
    private long abortFailed;

    private long cancelled;

    @JsonProperty("dispatch_failed")
    private long dispatchFailed;

    private long dispatching;

    private long redispatch;
}
