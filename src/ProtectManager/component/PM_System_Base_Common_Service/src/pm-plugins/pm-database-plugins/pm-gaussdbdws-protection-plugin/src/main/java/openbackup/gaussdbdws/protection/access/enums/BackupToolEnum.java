/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdbdws.protection.access.enums;

/**
 * 备份方式枚举
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-07
 */
public enum BackupToolEnum {
    /**
     * ROACH备份方式
     */
    ROACH("0"),

    /**
     * GDS备份方式
     */
    GDS("1");

    BackupToolEnum(String type) {
        this.type = type;
    }

    private final String type;

    public String getType() {
        return type;
    }
}
