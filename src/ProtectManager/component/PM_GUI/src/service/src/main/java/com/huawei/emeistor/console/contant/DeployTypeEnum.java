/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.emeistor.console.contant;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 部署场景枚举
 *
 * @author r00570078
 * @version OceanCyber 300 1.2.0
 * @since 2024-07-24
 */
@Getter
@AllArgsConstructor
public enum DeployTypeEnum {
    /**
     * A8000
     */
    A8000("a8000"),

    /**
     * CLOUD_BACKUP_OLD
     */
    CLOUD_BACKUP_OLD("cloudbackup"),

    /**
     * X8000
     */
    X8000("d0"),

    /**
     * X6000
     */
    X6000("d1"),

    /**
     * X3000
     */
    X3000("d2"),

    /**
     * CLOUD_BACKUP
     */
    CLOUD_BACKUP("d3"),

    /**
     * HYPER_DETECT
     */
    HYPER_DETECT("d4"),

    /**
     * OCEAN_CYBER
     */
    OCEAN_CYBER("d5"),

    /**
     * X9000
     */
    X9000("d6"),

    /**
     * E6000
     */
    E6000("d7"),

    /**
     * E1000
     */
    E1000("d8");

    private final String value;

    /**
     * 根据value获取部署类型枚举
     *
     * @param value value
     * @return deployType
     */
    public static DeployTypeEnum getByValue(String value) {
        for (DeployTypeEnum deployTypeEnum : DeployTypeEnum.values()) {
            if (deployTypeEnum.value.equals(value)) {
                return deployTypeEnum;
            }
        }
        throw new IllegalArgumentException("Illegal deploy type value.");
    }
}