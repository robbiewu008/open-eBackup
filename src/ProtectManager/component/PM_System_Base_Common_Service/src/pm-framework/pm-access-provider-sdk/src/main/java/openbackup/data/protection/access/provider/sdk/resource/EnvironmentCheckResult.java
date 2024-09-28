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
package openbackup.data.protection.access.provider.sdk.resource;

/**
 * 受保护环境检查结果
 *
 */
public enum EnvironmentCheckResult {
    /**
     * 正常
     */
    NORMAL,
    /**
     * 网络连接超时
     */
    CONN_TIMEOUT,
    /**
     * 认证失败
     */
    AUTH_FAILED,

    /**
     * 参数错误
     */
    PARAM_ERROR
}
