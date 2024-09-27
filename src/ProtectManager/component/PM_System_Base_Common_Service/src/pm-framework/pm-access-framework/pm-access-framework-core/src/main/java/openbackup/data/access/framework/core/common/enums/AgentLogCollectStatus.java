/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.common.enums;

import lombok.Getter;

/**
 * 代理日志收集状态
 *
 * @author w00504341
 * @since 2023-01-28
 */
@Getter
public enum AgentLogCollectStatus {
    /**
     * 完成
     */
    COMPLETED("completed"),

    /**
     * 收集中
     */
    COLLECTING("collecting"),

    /**
     * 失败
     */
    FAILED("failed");

    private String status;

    AgentLogCollectStatus(String status) {
        this.status = status;
    }

    /**
     * 获取日志收集状态
     *
     * @return 状态
     */
    public String getCollectStatus() {
        return status;
    }
}
