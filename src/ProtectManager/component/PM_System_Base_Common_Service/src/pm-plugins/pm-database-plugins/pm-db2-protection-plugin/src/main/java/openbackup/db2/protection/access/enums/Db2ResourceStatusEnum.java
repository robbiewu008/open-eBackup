/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.db2.protection.access.enums;

import java.util.Arrays;

/**
 * db2资源状态
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-02-20
 */
public enum Db2ResourceStatusEnum {
    NORMAL("Normal"),

    BACKUPING("Backuping"),

    RESTORING("Restoring");

    Db2ResourceStatusEnum(String status) {
        this.status = status;
    }

    private final String status;

    public String getStatus() {
        return status;
    }

    /**
     * 根据status获取到对应的枚举
     *
     * @param status 枚举值
     * @return enum
     */
    public static Db2ResourceStatusEnum getByStatus(String status) {
        return Arrays.stream(Db2ResourceStatusEnum.values())
            .filter(location -> location.status.equals(status))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
