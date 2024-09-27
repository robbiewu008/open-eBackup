/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.agent.model;

import lombok.Data;

import java.util.List;

/**
 * 功能描述
 *
 * @author s00455050
 * @since 2021-08-31
 */
@Data
public class AgentUpdateResultResponse {
    /**
     * -1:未返回值；0：失败；1：成功；2：中间状态；8：异常状态；9：初始状态
     */
    private int upgradeStatus = -1;

    /**
     * 任务日志级别
     */
    private String level;

    /**
     * 任务日志详情
     */
    private String logDetail;

    /**
     * 任务日志详情参数
     */
    private List<String> logDetailParam;

    /**
     * 任务信息
     */
    private String logInfo;

    /**
     * 任务信息参数
     */
    private List<String> logInfoParam;

    /**
     * 发生时间
     */
    private Long startTime;

    /**
     * 当前运行的agent版本
     */
    private String version;
}
