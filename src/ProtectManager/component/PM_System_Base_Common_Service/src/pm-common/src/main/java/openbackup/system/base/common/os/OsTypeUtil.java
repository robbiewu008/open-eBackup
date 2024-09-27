/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.os;

import openbackup.system.base.common.os.enums.OsType;

import java.util.Locale;

/**
 * 操作系统工具
 *
 * @author w00493811
 * @since 2021-08-09
 */
public class OsTypeUtil {
    /**
     * 操作系统名称
     */
    private static final String OS_NAME = System.getProperty("os.name").toLowerCase(Locale.ENGLISH);

    /**
     * 默认构造函数
     */
    private OsTypeUtil() {
    }

    /**
     * 当前操作系统是否是Linux
     *
     * @return 当前操作系统是Linux返回True
     */
    public static final boolean isLinux() {
        return OS_NAME.indexOf("linux") >= 0;
    }

    /**
     * 当前操作系统是否是Windows
     *
     * @return 当前操作系统是Windows返回True
     */
    public static final boolean isWindows() {
        return OS_NAME.indexOf("windows") >= 0;
    }

    /**
     * 获取操作系统类型
     *
     * @return 操作系统类型
     */
    public static final OsType getOsType() {
        if (isLinux()) {
            return OsType.LINUX;
        } else if (isWindows()) {
            return OsType.WINDOWS;
        } else {
            return OsType.OTHERS;
        }
    }
}
