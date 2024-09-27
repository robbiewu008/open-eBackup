/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.config;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-06-19
 */
@Data
public class EmeiStorExceptionResponse {
    @JsonProperty("http_status")
    private String httpStatus;

    @JsonProperty("error_code")
    private String unifiedErrorCode;

    @JsonProperty("error_msg")
    private String unifiedErrorMsg;
}
