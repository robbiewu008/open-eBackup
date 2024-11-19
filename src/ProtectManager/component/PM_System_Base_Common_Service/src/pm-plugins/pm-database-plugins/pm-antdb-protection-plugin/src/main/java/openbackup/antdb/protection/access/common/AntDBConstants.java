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
package openbackup.antdb.protection.access.common;

/**
 * AntDB数据库常量类
 *
 */
public class AntDBConstants {
    /**
     * 恢复任务类型key
     */
    public static final String RESTORE_TYPE_KEY = "restoreType";

    /**
     * 恢复模式key
     */
    public static final String RESTORE_MODE_KEY = "restoreMode";

    /**
     * 恢复目标类型key
     */
    public static final String TARGET_LOCATION_KEY = "targetLocation";

    /**
     * 副本保护对象版本key
     */
    public static final String COPY_PROTECT_OBJECT_VERSION_KEY = "copyProtectObjectVersion";

    /**
     * 运行数据库的操作系统用户key
     */
    public static final String DB_OS_USER_KEY = "osUsername";

    /**
     * 数据库的安装路径key
     */
    public static final String DB_INSTALL_PATH_KEY = "clientPath";

    /**
     * 主节点
     */
    public static final String PRIMARY = "1";
}
