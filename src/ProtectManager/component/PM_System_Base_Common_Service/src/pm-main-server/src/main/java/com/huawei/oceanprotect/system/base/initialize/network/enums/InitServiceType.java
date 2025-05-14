/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.enums;

/**
 * 归档IP类型
 *
 * @author l00347293
 * @since 2020-12-19
 */
public enum InitServiceType {
    /**
     * Init Network
     */
    INIT_NETWORK("INIT_NETWORK"),
    /**
     * Modify BackupNetwork
     */
    MODIFY_BACKUP_NETWORK("MODIFY_BACKUP_NETWORK"),

    /**
     * Modify ArchiveNetwork
     */
    MODIFY_ARCHIVE_NETWORK("MODIFY_ARCHIVE_NETWORK"),

    /**
     * init Standard
     */
    INIT_STANDARD("INIT_STANDARD"),

    /**
     * init Standard
     */
    EXPANSION_NETWORK("EXPANSION_NETWORK");

    private String value;

    /**
     * 默认构造函数
     *
     * @param type 类型
     */
    InitServiceType(String type) {
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