/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.openstack.adapter.enums;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 云核OpenStack任务状态
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-05
 */
public enum OpenStackJobStatus {
    /**
     * 正在调度中
     */
    SCHEDULING("scheduling"),

    /**
     * 正在执行中
     */
    RUNNING("running"),

    /**
     * 已停止
     */
    STOP("stop"),

    /**
     * 已结束
     */
    COMPLETED("completed");

    private final String status;

    OpenStackJobStatus(String status) {
        this.status = status;
    }

    @JsonValue
    public String getStatus() {
        return status;
    }
}
