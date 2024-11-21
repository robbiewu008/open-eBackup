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
// 虚拟化资源详情展示的类型
export enum VM_COPY_TYPE {
  DISKS = 1, // 磁盘
  FILES, // 文件
  VOLUMES
}

// 副本支持的操作
export enum COPY_OPERATION {
  RESTORE = 'restore',
  INSTANT_RESTORE = 'instant-restore',
  MOUNT = 'mount',
  DELETE = 'delete',
  MODIFY_RETENTION = 'modify'
}

// vmware恢复类型
export enum VmRestoreOptionType {
  VM = 0,
  FILE = 'file',
  DISK = 1
}

export enum DatastoreType {
  SAME = 'same',
  DIFFERENT = 'different'
}

// 快照保护类型
export enum ProtectedObjType {
  FILESET = 'fileset',
  HOST = 'host',
  VM = 'vm',
  DATABASE = 'db'
}

export enum OracleParentType {
  HOST = 'host',
  CLUSTER = 'cluster'
}

export enum DATE_PICKER_MODE {
  MONTH = 'month',
  DATE = 'date',
  SIMPLE = 'simple'
}

export enum SLA_TYPE {
  ALL = 'all',
  UNPROTECTED = 'unprotected',
  PROTECTED = 'protected'
}

export enum RestoreLocationType {
  ORIGIN = 'O',
  NEW = 'N',
  CLOUD = 'cloud',
  NATIVE = 'native',
  SPECIFIED_FILE_SYSTEM = 'specified'
}

// 周期备份策略模式
export enum PeriodBackupMode {
  CDP = 'cdp',
  FULL_INCREMENTAL = 'full_incremental'
}

// 策略类型
export enum PolicyType {
  BACKUP = 'backup',
  ARCHIVING = 'archiving',
  REPLICATION = 'replication'
}

// 策略动作
export enum PolicyAction {
  FULL = 'full', // 全量备份
  LOG = 'log', // 日志备份
  SNAPSHOT = 'snapshot', // 生产快照
  INCREMENT = 'difference_increment', // 增量备份
  DIFFERENCE = 'cumulative_increment', // 差异备份
  PERMANENT = 'permanent_increment' // 永久增量
}

// 复制模式
export enum ReplicationModeType {
  INTRA_DOMAIN = 2,
  CROSS_DOMAIN = 1,
  CROSS_CLOUD = 3
}

// 调度触发类型
export enum ScheduleTrigger {
  PERIOD_EXECUTE = 1, // 周期执行
  BACKUP_EXECUTE, // 备份完立即执行
  SPECIFIED_TIME = 4 // 指定时间
}

// 保留类型
export enum RetentionType {
  PERMANENTLY_RETAINED = 1, // 永久保留
  TEMPORARY_RESERVATION, // 临时保留
  SPECIFIED_DATE // 指定日期副本
}

// 资源大类
export enum ResourceType {
  VM = 'VM',
  FUSION_COMPUTE = 'FusionCompute',
  FUSION_ONE = 'FusionOneCompute',
  VSPHERE = 'vSphere',
  CNWARE = 'CNware',
  HYPERV = 'Hyper-V',
  FUSIONSPHERE = 'FusionSphere',
  DATACENTER = 'Datacenter',
  CLUSTER = 'Cluster',
  NUTANIX = 'Nutanix',
  DATABASE = 'Database',
  HOST = 'Host',
  FILESET = 'Fileset',
  BIG_DATA = 'BigData',
  PLATFORM = 'Platform',
  CLOUD_PLATFORM = 'CloudPlatform',
  Storage = 'Storage',
  Vessel = 'Vessel',
  TENANT = 'Tenant',
  PROJECT = 'Project',
  CLOUD_HOST = 'CloudHost',
  HCS_CONTAINER = 'HCSContainer',
  OPENSTACK_CONTAINER = 'OpenStackContainer',
  HCS = 'HCS',
  Region = 'Region',
  NODE = 'Node',
  TABLE = 'Table',
  TABLE_SET = 'TableSet',
  CNA = 'CNA',
  Container = 'Container',
  OpenStack = 'OpenStack',
  StackProject = 'StackProject',
  OpenStackProject = 'OpenStackProject',
  OpenStackCloudServer = 'OpenStackCloudServer',
  CloudServer = 'CloudServer',
  StackDomain = 'StackDomain',
  OpenStackDomain = 'OpenStackDomain',
  Agentless = 'Agentless',
  ApsaraStack = 'ApsaraStack',
  VM_GROUP = 'vmGroup',
  CLOUD_GROUP = 'CloudGroup',
  HCSProject = 'HCSProject',
  Virtualization = 'Virtualization',
  VirtualPlatform = 'VirtualPlatform'
}

// 保护资源类型以及批量保护还是单个保护
export enum ProtectResourceCategory {
  host,
  hosts,
  fileset,
  filesets,
  oracle,
  oracles,
  mysql,
  mysqls,
  db2,
  db2s,
  SQLServer,
  SQLServers,
  vmware,
  vmwares,
  esix,
  cluster,
  Replica,
  GaussDB,
  GaussDBs,
  ImportCopy,
  NASFileSystem,
  NASShare,
  Kubernetes,
  HDFS,
  HBase,
  GaussDBDWS,
  OpenGauss,
  Dameng,
  ClickHouse,
  ClickHouseDatabase,
  ClickHouseTableset,
  GaussdbForOpengauss,
  LightCloudGaussDB,
  GeneralDB,
  Exchange
}

export enum SlaType {
  Backup = 1,
  DisasterRecovery
}

export enum ApplicationTypeView {
  General,
  Specified
}

// 用于资源集的类型判断
export enum ResourceSetType {
  VMware = 'VMware',
  CNware = 'CNware',
  Nutanix = 'Nutanix',
  FusionCompute = 'FusionCompute',
  FusionOne = 'FusionOneCompute',
  HyperV = 'HyperV',
  Kubernetes_CSI = 'Kubernetes_CSI',
  Kubernetes_FlexVolume = 'Kubernetes_FlexVolume',
  OpenStack = 'OpenStack',
  HCSStack = 'HCSStack',
  HCSStack_GaussDB = 'HCSGaussDB',
  ApsaraStack = 'ApsaraStack',
  ADDS = 'ADDS',
  Exchange = 'Exchange',
  SAP_HANA = 'SAP_HANA',
  SAP_ON_ORACLE = 'SAP_ON_ORACLE',
  StorageEquipment = 'StorageEquipment',
  NasShare = 'NasShare',
  NasFileSystem = 'NasFileSystem',
  ObjectStorage = 'ObjectStorage',
  CommonShare = 'CommonShare',
  Volume = 'Volume',
  Fileset = 'Fileset',
  Ndmp = 'NDMP',
  FilesetTemplate = 'FILE_SET_TEMPLATE',
  AntDB = 'AntDB',
  DB2 = 'DB2',
  Dameng = 'Dameng',
  GaussDB = 'GaussDB',
  GaussDB_T = 'GaussDBT',
  GoldenDB = 'GoldenDB',
  Informix = 'Informix',
  KingBase = 'KingBase',
  MySQL = 'MySQL',
  OceanBase = 'OceanBase',
  Oracle = 'Oracle',
  PostgreSQL = 'PostgreSQL',
  SQLServer = 'SQLServer',
  TDSQL = 'TDSQL',
  TiDB = 'TiDB',
  OpenGauss = 'OpenGauss',
  GeneralDb = 'GeneralDb',
  GaussDB_DWS = 'GaussDB_DWS',
  ClickHouse = 'ClickHouse',
  Elasticsearch = 'ElasticSearch',
  HBase = 'HBase',
  HDFS = 'HDFS',
  Hive = 'Hive',
  MongoDB = 'MongoDB',
  Redis = 'Redis',
  Agent = 'AGENT',
  RESOURCE_GROUP = 'RESOURCE_GROUP',
  SLA = 'SLA',
  QOS = 'QOS',
  type = 'type',
  AirGap = 'AIR_GAP',
  LiveMount = 'LIVE_MOUNT_POLICY',
  Worm = 'PREVENT_EXTORTION_AND_WORM',
  Report = 'REPORT',
  ReportSubscription = 'SCHEDULE_REPORT'
}

export enum ApplicationType {
  Common = 'Common',
  AntDB = 'AntDB',
  Oracle = 'Oracle',
  Vmware = 'vim.VirtualMachine',
  CNware = 'CNwareVm',
  Nutanix = 'NutanixVm',
  FusionCompute = 'FusionCompute',
  FusionOne = 'FusionOneCompute',
  Fileset = 'Fileset',
  HDFS = 'HDFSFileset',
  HBase = 'HBaseBackupSet',
  Hive = 'HiveBackupSet',
  Elasticsearch = 'ElasticSearchBackupSet',
  Exchange = 'Exchange-database',
  OpenStack = 'OpenStackCloudServer',
  Volume = 'Volume',
  SQLServer = 'SQLServer',
  MySQL = 'MySQL',
  PostgreSQL = 'PostgreSQL',
  KingBase = 'KingBase',
  OpenGauss = 'OpenGauss',
  GBase = 'GBase',
  DB2 = 'DB2',
  Dameng = 'Dameng',
  GaussDB = 'GaussDB',
  GaussDBT = 'GaussDBT',
  GaussDBDWS = 'DWS-cluster',
  Redis = 'Redis',
  SapHana = 'SAPHANA-database',
  Saponoracle = 'SAP_ON_ORACLE',
  MongoDB = 'MongoDB',
  HyperV = 'HyperV.VM',
  FusionSphere = 'FusionSphere',
  Replica = 'Replica',
  H3cCas = 'H3cCas',
  NasEquipment = 'NasEquipment',
  HCSCloudHost = 'HCSCloudHost',
  NASFileSystem = 'NasFileSystem',
  NASShare = 'NasShare',
  Ndmp = 'NDMP-BackupSet',
  KubernetesCommon = 'K8S-Common-dataset',
  KubernetesMySQL = 'K8S-MySQL-dataset',
  RuleManagement = 'RuleManagement',
  ImportCopy = 'ImportCopy',
  LocalFileSystem = 'CloudBackupFileSystem',
  LocalLun = 'LUN',
  HCS_CONTAINER = 'HCSContainer',
  ClickHouse = 'ClickHouse',
  KubernetesStatefulSet = 'KubernetesStatefulSet',
  KubernetesDatasetCommon = 'KubernetesClusterCommon',
  GaussDBForOpenGauss = 'HCSGaussDBInstance',
  GoldenDB = 'GoldenDB',
  GeneralDatabase = 'GeneralDb',
  Informix = 'Informix-service',
  LightCloudGaussDB = 'TPOPSGaussDBInstance',
  TDSQL = 'TDSQL-clusterInstance',
  OceanBase = 'OceanBase-cluster',
  TiDB = 'TiDB',
  ObjectStorage = 'ObjectSet',
  CommonShare = 'CommonShare',
  ActiveDirectory = 'ADDS',
  ApsaraStack = 'ApsaraStack'
}

export const allAppType = {
  // 用于资源集
  singleLayerApp: [
    ApplicationType.GaussDBT,
    ApplicationType.Dameng,
    ApplicationType.GeneralDatabase,
    ApplicationType.MongoDB,
    ApplicationType.Redis,
    ApplicationType.ActiveDirectory,
    ApplicationType.NASFileSystem,
    ApplicationType.NASShare,
    ApplicationType.CommonShare,
    ApplicationType.Volume
  ],
  virtualCloudApp: [
    ApplicationType.Vmware,
    ApplicationType.CNware,
    ApplicationType.Nutanix,
    ApplicationType.FusionCompute,
    ApplicationType.FusionOne,
    ApplicationType.HyperV,
    ApplicationType.HCSCloudHost,
    ApplicationType.OpenStack,
    ApplicationType.ApsaraStack
  ]
};

export const APP_HOST_ICONS = [
  {
    id: ApplicationType.Fileset,
    label: 'common_fileset_label'
  },
  {
    id: ApplicationType.AntDB,
    label: 'AntDB'
  },
  {
    id: ApplicationType.ActiveDirectory,
    label: 'Active Directory'
  },
  {
    id: ApplicationType.Volume,
    label: 'protection_volume_label'
  },
  {
    id: ApplicationType.Oracle,
    label: 'Oracle'
  },
  {
    id: ApplicationType.DB2,
    label: 'protection_db_two_label'
  },
  {
    id: ApplicationType.SQLServer,
    label: 'SQL Server'
  },
  {
    id: ApplicationType.MySQL,
    label: 'MySQL/MariaDB/GreatSQL'
  },
  {
    id: ApplicationType.PostgreSQL,
    label: 'PostgreSQL'
  },
  {
    id: ApplicationType.OpenGauss,
    label: 'common_opengauss_label'
  },
  {
    id: ApplicationType.GaussDBT,
    label: 'GaussDB T'
  },
  {
    id: ApplicationType.GaussDBDWS,
    label: 'GaussDB(DWS)'
  },
  {
    id: ApplicationType.Redis,
    label: 'Redis'
  },
  {
    id: ApplicationType.Dameng,
    label: 'Dameng'
  },
  {
    id: ApplicationType.KingBase,
    label: 'Kingbase'
  },
  {
    id: ApplicationType.OceanBase,
    label: 'OceanBase'
  },
  {
    id: ApplicationType.ClickHouse,
    label: 'ClickHouse'
  },
  {
    id: ApplicationType.MongoDB,
    label: 'MongoDB'
  },
  {
    id: ApplicationType.Exchange,
    label: 'Exchange'
  },
  {
    id: ApplicationType.GoldenDB,
    label: 'protection_goldendb_label'
  },
  {
    id: ApplicationType.Informix,
    label: 'Informix/GBase 8s'
  },
  {
    id: ApplicationType.TDSQL,
    label: 'TDSQL'
  },
  {
    id: ApplicationType.TiDB,
    label: 'TiDB'
  },
  {
    id: ApplicationType.GeneralDatabase,
    label: 'protection_general_database_label'
  },
  {
    id: ApplicationType.SapHana,
    label: 'SAP HANA'
  },
  {
    id: ApplicationType.Saponoracle,
    label: 'common_sap_on_oracle_label'
  }
];

export const VM_ICONS = [
  {
    id: ApplicationType.Vmware,
    label: 'VMware'
  },
  {
    id: ApplicationType.CNware,
    label: 'common_cnware_label'
  },
  {
    id: ApplicationType.Nutanix,
    label: 'common_nutanix_label'
  },
  {
    id: ApplicationType.KubernetesStatefulSet,
    label: 'protection_kubernetes_flexvolume_label'
  },
  {
    id: ApplicationType.FusionCompute,
    label: 'FusionCompute'
  },
  {
    id: ApplicationType.FusionOne,
    label: 'protection_fusionone_label'
  },
  {
    id: ApplicationType.KubernetesDatasetCommon,
    label: 'protection_kubernetes_container_label'
  },
  {
    id: ApplicationType.HyperV,
    label: 'common_hyperv_label'
  }
];

export const BIG_DATA_ICONS = [
  {
    id: ApplicationType.HDFS,
    label: 'HDFS'
  },
  {
    id: ApplicationType.HBase,
    label: 'HBase'
  },
  {
    id: ApplicationType.Hive,
    label: 'Hive'
  },
  {
    id: ApplicationType.Elasticsearch,
    label: 'ElasticSearch'
  }
];

export const COPIES_ICONS = [
  {
    id: ApplicationType.Replica,
    label: 'common_copy_a_copy_label'
  }
];

export const STORAGE_ICONS = [
  {
    id: ApplicationType.NASFileSystem,
    label: 'common_nas_file_system_label'
  },
  {
    id: ApplicationType.Ndmp,
    label: 'protection_ndmp_protocol_label'
  },
  {
    id: ApplicationType.NASShare,
    label: 'common_nas_shared_label'
  },
  {
    id: ApplicationType.LocalFileSystem,
    label: 'common_local_file_system_label'
  },
  {
    id: ApplicationType.LocalLun,
    label: 'protection_local_lun_label'
  },
  {
    id: ApplicationType.ObjectStorage,
    label: 'common_object_storage_label'
  },
  {
    id: ApplicationType.CommonShare,
    label: 'protection_commonshare_label'
  }
];
export const CLOUD_ICONS = [
  {
    id: ApplicationType.HCSCloudHost,
    label: 'common_cloud_label'
  },
  {
    id: ApplicationType.OpenStack,
    label: 'common_open_stack_label'
  },
  {
    id: ApplicationType.GaussDBForOpenGauss,
    label: 'protection_gaussdb_for_opengauss_label'
  },
  {
    id: ApplicationType.LightCloudGaussDB,
    label: 'protection_light_cloud_gaussdb_label'
  },
  {
    id: ApplicationType.ApsaraStack,
    label: 'protection_ali_cloud_label'
  }
];

export enum FileSetFilterType {
  Exclude = 1,
  Include
}

// 保护资源动作
export enum ProtectResourceAction {
  Create,
  Modify,
  Clone
}

// 副本恢复类型
export enum RestoreType {
  CommonRestore = 'CR',
  InstanceRestore = 'IR',
  MountRestore = 'MR',
  FileRestore = 'FLR'
}

// File恢复到原位置选择类型
export enum FileReplaceStrategy {
  Replace = 0,
  Ignore = 2,
  ReplaceOldFile = 1
}

// VMware的File恢复到原位置选择类型
export enum VmFileReplaceStrategy {
  Overwriting = 'OVERWRITING',
  Skip = 'SKIP',
  Replace = 'REPLACE',
  Download = 'DOWNLOAD'
}
// ASM认证类型
export enum AsmAuthType {
  Database,
  Os
}

export enum SQLServerAuthType {
  Os = 'using_winAuth',
  Database = 'using_sqlAuth'
}

export const PROTECTION_NAVIGATE_STATUS = {
  protectionStatus: '',
  osType: ''
};

export const JOB_NAVIGATE_PARAMS = {
  status: '',
  jobId: ''
};

export enum DatabaseTabsIndex {
  Instance = 'Instance',
  Cluster = 'Cluster'
}

export enum PermissonsAttributes {
  FolderOnly,
  FileAndFolder
}

export enum DaysOfType {
  DaysOfYear = 'year',
  DaysOfMonth = 'month',
  DaysOfWeek = 'week',
  DaysOfDay = 'day',
  DaysOfHour = 'hour',
  DaysOfMinute = 'minute'
}

export enum TRIGGER_TYPE {
  year = 'year',
  month = 'month',
  week = 'week',
  day = 'day',
  hour = 'hour'
}

export const SLA_BACKUP_NAME = {
  full: 'common_full_backup_label',
  log: 'common_log_backup_label',
  snapshot: 'common_production_snapshot_label',
  difference_increment: 'common_incremental_backup_label',
  cumulative_increment: 'common_diff_backup_label',
  permanent_increment: 'common_permanent_backup_label'
};

export enum HdfsFilesetReplaceOptions {
  Skip = 'Skip',
  Overwrite = 'Overwrite',
  Replace = 'Newer'
}

export enum ProxyHostSelectMode {
  Auto,
  Manual
}

export enum FilterBy {
  FileName = 1,
  FolderName,
  Format
}

export enum FilterMode {
  EXCLUDE = 1,
  INCLUDE = 2
}

export enum OverWriteOption {
  Overwrite = 1,
  Skip,
  Replace
}

export enum PermissonLimitation {
  Mapping,
  Retained
}

export enum RootPermisson {
  Disable,
  Enable
}

export enum PortPermisson {
  Fixed,
  Arbitrary
}

export enum RestoreV2LocationType {
  ORIGIN = 'original',
  NEW = 'new',
  NATIVE = 'native'
}

export enum RestoreV2Type {
  CommonRestore = 'normalRestore',
  InstanceRestore = 'instantRestore',
  MountRestore = 'MR',
  FileRestore = 'fineGrainedRestore'
}

export enum AgentsSubType {
  NasFileSystem = 'NasFileSystemPlugin',
  NasShare = 'NasSharePlugin',
  Ndmp = 'NDMP-BackupSetPlugin'
}

export enum RestoreFileType {
  Directory = 'd',
  File = 'f',
  Link = 'l'
}

export enum InstanceType {
  TopInstance = '1',
  NotTopinstance = '0'
}

export enum NasFileReplaceStrategy {
  Replace,
  Ignore,
  ReplaceOldFile
}

export enum NasSecurityStyle {
  NTFS = 2,
  UNIX = 3
}

export enum RestoreMode {
  DownloadRestore = 'DownloadRestore', //远端副本先下载到本地再恢复
  RemoteRestore = 'RemoteRestore', //远端副本直接恢复
  LocalRestore = 'LocalRestore' //本地副本直接恢复
}

export const HCSHostInNormalStatus = [
  'HARD_REBOOT',
  'BUILD',
  'DELETED',
  'ERROR',
  'MIGRATING',
  'REBOOT',
  'RESIZE',
  'REVERT_RESIZE',
  'SHELVED',
  'SHELVED_OFFLOADED',
  'UNKNOWN',
  'VERIFY_RESIZE'
];

export const FCVmInNormalStatus = [
  'unknown',
  'hibernated',
  'creating',
  'shutting-down',
  'migrating',
  'fault-resuming',
  'starting',
  'stopping',
  'hibernating',
  'recycling'
];

export enum SnapshotRetenTion {
  permanent = 1,
  specify = 2
}

export enum SnapshotRstore {
  ORIGIN = 'original',
  SHAREDPATH = 'shared-path'
}

export enum ClusterEnvironment {
  oralceClusterEnv = 'Oracle-clusterEnv'
}

export const MultiClusterStatus = {
  nodeStatus: []
};
