/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.controller.livemount.model;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * Live Mount Enable Status
 *
 * @author h3003246
 * @since 2020-12-08
 */
public enum LiveMountEnableStatus {
    /**
     * ACTIVATED
     */
    ACTIVATED("activated"),

    /**
     * DISABLED
     */
    DISABLED("disabled");

    private String name;

    LiveMountEnableStatus(String name) {
        this.name = name;
    }

    @JsonValue
    public String getName() {
        return name;
    }

    /**
     * get enum by name
     *
     * @param name name
     * @return enum
     */
    @JsonCreator
    public static LiveMountEnableStatus get(String name) {
        return EnumUtil.get(LiveMountEnableStatus.class, LiveMountEnableStatus::getName, name);
    }
}
