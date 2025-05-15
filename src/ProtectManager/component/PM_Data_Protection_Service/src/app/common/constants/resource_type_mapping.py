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
from app.common.enums.resource_enum import ResourceSubTypeEnum


class SubTypeMapper:
    # 定义支持复制再归档的副本类型
    copy_archive_types = [ResourceSubTypeEnum.Fileset, ResourceSubTypeEnum.Volume, ResourceSubTypeEnum.Oracle,
                          ResourceSubTypeEnum.ORACLE_CLUSTER, ResourceSubTypeEnum.ORACLE_PDB,
                          ResourceSubTypeEnum.GENERAL_DB,
                          ResourceSubTypeEnum.VirtualMachine, ResourceSubTypeEnum.ImportCopy,
                          ResourceSubTypeEnum.HBaseBackupSet, ResourceSubTypeEnum.HDFSFileset,
                          ResourceSubTypeEnum.NasFileSystem, ResourceSubTypeEnum.NasShare,
                          ResourceSubTypeEnum.NdmpBackupSet,
                          ResourceSubTypeEnum.MysqlInstance, ResourceSubTypeEnum.MysqlDatabase,
                          ResourceSubTypeEnum.MysqlClusterInstance, ResourceSubTypeEnum.GaussDBT,
                          ResourceSubTypeEnum.PostgreInstance, ResourceSubTypeEnum.PostgreClusterInstance,
                          ResourceSubTypeEnum.KingBaseInstance, ResourceSubTypeEnum.KingBaseClusterInstance,
                          ResourceSubTypeEnum.HiveBackupSet, ResourceSubTypeEnum.ElasticSearchBackupSet,
                          ResourceSubTypeEnum.Redis, ResourceSubTypeEnum.ClickHouse,
                          ResourceSubTypeEnum.OpenGaussInstance, ResourceSubTypeEnum.OpenGaussDatabase,
                          ResourceSubTypeEnum.DamengSingleNode, ResourceSubTypeEnum.DamengCluster,
                          ResourceSubTypeEnum.SQLServerInstance, ResourceSubTypeEnum.SQLServerClusterInstance,
                          ResourceSubTypeEnum.SQLServerDatabase, ResourceSubTypeEnum.SQLServerAlwaysOn,
                          ResourceSubTypeEnum.DWSCluster, ResourceSubTypeEnum.DWSDateBase,
                          ResourceSubTypeEnum.DWSSchema, ResourceSubTypeEnum.DWSTable,
                          ResourceSubTypeEnum.KubernetesStatefulSet, ResourceSubTypeEnum.HCSProject,
                          ResourceSubTypeEnum.HCSTenant, ResourceSubTypeEnum.HCSCloudHost,
                          ResourceSubTypeEnum.FusionCompute, ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER,
                          ResourceSubTypeEnum.DB2Database, ResourceSubTypeEnum.HCS_GAUSSDB_PROJECT,
                          ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE, ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT,
                          ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE, ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP,
                          ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE, ResourceSubTypeEnum.GOLDENDB_CLUSTER_INSTANCE,
                          ResourceSubTypeEnum.MONGODB_CLUSTER, ResourceSubTypeEnum.MONGODB_SINGLE,
                          ResourceSubTypeEnum.MongoDB, ResourceSubTypeEnum.OCEANBASE_TENANT,
                          ResourceSubTypeEnum.OCEANBASE_CLUSTER, ResourceSubTypeEnum.GAUSSDBT_SINGLE,
                          ResourceSubTypeEnum.InformixSingleInstance, ResourceSubTypeEnum.InformixClusterInstance,
                          ResourceSubTypeEnum.TiDB, ResourceSubTypeEnum.TiDB_CLUSTER, ResourceSubTypeEnum.TiDB_DATABASE,
                          ResourceSubTypeEnum.TiDB_TABLE, ResourceSubTypeEnum.COMMON_SHARE,
                          ResourceSubTypeEnum.EXCHANGE_DATABASE, ResourceSubTypeEnum.EXCHANGE_MAILBOX,
                          ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE, ResourceSubTypeEnum.APSARA_STACK,
                          ResourceSubTypeEnum.APSARA_STACK_INSTANCE, ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET,
                          ResourceSubTypeEnum.APSARA_STACK_ZONE, ResourceSubTypeEnum.APSARA_STACK_REGION,
                          ResourceSubTypeEnum.APSARA_STACK_DISK,
                          ResourceSubTypeEnum.EXCHANGE_GROUP, ResourceSubTypeEnum.CNWARE_VM,
                          ResourceSubTypeEnum.CNWARE_HOST, ResourceSubTypeEnum.CNWARE,
                          ResourceSubTypeEnum.NUTANIX_VM, ResourceSubTypeEnum.NUTANIX_HOST,
                          ResourceSubTypeEnum.NUTANIX,
                          ResourceSubTypeEnum.SAPHANA_DATABASE,
                          ResourceSubTypeEnum.OBJECT_STORAGE, ResourceSubTypeEnum.OBJECT_SET,
                          ResourceSubTypeEnum.HYPER_V_VM, ResourceSubTypeEnum.HyperV, ResourceSubTypeEnum.HYPER_V,
                          ResourceSubTypeEnum.HYPER_V_SCVMM, ResourceSubTypeEnum.HYPER_V_HOST,
                          ResourceSubTypeEnum.HYPER_V_CLUSTER, ResourceSubTypeEnum.HYPER_V_DISK,
                          ResourceSubTypeEnum.FUSION_ONE_COMPUTE,
                          ResourceSubTypeEnum.SAP_ON_ORACLE, ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE,
                          ResourceSubTypeEnum.AntDBClusterInstance, ResourceSubTypeEnum.AntDBInstance,
                          ResourceSubTypeEnum.EXCHANGE_ONLINE, ResourceSubTypeEnum.EXCHANGE_ONLINE_BACKUP_SET]

    skip_mutual_exclusion_types = [
        ResourceSubTypeEnum.Oracle.value, ResourceSubTypeEnum.ORACLE_CLUSTER.value, ResourceSubTypeEnum.ORACLE_PDB,
        ResourceSubTypeEnum.InformixSingleInstance.value, ResourceSubTypeEnum.InformixClusterInstance.value,
        ResourceSubTypeEnum.MysqlDatabase.value, ResourceSubTypeEnum.MysqlInstance.value,
        ResourceSubTypeEnum.MysqlClusterInstance.value, ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP.value,
        ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE.value, ResourceSubTypeEnum.OCEANBASE_TENANT.value,
        ResourceSubTypeEnum.OCEANBASE_CLUSTER.value, ResourceSubTypeEnum.DamengCluster.value,
        ResourceSubTypeEnum.DamengSingleNode.value, ResourceSubTypeEnum.DB2Database.value,
        ResourceSubTypeEnum.DB2Tablespace.value, ResourceSubTypeEnum.TiDB_CLUSTER.value,
        ResourceSubTypeEnum.SQLServerInstance.value, ResourceSubTypeEnum.SQLServerDatabase.value,
        ResourceSubTypeEnum.SQLServerAlwaysOn.value, ResourceSubTypeEnum.SQLServerClusterInstance.value,
        ResourceSubTypeEnum.MONGODB_CLUSTER.value, ResourceSubTypeEnum.MONGODB_SINGLE.value,
        ResourceSubTypeEnum.GOLDENDB_CLUSTER_INSTANCE.value, ResourceSubTypeEnum.GENERAL_DB.value
    ]
