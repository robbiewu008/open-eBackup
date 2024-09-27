/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.infrastructure.model;

import lombok.Getter;

/**
 * data type for infrastructure
 *
 * @author t00508428
 * @since 2020-12-17
 */
@Getter
public enum InfraDataTypeEnum {
    DB("DB"),
    CONFIG("CONFIG"),
    META_DATA("META_DATA");

    private final String value;

    InfraDataTypeEnum(String value) {
        this.value = value;
    }
}
