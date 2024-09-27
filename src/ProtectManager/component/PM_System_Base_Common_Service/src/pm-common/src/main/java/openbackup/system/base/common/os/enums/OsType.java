/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.os.enums;

/**
 * 操作系统类型
 *
 * @author w00493811
 * @since 2021-08-09
 */
public enum OsType {
    /**
     * Linux
     */
    LINUX("Linux"),
    /**
     * Windows
     */
    WINDOWS("Windows"),
    /**
     * Others
     */
    OTHERS("Others");

    /**
     * 操作系统名称
     */
    private String name;

    public String getName() {
        return name;
    }

    /**
     * 默认构造函数
     *
     * @param name 操作系统名称
     */
    OsType(String name) {
        this.name = name;
    }

    /**
     * 获取名称
     *
     * @return 名称
     */
    public String toString() {
        return name;
    }
}
