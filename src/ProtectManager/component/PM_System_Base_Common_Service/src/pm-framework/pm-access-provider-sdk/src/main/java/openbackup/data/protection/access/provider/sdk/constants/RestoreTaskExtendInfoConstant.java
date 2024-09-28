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
package openbackup.data.protection.access.provider.sdk.constants;

/**
 * 恢复任务扩展参数常量定义
 *
 **/
public final class RestoreTaskExtendInfoConstant {
    /**
     * 恢复任务是否开启副本校验
     */
    public static final String ENABLE_COPY_VERIFY = "copyVerify";

    /**
     * 是否需要强制恢复
     */
    public static final String FORCE_RECOVERY = "force_recovery";

    /**
     * 副本校验子任务进度范围开始
     */
    public static final int COPY_VERIFY_RANGE_START = 0;

    /**
     * 副本校验子任务进度范围结束
     */
    public static final int COPY_VERIFY_RANGE_END = 40;

    /**
     * 副本恢复子任务进度范围开始
     */
    public static final int RESTORE_RANGE_START = 41;

    /**
     * 副本恢复子任务进度范围结束
     */
    public static final int RESTORE_RANGE_END = 100;

    private RestoreTaskExtendInfoConstant() {
    }
}
