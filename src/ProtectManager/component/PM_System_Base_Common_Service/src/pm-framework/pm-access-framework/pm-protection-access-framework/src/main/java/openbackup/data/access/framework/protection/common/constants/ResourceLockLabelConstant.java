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
package openbackup.data.access.framework.protection.common.constants;

/**
 * 资源锁任务步骤国际化key定义
 *
 **/
public final class ResourceLockLabelConstant {
    /**
     * 对资源加锁
     */
    public static final String LOCK_START_KEY = "lock_running_label";

    /**
     * 对资源加锁成功
     */
    public static final String LOCK_SUCCESS_KEY = "lock_succeeded_label";

    /**
     * 对资源加失败
     */
    public static final String LOCK_FAILED_KEY = "lock_failed_label";

    /**
     * 对资源解锁
     */
    public static final String UNLOCK_START_KEY = "unlock_running_label";

    /**
     * 对资源解锁成功
     */
    public static final String UNLOCK_SUCCESS_KEY = "unlock_succeeded_label";

    /**
     * 对资源解锁失败
     */
    public static final String UNLOCK_FAILED_KEY = "unlock_failed_label";
}
