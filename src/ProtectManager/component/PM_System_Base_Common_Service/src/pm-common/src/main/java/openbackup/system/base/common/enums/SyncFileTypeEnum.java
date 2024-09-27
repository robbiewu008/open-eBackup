/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.common.enums;

import lombok.Getter;

/**
 * 同步文件到所有节点的文件类型枚举类
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-08-16
 */
@Getter
public enum SyncFileTypeEnum {
    /**
     * 文本文件
     */
    TEXT("TEXT"),

    /**
     * 二进制文件
     */
    BINARY("BINARY");

    private final String type;

    SyncFileTypeEnum(String type) {
        this.type = type;
    }
}
