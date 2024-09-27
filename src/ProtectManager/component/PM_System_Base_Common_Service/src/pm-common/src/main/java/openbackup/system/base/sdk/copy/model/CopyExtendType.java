/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.copy.model;

import openbackup.system.base.util.EnumUtil;

import com.alibaba.fastjson.annotation.JSONCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 功能描述
 *
 * @author d00886448
 * @since 2024-07-30
 */
public enum CopyExtendType {
    CHECKPOINT("checkPoint"); // 断点续备

    private final String value;

    CopyExtendType(String value) {
        this.value = value;
    }

    /**
     * get enum by value
     *
     * @param value value
     * @return copy status
     */
    @JSONCreator
    public static CopyExtendType get(String value) {
        return EnumUtil.get(CopyExtendType.class, CopyExtendType::getValue, value);
    }

    @JsonValue
    public String getValue() {
        return value;
    }
}
