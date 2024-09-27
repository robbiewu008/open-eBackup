/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.openstack.adapter.enums;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 开关任务动作枚举
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-22
 */
public enum SwitchAction {
    START("start"),
    STOP("stop");

    private final String action;

    SwitchAction(String action) {
        this.action = action;
    }

    @JsonValue
    public String getAction() {
        return action;
    }
}
