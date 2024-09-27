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
package openbackup.mysql.resources.access.common;

/**
 * mysql相关错误码
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/12
 */
public class MysqlErrorCode {
    /**
     * 错误场景：执行校验数据库集群操作时，由于集群信息填写错误，操作失败。
     * 原因：集群信息填写错误。
     * 建议：请确保集群信息正确。
     */
    public static final long CHECK_CLUSTER_FAILED = 1577209956L;

    /**
     * 错误场景：执行数据库恢复操作时，由于数据库版本信息不一致，操作失败。
     * 原因：数据库版本信息不一致。
     * 建议：请选择相同版本的数据库进行恢复。
     */
    public static final long CHECK_VERSION_FAILED = 1577209971L;

    /**
     * 错误场景：执行注册/修改应用集群操作时，由于选择的集群类型与应用集群类型不匹配，操作失败。
     * 原因：选择的集群类型与应用集群类型不匹配。
     * 建议：请选择与集群类型相匹配的应用后重试
     */
    public static final long CHECK_MYSQL_DEPLOYMENT_MODEL_FAILED = 1577209995L;

    /**
     * 错误场景：执行MySQL实例恢复时，由于目标实例与源实例部署的操作系统不同，操作失败。
     * 原因：目标实例与源实例部署的操作系统不同。
     * 建议：请部署与源实例相同的操作系统。
     */
    public static final long CHECK_OPERATING_SYSTEM_DEPLOYMENT_MODEL_FAILED = 1677933070L;

    /**
     * 错误场景：执行MySQL数据库实例恢复重命名数据库时，由于数据库名称不合法，操作失败。
     * 原因：数据库名称不合法。
     * 建议：请重命名数据库。
     */
    public static final long CHECK_DATABASE_NAME_FAILED = 1677933071L;

    /**
     * 原因：目标集群和原集群认证信息不一致。
     * 建议：请确保目标集群和原集群认证信息保持一致。
     */
    public static final long CHECK_AUTH_INFO_FAILED = 1677933073L;

    /**
     * 错误场景：执行查询数据库列表操作时，由于网络或数据库服务异常，操作失败。
     * 原因：网络或数据库服务异常。
     * 建议：请确保网络和数据库服务正常。
     */
    public static final long CHECK_SERVICE_FAILED = 1577209946L;

    /**
     * 错误场景：执行MySQL集群实例的数据库日志备份时，由于所选数据库为备节点数据库，操作失败。
     * 原因：所选数据库为备节点数据库。
     * 建议：请选择主节点数据库进行日志备份。
     */
    public static final long CHECK_CLUSTER_DATABASE_BACKUP_FAILED = 1677933568L;

    /**
     * 错误场景：执行MySQL备份操作时，由于MySQL实例未处于在线状态，操作失败。
     * 原因：MySQL实例处于未运行状态。
     * 建议：请检查代理主机网络，MySQL实例是否是运行状态或MySQL实例是否处于即时挂载状态。
     */
    public static final long SERVICE_NOT_RUNNING = 1577209965L;

    /**
     * 错误场景：执行注册\修改集群操作时，由于选择的节点数量超过限制，操作失败。
     * 原因：选择的节点数量超过{0}个。
     * 建议：请重新选择集群节点。
     */
    public static final long OVER_LIMIT_OF_NODES = 1677930065L;

    /**
     * 错误场景：执行SLA修改操作时，由于MySQL主主复制集群实例不支持日志备份，操作失败。
     * 原因：MySQL主主复制集群实例不支持日志备份。
     * 建议：无。
     */
    public static final long EAPP_MYSQL_NOT_SUPPORT_LOG_BACKUP = 1677931544L;
}
