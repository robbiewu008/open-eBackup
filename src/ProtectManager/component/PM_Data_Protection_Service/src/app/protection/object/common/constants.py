# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
from app.common.enums.resource_enum import ResourceSubTypeWithOrderEnum, ResourceSubTypeEnum


class CommonOperationID(object):
    # 创建保护
    CREATED_PROTECTION = "0x206403340002"

    # 创建保护-安全一体机
    CREATED_PROTECTION_CYBER = "0x20640334000C"

    # 修改保护
    MODIFY_PROTECTION = "0x206403340003"

    # 修改保护-安全一体机
    MODIFY_PROTECTION_CYBER = "0x206403340009"

    # 移除保护
    REMOVE_PROTECTION = "0x206403340001"

    # 移除保护-安全一体机
    REMOVE_PROTECTION_CYBER = "0x206403340008"

    # 激活保护
    ACTIVATE_PROTECTION = "0x206403340004"

    # 激活保护
    ACTIVATE_PROTECTION_CYBER = "0x20640334000A"

    # 取消激活保护
    DEACTIVATE_PROTECTION = "0x206403340005"

    # 禁用保护-安全一体机
    DEACTIVATE_PROTECTION_CYBER = "0x20640334000B"

    # 手动执行备份
    MANUALLY_BACK = "0x206403340006"

    # 手动执行勒索侦测快照生成-安全一体机
    MANUALLY_BACK_CYBER = "0x20640334000D"

    # 分配资源
    ALLOCATE_RESOURCES = "0x206403320012"

    # 回收资源
    RECLAIM_RESOURCES = "0x206403320013"

    # 修改数据库认证信息
    MODIFY_DATABASE_AUTHENTICATE_INFO = "0x206403320014"

    # 修改VMware注册信息
    MODIFYING_VMWARE_REGISTRATION_INFORMATION = "0x206403320015"

    # 移除VMware
    REMOVE_VMWARE = "0x206403320016"

    # 注册VMware
    REGISTER_VMWARE = "0x206403320017"

    # 重新扫描VMware信息
    RESCAN_VMWARE_INFO = "0x206403320018"

    # 创建主机
    CREATE_HOST = "0x206403320019"

    # 删除主机
    REMOVE_HOST = "0x20640332001A"

    # ASM认证
    ASM_AUTHENTICATE = "0x20640332001B"

    # 修改主机名
    MODIFY_HOSTNAME = "0x20640332001F"

    # 同步snmp trap信息
    SYNC_SNMP_TRAP = "0x206403320020"

    # 迁移主机
    MIGRATE_HOST = "0x206403320036"

    # 手动执行复制
    MANUAL_REPLICATION = "0x206403350007"

    # 手动执行归档
    MANUAL_ARCHIVE = "0x206403350008"

    # 复制副本受保护对象进行手动复制
    PROTECTED_COPY_REPLICATION = "0x206403340007"

    # 接受转发归档消息
    RECEIVE_ARCHIVE_DISPATCH = "0x206402580001"

    # 在自动扫描资源时，系统对资源创建保护。
    AUTO_SCAN_AND_CREATE_PROTECT = "0x20640334000E"

    # 在自动扫描资源时，系统对资源移除保护。
    AUTO_SCAN_AND_REMOVE_PROTECT = "0x20640334000F"

    # 在自动扫描资源时，系统对资源修改保护。
    AUTO_SCAN_AND_MODIFY_PROTECT = "0x206403340010"


class ResourceProtectionJobSteps(object):
    # 资源保护开始
    PROTECTION_START = "job_log_resource_protect_prepare_label"
    # 资源[xxx]绑定[SLA]执行保护成功
    PROTECTION_EXECUTING_SUCCESS = "job_log_resource_protect_execute_success_label"
    # 资源[xxx]绑定[SLA]执行保护失败。错误原因
    PROTECTION_EXECUTING_FAILED = "job_log_resource_protect_execute_failed_label"
    # 资源保护失败
    PROTECTION_FAILED = "job_log_resource_protect_failed_label"
    # 资源保护成功
    PROTECTION_FINISH = "job_log_resource_protect_finish_label"
    # 资源(xxx)启动手动备份失败
    PROTECTION_START_MANUAL_FAILED = "job_log_start_manual_failed_label"
    # 资源(xxx)启动手动备份成功
    PROTECTION_START_MANUAL_SUCCESS = "job_log_start_manual_success_label"


class ResourceProtectionModifyJobSteps(object):
    # 准备修改资源保护任务
    PROTECTION_MODIFY_START = "job_log_resource_protection_modify_prepare_label"
    # 对资源（{0}）执行修改保护操作
    PROTECTION_MODIFY_EXECUTING_SUCCESS = "job_log_resource_protection_modify_execute_success_label"
    # 对资源（{0}）执行修改保护操作失败
    PROTECTION_MODIFY_EXECUTING_FAILED = "job_log_resource_protection_modify_execute_failed_label"
    # 对资源（{0}）执行移除保护操作
    PROTECTION_REMOVE_SUCCESS = "job_log_resource_protection_remove_success_label"
    # 对资源（{0}）执行移除保护操作失败
    PROTECTION_REMOVE_FAILED = "job_log_resource_protection_remove_failed_label"
    # "为资源（{0}）关联SLA（{1}）
    PROTECTION_CREATE_SUCCESS = "job_log_resource_protect_execute_success_label"
    # 为资源（{0}）关联SLA（{1}）失败
    PROTECTION_CREATE_FAILED = "job_log_resource_protect_execute_failed_label"
    # 资源保护失败
    PROTECTION_MODIFY_FAILED = "job_log_resource_protection_modify_failed_label"
    # 修改资源保护任务完成
    PROTECTION_MODIFY_FINISH = "job_log_resource_protection_modify_finish_label"


class ProtectVolumeServiceEnum(object):
    HOST_VOLUME_PATHS_MAX_LENGTH = 8192
    WIN_REGULAR = r'^([A-Za-z]:\\)$'


class HDFSFilesetExtParam(object):
    # hdfs文件集前后置脚本的正则
    PROTECTION_REGEX = "^(.+)[\.]{1}(sh){1}$"


class LocalDiskSupportSubType(object):
    # hdfs文件集前后置脚本的正则
    local_disk_support_subtype = [
        ResourceSubTypeWithOrderEnum.GBASE.value[0], ResourceSubTypeWithOrderEnum.KINGBASE.value[0],
        ResourceSubTypeWithOrderEnum.KINGBASE_CLUSTER_INSTANCE.value[0],
        ResourceSubTypeWithOrderEnum.KINGBASE_INSTANCE.value[0],
        ResourceSubTypeWithOrderEnum.KINGBASE_CLUSTER.value[0], ResourceSubTypeEnum.GOLDENDB.value,
        ResourceSubTypeWithOrderEnum.GOLDENDB_CLUSETER_INSTANCE.value[0], ResourceSubTypeEnum.GOLDENDB_CLUSTER.value,
        ResourceSubTypeWithOrderEnum.FILESET.value[0], ResourceSubTypeWithOrderEnum.OPENGAUSS.value[0],
        ResourceSubTypeWithOrderEnum.OPENGAUSS_INSTANCE.value[0],
        ResourceSubTypeWithOrderEnum.OPENGAUSS_DATABASE.value[0],
        ResourceSubTypeWithOrderEnum.FUSION_COMPUTE.value[0], ResourceSubTypeEnum.FUSION_COMPUTE_HOST.value,
        ResourceSubTypeEnum.FUSION_COMPUTE_CLUSTER.value, ResourceSubTypeWithOrderEnum.POSTGRE_CLUSTER.value[0],
        ResourceSubTypeWithOrderEnum.POSTGRE_INSTANCE.value[0], ResourceSubTypeWithOrderEnum.POSTGRE_SQL.value[0],
        ResourceSubTypeWithOrderEnum.POSTGRE_CLUSTER_INSTANCE.value[0], ResourceSubTypeWithOrderEnum.MYSQL.value[0],
        ResourceSubTypeEnum.MysqlInstance.value, ResourceSubTypeWithOrderEnum.MYSQL_CLUSTER.value[0],
        ResourceSubTypeWithOrderEnum.MYSQL_CLUSTER_INSTANCE.value[0],
        ResourceSubTypeWithOrderEnum.MYSQL_DATABASE.value[0],
        ResourceSubTypeWithOrderEnum.MONGODB.value[0], ResourceSubTypeEnum.MONGODB_SINGLE.value,
        ResourceSubTypeEnum.MONGODB_CLUSTER.value, ResourceSubTypeWithOrderEnum.DAMENG.value[0],
        ResourceSubTypeEnum.DamengSingleNode.value, ResourceSubTypeWithOrderEnum.DAMENG_CLUSTER.value[0],
        ResourceSubTypeWithOrderEnum.TDSQL_CLUSTERINSTANCE.value[0], ResourceSubTypeEnum.TDSQL_CLUSTER.value,
        ResourceSubTypeWithOrderEnum.TDSQL_CLUSTERGROUP.value[0],
        ResourceSubTypeWithOrderEnum.TIDB_CLUSTERINSTANCE.value[0],
        ResourceSubTypeWithOrderEnum.TIDB_CLUSTER.value[0], ResourceSubTypeWithOrderEnum.TIDB_TABLE.value[0],
        ResourceSubTypeWithOrderEnum.TIDB_DATABASE.value[0], ResourceSubTypeWithOrderEnum.TIDB.value[0],
        ResourceSubTypeWithOrderEnum.OPEN_STACK.value[0], ResourceSubTypeEnum.OPENSTACK_CONTAINER.value,
        ResourceSubTypeEnum.OPENSTACK_PROJECT.value, ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.value,
        ResourceSubTypeEnum.OPENSTACK_DOMAIN.value, ResourceSubTypeEnum.CNWARE.value,
        ResourceSubTypeEnum.CNWARE_CLUSTER.value, ResourceSubTypeEnum.CNWARE_HOST.value,
        ResourceSubTypeEnum.CNWARE_HOST_POOL.value, ResourceSubTypeEnum.CNWARE_VM.value,
        ResourceSubTypeEnum.CNWARE_DISK.value, ResourceSubTypeEnum.NUTANIX.value,
        ResourceSubTypeEnum.NUTANIX_CLUSTER.value, ResourceSubTypeEnum.NUTANIX_HOST.value,
        ResourceSubTypeEnum.NUTANIX_VM.value, ResourceSubTypeEnum.NUTANIX_DISK.value,
        ResourceSubTypeWithOrderEnum.CNWARE_STORAGE_POOL.value[0], ResourceSubTypeWithOrderEnum.KUBERNETES.value[0],
        ResourceSubTypeWithOrderEnum.KUBERNETES_NAMESPACE.value[0], ResourceSubTypeEnum.KubernetesStatefulSet.value,
        ResourceSubTypeEnum.FUSION_ONE_COMPUTE.value, ResourceSubTypeWithOrderEnum.HCS_CONTAINER.value[0],
        ResourceSubTypeWithOrderEnum.HCS_TENANT.value[0], ResourceSubTypeWithOrderEnum.HCS_REGION.value[0],
        ResourceSubTypeWithOrderEnum.HCS_PROJECT.value[0], ResourceSubTypeWithOrderEnum.HCS_CLOUDHOST.value[0],
        ResourceSubTypeWithOrderEnum.INFORMIX_SINGLE_INSTANCE.value[0],
        ResourceSubTypeWithOrderEnum.INFORMIX_CLUSTER_INSTANCE.value[0],
        ResourceSubTypeWithOrderEnum.INFORMIX_SERVICE.value[0], ResourceSubTypeWithOrderEnum.INFORMIX.value[0],
        ResourceSubTypeWithOrderEnum.ANTDB_INSTANCE.value[0],
        ResourceSubTypeWithOrderEnum.ANTDB_CLUSTER_INSTANCE.value[0]
    ]


class WormValidityType(object):
    # 不开启worm
    WORM_NOT_OPEN = 0
    # 同副本保留时间一致
    COPY_RETENTION_TIME_CONSISTENT = 1
    # 自定义WORM过期时间
    CUSTOM_RETENTION_TIME = 2
