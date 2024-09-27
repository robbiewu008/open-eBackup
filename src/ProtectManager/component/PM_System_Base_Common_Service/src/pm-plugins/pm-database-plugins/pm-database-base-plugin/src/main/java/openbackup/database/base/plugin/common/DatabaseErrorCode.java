/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.database.base.plugin.common;

/**
 * 数据库应用通用错误码
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/15
 */
public class DatabaseErrorCode {
    /**
     * 集群节点数量不满足要求
     */
    public static final long CLUSTER_NODE_NUMBER_ERROR = 1677931437L;

    /**
     * 错误场景：注册/修改集群时，由于集群数量已达到上限，操作失败。
     * 原因：集群数量已达到上限（{0}）。
     * 建议：请删除不再使用的集群后重试。
     */
    public static final long RESOURCE_REACHED_THE_UPPER_LIMIT = 1677931389L;

    /**
     * 错误场景：执行注册数据库集群操作时，由于选择的部署模式和所选集群节点部署模式不匹配，操作失败。
     * 原因：选择的部署模式（{0}）和所选集群节点部署模式（{1}）不匹配。
     * 建议：请选择相匹配的部署模式后重试。
     */
    public static final long RESOURCE_DEPLOY_TYPE_ERROR = 1677931442L;

    /**
     * 错误场景：执行恢复时，由于目标实例与源实例部署的操作系统不同，操作失败。
     * 原因：目标实例与源实例部署的操作系统不同。
     * 建议：请部署与源实例相同的操作系统。
     */
    public static final long RESTORE_OS_INCONSISTENT = 1677933070L;

    /**
     * 错误场景：执行注册/修改应用集群操作时，由于选择的集群类型与应用集群类型不匹配，操作失败。
     * 原因：选择的集群类型与应用集群类型不匹配。
     * 建议：请选择与集群类型相匹配的应用后重试
     */
    public static final long RESTORE_RESOURCE_TYPE_INCONSISTENT = 1577209995L;

    /**
     * 错误场景：执行恢复操作时，由于选择的主机/集群类型与目标主机/集群类型不一致，操作失败。
     * 原因：选择的主机/集群类型与目标主机/集群类型不一致。
     * 建议：请选择与主机/集群类型一致的资源后重试。
     */
    public static final long RESTORE_TARGET_RESOURCE_TYPE_INCONSISTENT = 1677933072L;

    /**
     * 错误场景：执行数据库恢复操作时，由于数据库版本信息不一致，操作失败。
     * 原因：数据库版本信息不一致。
     * 建议：请选择相同版本的数据库进行恢复。
     */
    public static final long RESTORE_RESOURCE_VERSION_INCONSISTENT = 1577209971L;

    /**
     * 错误场景：执行注册数据库实例操作时，由于已存在同名数据库实例，操作失败。
     * 原因：已存在同名数据库实例。
     * 建议：请确保相同主机下不存在同名数据库实例
     */
    public static final long INSTANCE_HAS_REGISTERED = 1677931282L;

    /**
     * 错误场景：执行注册集群实例操作时，由于集群节点的数据库版本不一致，操作失败。
     * 原因：集群节点的数据库版本不一致。
     * 建议：请选择相同数据库版本的节点。
     */
    public static final long DATABASE_VERSION_INCONSISTENT = 1677931283L;

    /**
     * 错误场景：执行注册集群实例操作时，由于集群节点的操作系统不一致，操作失败。
     * 原因：集群节点的操作系统不一致。
     * 建议：请选择相同操作系统的节点。
     */
    public static final long DATABASE_OS_INCONSISTENT = 1677931287L;

    /**
     * 错误场景：执行注册集群实例操作时，由于集群节点的数据库位数不一致，操作失败。
     * 原因：集群节点的数据库位数不一致。
     * 建议：请选择相同数据库位数的节点。
     */
    public static final long DATABASE_BITS_INCONSISTENT = 1677931294L;

    /**
     * 错误场景：执行数据库备份操作，由于数据库不存在或已被删除，操作失败。
     * 原因：数据库({0})不存在或已被删除。
     * 建议：请对实例({1})进行资源扫描后重试。
     */
    public static final long DATABASE_NOT_EXISTS = 1577213540L;

    /**
     * 错误场景：执行资源注册时，由于主机类型无法注册，操作失败。
     * 原因：主机（{0}）类型（{1}）无法注册。
     * 建议：请重新选择其他主机。
     */
    public static final long ENV_OS_TYPE_ERROR = 1677873159L;

    /**
     * 错误场景：执行资源注册/修改时，由于集群节点的认证方式不统一，操作失败。
     * 原因：集群节点的认证方式不统一。
     * 建议：请采用统一认证方式后重试。
     */
    public static final long AUTH_TYPE_NO_CONSISTENT = 1677931350L;

    /**
     * 错误场景：执行下发备份任务时，由于存储快照备份不支持差异备份，操作失败。
     * 原因：存储快照备份不支持差异备份。
     * 建议：请选择其他备份类型或关闭存储快照备份后执行差异备份。
     */
    public static final long ERROR_EXEC_DIFF_BACKUP = 1577213544L;

    /**
     * 错误场景：执行下发存储快照备份任务时，由于未选择Linux操作系统的代理主机，操作失败。
     * 原因：未选择Linux操作系统的代理主机。
     * 建议：请选择Linux操作系统的代理主机后重试。
     */
    public static final long LINUX_AGENT_NOT_EXIST = 1577213545L;
}
