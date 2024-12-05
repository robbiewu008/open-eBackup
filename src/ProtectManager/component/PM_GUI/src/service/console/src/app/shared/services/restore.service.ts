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
import { CommonModule, DatePipe } from '@angular/common';
import { Component, Injectable, NgModule } from '@angular/core';
import { MessageboxService } from '@iux/live';
import { ObjectLevelRestoreComponent } from 'app/business/protection/application/active-directory/object-level-restore/object-level-restore.component';
import { ObjectLevelRestoreModule } from 'app/business/protection/application/active-directory/object-level-restore/object-level-restore.module';
import { RestoreComponent as ADRestoreComponent } from 'app/business/protection/application/active-directory/restore/restore.component';
import { RestoreModule as ADRestoreModule } from 'app/business/protection/application/active-directory/restore/restore.module';
import { SaphanaRestoreComponent } from 'app/business/protection/application/saphana/restore/saphana-restore.component';
import { SaphanaRestoreModule } from 'app/business/protection/application/saphana/restore/saphana-restore.module';
import { BackupsetRestoreComponent as ElasticsearchBackupSetRestoreComponent } from 'app/business/protection/big-data/elasticSearch/backupset-restore/backupset-restore.component';
import { BackupsetRestoreModule as ElasticsearchBackupSetRestoreModule } from 'app/business/protection/big-data/elasticSearch/backupset-restore/backupset-restore.module';
import { BackupSetRestoreComponent } from 'app/business/protection/big-data/hbase/backup-set/copy-data/backup-set-restore/backup-set-restore.component';
import { BackupSetRestoreModule } from 'app/business/protection/big-data/hbase/backup-set/copy-data/backup-set-restore/backup-set-restore.module';
import { HdfsFilesetRestoreComponent } from 'app/business/protection/big-data/hdfs/filesets/copy-data/hdfs-fileset-restore/hdfs-fileset-restore.component';
import { HdfsFilesetRestoreModule } from 'app/business/protection/big-data/hdfs/filesets/copy-data/hdfs-fileset-restore/hdfs-fileset-restore.module';
import { BackupsetRestoreComponent as HiveBackupsetRestoreComponent } from 'app/business/protection/big-data/hive/backupset-restore/backupset-restore.component';
import { BackupsetRestoreModule as HiveBackupsetRestoreModule } from 'app/business/protection/big-data/hive/backupset-restore/backupset-restore.module';
import { ApsDiskRestoreComponent } from 'app/business/protection/cloud/apsara-stack/aps-disk-restore/aps-disk-restore.component';
import { ApsDiskRestoreModule } from 'app/business/protection/cloud/apsara-stack/aps-disk-restore/aps-disk-restore.module';
import { ApsRestoreComponent } from 'app/business/protection/cloud/apsara-stack/aps-restore/aps-restore.component';
import { ApsRestoreModule } from 'app/business/protection/cloud/apsara-stack/aps-restore/aps-restore.module';
import { HCSRestoreComponent } from 'app/business/protection/cloud/huawei-stack/stack-list/copy-data/hcs-restore/hcs-restore.component';
import { HCSRestoreModule } from 'app/business/protection/cloud/huawei-stack/stack-list/copy-data/hcs-restore/hcs-restore.module';
import { HCSStoreResourceModule } from 'app/business/protection/cloud/huawei-stack/stack-list/environment-info/basic-info/store-resource/hcs-store-resource.module';
import { CopyRestoreComponent } from 'app/business/protection/cloud/openstack/restore/copy-restore/copy-restore.component';
import { CopyRestoreModule } from 'app/business/protection/cloud/openstack/restore/copy-restore/copy-restore.module';
import { RestoreComponent } from 'app/business/protection/cloud/openstack/restore/restore.component';
import { RestoreModule as OpenStackRestoreModule } from 'app/business/protection/cloud/openstack/restore/restore.module';
import { ClickHouseRestoreComponent } from 'app/business/protection/host-app/click-house/copy-data/click-house-restore/click-house-restore.component';
import { ClickHouseRestoreModule } from 'app/business/protection/host-app/click-house/copy-data/click-house-restore/click-house-restore.module';
import { DamengClusterRestoreComponent } from 'app/business/protection/host-app/dameng/copy-data/dameng-cluster-restore/dameng-cluster-restore.component';
import { DamengRestoreComponent } from 'app/business/protection/host-app/dameng/copy-data/dameng-restore/dameng-restore.component';
import { DamengRestoreModule } from 'app/business/protection/host-app/dameng/copy-data/dameng-restore/dameng-restore.module';
import { DbTwoRestoreComponent } from 'app/business/protection/host-app/db-two/db-two-restore/db-two-restore.component';
import { DbTwoRestoreModule } from 'app/business/protection/host-app/db-two/db-two-restore/db-two-restore.module';
import { RestoreComponent as ExchangeDatabaseGroupRestoreComponent } from 'app/business/protection/host-app/exchange/database-group-restore/restore.component';
import { RestoreModule as ExchangeDatabaseGroupRestoreModule } from 'app/business/protection/host-app/exchange/database-group-restore/restore.module';
import { ExchangeCopyDataModule } from 'app/business/protection/host-app/exchange/database/exchange-copy-data/exchange-copy-data.module';
import { UserLevelRestoreComponent } from 'app/business/protection/host-app/exchange/database/restore/user-level-restore.component';
import { UserLevelRestoreModule } from 'app/business/protection/host-app/exchange/database/restore/user-level-restore.module';
import { EmailLevelRestoreComponent } from 'app/business/protection/host-app/exchange/email/email-level-restore/email-level-restore.component';
import { EmailLevelRestoreModule } from 'app/business/protection/host-app/exchange/email/email-level-restore/email-level-restore.module';
import { FilesetRestoreComponent } from 'app/business/protection/host-app/fileset/fileset-restore/fileset-restore.component';
import { FilesetRestoreModule } from 'app/business/protection/host-app/fileset/fileset-restore/fileset-restore.module';
import { ClusterRestoreComponent as DWSClusterRestoreComponent } from 'app/business/protection/host-app/gaussdb-dws/restore-cluster/restore-cluster.component';
import { ClusterRestoreModule as DWSClusterRestoreModule } from 'app/business/protection/host-app/gaussdb-dws/restore-cluster/restore-cluster.module';
import { DatabaseRestoreComponent as DWSDatabaseRestoreComponent } from 'app/business/protection/host-app/gaussdb-dws/restore-database/restore-database.component';
import { DatabaseRestoreModule as DWSDatabaseRestoreModule } from 'app/business/protection/host-app/gaussdb-dws/restore-database/restore-database.module';
import { ModalRestoreComponent } from 'app/business/protection/host-app/gaussdb-dws/restore-modal/restore-modal.component';
import { ModalRestoreModule } from 'app/business/protection/host-app/gaussdb-dws/restore-modal/restore-modal.module';
import { TableRestoreComponent as DWSTableRestoreComponent } from 'app/business/protection/host-app/gaussdb-dws/restore-table/restore-table.component';
import { TableRestoreModule as DWSTableRestoreModule } from 'app/business/protection/host-app/gaussdb-dws/restore-table/restore-table.module';
import { GaussdbRestoreComponent } from 'app/business/protection/host-app/gaussdb-for-opengauss/gaussdb-for-openguss-restore/gaussdb-restore.component';
import { GaussdbRestoreModule } from 'app/business/protection/host-app/gaussdb-for-opengauss/gaussdb-for-openguss-restore/gaussdb-restore.module';
import { GaussDBTRestoreComponent } from 'app/business/protection/host-app/gaussdb-t/copy-data/gaussdb-t-restore/gauss-t-restore.component';
import { GaussDBTRestoreModule } from 'app/business/protection/host-app/gaussdb-t/copy-data/gaussdb-t-restore/gaussdb-t-restore.module';
import { GeneralDatabaseRestoreComponent } from 'app/business/protection/host-app/general-database/general-database-restore/general-database-restore.component';
import { GeneralDatabaseRestoreModule } from 'app/business/protection/host-app/general-database/general-database-restore/general-database-restore.module';
import { GoldendbRestoreComponent } from 'app/business/protection/host-app/goldendb/goldendb-restore/goldendb-restore.component';
import { GoldendbRestoreModule } from 'app/business/protection/host-app/goldendb/goldendb-restore/goldendb-restore.module';
import { InformixRestoreComponent } from 'app/business/protection/host-app/informix/restore/informix-restore.component';
import { InformixRestoreModule } from 'app/business/protection/host-app/informix/restore/informix-restore.module';
import { KingBaseRestoreComponent } from 'app/business/protection/host-app/king-base/instance-database/copy-data/king-base-restore/king-base-restore.component';
import { KingBaseRestoreModule } from 'app/business/protection/host-app/king-base/instance-database/copy-data/king-base-restore/king-base-restore.module';
import { GaussdbRestoreComponent as LightCloudGaussdbRestoreComponent } from 'app/business/protection/host-app/light-cloud-gaussdb/restore/gaussdb-restore.component';
import { GaussdbRestoreModule as LightCloudGaussdbRestoreModule } from 'app/business/protection/host-app/light-cloud-gaussdb/restore/gaussdb-restore.module';
import { RestoreComponent as MongoDBRestoreComponent } from 'app/business/protection/host-app/mongodb/restore/restore.component';
import { RestoreModule as MongoDBRestoreModule } from 'app/business/protection/host-app/mongodb/restore/restore.module';
import { MysqlRestoreComponent } from 'app/business/protection/host-app/mysql/instance-database/copy-data/mysql-restore/mysql-restore.component';
import { MysqlRestoreModule } from 'app/business/protection/host-app/mysql/instance-database/copy-data/mysql-restore/mysql-restore.module';
import { OceanBaseRestoreComponent } from 'app/business/protection/host-app/ocean-base/ocean-base-restore/ocean-base-restore.component';
import { OceanBaseRestoreModule } from 'app/business/protection/host-app/ocean-base/ocean-base-restore/ocean-base-restore.module';
import { DatabaseRestoreComponent } from 'app/business/protection/host-app/opengauss/base-template/copy-data/database-restore/database-restore.component';
import { InstanceRestoreComponent } from 'app/business/protection/host-app/opengauss/base-template/copy-data/instance-restore/instance-restore.component';
import { OracleRestoreComponent } from 'app/business/protection/host-app/oracle/database-list/copy-data/today/oracle-restore/oracle-restore.component';
import { OracleRestoreModule } from 'app/business/protection/host-app/oracle/database-list/copy-data/today/oracle-restore/oracle-restore.module';
import { OracleSingleFileRestoreComponent } from 'app/business/protection/host-app/oracle/database-list/restore/single-file-restore/oracle-single-file-restore.component';
import { OracleSingleFileRestoreModule } from 'app/business/protection/host-app/oracle/database-list/restore/single-file-restore/oracle-single-file-restore.module';
import { TableLevelRestoreComponent as OracleTableLevelRestoreComponent } from 'app/business/protection/host-app/oracle/database-list/restore/table-level-restore/table-level-restore.component';
import { OracleTableLevelRestoreModule } from 'app/business/protection/host-app/oracle/database-list/restore/table-level-restore/table-level-restore.module';
import { PdbSetRestoreComponent } from 'app/business/protection/host-app/oracle/pdb-set-list/pdb-set-restore/pdb-set-restore.component';
import { PdbSetRestoreModule } from 'app/business/protection/host-app/oracle/pdb-set-list/pdb-set-restore/pdb-set-restore.module';
import { PostgreSqlRestoreComponent } from 'app/business/protection/host-app/postgre-sql/instance-database/copy-data/postgre-sql-restore/postgre-sql-restore.component';
import { PostgreSqlRestoreModule } from 'app/business/protection/host-app/postgre-sql/instance-database/copy-data/postgre-sql-restore/postgre-sql-restore.module';
import { RestoreComponent as AntDBRestoreComponent } from 'app/business/protection/database/ant-db/restore/restore.component';
import { RestoreModule as AntDBRestoreModule } from 'app/business/protection/database/ant-db/restore/restore.module';
import { RedisRestoreComponent } from 'app/business/protection/host-app/redis/copy-data/redis-restore/redis-restore.component';
import { RedisRestoreModule } from 'app/business/protection/host-app/redis/copy-data/redis-restore/redis-restore.module';
import { SQLServerAlwaysOnComponent } from 'app/business/protection/host-app/sql-server/alwayson-restore/alwayson-restore.component';
import { SQLServerAlwaysonModule } from 'app/business/protection/host-app/sql-server/alwayson-restore/alwayson-restore.module';
import { InstanceRestoreComponent as SQLServerInstanceRestoreComponent } from 'app/business/protection/host-app/sql-server/instance-restore/instance-restore.component';
import { InstanceRestoreModule as SQLServerInstanceRestoreModule } from 'app/business/protection/host-app/sql-server/instance-restore/instance-restore.module';
import { SQLServerRestoreComponent } from 'app/business/protection/host-app/sql-server/sql-server-restore/sql-server-restore.component';
import { SQLServerRestoreModule } from 'app/business/protection/host-app/sql-server/sql-server-restore/sql-server-restore.module';
import { RestoreComponent as TdsqlDistributedInstanceRestoreComponent } from 'app/business/protection/host-app/tdsql/dirstibuted-instance/restore/restore.component';
import { RestoreModule as TdsqlDistributedInstanceRestoreModule } from 'app/business/protection/host-app/tdsql/dirstibuted-instance/restore/restore.module';
import { TdsqlRestoreComponent } from 'app/business/protection/host-app/tdsql/tdsql-restore/tdsql-restore.component';
import { TdsqlRestoreModule } from 'app/business/protection/host-app/tdsql/tdsql-restore/tdsql-restore.module';
import { TidbRestoreComponent } from 'app/business/protection/host-app/tidb/tidb-restore/tidb-restore.component';
import { TidbRestoreModule } from 'app/business/protection/host-app/tidb/tidb-restore/tidb-restore.module';
import { VolumeRestoreComponent } from 'app/business/protection/host-app/volume/restore-volume/volume-restore.component';
import { VolumeRestoreModule } from 'app/business/protection/host-app/volume/restore-volume/volume-restore.module';
import { RestoreCommonshareComponent } from 'app/business/protection/storage/commonshare/restore-commonshare/restore-commonshare.component';
import { RestoreCommonshareModule } from 'app/business/protection/storage/commonshare/restore-commonshare/restore-commonshare.module';
import { LocalFileSystemRestoreComponent } from 'app/business/protection/storage/local-file-system/local-file-system-restore/local-file-system-restore.component';
import { LocalFileSystemRestoreModule } from 'app/business/protection/storage/local-file-system/local-file-system-restore/local-file-system-restore.module';
import { ObjectRestoreComponent } from 'app/business/protection/storage/object/object-service/object-restore/object-restore.component';
import { CnwareRestoreComponent } from 'app/business/protection/virtualization/cnware/cnware-restore/cnware-restore.component';
import { CnwareRestoreModule } from 'app/business/protection/virtualization/cnware/cnware-restore/cnware-restore.module';
import { FusionComputeDiskRestoreComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/copy-data/fusion-compute-disk-restore/fusion-compute-disk-restore.component';
import { FusionComputeDiskRestoreModule } from 'app/business/protection/virtualization/fusion-compute/fusion-list/copy-data/fusion-compute-disk-restore/fusion-compute-disk-restore.module';
import { FusionComputeFileRestoreComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/copy-data/fusion-compute-file-restore/fusion-compute-file-restore.component';
import { FusionComputeFileRestoreModule } from 'app/business/protection/virtualization/fusion-compute/fusion-list/copy-data/fusion-compute-file-restore/fusion-compute-file-restore.module';
import { DiskRestoreComponent as HypervDiskRestoreComponent } from 'app/business/protection/virtualization/hyper-v/disk-restore/disk-restore.component';
import { DiskRestoreModule as HypervDiskRestoreModule } from 'app/business/protection/virtualization/hyper-v/disk-restore/disk-restore.module';
import { HypervRestoreComponent } from 'app/business/protection/virtualization/hyper-v/hyperv-restore/hyperv-restore.component';
import { HypervRestoreModule } from 'app/business/protection/virtualization/hyper-v/hyperv-restore/hyperv-restore.module';
import { NamespaceRestoreComponent } from 'app/business/protection/virtualization/kubernetes-container/namespace-restore/namespace-restore.component';
import { NamespaceRestoreModule } from 'app/business/protection/virtualization/kubernetes-container/namespace-restore/namespace-restore.module';
import { PvcRestoreComponent } from 'app/business/protection/virtualization/kubernetes-container/pvc-restore/pvc-restore.component';
import { PvcRestoreModule } from 'app/business/protection/virtualization/kubernetes-container/pvc-restore/pvc-restore.module';
import { KubernetesRestoreComponent } from 'app/business/protection/virtualization/kubernetes/statefulset/kubernetes-restore/kubernetes-restore.component';
import { KubernetesRestoreModule } from 'app/business/protection/virtualization/kubernetes/statefulset/kubernetes-restore/kubernetes-restore.module';
import { DiskRestoreComponent } from 'app/business/protection/virtualization/vmware/vm/copy-data/disk-restore/disk-restore.component';
import { DiskRestoreModule } from 'app/business/protection/virtualization/vmware/vm/copy-data/disk-restore/disk-restore.module';
import { FileRestoreComponent } from 'app/business/protection/virtualization/vmware/vm/copy-data/file-restore/file-restore.component';
import { FileRestoreModule } from 'app/business/protection/virtualization/vmware/vm/copy-data/file-restore/file-restore.module';
import { VmRestoreComponent } from 'app/business/protection/virtualization/vmware/vm/copy-data/vm-restore/vm-restore.component';
import { VmRestoreModule } from 'app/business/protection/virtualization/vmware/vm/copy-data/vm-restore/vm-restore.module';
import {
  CookieService,
  I18NService,
  isDatabaseApp,
  LANGUAGE,
  MODAL_COMMON,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type,
  WarningMessageService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, defer, get, includes, isEmpty, isFunction } from 'lodash';
import { combineLatest } from 'rxjs';
import { DoradoNasRestoreComponent } from '../components/dorado-nas-restore/dorado-nas-restore.component';
import { DoradoNasRestoreModule } from '../components/dorado-nas-restore/dorado-nas-restore.module';
import { FileExplorerLevelRestoreComponent } from '../components/file-explorer-level-restore/file-explorer-level-restore.component';
import { FileExplorerLevelRestoreModule } from '../components/file-explorer-level-restore/file-explorer-level-restore.module';
import { FileLevelRestoreComponent } from '../components/file-level-restore/file-level-restore.component';
import { FileLevelRestoreModule } from '../components/file-level-restore/file-level-restore.module';
import { VmFileLevelRestoreComponent } from '../components/vm-file-level-restore/vm-file-level-restore.component';
import { VmFileLevelRestoreModule } from '../components/vm-file-level-restore/vm-file-level-restore.module';
import { MESSAGE_BOX_ACTION, SYSTEM_TIME } from '../consts';
import { DataMap } from './../consts/data-map.config';
import { SaponoracleRestoreComponent } from '../../business/protection/application/saponoracle/restore/saponoracle-restore.component';
import { SaponoracleRestoreModule } from '../../business/protection/application/saponoracle/restore/saponoracle-restore.module';

export interface RestoreParams {
  childResType: any; // 资源类型的字资源的分类，比如VM副本的恢复分为VM、disk、file
  copyData;
  header?: string;
  restoreType?: RestoreType; // 恢复类型
  onOk?: () => void; // restore回调
  restoreLevel?: string; // 恢复粒度
  isMessageBox?: boolean; //恢复弹窗开关
}

@Injectable({
  providedIn: 'root'
})
export class RestoreService {
  private browserActionComponent = BrowserActionComponent;
  private beforeIntoRestoreTipsComponent = BeforeIntoRestoreTipsComponent;
  constructor(
    private drawModalService: DrawModalService,
    private i18n: I18NService,
    private cookieService: CookieService,
    private warningMessageService: WarningMessageService,
    private messageBox: MessageboxService
  ) {}

  restore(option: RestoreParams, drillCb?) {
    if (
      includes(
        [DataMap.Resource_Type.generalDatabase.value],
        option.childResType
      )
    ) {
      defer(() => {
        const resource = JSON.parse(
          get(option.copyData, 'resource_properties', '{}')
        );
        const enbaleMessagBox = get(resource, 'extendInfo.isNeedCloseDb');

        if (enbaleMessagBox === '1') {
          this.messageBox.info({
            lvOkText: this.i18n.get('common_close_label'),
            lvWidth: MODAL_COMMON.smallWidth + 40,
            lvHeader: this.i18n.get('common_alarms_info_label'),
            lvContent: this.beforeIntoRestoreTipsComponent
          });
        }
      });
    }
    if (
      includes(
        [
          DataMap.Resource_Type.MySQLDatabase.value,
          DataMap.Resource_Type.MySQLInstance.value,
          DataMap.Resource_Type.MySQLClusterInstance.value
        ],
        option.childResType
      )
    ) {
      defer(() => {
        this.messageBox.info({
          lvOkText: this.i18n.get('common_close_label'),
          lvWidth: MODAL_COMMON.smallWidth,
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvContent: this.i18n.get(
            'protection_mysql_instance_restore_tips_label'
          )
        });
      });
    }
    if (
      includes(
        [
          DataMap.Resource_Type.saphanaDatabase.value,
          DataMap.Resource_Type.saponoracleDatabase.value
        ],
        option.childResType
      )
    ) {
      defer(() => {
        this.messageBox.info({
          lvOkText: this.i18n.get('common_close_label'),
          lvWidth: MODAL_COMMON.smallWidth,
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvContent: this.i18n.get(
            'protection_saphana_instance_restore_tips_label'
          )
        });
      });
    }
    if (
      includes(
        [DataMap.Resource_Type.ActiveDirectory.value],
        option.childResType
      ) &&
      option.restoreType === RestoreType.CommonRestore
    ) {
      defer(() => {
        this.messageBox.info({
          lvOkText: this.i18n.get('common_close_label'),
          lvWidth: MODAL_COMMON.smallWidth,
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvContent: this.beforeIntoRestoreTipsComponent,
          lvComponentParams: {
            data: option
          }
        });
      });
    }

    if (
      includes(
        [
          DataMap.Resource_Type.Dameng_singleNode.value,
          DataMap.Resource_Type.Dameng_cluster.value,
          DataMap.Resource_Type.informixClusterInstance.value,
          DataMap.Resource_Type.informixInstance.value
        ],
        option.childResType
      )
    ) {
      defer(() => {
        this.messageBox.info({
          lvOkText: this.i18n.get('common_close_label'),
          lvWidth: MODAL_COMMON.smallWidth,
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvContent: this.i18n.get('common_database_restore_info_label')
        });
      });
    }
    if (
      includes(
        [
          DataMap.Resource_Type.PostgreSQLInstance.value,
          DataMap.Resource_Type.PostgreSQLClusterInstance.value,
          DataMap.Resource_Type.KingBaseInstance.value,
          DataMap.Resource_Type.KingBaseClusterInstance.value,
          DataMap.Resource_Type.MongodbSingleInstance.value,
          DataMap.Resource_Type.MongodbClusterInstance.value
        ],
        option.childResType
      )
    ) {
      defer(() => {
        this.messageBox.info({
          lvOkText: this.i18n.get('common_close_label'),
          lvWidth: MODAL_COMMON.smallWidth,
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvContent: this.i18n.get('common_instance_restore_info_label')
        });
      });
    }

    const modalExtParams = {};
    const childResType = option.childResType;
    switch (childResType) {
      case DataMap.Resource_Type.APSResourceSet.value:
      case DataMap.Resource_Type.APSZone.value:
        assign(modalExtParams, {
          lvContent: ApsRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        break;
      case DataMap.Resource_Type.APSCloudServer.value:
        if (option.copyData.diskRestore) {
          assign(modalExtParams, {
            lvHeader: this.i18n.get('common_disk_restore_label'),
            lvContent: ApsDiskRestoreComponent,
            lvWidth: MODAL_COMMON.largeWidth + 300
          });
          assign(option, {
            restoreType: RestoreV2Type.CommonRestore
          });
        } else {
          assign(modalExtParams, {
            lvContent: ApsRestoreComponent,
            lvWidth: MODAL_COMMON.normalWidth + 100
          });
          assign(option, {
            restoreType: RestoreV2Type.CommonRestore
          });
        }
        break;
      case DataMap.Resource_Type.ActiveDirectory.value:
        const isCommonRestore =
          option.restoreType === RestoreType.CommonRestore;
        assign(modalExtParams, {
          lvContent: isCommonRestore
            ? ADRestoreComponent
            : ObjectLevelRestoreComponent,
          lvWidth: isCommonRestore
            ? MODAL_COMMON.normalWidth + 100
            : MODAL_COMMON.xLargeWidth,
          lvHeader: isCommonRestore
            ? this.i18n.get('common_restore_label')
            : this.i18n.get('common_object_level_restore_label')
        });
        break;
      case DataMap.Resource_Type.ObjectSet.value:
        assign(modalExtParams, {
          lvContent: ObjectRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          header: this.i18n.get('protection_object_level_restore_label')
        });
        break;
      case DataMap.Resource_Type.commonShare.value:
        assign(modalExtParams, {
          lvContent: RestoreCommonshareComponent,
          lvWidth: MODAL_COMMON.normalWidth
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.tidbDatabase.value:
      case DataMap.Resource_Type.tidbTable.value:
      case DataMap.Resource_Type.tidbCluster.value:
        assign(modalExtParams, {
          lvContent: TidbRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.OceanBaseCluster.value:
      case DataMap.Resource_Type.OceanBase.value:
      case DataMap.Resource_Type.OceanBaseTenant.value:
        assign(modalExtParams, {
          lvContent: OceanBaseRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 300
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.tdsqlDistributedInstance.value:
        assign(modalExtParams, {
          lvContent: TdsqlDistributedInstanceRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 150
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.tdsqlInstance.value:
        assign(modalExtParams, {
          lvContent: TdsqlRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.informixInstance.value:
      case DataMap.Resource_Type.informixClusterInstance.value:
        assign(modalExtParams, {
          lvContent: InformixRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.gaussdbForOpengaussInstance.value:
        assign(modalExtParams, {
          lvContent: GaussdbRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.lightCloudGaussdbInstance.value:
        assign(modalExtParams, {
          lvContent: LightCloudGaussdbRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.oraclePDB.value:
        assign(modalExtParams, {
          lvContent: PdbSetRestoreComponent,
          lvWidth: this.i18n.isEn
            ? MODAL_COMMON.largeWidth
            : MODAL_COMMON.normalWidth + 200,
          lvHeader: this.i18n.get('common_restore_label')
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.oracle.value:
      case DataMap.Resource_Type.oracleCluster.value:
        if (option.restoreType === RestoreType.FileRestore) {
          assign(modalExtParams, {
            lvContent: OracleTableLevelRestoreComponent,
            lvWidth: MODAL_COMMON.xLargeWidth,
            lvHeader: this.i18n.get('protection_table_level_restore_label')
          });
          break;
        }
        assign(modalExtParams, {
          lvContent: OracleRestoreComponent,
          lvWidth: this.i18n.isEn
            ? MODAL_COMMON.largeWidth
            : MODAL_COMMON.normalWidth + 200,
          lvHeader:
            option.restoreType === RestoreType.InstanceRestore
              ? this.i18n.get('common_live_restore_job_label')
              : this.i18n.get('common_restore_label')
        });
        assign(option, {
          restoreType:
            option.restoreType === RestoreType.InstanceRestore
              ? RestoreV2Type.InstanceRestore
              : RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.virtualMachine.value:
        assign(modalExtParams, {
          lvContent: option.copyData.fileRestore
            ? FileRestoreComponent
            : option.copyData.diskRestore
            ? DiskRestoreComponent
            : VmRestoreComponent,
          lvWidth:
            this.i18n.language === 'en-us'
              ? MODAL_COMMON.largeWidth + 105
              : MODAL_COMMON.largeWidth + 75
        });
        break;
      case DataMap.Resource_Type.cNwareVm.value:
        assign(modalExtParams, {
          lvContent: CnwareRestoreComponent,
          lvWidth:
            option.copyData.diskRestore ||
            option.restoreType === RestoreType.InstanceRestore
              ? MODAL_COMMON.largeWidth + 300
              : MODAL_COMMON.largeWidth + 100,
          lvHeader: option.copyData.diskRestore
            ? this.i18n.get('common_disk_restore_label')
            : option.restoreType === RestoreType.InstanceRestore
            ? this.i18n.get('common_live_restore_job_label')
            : this.i18n.get('common_restore_label')
        });
        assign(option, {
          restoreType:
            option.restoreType === RestoreType.InstanceRestore
              ? RestoreV2Type.InstanceRestore
              : RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.NASFileSystem.value:
      case DataMap.Resource_Type.NASShare.value:
      case DataMap.Resource_Type.ndmp.value:
        assign(modalExtParams, {
          lvContent: DoradoNasRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.HDFSFileset.value:
        assign(modalExtParams, {
          lvContent: HdfsFilesetRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.HBaseBackupSet.value:
        assign(modalExtParams, {
          lvContent: BackupSetRestoreComponent
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.LocalFileSystem.value:
      case DataMap.Resource_Type.LocalLun.value:
        assign(modalExtParams, {
          lvContent: LocalFileSystemRestoreComponent
        });
        if (
          this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
        ) {
          assign(option, {
            restoreType: RestoreV2Type.CommonRestore
          });
        }
        break;
      case DataMap.Resource_Type.gaussdbTSingle.value:
      case DataMap.Resource_Type.GaussDB_T.value:
        assign(modalExtParams, {
          lvContent: GaussDBTRestoreComponent
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.MySQL.value:
      case DataMap.Resource_Type.MySQLInstance.value:
      case DataMap.Resource_Type.MySQLClusterInstance.value:
      case DataMap.Resource_Type.MySQLDatabase.value:
        assign(modalExtParams, {
          lvContent: MysqlRestoreComponent
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.HiveBackupSet.value:
        assign(modalExtParams, {
          lvContent: HiveBackupsetRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        break;
      case DataMap.Resource_Type.fileset.value:
        assign(modalExtParams, {
          lvContent: FilesetRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        break;
      case DataMap.Resource_Type.volume.value:
        assign(modalExtParams, {
          lvContent: VolumeRestoreComponent,
          lvWidth: MODAL_COMMON.largeWidth
        });
        break;
      case DataMap.Resource_Type.DWS_Cluster.value:
        assign(modalExtParams, {
          lvContent: DWSClusterRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        break;
      case DataMap.Resource_Type.DWS_Database.value:
        assign(modalExtParams, {
          lvContent: DWSDatabaseRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        break;
      case DataMap.Resource_Type.DWS_Table.value:
        assign(modalExtParams, {
          lvContent: DWSTableRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        break;
      case DataMap.Resource_Type.dbTwoDatabase.value:
      case DataMap.Resource_Type.dbTwoTableSet.value:
        assign(modalExtParams, {
          lvContent: DbTwoRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.SQLServerDatabase.value:
        assign(modalExtParams, {
          lvContent: SQLServerRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        break;
      case DataMap.Resource_Type.SQLServerClusterInstance.value:
        assign(modalExtParams, {
          lvContent: SQLServerInstanceRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        break;
      case DataMap.Resource_Type.SQLServerInstance.value:
        assign(modalExtParams, {
          lvContent: SQLServerInstanceRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        break;
      case DataMap.Resource_Type.SQLServerGroup.value:
        assign(modalExtParams, {
          lvContent: SQLServerAlwaysOnComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        break;
      case DataMap.Resource_Type.ElasticsearchBackupSet.value:
        assign(modalExtParams, {
          lvContent: ElasticsearchBackupSetRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        break;
      case DataMap.Resource_Type.ClickHouse.value:
        assign(modalExtParams, {
          lvContent: ClickHouseRestoreComponent
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.Redis.value:
        assign(modalExtParams, {
          lvContent: RedisRestoreComponent
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.AntDBInstance.value:
      case DataMap.Resource_Type.AntDBClusterInstance.value:
        assign(modalExtParams, {
          lvContent: AntDBRestoreComponent
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.PostgreSQLInstance.value:
      case DataMap.Resource_Type.PostgreSQLClusterInstance.value:
        assign(modalExtParams, {
          lvContent: PostgreSqlRestoreComponent
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.HCSCloudHost.value:
        assign(modalExtParams, {
          lvContent: HCSRestoreComponent,
          lvWidth: MODAL_COMMON.largeWidth + 200
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.KingBaseInstance.value:
      case DataMap.Resource_Type.KingBaseClusterInstance.value:
        assign(modalExtParams, {
          lvContent: KingBaseRestoreComponent
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.FusionCompute.value:
      case DataMap.Resource_Type.fusionOne.value:
        assign(modalExtParams, {
          lvContent: option.copyData.diskRestore
            ? FusionComputeDiskRestoreComponent
            : FusionComputeFileRestoreComponent,
          lvWidth:
            this.i18n.language === 'en-us'
              ? MODAL_COMMON.largeWidth + 200
              : MODAL_COMMON.largeWidth + 120
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.OpenGauss_instance.value:
        assign(modalExtParams, {
          lvContent: InstanceRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.OpenGauss_database.value:
        assign(modalExtParams, {
          lvContent: DatabaseRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.Dameng_singleNode.value:
        assign(modalExtParams, {
          lvContent: DamengRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.Dameng_cluster.value:
        assign(modalExtParams, {
          lvContent: DamengClusterRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.KubernetesStatefulset.value:
        assign(modalExtParams, {
          lvContent: KubernetesRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.openStackCloudServer.value:
        if (option.copyData.diskRestore) {
          assign(modalExtParams, {
            lvHeader: this.i18n.get('common_disk_restore_label'),
            lvContent: RestoreComponent,
            lvWidth: MODAL_COMMON.largeWidth + 300
          });
          assign(option, {
            restoreType: RestoreV2Type.CommonRestore
          });
        } else {
          assign(modalExtParams, {
            lvContent: CopyRestoreComponent,
            lvWidth: MODAL_COMMON.normalWidth + 100
          });
          assign(option, {
            restoreType: RestoreV2Type.CommonRestore
          });
        }
        break;
      case DataMap.Resource_Type.generalDatabase.value:
        assign(modalExtParams, {
          lvContent: GeneralDatabaseRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.goldendbInstance.value:
        assign(modalExtParams, {
          lvContent: GoldendbRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.MongodbClusterInstance.value:
      case DataMap.Resource_Type.MongodbSingleInstance.value:
        assign(modalExtParams, {
          lvContent: MongoDBRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.ExchangeEmail.value:
        assign(modalExtParams, {
          lvContent: EmailLevelRestoreComponent,
          lvWidth:
            option.restoreType === RestoreType.FileRestore
              ? MODAL_COMMON.xLargeWidth
              : MODAL_COMMON.normalWidth
        });
        if (option.restoreType === RestoreType.FileRestore) {
          assign(modalExtParams, {
            lvHeader: this.i18n.get('common_email_level_restore_label')
          });
        }
        assign(option, {
          restoreType:
            option.restoreType === RestoreType.FileRestore
              ? RestoreV2Type.FileRestore
              : RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.ExchangeSingle.value:
      case DataMap.Resource_Type.ExchangeGroup.value:
        assign(modalExtParams, {
          lvContent: ExchangeDatabaseGroupRestoreComponent,
          lvWidth: MODAL_COMMON.xLargeWidth
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        if (
          option.copyData.backup_type ===
            DataMap.CopyData_Backup_Type.log.value ||
          includes(
            [
              DataMap.CopyData_generatedType.cloudArchival.value,
              DataMap.CopyData_generatedType.tapeArchival.value
            ],
            option.copyData.generated_by
          )
        ) {
          // Exchange单机/DAG日志副本细粒度不需要选择数据库
          modalExtParams['lvWidth'] = MODAL_COMMON.normalWidth + 100;
        }
        break;
      case DataMap.Resource_Type.ExchangeDataBase.value:
        assign(modalExtParams, {
          lvContent: ExchangeDatabaseGroupRestoreComponent,
          lvWidth:
            option.restoreType === RestoreType.FileRestore
              ? MODAL_COMMON.xLargeWidth
              : MODAL_COMMON.normalWidth + 100
        });
        if (option.restoreType === RestoreType.FileRestore) {
          assign(modalExtParams, {
            lvContent: UserLevelRestoreComponent,
            lvHeader: this.i18n.get('common_user_level_restore_label')
          });
          if (
            option.copyData.backup_type ===
            DataMap.CopyData_Backup_Type.log.value
          ) {
            // Exchange数据库日志副本细粒度不需要选择数据库
            modalExtParams['lvWidth'] = MODAL_COMMON.normalWidth;
          }
        }
        assign(option, {
          restoreType:
            option.restoreType === RestoreType.FileRestore
              ? RestoreV2Type.FileRestore
              : RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.kubernetesNamespaceCommon.value:
      case DataMap.Resource_Type.kubernetesDatasetCommon.value:
        if (option.copyData.diskRestore) {
          assign(modalExtParams, {
            lvContent: PvcRestoreComponent,
            lvWidth: MODAL_COMMON.largeWidth,
            lvHeader: this.i18n.get('protection_pvc_recovery_label')
          });
          assign(option, {
            restoreType: RestoreV2Type.CommonRestore
          });
        } else {
          assign(modalExtParams, {
            lvContent: NamespaceRestoreComponent,
            lvWidth: MODAL_COMMON.normalWidth + 100
          });
          assign(option, {
            restoreType: RestoreV2Type.CommonRestore
          });
        }
        break;
      case DataMap.Resource_Type.hyperVVm.value:
        assign(modalExtParams, {
          lvContent: option.copyData?.diskRestore
            ? HypervDiskRestoreComponent
            : HypervRestoreComponent,
          lvWidth: option.copyData.diskRestore
            ? MODAL_COMMON.largeWidth + 150
            : MODAL_COMMON.normalWidth + 200
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.saphanaDatabase.value:
        assign(modalExtParams, {
          lvContent: SaphanaRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.saponoracleDatabase.value:
        assign(modalExtParams, {
          lvContent: SaponoracleRestoreComponent,
          lvWidth: MODAL_COMMON.normalWidth + 100
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
      case DataMap.Resource_Type.nutanixVm.value:
        assign(modalExtParams, {
          lvContent: CnwareRestoreComponent,
          lvWidth: MODAL_COMMON.largeWidth + 100,
          lvHeader: this.i18n.get('common_restore_label')
        });
        assign(option, {
          restoreType: RestoreV2Type.CommonRestore
        });
        break;
    }
    this.drawModalService.create(
      assign(
        {},
        MODAL_COMMON.generateDrawerOptions(),
        {
          lvHeader:
            [
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.virtualMachine.value
            ].includes(option.childResType) &&
            (!isEmpty(option.copyData.fineGrainedData) ||
              option.copyData.fileRestore)
              ? this.i18n.get('protection_restore_file_label')
              : option.restoreType === RestoreType.InstanceRestore
              ? this.i18n.get('common_live_restore_job_label')
              : [
                  DataMap.Resource_Type.FusionCompute.value,
                  DataMap.Resource_Type.hyperVVm.value,
                  DataMap.Resource_Type.fusionOne.value
                ].includes(option.childResType) && option.copyData.diskRestore
              ? this.i18n.get('common_disk_restore_label')
              : this.i18n.get('common_restore_label'),
          lvWidth: MODAL_COMMON.normalWidth,
          lvOkDisabled:
            [
              DataMap.Resource_Type.ABBackupClient.value,
              DataMap.Resource_Type.msVirtualMachine.value
            ].includes(childResType) ||
            (!(
              childResType === DataMap.Resource_Type.virtualMachine.value &&
              option.copyData.diskRestore &&
              option.copyData.generated_by !==
                DataMap.CopyData_generatedType.replicate.value
            ) &&
              !includes(
                [
                  DataMap.Resource_Type.HBaseBackupSet.value,
                  DataMap.Resource_Type.NASShare.value,
                  DataMap.Resource_Type.NASFileSystem.value,
                  DataMap.Resource_Type.ndmp.value,
                  DataMap.Resource_Type.LocalFileSystem.value,
                  DataMap.Resource_Type.LocalLun.value,
                  DataMap.Resource_Type.KubernetesStatefulset.value,
                  DataMap.Resource_Type.FusionCompute.value,
                  DataMap.Resource_Type.fusionOne.value,
                  DataMap.Resource_Type.PostgreSQLInstance.value,
                  DataMap.Resource_Type.PostgreSQLClusterInstance.value,
                  DataMap.Resource_Type.MongodbClusterInstance.value,
                  DataMap.Resource_Type.MongodbSingleInstance.value,
                  DataMap.Resource_Type.ExchangeEmail.value
                ],
                childResType
              )) ||
            (includes(
              [
                DataMap.Resource_Type.FusionCompute.value,
                DataMap.Resource_Type.fusionOne.value
              ],
              childResType
            ) &&
              option.copyData.diskRestore),
          lvAfterOpen: modal => {
            const content = modal.getContentComponent();
            const modalIns = modal.getInstance();

            switch (option.childResType) {
              case DataMap.Resource_Type.DB2.value:
              case DataMap.Resource_Type.fileset.value:
                const combined: any = combineLatest(
                  content.formGroup.statusChanges,
                  content.fileValid$
                );
                combined.subscribe(latestValues => {
                  const [formGroupStatus, fileValid] = latestValues;
                  modalIns.lvOkDisabled = !(
                    formGroupStatus === 'VALID' && fileValid
                  );
                });
                break;
              case DataMap.Resource_Type.ABBackupClient.value:
                const combinedHost: any = combineLatest(
                  content.formGroup.statusChanges,
                  content.originalVolumeValid$,
                  content.newVolumeValid$
                );
                combinedHost.subscribe(latestValues => {
                  const [
                    formGroupStatus,
                    originalVolumeValid,
                    newVolumeValid
                  ] = latestValues;
                  modalIns.lvOkDisabled = !(
                    formGroupStatus === 'VALID' &&
                    originalVolumeValid &&
                    newVolumeValid
                  );
                });
                break;
              case DataMap.Resource_Type.SQLServer.value:
                const combinedSqlServer: any = combineLatest(
                  content.formGroup.statusChanges,
                  content.dataFileValid$,
                  content.logFileValid$
                );
                combinedSqlServer.subscribe(latestValues => {
                  const [
                    formGroupStatus,
                    dataFileValid,
                    logFileValid
                  ] = latestValues;
                  modalIns.lvOkDisabled = !(
                    formGroupStatus === 'VALID' &&
                    dataFileValid &&
                    logFileValid
                  );
                });
                break;
              case DataMap.Resource_Type.HDFSFileset.value:
                const combinedValid: any = combineLatest(
                  content.formGroup.statusChanges,
                  content.fileValid$
                );
                combinedValid.subscribe(res => {
                  const [formGroupStatus, fileValid] = res;
                  modalIns.lvOkDisabled = !(
                    formGroupStatus === 'VALID' && fileValid
                  );
                });
                break;
              case DataMap.Resource_Type.KubernetesStatefulset.value:
                const combinedStatefulset: any = combineLatest([
                  content.formGroup.statusChanges,
                  content.pvcValid$
                ]);
                combinedStatefulset.subscribe(res => {
                  const [formGroupStatus, pvcValid] = res;
                  modalIns.lvOkDisabled = !(
                    formGroupStatus === 'VALID' && pvcValid
                  );
                });
                break;
              case DataMap.Resource_Type.FusionCompute.value:
              case DataMap.Resource_Type.fusionOne.value:
                if (option.copyData?.diskRestore) {
                  content.disk$.subscribe(res => {
                    modalIns.lvOkDisabled = !res;
                  });
                } else {
                  content.formGroup.statusChanges.subscribe(res => {
                    modalIns.lvOkDisabled = res !== 'VALID';
                  });
                }
                break;
              case DataMap.Resource_Type.HCSCloudHost.value:
                content.disk$.subscribe(res => {
                  modalIns.lvOkDisabled = !res;
                });
                break;
              case DataMap.Resource_Type.openStackCloudServer.value:
                if (option.copyData?.diskRestore) {
                  content.disk$.subscribe(res => {
                    modalIns.lvOkDisabled = !res;
                  });
                } else {
                  content.valid$?.subscribe(res => {
                    modalIns.lvOkDisabled = !res;
                  });
                }
                break;
              case DataMap.Resource_Type.APSCloudServer.value:
                if (option.copyData?.diskRestore) {
                  content.disk$.subscribe(res => {
                    modalIns.lvOkDisabled = !res;
                  });
                } else {
                  content.valid$?.subscribe(res => {
                    modalIns.lvOkDisabled = !res;
                  });
                }
                break;
              case DataMap.Resource_Type.kubernetesNamespaceCommon.value:
              case DataMap.Resource_Type.kubernetesDatasetCommon.value:
                if (option.copyData?.diskRestore) {
                  content.pvc$.subscribe(res => {
                    modalIns.lvOkDisabled = !res;
                  });
                } else {
                  content.formGroup.statusChanges.subscribe(res => {
                    modalIns.lvOkDisabled = res !== 'VALID';
                  });
                }
                content.formGroup.updateValueAndValidity();
                break;
              case DataMap.Resource_Type.cNwareVm.value:
              case DataMap.Resource_Type.nutanixVm.value:
                content.valid$?.subscribe(res => {
                  modalIns.lvOkDisabled = !res;
                });
                break;
              case DataMap.Resource_Type.hyperVVm.value:
                if (option.copyData.diskRestore) {
                  content.disk$.subscribe(res => {
                    modalIns.lvOkDisabled = !res;
                  });
                } else {
                  const combinedHyperv: any = combineLatest([
                    content.formGroup.statusChanges,
                    content.valid$
                  ]);
                  combinedHyperv.subscribe(res => {
                    const [formGroupStatus, valid] = res;
                    modalIns.lvOkDisabled = !(
                      formGroupStatus === 'VALID' && valid
                    );
                  });
                  content.valid$.next(true);
                }
                break;
              case DataMap.Resource_Type.oracle.value:
              case DataMap.Resource_Type.oracleCluster.value:
              case DataMap.Resource_Type.oraclePDB.value:
              case DataMap.Resource_Type.ActiveDirectory.value:
                if (option.restoreType === RestoreType.FileRestore) {
                  content.valid$.subscribe(res => {
                    modalIns.lvOkDisabled = res;
                  });
                  break;
                } else {
                  content.formGroup.statusChanges.subscribe(res => {
                    modalIns.lvOkDisabled = res !== 'VALID';
                  });
                  break;
                }
              default:
                content.formGroup.statusChanges.subscribe(res => {
                  modalIns.lvOkDisabled = res !== 'VALID';
                });
                break;
            }
            if (childResType === DataMap.Resource_Type.ABBackupClient.value) {
              content.newVolumeValid$.next(true);
              content.formGroup.get('targetHost').updateValueAndValidity();
            } else if (childResType === DataMap.Resource_Type.DB2.value) {
              if (
                option.copyData &&
                option.copyData.generated_by ===
                  DataMap.CopyData_generatedType.replicate.value
              ) {
                content.fileValid$.next(false);
              } else {
                content.fileValid$.next(true);
              }
            } else if (childResType === DataMap.Resource_Type.SQLServer.value) {
              content.dataFileValid$.next(true);
              content.logFileValid$.next(true);
            } else if (childResType === DataMap.Resource_Type.fileset.value) {
              content.formGroup.get('selectedHost')?.updateValueAndValidity();
              if (option.copyData && option.copyData.isSingleRestore) {
                content.fileValid$.next(false);
              } else {
                content.fileValid$.next(true);
              }
            } else if (
              childResType === DataMap.Resource_Type.KubernetesStatefulset.value
            ) {
              content.formGroup.updateValueAndValidity();
              content.validPvc();
            } else if (
              childResType === DataMap.Resource_Type.virtualMachine.value &&
              option.copyData.diskRestore &&
              option.copyData.generated_by !==
                DataMap.CopyData_generatedType.replicate.value
            ) {
              try {
                const allDiskDatas = JSON.parse(option.copyData.properties)
                  .vmware_metadata?.disk_info;
                if (
                  !isEmpty(allDiskDatas) &&
                  allDiskDatas[0].DSNAME?.startsWith('TemporaryDsForIR')
                ) {
                  content.formGroup.updateValueAndValidity();
                }
              } catch (error) {}
            }
          },
          lvComponentParams: {
            rowCopy: {
              ...option.copyData,
              environment_os_type: option.copyData.resource_properties
                ? JSON.parse(option.copyData.resource_properties)
                    .environment_os_type
                : option.copyData.environment_os_type
            },
            // 资源类型的字资源的分类，比如VM副本的恢复分为VM、disk、file
            childResType: option.childResType,
            restoreType: option.restoreType,
            isDrill: isFunction(drillCb)
          },
          lvOk: modal => {
            const content = modal.getContentComponent();
            // 演练配置参数调用
            if (isFunction(drillCb)) {
              const recoveryParams = isFunction(content.getParams)
                ? content.getParams()
                : {};
              drillCb(recoveryParams);
              return true;
            }
            if (isFunction(content.restore)) {
              return new Promise(resolve => {
                if (
                  childResType === DataMap.Resource_Type.HBaseBackupSet.value &&
                  option.copyData?.backup_type ===
                    DataMap.CopyData_Backup_Type.log.value
                ) {
                  this.messageBox.confirm({
                    lvDialogIcon: 'lv-icon-popup-danger-48',
                    lvWidth: MODAL_COMMON.smallWidth,
                    lvContent: this.i18n.get(
                      'explore_hdfs_log_restore_tips_label'
                    ),
                    lvCloseButtonDisplay: false,
                    lvOk: () => {
                      content.restore().subscribe({
                        next: () => {
                          resolve(true);
                          if (isFunction(option.onOk)) {
                            option.onOk();
                          }
                        },
                        error: () => {
                          resolve(false);
                        }
                      });
                    },
                    lvCancel: () => {
                      resolve(false);
                    }
                  });
                } else if (
                  childResType ===
                  DataMap.Resource_Type.SQLServerClusterInstance.value
                ) {
                  this.warningMessageService.create({
                    content: this.i18n.get(
                      'protection_sqlserver_clusterinstance_restore_warnning_label'
                    ),
                    onOK: () => {
                      content.restore().subscribe({
                        next: () => {
                          resolve(true);
                          if (isFunction(option.onOk)) {
                            option.onOk();
                          }
                        },
                        error: () => {
                          resolve(false);
                        }
                      });
                    },
                    onCancel: () => {
                      resolve(false);
                    },
                    lvAfterClose: result => {
                      if (
                        result &&
                        result.trigger === MESSAGE_BOX_ACTION.close
                      ) {
                        resolve(false);
                      }
                    }
                  });
                } else if (option.isMessageBox) {
                  let targetPath = '';
                  let targetPort = '';
                  let clusterName = [];
                  let version = 0;
                  if (isFunction(content.getTargetPath)) {
                    targetPath = content?.getTargetPath();
                  }
                  if (isFunction(content.getTargetPort)) {
                    targetPort = content?.getTargetPort();
                  }
                  if (isFunction(content.getClusterName)) {
                    clusterName = content?.getClusterName();
                  }
                  if (content?.version) {
                    version = content.version;
                  }
                  if (
                    includes(
                      [
                        DataMap.Resource_Type.dbTwoDatabase.value,
                        DataMap.Resource_Type.dbTwoTableSet.value,
                        DataMap.Resource_Type.goldendbInstance.value,
                        DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
                        DataMap.Resource_Type.lightCloudGaussdbInstance.value
                      ],
                      childResType
                    ) ||
                    (includes(
                      [
                        DataMap.Resource_Type.DWS_Cluster.value,
                        DataMap.Resource_Type.DWS_Schema.value,
                        DataMap.Resource_Type.DWS_Table.value
                      ],
                      childResType
                    ) &&
                      get(content?.getParams(), 'targetLocation') ===
                        RestoreV2LocationType.ORIGIN)
                  ) {
                    this.messageBox.danger({
                      lvHeader: this.i18n.get('common_restore_tips_label'),
                      lvContent: this.browserActionComponent,
                      lvWidth: MODAL_COMMON.smallWidth + 50,
                      lvCancelType: 'default',
                      lvOkType: 'primary',
                      lvComponentParams: {
                        targetPath: targetPath,
                        data: option,
                        location: get(content?.getParams(), 'targetLocation')
                      },
                      lvOk: () => {
                        content.restore().subscribe({
                          next: () => {
                            resolve(true);
                            if (isFunction(option.onOk)) {
                              option.onOk();
                            }
                          },
                          error: () => {
                            resolve(false);
                          }
                        });
                      },
                      lvCancel: () => {
                        resolve(false);
                      },
                      lvAfterClose: result => {
                        if (
                          result &&
                          result.trigger === MESSAGE_BOX_ACTION.close
                        ) {
                          resolve(false);
                        }
                      }
                    });
                  } else {
                    let isFileLevelRestore = false;
                    if (
                      includes(
                        [
                          DataMap.Resource_Type.oracle.value,
                          DataMap.Resource_Type.oracleCluster.value
                        ],
                        option.copyData.resource_sub_type
                      ) &&
                      option.restoreType === RestoreType.FileRestore
                    ) {
                      isFileLevelRestore = true;
                    }
                    this.messageBox.confirm({
                      lvHeader: this.i18n.get('common_restore_tips_label'),
                      lvDialogIcon: 'lv-icon-popup-danger-48',
                      lvContent: this.browserActionComponent,
                      lvWidth: includes(
                        [
                          DataMap.Resource_Type.tidbCluster.value,
                          DataMap.Resource_Type.tidbDatabase.value,
                          DataMap.Resource_Type.tidbTable.value
                        ],
                        option.copyData.resource_sub_type
                      )
                        ? MODAL_COMMON.normalWidth
                        : MODAL_COMMON.smallWidth + 50,
                      lvCancelType: 'default',
                      lvOkType: 'primary',
                      lvComponentParams: {
                        targetPath: targetPath,
                        data: option,
                        port: targetPort,
                        clusterInfo: clusterName,
                        version: version,
                        isFileRestore: isFileLevelRestore,
                        isVolume:
                          option.copyData.resource_sub_type ===
                          DataMap.Resource_Type.volume.value
                            ? content?.formGroup?.get('restorationType')
                                .value ===
                                DataMap.windowsVolumeBackupType.volume.value ||
                              (!content.isWindows &&
                                !content?.formGroup.get(
                                  'enable_bare_metal_restore'
                                ).value)
                            : false
                      },
                      lvOk: () => {
                        content.restore().subscribe({
                          next: () => {
                            resolve(true);
                            if (isFunction(option.onOk)) {
                              option.onOk();
                            }
                          },
                          error: () => {
                            resolve(false);
                          }
                        });
                      },
                      lvCancel: () => {
                        resolve(false);
                      },
                      lvAfterClose: result => {
                        if (
                          result &&
                          result.trigger === MESSAGE_BOX_ACTION.close
                        ) {
                          resolve(false);
                        }
                      }
                    });
                  }
                } else {
                  content.restore().subscribe({
                    next: () => {
                      resolve(true);
                      if (isFunction(option.onOk)) {
                        option.onOk();
                      }
                    },
                    error: () => {
                      resolve(false);
                    }
                  });
                }
              });
            }
          }
        },
        modalExtParams
      )
    );
  }

  restoreCommonShare(option: {
    copyData: object;
    isConfig: boolean;
    onOk?: () => void;
  }) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvModalKey: 'createStep1',
        lvWidth: MODAL_COMMON.largeWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('explore_commonshare_setting_shareinfo_label'),
        lvContent: RestoreCommonshareComponent,
        lvComponentParams: {
          copyData: option.copyData,
          isConfig: option.isConfig,
          refreshFunc: option.onOk
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RestoreCommonshareComponent;
          const modalIns = modal.getInstance();

          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RestoreCommonshareComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
              },
              error: () => {
                resolve(false);
              }
            });
          });
        }
      }
    });
  }

  singleFileRestore(option: RestoreParams) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvModalKey: 'singleFileRestore',
        lvWidth: MODAL_COMMON.largeWidth,
        lvOkDisabled: true,
        lvHeader: option.header,
        lvContent: OracleSingleFileRestoreComponent,
        lvComponentParams: {
          rowCopy: option.copyData
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as OracleSingleFileRestoreComponent;
          const modalIns = modal.getInstance();
          modalIns.lvOkDisabled = true;
          content.valid$.subscribe(res => {
            modalIns.lvOkDisabled = !res;
          });
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as OracleSingleFileRestoreComponent;
          const tips = this.i18n.get(
            'protection_oracle_single_file_restore_tips_content_label'
          );
          return new Promise(resolve => {
            this.drawModalService.create({
              ...MODAL_COMMON.generateDrawerOptions(),
              lvModalKey: 'copy-info-message',
              ...{
                lvType: 'dialog',
                lvDialogIcon: 'lv-icon-popup-danger-48',
                lvHeader: this.i18n.get(
                  'protection_oracle_single_file_restore_tips_header_label'
                ),
                lvContent: this.browserActionComponent,
                lvWidth: 500,
                lvOkType: 'primary',
                lvCancelType: 'default',
                lvOkDisabled: false,
                lvFocusButtonId: 'cancel',
                lvCloseButtonDisplay: true,
                lvComponentParams: {
                  tips
                },
                lvOk: () => {
                  content.onOK().subscribe({
                    next: () => {
                      resolve(true);
                    },
                    error: () => {
                      resolve(false);
                    }
                  });
                },
                lvCancel: () => {
                  resolve(false);
                },
                lvAfterClose: result => {
                  if (result && result.trigger === MESSAGE_BOX_ACTION.close) {
                    resolve(false);
                  }
                }
              }
            });
          });
        }
      }
    });
  }

  fileLevelRestore(option: RestoreParams) {
    if (
      includes(
        [DataMap.Resource_Type.Dameng_singleNode.value],
        option.childResType
      )
    ) {
      defer(() => {
        this.messageBox.info({
          lvOkText: this.i18n.get('common_close_label'),
          lvWidth: MODAL_COMMON.smallWidth,
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvContent: this.i18n.get('common_database_restore_info_label')
        });
      });
    }
    if (
      includes(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value,
          DataMap.Resource_Type.NASShare.value,
          DataMap.Resource_Type.HDFSFileset.value,
          DataMap.Resource_Type.HBaseBackupSet.value,
          DataMap.Resource_Type.HiveBackupSet.value,
          DataMap.Resource_Type.ElasticsearchBackupSet.value
        ],
        option.childResType
      )
    ) {
      assign(option, {
        restoreType: RestoreV2Type.FileRestore
      });
    }
    const isVm = includes(
      [
        DataMap.Resource_Type.virtualMachine.value,
        DataMap.Resource_Type.FusionCompute.value,
        DataMap.Resource_Type.fusionOne.value,
        DataMap.Resource_Type.HCSCloudHost.value,
        DataMap.Resource_Type.openStackCloudServer.value,
        DataMap.Resource_Type.APSCloudServer.value,
        DataMap.Resource_Type.cNwareVm.value,
        DataMap.Resource_Type.hyperVVm.value,
        DataMap.Resource_Type.nutanixVm.value
      ],
      option.childResType
    );
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader:
          option.header || this.i18n.get('common_file_level_restore_label'),
        lvWidth: isVm
          ? MODAL_COMMON.largeModal
          : option.childResType === DataMap.Resource_Type.ObjectSet.value
          ? MODAL_COMMON.largeWidth
          : option.copyData.isSearchRestore
          ? MODAL_COMMON.normalWidth + 100
          : this.i18n.language === LANGUAGE.CN
          ? MODAL_COMMON.largeWidth + 300
          : MODAL_COMMON.largeWidth + 350,
        lvContent: isVm
          ? VmFileLevelRestoreComponent
          : option.childResType === DataMap.Resource_Type.ObjectSet.value
          ? FileExplorerLevelRestoreComponent
          : FileLevelRestoreComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowCopy: {
            ...option.copyData,
            environment_os_type: option.copyData.resource_properties
              ? JSON.parse(option.copyData.resource_properties)
                  .environment_os_type
              : option.copyData.environment_os_type
          },
          childResType: option.childResType,
          restoreType: option.restoreType,
          restoreLevel: option?.restoreLevel
        },
        lvOk: modal => {
          const content = modal.getContentComponent();

          return new Promise(resolve => {
            if (this.cookieService.isCloudBackup) {
              this.warningMessageService.create({
                content: this.i18n.get(
                  'protection_file_level_restore_warn_label'
                ),
                onOK: () => {
                  content.onOK().subscribe({
                    next: () => {
                      resolve(true);
                      if (isFunction(option.onOk)) {
                        option.onOk();
                      }
                    },
                    error: () => resolve(false)
                  });
                },
                onCancel: () => resolve(false)
              });
            } else if (
              option.childResType ===
                DataMap.Resource_Type.HBaseBackupSet.value &&
              option.copyData?.backup_type ===
                DataMap.CopyData_Backup_Type.log.value
            ) {
              this.messageBox.confirm({
                lvDialogIcon: 'lv-icon-popup-danger-48',
                lvWidth: MODAL_COMMON.smallWidth,
                lvContent: this.i18n.get('explore_hdfs_log_restore_tips_label'),
                lvCloseButtonDisplay: false,
                lvOk: () => {
                  content.onOK().subscribe({
                    next: () => {
                      resolve(true);
                      if (isFunction(option.onOk)) {
                        option.onOk();
                      }
                    },
                    error: () => {
                      resolve(false);
                    }
                  });
                },
                lvCancel: () => {
                  resolve(false);
                }
              });
            } else if (
              content.targetParams?.restoreLocation ===
                RestoreV2LocationType.NEW &&
              includes(
                [
                  DataMap.Resource_Type.DWS_Cluster.value,
                  DataMap.Resource_Type.DWS_Schema.value
                ],
                option.childResType
              )
            ) {
              this.drawModalService.create({
                lvHeader: this.i18n.get('explore_confirm_restore_label'),
                lvContent: ModalRestoreComponent,
                lvWidth: MODAL_COMMON.normalWidth,
                lvComponentParams: {
                  data: content.targetParams,
                  path: content.originalSelection
                },
                lvOk: modal => {
                  const content = modal.getContentComponent() as ModalRestoreComponent;
                  content.onOK().subscribe({
                    next: () => {
                      resolve(true);
                      if (isFunction(option.onOk)) {
                        option.onOk();
                      }
                    },
                    error: () => resolve(false)
                  });
                },
                lvCancel: () => resolve(false),
                lvAfterClose: result => {
                  if (result && result.trigger === MESSAGE_BOX_ACTION.close) {
                    resolve(false);
                  }
                }
              });
            } else if (
              content.targetParams?.restoreLocation ===
                RestoreV2LocationType.ORIGIN &&
              includes(
                [
                  DataMap.Resource_Type.DWS_Cluster.value,
                  DataMap.Resource_Type.DWS_Schema.value,
                  DataMap.Resource_Type.DWS_Table.value
                ],
                option.childResType
              )
            ) {
              let tips = this.i18n.get(
                isDatabaseApp(option.childResType)
                  ? 'protection_database_filelevel_restore_tip_label'
                  : 'protection_filelevel_restore_tip_label',
                [content.getTargetPath().tips]
              );
              let targetPath;
              targetPath = content.getTargetPath().targetPath;
              this.messageBox.danger({
                lvHeader: this.i18n.get('common_restore_tips_label'),
                lvContent: this.browserActionComponent,
                lvWidth: MODAL_COMMON.smallWidth + 50,
                lvCancelType: 'default',
                lvOkType: 'primary',
                lvComponentParams: {
                  targetPath: targetPath,
                  data: option,
                  tips: tips,
                  isFileRestore: true
                },
                lvOk: () => {
                  content.onOK().subscribe({
                    next: () => {
                      resolve(true);
                      if (isFunction(option.onOk)) {
                        option.onOk();
                      }
                    },
                    error: () => {
                      resolve(false);
                    }
                  });
                },
                lvCancel: () => {
                  resolve(false);
                },
                lvAfterClose: result => {
                  if (result && result.trigger === MESSAGE_BOX_ACTION.close) {
                    resolve(false);
                  }
                }
              });
            } else {
              let notices;
              if (
                includes(
                  [
                    DataMap.Resource_Type.tidbCluster.value,
                    DataMap.Resource_Type.tidbDatabase.value,
                    DataMap.Resource_Type.tidbTable.value
                  ],
                  option.childResType
                )
              ) {
                notices = this.i18n.get('explore_tidb_restore_tip_label');
                if (content.version === 1) {
                  notices += this.i18n.get(
                    'explore_tidb_restore_version_tip_label'
                  );
                }
              } else {
                notices = '';
              }
              let tips = this.i18n.get(
                isDatabaseApp(option.childResType)
                  ? 'protection_database_filelevel_restore_tip_label'
                  : includes(
                      [DataMap.Resource_Type.NASShare.value],
                      option.childResType
                    )
                  ? 'common_nasshare_file_level_restore_to_location_tip_label'
                  : 'protection_filelevel_restore_tip_label',
                [content.getTargetPath().tips]
              );
              let targetPath;
              targetPath = content.getTargetPath().targetPath;
              this.messageBox.confirm({
                lvHeader: this.i18n.get('common_restore_tips_label'),
                lvDialogIcon: 'lv-icon-popup-danger-48',
                lvContent: this.browserActionComponent,
                lvWidth: includes(
                  [
                    DataMap.Resource_Type.tidbCluster.value,
                    DataMap.Resource_Type.tidbDatabase.value,
                    DataMap.Resource_Type.tidbTable.value
                  ],
                  option.copyData.resource_sub_type
                )
                  ? MODAL_COMMON.normalWidth
                  : MODAL_COMMON.smallWidth + 50,
                lvCancelType: 'default',
                lvOkType: 'primary',
                lvComponentParams: {
                  targetPath: targetPath,
                  data: option,
                  tips: tips,
                  isFileRestore: true,
                  location: content.targetParams?.restoreLocation
                },
                lvOk: () => {
                  content.onOK().subscribe({
                    next: () => {
                      resolve(true);
                      if (isFunction(option.onOk)) {
                        option.onOk();
                      }
                    },
                    error: () => {
                      resolve(false);
                    }
                  });
                },
                lvCancel: () => {
                  resolve(false);
                },
                lvAfterClose: result => {
                  if (result && result.trigger === MESSAGE_BOX_ACTION.close) {
                    resolve(false);
                  }
                }
              });
            }
          });
        }
      })
    );
  }
}

@NgModule({
  imports: [
    CommonModule,
    VmRestoreModule,
    CnwareRestoreModule,
    FileRestoreModule,
    DiskRestoreModule,
    OracleRestoreModule,
    DoradoNasRestoreModule,
    FileLevelRestoreModule,
    VmFileLevelRestoreModule,
    HdfsFilesetRestoreModule,
    BackupSetRestoreModule,
    LocalFileSystemRestoreModule,
    MysqlRestoreModule,
    RedisRestoreModule,
    HiveBackupsetRestoreModule,
    ElasticsearchBackupSetRestoreModule,
    GaussDBTRestoreModule,
    KubernetesRestoreModule,
    DWSClusterRestoreModule,
    DWSDatabaseRestoreModule,
    DWSTableRestoreModule,
    DamengRestoreModule,
    SQLServerRestoreModule,
    SQLServerInstanceRestoreModule,
    PostgreSqlRestoreModule,
    AntDBRestoreModule,
    HCSStoreResourceModule,
    FusionComputeFileRestoreModule,
    FusionComputeDiskRestoreModule,
    KingBaseRestoreModule,
    ModalRestoreModule,
    HCSRestoreModule,
    ClickHouseRestoreModule,
    MongoDBRestoreModule,
    SQLServerAlwaysonModule,
    GeneralDatabaseRestoreModule,
    GaussdbRestoreModule,
    DbTwoRestoreModule,
    GoldendbRestoreModule,
    OpenStackRestoreModule,
    InformixRestoreModule,
    LightCloudGaussdbRestoreModule,
    TdsqlRestoreModule,
    TdsqlDistributedInstanceRestoreModule,
    OceanBaseRestoreModule,
    TidbRestoreModule,
    NamespaceRestoreModule,
    PvcRestoreModule,
    CopyRestoreModule,
    FilesetRestoreModule,
    VolumeRestoreModule,
    RestoreCommonshareModule,
    ADRestoreModule,
    ExchangeCopyDataModule,
    ObjectLevelRestoreModule,
    ApsRestoreModule,
    ApsDiskRestoreModule,
    ExchangeDatabaseGroupRestoreModule,
    EmailLevelRestoreModule,
    UserLevelRestoreModule,
    HypervDiskRestoreModule,
    HypervRestoreModule,
    SaphanaRestoreModule,
    SaponoracleRestoreModule,
    FileExplorerLevelRestoreModule,
    OracleSingleFileRestoreModule,
    OracleTableLevelRestoreModule,
    PdbSetRestoreModule
  ],
  providers: [RestoreService]
})
export class RestoreModule {}

@Component({
  selector: 'aui-browser-action',
  template: `
    <div style="margin-bottom: 15px;" [innerHTML]="tips"></div>
    <div
      style="color: #f45c5e;position: relative;top: -15px"
      *ngIf="isCnwareDisk"
    >
      {{ cnwareDiskTip }}
    </div>
    <div
      *ngIf="isNdmp"
      style="color: #f45c5e;position: relative;top: -15px"
      [innerHTML]="ndmpWarn"
    ></div>
    <div *ngIf="targetPath">
      <span style="color: #aaafbc; margin-right:15px;"
        >{{ locationLable }}
      </span>
      <span>{{ targetPath }}</span>
    </div>
    <div *ngIf="isTidb" style="margin-top: 15px;" [innerHTML]="notices"></div>
  `,
  styles: [],
  providers: [DatePipe]
})
export class BrowserActionComponent {
  isFileRestore;
  isTidb;
  version;
  tips;
  notices;
  targetPath;
  location;
  data;
  isVolume;
  port; // tdsql需要对端口进行判断
  clusterInfo; // oecanbase要特殊展示选中的租户
  locationLable = this.i18n.get('protection_restore_target_label');
  isCnwareDisk = false; // cnware磁盘恢复提示
  cnwareDiskTip = this.i18n.get('protection_cnware_disk_restore_tip_label');
  // ndmp提示
  isNdmp = false;
  ndmpWarn = this.i18n.get('protection_ndmp_file_restore_warn_label');
  constructor(private i18n: I18NService, private datePipe: DatePipe) {}
  ngOnInit(): void {
    // NDMP新位置提示
    this.isNdmp =
      this.data?.childResType === DataMap.Resource_Type.ndmp.value &&
      this.data?.restoreType === RestoreV2Type.FileRestore &&
      this.location === RestoreV2LocationType.NEW;
    this.isCnwareDisk =
      this.data?.childResType === DataMap.Resource_Type.cNwareVm.value &&
      this.data?.copyData?.diskRestore === true;
    const restoreTips = [
      this.datePipe.transform(
        this.data.copyData.display_timestamp ||
          this.data?.copyData?.restoreTimeStamp * 1000,
        'yyyy-MM-dd HH:mm:ss'
      )
    ];
    if (
      includes(
        [DataMap.Resource_Type.HiveBackupSet.value],
        this.data?.childResType
      )
    ) {
      restoreTips.unshift(...this.clusterInfo);
    }
    if (
      includes(
        [
          DataMap.Resource_Type.OceanBaseCluster.value,
          DataMap.Resource_Type.OceanBaseTenant.value
        ],
        this.data?.childResType
      )
    ) {
      restoreTips.push(...this.clusterInfo);
    }
    this.isTidb = includes(
      [
        DataMap.Resource_Type.tidbCluster.value,
        DataMap.Resource_Type.tidbDatabase.value,
        DataMap.Resource_Type.tidbTable.value
      ],
      this.data?.childResType
    );
    if (this.isTidb) {
      this.notices = this.i18n.get('explore_tidb_restore_tip_label');
      if (this.version === 1) {
        this.notices += this.i18n.get('explore_tidb_restore_version_tip_label');
      }
    }
    if (this.isFileRestore) {
      if (
        this.data?.childResType === DataMap.Resource_Type.HiveBackupSet.value
      ) {
        this.tips = this.i18n.get(
          'protection_hive_filelevel_restore_tip_label'
        );
      }
      if (this.data?.childResType === DataMap.Resource_Type.ClickHouse.value) {
        this.tips = this.i18n.get(
          'common_clickhouse_filelevel_restore_tips_label'
        );
      }
      if (
        includes(
          [
            DataMap.Resource_Type.oracleCluster.value,
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oraclePDB.value
          ],
          this.data?.childResType
        )
      ) {
        this.tips = this.i18n.get(
          'common_restore_to_location_tip_label',
          restoreTips
        );
      }
      return;
    } else {
      this.tips = this.i18n.get(
        isDatabaseApp(this.data?.childResType)
          ? 'common_database_restore_to_location_tip_label'
          : includes(
              [
                DataMap.Resource_Type.APSCloudServer.value,
                DataMap.Resource_Type.openStackCloudServer.value,
                DataMap.Resource_Type.FusionCompute.value,
                DataMap.Resource_Type.fusionOne.value
              ],
              this.data?.childResType
            ) && this.data?.copyData?.diskRestore
          ? 'common_disk_restore_to_location_tip_label'
          : includes(
              [
                DataMap.Resource_Type.kubernetesNamespaceCommon.value,
                DataMap.Resource_Type.kubernetesDatasetCommon.value
              ],
              this.data?.childResType
            ) && this.data?.copyData?.diskRestore
          ? 'common_pvc_restore_to_location_tip_label'
          : includes(
              [DataMap.Resource_Type.HCSCloudHost.value],
              this.data?.childResType
            )
          ? 'common_cloud_disk_restore_to_location_tip_label'
          : includes(
              [
                DataMap.Resource_Type.informixInstance.value,
                DataMap.Resource_Type.informixClusterInstance.value,
                DataMap.Resource_Type.dbTwoDatabase.value,
                DataMap.Resource_Type.dbTwoTableSet.value,
                DataMap.Resource_Type.lightCloudGaussdbInstance.value
              ],
              this.data?.childResType
            )
          ? 'common_db_two_restore_to_location_tip_label'
          : includes(
              [DataMap.Resource_Type.gaussdbForOpengaussInstance.value],
              this.data?.childResType
            )
          ? 'common_db_restore_to_location_tip_label'
          : includes(
              [DataMap.Resource_Type.generalDatabase.value],
              this.data?.childResType
            )
          ? this.getGeneralDbTips()
          : includes(
              [DataMap.Resource_Type.goldendbInstance.value],
              this.data?.childResType
            )
          ? this.getGoldenDbTips()
          : includes(
              [DataMap.Resource_Type.tdsqlInstance.value],
              this.data?.childResType
            )
          ? this.getTdsqlTips()
          : includes(
              [
                DataMap.Resource_Type.OceanBaseCluster.value,
                DataMap.Resource_Type.OceanBaseTenant.value
              ],
              this.data?.childResType
            )
          ? 'common_oceanbase_restore_tips_label'
          : includes(
              [DataMap.Resource_Type.HiveBackupSet.value],
              this.data?.childResType
            )
          ? 'protection_hive_restore_label'
          : includes(
              [DataMap.Resource_Type.ClickHouse.value],
              this.data?.childResType
            )
          ? 'common_clickhouse_restore_tips_label'
          : includes(
              [DataMap.Resource_Type.ActiveDirectory.value],
              this.data?.childResType
            ) && this.data.restoreType === RestoreType.CommonRestore
          ? 'protection_system_status_restore_tips_label'
          : includes(
              [
                DataMap.Resource_Type.ExchangeSingle.value,
                DataMap.Resource_Type.ExchangeGroup.value,
                DataMap.Resource_Type.ExchangeDataBase.value,
                DataMap.Resource_Type.ExchangeEmail.value
              ],
              this.data?.childResType
            ) &&
            this.data.copyData.backup_type ===
              DataMap.CopyData_Backup_Type.log.value
          ? 'protection_exchange_log_backup_restore_tips_label'
          : includes(
              [DataMap.Resource_Type.ndmp.value],
              this.data?.childResType
            )
          ? 'common_ndmp_restore_to_location_tip_label'
          : includes(
              [DataMap.Resource_Type.NASShare.value],
              this.data?.childResType
            )
          ? 'common_nasshare_restore_to_location_tip_label'
          : includes(
              [DataMap.Resource_Type.volume.value],
              this.data?.childResType
            ) && this?.isVolume
          ? 'common_volume_restore_to_location_tip_label'
          : 'common_restore_to_location_tip_label',
        restoreTips
      );
    }
  }

  getGeneralDbTips() {
    const resource = JSON.parse(
      get(this.data?.copyData, 'resource_properties', '{}')
    );
    const enbaleMessagBox = get(resource, 'extendInfo.isNeedCloseDb');

    if (enbaleMessagBox === '1') {
      return 'common_general_db_restore_tip_label';
    } else {
      return 'common_hana_restore_to_location_tip_label';
    }
  }

  getGoldenDbTips() {
    if (this.location === RestoreV2LocationType.ORIGIN) {
      return 'common_db_restore_to_location_tip_label';
    } else {
      return 'common_goldendb_restore_to_location_tip_label';
    }
  }

  getTdsqlTips() {
    if (this.port === '4001')
      return 'protection_tdsql_instance_restore_warnning_label';
    else return 'common_restore_to_location_tip_label';
  }
}
@NgModule({
  imports: [CommonModule],
  declarations: [BrowserActionComponent]
})
export class BrowserActionModule {}

@Component({
  selector: 'aui-hana-restore-tips',
  template: `
    <span [innerHTML]="tips"></span>
  `,
  styles: [],
  providers: [DatePipe]
})
export class BeforeIntoRestoreTipsComponent {
  tips = this.i18n.get('explore_hana_restore_info_label');
  data;
  constructor(public i18n: I18NService) {}
  ngOnInit() {
    if (
      this.data?.childResType === DataMap.Resource_Type.ActiveDirectory.value
    ) {
      this.tips = this.i18n.get(
        'protection_active_directory_restore_tips_label'
      );
    }
  }
}
@NgModule({
  imports: [CommonModule],
  declarations: [BeforeIntoRestoreTipsComponent]
})
export class HanaRestoreTipsModule {}
