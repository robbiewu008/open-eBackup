/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.common.os;

import openbackup.system.base.common.os.enums.OsType;

import java.util.Locale;

/**
 * 操作系统工具
 *
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
