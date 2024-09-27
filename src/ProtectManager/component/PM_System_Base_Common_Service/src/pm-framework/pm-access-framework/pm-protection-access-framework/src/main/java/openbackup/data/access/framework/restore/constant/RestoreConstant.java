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
package openbackup.data.access.framework.restore.constant;

/**
 * RestoreConstant
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-12-20
 */
public class RestoreConstant {
    /**
     * 恢复任务 native
     */
    public static final String TARGET_LOCATION_NATIVE = "Native";

    /**
     * 恢复锁定优先级
     */
    public static final int RESTORE_LOCK_PRIORITY = 10;

    /**
     * 恢复类型: "0" - 虚拟机恢复  "1" - 磁盘恢复
     */
    public static final String RESTORE_LEVEL = "restoreLevel";

    /**
     * 恢复类型: "0" - 虚拟机恢复
     */
    public static final String RESTORE_LEVEL_ZERO = "0";

    /**
     * 恢复类型: "1" - 磁盘恢复
     */
    public static final String RESTORE_LEVEL_ONE = "1";

    /**
     * 恢复的目标位置 key
     */
    public static final String RESTORE_LOCATION = "restoreLocation";
}
