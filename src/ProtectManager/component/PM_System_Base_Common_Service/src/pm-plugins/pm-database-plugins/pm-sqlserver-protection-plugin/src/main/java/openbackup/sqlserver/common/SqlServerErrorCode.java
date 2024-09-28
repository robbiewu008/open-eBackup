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
package openbackup.sqlserver.common;

/**
 * Sql Server应用错误码
 *
 */
public class SqlServerErrorCode {
    /**
     * Agent执行成功返回码
     */
    public static final long AGENT_RETURN_CODE_SUCCESS = 0L;

    /**
     * 原因：添加的实例未包含该集群所有实例。
     * 建议：请添加所有实例节点后重试。
     */
    public static final long CHECK_CLUSTER_NUM_FAILED = 1577209991L;

    /**
     * 原因：集群（{0}）注册集群实例数量小于两个。
     * 建议：请为集群（{0}）注册至少两个集群实例或选择其他已注册两个及以上实例的集群。
     */
    public static final long SQLSERVER_RESTORE_INSTANCE_INSUFFICIENT = 1677933069L;

    private SqlServerErrorCode() {
    }
}
