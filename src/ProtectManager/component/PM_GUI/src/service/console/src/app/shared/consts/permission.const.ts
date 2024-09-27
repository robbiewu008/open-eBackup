import { find, includes, isEmpty, isUndefined, map } from 'lodash';

// 用户角色
export enum RoleType {
  Null = -1,
  SysAdmin = 1, // 系统管理员
  DataProtectionAdmin = 2, // 数据保护管理员
  Auditor = 5 // 审计员
}

/**
 * 判断是否可视为数据保护管理员。RBAC自定义角色拥有与数据保护管理员相同的基本权限，且其uuid转化为Number后一定为NaN。
 * @param role 角色uuid，数字或字符串格式均可
 */
export function isRBACDPAdmin(role) {
  return !Number(role) || Number(role) === RoleType.DataProtectionAdmin;
}

// 所有内置角色
export const DefaultRoles = {
  sysAdmin: {
    roleId: '1',
    roleName: 'Role_SYS_Admin'
  },
  dpAdmin: {
    roleId: '2',
    roleName: 'Role_DP_Admin'
  },
  audit: {
    roleId: '5',
    roleName: 'Role_Auditor'
  },
  rdAdmin: {
    roleId: '6',
    roleName: 'Role_RD_Admin'
  },
  drAdmin: {
    roleId: '7',
    roleName: 'Role_DR_Admin'
  }
};

// 角色权限映射
export const RoleOperationMap = {
  manageClient: 'manageClient', // 管理客户端
  newRestore: 'newRestore', // 新位置恢复
  nativeRestore: 'nativeRestore', // 本机恢复
  restoreExercise: 'restoreExercise', // 恢复演练
  airGap: 'airGap', // 管理air gap
  livemount: 'liveMount', // 即时挂载
  preventExtortionAndWorm: 'preventExtortionAndWorm', // 管理防勒索&worm
  desensitization: 'desensitization', // 脱敏
  manageResource: 'manageResource', // 管理生产资源（组）
  backup: 'backup', // 备份
  replication: 'replication', // 复制
  archive: 'archive', // 归档
  sla: 'sla', // 管理SLA
  speedLimitStrategy: 'speedLimitStrategy', // 管理限速策略
  deleteCopy: 'deleteCopy', // 副本删除
  originalRestore: 'originalRestore', // 原位置恢复
  report: 'report', // 报表
  liveMountPolicy: 'liveMountPolicy', // 挂载更新策略
  copyDelete: 'copyDelete', // 副本删除
  copyIndex: 'copyIndex' // 副本索引
};

// 用户角色具有的操作权限，进入页面会查询并写入
export const RoleOperationAuth = [];

// 权限函数
export function hasPermission(item, authKey: string): boolean {
  if (isUndefined(item.resourceRoleAuth)) {
    return true;
  }
  return includes(map(item.resourceRoleAuth, 'authOperation'), authKey);
}

// 资源管理权限
export function hasResourcePermission(item): boolean {
  return hasPermission(item, RoleOperationMap.manageResource);
}

// 资源备份权限
export function hasBackupPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.backup);
}

// 复制权限
export function hasReplicationPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.replication);
}

// 资源保护权限
export function hasProtectPermission(item): boolean {
  if (isUndefined(item.resourceRoleAuth)) {
    return true;
  }
  return !isEmpty(
    find(map(item.resourceRoleAuth, 'authOperation'), v =>
      includes(
        [
          RoleOperationMap.backup,
          RoleOperationMap.replication,
          RoleOperationMap.archive
        ],
        v
      )
    )
  );
}

// 资源原位置恢复权限
export function hasOriginalRecoveryPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.originalRestore);
}

// 资源新位置恢复权限
export function hasNewRecoveryPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.newRestore);
}

// 资源本机恢复权限
export function hasNativeRecoveryPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.nativeRestore);
}

// 资源恢复
export function hasRecoveryPermission(item): boolean {
  return (
    hasOriginalRecoveryPermission(item) ||
    hasNewRecoveryPermission(item) ||
    hasNativeRecoveryPermission(item)
  );
}

// 是否具有SLA权限
export function hasSlaPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.sla);
}

// 是否具有限速策略权限
export function hasSpeedLimitPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.speedLimitStrategy);
}

// 是否具有报表权限
export function hasReportPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.report);
}

// 挂载策略
export function hasLivemountPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.livemount);
}

// 更新挂载策略权限
export function hasLivemountPolicyPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.liveMountPolicy);
}

// 副本删除权限
export function hasCopyDeletePermission(item): boolean {
  return hasPermission(item, RoleOperationMap.copyDelete);
}

// 副本索引权限
export function hasCopyIndexPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.copyIndex);
}

// airgap权限
export function hasAirgapPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.airGap);
}

// 防勒索权限
export function hasWormPermission(item): boolean {
  return hasPermission(item, RoleOperationMap.preventExtortionAndWorm);
}

// RBAC权限表
export const PermissionTable = [
  {
    value: 'ResourceManagement',
    label: 'system_resource_management_label',
    expanded: true,
    children: [
      {
        value: '1',
        label: 'system_manage_client_label'
      },
      {
        value: '2',
        label: 'system_manage_resource_label'
      }
    ]
  },
  {
    value: 'ProductionManagement',
    label: 'system_protect_management_label',
    expanded: true,
    children: [
      {
        value: '3',
        label: 'common_backup_label'
      },
      {
        value: '4',
        label: 'common_replication_label'
      },
      {
        value: '5',
        label: 'common_archive_label'
      },
      {
        value: '6',
        label: 'system_manage_sla_label'
      },
      {
        value: '7',
        label: 'system_manage_speed_limit_label'
      }
    ]
  },
  {
    value: 'CopyManagement',
    label: 'system_copy_management_label',
    expanded: true,
    children: [
      {
        value: '8',
        label: 'system_restore_to_origin_label'
      },
      {
        value: '9',
        label: 'system_restore_to_new_label'
      },
      {
        value: '10',
        label: 'system_native_restore_label'
      },
      {
        value: '11',
        label: 'explore_recovery_drill_label'
      },
      {
        value: '17',
        label: 'common_delete_copy_label'
      },
      {
        value: '18',
        label: 'system_copy_index_label'
      },
      {
        value: '13',
        label: 'common_live_mount_label',
        tooltip: 'system_live_mount_tip_label'
      },
      {
        value: '19',
        label: 'system_live_mount_policy_label'
      }
    ]
  },
  {
    value: 'DataSecurity',
    label: 'system_data_security_label',
    expanded: true,
    children: [
      {
        value: '12',
        label: 'system_manage_air_gap_label'
      },
      {
        value: '14',
        label: 'common_anti_policy_label'
      },
      {
        value: '15',
        label: 'common_data_desensitization_label'
      }
    ]
  },
  {
    value: 'Report',
    label: 'common_report_label',
    expanded: true,
    children: [
      {
        value: '16',
        label: 'common_report_label'
      }
    ]
  }
];

// 系统所有url路由
export enum RouterUrl {
  Login = '/login',
  Init = '/init',
  Home = '/home',
  ErrorPage = '/error-page',
  ProtectionResource = '/protection/resource',
  ReportDetail = '/report-detail',
  ProtectionSummary = '/protection/summary',
  ProtectionDatabase = '/protection/database',
  ProtectionBigData = '/protection/big-data',
  ProtectionContainer = '/protection/container',
  ProtectionCloud = '/protection/cloud',
  ProtectionPrivateCloud = '/protection/private-cloud',
  ProtectionVirtualization = '/protection/virtualization',
  ProtectionFileService = '/protection/file-service',
  ProtectionApplication = '/protection/application',
  ProtectionActiveDirectory = '/protection/application/active-directory',
  ProtectionBareMetal = '/protection/bare-metal',
  ProtectionBareMetalFilesetTemplate = '/protection/bare-metal/fileset-template',
  ProtectionHostAppHost = '/protection/host',
  ProtectionHostAppHostRegister = '/protection/register-host',
  ProtectionHostAppFilesetTemplate = '/protection/file-service/fileset-template',
  ProtectionHostAppVolume = '/protection/file-service/volume',
  ProtectionHostAppOracle = '/protection/database/oracle',
  ProtectionHostAppGaussDBT = '/protection/database/gaussdb-t',
  ProtectionHostAppGaussDBDWS = '/protection/big-data/gaussdb-dws',
  ProtectionHostAppMySQL = '/protection/database/mysql',
  ProtectionHostAppPostgreSQL = '/protection/database/postgre-sql',
  ProtectionHostAppKingBase = '/protection/database/king-base',
  ProtectionHostAppDB2 = '/protection/database/db-two',
  ProtectionHostAppSQLServer = '/protection/database/sql-server',
  ProtectionHostAppGBase = '/protection/host-app/gbase',
  ProtectionHostAppDameng = '/protection/database/dameng',
  ProtectionHostAppGaussDB = '/protection/host-app/gaussdb',
  ProtectionHostAppRedis = '/protection/big-data/redis',
  ProtectionHostAppGoldendb = '/protection/database/goldendb',
  ProtectionHostAppInformix = '/protection/database/informix',
  ProtectionHostAppClickHouse = '/protection/big-data/click-house',
  ProtectionOpenGauss = '/protection/database/opengauss',
  ProtectionHostAppOceanBase = '/protection/database/ocean-base',
  ProtectionDameng = '/protection/dameng',
  ProtectionHostAppMongoDB = '/protection/big-data/mongodb',
  ProtectionHostAppSapHana = '/protection/application/sap-hana',
  ProtectionHostAppExchange = '/protection/application/exchange',
  ProtectionHostGeneralDatabase = '/protection/database/general-database',
  ProtectionGbase = '/protection/database/gbase',
  ProtectionHostAppTdsql = '/protection/database/tdsql',
  ProtectionHostAppTidb = '/protection/database/tidb',
  ProtectionVirtualizationVmware = '/protection/virtualization/vmware',
  ProtectionVirtualizationCnware = '/protection/virtualization/cnware',
  ProtectionVirtualizationFusionCompute = '/protection/virtualization/fusion-compute',
  ProtectionVirtualizationFusionOne = '/protection/virtualization/fusion-one',
  ProtectionVirtualizationFusionsphere = '/protection/virtualization/fusionsphere',
  ProtectionVirtualizationHyperV = '/protection/virtualization/hyper-v',
  ProtectionVirtualizationH3cCas = '/protection/virtualization/h3c-cas',
  ProtectionCloudHuaweiStack = '/protection/cloud/huawei-stack',
  ProtectionVirtualizationKubernetes = '/protection/container/kubernetes',
  ProtectionVirtualizationKubernetesContainer = '/protection/container/kubernetes-container',
  ProtectionCloudOpenstack = '/protection/cloud/openstack',
  ProtectionHostAppGaussDBForOpengauss = '/protection/cloud/gaussdb-for-opengauss',
  ProtectionHostApLightCloudGaussDB = '/protection/database/light-cloud-gaussdb',
  ProtectionObject = '/protection/file-service/object',
  ProtectionApsaraStack = '/protection/cloud/apsara-stack',
  ProtectionBigDataHdfs = '/protection/big-data/hdfs',
  ProtectionBigDataHbase = '/protection/big-data/hbase',
  ProtectionBigDataHive = '/protection/big-data/hive',
  ProtectionBigDataElasticsearch = '/protection/big-data/elasticsearch',
  ProtectionStorageDeviceInfo = '/protection/file-service/storage-device-info',
  ProtectionDoradoFileSystem = '/protection/file-service/dorado-file-system',
  ProtectionLocalFileSystem = '/protection/storage/local-file-system',
  ProtectionLocalResource = '/protection/storage/local-resource',
  ProtectionNasShared = '/protection/file-service/nas-shared',
  ProtectionCommonShare = '/protection/file-service/common-share',
  ProtectionLimitRatePolicy = '/protection/policy/limit-rate-policy',
  ProtectionSla = '/protection/policy/sla',
  ProtectionKubernetesRule = '/protection/policy/kubernetes-rule',
  ExploreStorageDevice = '/explore/storage-device',
  ExploreAntiRansomwareProtection = '/explore/ransomware-protection',
  ExploreAntiRansomwareProtectionFileInterception = '/explore/ransomware-protection/file-interception',
  ExploreAntiRansomwareProtectionFileBlock = '/explore/ransomware-protection/file-block',
  ExploreAntiRansomwareProtectionDataBackup = '/explore/ransomware-protection/data-backup',
  ExploreAntiRansomwareProtectionRealTimeDetection = '/explore/ransomware-protection/real-time-detection',
  ExploreAntiRansomwareProtectionIntellIgent = '/explore/ransomware-protection/intelligent-detection',
  ExploreAntiRansomwareProtectionModel = '/explore/ransomware-protection/detection-model',
  ExploreSnapShotData = '/explore/snapshot-data',
  ExploreDetectionReport = '/explore/detection-report',
  ExploreCopyDataApplication = '/explore/copy-data/application',
  ExploreCopyDataActiveDirectory = '/explore/copy-data/application/active-directory',
  ExploreCopyData = '/explore/copy-data',
  ExploreCopyDataDataBase = '/explore/copy-data/database',
  ExploreCopyDataBigData = '/explore/copy-data/big-data',
  ExploreCopyDataVirtualization = '/explore/copy-data/virtualization',
  ExploreCopyDataContainer = '/explore/copy-data/container',
  ExploreCopyDataCloud = '/explore/copy-data/cloud',
  ExploreCopyDataPrivateCloud = '/explore/copy-data/private-cloud',
  ExploreCopyDataFileService = '/explore/copy-data/file-service',
  ExploreCopyDataBareMetal = '/explore/copy-data/bare-metal',
  ExploreCopyDataBareMetalFileset = '/explore/copy-data/bare-metal/fileset',
  ExploreCopyDataHost = '/explore/copy-data/host',
  ExploreCopyDataFileset = '/explore/copy-data/file-service/fileset',
  ExploreCopyDataVolume = '/explore/copy-data/file-service/volume',
  ExploreCopyDataOracle = '/explore/copy-data/database/oracle',
  ExploreCopyDataGaussDBT = '/explore/copy-data/database/gaussdb-t',
  ExploreCopyDataGaussDBDWS = '/explore/copy-data/big-data/gaussdb-dws',
  ExploreCopyDataMySQL = '/explore/copy-data/database/mysql',
  ExploreCopyDataOpenGauss = '/explore/copy-data/database/opengauss',
  ExportCopyDataDameng = '/explore/copy-data/database/dameng',
  ExploreCopyDataKingBase = '/explore/copy-data/database/king-base',
  ExploreCopyDataRedis = '/explore/copy-data/big-data/redis',
  ExploreCopyDataPostgreSQL = '/explore/copy-data/database/postgre-sql',
  ExploreCopyDataDB2 = '/explore/copy-data/database/db2',
  ExploreCopyDataSQLServer = '/explore/copy-data/database/sql-server',
  ExploreCopyDataGoldendb = '/explore/copy-data/database/goldendb',
  ExploreCopyDataInformix = '/explore/copy-data/database/informix',
  ExploreCopyDataGeneralDatabase = '/explore/copy-data/database/general-database',
  ExploreCopyDataDatabaseGbase = '/explore/copy-data/database/gbase',
  ExploreCopyDataDatabaseExchange = '/explore/copy-data/application/exchange',
  ExploreCopyDataVMware = '/explore/copy-data/virtualization/vmware',
  ExploreCopyDataCNware = '/explore/copy-data/virtualization/cnware',
  ExploreCopyDataOceanBase = '/explore/copy-data/database/ocean-base',
  ExploreCopyDataTDSQL = '/explore/copy-data/database/tdsql',
  ExploreCopyDataTiDB = '/explore/copy-data/database/tidb',
  ExploreCopyDataHyperv = '/explore/copy-data/virtualization/hyper-v',
  ExploreCopyDataFusionCompute = '/explore/copy-data/virtualization/fusion-compute',
  ExploreCopyDataFusionOne = '/explore/copy-data/virtualization/fusion-one',
  ExploreCopyDataHuaweiStack = '/explore/copy-data/cloud/huawei-stack',
  ExploreCopyDataOpenStack = '/explore/copy-data/cloud/openstack',
  ExploreCopyDataApsaraStack = '/explore/copy-data/cloud/apsara-stack',
  ExploreCopyDataHadoop = '/explore/copy-data/hadoop',
  ExploreCopyDataGaussDB = '/explore/copy-data/gaussdb',
  ExploreCopyDataGBase = '/explore/copy-data/gbase',
  ExploreCopyDataMongoDB = '/explore/copy-data/big-data/mongodb',
  ExploreCopyDataClickHouse = '/explore/copy-data/big-data/click-house',
  ExploreCopyDataGaussdbForOpengauss = '/explore/copy-data/cloud/gaussdb-for-opengauss',
  ExploreCopyDataLightCloudGaussdb = '/explore/copy-data/database/light-cloud-gaussdb',
  ExploreCopyDataObject = '/explore/copy-data/file-service/object',
  ExploreCopyDataH3CCas = '/explore/copy-data/h3c-cas',
  ExploreCopyDataDeviceInfo = '/explore/copy-data/storage-device-info',
  ExploreCopyDataFileSystem = '/explore/copy-data/file-service/dorado-file-system',
  ExploreCopyLocalFileSystem = '/explore/copy-data/local-file-system',
  ExploreCopyDataNasShared = '/explore/copy-data/file-service/nas-shared',
  ExploreCopyDataCommonShare = '/explore/copy-data/file-service/common-share',
  ExploreCopyDataKubernetes = '/explore/copy-data/container/kubernetes',
  ExploreCopyDataHdfs = '/explore/copy-data/big-data/hdfs',
  ExploreCopyDataHbase = '/explore/copy-data/big-data/hbase',
  ExploreCopyDataHive = '/explore/copy-data/big-data/hive',
  ExploreCopyDataElasticsearch = '/explore/copy-data/big-data/elasticsearch',
  ExploreCopyDataKubernetesContainer = '/explore/copy-data/container/kubernetes-container',
  ExploreCopyDataSapHana = '/explore/copy-data/application/sap-hana',
  ExploreLiveMount = '/explore/live-mounts',
  ExploreLiveMountOracle = '/explore/live-mounts/oracle',
  ExploreLiveMountFileset = '/explore/live-mounts/fileset',
  ExploreLiveMountVolume = '/explore/live-mounts/volume',
  ExploreLiveMountMysql = '/explore/live-mounts/mysql',
  ExploreLiveMountTdsql = '/explore/live-mounts/tdsql',
  ExploreLiveMountExchange = '/explore/live-mounts/exchange',
  ExploreLiveMountApplication = '/explore/live-mounts/application',
  ExploreLiveMountApplicationFileset = '/explore/live-mounts/application/fileset',
  ExploreLiveMountApplicationVolume = '/explore/live-mounts/application/volume',
  ExploreLiveMountApplicationOracle = '/explore/live-mounts/application/oracle',
  ExploreLiveMountApplicationMysql = '/explore/live-mounts/application/mysql',
  ExploreLiveMountApplicationTdsql = '/explore/live-mounts/application/tdsql',
  ExploreLiveMountApplicationExchange = '/explore/live-mounts/application/exchange',
  ExploreLiveMountApplicationVmware = '/explore/live-mounts/application/vmware',
  ExploreLiveMountApplicationCnware = '/explore/live-mounts/application/cnware',
  ExploreLiveMountApplicationFileSystem = '/explore/live-mounts/application/dorado-file-system',
  ExploreLiveMountApplicationNasshare = '/explore/live-mounts/application/nas-shared',
  ExploreAntiRansomware = '/explore/anti-ransomware/overview',
  ExploreAntiVmware = '/explore/anti-ransomware/vmware',
  ExploreAntiCNware = '/explore/anti-ransomware/cnware',
  ExploreAntiDoradoFileSystem = '/explore/anti-ransomware/dorado-file-system',
  ExploreAntiNasShared = '/explore/anti-ransomware/nas-shared',
  ExploreAntiFileset = '/explore/anti-ransomware/fileset',
  ExploreAntiHuaweiStack = '/explore/anti-ransomware/huawei-stack',
  ExploreAntiApplication = '/explore/anti-ransomware/application',
  ExploreAntiApplicationVmware = '/explore/anti-ransomware/application/vmware',
  ExploreAntiApplicationCnware = '/explore/anti-ransomware/application/cnware',
  ExploreAntiApplicationDoradoFileSystem = '/explore/anti-ransomware/application/dorado-file-system',
  ExploreAntiApplicationNasShared = '/explore/anti-ransomware/application/nas-shared',
  ExploreAntiApplicationFileset = '/explore/anti-ransomware/application/fileset',
  ExploreLiveMountVmware = '/explore/live-mounts/vmware',
  ExploreLiveMountCmware = '/explore/live-mounts/cnware',
  ExploreLiveMountFileSystem = '/explore/live-mounts/dorado-file-system',
  ExploreLiveMountNasShared = '/explore/live-mounts/nas-shared',
  ExploreLiveMountKubernetes = '/explore/live-mounts/kubernetes',
  ExploreDataDesensitization = '/explore/data-desensitization',
  ExploreDataDesensitizationOracle = '/explore/data-desensitization/oracle',
  ExploreDataDesensitizationMysql = '/explore/data-desensitization/mysql',
  ExploreDataDesensitizationSqlServer = '/explore/data-desensitization/sql-server',
  ExplorePolicy = '/explore/policy',
  ExplorePolicyMountUpdatePolicy = '/explore/policy/mount-update-policy',
  ExplorePolicyDesensitizationPolicy = '/explore/policy/desensitization-policy',
  ExplorePolicyAntiPolicySetting = '/explore/policy/anti-policy-setting',
  ExplorePolicyAirgap = '/explore/airgap',
  ExploreRansomwareOverview = '/explore/anti-ransomware/overview',
  ExploreRansomwareCloudBackupOverview = '/explore/anti-ransomware/cloud-backup-overview',
  ExploreRansomwareDetectionSetting = '/explore/anti-ransomware/detection-setting',
  ExploreRansomwareBlockingRuleList = '/explore/anti-ransomware/blocking-rule-list',
  ExploreRansomwareRealtimeDetection = '/explore/anti-ransomware/real-time-detect',
  ExploreRansomwareDetectionModelList = '/explore/anti-ransomware/detection-model-list',
  ExploreRansomwareVMware = '/explore/anti-ransomware/vmware',
  ExploreRansomwareDoradoFileSystem = '/explore/anti-ransomware/dorado-file-system',
  ExploreRansomwareNasShared = '/explore/anti-ransomware/nas-shared',
  ExploreRansomwareLocalFileSystem = '/explore/local-file-system',
  ExploreRansomwareKubernetes = '/explore/anti-ransomware/kubernetes',
  ExploreRecoveryDrill = '/explore/recovery-drill',
  ExploreCreateDrill = '/explore/create-drill',
  ExploreModifyDrill = '/explore/modify-drill',
  ExploreDrillDetail = '/explore/drill-detail',
  ExploreDrillExecuteLog = '/explore/drill-execute-log',
  InsightReport = '/insight/report',
  InsightAlarms = '/insight/alarms',
  InsightJobs = '/insight/jobs',
  InsightReports = '/insight/reports',
  InsightPerformance = '/insight/performance',
  SystemInfrastructure = '/system/infrastructure',
  SystemInfrastructureClusterManagement = '/system/infrastructure/cluster-management',
  SystemInfrastructureLocalStorage = '/system/infrastructure/local-storage',
  SystemInfrastructureExternalStorage = '/system/infrastructure/external-storage',
  SystemInfrastructureArchiveStorage = '/system/infrastructure/archive-storage',
  SystemInfrastructureBackupStorage = '/system/infrastructure/backup-storage',
  SystemInfrastructureNasBackupStorage = '/system/infrastructure/nas-backup-storage',
  SystemInfrastructureHcsStorage = '/system/infrastructure/hcs-storage',
  SystemSecurity = '/system/security',
  SystemSecurityRbac = '/system/security/rbac',
  SystemSecurityUserrole = '/system/security/userrole',
  SystemSecurityUserQuota = '/system/security/user-quota',
  SystemSecuritySecurityPolicy = '/system/security/securitypolicy',
  SystemSecurityCertificate = '/system/security/certificate',
  SystemSecurityKerberos = '/system/security/kerberos',
  SystemSamlSsoConfig = '/system/security/samlSsoConfig',
  SystemDataSecurity = '/system/security/dataSecurity',
  SystemSecurityLdap = '/system/security/ldapService',
  SystemLicense = '/system/license',
  SystemNetworkConfig = '/system/network-config',
  SystemTagManagement = '/system/settings/tag-management',
  LogManagement = '/system/log-management',
  ExternalAssociatedSystems = '/system/external-associated-systems',
  ExportQuery = '/system/export-query',
  SystemSettings = '/system/settings',
  SystemSettingsSystemBackup = '/system/settings/system-backup',
  SystemSettingsAlarmNotify = '/system/settings/alarm-notify',
  SystemSettingsAlarmNotifySettings = '/system/settings/alarm-notify-settings',
  SystemSettingsAlarmSettings = '/system/settings/alarm-settings',
  SystemSettingsAlarmDump = '/system/settings/alarm-dump',
  SystemSettingsSnmpTrap = '/system/settings/snmp-trap',
  SystemDeviceTime = '/system/settings/system-time',
  SftpService = '/system/settings/sftp-service',
  IbmcService = '/system/settings/ibmc',
  SystemSettingConfigNetwork = '/system/settings/config-network',
  hostTrustworthiness = '/system/security/hostTrustworthiness',
  GlobalSearch = '/search',
  SystemSecurityAdfs = '/system/security/adfsConfig'
}

// 操作权限
export enum OperateItems {
  SysadminOnly, // 仅限系统管理员具有操作权限
  ModifyHost, // 移除主机
  RemoveHost, // 移除主机
  ProtectHosts, // 保护主机
  ModifyHostProtection, // 修改保护
  RemoveHostProtection, // 移除保护
  ManuallyBackHost, // 手工执行备份
  ManuallyArchiveHost, // 手工执行归档
  ActivateHostProtection, // 激活保护
  DeactivateHostProtection, // 去激活保护
  ResourceAuth, // 资源授权
  ResourceReclaiming, // 资源回收
  SynchTrapInfo, // 同步Trap信息
  CreateHostFileset, // 创建文件集
  DeleteHostFileset, // 删除文件集
  ModifyHostFileset, // 修改文件集
  ProtectHostFileset, // 保护文件集
  ModifyFilesetProtection, // 修改保护
  RemoveFilesetProtection, // 移除保护
  ManuallyBackFileset, // 手动执行备份
  ManuallyArchiveFileset, // 手动执行归档
  ManuallyReplicationFileset, // 手动执行复制
  ActivateFilesetProtection, // 激活保护
  DeactivateFilesetProtection, // 去激活保护
  RegisterDatabase, // 注册数据库
  ProtectDatabases, // 保护数据库
  ModifyDatabaseProtection, // 修改保护
  RemoveDatabaseProtection, // 移除保护
  ManuallyBackDatabase, // 手动执行备份
  ManuallyDatabaseProtection, // 手工执行保护
  ManuallyArchiveDatabase, // 手工执行归档
  ManuallyReplicationDatabase, // 手工执行复制
  ActivateDatabaseProtection, // 激活保护
  DeactivateDatabseProtection, // 去激活保护
  ModifyDatabaseAuth, // 数据库修改认证信息
  DeleteDatabase, // 删除数据库
  CreatingCluster, // 创建集群
  Deletingcluster, // 删除集群
  ModifyingCluster, // 修改集群
  ModifyingASMAuth, // 修改ASM授权
  RegisterApplication, // 注册
  ProtectApplication, // 保护
  ModifyApplicationProtection, // 修改保护
  RemoveApplicationProtection, // 移除保护
  ManuallyApplicationProtection, // 手工执行保护
  ManuallyArchiveApplication, // 手工执行归档
  ManuallyReplicationApplication, // 手工执行复制
  ActivateApplicationProtection, // 激活保护
  DeactivateApplicationProtection, // 去激活保护
  RegisterVirtualizationPlatform, // 注册虚拟化平台
  ProtectVM, // 保护
  ProtectTENANT, // 保护租户
  ProtectFusionCompute, // 保护
  RemoveVMProtection, // 移除保护
  RemoveFCProtection, // 移除保护
  ModifyVMProtection, // 修改保护
  ManuallyBackVM, // 手工执行备份
  ManuallyVMProtection, // 手工执行保护
  ManuallyArchiveVM, // 手工执行归档
  ManuallyReplicationVM, // 手工执行复制
  ActivateVMProtection, // 激活保护
  DeactivateVMProtection, // 去激活保护
  ActivateFCProtection, // 激活保护
  DeactivateFCProtection, // 去激活保护
  ModifyVirtualizationRegister, // 修改虚拟化平台注册信息
  RescanVirtualizationPlatform, // 重新扫描虚拟化平台
  DeregisterVirtualizationPlatform, // 取消注册虚拟化平台,
  RegisterBigDataCluster, // 注册大数据集群
  DeregisterBigDataCluster, // 取消注册大数据集群
  ModifyBigDataCluster, // 修改大数据集群
  CreatingBigDataFileset, // 创建大数据文件集
  DeletingBigDataFileset, // 删除大数据文件集
  ModifyBigDataFileset, // 修改大数据文件集
  ProtectBigDataFilesets, // 保护大数据文件集
  ModifyBigDataFilesetProtection, // 修改大数据文件集保护
  RemoveBigDataFilesetProtection, // 移除大数据文件集群保护
  ManuallyBackingBigDataFileset, // 手动执行大数据文件集群备份
  ManuallyArchivingBigDataFileset, // 手动执行大数据文件集归档
  ManuallyReplicationBigDataFileset, // 手动执行大数据文件集复制
  ActivateBigDataFilesetProtection, // 激活大数据文件集保护
  DeactivatingBigDataFilesetProtection, // 去激活大数据文集集保护
  QueryingCloudPlatform, // 查询云平台
  RegisteringCloudPlatform, // 注册云平台
  RegisteringTenant, // 注册租户
  CreateSLA, // 创建SLA
  ModifySLA, // 修改SLA
  DeleteSLA, // 删除SLA
  CloneSLA, // 克隆SLA
  CreateQos, // 创建Qos
  ModifyQos, // 修改Qos
  DeleteQos, // 删除Qos
  CreateKubernetesRule, // 创建Kubernetes规则
  ModifyKubernetesRule, // 修改Kubernetes规则
  DeleteKubernetesRule, // 删除Kubernetes规则
  ModifyingCopyRetentionPolicy, // 修改副本保留策略
  CopyDuplicate, // 副本复制
  DeletingCopy, // 删除副本
  DownloadCopy, // 下载副本
  RestoreCopy, // 恢复
  InstanceRecovery, // 即时恢复
  MountingCopy, // 挂载
  CreatingLiveMount, // 创建
  DestroyLiveMount, // 销毁
  UpdateLatestCopyLiveMount, // 使用最新的副本更新
  ModifiedLiveMount, // 修改
  UpdateLiveMount, // 更新
  MigrateLiveMount, // 迁移
  DisableLiveMount, // 禁用
  ActivateLiveMount, // 激活
  ViewLiveMount, // 查看
  IdentitySensitiveData, // 识别敏感数据
  StartDataAnonymization, // 开始脱敏
  CheckAnonymization, // 查看脱敏结果
  CreateUpdatingPolicy, // 创建更新策略
  ModifyUpdatingPolicy, // 修改更新策略
  DeleteUpdatingPolicy, // 删除更新策略
  CloneUpdatingPolicy, // 克隆更新策略
  CreateDesensitizationPolicy, // 创建数据脱敏策略
  ModifyDesensitizationPolicy, // 修改数据脱敏策略
  DeleteDesensitizationPolicy, // 删除数据脱敏策略
  CloneDesensitizationPolicy, // 克隆数据脱敏策略
  CreateIdentificationRule, // 创建识别规则
  ModifyIdentificationRule, // 修改识别规则
  DeleteIdentificationRule, // 删除识别规则
  CloneIdentificationRule, // 克隆识别规则
  CreateDesensitizationRule, // 创建脱敏规则
  ModifyDesensitizationRule, // 修改脱敏规则
  DeleteDesensitizationRule, // 删除脱敏规则
  CloneDesensitizationRule, // 克隆脱敏规则
  CreateDataAnonymization, // 创建数据脱敏
  ModifyDataAnonymization, // 修改数据脱敏
  DeleteDataAnonymization, // 删除数据脱敏
  ChangePerformanceStatus, // 开启或者关闭性能监控开关
  AbortJob, // 终止任务
  ExportJob, //导出任务
  QueryingInfrastructureCluster, // 查询集群
  QueryingLocalClusterDetails, // 查询本地集群详情
  SettingLocalClusterIP, // 设置本地集群业务IP
  ClearingLocalClusterIP, // 清除本地集群业务IP
  ModifyingTargetClusterAuth, // 修改目标集群认证信息
  AddingTargetCluster, // 添加目标集群
  DeletingTargetCluster, // 删除目标集群
  ManagedTargetCluster, // 删除目标集群
  UnManagedTargetCluster, // 删除目标集群
  ManageHA, //HA管理
  AddHA, //添加HA成员
  ModifyHA, //修改HA参数
  DeleteHA, //移除HA成员
  QueryingLocalStorage, // 查询本地存储
  OpenDeviceManagement, // 打开设备管理
  ModifyAuthenticationStatus, // 修改认证状态
  AddingExternalStorage, // 添加外部存储
  ModifyingExternalStorage, // 修改外部存储
  DeletingExternalStorage, // 删除外部存储
  CreateArchiveStorage, // 创建归档存储
  ModifyingArchiveStorage, // 修改归档存储
  DeletingArchiveStorage, // 删除归档存储
  ImportingArchiveStorageCopy, // 导入归档存储副本
  ModifyStorageAlarmThreshold, // 修改告警阈值
  QueryingStorageDevices, // 查询存储
  CreatingStorageDevice, // 创建存储
  DeletingstorageDevice, // 删除存储
  CreateUserComponent, // 创建用户
  ModifyingUser, // 修改用户
  DeletingUser, // 删除用户
  LockingUser, // 锁定用户
  UnlockingUser, // 解锁用户
  ResetPassword, // 重置密码
  ModifyingPassword, // 修改密码
  QueryingRole, // 查询角色
  QuerySecurityPolicy, // 查询安全策略
  ModifySecurityPolicy, // 修改安全策略
  ViewingCertificateList, // 查看证书列表
  ViewingComponentCertificateDetails, // 查看组件证书详情
  AddingExternalComponentCertificates, // 添加外部组件证书
  ImportingExternalComponentCertificates, // 导入外部组件证书
  ImportingInternalComponentCertificates, // 导入内部组件证书
  ChangingCertificateAlarmThreshold, // 修改证书告警阈值
  DownloadingCertificate, // 下载证书
  RegenerateCertificate, // 重新生成证书
  DeletingCertificate, // 删除证书
  UpdateHACertificate, //更新HA证书
  ExportingRequestFile, // 导出请求文件
  ImportingRevocationList, // 导入吊销列表
  ViewingCRL, // 查看证书吊销列表
  DeletingCRL, // 删除证书吊销列表
  ViewingLicense, // 查看License
  ImportingLicenseFile, // 导入License
  ExportingLicenseFile, // 导出License
  ActivatingLicense, // 激活License
  ObtainingESN, // 获取ESN
  ViewingBackupPolicy, // 查看备份策略
  ModifyingBackupPolicy, // 修改备份策略
  ViewingBackupTasks, // 查看备份任务
  DeletingBackup, // 删除备份
  RevertingBackup, // 恢复备份
  ManuallyBackup, // 手动备份
  ImportingBackup, // 导出备份
  ExportingBackup, // 导出备份
  OpenSFTPService, // 开启或者关闭sftp服务开关
  AddSFTPUser, // 添加sftp用户
  DeleteSFTPUser, // 删除sftp用户
  ModifySFTPPassword, // 修改sftp用户密码
  ViewingNotifySendingConfigurations, // 查看通知发送配置
  ViewingNotifyReceivingSettings, // 查看通知接收设置
  ModifyNotifySendingSettings, // 修改通知发送设置
  ManagingNotifyReceivingSettings, // 管理通知接收设置
  ViewingAlarmDumpSettings, // 查看告警转储设置
  ModifyDumpSettings, // 修改转储设置
  DownloadDumpFile, // 下载转储文件
  DeletingDumpFile, // 删除转储文件
  UploadDumpFile, // 删除转储文件
  ModifySystemTime, // 修改系统时间,
  Protection, // 保护
  ModifyProtection, // 修改保护
  ActivateProtection, // 激活保护
  DeactivateProtection, // 取消激活保护
  RemoveProtection, // 移除保护
  ManualBackup, // 手动备份
  DeleteResource, // 删除资源
  RegisterNasShare, // 注册Nas Share
  RegisterRedis, // 注册Redis
  RegisterHCS, // 注册HCS
  ModifyHCSTenant, // 修改HCS租户
  UnRegisterHCSTenant, // 取消注册HCS租户
  HCSEnvironmentInfo, // HCS环境信息
  CreateDataset, // 创建数据集
  RegisterHdfsCluster,
  ModifyHdfsCluster,
  UnRegisterHdfsCluster,
  CreateKerberos,
  ModifyKerberos,
  DeleteKerberos,
  FileLevelRestore, // 文件级恢复
  SchemaLevelRestore, // Schema级恢复
  ExportFile, // 文件导出
  ManualIndex, // 手动索引
  DeleteIndex, // 删除索引
  AddStorageDevice, // 创建设备
  DeleteStorageDevice, // 删除存储设备
  ModifyStorageDevice, // 修改存储设备
  DownloadExportQuery,
  DeleteExportQuery,
  SetLogLevel,
  SetRestPswdEmail, // 找回密码邮箱设置
  ScanHCSProject,
  ScanTapeLibrary,
  AddBackupStorage,
  ModifyBackupStorage,
  DeleteBackupStorage,
  AddBackupStorageUnit,
  ModifyBackupStorageUnit,
  DeleteBackupStorageUnit,
  upgrateBackupStorageUnit,
  ModifyDataSecurity, // 数据安全
  ModifyHostTrustWorthiness, // 主机授信
  ModifyLdapConfig, // 修改ldap服务,
  AddReport, // 添加报告,
  DeleteReport, // 删除报告
  AddHcsStorage,
  CreateDrillPlan, // 演练计划
  AddExternalAssociatedSystem, // 添加外部关联系统
  EditExternalAssociatedSystem, // 编辑外部关联系统
  DeleteExternalAssociatedSystem, // 删除外部关联系统
  JumpExternalAssociatedSystem, // 跳转外部关联系统
  EditPacificNodeNetwork, // 修改业务网络
  EditTag, // 编辑标签名称
  DeleteTag, // 删除标签名称
  AddTag, //添加标签
  RemoveTag // 移除标签
}

/**
 * url权限
 */
export const URL_PERMISSION = {
  [RoleType.Null]: [],
  // 系统管理员
  [RoleType.SysAdmin]: [
    RouterUrl.Init,
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ProtectionResource,
    RouterUrl.ProtectionSummary,
    RouterUrl.ProtectionDatabase,
    RouterUrl.ProtectionBigData,
    RouterUrl.ProtectionVirtualization,
    RouterUrl.ProtectionContainer,
    RouterUrl.ProtectionCloud,
    RouterUrl.ProtectionPrivateCloud,
    RouterUrl.ProtectionFileService,
    RouterUrl.ProtectionApplication,
    RouterUrl.ProtectionActiveDirectory,
    RouterUrl.ProtectionBareMetal,
    RouterUrl.ProtectionBareMetalFilesetTemplate,
    RouterUrl.ProtectionHostAppHost,
    RouterUrl.ProtectionHostAppHostRegister,
    RouterUrl.ProtectionHostAppFilesetTemplate,
    RouterUrl.ProtectionHostAppVolume,
    RouterUrl.ProtectionHostAppOracle,
    RouterUrl.ProtectionHostAppGaussDBT,
    RouterUrl.ProtectionHostAppGaussDBDWS,
    RouterUrl.ProtectionHostAppMySQL,
    RouterUrl.ProtectionHostAppPostgreSQL,
    RouterUrl.ProtectionHostAppKingBase,
    RouterUrl.ProtectionHostAppDB2,
    RouterUrl.ProtectionHostAppSQLServer,
    RouterUrl.ProtectionHostAppGBase,
    RouterUrl.ProtectionHostAppDameng,
    RouterUrl.ProtectionHostAppGaussDB,
    RouterUrl.ProtectionHostAppRedis,
    RouterUrl.ProtectionOpenGauss,
    RouterUrl.ProtectionDameng,
    RouterUrl.ProtectionHostAppMongoDB,
    RouterUrl.ProtectionHostAppSapHana,
    RouterUrl.ProtectionHostAppExchange,
    RouterUrl.ProtectionGbase,
    RouterUrl.ProtectionVirtualizationVmware,
    RouterUrl.ProtectionVirtualizationCnware,
    RouterUrl.ProtectionVirtualizationFusionCompute,
    RouterUrl.ProtectionVirtualizationFusionOne,
    RouterUrl.ProtectionVirtualizationFusionsphere,
    RouterUrl.ProtectionVirtualizationHyperV,
    RouterUrl.ProtectionVirtualizationH3cCas,
    RouterUrl.ProtectionVirtualizationKubernetes,
    RouterUrl.ProtectionVirtualizationKubernetesContainer,
    RouterUrl.ProtectionCloudHuaweiStack,
    RouterUrl.ProtectionCloudOpenstack,
    RouterUrl.ProtectionApsaraStack,
    RouterUrl.ProtectionBigDataHdfs,
    RouterUrl.ProtectionBigDataHbase,
    RouterUrl.ProtectionBigDataHive,
    RouterUrl.ProtectionBigDataElasticsearch,
    RouterUrl.ProtectionStorageDeviceInfo,
    RouterUrl.ProtectionDoradoFileSystem,
    RouterUrl.ProtectionLocalFileSystem,
    RouterUrl.ProtectionNasShared,
    RouterUrl.ProtectionObject,
    RouterUrl.ProtectionCommonShare,
    RouterUrl.ProtectionLimitRatePolicy,
    RouterUrl.ProtectionSla,
    RouterUrl.ProtectionHostAppGoldendb,
    RouterUrl.ProtectionHostAppInformix,
    RouterUrl.ProtectionHostAppOceanBase,
    RouterUrl.ProtectionHostAppClickHouse,
    RouterUrl.ProtectionHostAppGaussDBForOpengauss,
    RouterUrl.ProtectionHostApLightCloudGaussDB,
    RouterUrl.ProtectionHostAppTdsql,
    RouterUrl.ProtectionHostAppTidb,
    RouterUrl.ProtectionHostGeneralDatabase,
    RouterUrl.ExploreCopyData,
    RouterUrl.ExploreCopyDataApplication,
    RouterUrl.ExploreCopyDataActiveDirectory,
    RouterUrl.ExploreCopyDataDataBase,
    RouterUrl.ExploreCopyDataBigData,
    RouterUrl.ExploreCopyDataVirtualization,
    RouterUrl.ExploreCopyDataContainer,
    RouterUrl.ExploreCopyDataCloud,
    RouterUrl.ExploreCopyDataPrivateCloud,
    RouterUrl.ExploreCopyDataFileService,
    RouterUrl.ExploreCopyDataBareMetal,
    RouterUrl.ExploreCopyDataBareMetalFileset,
    RouterUrl.ExploreCopyDataHost,
    RouterUrl.ExploreCopyDataFileset,
    RouterUrl.ExploreCopyDataVolume,
    RouterUrl.ExploreCopyDataOracle,
    RouterUrl.ExploreCopyDataGaussDBT,
    RouterUrl.ExploreCopyDataGaussDBDWS,
    RouterUrl.ExploreCopyDataMySQL,
    RouterUrl.ExploreCopyDataOpenGauss,
    RouterUrl.ExploreCopyDataRedis,
    RouterUrl.ExploreCopyDataPostgreSQL,
    RouterUrl.ExploreCopyDataKingBase,
    RouterUrl.ExploreCopyDataOceanBase,
    RouterUrl.ExploreCopyDataDB2,
    RouterUrl.ExploreCopyDataSQLServer,
    RouterUrl.ExploreCopyDataGoldendb,
    RouterUrl.ExploreCopyDataInformix,
    RouterUrl.ExploreCopyDataTDSQL,
    RouterUrl.ExploreCopyDataTiDB,
    RouterUrl.ExploreCopyDataGeneralDatabase,
    RouterUrl.ExploreCopyDataDatabaseGbase,
    RouterUrl.ExploreCopyDataDatabaseExchange,
    RouterUrl.ExploreCopyDataVMware,
    RouterUrl.ExploreCopyDataCNware,
    RouterUrl.ExploreCopyDataKubernetes,
    RouterUrl.ExploreCopyDataKubernetesContainer,
    RouterUrl.ExploreCopyDataHyperv,
    RouterUrl.ExploreCopyDataHuaweiStack,
    RouterUrl.ExploreCopyDataOpenStack,
    RouterUrl.ExploreCopyDataApsaraStack,
    RouterUrl.ExploreCopyDataFusionCompute,
    RouterUrl.ExploreCopyDataFusionOne,
    RouterUrl.ExploreCopyDataHadoop,
    RouterUrl.ExploreCopyDataGaussDB,
    RouterUrl.ExploreCopyDataGBase,
    RouterUrl.ExploreCopyDataMongoDB,
    RouterUrl.ExploreCopyDataClickHouse,
    RouterUrl.ExploreCopyDataGaussdbForOpengauss,
    RouterUrl.ExploreCopyDataLightCloudGaussdb,
    RouterUrl.ExploreCopyDataH3CCas,
    RouterUrl.ExploreCopyDataDeviceInfo,
    RouterUrl.ExploreCopyDataFileSystem,
    RouterUrl.ExploreCopyDataNasShared,
    RouterUrl.ExploreCopyDataObject,
    RouterUrl.ExploreCopyDataCommonShare,
    RouterUrl.ExploreCopyDataHdfs,
    RouterUrl.ExploreCopyDataHbase,
    RouterUrl.ExportCopyDataDameng,
    RouterUrl.ExploreCopyDataHive,
    RouterUrl.ExploreCopyDataElasticsearch,
    RouterUrl.ExploreCopyDataSapHana,
    RouterUrl.ExploreLiveMount,
    RouterUrl.ExplorePolicyAntiPolicySetting,
    RouterUrl.ExplorePolicyAirgap,
    RouterUrl.ExploreLiveMountOracle,
    RouterUrl.ExploreLiveMountFileset,
    RouterUrl.ExploreLiveMountVolume,
    RouterUrl.ExploreLiveMountMysql,
    RouterUrl.ExploreLiveMountTdsql,
    RouterUrl.ExploreLiveMountExchange,
    RouterUrl.ExploreLiveMountApplication,
    RouterUrl.ExploreLiveMountApplicationFileset,
    RouterUrl.ExploreLiveMountApplicationVolume,
    RouterUrl.ExploreLiveMountApplicationOracle,
    RouterUrl.ExploreLiveMountApplicationMysql,
    RouterUrl.ExploreLiveMountApplicationTdsql,
    RouterUrl.ExploreLiveMountApplicationExchange,
    RouterUrl.ExploreLiveMountApplicationVmware,
    RouterUrl.ExploreLiveMountApplicationCnware,
    RouterUrl.ExploreLiveMountApplicationFileSystem,
    RouterUrl.ExploreLiveMountApplicationNasshare,
    RouterUrl.ExploreLiveMountVmware,
    RouterUrl.ExploreLiveMountCmware,
    RouterUrl.ExploreLiveMountFileSystem,
    RouterUrl.ExploreLiveMountNasShared,
    RouterUrl.ExploreDataDesensitization,
    RouterUrl.ExploreDataDesensitizationOracle,
    RouterUrl.ExploreDataDesensitizationMysql,
    RouterUrl.ExploreDataDesensitizationSqlServer,
    RouterUrl.ExplorePolicy,
    RouterUrl.ExplorePolicyMountUpdatePolicy,
    RouterUrl.ExplorePolicyDesensitizationPolicy,
    RouterUrl.ExploreRecoveryDrill,
    RouterUrl.ExploreCreateDrill,
    RouterUrl.ExploreDrillDetail,
    RouterUrl.ExploreModifyDrill,
    RouterUrl.ExploreDrillExecuteLog,
    RouterUrl.InsightAlarms,
    RouterUrl.InsightJobs,
    RouterUrl.InsightReports,
    RouterUrl.InsightPerformance,
    RouterUrl.SystemInfrastructure,
    RouterUrl.SystemInfrastructureClusterManagement,
    RouterUrl.SystemInfrastructureLocalStorage,
    RouterUrl.SystemInfrastructureExternalStorage,
    RouterUrl.SystemInfrastructureArchiveStorage,
    RouterUrl.SystemInfrastructureNasBackupStorage,
    RouterUrl.SystemInfrastructureHcsStorage,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.SystemSecurityUserQuota,
    RouterUrl.SystemSecuritySecurityPolicy,
    RouterUrl.SystemSecurityCertificate,
    RouterUrl.SystemSecurityKerberos,
    RouterUrl.SystemSamlSsoConfig,
    RouterUrl.SystemDataSecurity,
    RouterUrl.SystemSecurityLdap,
    RouterUrl.SystemLicense,
    RouterUrl.ExploreAntiRansomware,
    RouterUrl.ExploreAntiVmware,
    RouterUrl.ExploreAntiCNware,
    RouterUrl.ExploreAntiDoradoFileSystem,
    RouterUrl.ExploreAntiNasShared,
    RouterUrl.ExploreAntiFileset,
    RouterUrl.ExploreAntiHuaweiStack,
    RouterUrl.ExploreAntiApplication,
    RouterUrl.ExploreAntiApplicationVmware,
    RouterUrl.ExploreAntiApplicationCnware,
    RouterUrl.ExploreAntiApplicationDoradoFileSystem,
    RouterUrl.ExploreAntiApplicationNasShared,
    RouterUrl.ExploreAntiApplicationFileset,
    RouterUrl.SystemTagManagement,
    RouterUrl.LogManagement,
    RouterUrl.ExternalAssociatedSystems,
    RouterUrl.ExportQuery,
    RouterUrl.SystemSettings,
    RouterUrl.SystemSettingsSystemBackup,
    RouterUrl.SystemSettingsAlarmNotify,
    RouterUrl.SystemSettingsAlarmSettings,
    RouterUrl.SystemSettingsAlarmDump,
    RouterUrl.SystemSettingsSnmpTrap,
    RouterUrl.SystemDeviceTime,
    RouterUrl.SystemSettingConfigNetwork,
    RouterUrl.SftpService,
    RouterUrl.GlobalSearch,
    RouterUrl.hostTrustworthiness,
    RouterUrl.SystemSecurityAdfs
  ],
  [RoleType.DataProtectionAdmin]: [
    RouterUrl.Init,
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ProtectionResource,
    RouterUrl.ProtectionSummary,
    RouterUrl.ProtectionDatabase,
    RouterUrl.ProtectionBigData,
    RouterUrl.ProtectionVirtualization,
    RouterUrl.ProtectionContainer,
    RouterUrl.ProtectionCloud,
    RouterUrl.ProtectionPrivateCloud,
    RouterUrl.ProtectionFileService,
    RouterUrl.ProtectionApplication,
    RouterUrl.ProtectionActiveDirectory,
    RouterUrl.ProtectionBareMetal,
    RouterUrl.ProtectionBareMetalFilesetTemplate,
    RouterUrl.ProtectionHostAppHost,
    RouterUrl.ProtectionHostAppHostRegister,
    RouterUrl.ProtectionHostAppFilesetTemplate,
    RouterUrl.ProtectionHostAppVolume,
    RouterUrl.ProtectionHostAppOracle,
    RouterUrl.ProtectionHostAppGaussDBT,
    RouterUrl.ProtectionHostAppGaussDBDWS,
    RouterUrl.ProtectionHostAppMySQL,
    RouterUrl.ProtectionHostAppPostgreSQL,
    RouterUrl.ProtectionHostAppKingBase,
    RouterUrl.ProtectionHostAppDB2,
    RouterUrl.ProtectionHostAppSQLServer,
    RouterUrl.ProtectionHostAppGBase,
    RouterUrl.ProtectionHostAppDameng,
    RouterUrl.ProtectionHostAppGaussDB,
    RouterUrl.ProtectionHostAppRedis,
    RouterUrl.ProtectionHostAppMongoDB,
    RouterUrl.ProtectionOpenGauss,
    RouterUrl.ProtectionDameng,
    RouterUrl.ProtectionHostAppSapHana,
    RouterUrl.ProtectionHostAppExchange,
    RouterUrl.ProtectionGbase,
    RouterUrl.ProtectionVirtualizationVmware,
    RouterUrl.ProtectionVirtualizationCnware,
    RouterUrl.ProtectionVirtualizationFusionCompute,
    RouterUrl.ProtectionVirtualizationFusionOne,
    RouterUrl.ProtectionVirtualizationFusionsphere,
    RouterUrl.ProtectionVirtualizationHyperV,
    RouterUrl.ProtectionVirtualizationH3cCas,
    RouterUrl.ProtectionVirtualizationKubernetes,
    RouterUrl.ProtectionVirtualizationKubernetesContainer,
    RouterUrl.ProtectionCloudHuaweiStack,
    RouterUrl.ProtectionCloudOpenstack,
    RouterUrl.ProtectionApsaraStack,
    RouterUrl.ProtectionBigDataHdfs,
    RouterUrl.ProtectionBigDataHbase,
    RouterUrl.ProtectionBigDataHive,
    RouterUrl.ProtectionBigDataElasticsearch,
    RouterUrl.ProtectionStorageDeviceInfo,
    RouterUrl.ProtectionDoradoFileSystem,
    RouterUrl.ProtectionLocalFileSystem,
    RouterUrl.ProtectionNasShared,
    RouterUrl.ProtectionObject,
    RouterUrl.ProtectionCommonShare,
    RouterUrl.ProtectionLimitRatePolicy,
    RouterUrl.ProtectionSla,
    RouterUrl.ProtectionHostAppGoldendb,
    RouterUrl.ProtectionHostAppInformix,
    RouterUrl.ProtectionHostAppOceanBase,
    RouterUrl.ProtectionHostAppClickHouse,
    RouterUrl.ProtectionHostAppGaussDBForOpengauss,
    RouterUrl.ProtectionHostApLightCloudGaussDB,
    RouterUrl.ProtectionHostAppTdsql,
    RouterUrl.ProtectionHostAppTidb,
    RouterUrl.ProtectionHostGeneralDatabase,
    RouterUrl.ExploreCopyData,
    RouterUrl.ExploreCopyDataApplication,
    RouterUrl.ExploreCopyDataActiveDirectory,
    RouterUrl.ExploreCopyDataDataBase,
    RouterUrl.ExploreCopyDataBigData,
    RouterUrl.ExploreCopyDataVirtualization,
    RouterUrl.ExploreCopyDataContainer,
    RouterUrl.ExploreCopyDataCloud,
    RouterUrl.ExploreCopyDataPrivateCloud,
    RouterUrl.ExploreCopyDataFileService,
    RouterUrl.ExploreCopyDataBareMetal,
    RouterUrl.ExploreCopyDataBareMetalFileset,
    RouterUrl.ExploreCopyDataHost,
    RouterUrl.ExploreCopyDataFileset,
    RouterUrl.ExploreCopyDataVolume,
    RouterUrl.ExploreCopyDataOracle,
    RouterUrl.ExploreCopyDataGaussDBT,
    RouterUrl.ExploreCopyDataGaussDBDWS,
    RouterUrl.ExploreCopyDataMySQL,
    RouterUrl.ExploreCopyDataRedis,
    RouterUrl.ExploreCopyDataPostgreSQL,
    RouterUrl.ExploreCopyDataKingBase,
    RouterUrl.ExploreCopyDataOceanBase,
    RouterUrl.ExploreCopyDataDB2,
    RouterUrl.ExploreCopyDataSQLServer,
    RouterUrl.ExploreCopyDataGoldendb,
    RouterUrl.ExploreCopyDataInformix,
    RouterUrl.ExploreCopyDataTDSQL,
    RouterUrl.ExploreCopyDataTiDB,
    RouterUrl.ExploreCopyDataGeneralDatabase,
    RouterUrl.ExploreCopyDataDatabaseGbase,
    RouterUrl.ExploreCopyDataDatabaseExchange,
    RouterUrl.ExportCopyDataDameng,
    RouterUrl.ExploreCopyDataVMware,
    RouterUrl.ExploreCopyDataCNware,
    RouterUrl.ExploreCopyDataKubernetes,
    RouterUrl.ExploreCopyDataKubernetesContainer,
    RouterUrl.ExploreCopyDataHyperv,
    RouterUrl.ExploreCopyDataHuaweiStack,
    RouterUrl.ExploreCopyDataOpenStack,
    RouterUrl.ExploreCopyDataApsaraStack,
    RouterUrl.ExploreCopyDataFusionCompute,
    RouterUrl.ExploreCopyDataFusionOne,
    RouterUrl.ExploreCopyDataHadoop,
    RouterUrl.ExploreCopyDataGaussDB,
    RouterUrl.ExploreCopyDataGBase,
    RouterUrl.ExploreCopyDataMongoDB,
    RouterUrl.ExploreCopyDataClickHouse,
    RouterUrl.ExploreCopyDataGaussdbForOpengauss,
    RouterUrl.ExploreCopyDataLightCloudGaussdb,
    RouterUrl.ExploreCopyDataH3CCas,
    RouterUrl.ExploreCopyDataDeviceInfo,
    RouterUrl.ExploreCopyDataFileSystem,
    RouterUrl.ExploreCopyDataNasShared,
    RouterUrl.ExploreCopyDataObject,
    RouterUrl.ExploreCopyDataCommonShare,
    RouterUrl.ExploreCopyDataHdfs,
    RouterUrl.ExploreCopyDataHbase,
    RouterUrl.ExploreCopyDataOpenGauss,
    RouterUrl.ExploreCopyDataHive,
    RouterUrl.ExploreCopyDataElasticsearch,
    RouterUrl.ExploreCopyDataSapHana,
    RouterUrl.ExploreLiveMount,
    RouterUrl.ExploreLiveMountOracle,
    RouterUrl.ExploreLiveMountFileset,
    RouterUrl.ExploreLiveMountVolume,
    RouterUrl.ExploreLiveMountMysql,
    RouterUrl.ExploreLiveMountTdsql,
    RouterUrl.ExploreLiveMountExchange,
    RouterUrl.ExploreLiveMountApplication,
    RouterUrl.ExploreLiveMountApplicationFileset,
    RouterUrl.ExploreLiveMountApplicationVolume,
    RouterUrl.ExploreLiveMountApplicationOracle,
    RouterUrl.ExploreLiveMountApplicationMysql,
    RouterUrl.ExploreLiveMountApplicationTdsql,
    RouterUrl.ExploreLiveMountApplicationExchange,
    RouterUrl.ExploreLiveMountApplicationVmware,
    RouterUrl.ExploreLiveMountApplicationCnware,
    RouterUrl.ExploreLiveMountApplicationFileSystem,
    RouterUrl.ExploreLiveMountApplicationNasshare,
    RouterUrl.ExploreAntiRansomware,
    RouterUrl.ExplorePolicyAntiPolicySetting,
    RouterUrl.ExplorePolicyAirgap,
    RouterUrl.ExploreAntiVmware,
    RouterUrl.ExploreAntiCNware,
    RouterUrl.ExploreAntiDoradoFileSystem,
    RouterUrl.ExploreAntiNasShared,
    RouterUrl.ExploreAntiFileset,
    RouterUrl.ExploreAntiHuaweiStack,
    RouterUrl.ExploreAntiApplication,
    RouterUrl.ExploreAntiApplicationVmware,
    RouterUrl.ExploreAntiApplicationCnware,
    RouterUrl.ExploreAntiApplicationDoradoFileSystem,
    RouterUrl.ExploreAntiApplicationNasShared,
    RouterUrl.ExploreAntiApplicationFileset,
    RouterUrl.ExploreLiveMountVmware,
    RouterUrl.ExploreLiveMountCmware,
    RouterUrl.ExploreLiveMountFileSystem,
    RouterUrl.ExploreLiveMountNasShared,
    RouterUrl.ExploreDataDesensitization,
    RouterUrl.ExploreDataDesensitizationOracle,
    RouterUrl.ExploreDataDesensitizationMysql,
    RouterUrl.ExploreDataDesensitizationSqlServer,
    RouterUrl.ExplorePolicy,
    RouterUrl.ExplorePolicyMountUpdatePolicy,
    RouterUrl.ExplorePolicyDesensitizationPolicy,
    RouterUrl.ExploreRecoveryDrill,
    RouterUrl.ExploreCreateDrill,
    RouterUrl.ExploreDrillDetail,
    RouterUrl.ExploreModifyDrill,
    RouterUrl.ExploreDrillExecuteLog,
    RouterUrl.InsightAlarms,
    RouterUrl.InsightJobs,
    RouterUrl.InsightReports,
    RouterUrl.InsightPerformance,
    RouterUrl.SystemInfrastructureClusterManagement,
    RouterUrl.SystemInfrastructureArchiveStorage,
    RouterUrl.SystemInfrastructureNasBackupStorage,
    RouterUrl.SystemInfrastructureHcsStorage,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.SystemSecurityKerberos,
    RouterUrl.ExportQuery,
    RouterUrl.SystemSecurityUserQuota,
    RouterUrl.GlobalSearch,
    RouterUrl.SystemDeviceTime,
    RouterUrl.SystemTagManagement
  ],
  [RoleType.Auditor]: [
    RouterUrl.Init,
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ProtectionResource,
    RouterUrl.ProtectionSummary,
    RouterUrl.ProtectionDatabase,
    RouterUrl.ProtectionBigData,
    RouterUrl.ProtectionVirtualization,
    RouterUrl.ProtectionContainer,
    RouterUrl.ProtectionCloud,
    RouterUrl.ProtectionPrivateCloud,
    RouterUrl.ProtectionFileService,
    RouterUrl.ProtectionApplication,
    RouterUrl.ProtectionActiveDirectory,
    RouterUrl.ProtectionBareMetal,
    RouterUrl.ProtectionBareMetalFilesetTemplate,
    RouterUrl.ProtectionHostAppHost,
    RouterUrl.ProtectionHostAppHostRegister,
    RouterUrl.ProtectionHostAppFilesetTemplate,
    RouterUrl.ProtectionHostAppVolume,
    RouterUrl.ProtectionHostAppOracle,
    RouterUrl.ProtectionHostAppGaussDBT,
    RouterUrl.ProtectionHostAppGaussDBDWS,
    RouterUrl.ProtectionHostAppMySQL,
    RouterUrl.ProtectionHostAppDB2,
    RouterUrl.ProtectionHostAppSQLServer,
    RouterUrl.ProtectionHostAppGBase,
    RouterUrl.ProtectionHostAppDameng,
    RouterUrl.ProtectionHostAppGaussDB,
    RouterUrl.ProtectionHostAppPostgreSQL,
    RouterUrl.ProtectionHostAppKingBase,
    RouterUrl.ProtectionHostAppRedis,
    RouterUrl.ProtectionOpenGauss,
    RouterUrl.ProtectionDameng,
    RouterUrl.ProtectionHostAppMongoDB,
    RouterUrl.ProtectionHostAppSapHana,
    RouterUrl.ProtectionHostAppExchange,
    RouterUrl.ProtectionGbase,
    RouterUrl.ProtectionVirtualizationVmware,
    RouterUrl.ProtectionVirtualizationCnware,
    RouterUrl.ProtectionVirtualizationFusionCompute,
    RouterUrl.ProtectionVirtualizationFusionOne,
    RouterUrl.ProtectionVirtualizationFusionsphere,
    RouterUrl.ProtectionVirtualizationHyperV,
    RouterUrl.ProtectionVirtualizationH3cCas,
    RouterUrl.ProtectionVirtualizationKubernetes,
    RouterUrl.ProtectionVirtualizationKubernetesContainer,
    RouterUrl.ProtectionCloudHuaweiStack,
    RouterUrl.ProtectionCloudOpenstack,
    RouterUrl.ProtectionApsaraStack,
    RouterUrl.ProtectionBigDataHdfs,
    RouterUrl.ProtectionBigDataHbase,
    RouterUrl.ProtectionBigDataHive,
    RouterUrl.ProtectionBigDataElasticsearch,
    RouterUrl.ProtectionStorageDeviceInfo,
    RouterUrl.ProtectionDoradoFileSystem,
    RouterUrl.ProtectionLocalFileSystem,
    RouterUrl.ProtectionNasShared,
    RouterUrl.ProtectionObject,
    RouterUrl.ProtectionCommonShare,
    RouterUrl.ProtectionLimitRatePolicy,
    RouterUrl.ProtectionSla,
    RouterUrl.ProtectionHostAppGoldendb,
    RouterUrl.ProtectionHostAppInformix,
    RouterUrl.ProtectionHostAppOceanBase,
    RouterUrl.ProtectionHostAppTidb,
    RouterUrl.ProtectionHostAppClickHouse,
    RouterUrl.ProtectionHostAppGaussDBForOpengauss,
    RouterUrl.ProtectionHostApLightCloudGaussDB,
    RouterUrl.ProtectionHostAppTdsql,
    RouterUrl.ProtectionHostGeneralDatabase,
    RouterUrl.ExploreCopyData,
    RouterUrl.ExploreCopyDataApplication,
    RouterUrl.ExploreCopyDataActiveDirectory,
    RouterUrl.ExploreCopyDataDataBase,
    RouterUrl.ExploreCopyDataBigData,
    RouterUrl.ExploreCopyDataVirtualization,
    RouterUrl.ExploreCopyDataContainer,
    RouterUrl.ExploreCopyDataCloud,
    RouterUrl.ExploreCopyDataPrivateCloud,
    RouterUrl.ExploreCopyDataFileService,
    RouterUrl.ExploreCopyDataBareMetal,
    RouterUrl.ExploreCopyDataBareMetalFileset,
    RouterUrl.ExploreCopyDataHost,
    RouterUrl.ExploreCopyDataFileset,
    RouterUrl.ExploreCopyDataVolume,
    RouterUrl.ExploreCopyDataOracle,
    RouterUrl.ExploreCopyDataGaussDBT,
    RouterUrl.ExploreCopyDataGaussDBDWS,
    RouterUrl.ExploreCopyDataMySQL,
    RouterUrl.ExploreCopyDataOpenGauss,
    RouterUrl.ExploreCopyDataRedis,
    RouterUrl.ExploreCopyDataPostgreSQL,
    RouterUrl.ExploreCopyDataKingBase,
    RouterUrl.ExploreCopyDataOceanBase,
    RouterUrl.ExploreCopyDataDB2,
    RouterUrl.ExploreCopyDataSQLServer,
    RouterUrl.ExploreCopyDataGoldendb,
    RouterUrl.ExploreCopyDataInformix,
    RouterUrl.ExploreCopyDataTDSQL,
    RouterUrl.ExploreCopyDataTiDB,
    RouterUrl.ExploreCopyDataGeneralDatabase,
    RouterUrl.ExploreCopyDataDatabaseGbase,
    RouterUrl.ExploreCopyDataDatabaseExchange,
    RouterUrl.ExploreCopyDataVMware,
    RouterUrl.ExploreCopyDataCNware,
    RouterUrl.ExploreCopyDataKubernetes,
    RouterUrl.ExploreCopyDataKubernetesContainer,
    RouterUrl.ExploreCopyDataHyperv,
    RouterUrl.ExportCopyDataDameng,
    RouterUrl.ExploreCopyDataHuaweiStack,
    RouterUrl.ExploreCopyDataOpenStack,
    RouterUrl.ExploreCopyDataApsaraStack,
    RouterUrl.ExploreCopyDataFusionCompute,
    RouterUrl.ExploreCopyDataFusionOne,
    RouterUrl.ExploreCopyDataHadoop,
    RouterUrl.ExploreCopyDataGaussDB,
    RouterUrl.ExploreCopyDataLightCloudGaussdb,
    RouterUrl.ExploreCopyDataGBase,
    RouterUrl.ExploreCopyDataMongoDB,
    RouterUrl.ExploreCopyDataH3CCas,
    RouterUrl.ExploreCopyDataDeviceInfo,
    RouterUrl.ExploreCopyDataFileSystem,
    RouterUrl.ExploreCopyDataNasShared,
    RouterUrl.ExploreCopyDataObject,
    RouterUrl.ExploreCopyDataCommonShare,
    RouterUrl.ExploreCopyDataHdfs,
    RouterUrl.ExploreCopyDataHbase,
    RouterUrl.ExploreCopyDataHive,
    RouterUrl.ExploreCopyDataElasticsearch,
    RouterUrl.ExploreCopyDataSapHana,
    RouterUrl.ExplorePolicyAntiPolicySetting,
    RouterUrl.ExplorePolicyAirgap,
    RouterUrl.ExploreRansomwareOverview,
    RouterUrl.ExploreRansomwareVMware,
    RouterUrl.ExploreRansomwareDoradoFileSystem,
    RouterUrl.ExploreRansomwareNasShared,
    RouterUrl.ExploreLiveMount,
    RouterUrl.ExploreLiveMountOracle,
    RouterUrl.ExploreLiveMountFileset,
    RouterUrl.ExploreLiveMountVolume,
    RouterUrl.ExploreLiveMountMysql,
    RouterUrl.ExploreLiveMountTdsql,
    RouterUrl.ExploreLiveMountExchange,
    RouterUrl.ExploreLiveMountApplication,
    RouterUrl.ExploreLiveMountApplicationFileset,
    RouterUrl.ExploreLiveMountApplicationVolume,
    RouterUrl.ExploreLiveMountApplicationOracle,
    RouterUrl.ExploreLiveMountApplicationMysql,
    RouterUrl.ExploreLiveMountApplicationTdsql,
    RouterUrl.ExploreLiveMountApplicationExchange,
    RouterUrl.ExploreLiveMountApplicationVmware,
    RouterUrl.ExploreLiveMountApplicationCnware,
    RouterUrl.ExploreLiveMountApplicationFileSystem,
    RouterUrl.ExploreLiveMountApplicationNasshare,
    RouterUrl.ExploreLiveMountVmware,
    RouterUrl.ExploreLiveMountCmware,
    RouterUrl.ExploreLiveMountFileSystem,
    RouterUrl.ExploreLiveMountNasShared,
    RouterUrl.ExploreDataDesensitization,
    RouterUrl.ExploreDataDesensitizationOracle,
    RouterUrl.ExplorePolicy,
    RouterUrl.ExplorePolicyMountUpdatePolicy,
    RouterUrl.ExplorePolicyDesensitizationPolicy,
    RouterUrl.ExploreRecoveryDrill,
    RouterUrl.ExploreCreateDrill,
    RouterUrl.ExploreDrillDetail,
    RouterUrl.ExploreModifyDrill,
    RouterUrl.ExploreDrillExecuteLog,
    RouterUrl.InsightAlarms,
    RouterUrl.InsightJobs,
    RouterUrl.InsightReports,
    RouterUrl.InsightPerformance,
    RouterUrl.SystemInfrastructure,
    RouterUrl.SystemInfrastructureClusterManagement,
    RouterUrl.SystemInfrastructureLocalStorage,
    RouterUrl.SystemInfrastructureExternalStorage,
    RouterUrl.SystemInfrastructureArchiveStorage,
    RouterUrl.SystemInfrastructureNasBackupStorage,
    RouterUrl.SystemInfrastructureHcsStorage,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.SystemSecurityUserQuota,
    RouterUrl.SystemSecuritySecurityPolicy,
    RouterUrl.SystemSecurityCertificate,
    RouterUrl.SystemSecurityKerberos,
    RouterUrl.SystemDataSecurity,
    RouterUrl.hostTrustworthiness,
    RouterUrl.SystemSecurityLdap,
    RouterUrl.SystemSamlSsoConfig,
    RouterUrl.SystemLicense,
    RouterUrl.ExploreAntiRansomware,
    RouterUrl.ExploreAntiVmware,
    RouterUrl.ExploreAntiCNware,
    RouterUrl.ExploreAntiDoradoFileSystem,
    RouterUrl.ExploreAntiNasShared,
    RouterUrl.ExploreAntiFileset,
    RouterUrl.ExploreAntiHuaweiStack,
    RouterUrl.ExploreAntiApplication,
    RouterUrl.ExploreAntiApplicationVmware,
    RouterUrl.ExploreAntiApplicationCnware,
    RouterUrl.ExploreAntiApplicationDoradoFileSystem,
    RouterUrl.ExploreAntiApplicationNasShared,
    RouterUrl.ExploreAntiApplicationFileset,
    RouterUrl.SystemTagManagement,
    RouterUrl.LogManagement,
    RouterUrl.ExternalAssociatedSystems,
    RouterUrl.ExportQuery,
    RouterUrl.SystemSettings,
    RouterUrl.SystemSettingsSystemBackup,
    RouterUrl.SystemSettingsAlarmNotify,
    RouterUrl.SystemSettingsAlarmSettings,
    RouterUrl.SystemSettingsAlarmDump,
    RouterUrl.SystemDeviceTime,
    RouterUrl.SftpService,
    RouterUrl.GlobalSearch,
    RouterUrl.SystemSecurityAdfs
  ]
};

export const URL_PERMISSION_CLOUD_BACKUP = {
  [RoleType.Null]: [],
  [RoleType.SysAdmin]: [
    RouterUrl.Init,
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ProtectionLocalFileSystem,
    RouterUrl.ProtectionLimitRatePolicy,
    RouterUrl.ProtectionSla,
    RouterUrl.ExploreCopyData,
    RouterUrl.ExploreCopyLocalFileSystem,
    RouterUrl.InsightAlarms,
    RouterUrl.ExploreRansomwareCloudBackupOverview,
    RouterUrl.ExploreRansomwareDetectionSetting,
    RouterUrl.ExploreRansomwareBlockingRuleList,
    RouterUrl.ExploreRansomwareDetectionModelList,
    RouterUrl.ExploreRansomwareLocalFileSystem,
    RouterUrl.InsightJobs,
    RouterUrl.InsightPerformance,
    RouterUrl.SystemInfrastructure,
    RouterUrl.SystemInfrastructureLocalStorage,
    RouterUrl.SystemInfrastructureBackupStorage,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.SystemSecuritySecurityPolicy,
    RouterUrl.SystemSecurityCertificate,
    RouterUrl.LogManagement,
    RouterUrl.ExternalAssociatedSystems,
    RouterUrl.ExportQuery,
    RouterUrl.SystemSettings,
    RouterUrl.SystemSettingsSystemBackup,
    RouterUrl.SystemSettingsAlarmNotify,
    RouterUrl.SystemSettingsAlarmSettings,
    RouterUrl.SystemSettingsAlarmDump,
    RouterUrl.SystemSettingsSnmpTrap,
    RouterUrl.SystemSettingConfigNetwork,
    RouterUrl.GlobalSearch
  ],
  [RoleType.DataProtectionAdmin]: [
    RouterUrl.Init,
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ProtectionLocalFileSystem,
    RouterUrl.ProtectionLimitRatePolicy,
    RouterUrl.ProtectionSla,
    RouterUrl.ExploreCopyData,
    RouterUrl.ExploreCopyLocalFileSystem,
    RouterUrl.ExploreRansomwareCloudBackupOverview,
    RouterUrl.ExploreRansomwareDetectionSetting,
    RouterUrl.ExploreRansomwareBlockingRuleList,
    RouterUrl.ExploreRansomwareDetectionModelList,
    RouterUrl.ExploreRansomwareLocalFileSystem,
    RouterUrl.InsightAlarms,
    RouterUrl.InsightJobs,
    RouterUrl.InsightPerformance,
    RouterUrl.SystemInfrastructure,
    RouterUrl.SystemInfrastructureLocalStorage,
    RouterUrl.SystemInfrastructureBackupStorage,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.SystemSecuritySecurityPolicy,
    RouterUrl.SystemSecurityCertificate,
    RouterUrl.LogManagement,
    RouterUrl.ExternalAssociatedSystems,
    RouterUrl.ExportQuery,
    RouterUrl.SystemSettings,
    RouterUrl.SystemSettingsSystemBackup,
    RouterUrl.SystemSettingsAlarmNotify,
    RouterUrl.SystemSettingsAlarmSettings,
    RouterUrl.SystemSettingsAlarmDump,
    RouterUrl.SystemSettingsSnmpTrap,
    RouterUrl.GlobalSearch
  ],
  [RoleType.Auditor]: [
    RouterUrl.Init,
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ProtectionLocalFileSystem,
    RouterUrl.ProtectionLimitRatePolicy,
    RouterUrl.ProtectionSla,
    RouterUrl.ExploreCopyData,
    RouterUrl.ExploreCopyLocalFileSystem,
    RouterUrl.ExploreRansomwareCloudBackupOverview,
    RouterUrl.ExploreRansomwareDetectionSetting,
    RouterUrl.ExploreRansomwareBlockingRuleList,
    RouterUrl.ExploreRansomwareDetectionModelList,
    RouterUrl.ExploreRansomwareLocalFileSystem,
    RouterUrl.InsightAlarms,
    RouterUrl.InsightJobs,
    RouterUrl.InsightPerformance,
    RouterUrl.SystemInfrastructure,
    RouterUrl.SystemInfrastructureLocalStorage,
    RouterUrl.SystemInfrastructureBackupStorage,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.SystemSecuritySecurityPolicy,
    RouterUrl.SystemSecurityCertificate,
    RouterUrl.LogManagement,
    RouterUrl.ExternalAssociatedSystems,
    RouterUrl.ExportQuery,
    RouterUrl.SystemSettings,
    RouterUrl.SystemSettingsSystemBackup,
    RouterUrl.SystemSettingsAlarmNotify,
    RouterUrl.SystemSettingsAlarmSettings,
    RouterUrl.SystemSettingsAlarmDump,
    RouterUrl.SystemSettingsSnmpTrap,
    RouterUrl.GlobalSearch
  ]
};

export const URL_PERMISSION_HYPER_DETECT = {
  [RoleType.Null]: [],
  [RoleType.SysAdmin]: [
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ProtectionLocalFileSystem,
    RouterUrl.ProtectionLocalResource,
    RouterUrl.ProtectionSla,
    RouterUrl.InsightAlarms,
    RouterUrl.ExploreRansomwareCloudBackupOverview,
    RouterUrl.ExploreRansomwareDetectionSetting,
    RouterUrl.ExploreRansomwareBlockingRuleList,
    RouterUrl.ExploreRansomwareDetectionModelList,
    RouterUrl.ExploreRansomwareRealtimeDetection,
    RouterUrl.ExploreRansomwareLocalFileSystem,
    RouterUrl.InsightJobs,
    RouterUrl.SystemInfrastructure,
    RouterUrl.SystemInfrastructureLocalStorage,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.SystemSecuritySecurityPolicy,
    RouterUrl.SystemSecurityCertificate,
    RouterUrl.LogManagement,
    RouterUrl.ExternalAssociatedSystems,
    RouterUrl.ExportQuery,
    RouterUrl.SystemSettings,
    RouterUrl.SystemSettingsAlarmNotify,
    RouterUrl.SystemSettingsAlarmSettings,
    RouterUrl.SystemSettingsAlarmDump,
    RouterUrl.SystemSettingsSnmpTrap,
    RouterUrl.GlobalSearch
  ],
  [RoleType.DataProtectionAdmin]: [
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ProtectionLocalFileSystem,
    RouterUrl.ProtectionLocalResource,
    RouterUrl.ProtectionSla,
    RouterUrl.ExploreRansomwareCloudBackupOverview,
    RouterUrl.ExploreRansomwareDetectionSetting,
    RouterUrl.ExploreRansomwareBlockingRuleList,
    RouterUrl.ExploreRansomwareDetectionModelList,
    RouterUrl.ExploreRansomwareRealtimeDetection,
    RouterUrl.ExploreRansomwareLocalFileSystem,
    RouterUrl.InsightAlarms,
    RouterUrl.InsightJobs,
    RouterUrl.SystemInfrastructure,
    RouterUrl.SystemInfrastructureLocalStorage,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.SystemSecuritySecurityPolicy,
    RouterUrl.SystemSecurityCertificate,
    RouterUrl.LogManagement,
    RouterUrl.ExternalAssociatedSystems,
    RouterUrl.ExportQuery,
    RouterUrl.SystemSettings,
    RouterUrl.SystemSettingsAlarmNotify,
    RouterUrl.SystemSettingsAlarmSettings,
    RouterUrl.SystemSettingsAlarmDump,
    RouterUrl.SystemSettingsSnmpTrap,
    RouterUrl.GlobalSearch
  ],
  [RoleType.Auditor]: [
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ProtectionLocalFileSystem,
    RouterUrl.ProtectionLocalResource,
    RouterUrl.ProtectionSla,
    RouterUrl.ExploreRansomwareCloudBackupOverview,
    RouterUrl.ExploreRansomwareDetectionSetting,
    RouterUrl.ExploreRansomwareBlockingRuleList,
    RouterUrl.ExploreRansomwareDetectionModelList,
    RouterUrl.ExploreRansomwareRealtimeDetection,
    RouterUrl.ExploreRansomwareLocalFileSystem,
    RouterUrl.InsightAlarms,
    RouterUrl.InsightJobs,
    RouterUrl.SystemInfrastructure,
    RouterUrl.SystemInfrastructureLocalStorage,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.SystemSecuritySecurityPolicy,
    RouterUrl.SystemSecurityCertificate,
    RouterUrl.LogManagement,
    RouterUrl.ExternalAssociatedSystems,
    RouterUrl.ExportQuery,
    RouterUrl.SystemSettings,
    RouterUrl.SystemSettingsAlarmNotify,
    RouterUrl.SystemSettingsAlarmSettings,
    RouterUrl.SystemSettingsAlarmDump,
    RouterUrl.SystemSettingsSnmpTrap,
    RouterUrl.GlobalSearch
  ]
};

export const URL_PERMISSION_CYBER_ENGINE = {
  [RoleType.Null]: [],
  [RoleType.SysAdmin]: [
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ReportDetail,
    RouterUrl.ExploreStorageDevice,
    RouterUrl.ExploreAntiRansomwareProtection,
    RouterUrl.ExploreAntiRansomwareProtectionFileInterception,
    RouterUrl.ExploreAntiRansomwareProtectionFileBlock,
    RouterUrl.ExploreAntiRansomwareProtectionDataBackup,
    RouterUrl.ExploreAntiRansomwareProtectionRealTimeDetection,
    RouterUrl.ExploreAntiRansomwareProtectionIntellIgent,
    RouterUrl.ExploreAntiRansomwareProtectionModel,
    RouterUrl.ExploreSnapShotData,
    RouterUrl.ExploreDetectionReport,
    RouterUrl.InsightAlarms,
    RouterUrl.InsightJobs,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.SystemSecuritySecurityPolicy,
    RouterUrl.SystemSecurityCertificate,
    RouterUrl.LogManagement,
    RouterUrl.ExternalAssociatedSystems,
    RouterUrl.ExportQuery,
    RouterUrl.SystemSettings,
    RouterUrl.SystemLicense,
    RouterUrl.ExplorePolicyAirgap,
    RouterUrl.SystemSettingsSystemBackup,
    RouterUrl.SystemSettingsAlarmNotify,
    RouterUrl.SystemSettingsAlarmNotifySettings,
    RouterUrl.SystemSettingsAlarmDump,
    RouterUrl.SystemNetworkConfig,
    RouterUrl.SystemSettingsSnmpTrap,
    RouterUrl.IbmcService,
    RouterUrl.SystemDeviceTime,
    RouterUrl.SystemSecurityLdap
  ],
  [RoleType.DataProtectionAdmin]: [
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ReportDetail,
    RouterUrl.ExploreStorageDevice,
    RouterUrl.ExploreAntiRansomwareProtection,
    RouterUrl.ExploreAntiRansomwareProtectionFileInterception,
    RouterUrl.ExploreAntiRansomwareProtectionFileBlock,
    RouterUrl.ExploreAntiRansomwareProtectionDataBackup,
    RouterUrl.ExploreAntiRansomwareProtectionRealTimeDetection,
    RouterUrl.ExploreAntiRansomwareProtectionIntellIgent,
    RouterUrl.ExploreAntiRansomwareProtectionModel,
    RouterUrl.ExploreSnapShotData,
    RouterUrl.ExploreDetectionReport,
    RouterUrl.ExplorePolicyAirgap,
    RouterUrl.InsightAlarms,
    RouterUrl.InsightJobs,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.ExportQuery
  ],
  [RoleType.Auditor]: [
    RouterUrl.Home,
    RouterUrl.ErrorPage,
    RouterUrl.ReportDetail,
    RouterUrl.ExploreStorageDevice,
    RouterUrl.ExploreAntiRansomwareProtection,
    RouterUrl.ExploreAntiRansomwareProtectionFileInterception,
    RouterUrl.ExploreAntiRansomwareProtectionFileBlock,
    RouterUrl.ExploreAntiRansomwareProtectionDataBackup,
    RouterUrl.ExploreAntiRansomwareProtectionRealTimeDetection,
    RouterUrl.ExploreAntiRansomwareProtectionIntellIgent,
    RouterUrl.ExploreAntiRansomwareProtectionModel,
    RouterUrl.ExploreSnapShotData,
    RouterUrl.ExploreDetectionReport,
    RouterUrl.InsightAlarms,
    RouterUrl.InsightJobs,
    RouterUrl.SystemSecurity,
    RouterUrl.SystemSecurityRbac,
    RouterUrl.SystemSecurityUserrole,
    RouterUrl.SystemSecuritySecurityPolicy,
    RouterUrl.SystemSecurityCertificate,
    RouterUrl.LogManagement,
    RouterUrl.ExternalAssociatedSystems,
    RouterUrl.ExportQuery,
    RouterUrl.SystemSettings,
    RouterUrl.SystemLicense,
    RouterUrl.ExplorePolicyAirgap,
    RouterUrl.SystemSettingsSystemBackup,
    RouterUrl.SystemSettingsAlarmNotify,
    RouterUrl.SystemSettingsAlarmNotifySettings,
    RouterUrl.SystemSettingsAlarmSettings,
    RouterUrl.SystemSettingsAlarmDump,
    RouterUrl.SystemNetworkConfig,
    RouterUrl.SystemSettingsSnmpTrap,
    RouterUrl.IbmcService,
    RouterUrl.SystemDeviceTime,
    RouterUrl.SystemSecurityLdap
  ]
};

/**
 * 操作项权限
 */
export const OPERATE_PERMISSION = {
  [RoleType.Null]: [],
  // 系统管理员
  [RoleType.SysAdmin]: [
    OperateItems.SysadminOnly,
    OperateItems.ModifyHost,
    OperateItems.RemoveHost,
    OperateItems.ProtectHosts,
    OperateItems.ModifyHostProtection,
    OperateItems.RemoveHostProtection,
    OperateItems.ManuallyBackHost,
    OperateItems.ManuallyArchiveHost,
    OperateItems.ActivateHostProtection,
    OperateItems.DeactivateHostProtection,
    OperateItems.SynchTrapInfo,
    OperateItems.CreateHostFileset,
    OperateItems.DeleteHostFileset,
    OperateItems.ModifyHostFileset,
    OperateItems.ProtectHostFileset,
    OperateItems.ModifyFilesetProtection,
    OperateItems.RemoveFilesetProtection,
    OperateItems.ManuallyBackFileset,
    OperateItems.ManuallyArchiveFileset,
    OperateItems.ManuallyReplicationFileset,
    OperateItems.ActivateFilesetProtection,
    OperateItems.DeactivateFilesetProtection,
    OperateItems.RegisterDatabase,
    OperateItems.ProtectDatabases,
    OperateItems.ModifyDatabaseProtection,
    OperateItems.RemoveDatabaseProtection,
    OperateItems.ManuallyBackDatabase,
    OperateItems.ManuallyDatabaseProtection,
    OperateItems.ManuallyArchiveDatabase,
    OperateItems.ManuallyReplicationDatabase,
    OperateItems.ActivateDatabaseProtection,
    OperateItems.DeactivateDatabseProtection,
    OperateItems.ModifyDatabaseAuth,
    OperateItems.CreatingCluster,
    OperateItems.Deletingcluster,
    OperateItems.ModifyingCluster,
    OperateItems.ModifyingASMAuth,
    OperateItems.ResourceAuth,
    OperateItems.ResourceReclaiming,
    OperateItems.DeleteDatabase,
    OperateItems.RegisterApplication,
    OperateItems.ProtectApplication,
    OperateItems.ModifyApplicationProtection,
    OperateItems.RemoveApplicationProtection,
    OperateItems.ManuallyApplicationProtection,
    OperateItems.ManuallyArchiveApplication,
    OperateItems.ManuallyReplicationApplication,
    OperateItems.ActivateApplicationProtection,
    OperateItems.DeactivateApplicationProtection,
    OperateItems.RegisterVirtualizationPlatform,
    OperateItems.ProtectVM,
    OperateItems.ProtectTENANT,
    OperateItems.RemoveVMProtection,
    OperateItems.RemoveFCProtection,
    OperateItems.ModifyVMProtection,
    OperateItems.ManuallyBackVM,
    OperateItems.ScanHCSProject,
    OperateItems.ManuallyVMProtection,
    OperateItems.ManuallyArchiveVM,
    OperateItems.ManuallyReplicationVM,
    OperateItems.ActivateVMProtection,
    OperateItems.DeactivateVMProtection,
    OperateItems.ActivateFCProtection,
    OperateItems.DeactivateFCProtection,
    OperateItems.DeregisterVirtualizationPlatform,
    OperateItems.ModifyVirtualizationRegister,
    OperateItems.RescanVirtualizationPlatform,
    OperateItems.HCSEnvironmentInfo,
    OperateItems.RegisterBigDataCluster,
    OperateItems.DeregisterBigDataCluster,
    OperateItems.ModifyBigDataCluster,
    OperateItems.CreatingBigDataFileset,
    OperateItems.DeletingBigDataFileset,
    OperateItems.ModifyBigDataFileset,
    OperateItems.ProtectBigDataFilesets,
    OperateItems.ModifyBigDataFilesetProtection,
    OperateItems.RemoveBigDataFilesetProtection,
    OperateItems.ManuallyBackingBigDataFileset,
    OperateItems.ManuallyArchivingBigDataFileset,
    OperateItems.ManuallyReplicationBigDataFileset,
    OperateItems.ActivateBigDataFilesetProtection,
    OperateItems.DeactivatingBigDataFilesetProtection,
    OperateItems.QueryingCloudPlatform,
    OperateItems.RegisteringCloudPlatform,
    OperateItems.RegisteringTenant,
    OperateItems.CreateSLA,
    OperateItems.ModifySLA,
    OperateItems.DeleteSLA,
    OperateItems.CloneSLA,
    OperateItems.CreateQos,
    OperateItems.ModifyQos,
    OperateItems.DeleteQos,
    OperateItems.CreateKubernetesRule,
    OperateItems.ModifyKubernetesRule,
    OperateItems.DeleteKubernetesRule,
    OperateItems.ModifyingCopyRetentionPolicy,
    OperateItems.CopyDuplicate,
    OperateItems.DeletingCopy,
    OperateItems.DownloadCopy,
    OperateItems.RestoreCopy,
    OperateItems.InstanceRecovery,
    OperateItems.MountingCopy,
    OperateItems.CreatingLiveMount,
    OperateItems.DestroyLiveMount,
    OperateItems.UpdateLatestCopyLiveMount,
    OperateItems.ModifiedLiveMount,
    OperateItems.UpdateLiveMount,
    OperateItems.MigrateLiveMount,
    OperateItems.DisableLiveMount,
    OperateItems.ActivateLiveMount,
    OperateItems.IdentitySensitiveData,
    OperateItems.StartDataAnonymization,
    OperateItems.CheckAnonymization,
    OperateItems.ViewLiveMount,
    OperateItems.CreateUpdatingPolicy,
    OperateItems.ModifyUpdatingPolicy,
    OperateItems.DeleteUpdatingPolicy,
    OperateItems.CloneUpdatingPolicy,
    OperateItems.CreateDesensitizationPolicy,
    OperateItems.ModifyDesensitizationPolicy,
    OperateItems.DeleteDesensitizationPolicy,
    OperateItems.CloneDesensitizationPolicy,
    OperateItems.CreateIdentificationRule,
    OperateItems.ModifyIdentificationRule,
    OperateItems.DeleteIdentificationRule,
    OperateItems.CloneIdentificationRule,
    OperateItems.CreateDesensitizationRule,
    OperateItems.ModifyDesensitizationRule,
    OperateItems.DeleteDesensitizationRule,
    OperateItems.CloneDesensitizationRule,
    OperateItems.CreateDataAnonymization,
    OperateItems.ModifyDataAnonymization,
    OperateItems.DeleteDataAnonymization,
    OperateItems.ChangePerformanceStatus,
    OperateItems.AbortJob,
    OperateItems.ExportJob,
    OperateItems.AddingTargetCluster,
    OperateItems.QueryingLocalClusterDetails,
    OperateItems.DeletingTargetCluster,
    OperateItems.ManagedTargetCluster,
    OperateItems.UnManagedTargetCluster,
    OperateItems.ManageHA,
    OperateItems.AddHA,
    OperateItems.ModifyHA,
    OperateItems.DeleteHA,
    OperateItems.ModifyingTargetClusterAuth,
    OperateItems.ModifyAuthenticationStatus,
    OperateItems.AddingExternalStorage,
    OperateItems.ModifyingExternalStorage,
    OperateItems.DeletingExternalStorage,
    OperateItems.CreateArchiveStorage,
    OperateItems.ModifyingArchiveStorage,
    OperateItems.DeletingArchiveStorage,
    OperateItems.ImportingArchiveStorageCopy,
    OperateItems.ModifyStorageAlarmThreshold,
    OperateItems.QueryingStorageDevices,
    OperateItems.QueryingStorageDevices,
    OperateItems.CreatingStorageDevice,
    OperateItems.DeletingstorageDevice,
    OperateItems.CreateUserComponent,
    OperateItems.ModifyingUser,
    OperateItems.DeletingUser,
    OperateItems.LockingUser,
    OperateItems.UnlockingUser,
    OperateItems.ResetPassword,
    OperateItems.SetRestPswdEmail,
    OperateItems.ModifyingPassword,
    OperateItems.QueryingRole,
    OperateItems.QuerySecurityPolicy,
    OperateItems.ModifySecurityPolicy,
    OperateItems.ViewingComponentCertificateDetails,
    OperateItems.AddingExternalComponentCertificates,
    OperateItems.ImportingExternalComponentCertificates,
    OperateItems.ImportingInternalComponentCertificates,
    OperateItems.ChangingCertificateAlarmThreshold,
    OperateItems.DownloadingCertificate,
    OperateItems.RegenerateCertificate,
    OperateItems.DeletingCertificate,
    OperateItems.UpdateHACertificate,
    OperateItems.ExportingRequestFile,
    OperateItems.ImportingRevocationList,
    OperateItems.ViewingCRL,
    OperateItems.DeletingCRL,
    OperateItems.ViewingLicense,
    OperateItems.ImportingLicenseFile,
    OperateItems.ExportingLicenseFile,
    OperateItems.ActivatingLicense,
    OperateItems.ObtainingESN,
    OperateItems.ViewingBackupPolicy,
    OperateItems.ModifyingBackupPolicy,
    OperateItems.ViewingBackupTasks,
    OperateItems.DeletingBackup,
    OperateItems.RevertingBackup,
    OperateItems.ManuallyBackup,
    OperateItems.ImportingBackup,
    OperateItems.ExportingBackup,
    OperateItems.OpenSFTPService,
    OperateItems.AddSFTPUser,
    OperateItems.DeleteSFTPUser,
    OperateItems.ModifySFTPPassword,
    OperateItems.ViewingNotifySendingConfigurations,
    OperateItems.ViewingNotifyReceivingSettings,
    OperateItems.ModifyNotifySendingSettings,
    OperateItems.ManagingNotifyReceivingSettings,
    OperateItems.ViewingAlarmDumpSettings,
    OperateItems.ModifyDumpSettings,
    OperateItems.DownloadDumpFile,
    OperateItems.UploadDumpFile,
    OperateItems.DeletingDumpFile,
    OperateItems.OpenDeviceManagement,
    OperateItems.ModifySystemTime,
    OperateItems.Protection,
    OperateItems.ModifyProtection,
    OperateItems.ActivateProtection,
    OperateItems.DeactivateProtection,
    OperateItems.RemoveProtection,
    OperateItems.ManualBackup,
    OperateItems.DeleteResource,
    OperateItems.RegisterNasShare,
    OperateItems.RegisterRedis,
    OperateItems.CreateDataset,
    OperateItems.RegisterBigDataCluster,
    OperateItems.ModifyHdfsCluster,
    OperateItems.RegisterHdfsCluster,
    OperateItems.UnRegisterHdfsCluster,
    OperateItems.UnRegisterHCSTenant,
    OperateItems.ModifyHCSTenant,
    OperateItems.CreateKerberos,
    OperateItems.ModifyKerberos,
    OperateItems.DeleteKerberos,
    OperateItems.FileLevelRestore,
    OperateItems.SchemaLevelRestore,
    OperateItems.ExportFile,
    OperateItems.ManualIndex,
    OperateItems.DeleteIndex,
    OperateItems.AddStorageDevice,
    OperateItems.DeleteStorageDevice,
    OperateItems.ModifyStorageDevice,
    OperateItems.DownloadExportQuery,
    OperateItems.DeleteExportQuery,
    OperateItems.SetLogLevel,
    OperateItems.ScanTapeLibrary,
    OperateItems.AddBackupStorage,
    OperateItems.ModifyBackupStorage,
    OperateItems.DeleteBackupStorage,
    OperateItems.AddBackupStorageUnit,
    OperateItems.ModifyBackupStorageUnit,
    OperateItems.DeleteBackupStorageUnit,
    OperateItems.upgrateBackupStorageUnit,
    OperateItems.ModifyDataSecurity,
    OperateItems.ModifyHostTrustWorthiness,
    OperateItems.ModifyLdapConfig,
    OperateItems.AddReport,
    OperateItems.DeleteReport,
    OperateItems.AddHcsStorage,
    OperateItems.CreateDrillPlan,
    OperateItems.AddExternalAssociatedSystem,
    OperateItems.EditExternalAssociatedSystem,
    OperateItems.DeleteExternalAssociatedSystem,
    OperateItems.JumpExternalAssociatedSystem,
    OperateItems.EditPacificNodeNetwork,
    OperateItems.EditTag,
    OperateItems.DeleteTag,
    OperateItems.AddTag,
    OperateItems.RemoveTag
  ],
  // 数据保护管理员
  [RoleType.DataProtectionAdmin]: [
    OperateItems.ModifyHost,
    OperateItems.RemoveHost,
    OperateItems.RemoveHost,
    OperateItems.ProtectHosts,
    OperateItems.ModifyHostProtection,
    OperateItems.RemoveHostProtection,
    OperateItems.ManuallyBackHost,
    OperateItems.ManuallyArchiveHost,
    OperateItems.ActivateHostProtection,
    OperateItems.DeactivateHostProtection,
    OperateItems.SynchTrapInfo,
    OperateItems.CreateHostFileset,
    OperateItems.DeleteHostFileset,
    OperateItems.ModifyHostFileset,
    OperateItems.ProtectHostFileset,
    OperateItems.ModifyFilesetProtection,
    OperateItems.RemoveFilesetProtection,
    OperateItems.ManuallyBackFileset,
    OperateItems.ManuallyArchiveFileset,
    OperateItems.ManuallyReplicationFileset,
    OperateItems.ActivateFilesetProtection,
    OperateItems.DeactivateFilesetProtection,
    OperateItems.RegisterDatabase,
    OperateItems.ProtectDatabases,
    OperateItems.ModifyDatabaseProtection,
    OperateItems.RemoveDatabaseProtection,
    OperateItems.ManuallyBackDatabase,
    OperateItems.ManuallyDatabaseProtection,
    OperateItems.ManuallyArchiveDatabase,
    OperateItems.ManuallyReplicationDatabase,
    OperateItems.ActivateDatabaseProtection,
    OperateItems.DeactivateDatabseProtection,
    OperateItems.DeleteDatabase,
    OperateItems.ModifyDatabaseAuth,
    OperateItems.CreatingCluster,
    OperateItems.Deletingcluster,
    OperateItems.ModifyingCluster,
    OperateItems.ModifyingASMAuth,
    OperateItems.RegisterApplication,
    OperateItems.ProtectApplication,
    OperateItems.ModifyApplicationProtection,
    OperateItems.RemoveApplicationProtection,
    OperateItems.ManuallyApplicationProtection,
    OperateItems.ManuallyArchiveApplication,
    OperateItems.ManuallyReplicationApplication,
    OperateItems.ActivateApplicationProtection,
    OperateItems.DeactivateApplicationProtection,
    OperateItems.RegisterVirtualizationPlatform,
    OperateItems.ProtectVM,
    OperateItems.ProtectTENANT,
    OperateItems.RemoveVMProtection,
    OperateItems.RemoveFCProtection,
    OperateItems.ModifyVMProtection,
    OperateItems.ManuallyBackVM,
    OperateItems.ScanHCSProject,
    OperateItems.ManuallyVMProtection,
    OperateItems.ManuallyArchiveVM,
    OperateItems.ManuallyReplicationVM,
    OperateItems.ActivateVMProtection,
    OperateItems.DeactivateVMProtection,
    OperateItems.ActivateFCProtection,
    OperateItems.DeactivateFCProtection,
    OperateItems.DeregisterVirtualizationPlatform,
    OperateItems.ModifyVirtualizationRegister,
    OperateItems.RescanVirtualizationPlatform,
    OperateItems.HCSEnvironmentInfo,
    OperateItems.RegisterBigDataCluster,
    OperateItems.DeregisterBigDataCluster,
    OperateItems.ModifyBigDataCluster,
    OperateItems.CreatingBigDataFileset,
    OperateItems.DeletingBigDataFileset,
    OperateItems.ModifyBigDataFileset,
    OperateItems.ProtectBigDataFilesets,
    OperateItems.ModifyBigDataFilesetProtection,
    OperateItems.RemoveBigDataFilesetProtection,
    OperateItems.ManuallyBackingBigDataFileset,
    OperateItems.ManuallyArchivingBigDataFileset,
    OperateItems.ManuallyReplicationBigDataFileset,
    OperateItems.ActivateBigDataFilesetProtection,
    OperateItems.DeactivatingBigDataFilesetProtection,
    OperateItems.QueryingCloudPlatform,
    OperateItems.RegisteringCloudPlatform,
    OperateItems.RegisteringTenant,
    OperateItems.CreateSLA,
    OperateItems.ModifySLA,
    OperateItems.DeleteSLA,
    OperateItems.CloneSLA,
    OperateItems.CreateQos,
    OperateItems.ModifyQos,
    OperateItems.DeleteQos,
    OperateItems.CreateKubernetesRule,
    OperateItems.ModifyKubernetesRule,
    OperateItems.DeleteKubernetesRule,
    OperateItems.ModifyingCopyRetentionPolicy,
    OperateItems.CopyDuplicate,
    OperateItems.DeletingCopy,
    OperateItems.DownloadCopy,
    OperateItems.RestoreCopy,
    OperateItems.InstanceRecovery,
    OperateItems.MountingCopy,
    OperateItems.CreatingLiveMount,
    OperateItems.DestroyLiveMount,
    OperateItems.UpdateLatestCopyLiveMount,
    OperateItems.ModifiedLiveMount,
    OperateItems.UpdateLiveMount,
    OperateItems.MigrateLiveMount,
    OperateItems.ActivateLiveMount,
    OperateItems.ViewLiveMount,
    OperateItems.IdentitySensitiveData,
    OperateItems.StartDataAnonymization,
    OperateItems.CheckAnonymization,
    OperateItems.DisableLiveMount,
    OperateItems.CreateUpdatingPolicy,
    OperateItems.ModifyUpdatingPolicy,
    OperateItems.DeleteUpdatingPolicy,
    OperateItems.CloneUpdatingPolicy,
    OperateItems.CreateDesensitizationPolicy,
    OperateItems.ModifyDesensitizationPolicy,
    OperateItems.DeleteDesensitizationPolicy,
    OperateItems.CloneDesensitizationPolicy,
    OperateItems.CreateIdentificationRule,
    OperateItems.ModifyIdentificationRule,
    OperateItems.DeleteIdentificationRule,
    OperateItems.CloneIdentificationRule,
    OperateItems.CreateDesensitizationRule,
    OperateItems.ModifyDesensitizationRule,
    OperateItems.DeleteDesensitizationRule,
    OperateItems.CloneDesensitizationRule,
    OperateItems.CreateDataAnonymization,
    OperateItems.ModifyDataAnonymization,
    OperateItems.DeleteDataAnonymization,
    OperateItems.AbortJob,
    OperateItems.ExportJob,
    OperateItems.ModifyingPassword,
    OperateItems.QuerySecurityPolicy,
    OperateItems.ModifySecurityPolicy,
    OperateItems.Protection,
    OperateItems.ModifyProtection,
    OperateItems.ActivateProtection,
    OperateItems.DeactivateProtection,
    OperateItems.RemoveProtection,
    OperateItems.ManualBackup,
    OperateItems.DeleteResource,
    OperateItems.RegisterNasShare,
    OperateItems.RegisterRedis,
    OperateItems.CreateDataset,
    OperateItems.RegisterBigDataCluster,
    OperateItems.ModifyHdfsCluster,
    OperateItems.RegisterHdfsCluster,
    OperateItems.UnRegisterHdfsCluster,
    OperateItems.ModifyHCSTenant,
    OperateItems.UnRegisterHCSTenant,
    OperateItems.CreateKerberos,
    OperateItems.ModifyKerberos,
    OperateItems.DeleteKerberos,
    OperateItems.FileLevelRestore,
    OperateItems.SchemaLevelRestore,
    OperateItems.ExportFile,
    OperateItems.ManualIndex,
    OperateItems.DeleteIndex,
    OperateItems.AddStorageDevice,
    OperateItems.DeleteStorageDevice,
    OperateItems.ModifyStorageDevice,
    OperateItems.DownloadExportQuery,
    OperateItems.DeleteExportQuery,
    OperateItems.SetLogLevel,
    OperateItems.AddReport,
    OperateItems.DeleteReport,
    OperateItems.CreateDrillPlan,
    OperateItems.EditTag,
    OperateItems.DeleteTag,
    OperateItems.AddTag,
    OperateItems.RemoveTag
  ],
  // 审计员
  [RoleType.Auditor]: [
    OperateItems.QueryingInfrastructureCluster,
    OperateItems.QueryingLocalClusterDetails,
    OperateItems.CreatingStorageDevice,
    OperateItems.DeletingstorageDevice,
    OperateItems.QueryingRole,
    OperateItems.QuerySecurityPolicy,
    OperateItems.ModifySecurityPolicy,
    OperateItems.ViewLiveMount,
    OperateItems.ViewingCertificateList,
    OperateItems.ViewingComponentCertificateDetails,
    OperateItems.ViewingCRL,
    OperateItems.ViewingLicense,
    OperateItems.ViewingBackupPolicy,
    OperateItems.ViewingBackupTasks,
    OperateItems.ViewingNotifySendingConfigurations,
    OperateItems.ViewingNotifyReceivingSettings,
    OperateItems.ViewingAlarmDumpSettings,
    OperateItems.ModifyingPassword
  ]
};
