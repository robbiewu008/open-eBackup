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
package openbackup.data.access.framework.copy.verify.constant;

/**
 * 副本校验任务步骤国际化标签常量类
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/30
 **/
public final class CopyVerifyJobLabelConstant {
    /**
     * 恢复任务初始化
     */
    public static final String COPY_CHECK_INIT = "job_log_copy_verify_init_label";

    /**
     * 恢复任务执行
     */
    public static final String COPY_CHECK_START = "job_log_copy_verify_execute_label";

    /**
     * 恢复任务完成
     */
    public static final String COPY_CHECK_COMPLETE = "job_log_copy_verify_complete_label";

    private CopyVerifyJobLabelConstant() {
    }
}
