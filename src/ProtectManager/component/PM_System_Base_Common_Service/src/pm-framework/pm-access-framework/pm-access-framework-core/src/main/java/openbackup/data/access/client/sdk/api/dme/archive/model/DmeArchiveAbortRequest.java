/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 终止归档任务请求体
 *
 * @author y00490893
 * @since 2021-1-15
 */
@Data
public class DmeArchiveAbortRequest {
    @JsonProperty("RequestId")
    private String requestId;
}
