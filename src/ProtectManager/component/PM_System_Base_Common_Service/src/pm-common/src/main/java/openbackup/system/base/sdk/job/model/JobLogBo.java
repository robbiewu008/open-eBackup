/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.job.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 任务日志
 *
 * @author y00407642
 * @version [8.0.1]
 * @since 2020-06-12
 */
@Data
public class JobLogBo {
    private String jobId;

    private Long startTime;

    private Long endTime;

    private String logInfo;

    private String level;

    private List<String> logInfoParam;

    private String logDetail;

    private List<String> logDetailParam;

    private String logDetailInfo;

    /**
     * 该条日志是否唯一
     */
    @JsonProperty("unique")
    private boolean isUnique;
}
