/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.controller.livemount.model;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * Live Mount Update Mode
 *
 * @author l00272247
 * @since 2020-09-18
 */
public enum LiveMountUpdateMode {
    /**
     * LATEST
     */
    LATEST("latest"),
    /**
     * SPECIFIED
     */
    SPECIFIED("specified");

    private final String value;

    LiveMountUpdateMode(String value) {
        this.value = value;
    }

    /**
     * get enum by value
     *
     * @param value value
     * @return enum
     */
    @JsonCreator
    public static LiveMountUpdateMode get(String value) {
        return EnumUtil.get(LiveMountUpdateMode.class, LiveMountUpdateMode::getValue, value);
    }

    @JsonValue
    public String getValue() {
        return value;
    }
}
