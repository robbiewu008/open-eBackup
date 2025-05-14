/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.enums;

/**
 * 备份信息枚举
 *
 * @author swx10105722
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-29
 */
public enum NetworkType {
    /**
     * 备份
     */
    BACKUP("backupNetPlane"),
    /**
     * 归档
     */
    ARCHIVE("archiveNetPlane"),
    /**
     * 复制
     */
    COPY("copyNetPlane");

    private String value;

    /**
     * 默认构造函数
     *
     * @param type 类型
     */
    NetworkType(String type) {
        value = type;
    }

    /**
     * 获取IP类型时ipv4还是ipv6
     *
     * @return ipType
     */
    public String getType() {
        return value;
    }
}