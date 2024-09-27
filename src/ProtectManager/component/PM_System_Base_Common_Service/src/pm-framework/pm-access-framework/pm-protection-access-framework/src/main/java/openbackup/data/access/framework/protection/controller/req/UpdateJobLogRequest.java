/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.data.access.framework.protection.controller.req;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 更新任务状态时，任务日志数据类型
 *
 * @author w00616953
 * @since 2021-12-02
 */
@Data
public class UpdateJobLogRequest {
    /**
     * 任务日志信息
     */
    private String logInfo;

    /**
     * 任务日志参数
     */
    private List<String> logInfoParam;

    /**
     * 开始时间
     */
    private Long logTimestamp;

    /**
     * 任务详情附加信息（失败时为错误码）
     */
    private Long logDetail;

    /**
     * 任务详情的详细打印
     */
    private List<String> logDetailInfo;

    /**
     * 任务详情附加参数
     */
    private List<String> logDetailParam;

    /**
     * 任务级别
     */
    private Integer logLevel;

    /**
     * 是否唯一
     */
    @JsonProperty("unique")
    private boolean isUnique;
}
