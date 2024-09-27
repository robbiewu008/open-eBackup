import { Injectable } from '@angular/core';
import { JobResourceComponent } from 'app/business/insight/job/job-resource/job-resource.component';
import { SummaryComponent as ActiveDirectorySummaryComponent } from 'app/business/protection/application/active-directory/summary/summary.component';
import { SummaryDatabaseListComponent as SaphanaSummaryDatabaseListComponent } from 'app/business/protection/application/saphana/summary-database-list/summary-database-list.component';
import { SummaryDatabaseComponent as SaphanaSummaryDatabaseComponent } from 'app/business/protection/application/saphana/summary-database/summary-database.component';
import { SummaryInstanceComponent as SaphanaSummaryInstanceComponent } from 'app/business/protection/application/saphana/summary-instance/summary-instance.component';
import { CopyDataComponent as BackupSetCopyDataComponent } from 'app/business/protection/big-data/hbase/backup-set/copy-data/copy-data.component';
import { SummaryComponent as BackupSetSummaryComponent } from 'app/business/protection/big-data/hbase/backup-set/summary/summary.component';
import { ClusterBackupsetDetailComponent } from 'app/business/protection/big-data/hbase/clusters/cluster-backupset-detail/cluster-backupset-detail.component';
import { ClusterDetailComponent as HiveBasicInfoComponent } from 'app/business/protection/big-data/hbase/clusters/cluster-detail/cluster-detail.component';
import { CopyDataComponent as HdfsFilesetCopyDataComponent } from 'app/business/protection/big-data/hdfs/filesets/copy-data/copy-data.component';
import { ResourceSetSummaryComponent } from 'app/business/protection/cloud/apsara-stack/resource-set-summary/resource-set-summary.component';
import { HCSCopyDataComponent } from 'app/business/protection/cloud/huawei-stack/stack-list/copy-data/hcs-copy-data.component';
import { HCSHostSummaryComponent } from 'app/business/protection/cloud/huawei-stack/stack-list/host-summary/host-summary.component';
import { ProjectSummaryComponent } from 'app/business/protection/cloud/huawei-stack/stack-list/project-summary/project-summary.component';
import { SummaryComponent } from 'app/business/protection/cloud/openstack/openstack-list/summary/summary.component';
import { DetailListComponent as ClickHouseClusterDetailListComponent } from 'app/business/protection/host-app/click-house/cluster/detail-list/detail-list.component';
import { SummaryComponent as ClickHouseClusterSumaryComponent } from 'app/business/protection/host-app/click-house/cluster/summary/summary.component';
import { SummaryComponent as ClickHouseDatabaseSumaryComponent } from 'app/business/protection/host-app/click-house/database/summary/summary.component';
import { SummaryComponent as ClickHouseTablesetSumaryComponent } from 'app/business/protection/host-app/click-house/tabel-set/summary/summary.component';
import { CopyDataComponent as DamengCopyDataComponent } from 'app/business/protection/host-app/dameng/copy-data/copy-data.component';
import { SummaryComponent as DamengSummaryComponent } from 'app/business/protection/host-app/dameng/summary/summary.component';
import { CopyDataComponent } from 'app/business/protection/host-app/database-template/copy-data/copy-data.component';
import { CopyDataComponent as DbTwoCopyDataComponent } from 'app/business/protection/host-app/db-two/copy-data/copy-data.component';
import { SummaryComponent as DbTwoDatabaseListComponent } from 'app/business/protection/host-app/db-two/database-summary/summary.component';
import { SummaryComponent as DbTwoReousrceSummaryComponent } from 'app/business/protection/host-app/db-two/db-two-summary/summary.component';
import { SummaryComponent as DbTwoSummaryComponent } from 'app/business/protection/host-app/db-two/summary/summary.component';
import { ExchangeDetailsComponent } from 'app/business/protection/host-app/exchange/availabilty-group/details/details.component';
import { AdvancedParameterComponent } from 'app/business/protection/host-app/fileset/advanced-parameter/advanced-parameter.component';
import { CopyDataComponent as FilesetCopyDataComponent } from 'app/business/protection/host-app/fileset/copy-data/copy-data.component';
import { SummaryComponent as FilesetSummaryComponent } from 'app/business/protection/host-app/fileset/summary/summary.component';
import { CopyDataComponent as GaussDBDWSCopyDataComponent } from 'app/business/protection/host-app/gaussdb-dws/copy-data/copy-data.component';
import { SummaryComponent as GaussDBDWSSummaryComponent } from 'app/business/protection/host-app/gaussdb-dws/instance-database/summary/summary.component';
import { SummaryComponent as GaussdbForOpengaussProjectSummaryComponent } from 'app/business/protection/host-app/gaussdb-for-opengauss/project-summary/summary.component';
import { SummaryComponent as GaussdbForOpengaussSummaryComponent } from 'app/business/protection/host-app/gaussdb-for-opengauss/summary/summary.component';
import { CopyDataComponent as GaussDBTCopyDataComponent } from 'app/business/protection/host-app/gaussdb-t/copy-data/copy-data.component';
import { SummaryComponent as GaussDBTSummaryComponent } from 'app/business/protection/host-app/gaussdb-t/summary/summary.component';
import { CopyDataComponent as GeneralDatabaseCopyDataComponent } from 'app/business/protection/host-app/general-database/copy-data/copy-data.component';
import { SummaryComponent as GeneralDatabaseSummaryComponent } from 'app/business/protection/host-app/general-database/summary/summary.component';
import { ClusterBackupsetDetailComponent as GoldenDBClusterBackupsetDetailComponent } from 'app/business/protection/host-app/goldendb/cluster-backupset-detail/cluster-backupset-detail.component';
import { ClusterDetailComponent as GoldenDBClusterDetailComponent } from 'app/business/protection/host-app/goldendb/cluster-detail/cluster-detail.component';
import { SummaryComponent as GoldendbSummaryComponent } from 'app/business/protection/host-app/goldendb/summary/summary.component';
import { SummaryComponent as HostSummaryComponent } from 'app/business/protection/host-app/host/summary/summary.component';
import { SummaryComponent as InformixSummaryComponent } from 'app/business/protection/host-app/informix/summary-instance/summary.component';
import { SummaryServiceComponent } from 'app/business/protection/host-app/informix/summary-service/summary-service.component';
import { KingBaseCopyDataComponent } from 'app/business/protection/host-app/king-base/instance-database/copy-data/king-base-copy-data.component';
import { KingBaseSummaryComponent } from 'app/business/protection/host-app/king-base/instance-database/summary/king-base-summary.component';
import { SummaryComponent as LightCloudGaussdbProjectSummaryComponent } from 'app/business/protection/host-app/light-cloud-gaussdb/project-summary/summary.component';
import { SummaryComponent as LightCloudGaussdbSummaryComponent } from 'app/business/protection/host-app/light-cloud-gaussdb/summary/summary.component';
import { CopyDataComponent as MongoDBCopyDataComponent } from 'app/business/protection/host-app/mongodb/copy-data/copy-data.component';
import { SummaryComponent as MongoDBSummaryComponent } from 'app/business/protection/host-app/mongodb/summary/summary.component';
import { CopyDataComponent as MysqlCopyDataComponent } from 'app/business/protection/host-app/mysql/instance-database/copy-data/copy-data.component';
import { SummaryComponent as MysqlSummaryComponent } from 'app/business/protection/host-app/mysql/instance-database/summary/summary.component';
import { SummaryClusterComponent as OceanBaseSummaryClusterComponent } from 'app/business/protection/host-app/ocean-base/summary-cluster/summary-cluster.component';
import { SummaryTenantListComponent as OceanBaseSummaryTenantListComponent } from 'app/business/protection/host-app/ocean-base/summary-tenant-list/summary-tenant-list.component';
import { SummaryTenantComponent as OceanBaseSummaryTenantComponent } from 'app/business/protection/host-app/ocean-base/summary-tenant/summary-tenant.component';
import { CopyDataComponent as OpenGauss_copyDataComponent } from 'app/business/protection/host-app/opengauss/base-template/copy-data/copy-data.component';
import { SummaryComponent as OpenGauss_instanceSummaryComponent } from 'app/business/protection/host-app/opengauss/base-template/summary/summary.component';
import { AdvancedParameterComponent as OracleAdvancedParameterComponent } from 'app/business/protection/host-app/oracle/database-list/advanced-parameter/advanced-parameter.component';
import { CopyDataComponent as OracleCopyDataComponent } from 'app/business/protection/host-app/oracle/database-list/copy-data/copy-data.component';
import { SummaryComponent as OracleSummaryComponent } from 'app/business/protection/host-app/oracle/database-list/summary/summary.component';
import { PostgreCopyDataComponent } from 'app/business/protection/host-app/postgre-sql/instance-database/copy-data/postgre-copy-data.component';
import { PostgreSummaryComponent } from 'app/business/protection/host-app/postgre-sql/instance-database/summary/postgre-summary.component';
import { RedisCopyDataComponent } from 'app/business/protection/host-app/redis/copy-data/redis-copy-data.component';
import { SummaryComponent as RedisSummaryComponent } from 'app/business/protection/host-app/redis/summary/summary.component';
import { ClusterBackupsetDetailComponent as SQLServerClusterBackupsetDetailComponent } from 'app/business/protection/host-app/sql-server/cluster-backupset-detail/cluster-backupset-detail.component';
import { ClusterDetailComponent as SQLServerClusterDetailComponent } from 'app/business/protection/host-app/sql-server/cluster-detail/cluster-detail.component';
import { SummaryComponent as SQLServerSummaryComponent } from 'app/business/protection/host-app/sql-server/summary/summary.component';
import { SummaryComponent as SummaryTDSQLDistributedInstanceComponent } from 'app/business/protection/host-app/tdsql/dirstibuted-instance/summary/summary.component';
import { SummaryClusterComponent as TDSQLSummaryClusterComponent } from 'app/business/protection/host-app/tdsql/summary-cluster/summary-cluster.component';
import { SummaryInstanceListComponent as TDSQLSummaryInstanceListComponent } from 'app/business/protection/host-app/tdsql/summary-instance-list/summary-instance-list.component';
import { SummaryComponent as SummaryTDSQLInstanceComponent } from 'app/business/protection/host-app/tdsql/summary-instance/summary.component';
import { SummaryClusterComponent as TiDBSummaryClusterComponent } from 'app/business/protection/host-app/tidb/summary-cluster/summary-cluster.component';
import { SummaryDatabaseComponent as TiDBSummaryDatabaseComponent } from 'app/business/protection/host-app/tidb/summary-database/summary-database.component';
import { SummaryTableComponent as TiDBSummaryTableComponent } from 'app/business/protection/host-app/tidb/summary-table/summary-table.component';
import { LinkComponent } from 'app/business/protection/storage/commonshare/link/link.component';
import { SummaryCommonShareComponent } from 'app/business/protection/storage/commonshare/summary-commonshare/summary-commonshare.component';
import { CopyDataComponent as NasSharedCopyDataComponent } from 'app/business/protection/storage/nas-shared/copy-data/copy-data.component';
import { SummaryComponent as NasSharedSummaryComponent } from 'app/business/protection/storage/nas-shared/summary/summary.component';
import { SummaryComponent as ObjectSetSummaryComponent } from 'app/business/protection/storage/object/object-service/summary/summary.component';
import { CopyDataComponent as CnwareCopyDataComponent } from 'app/business/protection/virtualization/cnware/copy-data/copy-data.component';
import { SummaryComponent as CnwareSummaryComponent } from 'app/business/protection/virtualization/cnware/summary/summary.component';
import { FusionComputeCopyDataComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/copy-data/fusion-compute-copy-data.component';
import { FusionClusterSummaryComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/fusion-cluster-summary/fusion-cluster-summary.component';
import { FusionHostSummaryComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/fusion-host-summary/fusion-host-summary.component';
import { FusionSummaryComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/fusion-summary/fusion-summary.component';
import { HypervCopyDataComponent } from 'app/business/protection/virtualization/hyper-v/hyperv-copy-data/hyperv-copy-data.component';
import { SummaryComponent as HypervSummaryComponent } from 'app/business/protection/virtualization/hyper-v/summary/summary.component';
import { NameDetailComponent } from 'app/business/protection/virtualization/kubernetes/base-template/name-detail/name-detail.component';
import { SummaryComponent as StateFulSetSummaryComponent } from 'app/business/protection/virtualization/kubernetes/base-template/summary/summary.component';
import { CopyDataComponent as StateFulSetCopyDataComponent } from 'app/business/protection/virtualization/kubernetes/statefulset/copy-data/copy-data.component';
import { GroupSummaryComponent } from 'app/business/protection/virtualization/virtualization-group/group-summary/group-summary.component';
import { CopyDataComponent as VmwareCopyDataComponent } from 'app/business/protection/virtualization/vmware/vm/copy-data/copy-data.component';
import { SummaryComponent as VmwareSummaryComponent } from 'app/business/protection/virtualization/vmware/vm/summary/summary.component';
import { DataMap, MODAL_COMMON } from 'app/shared/consts';
import { GlobalService, I18NService } from 'app/shared/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  includes,
  isFunction,
  mapValues
} from 'lodash';
import { DetailModalComponent } from '../components/detail-modal/detail-modal.component';
import { LinkStatusComponent } from '../components/link-status/link-status.component';
import { CopyDataComponent as LunCopyDataComponent } from 'app/business/protection/storage/local-lun/copy-data/copy-data.component';

const JOBS_CONFIG = [
  {
    title: 'common_jobs_label',
    activeId: 'jobs',
    component: JobResourceComponent
  }
];

const DETAIL_CONFIG = {
  [DataMap.Resource_Type.APSResourceSet.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ResourceSetSummaryComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.APSCloudServer.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: HCSHostSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: HCSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.APSZone.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ProjectSummaryComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.Exchange.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ExchangeDetailsComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MongoDBCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ExchangeSingle.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ExchangeDetailsComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MongoDBCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ExchangeGroup.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ExchangeDetailsComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MongoDBCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ExchangeEmail.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: OpenGauss_instanceSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MongoDBCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ExchangeDataBase.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: OpenGauss_instanceSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MongoDBCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ObjectSet.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ObjectSetSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.commonShare.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: SummaryCommonShareComponent
      },
      {
        title: 'protection_commonshare_copydata_link_label',
        activeId: 'link',
        component: LinkComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ActiveDirectory.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ActiveDirectorySummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.tidbCluster.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: TiDBSummaryClusterComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.tidbDatabase.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_database_label',
        activeId: 'summary',
        component: TiDBSummaryDatabaseComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.tidbTable.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'protection_table_set_label',
        activeId: 'summary',
        component: TiDBSummaryTableComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.OceanBaseCluster.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: OceanBaseSummaryClusterComponent
      },
      {
        title: 'common_tenant_else_label',
        activeId: 'tenant',
        component: OceanBaseSummaryTenantListComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.OceanBaseTenant.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: OceanBaseSummaryTenantComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.tdsqlCluster.value]: {
    lvWidth: MODAL_COMMON.largeModal,
    tabs: [
      {
        title: 'common_basic_info_label',
        activeId: 'summary',
        component: TDSQLSummaryClusterComponent
      },
      {
        title: 'protection_database_instance_label',
        activeId: 'copydata',
        component: TDSQLSummaryInstanceListComponent
      }
    ]
  },
  [DataMap.Resource_Type.tdsqlInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: SummaryTDSQLInstanceComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.tdsqlDistributedInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: SummaryTDSQLDistributedInstanceComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.informixService.value]: {
    lvWidth: MODAL_COMMON.smallModal,
    tabs: [
      {
        title: 'common_basic_info_label',
        activeId: 'summary',
        component: SummaryServiceComponent
      }
    ]
  },
  [DataMap.Resource_Type.informixInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: InformixSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.informixClusterInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: InformixSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.gbaseCluster.value]: {
    lvWidth: MODAL_COMMON.smallModal,
    tabs: [
      {
        title: 'common_basic_info_label',
        activeId: 'summary',
        component: SummaryServiceComponent
      }
    ]
  },
  [DataMap.Resource_Type.gbaseClusterInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: InformixSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.gbaseInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: InformixSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.gaussdbForOpengaussInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GaussdbForOpengaussSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.gaussdbForOpengaussProject.value]: {
    lvWidth: MODAL_COMMON.smallModal,
    tabs: [
      {
        title: 'common_basic_info_label',
        activeId: 'summary',
        component: GaussdbForOpengaussProjectSummaryComponent
      }
    ]
  },
  [DataMap.Resource_Type.lightCloudGaussdbInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: LightCloudGaussdbSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.lightCloudGaussdbProject.value]: {
    lvWidth: MODAL_COMMON.smallModal,
    tabs: [
      {
        title: 'common_basic_info_label',
        activeId: 'summary',
        component: LightCloudGaussdbProjectSummaryComponent
      }
    ]
  },
  [DataMap.Resource_Type.goldendbInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GoldendbSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.dbTwoCluster.value]: {
    lvWidth: MODAL_COMMON.largeModal,
    tabs: [
      {
        title: 'common_basic_info_label',
        activeId: 'summary',
        component: DbTwoSummaryComponent
      }
    ]
  },
  [DataMap.Resource_Type.dbTwoInstance.value]: {
    lvWidth: MODAL_COMMON.largeModal,
    tabs: [
      {
        title: 'common_basic_info_label',
        activeId: 'summary',
        component: DbTwoSummaryComponent
      },
      {
        title: 'common_database_label',
        activeId: 'database',
        component: DbTwoDatabaseListComponent
      }
    ]
  },
  [DataMap.Resource_Type.dbTwoClusterInstance.value]: {
    lvWidth: MODAL_COMMON.largeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: DbTwoSummaryComponent
      },
      {
        title: 'common_database_label',
        activeId: 'database',
        component: DbTwoDatabaseListComponent
      }
    ]
  },
  [DataMap.Resource_Type.dbTwoDatabase.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: DbTwoReousrceSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: DbTwoCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.dbTwoTableSet.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: DbTwoReousrceSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: DbTwoCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ABBackupClient.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: HostSummaryComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.generalDatabase.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GeneralDatabaseSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GeneralDatabaseCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.oracle.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: OracleSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: OracleCopyDataComponent
      },
      {
        title: 'protection_fileset_protection_detail_label',
        activeId: 'protectionDetails',
        component: OracleAdvancedParameterComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.oracleCluster.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: OracleSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: OracleCopyDataComponent
      },
      {
        title: 'protection_fileset_protection_detail_label',
        activeId: 'protectionDetails',
        component: OracleAdvancedParameterComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.virtualMachine.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: VmwareSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: VmwareCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.vmGroup.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GroupSummaryComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.HCSCloudHost.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: HCSHostSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: HCSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },

  [DataMap.Resource_Type.HCSProject.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ProjectSummaryComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.FusionCompute.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: FusionSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: FusionComputeCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.fusionComputeVirtualMachine.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: FusionSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: FusionComputeCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.fusionComputeCluster.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: FusionClusterSummaryComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.fusionComputeCNA.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: FusionHostSummaryComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.fusionOne.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: FusionSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: FusionComputeCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.NASFileSystem.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NasSharedSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: NasSharedCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ndmp.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NasSharedSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: NasSharedCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ndmp.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NasSharedSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: NasSharedCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.LocalFileSystem.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NasSharedSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: NasSharedCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.LocalLun.value]: {
    lvWidth: 1150,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NasSharedSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: LunCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.LocalLun.value]: {
    lvWidth: 1150,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NasSharedSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: LunCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.NASShare.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NasSharedSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: NasSharedCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.HDFSFileset.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: FilesetSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: HdfsFilesetCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.HBaseBackupSet.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: BackupSetSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: BackupSetCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.GaussDB_T.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GaussDBTSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBTCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.gaussdbTSingle.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GaussDBTSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBTCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.Redis.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: RedisSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: RedisCopyDataComponent
      },

      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.MySQL.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: MysqlSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MysqlCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },

  [DataMap.Resource_Type.KingBaseClusterInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: KingBaseSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: KingBaseCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },

  [DataMap.Resource_Type.KingBaseInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: KingBaseSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: KingBaseCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.Dameng_cluster.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: DamengSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: DamengCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.Dameng_singleNode.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: DamengSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: DamengCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.MySQLDatabase.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: MysqlSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MysqlCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.MySQLClusterInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: MysqlSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MysqlCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.MySQLInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: MysqlSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MysqlCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },

  [DataMap.Resource_Type.PostgreSQLClusterInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: PostgreSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: PostgreCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },

  [DataMap.Resource_Type.PostgreSQLInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: PostgreSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: PostgreCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },

  [DataMap.Resource_Type.HiveBackupSet.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: BackupSetSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: BackupSetCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ElasticsearchBackupSet.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: BackupSetSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: BackupSetCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.Hive.value]: {
    lvWidth: MODAL_COMMON.largeModal,
    tabs: [
      {
        title: 'common_basic_info_label',
        activeId: 'basicinfo',
        component: HiveBasicInfoComponent
      },
      {
        title: 'protection_backup_set_label',
        activeId: 'backupset',
        component: ClusterBackupsetDetailComponent
      }
    ]
  },
  [DataMap.Resource_Type.fileset.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: FilesetSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: FilesetCopyDataComponent
      },
      {
        title: 'protection_fileset_protection_detail_label',
        activeId: 'protectionDetails',
        component: AdvancedParameterComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.volume.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: FilesetSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: FilesetCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.Dameng.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: DamengSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: DamengCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.OpenGauss_instance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: OpenGauss_instanceSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: OpenGauss_copyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.OpenGauss_database.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: OpenGauss_instanceSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: OpenGauss_copyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.GaussDB_DWS.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GaussDBDWSSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ClickHouse.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ClickHouseDatabaseSumaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ClickHouseCluster.value]: {
    lvWidth: MODAL_COMMON.smallModal,
    tabs: [
      {
        title: 'common_basic_info_label',
        activeId: 'summary',
        component: ClickHouseClusterSumaryComponent
      },
      {
        title: 'common_database_label',
        activeId: 'database',
        component: ClickHouseClusterDetailListComponent
      },
      {
        title: 'protection_table_set_label',
        activeId: 'tableset',
        component: ClickHouseClusterDetailListComponent
      }
    ]
  },
  [DataMap.Resource_Type.ClickHouseDatabase.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ClickHouseDatabaseSumaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.ClickHouseTableset.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ClickHouseTablesetSumaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.DWS_Database.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GaussDBDWSSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.goldendbCluter.value]: {
    lvWidth: MODAL_COMMON.smallModal,
    tabs: [
      {
        title: 'common_basic_info_label',
        activeId: 'summary',
        component: GoldenDBClusterDetailComponent
      },
      {
        title: 'protection_database_instance_label',
        activeId: 'copydata',
        component: GoldenDBClusterBackupsetDetailComponent
      }
    ]
  },
  [DataMap.Resource_Type.SQLServerCluster.value]: {
    lvWidth: MODAL_COMMON.smallModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: SQLServerClusterDetailComponent
      },
      {
        title: 'protection_database_instance_label',
        activeId: 'copydata',
        component: SQLServerClusterBackupsetDetailComponent
      }
    ]
  },
  [DataMap.Resource_Type.SQLServerClusterInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: SQLServerSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.SQLServerInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: SQLServerSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.SQLServerGroup.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GaussDBDWSSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.SQLServerDatabase.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GaussDBDWSSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.KubernetesNamespace.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NameDetailComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.kubernetesNamespaceCommon.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NameDetailComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: StateFulSetCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.KubernetesStatefulset.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: StateFulSetSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: StateFulSetCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.kubernetesDatasetCommon.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: StateFulSetSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: StateFulSetCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.DWS_Schema.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GaussDBDWSSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.DWS_Table.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: GaussDBDWSSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: GaussDBDWSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.openStackProject.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: ProjectSummaryComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.openStackCloudServer.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: SummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: HCSCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.MongoDB.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: MongoDBSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MongoDBCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.MongodbClusterInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: MongoDBSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MongoDBCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.MongodbSingleInstance.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: MongoDBSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: MongoDBCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.cNwareVm.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: CnwareSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CnwareCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.hyperVCluster.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: HypervSummaryComponent
      }
    ]
  },
  [DataMap.Resource_Type.hyperVScvmm.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: HypervSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: HypervCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.hyperVHost.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: HypervSummaryComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.hyperVVm.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: HypervSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: HypervCopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  },
  [DataMap.Resource_Type.saphanaInstance.value]: {
    lvWidth: MODAL_COMMON.smallModal,
    tabs: [
      {
        title: 'common_basic_info_label',
        activeId: 'summary',
        component: SaphanaSummaryInstanceComponent
      },
      {
        title: 'common_database_label',
        activeId: 'copydata',
        component: SaphanaSummaryDatabaseListComponent
      }
    ]
  },
  [DataMap.Resource_Type.saphanaDatabase.value]: {
    lvWidth: MODAL_COMMON.xLargeModal,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: SaphanaSummaryDatabaseComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: CopyDataComponent
      },
      ...JOBS_CONFIG
    ]
  }
};

@Injectable({ providedIn: 'root' })
export class ResourceDetailService {
  constructor(
    private drawModalService: DrawModalService,
    private i18n: I18NService,
    private globalService: GlobalService
  ) {}

  openDetailModal(type, option: { [key: string]: any } = {}, closeCallback?) {
    if (
      includes(
        mapValues(this.drawModalService.modals, 'key'),
        'slaDetailModalKey'
      )
    ) {
      this.drawModalService.destroyModal('slaDetailModalKey');
    }
    const config = cloneDeep(DETAIL_CONFIG[type]);
    if (option.formCopyDataList) {
      config.tabs = filter(config.tabs as any, item => {
        return item.activeId === 'copydata';
      });
    }
    if (
      includes(
        [
          DataMap.Deploy_Type.hyperdetect.value,
          DataMap.Deploy_Type.cyberengine.value
        ],
        this.i18n.get('deploy_type')
      )
    ) {
      each(config.tabs as any, item => {
        if (item.activeId === 'copydata') {
          item.title = 'common_hyperdetect_copy_data_label';
        }
      });
    }
    if (type === DataMap.Resource_Type.fileset.value) {
      if (
        option.data.protectionStatus !==
        DataMap.Protection_Status.protected.value
      ) {
        config.tabs = filter(config.tabs as any, item => {
          return item.activeId !== 'protectionDetails';
        });
      }
    }
    const modalServiceParam = assign({}, MODAL_COMMON.generateDrawerOptions(), {
      lvModalKey: 'detail-modal',
      lvModality: false,
      lvWidth: config.lvWidth || MODAL_COMMON.xLargeModal,
      lvHeader: option.lvHeader,
      lvContent: DetailModalComponent,
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ],
      lvComponentParams: {
        resourceConfigs: config.tabs,
        detailData: assign({}, option.data, { resourceType: type }),
        activeId: option.data.activeId || config.tabs[0].activeId
      },
      lvAfterClose: () => {
        if (isFunction(closeCallback)) {
          closeCallback();
        }
        // 关闭详情框，发布全局消息
        this.globalService.emitStore({
          action: 'detailModalClose',
          state: true
        });
      }
    });

    // 一体机资源详情
    if (
      this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value
    ) {
      assign(modalServiceParam, { lvWidth: this.i18n.isEn ? 1300 : 1200 });
    }

    if (config) {
      this.drawModalService.openDetailModal(modalServiceParam);
    }
  }

  openLinkModal(item) {
    if (
      includes(
        mapValues(this.drawModalService.modals, 'key'),
        'linkStatusModalKey'
      )
    ) {
      this.drawModalService.destroyModal('linkStatusModalKey');
    }

    const modalServiceParam = assign({}, MODAL_COMMON.generateDrawerOptions(), {
      lvModalKey: 'detail-modal',
      lvModality: false,
      lvWidth: MODAL_COMMON.xLargeModal,
      lvHeader: this.i18n.get('common_status_detail_label'),
      lvContent: LinkStatusComponent,
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ],
      lvComponentParams: {
        data: item
      }
    });
    if (item) {
      this.drawModalService.openDetailModal(modalServiceParam);
    }
  }
}
