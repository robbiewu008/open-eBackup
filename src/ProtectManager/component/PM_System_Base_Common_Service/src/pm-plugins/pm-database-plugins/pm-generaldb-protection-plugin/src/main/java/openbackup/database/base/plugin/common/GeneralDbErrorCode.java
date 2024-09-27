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
package openbackup.database.base.plugin.common;

/**
 * 通用数据库错误码
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-09
 */
public class GeneralDbErrorCode {
    /**
     * 原因：主机或集群下该类型的数据库名称（{0}）已存在。
     * 建议：请填写主机或集群下该类型的其他数据库名称。
     */
    public static final long GENERAL_DB_DUPLICATE_NAME = 1677931280L;

    /**
     * 原因：该版本（{0}）的应用不支持（{1}）备份。
     * 建议：无。
     */
    public static final long VERSION_DO_NOT_SUPPORT_BACKUP = 1677933570L;

    /**
     * 错误场景：执行修改SLA时，由于关联资源的类型不支持保护策略，操作失败。
     * 原因：关联资源的类型（{0}）不支持保护策略（{1}）。
     * 建议：请删除关联资源或者重新创建保护策略。
     */
    public static final long NO_SUPPORT_SLA = 1677933571L;

    /**
     * 场景: 资源已经注册，不能重复注册。
     * 原因: 资源已经注册，不能重复注册。
     * 建议: 无。
     */
    public static final long RESOURCE_IS_REGISTERED = 1677933572L;
}
