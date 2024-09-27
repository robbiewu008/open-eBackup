/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.job.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 功能描述
 *
 * @author w00448845
 * @version [BCManager 8.0.0]
 * @since 2020-04-01
 */
@Data
public class JobLogUpdateBo {
    @JsonProperty("log_timestamp")
    private Long startTime;

    @JsonProperty("log_info")
    private String logInfo;

    @JsonProperty("log_info_param")
    private String logInfoParam;

    @JsonProperty("log_detail")
    private String logDetail;

    @JsonProperty("log_detail_param")
    private String logDetailParam;
}