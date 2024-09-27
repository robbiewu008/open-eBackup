/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.model.job.request;

import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.SortingParamRequest;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * The QueryReportJobsRequest
 *
 * @author x30021699
 * @since 2023-01-20
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class QueryReportJobsRequest {
    private QueryJobRequest queryJobRequest;

    private PagingParamRequest pagingParamRequest;

    private SortingParamRequest sortingParamRequest;
}
