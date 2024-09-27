/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.dto;

import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * dme task entity
 *
 * @author y30000858
 * @since 2020-09-21
 */
@Data
public class TaskCompleteMessageDto {
    @JsonProperty("job_request_id")
    private String jobRequestId;

    @JsonProperty("job_id")
    private String jobId;

    private String taskId;

    @JsonProperty("job_status")
    private Integer jobStatus;

    @JsonProperty("job_progress")
    private Integer jobProgress;

    @JsonProperty("extend_info")
    private JSONObject extendsInfo;

    @JsonProperty("additional_status")
    private String additionalStatus;

    private String jobType;

    private Long speed;

    /**
     * get extend info
     *
     * @param type type
     * @param <T> template type
     * @return result
     */
    public <T> T getExtendsInfo(Class<T> type) {
        return getExtendsInfo(type, null);
    }

    /**
     * get extend info
     *
     * @param type type
     * @param defaultInfo default info
     * @param <T> template type
     * @return result
     */
    public <T> T getExtendsInfo(Class<T> type, T defaultInfo) {
        if (extendsInfo == null) {
            return defaultInfo;
        }
        return extendsInfo.toBean(type);
    }
}
