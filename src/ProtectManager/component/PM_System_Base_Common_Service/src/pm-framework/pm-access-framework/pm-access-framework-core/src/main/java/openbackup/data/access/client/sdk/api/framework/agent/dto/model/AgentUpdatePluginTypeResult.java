/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 修改agent应用类型结果
 *
 * @author c30035089
 * @since 2023-08-25
 */
@Getter
@Setter
public class AgentUpdatePluginTypeResult {
    /**
     * -1:未返回值；0：失败；1：成功；2：中间状态；8：异常状态；9：初始状态
     */
    private int modifyStatus = -1;

    /**
     * 错误label
     */
    private String error;

    /**
     * 日志级别
     */
    private String level;

    /**
     * 发生时间
     */
    private Long startTime;
}
