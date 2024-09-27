/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.livemount.model;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * Live Mount Location
 *
 * @author l00272247
 * @since 2020-09-18
 */
public enum LiveMountTargetLocation {
    ORIGINAL("original"),
    OTHERS("others");

    private final String value;

    LiveMountTargetLocation(String value) {
        this.value = value;
    }

    /**
     * get enum by value
     *
     * @param value value
     * @return enum
     */
    @JsonCreator
    public static LiveMountTargetLocation get(String value) {
        return EnumUtil.get(LiveMountTargetLocation.class, LiveMountTargetLocation::getValue, value);
    }

    @JsonValue
    public String getValue() {
        return value;
    }
}
