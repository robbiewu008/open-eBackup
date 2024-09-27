/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.model.job;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Task summary response
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-01-15
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class JobSummary {
    private long total;

    private long success;

    private long fail;

    private long pending;

    private long dispatching;

    private long redispatch;

    private long running;

    private long aborted;

    private long ready;

    private long aborting;

    @JsonProperty("partial_success")
    private long partialSuccess;

    private long abnormal;

    @JsonProperty("abort_failed")
    private long abortFailed;

    @JsonProperty("dispatch_failed")
    private long dispatchFailed;

    private long cancelled;
}
