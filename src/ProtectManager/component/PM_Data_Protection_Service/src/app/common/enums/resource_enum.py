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


class ResourceTypeEnum(str, Enum):
    KubernetesCommon = "KubernetesCommon"
    Host = "Host"
    Fileset = "Fileset"
    DFSFileset = "DFSFileset"
    Database = "Database"
    Application = "Application"
    VirtualPlatform = "VirtualPlatform"
    vSphere = "vSphere"
    Cluster = "Cluster"
    Folder = "Folder"
    ResourcePool = "ResourcePool"
    CloudPlatform = "CloudPlatform"
    VM = "VM"
    BigData = "BigData"
    HyperV = "Hyper-V"
    HDFS = "HDFS"
    HBASE = "HBASE"
    DatabaseInstance = "DatabaseInstance"
    ImportCopy = "ImportCopy"
    Storage = "Storage"
    FileSystem = "FileSystem"
    Hive = "Hive"
    ElasticSearch = "ElasticSearch"
    StatefulSet = "StatefulSet"
    Namespace = "Namespace"
    Disk = "Disk"
    NetworkAdapter = "NetworkAdapter"
    Datastore = "Datastore"
    Platform = "Platform"
    CloudHost = "CloudHost"
    Project = "Project"
    TableSet = "TableSet"
    PLUGIN = "Plugin"
    OPENSTACK = "OpenStack"
    STACK_PROJECT = "StackProject"
    CLOUD_SERVER = "CloudServer"
    Agentless = "Agentless"
    CNWARE = "CNware"
    NUTANIX = "Nutanix"
    ApsaraStack = "ApsaraStack"
    NDMP = "NDMP"
    ObjectStorage = "ObjectStorage"
    Virtualization = "Virtualization"


class AuthType(str, Enum):
    WinAuth = "using_winAuth"
    SqlAuth = "using_sqlAuth"


class GeneralDbSubTypeMappingEnum(Enum):
    Gbase = ("GBase 8a", "Gbase")

    def __init__(self, general_db_sub_type: str, resource_sub_type: str):
        self.general_db_sub_type = general_db_sub_type
        self.resource_sub_type = resource_sub_type

    @classmethod
    def get_resource_sub_type(cls, general_db_sub_type: str):
        for type_mapping in GeneralDbSubTypeMappingEnum:
            if type_mapping.general_db_sub_type == general_db_sub_type:
                return type_mapping.resource_sub_type
        return None


class ResourceSubTypeEnum(str, Enum):
    DBBackupAgent = "DBBackupAgent"
    VMBackupAgent = "VMBackupAgent"
    ABBackupClient = "ABBackupClient"
    DWSBackupAgent = "DWSBackupAgent"
    UBackupAgent = "UBackupAgent"
    PROTECT_AGENT = "ProtectAgent"
    Fileset = "Fileset"
    Volume = "Volume"
    HDFSFileset = "HDFSFileset"
    Hive = "Hive"
    ElasticSearch = "ElasticSearch"
    DFSFileset = "DFSFileset"
    Oracle = "Oracle"
    GaussDBT = "GaussDBT"
    OpenGauss = "OpenGauss"
    OpenGaussInstance = "OpenGauss-instance"
    OpenGaussDatabase = "OpenGauss-database"
    OracleApp = "OracleApp"
    SQLServer = "SQLServer"
    SQLServerInstance = "SQLServer-instance"
    SQLServerDatabase = "SQLServer-database"
    SQLServerCluster = "SQLServer-cluster"
    SQLServerAlwaysOn = "SQLServer-alwaysOn"
    SQLServerClusterInstance = "SQLServer-clusterInstance"
    DB2 = "DB2"
    MySQL = "MySQL"
    GaussDB = "GaussDB"
    SAP_HANA = "SAP HANA"
    SAPHANA_INSTANCE = "SAPHANA-instance"
    SAPHANA_DATABASE = "SAPHANA-database"
    KingBase = "KingBase"
    KingBaseInstance = "KingBaseInstance"
    KingBaseClusterInstance = "KingBaseClusterInstance"
    KingBaseCluster = "KingBaseCluster"
    Sybase_IQ = "Sybase IQ"
    InformixService = "Informix-service"
    InformixSingleInstance = "Informix-singleInstance"
    InformixClusterInstance = "Informix-clusterInstance"
    TimesTen = "TimesTen"
    Gbase = "GBase"
    Dameng = "Dameng"
    Cassandra = "Cassandra"
    OscarDB = "OscarDB"
    EXCHANGE_GROUP = "Exchange-group"
    EXCHANGE_DATABASE = "Exchange-database"
    EXCHANGE_MAILBOX = "Exchange-mailbox"
    EXCHANGE_SINGLE_NODE = "Exchange-single-node"
    EXCHANGE = "Exchange"
    vCenter = "VMware vCenter Server"
    VMware = "VMware"
    ESX = "VMware ESX"
    ESXi = "VMware ESXi"
    HyperV = "Hyper-V"
    HDFS = "HDFS"
    FusionSphere = "FusionSphere"
    ClusterComputeResource = "vim.ClusterComputeResource"
    HostSystem = "vim.HostSystem"
    Folder = "vim.Folder"
    ResourcePool = "vim.ResourcePool"
    OpenStack = "OpenStack"
    HuaweiCloudStack = "HuaweiCloudStack"
    VirtualMachine = "vim.VirtualMachine"
    MicroSoftVirtualMachine = "ms.VirtualMachine"
    Hadoop = "Hadoop"
    FusionInsight = "FusionInsight"
    # 用于VMware接入资源屏蔽
    VirtualApp = "vim.VirtualApp"
    Datacenter = "vim.Datacenter"
    # 只针对SLA使用
    Common = "Common"
    Replica = "Replica"
    ImportCopy = "ImportCopy"
    NasFileSystem = "NasFileSystem"
    NasShare = "NasShare"
    NdmpBackupSet = "NDMP-BackupSet"
    HBaseBackupSet = "HBaseBackupSet"
    MysqlInstance = "MySQL-instance"
    MysqlDatabase = "MySQL-database"
    MysqlCluster = "MySQL-cluster"
    MysqlClusterInstance = "MySQL-clusterInstance"
    CloudBackupFileSystem = "CloudBackupFileSystem"
    HiveBackupSet = "HiveBackupSet"
    ElasticSearchBackupSet = "ElasticSearchBackupSet"
    Kubernetes = "Kubernetes"
    KubernetesNamespace = "KubernetesNamespace"
    KubernetesStatefulSet = "KubernetesStatefulSet"
    DWSCluster = "DWS-cluster"
    DWSDateBase = "DWS-database"
    DWSSchema = "DWS-schema"
    DWSTable = "DWS-table"
    Redis = "Redis"
    ClickHouse = "ClickHouse"
    PostgreInstance = "PostgreInstance"
    PostgreClusterInstance = "PostgreClusterInstance"
    AntDB = "AntDB"
    AntDBInstance = "AntDBInstance"
    AntDBClusterInstance = "AntDBClusterInstance"
    PostgreSQL = "PostgreSQL"
    PostgreCluster = "PostgreCluster"
    DamengSingleNode = "Dameng-singleNode"
    DamengCluster = "Dameng-cluster"
    FusionCompute = "FusionCompute"
    HCSTenant = "HCSTenant"
    HCSProject = "HCSProject"
    HCSCloudHost = "HCSCloudHost"
    OCEAN_STOR_V6 = "OceanStorV6"
    DORADO_V6 = "DoradoV6"
    OCEAN_STOR_V5 = "OceanStorV5"
    OCEAN_STOR_PACIFIC = "OceanStorPacific"
    NET_APP = "NetApp"
    STORAGE_OTHERS = "StorageOthers"
    CLOUD_BACKUP = "OceanStorDoradoV6"
    ORACLE_CLUSTER = "Oracle-cluster"
    ORACLE_CLUSTER_INSTANCE = "Oracle-clusterInstance"
    ORACLE_CLUSTER_ENV = "Oracle-clusterEnv"
    ORACLE_PDB = "Oracle-pdb"
    OPENSTACK_CONTAINER = "OpenStackContainer"
    OPENSTACK_PROJECT = "OpenStackProject"
    OPENSTACK_CLOUD_SERVER = "OpenStackCloudServer"
    DB2Cluster = "DB2-cluster"
    DB2Instance = "DB2-instance"
    DB2ClusterInstance = "DB2-clusterInstance"
    DB2Database = "DB2-database"
    DB2Tablespace = "DB2-tablespace"
    GENERAL_DB = "GeneralDb"
    GOLDENDB_CLUSTER_INSTANCE = "GoldenDB-clusterInstance"
    GOLDENDB_CLUSTER = "GoldenDB-cluster"
    GOLDENDB = "GoldenDB"
    TDSQL_CLUSTER_INSTANCE = "TDSQL-clusterInstance"
    TDSQL_CLUSTER = "TDSQL-cluster"
    TDSQL_CLUSTER_GROUP = "TDSQL-clusterGroup"
    TDSQL = "TDSQL"
    HCS_GAUSSDB_PROJECT = "HCSGaussDBProject"
    HCS_GAUSSDB_INSTANCE = "HCSGaussDBInstance"
    MONGODB_SINGLE = "MongoDB-single"
    MONGODB_CLUSTER = "MongoDB-cluster"
    MongoDB = "MongoDB"
    TPOPS_GAUSSDB_PROJECT = "TPOPSGaussDBProject"
    TPOPS_GAUSSDB_INSTANCE = "TPOPSGaussDBInstance"
    S3_STORAGE = "S3.storage"
    HCS_CONTAINER = "HCSContainer"
    FUSION_COMPUTE_HOST = "FusionComputeHost"
    FUSION_COMPUTE_CLUSTER = "FusionComputeCluster"
    CYBERENGINE_DORADO_V6 = "CyberEngineDoradoV6"
    CYBERENGINE_OCEAN_PROTECT = "CyberEngineOceanProtect"
    OPENSTACK_DOMAIN = "OpenStackDomain"
    OCEANBASE_TENANT = "OceanBase-tenant"
    OCEANBASE_CLUSTER = "OceanBase-cluster"
    GAUSSDBT_SINGLE = "GaussDBT-single"
    TiDB_CLUSTER = "TiDB-cluster"
    TiDB_DATABASE = "TiDB-database"
    TiDB_TABLE = "TiDB-table"
    TiDB = "TiDB"
    CNWARE = "CNware"
    CNWARE_CLUSTER = "CNwareCluster"
    CNWARE_HOST = "CNwareHost"
    CNWARE_HOST_POOL = "CNwareHostPool"
    CNWARE_VM = "CNwareVm"
    CNWARE_DISK = "CNwareDisk"
    APSARA_STACK = "ApsaraStack"
    APSARA_STACK_ORGANIZATION = "APS-organization"
    APSARA_STACK_REGION = "APS-region"
    APSARA_STACK_ZONE = "APS-zone"
    APSARA_STACK_RESOURCE_SET = "APS-resourceSet"
    APSARA_STACK_INSTANCE = "APS-instance"
    APSARA_STACK_DISK = "APS-disk"
    KUBERNETES_CLUSTER_COMMON = "KubernetesClusterCommon"
    KUBERNETES_NAMESPACE_COMMON = "KubernetesNamespaceCommon"
    KUBERNETES_DATASET_COMMON = "KubernetesDatasetCommon"
    HCS_ENV_OP = "HcsEnvOp"
    HYPER_V_CLUSTER = "HyperV.Cluster"
    HYPER_V_HOST = "HyperV.Host"
    HYPER_V_VM = "HyperV.VM"
    HYPER_V_DISK = "HyperV.Disk"
    HYPER_V_SCVMM = "HyperV.SCVMM"
    HYPER_V = "HyperV"
    LUN = "LUN"
    FUSION_ONE_COMPUTE = "FusionOneCompute"
    NUTANIX = "Nutanix"
    NUTANIX_CLUSTER = "NutanixCluster"
    NUTANIX_HOST = "NutanixHost"
    NUTANIX_VM = "NutanixVm"
    NUTANIX_DISK = "NutanixDisk"

    """
    SanClient代理主机类型的子类型
    """
    S_BACKUP_AGENT = "SBackupAgent"
    COMMON_SHARE = "CommonShare"
    OBJECT_SET = "ObjectSet"
    OBJECT_STORAGE = "ObjectStorage"
    AD = "ADDS"
    COPY = "COPY"

    SAP_ON_ORACLE = "SAP_ON_ORACLE"
    SAP_ON_ORACLE_SINGLE = "SAP_ON_ORACLE_SINGLE"
    EXCHANGE_ONLINE = "ExchangeOnline"
    EXCHANGE_ONLINE_BACKUP_SET = "ExchangeOnlineBackupSet"


class ResourceSubTypeWithOrderEnum(Enum):
    DBBACKUP_AGENT = ("DBBackupAgent", 1)
    VMBACKUP_AGENT = ("VMBackupAgent", 2)
    ABBACKUP_CLIENT = ("ABBackupClient", 3)
    UBACKUP_AGENT = ("UBackupAgent", 4)
    DWSBACKUP_AGENT = ("DWSBackupAgent", 5)
    PROTECT_AGENT = ("ProtectAgent", 6)
    FILESET = ("Fileset", 7)
    DFS_FILESET = ("DFSFileset", 8)
    ORACLE = ("Oracle", 9)
    SQLSERVER = ("SQLServer", 10)
    SQLSERVER_INSTANCE = ("SQLServer-instance", 11)
    SQLSERVER_DATABASE = ("SQLServer-database", 12)
    SQLSERVER_CLUSTER = ("SQLServer-cluster", 13)
    SQLSERVER_ALWAYSON = ("SQLServer-alwaysOn", 14)
    SQLSERVER_CLUSTER_INSTANCE = ("SQLServer-clusterInstance", 15)
    DB2 = ("DB2", 16)
    MYSQL = ("MySQL", 17)
    MYSQL_INSTANCE = ("MySQL-instance", 18)
    MYSQL_CLUSTER = ("MySQL-cluster", 19)
    MYSQL_CLUSTER_INSTANCE = ("MySQL-clusterInstance", 20)
    MYSQL_DATABASE = ("MySQL-database", 21)
    REDIS = ("Redis", 22)
    CLICKHOUSE = ("ClickHouse", 23)
    GAUSSDB = ("GaussDB", 24)
    GAUSSDBT = ("GaussDBT", 25)
    DWS_CLUSTER = ("DWS-cluster", 26)
    DWS_DATEBASE = ("DWS-database", 27)
    DWS_SCHEMA = ("DWS-schema", 28)
    DWS_TABLE = ("DWS-table", 29)
    OPENGAUSS = ("OpenGauss", 30)
    OPENGAUSS_INSTANCE = ("OpenGauss-instance", 31)
    OPENGAUSS_DATABASE = ("OpenGauss-database", 32)
    SAP_HANA = ("SAP HANA", 33)
    KINGBASE = ("KingBase", 34)
    KINGBASE_CLUSTER = ("KingBaseCluster", 35)
    KINGBASE_INSTANCE = ("KingBaseInstance", 36)
    KINGBASE_CLUSTER_INSTANCE = ("KingBaseClusterInstance", 37)
    SYBASE_IQ = ("Sybase IQ", 38)
    INFORMIX = ("Informix", 39)
    TIMESTEN = ("TimesTen", 40)
    GBASE = ("Gbase", 41)
    DAMENG = ("Dameng", 42)
    DAMENG_SINGLENODE = ("Dameng-singleNode", 43)
    DAMENG_CLUSTER = ("Dameng-cluster", 44)
    CASSANDRA = ("Cassandra", 45)
    OSCARDB = ("OscarDB", 46)
    EXCHANGE = ("Exchange", 47)
    VCENTER = ("VMware vCenter Server", 48)
    ESX = ("VMware ESX", 49)
    ESXI = ("VMware ESXi", 50)
    HYPERV = ("Hyper-V", 51)
    MICROSOFT_VIRTUAL_MACHINE = ("ms.VirtualMachine", 52)
    HYPER_VHOST = ("ms.HostSystem", 53)
    FUSION_SPHERE = ("FusionSphere", 54)
    CLUSTER_COMPUTE_RESOURCE = ("vim.ClusterComputeResource", 55)
    HOST_SYSTEM = ("vim.HostSystem", 56)
    FOLDER = ("vim.Folder", 57)
    RESOURCE_POOL = ("vim.ResourcePool", 58)
    OPEN_STACK = ("OpenStack", 59)
    VIRTUAL_MACHINE = ("vim.VirtualMachine", 60)
    HADOOP = ("Hadoop", 61)
    VIRTUAL_APP = ("vim.VirtualApp", 62)
    FUSION_INSIGHT = ("FusionInsight", 63)
    HDFS = ("HDFS", 64)
    HDFS_FILESET = ("HDFSFileset", 65)
    HBASE = ("HBase", 66)
    HBASE_BACKUPSET = ("HBaseBackupSet", 67)
    HIVE = ("Hive", 68)
    HIVE_BACKUPSET = ("HiveBackupSet", 69)
    ELASTIC_SEARCH = ("ElasticSearch", 70)
    ELASTICSEARCH_BACKUPSET = ("ElasticSearchBackupSet", 71)
    POSTGRE_CLUSTER = ("PostgreCluster", 72)
    POSTGRE_INSTANCE = ("PostgreInstance", 73)
    POSTGRE_SQL = ("PostgreSQL", 74)
    POSTGRE_CLUSTER_INSTANCE = ("PostgreClusterInstance", 75)
    IMPORTCOPY = ("ImportCopy", 76)
    REPLICA = ("Replica", 77)
    NAS_SHARE = ("NasShare", 78)
    NAS_FILESYSTEM = ("NasFileSystem", 79)
    CLOUDBACKUP_FILESYSTEM = ("CloudBackupFileSystem", 80)
    S3_STORAGE = ("S3.storage", 81)
    KUBERNETES = ("Kubernetes", 82)
    KUBERNETES_NAMESPACE = ("KubernetesNamespace", 83)
    KUBERNETES_STATEFULSET = ("KubernetesStatefulSet", 84)
    FUSION_COMPUTE = ("FusionCompute", 85)
    COMMON = ("Common", 86)
    HUAWEI_CLOUD_STACK = ("HuaweiCloudStack", 87)
    HCS_CONTAINER = ("HCSContainer", 88)
    HCS_TENANT = ("HCSTenant", 89)
    HCS_REGION = ("HCSRegion", 90)
    HCS_PROJECT = ("HCSProject", 91)
    HCS_CLOUDHOST = ("HCSCloudHost", 92)
    OCEAN_STOR_V6 = ("OceanStorV6", 93)
    DORADO_V6 = ("DoradoV6", 94)
    OCEAN_STOR_V5 = ("OceanStorV5", 95)
    OCEAN_STOR_PACIFIC = ("OceanStorPacific", 96)
    NET_APP = ("NetApp", 97)
    STORAGE_OTHERS = ("StorageOthers", 98)
    CLOUD_BACKUP = ("OceanStorDoradoV6", 99)
    FUSION_COMPUTE_HOST = ("FusionComputeHost", 100)
    FUSION_COMPUTE_CLUSTER = ("FusionComputeCluster", 101)
    DB2_TABLESPACE = ("DB2-tablespace", 102)
    GENERAL_DB = ("GeneralDb", 103)
    OPENSTACK_CONTAINER = ("OpenStackContainer", 104)
    OPENSTACK_PROJECT = ("OpenStackProject", 105)
    OPENSTACK_CLOUD_SERVER = ("OpenStackCloudServer", 106)
    CYBERENGINE_DORADO_V6 = ("CyberEngineDoradoV6", 107)
    CYBERENGINE_OCEAN_PROTECT = ("CyberEngineOceanProtect", 108)
    GOLDENDB_CLUSETER_INSTANCE = ("GoldenDB-clusterInstance", 109)
    GOLDENDB_CLUSTER = ("GoldenDB-cluster", 110)
    GOLDENDB = ("GoldenDB", 111)
    DB2_CLUSTER = ("DB2-cluster", 112)
    DB2_INSTANCE = ("DB2-instance", 113)
    DB2_CLUSTER_INSTANCE = ("DB2-clusterInstance", 114)
    DB2_DATABASE = ("DB2-database", 115)
    ORACLE_CLUSTER = ("Oracle-cluster", 116)
    ORACLE_CLUSTER_INSTANCE = ("Oracle-clusterInstance", 117)
    ORACLE_CLUSTER_ENV = ("Oracle-clusterEnv", 118)
    OPENSTACK_DOMAIN = ("OpenStackDomain", 119)
    HCS_GAUSSDB_PROJECT = ("HCSGaussDBProject", 120)
    HCS_GAUSSDB_INSTANCE = ("HCSGaussDBInstance", 121)
    MONGODB = ("MongoDB", 122)
    MONGODB_SINGLE = ("MongoDB-single", 123)
    MONGODB_CLUSTER = ("MongoDB-cluster", 124)
    TPOPS_GAUSSDB_PROJECT = ("TPOPSGaussDBProject", 125)
    TPOPS_GAUSSDB_INSTANCE = ("TPOPSGaussDBInstance", 126)
    EXCHANGE_GROUP = ("Exchange-group", 127)
    EXCHANGE_DATABASE = ("Exchange-database", 128)
    NDMP = ("NDMP", 129)
    NDMP_BACKUPSET = ("NDMP-BackupSet", 130)
    NDMP_SERVER = ("NDMP-server", 131)
    SAPHANA_INSTANCE = ("SAPHANA-instance", 132)
    SAPHANA_DATABASE = ("SAPHANA-database", 133)
    REDHAT_VIRTUALIZATION = ("RedHatVirtualization", 134)
    REDHAT_DATACENTER = ("RHVDatacenter", 135)
    REDHAT_CLUSTER = ("RHVCluster", 136)
    REDHAT_HOST = ("RHVHost", 137)
    REDHAT_VM = ("RHVVM", 138)
    REDHAT_DISK = ("RHVDisk", 139)
    BACKUP_MEMBER_CLUSTER = ("BackMemberCluster", 140)
    INFORMIX_SINGLE_INSTANCE = ("Informix-singleInstance", 141)
    INFORMIX_CLUSTER_INSTANCE = ("Informix-clusterInstance", 142)
    INFORMIX_SERVICE = ("Informix-service", 143)
    HYPER_V_CLUSTER = ("HyperV.Cluster", 144)
    HYPER_V_HOST = ("HyperV.Host", 145)
    HYPER_V_VM = ("HyperV.VM", 146)
    HYPER_V_DISK = ("HyperV.Disk", 147)
    HYPER_V_SCVMM = ("HyperV.SCVMM", 148)
    S_BACKUP_AGENT = ("SBackupAgent", 149)
    TDSQL_CLUSTERINSTANCE = ("TDSQL-clusterInstance", 150)
    TDSQL_CLUSTER = ("TDSQL-cluster", 151)
    OCEAN_BASE_CLUSTER = ("OceanBase-cluster", 152)
    OCEAN_BASE_TENANT = ("OceanBase-tenant", 153)
    TIDB_CLUSTERINSTANCE = ("TiDB-clusterInstance", 154)
    TIDB_CLUSTER = ("TiDB-cluster", 155)
    TIDB_TABLE = ("TiDB-table", 156)
    TIDB_DATABASE = ("TiDB-database", 157)
    TIDB = ("TiDB", 158)
    GAUSSDBT_SINGLE = ("GaussDBT-single", 159)
    KUBERNETES_CLUSTER_COMMON = ("KubernetesClusterCommon", 160)
    KUBERNETES_NAMESPACE_COMMON = ("KubernetesNamespaceCommon", 161)
    KUBERNETES_DATASET_COMMON = ("KubernetesDatasetCommon", 162)
    HCS_ENV_OP = ("HcsEnvOp", 163)
    GAUSSDB_DWS_PROJECT = ("DWS-project", 164)
    TDSQL_CLUSTERGROUP = ("TDSQL-clusterGroup", 165)

    Volume = ("Volume", 166)
    COMMON_SHARE = ("CommonShare", 168)
    OBJECT_SET = ("ObjectSet", 169)
    AD = ("ADDS", 170)

    CNWARE = ("CNware", 174)
    CNWARE_CLUSTER = ("CNwareCluster", 175)
    CNWARE_HOST = ("CNwareHost", 176)
    CNWARE_HOST_POOL = ("CNwareHostPool", 177)
    CNWARE_VM = ("CNwareVm", 178)
    CNWARE_DISK = ("CNwareDisk", 179)
    CNWARE_STORAGE_POOL = ("CNwareStoragePool", 180)
    APSARA_STACK = ("ApsaraStack", 181)
    APSARA_STACK_ORGANIZATION = ("APS-organization", 182)
    APSARA_STACK_RESOURCE_SET = ("APS-resourceSet", 183)
    APSARA_STACK_REGION = ("APS-region", 184)
    APSARA_STACK_ZONE = ("APS-zone", 185)
    APSARA_STACK_INSTANCE = ("APS-instance", 186)
    APSARA_STACK_DISK = ("APS-disk", 187)
    EXCHANGE_SINGLE_NODE = ("Exchange-single-node", 188)
    EXCHANGE_MAILBOX = ("Exchange-mailbox", 189)
    EXERCISE_DEFAULT_DATABASE = ("ExerciseDefaultDatabase", 190)
    HYPER_V = ("HyperV", 191)
    LUN = ("LUN", 192)
    FUSION_ONE_COMPUTE = ("FusionOneCompute", 193)
    SAP_ON_ORACLE = ("SAP_ON_ORACLE", 200)
    SAP_ON_ORACLE_SINGLE = ("SAP_ON_ORACLE_SINGLE", 201)
    ANTDB = ("AntDB", 202)
    ANTDB_INSTANCE = ("AntDBInstance", 203)
    ANTDB_CLUSTER_INSTANCE = ("AntDBClusterInstance", 204)

    NUTANIX = ("Nutanix", 202)
    NUTANIX_CLUSTER = ("NutanixCluster", 203)
    NUTANIX_HOST = ("NutanixHost", 204)
    NUTANIX_VM = ("NutanixVm", 205)
    NUTANIX_DISK = ("NutanixDisk", 206)
    EXCHANGE_ONLINE = ("ExchangeOnline", 209)
    EXCHANGE_ONLINE_BACKUP_SET = ("ExchangeOnlineBackupSet", 210)

    def __init__(self, resource_sub_type: str, order: int):
        self.resource_sub_type = resource_sub_type
        self.order = order

    @classmethod
    def get_order(cls, resource_sub_type: str):
        default_order = -1
        for resource_sub_type_with_order in ResourceSubTypeWithOrderEnum:
            if resource_sub_type_with_order.resource_sub_type == resource_sub_type:
                return resource_sub_type_with_order.order
        return default_order


class ProtectionStatusEnum(int, Enum):
    # 未保护
    unprotected = 0
    # 已保护
    protected = 1
    # 保护中
    protecting = 2


class LinkStatusEnum(int, Enum):
    # 在线/可连通
    Online = 1
    # 离线/不可连通
    Offline = 0


class DeployTypeEnum(str, Enum):
    A8000 = "a8000"
    X8000 = "d0"
    X6000 = "d1"
    X3000 = "d2"
    CloudBackupOld = "cloudbackup"
    CloudBackup = "d3"
    HyperDetect = "d4"
    CYBER_ENGINE = "d5"
    X9000 = "d6"
    PACIFIC = "d7"
    DEPENDENT = "d8"


class OcDeviceTypeEnum(str, Enum):
    OceanProtect = "CyberEngineOceanProtect"
    Dorado = "CyberEngineDoradoV6"
    Pacific = "CyberEnginePacific"
