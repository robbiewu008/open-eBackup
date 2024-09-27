/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 部署场景枚举
 *
 * @author g00500588
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/5/26
 */
@Getter
@AllArgsConstructor
public enum DeployTypeEnum {
    A8000("a8000"),
    CLOUD_BACKUP_OLD("cloudbackup"),
    X8000("d0"),
    X6000("d1"),
    X3000("d2"),
    CLOUD_BACKUP("d3"),
    HYPER_DETECT("d4"),
    CYBER_ENGINE("d5"),
    X9000("d6"),
    E6000("d7"),
    E1000("d8"),
    OPEN_SOURCE("d9"),
    OPEN_SERVER("d10");

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