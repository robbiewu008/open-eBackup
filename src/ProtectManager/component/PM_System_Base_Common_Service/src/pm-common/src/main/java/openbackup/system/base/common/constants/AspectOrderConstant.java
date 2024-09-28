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
package openbackup.system.base.common.constants;

/**
 * rest接口切面的执行顺序常量
 *
 */
public class AspectOrderConstant {
    /**
     * 日志切面
     */
    public static final int LOGGING_ASPECT_ORDER = 0;

    /**
     * 操作日志切面
     */
    public static final int OPERATION_LOG_ASPECT_ORDER = 1000;

    /**
     * 权限切面
     */
    public static final int PERMISSION_ASPECT_ORDER = 2000;

    /**
     * 文件校验切面
     */
    public static final int FILE_CHECK_ASPECT_ORDER = 3000;

    /**
     * 部署方式校验切面
     */
    public static final int DEPLOY_TYPE_ASPECT_ORDER = 4000;
}
