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
 * hcs复制常量类
 *
 */
public class HcsErrorCode {
    /**
     * 错误场景：执行创建复制策略操作时，租户未关联资源集，操作失败。
     * 原因：租户未关联资源集。
     * 建议：请为相应租户关联资源集后，重试。
     */
    public static final long PROJECT_EMPTY = 1677931548L;

    /**
     * 错误场景：执行创建复制策略操作时，用户权限不足，操作失败。
     * 原因：用户权限不足。
     * 建议：请使用VDC管理员用户重试。
     */
    public static final long INSUFFICIENT_USER_RIGHTS = 1677931551L;

    /**
     * 错误场景：执行创建复制策略操作时，VDC管理员用户名密码错误，操作失败。
     * 原因：VDC管理员用户名密码错误。
     * 建议：请输入正确的VDC管理员用户名密码后重试。
     */
    public static final long USER_AUTH_FAIL = 1677931550L;

    /**
     * 错误场景：执行注册HCS复制集群操作时，输入的用户名密码错误，操作失败。
     * 原因：输入的用户名密码错误。
     * 建议：请输入正确的用户名密码后重试。
     */
    public static final long CREATE_HCS_CLUSTER_USER_AUTH_FAIL = 1677930085L;

    /**
     * 错误场景：执行创建复制策略时，HCS复制集群的用户名或者密码错误，操作失败。
     * 原因：HCS复制集群的用户名或者密码错误。
     * 建议：请修改HCS复制集群的用户名密码后重试。
     */
    public static final long CREATE_COPY_POLICY_HCS_CLUSTER_USER_AUTH_FAIL = 1677931552L;
}
