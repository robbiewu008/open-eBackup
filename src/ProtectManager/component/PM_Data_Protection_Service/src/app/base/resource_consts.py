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
from enum import Enum

from app.common.enums.job_enum import JobStatus, JobType
from app.common.enums.resource_enum import ResourceSubTypeEnum


class LinuxOsTypeEnum(str, Enum):
    """Agent发现的Linux系统os_type类别"""
    REDHAT = "RedHat"
    SUSE = "SUSE"
    ROCKY = "ROCKY"
    OEL = "OEL"
    ISOFT = "ISOFT"
    CENTOS = "CentOS"
    KYLIN = "Kylin"
    NEOKYLIN = "NeoKylin"
    DEBIAN = "Debian"
    UBUNTU = "Ubuntu"


class UnixOsTypeEnum(str, Enum):
    """Unix系统的os_type类别"""
    AIX = "aix"
    HPUX = "hp_ux"
    SOLARIS = "solaris"
    SUNOS = "sunos"


class ResourceConstant:
    # 资源统计的VMWare资源子类型
    VIRTUALRESOURCE_SUBTYPE_MAP = {
        "ClusterComputeResource": ResourceSubTypeEnum.ClusterComputeResource.value,
        "HostSystem": ResourceSubTypeEnum.HostSystem.value,
        "VirtualMachine": ResourceSubTypeEnum.VirtualMachine.value,
    }
    # 主机资源子类型
    HOST_SUBTYPE_MAP = {
        "DBBackupAgent": ResourceSubTypeEnum.DBBackupAgent.value,
        "VMBackupAgent": ResourceSubTypeEnum.VMBackupAgent.value,
        "DWSBackupAgent": ResourceSubTypeEnum.DWSBackupAgent.value,
        "UBackupAgent": ResourceSubTypeEnum.UBackupAgent.value,
        "SBackupAgent": ResourceSubTypeEnum.S_BACKUP_AGENT.value,
    }
    # Agent发现的Unix系统的os_type类别映射
    AGENT_UNIX_OS_TYPE_MAP = {
        "AIX": UnixOsTypeEnum.AIX.value,
        "HP-UX": UnixOsTypeEnum.HPUX.value,
        "HPUX IA": UnixOsTypeEnum.HPUX.value,
        "Solaris": UnixOsTypeEnum.SOLARIS.value,
        "SOLARIS": UnixOsTypeEnum.SOLARIS.value,
        "SunOS": UnixOsTypeEnum.SUNOS.value
    }
    # Agent发现的Linux系统os_type类别
    LINUX_HOST_OS_TYPE_LIST = [e.value for e in LinuxOsTypeEnum]
    UNIX_HOST_OS_TYPE_LIST = [e.value for e in UnixOsTypeEnum]
    HOST_OS_TYPE_LINUX = 'linux'
    HOST_OS_TYPE_WINDOWS = 'windows'
    # 主机os_type列表：windows，linux，支持的unix类别
    HOST_OS_TYPE_LIST = [HOST_OS_TYPE_WINDOWS, HOST_OS_TYPE_LINUX] + [e.value for e in UnixOsTypeEnum]
    # 资源统计的所有资源sub_type列表
    SUB_TYPE_ALL_LIST = HOST_OS_TYPE_LIST + [e.value for e in ResourceSubTypeEnum] + ['MongoDB']
    OTHER_SUB_TYPE_LIST = [
        ResourceSubTypeEnum.NUTANIX_VM, ResourceSubTypeEnum.NUTANIX_HOST, ResourceSubTypeEnum.NUTANIX_CLUSTER,
        ResourceSubTypeEnum.CNWARE_VM, ResourceSubTypeEnum.CNWARE_HOST, ResourceSubTypeEnum.CNWARE_CLUSTER,
        ResourceSubTypeEnum.NdmpBackupSet,
        ResourceSubTypeEnum.APSARA_STACK, ResourceSubTypeEnum.APSARA_STACK_ORGANIZATION,
        ResourceSubTypeEnum.APSARA_STACK_REGION, ResourceSubTypeEnum.APSARA_STACK_ZONE,
        ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET, ResourceSubTypeEnum.APSARA_STACK_INSTANCE,
        ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON, ResourceSubTypeEnum.KUBERNETES_DATASET_COMMON,
        ResourceSubTypeEnum.CloudBackupFileSystem.value,
        ResourceSubTypeEnum.HDFSFileset.value, ResourceSubTypeEnum.HBaseBackupSet.value,
        ResourceSubTypeEnum.ElasticSearchBackupSet.value, ResourceSubTypeEnum.HiveBackupSet.value,
        ResourceSubTypeEnum.NasFileSystem.value, ResourceSubTypeEnum.NasShare.value,
        ResourceSubTypeEnum.KubernetesNamespace, ResourceSubTypeEnum.KubernetesStatefulSet,
        ResourceSubTypeEnum.MysqlDatabase, ResourceSubTypeEnum.MysqlClusterInstance,
        ResourceSubTypeEnum.GaussDBT,
        ResourceSubTypeEnum.DamengCluster,
        ResourceSubTypeEnum.PostgreClusterInstance,
        ResourceSubTypeEnum.OpenGaussInstance, ResourceSubTypeEnum.OpenGaussDatabase,
        ResourceSubTypeEnum.KingBaseClusterInstance,
        ResourceSubTypeEnum.SQLServerInstance,
        ResourceSubTypeEnum.SQLServerAlwaysOn, ResourceSubTypeEnum.SQLServerClusterInstance,
        ResourceSubTypeEnum.DWSCluster, ResourceSubTypeEnum.DWSTable,
        ResourceSubTypeEnum.DWSSchema, ResourceSubTypeEnum.DWSDateBase,
        ResourceSubTypeEnum.DB2Database, ResourceSubTypeEnum.DB2Tablespace,
        ResourceSubTypeEnum.FusionCompute, ResourceSubTypeEnum.HCSProject,
        ResourceSubTypeEnum.HCSCloudHost, ResourceSubTypeEnum.Fileset, ResourceSubTypeEnum.Volume,
        ResourceSubTypeEnum.OPENSTACK_PROJECT, ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER,
        ResourceSubTypeEnum.GOLDENDB_CLUSTER, ResourceSubTypeEnum.GOLDENDB_CLUSTER_INSTANCE,
        ResourceSubTypeEnum.TDSQL_CLUSTER, ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE,
        ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP,
        ResourceSubTypeEnum.HCS_GAUSSDB_PROJECT, ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE,
        ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE,
        ResourceSubTypeEnum.GENERAL_DB,
        ResourceSubTypeEnum.EXCHANGE_GROUP, ResourceSubTypeEnum.EXCHANGE_DATABASE,
        ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE, ResourceSubTypeEnum.EXCHANGE_MAILBOX,
        ResourceSubTypeEnum.EXCHANGE_ONLINE, ResourceSubTypeEnum.EXCHANGE_ONLINE_BACKUP_SET,
        ResourceSubTypeEnum.Oracle, ResourceSubTypeEnum.ORACLE_CLUSTER, ResourceSubTypeEnum.ORACLE_PDB,
        ResourceSubTypeEnum.MongoDB, ResourceSubTypeEnum.GAUSSDBT_SINGLE,
        ResourceSubTypeEnum.TiDB, ResourceSubTypeEnum.TiDB_CLUSTER, ResourceSubTypeEnum.TiDB_DATABASE,
        ResourceSubTypeEnum.TiDB_TABLE, ResourceSubTypeEnum.SAPHANA_DATABASE,
        ResourceSubTypeEnum.OCEANBASE_TENANT, ResourceSubTypeEnum.OCEANBASE_CLUSTER, ResourceSubTypeEnum.COMMON_SHARE,
        ResourceSubTypeEnum.OBJECT_SET, ResourceSubTypeEnum.AD,
        ResourceSubTypeEnum.HYPER_V_HOST, ResourceSubTypeEnum.HYPER_V_VM, ResourceSubTypeEnum.FUSION_ONE_COMPUTE,
        ResourceSubTypeEnum.LUN,
        ResourceSubTypeEnum.SAP_ON_ORACLE, ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE,
        ResourceSubTypeEnum.AntDBClusterInstance
    ]
    # 统计资源条件：RESOURCES保存的记录可以直接统计的资源
    DIRECT_STATS_SUB_TYPE_LIST = OTHER_SUB_TYPE_LIST + list(VIRTUALRESOURCE_SUBTYPE_MAP.values())
    # 统计资源条件：DISCRIMINATOR字段为“RESOURCES”，且要求拓展字段中isTopInstance为1
    TOP_INST_SUB_TYPE_LIST = [
        ResourceSubTypeEnum.MysqlInstance, ResourceSubTypeEnum.PostgreInstance, ResourceSubTypeEnum.KingBaseInstance,
        ResourceSubTypeEnum.MONGODB_SINGLE, ResourceSubTypeEnum.MONGODB_CLUSTER,
        ResourceSubTypeEnum.InformixSingleInstance, ResourceSubTypeEnum.InformixClusterInstance,
        ResourceSubTypeEnum.DamengSingleNode, ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT,
        ResourceSubTypeEnum.AntDBInstance
    ]
    # 统计资源条件：DISCRIMINATOR字段为“RESOURCES”，且要求拓展字段中agId为空
    AG_ID_SUB_TYPE_LIST = [
        ResourceSubTypeEnum.SQLServerDatabase
    ]
    ALARM_ENVIRONMENT_LINK_STATUS_OFFLINE = "0x106403320001"
    ALARM_ORACLE_ENVIRONMENT_LINK_STATUS_OFFLINE = "0x6403320001"
    ALARM_VMWARE_HOST_AGENT_LINK_STATUS_OFFLINE = "0x6403320002"

    # 任务失败告警
    RESOURCE_TASK_ALARM_AFTER_FAILURE = "0x64006E0001"

    # 任务部分成功告警
    RESOURCE_TASK_ALARM_AFTER_PARTIAL_SUCCESS = "0x64006E0004"

    COMMON_FAIL_LABEL = "0"
    COMMON_PARTIAL_SUCCESS_LABEL = "1"
    COMMON_SUCCESS_LABEL = "2"
    COMMON_BACKUP_LABEL = "0"
    COMMON_ARCHIVE_LABEL = "1"
    COMMON_REPLICATE_LABEL = "2"

    transition_status = {
        0: COMMON_FAIL_LABEL,
        1: COMMON_SUCCESS_LABEL,
        2: COMMON_PARTIAL_SUCCESS_LABEL,
        "FAIL": COMMON_FAIL_LABEL,
        "SUCCESS": COMMON_SUCCESS_LABEL,
        "PARTIAL_SUCCESS": COMMON_PARTIAL_SUCCESS_LABEL,
    }
    transition_task_type = {
        JobType.BACKUP.value: COMMON_BACKUP_LABEL,
        JobType.ARCHIVE.value: COMMON_ARCHIVE_LABEL,
        JobType.COPY_REPLICATION.value: COMMON_REPLICATE_LABEL,
        "backup": COMMON_BACKUP_LABEL,
        "archiving": COMMON_ARCHIVE_LABEL,
        "replication": COMMON_REPLICATE_LABEL,
    }
    status_switcher = {
        # 失败
        0: JobStatus.FAIL,
        # 成功
        1: JobStatus.SUCCESS,
        # 部分成功
        2: JobStatus.PARTIAL_SUCCESS,
        # 终止
        3: JobStatus.ABORTED
    }
