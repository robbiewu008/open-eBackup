/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model.request;

/**
 * 来源枚举
 *
 * @author y30021475
 * @since 2023-08-07
 */
public enum PresetAccountSourceTypeEnum {
    /**
     * 来源平台 hcs
     */
    HCS("hcs");

    private final String sourceType;

    /**
     * 构造方法
     *
     * @param sourceType sourceType
     */
    PresetAccountSourceTypeEnum(String sourceType) {
        this.sourceType = sourceType;
    }

    public String getValue() {
        return sourceType;
    }
}
