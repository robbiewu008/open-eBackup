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
import { NgModule } from '@angular/core';
import { SelectCopyResourceComponent } from 'app/business/explore/copy-data/copy-resource-list/resource-replica-list/select-copy-resource/select-copy-resource.component';
import { SelectCopyResourceModule } from 'app/business/explore/copy-data/copy-resource-list/resource-replica-list/select-copy-resource/select-copy-resource.module';
import { LearningConfigComponent } from 'app/business/explore/ransomware-protection/data-backup/file-system/learning-config/learning-config.component';
import { LearningConfigModule } from 'app/business/explore/ransomware-protection/data-backup/file-system/learning-config/learning-config.module';
import { AdvancedParameterComponent as ActiveDirectoryAdvancedComponent } from 'app/business/protection/application/active-directory/advanced-parameter/advanced-parameter.component';
import { AdvancedParameterModule as ActiveDirectoryAdvancedModule } from 'app/business/protection/application/active-directory/advanced-parameter/advanced-parameter.module';
import { SelectBackupSetListComponent as HBaseSelectBackupSetListComponent } from 'app/business/protection/big-data/hbase/backup-set/select-backup-set-list/select-backup-set-list.component';
import { SelectBackupSetListModule as HBaseSelectBackupSetListModule } from 'app/business/protection/big-data/hbase/backup-set/select-backup-set-list/select-backup-set-list.module';
import { AdvancedParameterModule as HDFSAdvancedParameterModule } from 'app/business/protection/big-data/hdfs/filesets/advanced-parameter/advanced-parameter.module';
import { SelectFilesetsListComponent as HDFSSelectFilesetsListComponent } from 'app/business/protection/big-data/hdfs/filesets/select-filesets-list/select-filesets-list.component';
import { SelectFilesetsListModule as HDFSSelectFilesetsListModule } from 'app/business/protection/big-data/hdfs/filesets/select-filesets-list/select-filesets-list.module';
import { ApsAdvanceParameterComponent } from 'app/business/protection/cloud/apsara-stack/aps-advance-parameter/aps-advance-parameter.component';
import { ApsAdvanceParameterModule } from 'app/business/protection/cloud/apsara-stack/aps-advance-parameter/aps-advance-parameter.module';
import { ApsProtectSelectComponent } from 'app/business/protection/cloud/apsara-stack/aps-protect-select/aps-protect-select.component';
import { ApsProtectSelectModule } from 'app/business/protection/cloud/apsara-stack/aps-protect-select/aps-protect-select.module';
import { SelectDatabaseListComponent } from 'app/business/protection/cloud/huawei-stack/stack-list/select-database-list/select-database-list.component';
import { CloudStackAdvancedParameterComponent } from 'app/business/protection/cloud/huawei-stack/stack-list/stack-advanced-parameter/cloud-stack-advanced-parameter.component';
import { AdvancedComponent } from 'app/business/protection/cloud/openstack/openstack-list/advanced/advanced.component';
import { AdvancedModule } from 'app/business/protection/cloud/openstack/openstack-list/advanced/advanced.module';
import { SelectPoComponent } from 'app/business/protection/cloud/openstack/openstack-list/select-po/select-po.component';
import { SelectPoModule } from 'app/business/protection/cloud/openstack/openstack-list/select-po/select-po.module';
import { AdvancedParameterComponent as MysqlAdvancedParameterComponent } from 'app/business/protection/host-app//mysql/instance-database/advanced-parameter/advanced-parameter.component';
import { AdvancedParameterModule as MysqlAdvancedParameterModule } from 'app/business/protection/host-app//mysql/instance-database/advanced-parameter/advanced-parameter.module';
import { SelectDatabaseComponent as ClickHouseDataBaseSelectDatabaseComponent } from 'app/business/protection/host-app/click-house/database/select-database/select-database.component';
import { SelectProtectRowComponent as ClickHouseTablesetSelectDatabaseComponent } from 'app/business/protection/host-app/click-house/tabel-set/select-protect-row/select-protect-row.component';
import { AdvancedExchangeComponent } from 'app/business/protection/host-app/exchange/database/advanced-exchange/advanced-exchange.component';
import { AdvancedExchangeModule } from 'app/business/protection/host-app/exchange/database/advanced-exchange/advanced-exchange.module';
import { AdvancedEmailComponent } from 'app/business/protection/host-app/exchange/email/advanced-email/advanced-email.component';
import { AdvancedEmailModule } from 'app/business/protection/host-app/exchange/email/advanced-email/advanced-email.module';
import { AdvancedParameterComponent as FilesetAdvancedParameterComponent } from 'app/business/protection/host-app/fileset/advanced-parameter/advanced-parameter.component';
import { AdvancedParameterModule as FilesetAdvancedParameterModule } from 'app/business/protection/host-app/fileset/advanced-parameter/advanced-parameter.module';
import { SelectFilesetListComponent } from 'app/business/protection/host-app/fileset/select-fileset-list/select-fileset-list.component';
import { SelectFilesetListModule } from 'app/business/protection/host-app/fileset/select-fileset-list/select-fileset-list.module';
import { SelectInstanceDatabaseComponent as GaussDBDWSSelectDatabaseComponent } from 'app/business/protection/host-app/gaussdb-dws/instance-database/select-instance-database/select-instance-database.component';
import { SelectGaussdbTListComponent } from 'app/business/protection/host-app/gaussdb-t/select-guassdb-t-list/select-guassdb-t-list.component';
import { SelectGaussdbTListModule } from 'app/business/protection/host-app/gaussdb-t/select-guassdb-t-list/select-guassdb-t-list.module';
import { ProtectionAdvanceComponent } from 'app/business/protection/host-app/mongodb/protection-advance/protection-advance.component';
import { ProtectionAdvanceModule } from 'app/business/protection/host-app/mongodb/protection-advance/protection-advance.module';
import { SelectInstanceDatabaseComponent } from 'app/business/protection/host-app/mysql/instance-database/select-instance-database/select-instance-database.component';
import { SelectInstanceDatabaseModule } from 'app/business/protection/host-app/mysql/instance-database/select-instance-database/select-instance-database.module';
import { AdvancedParameterComponent as OracleAdvancedParameterComponent } from 'app/business/protection/host-app/oracle/database-list/advanced-parameter/advanced-parameter.component';
import { AdvancedParameterModule as OracleAdvancedParameterModule } from 'app/business/protection/host-app/oracle/database-list/advanced-parameter/advanced-parameter.module';
import { SelectDatabaseListComponent as OracleSelectDatabaseListComponent } from 'app/business/protection/host-app/oracle/database-list/select-database-list/select-database-list.component';
import { SelectDatabaseListModule as OracleSelectDatabaseListModule } from 'app/business/protection/host-app/oracle/database-list/select-database-list/select-database-list.module';
import { SelectInstanceDatabaseComponent as SQLServerSelectDatabaseComponent } from 'app/business/protection/host-app/sql-server/select-instance-database/select-instance-database.component';
import { AdvancedParameterComponent as TDSQLAdvancedParameterComponent } from 'app/business/protection/host-app/tdsql/advanced-parameter/advanced-parameter.component';
import { AdvancedParameterModule as TDSQLAdvancedParameterModule } from 'app/business/protection/host-app/tdsql/advanced-parameter/advanced-parameter.module';
import { VolumeAdvancedParameterComponent } from 'app/business/protection/host-app/volume/volume-advanced-parameter/volume-advanced-parameter.component';
import { SelectDoradoListComponent } from 'app/business/protection/storage/dorado-file-system/select-dorado-list/select-dorado-list.component';
import { SelectDoradoListModule } from 'app/business/protection/storage/dorado-file-system/select-dorado-list/select-dorado-list.module';
import { AdvancedParameterComponent as NasSharedAdvancedParameterComponent } from 'app/business/protection/storage/nas-shared/advanced-parameter/advanced-parameter.component';
import { AdvancedParameterModule as NasSharedAdvancedParameterModule } from 'app/business/protection/storage/nas-shared/advanced-parameter/advanced-parameter.module';
import { SelectNasSharedListComponent } from 'app/business/protection/storage/nas-shared/select-nas-shared-list/select-nas-shared-list.component';
import { SelectNasSharedListModule } from 'app/business/protection/storage/nas-shared/select-nas-shared-list/select-nas-shared-list.module';
import { ObjectAdvancedParameterComponent } from 'app/business/protection/storage/object/object-service/object-advanced-parameter/object-advanced-parameter.component';
import { ObjectAdvancedParameterModule } from 'app/business/protection/storage/object/object-service/object-advanced-parameter/object-advanced-parameter.module';
import { FusionAdvancedParameterComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/fusion-advanced-parameter/fusion-advanced-parameter.component';
import { SelectDatabaseListComponent as FusionSelectDatabaseListComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/select-database-list/select-database-list.component';
import { AdvancedParamComponent } from 'app/business/protection/virtualization/kubernetes-container/advanced-param/advanced-param.component';
import { AdvancedParamModule } from 'app/business/protection/virtualization/kubernetes-container/advanced-param/advanced-param.module';
import { AdvancedParameterComponent as KubAdvancedParameterComponentComponent } from 'app/business/protection/virtualization/kubernetes/base-template/advanced-parameter/advanced-parameter.component';
import { SelectDatabaseListComponent as KubernetesSelectDatabaseListComponent } from 'app/business/protection/virtualization/kubernetes/base-template/select-database-list/select-database-list.component';
import { ProtectionAdvancedComponent } from 'app/business/protection/virtualization/virtualization-base/protection-advanced/protection-advanced.component';
import { ProtectionAdvancedModule } from 'app/business/protection/virtualization/virtualization-base/protection-advanced/protection-advanced.module';
import { ProtectionObjectComponent } from 'app/business/protection/virtualization/virtualization-base/protection-object/protection-object.component';
import { ProtectionObjectModule } from 'app/business/protection/virtualization/virtualization-base/protection-object/protection-object.module';
import { AdvancedComponent as VmAdvancedComponent } from 'app/business/protection/virtualization/vmware/vm/advanced/advanced.component';
import { AdvancedModule as VmAdvancedModule } from 'app/business/protection/virtualization/vmware/vm/advanced/advanced.module';
import { SelectObjectsComponent as VmSelectObjectsComponent } from 'app/business/protection/virtualization/vmware/vm/select-objects/select-objects.component';
import { SelectObjectsModule as VmSelectObjectsModule } from 'app/business/protection/virtualization/vmware/vm/select-objects/select-objects.module';
import { ProtectResourceCategory } from 'app/shared';
import { SelectSlaComponent } from '../components/protect/select-sla/select-sla.component';
import { SelectSlaModule } from '../components/protect/select-sla/select-sla.module';
import { ReplicaAdvancedParameterComponent } from '../components/replica-advanced-parameter/replica-advanced-parameter.component';
import { ReplicaAdvancedParameterModule } from '../components/replica-advanced-parameter/replica-advanced-parameter.module';
import { DataMap } from './data-map.config';

const modules = [
  SelectSlaModule,
  SelectFilesetListModule,
  OracleSelectDatabaseListModule,
  OracleAdvancedParameterModule,
  FilesetAdvancedParameterModule,
  VmSelectObjectsModule,
  VmAdvancedModule,
  SelectCopyResourceModule,
  SelectDoradoListModule,
  SelectNasSharedListModule,
  NasSharedAdvancedParameterModule,
  HDFSAdvancedParameterModule,
  HDFSSelectFilesetsListModule,
  HBaseSelectBackupSetListModule,
  SelectGaussdbTListModule,
  SelectInstanceDatabaseModule,
  MysqlAdvancedParameterModule,
  SelectPoModule,
  AdvancedModule,
  AdvancedParamModule,
  LearningConfigModule,
  ProtectionAdvanceModule,
  ActiveDirectoryAdvancedModule,
  ObjectAdvancedParameterModule,
  ProtectionObjectModule,
  ProtectionAdvancedModule,
  ObjectAdvancedParameterModule,
  AdvancedExchangeModule,
  ProtectionAdvancedModule,
  ApsAdvanceParameterModule,
  ApsProtectSelectModule,
  AdvancedEmailModule,
  TDSQLAdvancedParameterModule,
  ReplicaAdvancedParameterModule
];

const BASIC_CONFIG = [
  {
    component: SelectSlaComponent
  }
];
export const PROTECTION_CONFIG: any = {
  [DataMap.Resource_Type.APSCloudServer.value]: {
    steps: [
      {
        component: ApsProtectSelectComponent
      },
      ...BASIC_CONFIG,
      {
        component: ApsAdvanceParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.APSZone.value]: {
    steps: [
      {
        component: ApsProtectSelectComponent
      },
      ...BASIC_CONFIG,
      {
        component: ApsAdvanceParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.APSResourceSet.value]: {
    steps: [
      {
        component: ApsProtectSelectComponent
      },
      ...BASIC_CONFIG,
      {
        component: ApsAdvanceParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.ObjectSet.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG,
      {
        component: ObjectAdvancedParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.ActiveDirectory.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG,
      {
        component: ActiveDirectoryAdvancedComponent
      }
    ]
  },
  [DataMap.Resource_Type.volume.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG,
      {
        component: VolumeAdvancedParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.tidbCluster.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [DataMap.Resource_Type.tidbDatabase.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [ProtectResourceCategory.GeneralDB]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [ProtectResourceCategory.fileset]: {
    steps: [
      {
        component: SelectFilesetListComponent
      },
      ...BASIC_CONFIG,
      {
        component: FilesetAdvancedParameterComponent
      }
    ]
  },
  [ProtectResourceCategory.filesets]: {
    steps: [
      {
        component: SelectFilesetListComponent
      },
      ...BASIC_CONFIG,
      {
        component: FilesetAdvancedParameterComponent
      }
    ]
  },
  [ProtectResourceCategory.oracle]: {
    steps: [
      {
        component: OracleSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: OracleAdvancedParameterComponent
      }
    ]
  },
  [ProtectResourceCategory.oracles]: {
    steps: [
      {
        component: OracleSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: OracleAdvancedParameterComponent
      }
    ]
  },
  [ProtectResourceCategory.vmware]: {
    steps: [
      {
        component: VmSelectObjectsComponent
      },
      ...BASIC_CONFIG,
      {
        component: VmAdvancedComponent
      }
    ]
  },
  [ProtectResourceCategory.vmwares]: {
    steps: [
      {
        component: VmSelectObjectsComponent
      },
      ...BASIC_CONFIG,
      {
        component: VmAdvancedComponent
      }
    ]
  },
  [ProtectResourceCategory.esix]: {
    steps: [
      {
        component: VmSelectObjectsComponent
      },
      ...BASIC_CONFIG,
      {
        component: VmAdvancedComponent
      }
    ]
  },
  [ProtectResourceCategory.cluster]: {
    steps: [
      {
        component: VmSelectObjectsComponent
      },
      ...BASIC_CONFIG,
      {
        component: VmAdvancedComponent
      }
    ]
  },
  [DataMap.Resource_Type.cNwareVm.value]: {
    steps: [
      {
        component: ProtectionObjectComponent
      },
      ...BASIC_CONFIG,
      {
        component: ProtectionAdvancedComponent
      }
    ]
  },
  [DataMap.Resource_Type.cNwareHost.value]: {
    steps: [
      {
        component: ProtectionObjectComponent
      },
      ...BASIC_CONFIG,
      {
        component: ProtectionAdvancedComponent
      }
    ]
  },
  [DataMap.Resource_Type.cNwareCluster.value]: {
    steps: [
      {
        component: ProtectionObjectComponent
      },
      ...BASIC_CONFIG,
      {
        component: ProtectionAdvancedComponent
      }
    ]
  },
  [DataMap.Resource_Type.Project.value]: {
    steps: [
      {
        component: SelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: CloudStackAdvancedParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.CloudHost.value]: {
    steps: [
      {
        component: SelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: CloudStackAdvancedParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.fusionComputeVirtualMachine.value]: {
    steps: [
      {
        component: FusionSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: FusionAdvancedParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.fusionComputeCNA.value]: {
    steps: [
      {
        component: FusionSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: FusionAdvancedParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.fusionComputeCluster.value]: {
    steps: [
      {
        component: FusionSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: FusionAdvancedParameterComponent
      }
    ]
  },
  [ProtectResourceCategory.Replica]: {
    steps: [
      {
        component: SelectCopyResourceComponent
      },
      ...BASIC_CONFIG,
      {
        component: ReplicaAdvancedParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.NASFileSystem.value]: {
    steps: [
      { component: SelectDoradoListComponent },
      ...BASIC_CONFIG,
      {
        component: NasSharedAdvancedParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.LocalFileSystem.value]: {
    steps: [
      { component: SelectDoradoListComponent },
      ...BASIC_CONFIG,
      { component: LearningConfigComponent }
    ]
  },
  [DataMap.Resource_Type.LocalLun.value]: {
    steps: [{ component: SelectDoradoListComponent }, ...BASIC_CONFIG]
  },
  [ProtectResourceCategory.NASShare]: {
    steps: [
      {
        component: SelectNasSharedListComponent
      },
      ...BASIC_CONFIG,
      {
        component: NasSharedAdvancedParameterComponent
      }
    ]
  },
  [ProtectResourceCategory.HDFS]: {
    steps: [
      {
        component: HDFSSelectFilesetsListComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [ProtectResourceCategory.HBase]: {
    steps: [
      {
        component: HBaseSelectBackupSetListComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [DataMap.Resource_Type.GaussDB_T.value]: {
    steps: [
      {
        component: SelectGaussdbTListComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [DataMap.Resource_Type.gaussdbTSingle.value]: {
    steps: [
      {
        component: SelectGaussdbTListComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [DataMap.Resource_Type.MySQL.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG,
      {
        component: MysqlAdvancedParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.generalDatabase.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [ProtectResourceCategory.db2]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [DataMap.Resource_Type.PostgreSQL.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },

  [DataMap.Resource_Type.KingBase.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [DataMap.Resource_Type.Redis.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [ProtectResourceCategory.OpenGauss]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },

  [ProtectResourceCategory.Dameng]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [ProtectResourceCategory.GaussDBDWS]: {
    steps: [
      {
        component: GaussDBDWSSelectDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [ProtectResourceCategory.SQLServer]: {
    steps: [
      {
        component: SQLServerSelectDatabaseComponent
      },
      ...BASIC_CONFIG,
      {
        component: MysqlAdvancedParameterComponent
      }
    ]
  },
  [ProtectResourceCategory.ClickHouse]: {
    steps: [
      {
        component: ClickHouseDataBaseSelectDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [ProtectResourceCategory.ClickHouseDatabase]: {
    steps: [
      {
        component: ClickHouseDataBaseSelectDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [ProtectResourceCategory.ClickHouseTableset]: {
    steps: [
      {
        component: ClickHouseTablesetSelectDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [DataMap.Resource_Type.KubernetesStatefulset.value]: {
    steps: [
      {
        component: KubernetesSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: KubAdvancedParameterComponentComponent
      }
    ]
  },
  [DataMap.Resource_Type.KubernetesNamespace.value]: {
    steps: [
      {
        component: KubernetesSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: KubAdvancedParameterComponentComponent
      }
    ]
  },
  [DataMap.Resource_Type.kubernetesNamespaceCommon.value]: {
    steps: [
      {
        component: KubernetesSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: AdvancedParamComponent
      }
    ]
  },
  [DataMap.Resource_Type.kubernetesDatasetCommon.value]: {
    steps: [
      {
        component: KubernetesSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: AdvancedParamComponent
      }
    ]
  },
  [ProtectResourceCategory.GaussdbForOpengauss]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [DataMap.Resource_Type.openStackProject.value]: {
    steps: [
      {
        component: SelectPoComponent
      },
      ...BASIC_CONFIG,
      {
        component: AdvancedComponent
      }
    ]
  },
  [DataMap.Resource_Type.openStackCloudServer.value]: {
    steps: [
      {
        component: SelectPoComponent
      },
      ...BASIC_CONFIG,
      {
        component: AdvancedComponent
      }
    ]
  },
  [DataMap.Resource_Type.MongoDB.value]: {
    steps: [
      {
        component: KubernetesSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: ProtectionAdvanceComponent
      }
    ]
  },
  [DataMap.Resource_Type.MongodbClusterInstance.value]: {
    steps: [
      {
        component: KubernetesSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: ProtectionAdvanceComponent
      }
    ]
  },
  [DataMap.Resource_Type.MongodbSingleInstance.value]: {
    steps: [
      {
        component: KubernetesSelectDatabaseListComponent
      },
      ...BASIC_CONFIG,
      {
        component: ProtectionAdvanceComponent
      }
    ]
  },
  [DataMap.Resource_Type.ExchangeDataBase.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG,
      {
        component: AdvancedExchangeComponent
      }
    ]
  },
  [DataMap.Resource_Type.Exchange.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG,
      {
        component: AdvancedExchangeComponent
      }
    ]
  },
  [DataMap.Resource_Type.ExchangeEmail.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG,
      {
        component: AdvancedEmailComponent
      }
    ]
  },
  [DataMap.Resource_Type.vmGroup.value]: {
    steps: [
      {
        component: KubernetesSelectDatabaseListComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [DataMap.Resource_Type.tdsqlInstance.value]: {
    steps: [
      {
        component: SelectInstanceDatabaseComponent
      },
      ...BASIC_CONFIG,
      {
        component: TDSQLAdvancedParameterComponent
      }
    ]
  },
  [DataMap.Resource_Type.hyperVHost.value]: {
    steps: [
      {
        component: ProtectionObjectComponent
      },
      ...BASIC_CONFIG,
      {
        component: ProtectionAdvancedComponent
      }
    ]
  },
  [DataMap.Resource_Type.hyperVVm.value]: {
    steps: [
      {
        component: ProtectionObjectComponent
      },
      ...BASIC_CONFIG
    ]
  },
  [DataMap.Resource_Type.ndmp.value]: {
    steps: [
      { component: SelectDoradoListComponent },
      ...BASIC_CONFIG,
      {
        component: NasSharedAdvancedParameterComponent
      }
    ]
  }
};

@NgModule({
  exports: modules
})
export class ProtectConfigModule {}
