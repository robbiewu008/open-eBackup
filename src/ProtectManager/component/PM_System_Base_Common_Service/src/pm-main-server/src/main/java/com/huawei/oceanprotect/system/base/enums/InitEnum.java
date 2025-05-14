/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024 All rights reserved.
 */

package com.huawei.oceanprotect.system.base.enums;

import lombok.Getter;

/**
 * 初始化相关的枚举
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-21
 */
public class InitEnum {
    /**
     * E6000业务网络类型枚举
     */
    @Getter
    public enum E6000NetworkTypeEnum {
        /**
         * 备份
         */
        BACKUP("1"),
        /**
         * 归档
         */
        ARCHIVE("2");

        private final String value;

        E6000NetworkTypeEnum(String value) {
            this.value = value;
        }
    }
}
