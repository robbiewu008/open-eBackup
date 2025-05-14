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
public enum RollBackStandardBackupStatusType {
    /**
     * 回滚中
     */
    ROLL_BACK("rollbacking"),
    /**
     * 回滚失败
     */
    FAILED("failed"),
    /**
     * 回滚成功
     */
    READY("ready-init"),
    /**
     * 未回滚
     */
    UNROLL_BACK("ready");

    private String value;

    /**
     * 默认构造函数
     *
     * @param type 类型
     */
    RollBackStandardBackupStatusType(String type) {
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