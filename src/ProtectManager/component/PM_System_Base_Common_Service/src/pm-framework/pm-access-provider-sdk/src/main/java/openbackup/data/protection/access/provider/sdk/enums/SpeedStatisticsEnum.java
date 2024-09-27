/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.enums;

/**
 * 速率统计
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-22
 */
public enum SpeedStatisticsEnum {
    /**
     * ubc统计速率
     */
    UBC("1"),

    /**
     * 应用自己统计速率
     */
    APPLICATION("2");

    SpeedStatisticsEnum(String type) {
        this.type = type;
    }

    private final String type;

    public String getType() {
        return type;
    }
}
