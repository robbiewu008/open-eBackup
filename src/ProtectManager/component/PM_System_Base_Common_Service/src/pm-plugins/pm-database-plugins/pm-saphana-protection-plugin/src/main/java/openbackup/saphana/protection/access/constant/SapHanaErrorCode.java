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
package openbackup.saphana.protection.access.constant;

/**
 * SAP HANA错误码类
 *
 */
public class SapHanaErrorCode {
    /**
     * 场景: 资源已经注册，不能重复注册。
     * 原因: 资源已经注册，不能重复注册。
     * 建议: 无。
     */
    public static final long RESOURCE_IS_REGISTERED = 1677931274L;

    /**
     * 场景: 执行SAP HANA数据库日志备份操作时，由于实例未开启日志备份，操作失败。
     * 原因: 实例未开启日志备份。
     * 建议: 请开启实例日志备份后重试。
     */
    public static final long INSTANCE_NOT_ENABLE_LOG_BACKUP = 1677932050L;

    /**
     * 场景: 执行恢复操作时，由于备份副本的数据库版本与目标位置数据库版本不匹配，操作失败。
     * 原因: 备份副本的数据库版本（{0}）与目标位置数据库版本（{1}）不匹配。
     * 建议: 请选择版本匹配的目标位置进行恢复。
     */
    public static final long VERSION_DISMATCH = 1577209921L;

    /**
     * 场景: 执行恢复操作时，由于备份副本的数据库拓扑结构与目标位置数据库拓扑结构不一致，操作失败。
     * 原因: 备份副本的数据库拓扑结构与目标位置数据库拓扑结构不一致。
     * 建议：请选择拓扑结构一致的目标位置。
     */
    public static final long TOPOLOGY_DISMATCH = 1577209926L;

    /**
     * 场景: 执行数据库恢复操作时，由于备份副本的systemId与目标位置数据库的systemId不一致，操作失败。
     * 原因：备份副本的systemId（{0}）与目标位置数据库的systemId（{1}）不一致。
     * 建议：请选择systemId一致的目标位置进行恢复。
     */
    public static final long SYSTEM_ID_NOT_EQUAL = 1577213498L;

    /**
     * 场景: 执行数据库恢复操作时，由于备份副本的数据库与目标数据库类型不一致，无法互相恢复，操作失败。
     * 原因: 备份副本的数据库（{0}）与目标数据库（{1}）类型不一致，无法互相恢复。
     * 建议: 无。
     */
    public static final long SYSTEM_TENANT_NOT_RECOVER_EACH_OTHER = 1577213499L;
}
