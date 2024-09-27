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
import { DatePipe } from '@angular/common';
import {
  Component,
  Input,
  OnChanges,
  OnDestroy,
  OnInit,
  SimpleChanges
} from '@angular/core';
import { MessageboxService, MessageService } from '@iux/live';
import {
  CommonShareRestoreApiService,
  CookieService,
  CopyControllerService,
  DataMapService,
  disableOpenstackVmRestore,
  disableOracleRestoreNewLocation,
  disableOracleRestoreOriginalLocation,
  disableValidCopyBtn,
  filterBackupType,
  filterGeneratedBy,
  getGeneralDatabaseConf,
  getLabelList,
  getPermissionMenuItem,
  hiddenDwsFileLevelRestore,
  hiddenHcsUserFileLevelRestore,
  hiddenOracleFileLevelRestore,
  I18NService,
  isHideOracleInstanceRestore,
  isHideOracleMount,
  TapeCopyApiService,
  WarningMessageService
} from 'app/shared';
import { CopiesService } from 'app/shared/api/services/copies.service';
import {
  CommonConsts,
  DataMap,
  GROUP_COMMON,
  hasCopyDeletePermission,
  hasCopyIndexPermission,
  hasLivemountPermission,
  hasRecoveryPermission,
  hasReplicationPermission,
  MODAL_COMMON,
  OperateItems,
  RestoreType,
  RestoreV2Type,
  VM_COPY_TYPE,
  WormStatusEnum
} from 'app/shared/consts';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { CopyActionService } from 'app/shared/services/copy-action.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ManualMountService } from 'app/shared/services/manual-mount.service';
import { RestoreService } from 'app/shared/services/restore.service';
import { TakeManualArchiveService } from 'app/shared/services/take-manual-archive.service';
import {
  assign,
  cloneDeep,
  each,
  extend,
  filter,
  find,
  get,
  includes,
  isBoolean,
  isEmpty,
  isFunction,
  isString,
  mapValues,
  reject,
  trim
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { ModifyRetentionPolicyComponent } from '..';
import { CopyDataDetailComponent } from '../copy-data-detail/copy-data-detail.component';
import { CopyVerifyComponent } from '../copy-verify-proxy/copy-verify.component';

@Component({
  selector: 'aui-copy-data-list',
  templateUrl: './copy-data-list.component.html',
  styleUrls: ['./copy-data-list.component.less'],
  providers: [DatePipe]
})
export class CopyDataListComponent implements OnInit, OnChanges, OnDestroy {
  @Input() id: string;
  @Input() resType;
  @Input() rowData: any;
  @Input() currentDate;
  @Input() hiddenColumn;
  @Input() isGlobalSearch;

  timeSub$: Subscription;
  destroy$ = new Subject();

  tableData = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  dataMap = DataMap;
  orders = ['-display_timestamp'];
  columns;
  filterMap: any = {};
  groupOptions = GROUP_COMMON;
  vmCopyTypeOptons = VM_COPY_TYPE;
  detailsModalOptions: any = {};
  resourceType = DataMap.Resource_Type;
  filterParams = {};
  copyClusterName;
  copyName;
  resourceName;
  locationName;
  _parse = JSON.parse;
  _get = get;
  tableColumnKey = 'copy_data_list_table';
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  isHcsUser = this.appUtilsService.isHcsUser;
  tableScrollParams = {
    autosize: true
  };

  constructor(
    private i18n: I18NService,
    public drawModalService: DrawModalService,
    public copiesApiService: CopiesService,
    public dataMapService: DataMapService,
    public restoreService: RestoreService,
    public datePipe: DatePipe,
    public warningMessageService: WarningMessageService,
    private manualMountService: ManualMountService,
    private messageBox: MessageboxService,
    private message: MessageService,
    private tapeCopyApiService: TapeCopyApiService,
    private cookieService: CookieService,
    private copyActionService: CopyActionService,
    private copyControllerService: CopyControllerService,
    private takeManualArchiveService: TakeManualArchiveService,
    private commonShareRestoreApiService: CommonShareRestoreApiService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initData();
  }

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnChanges(changes: SimpleChanges) {
    if (this.id) {
      this.getCopyData();
    }
  }

  initData() {
    this.initColumns();
  }

  _isWorm(row) {
    return [
      WormStatusEnum.SETTING,
      WormStatusEnum.SET_SUCCESS,
      WormStatusEnum.SET_FAILED
    ].includes(row?.worm_status);
  }

  getWormLabel(row) {
    const wormStatus = row?.worm_status;
    switch (wormStatus) {
      case WormStatusEnum.UNSET:
        return 'explore_copy_worm_unset_label';
      case WormStatusEnum.SETTING:
        return 'explore_copy_worm_setting_label';
      case WormStatusEnum.SET_SUCCESS:
        return 'explore_copy_worm_setted_label';
      case WormStatusEnum.SET_FAILED:
        return 'explore_copy_worm_set_fail_label';
      default:
        return 'explore_copy_worm_unset_label';
    }
  }

  getWormIcon(row) {
    const wormStatus = row?.worm_status;
    switch (wormStatus) {
      case WormStatusEnum.UNSET:
        return 'aui-copy-data-worm-unset';
      case WormStatusEnum.SETTING:
        return 'aui-icon-loading';
      case WormStatusEnum.SET_SUCCESS:
        return 'aui-copy-data-worm-set-success';
      case WormStatusEnum.SET_FAILED:
        return 'aui-copy-data-worm-set-faild';
      default:
        return 'aui-copy-data-worm-unset';
    }
  }

  initColumns() {
    const getStatusOpts = () => {
      const opts = this.dataMapService
        .toArray('copydata_validStatus', [
          DataMap.copydata_validStatus.normal.value,
          DataMap.copydata_validStatus.invalid.value,
          DataMap.copydata_validStatus.deleting.value,
          DataMap.copydata_validStatus.restoring.value
        ])
        .filter(item => {
          if (this.resType !== DataMap.Resource_Type.commonShare.value) {
            return !includes(
              [
                DataMap.copydata_validStatus.sharing.value,
                DataMap.copydata_validStatus.downloading.value
              ],
              item.value
            );
          }
          return true;
        })
        .filter(item => {
          return [DataMap.Resource_Type.ImportCopy.value].includes(this.resType)
            ? [
                DataMap.copydata_validStatus.normal.value,
                DataMap.copydata_validStatus.invalid.value,
                DataMap.copydata_validStatus.deleting.value,
                DataMap.copydata_validStatus.restoring.value
              ].includes(item.value)
            : [
                DataMap.Resource_Type.HCSCloudHost.value,
                DataMap.Resource_Type.openStackCloudServer.value,
                DataMap.Resource_Type.cNwareVm.value,
                DataMap.Resource_Type.dbTwoDatabase.value,
                DataMap.Resource_Type.dbTwoTableSet.value,
                DataMap.Resource_Type.APSCloudServer.value
              ].includes(this.resType)
            ? [
                DataMap.copydata_validStatus.normal.value,
                DataMap.copydata_validStatus.invalid.value,
                DataMap.copydata_validStatus.deleting.value,
                DataMap.copydata_validStatus.restoring.value,
                DataMap.copydata_validStatus.verifying.value,
                DataMap.copydata_validStatus.deleteFailed.value
              ].includes(item.value)
            : [
                DataMap.Resource_Type.PostgreSQLInstance.value,
                DataMap.Resource_Type.PostgreSQLClusterInstance.value,
                DataMap.Resource_Type.KingBaseInstance.value,
                DataMap.Resource_Type.KingBaseClusterInstance.value,
                DataMap.Resource_Type.Redis.value,
                DataMap.Resource_Type.KubernetesStatefulset.value,
                DataMap.Resource_Type.kubernetesNamespaceCommon.value,
                DataMap.Resource_Type.kubernetesDatasetCommon.value,
                DataMap.Resource_Type.HDFSFileset.value,
                DataMap.Resource_Type.HBaseBackupSet.value,
                DataMap.Resource_Type.HiveBackupSet.value,
                DataMap.Resource_Type.ElasticsearchBackupSet.value,
                DataMap.Resource_Type.FusionCompute.value,
                DataMap.Resource_Type.fusionComputeVirtualMachine.value,
                DataMap.Resource_Type.Dameng_cluster.value,
                DataMap.Resource_Type.Dameng_singleNode.value,
                DataMap.Resource_Type.Dameng.value,
                DataMap.Resource_Type.OpenGauss.value,
                DataMap.Resource_Type.OpenGauss_database.value,
                DataMap.Resource_Type.OpenGauss_instance.value,
                DataMap.Resource_Type.GaussDB_T.value,
                DataMap.Resource_Type.gaussdbTSingle.value,
                DataMap.Resource_Type.generalDatabase.value,
                DataMap.Resource_Type.tidbDatabase.value,
                DataMap.Resource_Type.tidbCluster.value,
                DataMap.Resource_Type.tidbTable.value,
                DataMap.Resource_Type.MongodbClusterInstance.value,
                DataMap.Resource_Type.MongodbSingleInstance.value,
                DataMap.Resource_Type.ActiveDirectory.value,
                DataMap.Resource_Type.ExchangeSingle.value,
                DataMap.Resource_Type.ExchangeGroup.value,
                DataMap.Resource_Type.ExchangeEmail.value,
                DataMap.Resource_Type.ExchangeDataBase.value,
                DataMap.Resource_Type.saphanaDatabase.value,
                DataMap.Resource_Type.ObjectSet.value
              ].includes(this.resType)
            ? [
                DataMap.copydata_validStatus.normal.value,
                DataMap.copydata_validStatus.invalid.value,
                DataMap.copydata_validStatus.deleting.value,
                DataMap.copydata_validStatus.restoring.value,
                DataMap.copydata_validStatus.deleteFailed.value
              ].includes(item.value)
            : item;
        });

      if (
        includes(
          [
            DataMap.Resource_Type.SQLServerClusterInstance.value,
            DataMap.Resource_Type.SQLServerInstance.value,
            DataMap.Resource_Type.SQLServerGroup.value,
            DataMap.Resource_Type.SQLServerDatabase.value,
            DataMap.Resource_Type.DWS_Cluster.value,
            DataMap.Resource_Type.DWS_Database.value,
            DataMap.Resource_Type.DWS_Schema.value,
            DataMap.Resource_Type.DWS_Table.value,
            DataMap.Resource_Type.ElasticsearchBackupSet.value,
            DataMap.Resource_Type.ClickHouse.value,
            DataMap.Resource_Type.ClickHouseDatabase.value,
            DataMap.Resource_Type.ClickHouseTableset.value,
            DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
            DataMap.Resource_Type.lightCloudGaussdbInstance.value,
            DataMap.Resource_Type.OceanBaseCluster.value,
            DataMap.Resource_Type.OceanBaseTenant.value,
            DataMap.Resource_Type.tdsqlDistributedInstance.value
          ],
          this.resType
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.copydata_validStatus.mounting.value,
              DataMap.copydata_validStatus.mounted.value,
              DataMap.copydata_validStatus.unmounting.value,
              DataMap.copydata_validStatus.verifying.value
            ],
            item.value
          )
        );
      }

      if (
        includes(
          [
            DataMap.Resource_Type.MySQL.value,
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.fileset.value,
            DataMap.Resource_Type.NASShare.value,
            DataMap.Resource_Type.MySQLDatabase.value,
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.NASFileSystem.value,
            DataMap.Resource_Type.tdsqlInstance.value,
            DataMap.Resource_Type.virtualMachine.value,
            DataMap.Resource_Type.OceanBaseTenant.value,
            DataMap.Resource_Type.OceanBaseCluster.value,
            DataMap.Resource_Type.MySQLClusterInstance.value
          ],
          this.resType
        )
      ) {
        return reject(opts, item =>
          includes([DataMap.copydata_validStatus.verifying.value], item.value)
        );
      }

      return opts;
    };
    const getGeneratedTypeOpts = () => {
      const opts = this.dataMapService
        .toArray('CopyData_generatedType', [
          DataMap.CopyData_generatedType.backup.value,
          DataMap.CopyData_generatedType.Imported.value
        ])
        .filter(
          item =>
            filterGeneratedBy(item, [this.resType]) &&
            item.value !== DataMap.CopyData_generatedType.import.value
        );

      if (
        includes(
          [
            DataMap.Resource_Type.MySQL.value,
            DataMap.Resource_Type.MySQLClusterInstance.value,
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.MySQLDatabase.value,
            DataMap.Resource_Type.fileset.value,
            DataMap.Resource_Type.tdsqlInstance.value
          ],
          this.resType
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_generatedType.Imported.value,
              DataMap.CopyData_generatedType.download.value,
              DataMap.CopyData_generatedType.snapshot.value
            ],
            item.value
          )
        );
      }

      if (
        includes(
          [
            DataMap.Resource_Type.ElasticsearchBackupSet.value,
            DataMap.Resource_Type.KubernetesStatefulset.value,
            DataMap.Resource_Type.kubernetesNamespaceCommon.value,
            DataMap.Resource_Type.kubernetesDatasetCommon.value,
            DataMap.Resource_Type.ClickHouse.value,
            DataMap.Resource_Type.ClickHouseDatabase.value,
            DataMap.Resource_Type.ClickHouseTableset.value,
            DataMap.Resource_Type.DWS_Cluster.value,
            DataMap.Resource_Type.DWS_Database.value,
            DataMap.Resource_Type.DWS_Schema.value,
            DataMap.Resource_Type.DWS_Table.value,
            DataMap.Resource_Type.SQLServerClusterInstance.value,
            DataMap.Resource_Type.SQLServerInstance.value,
            DataMap.Resource_Type.SQLServerGroup.value,
            DataMap.Resource_Type.SQLServerDatabase.value,
            DataMap.Resource_Type.generalDatabase.value,
            DataMap.Resource_Type.dbTwoDatabase.value,
            DataMap.Resource_Type.dbTwoTableSet.value,
            DataMap.Resource_Type.lightCloudGaussdbInstance.value,
            DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
            DataMap.Resource_Type.tidbCluster.value,
            DataMap.Resource_Type.tidbDatabase.value,
            DataMap.Resource_Type.tidbTable.value,
            DataMap.Resource_Type.OceanBaseCluster.value,
            DataMap.Resource_Type.OceanBaseTenant.value,
            DataMap.Resource_Type.tdsqlDistributedInstance.value,
            DataMap.Resource_Type.ActiveDirectory.value,
            DataMap.Resource_Type.ExchangeEmail.value,
            DataMap.Resource_Type.ExchangeGroup.value,
            DataMap.Resource_Type.ExchangeSingle.value,
            DataMap.Resource_Type.ExchangeDataBase.value,
            DataMap.Resource_Type.ObjectSet.value,
            DataMap.Resource_Type.saphanaDatabase.value
          ],
          this.resType
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_generatedType.liveMount.value,
              DataMap.CopyData_generatedType.download.value,
              DataMap.CopyData_generatedType.snapshot.value,
              DataMap.CopyData_generatedType.Imported.value
            ],
            item.value
          )
        );
      }

      if (
        includes(
          [
            DataMap.Resource_Type.MongodbClusterInstance.value,
            DataMap.Resource_Type.MongodbSingleInstance.value
          ],
          this.resType
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_generatedType.liveMount.value,
              DataMap.CopyData_generatedType.download.value,
              DataMap.CopyData_generatedType.snapshot.value
            ],
            item.value
          )
        );
      }

      return opts;
    };

    const getBackupTypeOpts = () => {
      const opts = this.dataMapService
        .toArray('CopyData_Backup_Type', [
          DataMap.CopyData_Backup_Type.full.value,
          DataMap.CopyData_Backup_Type.incremental.value
        ])
        .filter(item => filterBackupType(item, [this.resType], this.i18n));

      if (this.resType === DataMap.Resource_Type.fileset.value) {
        if (this.rowData.subType === DataMap.Resource_Type.volume.value) {
          return reject(opts, item =>
            includes(
              [
                DataMap.CopyData_Backup_Type.log.value,
                DataMap.CopyData_Backup_Type.diff.value,
                DataMap.CopyData_Backup_Type.snapshot.value,
                DataMap.CopyData_Backup_Type.incremental.value
              ],
              item.value
            )
          );
        } else {
          return reject(opts, item =>
            includes(
              [
                DataMap.CopyData_Backup_Type.log.value,
                DataMap.CopyData_Backup_Type.diff.value,
                DataMap.CopyData_Backup_Type.snapshot.value
              ],
              item.value
            )
          );
        }
      }

      // 只有全量
      if (
        includes(
          [
            DataMap.Resource_Type.DWS_Table.value,
            DataMap.Resource_Type.DWS_Schema.value,
            DataMap.Resource_Type.DWS_Database.value,
            DataMap.Resource_Type.ClickHouse.value,
            DataMap.Resource_Type.ClickHouseDatabase.value,
            DataMap.Resource_Type.ClickHouseTableset.value,
            DataMap.Resource_Type.tidbTable.value,
            DataMap.Resource_Type.tidbDatabase.value,
            DataMap.Resource_Type.OceanBaseTenant.value
          ],
          this.resType
        )
      ) {
        return filter(opts, item =>
          includes([DataMap.CopyData_Backup_Type.full.value], item.value)
        );
      }

      // 全量 增量
      if (includes([DataMap.Resource_Type.ExchangeEmail.value], this.resType)) {
        return filter(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.full.value,
              DataMap.CopyData_Backup_Type.incremental.value
            ],
            item.value
          )
        );
      }

      // 全量 永久增量
      if (
        includes(
          [DataMap.Resource_Type.KubernetesStatefulset.value],
          this.resType
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.log.value,
              DataMap.CopyData_Backup_Type.diff.value,
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.incremental.value
            ],
            item.value
          )
        );
      }

      // 全量 增量 永久增量（合成全量）
      if (
        includes(
          [
            DataMap.Resource_Type.fileset.value,
            DataMap.Resource_Type.ObjectSet.value
          ],
          this.resType
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.log.value,
              DataMap.CopyData_Backup_Type.diff.value,
              DataMap.CopyData_Backup_Type.snapshot.value
            ],
            item.value
          )
        );
      }

      // 全量 差异
      if (this.resType === DataMap.Resource_Type.ElasticsearchBackupSet.value) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.log.value,
              DataMap.CopyData_Backup_Type.diff.value,
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.permanent.value
            ],
            item.value
          )
        );
      }

      // 全量 增量 差异
      if (includes([DataMap.Resource_Type.DWS_Cluster.value], this.resType)) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.log.value,
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.permanent.value
            ],
            item.value
          )
        );
      }

      // 全量 差异 日志
      if (
        includes(
          [
            DataMap.Resource_Type.SQLServerClusterInstance.value,
            DataMap.Resource_Type.SQLServerInstance.value,
            DataMap.Resource_Type.SQLServerGroup.value,
            DataMap.Resource_Type.SQLServerDatabase.value,
            DataMap.Resource_Type.gaussdbForOpengaussInstance.value
          ],
          this.resType
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.permanent.value,
              DataMap.CopyData_Backup_Type.incremental.value
            ],
            item.value
          )
        );
      }

      // 全量 增量 日志 差异
      if (
        includes(
          [
            DataMap.Resource_Type.MySQL.value,
            DataMap.Resource_Type.MySQLClusterInstance.value,
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.MySQLDatabase.value,
            DataMap.Resource_Type.generalDatabase.value,
            DataMap.Resource_Type.dbTwoDatabase.value,
            DataMap.Resource_Type.dbTwoTableSet.value
          ],
          this.resType
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.permanent.value
            ],
            item.value
          )
        );
      }
      // 只有全量 日志
      if (
        includes(
          [
            DataMap.Resource_Type.tidbCluster.value,
            DataMap.Resource_Type.tdsqlDistributedInstance.value
          ],
          this.resType
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.permanent.value,
              DataMap.CopyData_Backup_Type.incremental.value,
              DataMap.CopyData_Backup_Type.diff.value
            ],
            item.value
          )
        );
      }

      // 全量 增量 日志
      if (
        includes(
          [
            DataMap.Resource_Type.OceanBaseCluster.value,
            DataMap.Resource_Type.goldendbInstance.value
          ],
          this.resType
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.permanent.value,
              DataMap.CopyData_Backup_Type.diff.value
            ],
            item.value
          )
        );
      }

      // 全量 永久增量 日志
      if (
        includes(
          [
            DataMap.Resource_Type.tdsqlInstance.value,
            DataMap.Resource_Type.ExchangeSingle.value,
            DataMap.Resource_Type.ExchangeGroup.value,
            DataMap.Resource_Type.ExchangeDataBase.value
          ],
          this.resType
        )
      ) {
        return filter(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.full.value,
              DataMap.CopyData_Backup_Type.permanent.value,
              DataMap.CopyData_Backup_Type.log.value
            ],
            item.value
          )
        );
      }
      return opts;
    };

    this.columns = [
      {
        key: 'uuid',
        label: 'ID',
        isShow: this.isHyperdetect
      },
      {
        key: 'origin_backup_id',
        label: this.i18n.get('common_backup_copy_label'),
        isShow: false,
        displayCheck: () => {
          return this.isOceanProtect;
        }
      },
      {
        key: 'cluster_name',
        label: this.i18n.get('system_servers_label'),
        disabled: false,
        show: false,
        isLeaf: true,
        displayCheck: () => {
          return this.isOceanProtect && !this.appUtilsService.isDistributed;
        }
      },
      {
        key: 'name',
        label: this.i18n.get('common_name_label'),
        isShow: false
      },
      {
        key: 'origin_copy_time_stamp',
        label: this.i18n.get('system_launch_time_label'),
        width: '160px',
        sort: true,
        isShow: false,
        displayCheck: () => {
          return this.isOceanProtect;
        }
      },
      {
        key: 'display_timestamp',
        label: !this.isHyperdetect
          ? this.i18n.get('common_time_stamp_label')
          : this.i18n.get('common_hyperdetect_time_stamp_label'),
        width: !this.isHyperdetect ? '160px' : '280px',
        sort: true,
        isShow: true
      },
      {
        key: 'status',
        label: this.i18n.get('common_status_label'),
        width: !this.isHyperdetect ? '100px' : '200px',
        filters: getStatusOpts(),
        isShow: true
      },
      {
        key: 'location',
        label: this.i18n.get('common_location_label'),
        isShow: true,
        hidden:
          this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
      },
      {
        key: 'generated_by',
        label: this.i18n.get('common_generated_type_label'),
        filters: this.isHcsUser
          ? reject(getGeneratedTypeOpts(), item =>
              includes(
                [
                  DataMap.CopyData_generatedType.cloudArchival.value,
                  DataMap.CopyData_generatedType.tapeArchival.value,
                  DataMap.CopyData_generatedType.cascadedReplication.value,
                  DataMap.CopyData_generatedType.reverseReplication.value
                ],
                item.value
              )
            )
          : getGeneratedTypeOpts(),
        isShow: true
      },
      {
        key: 'backup_type',
        label: this.i18n.get('common_copy_type_label'),
        filters: getBackupTypeOpts(),
        hidden:
          [this.resourceType.ImportCopy.value].includes(this.resType) ||
          this.i18n.get('deploy_type') ===
            DataMap.Deploy_Type.hyperdetect.value,
        isShow: true
      },
      {
        key: 'extend_type',
        label: this.i18n.get('common_backup_data_label'),
        filters: this.dataMapService.toArray('objectBackupLevel'),
        hidden:
          ![DataMap.Resource_Type.ActiveDirectory.value].includes(
            this.resType
          ) ||
          [
            DataMap.Deploy_Type.cyberengine,
            DataMap.Deploy_Type.hyperdetect.value,
            DataMap.Deploy_Type.cloudbackup.value
          ].includes(this.i18n.get('deploy_type')),
        isShow: true
      },
      {
        key: 'storage_snapshot_flag',
        label: this.i18n.get('protection_snapshot_backup_copy_label'),
        filters: this.dataMapService.toArray('isBusinessOptions'),
        hidden:
          ![
            DataMap.Resource_Type.oracleCluster.value,
            DataMap.Resource_Type.oracle.value
          ].includes(this.resType) ||
          [
            DataMap.Deploy_Type.cyberengine,
            DataMap.Deploy_Type.hyperdetect.value,
            DataMap.Deploy_Type.cloudbackup.value
          ].includes(this.i18n.get('deploy_type')) ||
          this.isHcsUser,
        isShow: true
      },
      {
        key: 'copy_format',
        label: this.i18n.get('protection_copy_format_label'),
        width: '80px',
        hidden: true || this.hiddenColumn,
        isShow: true
      },
      {
        key: 'worm_status',
        label: this.i18n.get('explore_worm_th_label'),
        filters: this.dataMapService.toArray('copyDataWormStatus'),
        isShow: true
      },
      {
        key: 'copy_verify_status',
        label: this.i18n.get('common_copy_verify_status_label'),
        hidden: ![
          DataMap.Resource_Type.KubernetesStatefulset.value,
          DataMap.Resource_Type.HCSCloudHost.value,
          DataMap.Resource_Type.openStackCloudServer.value,
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.FusionComputeVM.value,
          DataMap.Resource_Type.fusionOne.value,
          DataMap.Resource_Type.cNwareVm.value,
          DataMap.Resource_Type.APSCloudServer.value
        ].includes(this.resType),
        isShow: true
      },
      {
        key: 'indexed',
        label: this.i18n.get('common_index_label'),
        filters: this.dataMapService.toArray('CopyData_fileIndex'),
        hidden:
          (![
            this.resourceType.virtualMachine.value,
            this.resourceType.fileset.value,
            this.resourceType.NASFileSystem.value,
            this.resourceType.ndmp.value,
            this.resourceType.NASShare.value,
            this.resourceType.LocalFileSystem.value,
            this.resourceType.HDFSFileset.value,
            this.resourceType.FusionCompute.value,
            this.resourceType.HCSCloudHost.value,
            this.resourceType.openStackCloudServer.value,
            this.resourceType.ObjectSet.value,
            this.resourceType.APSCloudServer.value,
            this.resourceType.cNwareVm.value,
            this.resourceType.hyperVVm.value
          ].includes(this.resType) &&
            !includes(
              [
                DataMap.Resource_Type.FusionCompute.value,
                DataMap.Resource_Type.fusionOne.value
              ],
              this.rowData?.subType
            )) ||
          this.isHyperdetect ||
          this.hiddenColumn ||
          this.isHcsUser,
        isShow: true
      },
      {
        key: 'isSanClient',
        label: this.i18n.get('explore_sanclient_copy_label'),
        hidden: !includes(
          [
            DataMap.Resource_Type.dbTwoDatabase.value,
            DataMap.Resource_Type.dbTwoTableSet.value,
            DataMap.Resource_Type.informixInstance.value,
            DataMap.Resource_Type.informixClusterInstance.value
          ],
          this.resType
        ),
        isShow: false,
        isLeaf: true
      },
      {
        key: 'isSystemBackup',
        label: this.i18n.get('protection_volume_advanced_backup_label'),
        hidden: !includes(
          [DataMap.Resource_Type.volume.value],
          this.rowData.resourceType
        ),
        isShow: true,
        isLeaf: true
      },
      {
        key: 'isBackupAcl',
        label: this.i18n.get('explore_acl_backup_label'),
        hidden: !includes(
          [DataMap.Resource_Type.ObjectSet.value],
          this.resType
        ),
        isShow: true,
        isLeaf: true
      },
      {
        key: 'canRestore',
        label: this.i18n.get('explore_is_copy_complete_label'),
        resourceType: [DataMap.Resource_Type.lightCloudGaussdbInstance.value],
        isShow: true,
        hidden: ![
          DataMap.Resource_Type.lightCloudGaussdbInstance.value
        ].includes(this.resType)
      },
      {
        key: 'expiration_time',
        width: '170px',
        hidden: this.hiddenColumn,
        label: this.i18n.get('common_expriration_time_label'),
        isShow: true
      },
      {
        key: 'labelList',
        label: this.i18n.get('common_tag_label'),
        isShow: true
      },
      {
        key: 'op',
        width: this.isOceanProtect ? '80px' : '110px',
        label: this.i18n.get('common_operation_label'),
        isShow: true
      }
    ];
    const deployType = this.i18n.get('deploy_type');
    if (['d3', 'd4', 'cloudbackup'].includes(deployType)) {
      this.columns = this.columns.filter(item => item.key !== 'worm_status');
    }
    // 如果配置项中写了displayCheck，就用对应的bool值
    this.columns = this.columns.filter(item =>
      isFunction(item.displayCheck) ? item.displayCheck() : true
    );
  }

  getCopyData(refreshData?) {
    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });

    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }
    let manualRefresh = true;
    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL_RESOURCE)
      .pipe(
        switchMap(index => {
          manualRefresh = !index;
          const conditions = {
            resource_id: this.id,
            ...this.filterParams
          };

          if (this.currentDate) {
            assign(conditions, {
              date: this.datePipe.transform(this.currentDate, 'yyyy-MM-dd')
            });
          }

          if (this.isGlobalSearch) {
            assign(conditions, {
              gn_range: [this.rowData.gnGte, this.rowData.gnLte],
              indexed: DataMap.CopyData_fileIndex.indexed.value
            });
            if (this.rowData.esn) {
              assign(conditions, {
                device_esn: this.rowData.esn
              });
            }
            if (
              includes(
                [
                  DataMap.Resource_Type.NASFileSystem.value,
                  DataMap.Resource_Type.NASShare.value,
                  DataMap.Resource_Type.LocalFileSystem.value,
                  DataMap.Resource_Type.HDFSFileset.value,
                  DataMap.Resource_Type.fileset.value
                ],
                this.rowData.resourceType
              )
            ) {
              assign(conditions, {
                chain_id: this.rowData.chainId,
                generated_by: this.rowData.generatedBy
              });
            }
            if (
              includes(
                [
                  DataMap.Resource_Type.virtualMachine.value,
                  DataMap.Resource_Type.FusionCompute.value,
                  DataMap.Resource_Type.fusionOne.value,
                  DataMap.Resource_Type.HCSCloudHost.value,
                  DataMap.Resource_Type.openStackCloudServer.value,
                  DataMap.Resource_Type.APSCloudServer.value,
                  DataMap.Resource_Type.cNware.value,
                  DataMap.Resource_Type.hyperVVm.value,
                  DataMap.Resource_Type.volume.value
                ],
                this.rowData.resourceType
              )
            ) {
              assign(conditions, {
                chain_id: this.rowData.chainId
              });
            }
          }

          return this.copiesApiService.queryResourcesV1CopiesGet({
            akLoading: !index,
            pageNo: this.pageIndex,
            pageSize: this.pageSize,
            orders: this.orders,
            conditions: JSON.stringify(conditions)
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        each(res.items, item => {
          this._getCopyVerifyStatus(item);
          // 获取标签数据
          const { showList, hoverList } = getLabelList(
            JSON.parse(item.properties || '{}')
          );
          assign(item, {
            backup_level: 0,
            labelList: get(JSON.parse(item.properties || '{}'), 'labelList'),
            showLabelList: showList,
            hoverLabelList: hoverList
          });
          return item;
        });
        this.assignNewAttribute(res.items);
        this.tableData = res.items;
        this.total = res.total;
        if (
          manualRefresh &&
          refreshData &&
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'copy-detail-modal'
          ) &&
          find(res.items, { uuid: refreshData.uuid })
        ) {
          this.getDetail(find(res.items, { uuid: refreshData.uuid }));
        }
      });
  }

  stateChange(source) {
    this.pageIndex = source.paginator.pageIndex;
    this.pageSize = source.paginator.pageSize;
    this.getCopyData();
  }

  sortData(e) {
    this.orders = [];
    this.orders.push((e.direction === 'asc' ? '+' : '-') + e.key);
    this.getCopyData();
  }

  filterChange(e) {
    if (
      [
        this.resourceType.fusionComputeVirtualMachine.value,
        this.resourceType.FusionCompute.value,
        this.resourceType.HCSCloudHost.value,
        this.resourceType.KubernetesStatefulset.value
      ].includes(this.resType)
    ) {
      e.value = e.value.map(item => (item === 5 ? 2 : item));
    }
    extend(this.filterParams, {
      [e.key === 'backup_type' ? 'source_copy_type' : e.key]: e.value
    });
    this.getCopyData();
  }

  searchByCopyClusterName(copyClusterName) {
    assign(this.filterParams, {
      cluster_name: trim(copyClusterName)
    });
    this.getCopyData();
  }

  searchByCopyName(copyName) {
    extend(this.filterParams, {
      name: trim(copyName)
    });
    this.getCopyData();
  }

  searchByName(resourceName) {
    extend(this.filterParams, {
      resource_name: trim(resourceName)
    });
    this.getCopyData();
  }

  searchByLocation(locationName) {
    extend(this.filterParams, {
      location: trim(locationName)
    });
    this.getCopyData();
  }

  searchByLabel(label) {
    extend(this.filterParams, {
      labelName: trim(label)
    });
    this.getCopyData();
  }

  getDetail(data, activeFileTab = false) {
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvWidth: MODAL_COMMON.largeWidth,
        lvModalKey: 'copy-detail-modal',
        lvContent: CopyDataDetailComponent,
        lvComponentParams: {
          data: {
            ...data,
            optItems: this.getOptsItems(data),
            resType: this.resType,
            name: this.datePipe.transform(
              data.display_timestamp,
              'yyyy-MM-dd HH:mm:ss'
            ),
            rowData: this.rowData
          },
          activeFileTab
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  modifyRetention(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('common_modify_retention_policy_label'),
        lvModalKey: 'modify_retention_policy',
        lvOkLoadingText: this.i18n.get('common_loading_label'),
        lvWidth: MODAL_COMMON.smallModal,
        lvContent: ModifyRetentionPolicyComponent,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as ModifyRetentionPolicyComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvComponentParams: {
          data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as ModifyRetentionPolicyComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.getCopyData(data);
              },
              error: error => resolve(false)
            });
          });
        }
      })
    );
  }

  optsCallback = data => {
    return this.getOptsItems(data);
  };

  getOptsItems(data) {
    const isSchemaLevelRestoreHideen = () => {
      if (
        ([DataMap.Resource_Type.ClickHouse.value].includes(
          data.resource_sub_type
        ) &&
          [
            DataMap.CopyData_generatedType.cloudArchival.value,
            DataMap.CopyData_generatedType.tapeArchival.value
          ].includes(data.generated_by) &&
          includes(
            [DataMap.Resource_Type.ClickHouse.value],
            data.resource_sub_type
          )) ||
        (includes(
          [DataMap.Resource_Type.DWS_Database.value],
          data.resource_sub_type
        ) &&
          includes(
            [
              DataMap.CopyData_generatedType.cascadedReplication.value,
              DataMap.CopyData_generatedType.replicate.value,
              DataMap.CopyData_generatedType.tapeArchival.value
            ],
            data.generated_by
          ))
      ) {
        return true;
      }
      return !includes(
        [
          DataMap.Resource_Type.DWS_Database.value,
          DataMap.Resource_Type.ClickHouse.value
        ],
        data.resource_sub_type
      );
    };
    const isSchemaLevelRestoreDisabled = () => {
      if (
        [DataMap.Resource_Type.ClickHouse.value].includes(
          data.resource_sub_type
        ) &&
        [
          DataMap.CopyData_generatedType.cloudArchival.value,
          DataMap.CopyData_generatedType.tapeArchival.value
        ].includes(data.generated_by) &&
        includes(
          [DataMap.Resource_Type.ClickHouse.value],
          data.resource_sub_type
        )
      ) {
        return true;
      }
      return includes(
        [
          DataMap.Resource_Type.LocalFileSystem.value,
          DataMap.Resource_Type.HDFSFileset.value
        ],
        data.resource_sub_type
      )
        ? !(
            data.status === DataMap.copydata_validStatus.normal.value &&
            includes([DataMap.CopyData_fileIndex.indexed.value], data.indexed)
          )
        : data.status !== DataMap.copydata_validStatus.normal.value ||
            ([
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.NASFileSystem.value
            ].includes(data.resource_sub_type) &&
              DataMap.CopyData_generatedType.replicate.value ===
                data.generated_by) ||
            (includes(
              [
                DataMap.Resource_Type.NASShare.value,
                DataMap.Resource_Type.NASFileSystem.value
              ],
              data.resource_sub_type
            ) &&
              DataMap.CopyData_generatedType.cloudArchival.value ===
                data.generated_by &&
              data.indexed !== DataMap.CopyData_fileIndex.indexed.value);
    };
    const objectLevelRestoreLabel = type => {
      let labelStr: string;
      switch (type) {
        case DataMap.Resource_Type.oracle.value:
        case DataMap.Resource_Type.oracleCluster.value:
          labelStr = 'protection_table_level_restore_label';
          break;
        case DataMap.Resource_Type.ExchangeEmail.value:
          labelStr = 'common_email_level_restore_label';
          break;
        case DataMap.Resource_Type.ExchangeDataBase.value:
          labelStr = 'common_user_level_restore_label';
          break;
        default:
          labelStr = 'common_object_level_restore_label';
          break;
      }
      return this.i18n.get(labelStr);
    };
    const isObjectLevelRestoreDisabled = () => {
      if (
        data.resource_sub_type === DataMap.Resource_Type.ActiveDirectory.value
      ) {
        // ad域备份数据有系统状态+对象，只有备份了对象才能对象级恢复
        return (
          data?.extend_type !== DataMap.objectBackupLevel.object.value ||
          data.status !== DataMap.copydata_validStatus.normal.value ||
          data.resource_status === DataMap.Resource_Status.notExist.value
        );
      } else if (
        includes(
          [DataMap.Resource_Type.ExchangeEmail.value],
          data.resource_sub_type
        )
      ) {
        // 邮箱细粒度只能恢复到原位置，资源不存在时，无法恢复
        return (
          data.status !== DataMap.copydata_validStatus.normal.value ||
          data.resource_status === DataMap.Resource_Status.notExist.value
        );
      } else if (
        includes(
          [DataMap.Resource_Type.ExchangeDataBase.value],
          data.resource_sub_type
        )
      ) {
        // 数据库细粒度可以恢复到新位置，资源不存在时也能恢复
        return data.status !== DataMap.copydata_validStatus.normal.value;
      } else if (
        includes(
          [
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oracleCluster.value
          ],
          data.resource_sub_type
        )
      ) {
        return false;
      }
      return true;
    };
    const isObjectLevelRestoreHidden = () => {
      return (
        !includes(
          [
            DataMap.Resource_Type.ActiveDirectory.value,
            DataMap.Resource_Type.ExchangeEmail.value,
            DataMap.Resource_Type.ExchangeDataBase.value,
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oracleCluster.value
          ],
          data.resource_sub_type
        ) ||
        (includes(
          [
            DataMap.CopyData_generatedType.tapeArchival.value,
            DataMap.CopyData_generatedType.cloudArchival.value
          ],
          data.generated_by
        ) &&
          includes(
            [
              DataMap.Resource_Type.ActiveDirectory.value,
              DataMap.Resource_Type.ExchangeDataBase.value
            ],
            data.resource_sub_type
          )) ||
        (includes(
          [
            DataMap.Resource_Type.ExchangeDataBase.value,
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oracleCluster.value
          ],
          data.resource_sub_type
        ) &&
          includes(
            [DataMap.CopyData_Backup_Type.log.value],
            data.backup_type
          )) ||
        hiddenOracleFileLevelRestore(data, resourceProperties, properties)
      );
    };
    const features = data.features
      ? data.features
          .toString(2)
          .split('')
          .reverse()
      : [];
    const resourceProperties = isString(data.resource_properties)
      ? JSON.parse(data.resource_properties)
      : data.resource_properties;
    const properties = isString(data.properties)
      ? JSON.parse(data.properties)
      : data.properties;
    const menus = [
      {
        id: 'restore',
        label:
          data.resource_sub_type === DataMap.Resource_Type.HCSCloudHost.value
            ? this.i18n.get('common_restoring_disks_label')
            : data.resource_sub_type ===
              DataMap.Resource_Type.ActiveDirectory.value
            ? this.i18n.get('common_system_status_restore_label')
            : this.i18n.get('common_restore_label'),
        tips:
          data.resource_sub_type ===
            DataMap.Resource_Type.lightCloudGaussdbInstance.value &&
          get(JSON.parse(data.properties), 'canRestore', true) === false
            ? this.i18n.get('explore_copy_incomplete_tip_label')
            : null,
        disabled:
          !(
            data.status === DataMap.copydata_validStatus.normal.value ||
            (includes(
              [
                DataMap.Resource_Type.FusionCompute.value,
                DataMap.Resource_Type.fusionOne.value,
                DataMap.Resource_Type.KubernetesStatefulset.value,
                DataMap.Resource_Type.HCSCloudHost.value,
                DataMap.Resource_Type.openStackCloudServer.value,
                DataMap.Resource_Type.cNwareVm.value,
                DataMap.Resource_Type.APSCloudServer.value,
                DataMap.Resource_Type.hyperVVm.value
              ],
              data.resource_sub_type
            ) &&
              data.status === DataMap.copydata_validStatus.invalid.value)
          ) ||
          (includes(
            [
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.dbTwoTableSet.value,
              DataMap.Resource_Type.lightCloudGaussdbInstance.value,
              DataMap.Resource_Type.ActiveDirectory.value,
              DataMap.Resource_Type.ExchangeEmail.value
            ],
            data.resource_sub_type
          ) &&
            data.resource_status === DataMap.Resource_Status.notExist.value) ||
          (data.generated_by ===
            DataMap.CopyData_generatedType.liveMount.value &&
            includes(
              [DataMap.Resource_Type.fileset.value],
              data.resource_sub_type
            )) ||
          disableValidCopyBtn(data, properties) ||
          disableOpenstackVmRestore(data, properties) ||
          (includes(
            [DataMap.Resource_Type.lightCloudGaussdbInstance.value],
            data.resource_sub_type
          ) &&
            get(JSON.parse(data.properties), 'canRestore', true) === false) ||
          !hasRecoveryPermission(data) ||
          (disableOracleRestoreNewLocation(
            data,
            properties,
            resourceProperties
          ) &&
            disableOracleRestoreOriginalLocation(data, resourceProperties)),
        hidden:
          data.resource_sub_type === DataMap.Resource_Type.ImportCopy.value ||
          (data.features ? features[1] !== '1' : false) ||
          (includes(
            [
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.MySQLClusterInstance.value,
              DataMap.Resource_Type.MySQLInstance.value,
              DataMap.Resource_Type.MySQLDatabase.value,
              DataMap.Resource_Type.tdsqlInstance.value
            ],
            data.resource_sub_type
          ) &&
            data.generated_by ===
              DataMap.CopyData_generatedType.liveMount.value) ||
          [DataMap.Resource_Type.ABBackupClient.value].includes(
            data.resource_sub_type
          ) ||
          includes(
            [
              DataMap.Resource_Type.DWS_Schema.value,
              DataMap.Resource_Type.commonShare.value
            ],
            data.resource_sub_type
          ) ||
          getGeneralDatabaseConf(
            data,
            RestoreV2Type.CommonRestore,
            this.dataMapService
          ),
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          this.restoreService.restore({
            childResType: data.resource_sub_type,
            copyData: data,
            restoreType: RestoreType.CommonRestore,
            isMessageBox: !includes(
              [
                DataMap.Resource_Type.oracle.value,
                DataMap.Resource_Type.oracleCluster.value
              ],
              data.resource_sub_type
            ),
            onOk: () => {
              this.getCopyData();
            }
          });
        }
      },
      {
        id: 'configShareInfo',
        label: this.i18n.get('explore_commonshare_setting_shareinfo_label'),
        hidden:
          !(
            data.resource_sub_type ===
              DataMap.Resource_Type.commonShare.value &&
            data.status === DataMap.copydata_validStatus.normal.value
          ) ||
          !(data.resource_sub_type === DataMap.Resource_Type.commonShare.value),
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          this.restoreService.restoreCommonShare({
            copyData: data,
            isConfig: true,
            onOk: () => {
              this.getCopyData();
            }
          });
        }
      },
      {
        id: 'viewShareInfo',
        label: this.i18n.get('explore_commonshare_view_shareinfo_label'),
        hidden:
          !(
            data.resource_sub_type ===
              DataMap.Resource_Type.commonShare.value &&
            data.status === DataMap.copydata_validStatus.sharing.value
          ) ||
          !(data.resource_sub_type === DataMap.Resource_Type.commonShare.value),
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          this.restoreService.restoreCommonShare({
            copyData: data,
            isConfig: false,
            onOk: () => {
              this.getCopyData();
            }
          });
        }
      },
      {
        id: 'deleteShareInfo',
        label: this.i18n.get('explore_commonshare_delete_shareinfo_label'),
        disabled: false,
        hidden: !(
          data.resource_sub_type === DataMap.Resource_Type.commonShare.value &&
          data.status === DataMap.copydata_validStatus.sharing.value
        ),
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          this.warningMessageService.create({
            content: this.i18n.get(
              'explore_commonshare_delete_file_system_tips_label'
            ),
            onOK: () => {
              this.commonShareRestoreApiService
                .StopRestore({ copyId: data.uuid })
                .subscribe(() => {
                  this.getCopyData();
                });
            }
          });
        }
      },
      {
        id: 'diskRestore',
        label: includes(
          [
            DataMap.Resource_Type.kubernetesNamespaceCommon.value,
            DataMap.Resource_Type.kubernetesDatasetCommon.value
          ],
          data.resource_sub_type
        )
          ? this.i18n.get('protection_pvc_recovery_label')
          : this.i18n.get('common_disk_restore_label'),
        disabled:
          !(
            data.status === DataMap.copydata_validStatus.normal.value ||
            (includes(
              [
                DataMap.Resource_Type.FusionCompute.value,
                DataMap.Resource_Type.fusionOne.value,
                DataMap.Resource_Type.KubernetesStatefulset.value,
                DataMap.Resource_Type.openStackCloudServer.value,
                DataMap.Resource_Type.APSCloudServer.value,
                DataMap.Resource_Type.cNwareVm.value,
                DataMap.Resource_Type.hyperVVm.value
              ],
              data.resource_sub_type
            ) &&
              data.status === DataMap.copydata_validStatus.invalid.value)
          ) || disableValidCopyBtn(data, properties),
        hidden: !includes(
          [
            DataMap.Resource_Type.virtualMachine.value,
            DataMap.Resource_Type.FusionCompute.value,
            DataMap.Resource_Type.fusionOne.value,
            DataMap.Resource_Type.openStackCloudServer.value,
            DataMap.Resource_Type.kubernetesNamespaceCommon.value,
            DataMap.Resource_Type.kubernetesDatasetCommon.value,
            DataMap.Resource_Type.cNwareVm.value,
            DataMap.Resource_Type.APSCloudServer.value,
            DataMap.Resource_Type.hyperVVm.value
          ],
          data.resource_sub_type
        ),
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          if (
            data.resource_sub_type ===
            DataMap.Resource_Type.virtualMachine.value
          ) {
            this.getDetail(data);
            return;
          }
          this.restoreService.restore({
            childResType: data.resource_sub_type,
            copyData: assign(cloneDeep(data), { diskRestore: true }),
            isMessageBox: true,
            restoreType: RestoreType.FileRestore,
            onOk: () => {
              this.getCopyData();
            }
          });
        }
      },
      {
        id: 'objectLevelRestore',
        label: objectLevelRestoreLabel(data.resource_sub_type),
        hidden: isObjectLevelRestoreHidden(),
        disabled: isObjectLevelRestoreDisabled(),
        onClick: () => {
          this.restoreService.restore({
            childResType: data.resource_sub_type,
            copyData: data,
            restoreType: RestoreType.FileRestore,
            isMessageBox: true,
            onOk: () => {
              this.getCopyData();
            }
          });
        }
      },
      {
        id: 'copyVerify',
        label: this.i18n.get('common_copies_verification_label'),
        disabled:
          (!includes(
            [
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.dbTwoTableSet.value
            ],
            data.resource_sub_type
          ) &&
            [DataMap.HCSCopyDataVerifyStatus.noGenerate.value].includes(
              properties.verifyStatus
            )) ||
          data.status !== DataMap.copydata_validStatus.normal.value ||
          includes(
            [
              DataMap.CopyData_generatedType.cloudArchival.value,
              DataMap.CopyData_generatedType.tapeArchival.value
            ],
            data.generated_by
          ) ||
          (data.status === DataMap.copydata_validStatus.invalid.value &&
            properties?.isMemberDeleted === 'true'),
        hidden:
          !includes(
            [
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.dbTwoTableSet.value,
              DataMap.Resource_Type.HCSCloudHost.value,
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.FusionCompute.value,
              DataMap.Resource_Type.fusionOne.value,
              DataMap.Resource_Type.KubernetesStatefulset.value,
              DataMap.Resource_Type.cNwareVm.value,
              DataMap.Resource_Type.APSCloudServer.value,
              DataMap.Resource_Type.hyperVVm.value
            ],
            data.resource_sub_type
          ) ||
          (includes(
            [
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.dbTwoTableSet.value
            ],
            data.resource_sub_type
          ) &&
            (!includes(
              [DataMap.CopyData_generatedType.backup.value],
              data.generated_by
            ) ||
              !includes(
                [
                  DataMap.CopyData_Backup_Type.full.value,
                  DataMap.CopyData_Backup_Type.incremental.value,
                  DataMap.CopyData_Backup_Type.diff.value
                ],
                data.backup_type
              ))),
        permission: OperateItems.RestoreCopy,
        tips: includes(
          [
            DataMap.CopyData_generatedType.cloudArchival.value,
            DataMap.CopyData_generatedType.tapeArchival.value
          ],
          data.generated_by
        )
          ? this.i18n.get('common_hcs_cloud_archive_verify_label')
          : properties.verifyStatus ===
              DataMap.HCSCopyDataVerifyStatus.noGenerate.value &&
            !includes(
              [
                DataMap.Resource_Type.dbTwoDatabase.value,
                DataMap.Resource_Type.dbTwoTableSet.value
              ],
              data.resource_sub_type
            )
          ? this.i18n.get('common_generate_verify_file_label')
          : '',
        onClick: () => {
          if (
            includes(
              [
                DataMap.Resource_Type.dbTwoDatabase.value,
                DataMap.Resource_Type.dbTwoTableSet.value
              ],
              data.resource_sub_type
            )
          ) {
            this.copyControllerService
              .ExecuteCopyVerifyTask({
                copyId: data.uuid,
                copyVerifyRequest: {
                  agents: ''
                }
              })
              .subscribe();
          } else {
            this.verifyCopy(data);
          }
        }
      },
      {
        id: 'schemaLevelRestore',
        label: includes(
          [DataMap.Resource_Type.ClickHouse.value],
          data.resource_sub_type
        )
          ? this.i18n.get('protection_table_level_restore_label')
          : this.i18n.get('common_schema_level_restore_label'),
        disabled:
          isSchemaLevelRestoreDisabled() ||
          (data.status === DataMap.copydata_validStatus.invalid.value &&
            properties?.isMemberDeleted === 'true'),
        hidden: isSchemaLevelRestoreHideen(),
        permission: OperateItems.SchemaLevelRestore,
        onClick: () => {
          this.restoreService.fileLevelRestore({
            header: includes(
              [DataMap.Resource_Type.ClickHouse.value],
              data.resource_sub_type
            )
              ? this.i18n.get('protection_table_level_restore_label')
              : this.i18n.get('common_schema_level_restore_label'),
            childResType: data.resource_sub_type,
            copyData: data,
            restoreType: RestoreType.FileRestore,
            onOk: () => {
              this.getCopyData();
            },
            restoreLevel: 'schema'
          });
        }
      },
      {
        id: 'fileLevelRestore',
        label: includes(
          [
            DataMap.Resource_Type.HBaseBackupSet.value,
            DataMap.Resource_Type.HiveBackupSet.value,
            DataMap.Resource_Type.DWS_Database.value,
            DataMap.Resource_Type.DWS_Cluster.value,
            DataMap.Resource_Type.DWS_Schema.value,
            DataMap.Resource_Type.DWS_Table.value,
            DataMap.Resource_Type.SQLServerDatabase.value,
            DataMap.Resource_Type.tidbDatabase.value,
            DataMap.Resource_Type.tidbCluster.value,
            DataMap.Resource_Type.OceanBaseCluster.value
          ],
          data.resource_sub_type
        )
          ? this.i18n.get('protection_table_level_restore_label')
          : includes(
              [
                DataMap.Resource_Type.SQLServerClusterInstance.value,
                DataMap.Resource_Type.SQLServerInstance.value,
                DataMap.Resource_Type.SQLServerGroup.value
              ],
              data.resource_sub_type
            )
          ? this.i18n.get('explore_database_restore_label')
          : includes(
              [DataMap.Resource_Type.ElasticsearchBackupSet.value],
              data.resource_sub_type
            )
          ? this.i18n.get('explore_index_level_restore_label')
          : includes(
              [DataMap.Resource_Type.Dameng_singleNode.value],
              data.resource_sub_type
            )
          ? this.i18n.get('common_file_table_level_restore_label')
          : includes(
              [DataMap.Resource_Type.ObjectSet.value],
              data.resource_sub_type
            )
          ? this.i18n.get('common_object_level_restore_label')
          : this.i18n.get('common_file_level_restore_label'),
        disabled: includes(
          [
            DataMap.Resource_Type.LocalFileSystem.value,
            DataMap.Resource_Type.HDFSFileset.value,
            DataMap.Resource_Type.FusionCompute.value,
            DataMap.Resource_Type.fusionOne.value,
            DataMap.Resource_Type.HCSCloudHost.value,
            DataMap.Resource_Type.openStackCloudServer.value,
            DataMap.Resource_Type.APSCloudServer.value,
            DataMap.Resource_Type.cNwareVm.value,
            DataMap.Resource_Type.hyperVVm.value,
            DataMap.Resource_Type.ndmp.value
          ],
          data.resource_sub_type
        )
          ? !(
              data.status === DataMap.copydata_validStatus.normal.value &&
              includes([DataMap.CopyData_fileIndex.indexed.value], data.indexed)
            )
          : data.status !== DataMap.copydata_validStatus.normal.value ||
            (includes(
              [
                DataMap.Resource_Type.NASShare.value,
                DataMap.Resource_Type.NASFileSystem.value
              ],
              data.resource_sub_type
            ) &&
              DataMap.CopyData_generatedType.cloudArchival.value ===
                data.generated_by &&
              data.indexed !== DataMap.CopyData_fileIndex.indexed.value) ||
            (includes(
              [DataMap.Resource_Type.DWS_Cluster.value],
              data.resource_sub_type
            ) &&
              get(JSON.parse(data.properties), 'replicate_count') > 2 &&
              data.generated_by ===
                DataMap.CopyData_generatedType.reverseReplication.value) ||
            (data.generated_by ===
              DataMap.CopyData_generatedType.liveMount.value &&
              includes(
                [DataMap.Resource_Type.fileset.value],
                data.resource_sub_type
              )) ||
            (includes(
              [DataMap.Resource_Type.volume.value],
              data.resource_sub_type
            ) &&
              data.indexed !== DataMap.CopyData_fileIndex.indexed.value) ||
            (data.status === DataMap.copydata_validStatus.invalid.value &&
              properties?.isMemberDeleted === 'true'),
        hidden:
          !includes(
            [
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.LocalFileSystem.value,
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.HBaseBackupSet.value,
              DataMap.Resource_Type.HiveBackupSet.value,
              DataMap.Resource_Type.ElasticsearchBackupSet.value,
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.volume.value,
              DataMap.Resource_Type.Dameng_singleNode.value,
              DataMap.Resource_Type.DWS_Database.value,
              DataMap.Resource_Type.DWS_Cluster.value,
              DataMap.Resource_Type.DWS_Schema.value,
              DataMap.Resource_Type.DWS_Table.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerGroup.value,
              DataMap.Resource_Type.FusionCompute.value,
              DataMap.Resource_Type.fusionOne.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.OceanBaseCluster.value,
              DataMap.Resource_Type.HCSCloudHost.value,
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.ObjectSet.value,
              DataMap.Resource_Type.APSCloudServer.value,
              DataMap.Resource_Type.cNwareVm.value,
              DataMap.Resource_Type.hyperVVm.value,
              DataMap.Resource_Type.ndmp.value,
              DataMap.Resource_Type.virtualMachine.value
            ],
            data.resource_sub_type
          ) ||
          (includes(
            [
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.NASFileSystem.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [DataMap.CopyData_generatedType.liveMount.value],
              data.generated_by
            )) ||
          this.isHyperdetect ||
          includes(
            [
              DataMap.CopyData_generatedType.tapeArchival.value,
              DataMap.CopyData_generatedType.import.value,
              DataMap.CopyData_generatedType.Imported.value
            ],
            data.generated_by
          ) ||
          (includes(
            [
              DataMap.Resource_Type.HBaseBackupSet.value,
              DataMap.Resource_Type.HiveBackupSet.value,
              DataMap.Resource_Type.ElasticsearchBackupSet.value,
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.SQLServerGroup.value,
              DataMap.Resource_Type.SQLServerDatabase.value,
              DataMap.Resource_Type.OceanBaseCluster.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.ObjectSet.value,
              DataMap.Resource_Type.oracle.value,
              DataMap.Resource_Type.oracleCluster.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [DataMap.CopyData_generatedType.cloudArchival.value],
              data.generated_by
            )) ||
          (includes(
            [
              DataMap.Resource_Type.Dameng_singleNode.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.OceanBaseCluster.value,
              DataMap.Resource_Type.oracle.value,
              DataMap.Resource_Type.oracleCluster.value
            ],
            data.resource_sub_type
          ) &&
            data.backup_type === DataMap.CopyData_Backup_Type.log.value) ||
          (includes(
            [DataMap.Resource_Type.Dameng_singleNode.value],
            data.resource_sub_type
          ) &&
            data.generated_by !==
              DataMap.CopyData_generatedType.backup.value) ||
          hiddenDwsFileLevelRestore(data) ||
          (includes(
            [
              DataMap.Resource_Type.DWS_Cluster.value,
              DataMap.Resource_Type.DWS_Schema.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [
                DataMap.CopyData_generatedType.replicate.value,
                DataMap.CopyData_generatedType.cascadedReplication.value
              ],
              data.generated_by
            )) ||
          (includes(
            [
              DataMap.Resource_Type.DWS_Cluster.value,
              DataMap.Resource_Type.DWS_Schema.value
            ],
            data.resource_sub_type
          ) &&
            data.resource_status === DataMap.Resource_Status.notExist.value) ||
          (data.generated_by ===
            DataMap.CopyData_generatedType.cloudArchival.value &&
            includes(
              [DataMap.Resource_Type.fileset.value],
              data.resource_sub_type
            ) &&
            data.indexed !== DataMap.CopyData_fileIndex.indexed.value) ||
          hiddenHcsUserFileLevelRestore(data, this.isHcsUser) ||
          ([
            DataMap.Resource_Type.fileset.value,
            DataMap.Resource_Type.volume.value,
            DataMap.Resource_Type.NASShare.value,
            DataMap.Resource_Type.NASFileSystem.value
          ].includes(data.resource_sub_type) &&
            includes(
              [
                DataMap.CopyData_generatedType.replicate.value,
                DataMap.CopyData_generatedType.cascadedReplication.value,
                DataMap.CopyData_generatedType.reverseReplication.value
              ],
              data.generated_by
            )),
        permission: OperateItems.FileLevelRestore,
        onClick: () => {
          // vmware文件级恢复打开副本详情
          if (
            data.resource_sub_type ===
            DataMap.Resource_Type.virtualMachine.value
          ) {
            this.getDetail(data, true);
            return;
          }
          this.restoreService.fileLevelRestore({
            header: includes(
              [
                DataMap.Resource_Type.HBaseBackupSet.value,
                DataMap.Resource_Type.HiveBackupSet.value,
                DataMap.Resource_Type.DWS_Database.value,
                DataMap.Resource_Type.DWS_Cluster.value,
                DataMap.Resource_Type.DWS_Schema.value,
                DataMap.Resource_Type.DWS_Table.value,
                DataMap.Resource_Type.SQLServerDatabase.value,
                DataMap.Resource_Type.OceanBaseCluster.value,
                DataMap.Resource_Type.tidbCluster.value,
                DataMap.Resource_Type.tidbDatabase.value,
                DataMap.Resource_Type.oracle.value,
                DataMap.Resource_Type.oracleCluster.value
              ],
              data.resource_sub_type
            )
              ? this.i18n.get('protection_table_level_restore_label')
              : includes(
                  [
                    DataMap.Resource_Type.SQLServerClusterInstance.value,
                    DataMap.Resource_Type.SQLServerInstance.value,
                    DataMap.Resource_Type.SQLServerGroup.value
                  ],
                  data.resource_sub_type
                )
              ? this.i18n.get('explore_database_restore_label')
              : includes(
                  [DataMap.Resource_Type.Dameng_singleNode.value],
                  data.resource_sub_type
                )
              ? this.i18n.get('common_file_table_level_restore_label')
              : includes(
                  [DataMap.Resource_Type.ElasticsearchBackupSet.value],
                  data.resource_sub_type
                )
              ? this.i18n.get('explore_index_level_restore_label')
              : includes(
                  [DataMap.Resource_Type.ObjectSet.value],
                  data.resource_sub_type
                )
              ? this.i18n.get('common_object_level_restore_label')
              : this.i18n.get('common_file_level_restore_label'),
            childResType: data.resource_sub_type,
            copyData: data,
            restoreType: RestoreType.FileRestore,
            onOk: () => {
              this.getCopyData();
            }
          });
        }
      },
      {
        id: 'singleFileRestore',
        label: this.i18n.get('common_single_file_level_restore_label'),
        hidden: !includes(
          [
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oracleCluster.value
          ],
          data.resource_sub_type
        ),
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          this.restoreService.singleFileRestore({
            header: this.i18n.get('common_single_file_level_restore_label'),
            childResType: data.resource_sub_type,
            copyData: assign(data, { singleFileRestore: true }),
            restoreType: RestoreType.FileRestore,
            onOk: () => {
              this.getCopyData();
            }
          });
        }
      },
      {
        id: 'instantRestore',
        label: this.i18n.get('common_live_restore_job_label'),
        disabled: !(
          data.status === DataMap.copydata_validStatus.normal.value ||
          (includes(
            [DataMap.Resource_Type.cNwareVm.value],
            data.resource_sub_type
          ) &&
            data.status === DataMap.copydata_validStatus.invalid.value)
        ),
        hidden:
          includes(
            [
              DataMap.Resource_Type.GaussDB_T.value,
              DataMap.Resource_Type.gaussdbTSingle.value,
              DataMap.Resource_Type.OpenGauss_database.value,
              DataMap.Resource_Type.OpenGauss_instance.value,
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.Dameng_cluster.value,
              DataMap.Resource_Type.Dameng_singleNode.value,
              DataMap.Resource_Type.ImportCopy.value,
              DataMap.Resource_Type.LocalFileSystem.value,
              DataMap.Resource_Type.Redis.value,
              DataMap.Resource_Type.ClickHouse.value,
              DataMap.Resource_Type.HCSCloudHost.value,
              DataMap.Resource_Type.PostgreSQLInstance.value,
              DataMap.Resource_Type.PostgreSQLClusterInstance.value,
              DataMap.Resource_Type.KingBaseInstance.value,
              DataMap.Resource_Type.KingBaseClusterInstance.value,
              DataMap.Resource_Type.ElasticsearchBackupSet.value,
              DataMap.Resource_Type.KubernetesStatefulset.value,
              DataMap.Resource_Type.HiveBackupSet.value,
              DataMap.Resource_Type.DWS_Cluster.value,
              DataMap.Resource_Type.DWS_Database.value,
              DataMap.Resource_Type.DWS_Schema.value,
              DataMap.Resource_Type.DWS_Table.value,
              DataMap.Resource_Type.FusionCompute.value,
              DataMap.Resource_Type.fusionOne.value,
              DataMap.Resource_Type.generalDatabase.value,
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.dbTwoTableSet.value,
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.goldendbInstance.value,
              DataMap.Resource_Type.informixInstance.value,
              DataMap.Resource_Type.informixClusterInstance.value,
              DataMap.Resource_Type.lightCloudGaussdbInstance.value,
              DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
              DataMap.Resource_Type.MongodbClusterInstance.value,
              DataMap.Resource_Type.MongodbSingleInstance.value,
              DataMap.Resource_Type.kubernetesDatasetCommon.value,
              DataMap.Resource_Type.kubernetesNamespaceCommon.value,
              DataMap.Resource_Type.tdsqlDistributedInstance.value,
              DataMap.Resource_Type.ActiveDirectory.value,
              DataMap.Resource_Type.ExchangeSingle.value,
              DataMap.Resource_Type.ExchangeGroup.value,
              DataMap.Resource_Type.ExchangeDataBase.value,
              DataMap.Resource_Type.ExchangeEmail.value,
              DataMap.Resource_Type.commonShare.value,
              DataMap.Resource_Type.saphanaDatabase.value,
              DataMap.Resource_Type.ObjectSet.value
            ],
            data.resource_sub_type
          ) ||
          ((data.features ? features[2] !== '1' : false) &&
            !includes(
              [DataMap.Resource_Type.cNwareVm.value],
              data.resource_sub_type
            )) ||
          (includes(
            [
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.MySQLClusterInstance.value,
              DataMap.Resource_Type.MySQLInstance.value,
              DataMap.Resource_Type.MySQLDatabase.value,
              DataMap.Resource_Type.tdsqlInstance.value,
              DataMap.Resource_Type.OceanBaseCluster.value,
              DataMap.Resource_Type.OceanBaseTenant.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [
                DataMap.CopyData_generatedType.liveMount.value,
                DataMap.CopyData_generatedType.replicate.value,
                DataMap.CopyData_generatedType.cascadedReplication.value,
                DataMap.CopyData_generatedType.reverseReplication.value
              ],
              data.generated_by
            )) ||
          includes(
            [
              DataMap.CopyData_generatedType.tapeArchival.value,
              DataMap.CopyData_generatedType.import.value
            ],
            data.generated_by
          ) ||
          (includes(
            [
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.HBaseBackupSet.value,
              DataMap.Resource_Type.SQLServerCluster.value,
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerDatabase.value,
              DataMap.Resource_Type.SQLServerGroup.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.tidbTable.value,
              DataMap.Resource_Type.volume.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [
                DataMap.CopyData_generatedType.replicate.value,
                DataMap.CopyData_generatedType.cascadedReplication.value,
                DataMap.CopyData_generatedType.reverseReplication.value
              ],
              data.generated_by
            )) ||
          isHideOracleInstanceRestore(resourceProperties),
        permission: OperateItems.InstanceRecovery,
        onClick: () =>
          this.restoreService.restore({
            childResType: data.resource_sub_type,
            isMessageBox: includes(
              [DataMap.Resource_Type.cNwareVm.value],
              data.resource_sub_type
            ),
            copyData: data,
            restoreType: RestoreType.InstanceRestore,
            onOk: () => {
              this.getCopyData();
            }
          })
      },
      {
        id: 'mount',
        label: this.i18n.get('common_live_mount_label'),
        disabled:
          DataMap.copydata_validStatus.normal.value !== data.status ||
          (data.generated_by === DataMap.CopyData_generatedType.liveMount.value
            ? data.generation > DataMap.CopyData_Generation.two.value
            : data.generation >= DataMap.CopyData_Generation.two.value &&
              !includes(
                [
                  DataMap.CopyData_generatedType.cascadedReplication.value,
                  DataMap.CopyData_generatedType.reverseReplication.value
                ],
                data.generated_by
              )) ||
          (data.resource_sub_type ===
            DataMap.Resource_Type.virtualMachine.value &&
            resourceProperties.ext_parameters &&
            isBoolean(resourceProperties.ext_parameters.all_disk) &&
            !resourceProperties.ext_parameters.all_disk) ||
          (includes(
            [
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.NASShare.value
            ],
            data.resource_sub_type
          ) &&
            get(JSON.parse(data.properties || '{}'), 'isAggregation') ===
              'true') ||
          (data.status === DataMap.copydata_validStatus.invalid.value &&
            properties?.isMemberDeleted === 'true') ||
          !hasLivemountPermission(data),
        hidden:
          includes(
            [
              DataMap.Resource_Type.GaussDB_T.value,
              DataMap.Resource_Type.gaussdbTSingle.value,
              DataMap.Resource_Type.OpenGauss_database.value,
              DataMap.Resource_Type.OpenGauss_instance.value,
              DataMap.Resource_Type.Dameng_cluster.value,
              DataMap.Resource_Type.Dameng_singleNode.value,
              DataMap.Resource_Type.ImportCopy.value,
              DataMap.Resource_Type.LocalFileSystem.value,
              DataMap.Resource_Type.KubernetesStatefulset.value,
              DataMap.Resource_Type.ElasticsearchBackupSet.value,
              DataMap.Resource_Type.HiveBackupSet.value,
              DataMap.Resource_Type.ClickHouse.value,
              DataMap.Resource_Type.DWS_Cluster.value,
              DataMap.Resource_Type.DWS_Database.value,
              DataMap.Resource_Type.DWS_Schema.value,
              DataMap.Resource_Type.DWS_Table.value,
              DataMap.Resource_Type.Redis.value,
              DataMap.Resource_Type.HCSCloudHost.value,
              DataMap.Resource_Type.FusionCompute.value,
              DataMap.Resource_Type.fusionOne.value,
              DataMap.Resource_Type.PostgreSQLInstance.value,
              DataMap.Resource_Type.PostgreSQLClusterInstance.value,
              DataMap.Resource_Type.KingBaseInstance.value,
              DataMap.Resource_Type.KingBaseClusterInstance.value,
              DataMap.Resource_Type.generalDatabase.value,
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.dbTwoTableSet.value,
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.goldendbInstance.value,
              DataMap.Resource_Type.informixInstance.value,
              DataMap.Resource_Type.informixClusterInstance.value,
              DataMap.Resource_Type.lightCloudGaussdbInstance.value,
              DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
              DataMap.Resource_Type.MongodbClusterInstance.value,
              DataMap.Resource_Type.MongodbSingleInstance.value,
              DataMap.Resource_Type.kubernetesDatasetCommon.value,
              DataMap.Resource_Type.kubernetesNamespaceCommon.value,
              DataMap.Resource_Type.tdsqlDistributedInstance.value,
              DataMap.Resource_Type.ActiveDirectory.value,
              DataMap.Resource_Type.commonShare.value,
              DataMap.Resource_Type.saphanaDatabase.value,
              DataMap.Resource_Type.ObjectSet.value
            ],
            data.resource_sub_type
          ) ||
          ((data.features ? features[3] !== '1' : false) &&
            !includes(
              [
                DataMap.Resource_Type.fileset.value,
                DataMap.Resource_Type.volume.value,
                DataMap.Resource_Type.tdsqlInstance.value,
                DataMap.Resource_Type.cNwareVm.value
              ],
              data.resource_sub_type
            )) ||
          includes(
            [
              DataMap.CopyData_generatedType.tapeArchival.value,
              DataMap.CopyData_generatedType.import.value
            ],
            data.generated_by
          ) ||
          (includes(
            [
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.HBaseBackupSet.value,
              DataMap.Resource_Type.ElasticsearchBackupSet.value,
              DataMap.Resource_Type.SQLServerCluster.value,
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerDatabase.value,
              DataMap.Resource_Type.SQLServerGroup.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.tidbTable.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [
                DataMap.CopyData_generatedType.replicate.value,
                DataMap.CopyData_generatedType.cascadedReplication.value,
                DataMap.CopyData_generatedType.reverseReplication.value
              ],
              data.generated_by
            )) ||
          (includes(
            [
              DataMap.Resource_Type.MySQLClusterInstance.value,
              DataMap.Resource_Type.MySQLInstance.value,
              DataMap.Resource_Type.MySQLDatabase.value,
              DataMap.Resource_Type.tdsqlInstance.value,
              DataMap.Resource_Type.OceanBaseCluster.value,
              DataMap.Resource_Type.OceanBaseTenant.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [
                DataMap.CopyData_generatedType.liveMount.value,
                DataMap.CopyData_generatedType.replicate.value,
                DataMap.CopyData_generatedType.cascadedReplication.value,
                DataMap.CopyData_generatedType.reverseReplication.value
              ],
              data.generated_by
            )) ||
          (includes(
            [DataMap.Resource_Type.fileset.value],
            data.resource_sub_type
          ) &&
            data.generated_by !==
              DataMap.CopyData_generatedType.backup.value) ||
          (includes(
            [
              DataMap.Resource_Type.oracle.value,
              DataMap.Resource_Type.oracleCluster.value,
              DataMap.Resource_Type.tdsqlInstance.value,
              DataMap.Resource_Type.MySQLInstance.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [DataMap.CopyData_Backup_Type.log.value],
              data.backup_type
            )) ||
          (this.appUtilsService.isDistributed &&
            includes(
              [
                DataMap.Resource_Type.fileset.value,
                DataMap.Resource_Type.NASShare.value
              ],
              data.resource_sub_type
            )) ||
          isHideOracleMount(resourceProperties) ||
          (includes(
            [
              DataMap.Resource_Type.tdsqlInstance.value,
              DataMap.Resource_Type.volume.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [
                DataMap.CopyData_generatedType.cloudArchival.value,
                DataMap.CopyData_generatedType.tapeArchival.value
              ],
              data.generated_by
            )),
        permission: OperateItems.MountingCopy,
        onClick: () => this.mount(data)
      },
      {
        id: 'copy',
        disabled:
          +data.generation === 3 ||
          !includes([DataMap.copydata_validStatus.normal.value], data.status) ||
          !hasReplicationPermission(data),
        tips:
          +data.generation === 3
            ? this.i18n.get('protection_copy_disable_tip_label')
            : '',
        label: this.i18n.get('common_replicate_label'),
        permission: OperateItems.CopyDuplicate,
        hidden:
          !includes(
            [
              DataMap.CopyData_generatedType.cascadedReplication.value,
              DataMap.CopyData_generatedType.replicate.value
            ],
            data.generated_by
          ) ||
          (includes(
            [
              DataMap.Resource_Type.DWS_Cluster.value,
              DataMap.Resource_Type.DWS_Database.value,
              DataMap.Resource_Type.DWS_Schema.value,
              DataMap.Resource_Type.DWS_Table.value
            ],
            data.resource_sub_type
          ) &&
            !!get(JSON.parse(data.properties), 'storage_id')),
        onClick: () =>
          this.copyActionService.copyReplicate(data, () => {
            this.getCopyData();
          })
      },
      {
        id: 'archive',
        label: this.i18n.get('common_manual_archive_label'),
        disabled:
          !includes(
            [
              DataMap.CopyData_generatedType.replicate.value,
              DataMap.CopyData_generatedType.backup.value,
              DataMap.CopyData_generatedType.Imported.value
            ],
            data.generated_by
          ) ||
          !includes([DataMap.copydata_validStatus.normal.value], data.status),
        onClick: () =>
          this.takeManualArchiveService.manualArchive(data, () => {
            this.getCopyData();
          })
      },
      {
        id: 'modify',
        disabled:
          !includes(
            [
              DataMap.copydata_validStatus.normal.value,
              DataMap.copydata_validStatus.deleteFailed.value
            ],
            data.status
          ) ||
          (data.status === DataMap.copydata_validStatus.invalid.value &&
            properties?.isMemberDeleted === 'true'),
        label: this.i18n.get('common_modify_retention_policy_label'),
        permission: OperateItems.ModifyingCopyRetentionPolicy,
        hidden:
          this.hiddenColumn ||
          (includes(
            [DataMap.Resource_Type.ImportCopy.value],
            data.resource_sub_type
          ) &&
            DataMap.CopyData_generatedType.download.value ===
              data.generated_by) ||
          (data.resource_sub_type ===
            DataMap.Resource_Type.HBaseBackupSet.value &&
            data.backup_type === DataMap.CopyData_Backup_Type.log.value) ||
          data.generated_by ===
            DataMap.CopyData_generatedType.tapeArchival.value,
        onClick: () => this.modifyRetention(data)
      },
      {
        id: 'download',
        label: this.i18n.get('common_download_label'),
        permission: OperateItems.DownloadCopy,
        disabled:
          !includes(
            [
              DataMap.copydata_validStatus.normal.value,
              DataMap.copydata_validStatus.invalid.value
            ],
            data.status
          ) ||
          (data.status === DataMap.copydata_validStatus.invalid.value &&
            properties?.isMemberDeleted === 'true'),
        hidden:
          !(
            includes(
              [DataMap.Resource_Type.ImportCopy.value],
              data.resource_sub_type
            ) &&
            data.generated_by ===
              DataMap.CopyData_generatedType.tapeArchival.value
          ) ||
          includes(
            [DataMap.CopyData_generatedType.import.value],
            data.generated_by
          ),
        onClick: () => {
          this.messageBox.info({
            lvHeader: this.i18n.get('common_tape_archive_download_label'),
            lvContent: this.i18n.get('common_tape_archive_download_tip_label', [
              this.datePipe.transform(
                data.display_timestamp,
                'yyyy-MM-dd HH:mm:ss'
              )
            ]),
            lvFooter: [
              {
                label: this.i18n.get('common_ok_label'),
                onClick: modal => {
                  this.tapeCopyApiService
                    .downloadDwsCopy({
                      copyId: data.uuid
                    })
                    .subscribe(
                      res => {
                        this.message.success(
                          this.i18n.get('common_operate_success_label')
                        );
                        modal.close();
                      },
                      err => {
                        modal.close();
                      }
                    );
                }
              },
              {
                label: this.i18n.get('common_cancel_label'),
                onClick: modal => {
                  modal.close();
                }
              }
            ],
            lvAfterClose: result => {
              if (result && result.trigger === 'close') {
                this.getCopyData();
              }
            }
          });
        }
      },
      {
        id: 'delete',
        hidden:
          this.hiddenColumn ||
          (data.resource_sub_type === DataMap.Resource_Type.ImportCopy.value &&
            DataMap.CopyData_generatedType.import.value ===
              data.generated_by) ||
          data.backup_type === DataMap.CopyData_Backup_Type.log.value ||
          data.generated_by ===
            DataMap.CopyData_generatedType.tapeArchival.value ||
          (data.resource_sub_type ===
            DataMap.Resource_Type.lightCloudGaussdbInstance.value &&
            data.backup_type ===
              DataMap.CopyData_Backup_Type.incremental.value),
        disabled:
          !includes(
            [
              DataMap.copydata_validStatus.normal.value,
              DataMap.copydata_validStatus.invalid.value,
              DataMap.copydata_validStatus.deleteFailed.value
            ],
            data.status
          ) || !hasCopyDeletePermission(data),
        divide: includes(
          [
            DataMap.Resource_Type.NASShare.value,
            DataMap.Resource_Type.NASFileSystem.value,
            DataMap.Resource_Type.HDFSFileset.value
          ],
          data.resource_sub_type
        ),
        permission: OperateItems.DeletingCopy,
        label: this.i18n.get('common_delete_label'),
        onClick: () => {
          if (
            this._isWorm(data) &&
            data.backup_type !== DataMap.CopyData_Backup_Type.snapshot.value
          ) {
            this.message.error(
              this.i18n.get('explore_worm_policy_error_del_label'),
              {
                lvMessageKey: 'lvMsg_key_explore_worm_policy_error_del_label',
                lvShowCloseButton: true
              }
            );
            return;
          }
          this.warningMessageService.create({
            content: this.i18n.get('common_copy_delete_label', [
              this.datePipe.transform(
                data.display_timestamp,
                'yyyy-MM-dd HH:mm:ss'
              )
            ]),
            onOK: () => {
              this.copiesApiService
                .deleteCopyV1CopiesCopyIdDelete({
                  copyId: data.uuid
                })
                .subscribe(res => {
                  if (
                    includes(
                      mapValues(this.drawModalService.modals, 'key'),
                      'copy-detail-modal'
                    )
                  ) {
                    this.drawModalService.destroyModal('copy-detail-modal');
                  }
                  this.getCopyData();
                });
            }
          });
        }
      },
      {
        id: 'exportFile',
        label: this.i18n.get('protection_download_file_label'),
        permission: OperateItems.ExportFile,
        divide: true,
        disabled:
          DataMap.CopyData_generatedType.replicate.value ===
            data.generated_by ||
          data.indexed !== DataMap.CopyData_fileIndex.indexed.value ||
          (includes(
            [
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.fileset.value
            ],
            data.resource_sub_type
          ) &&
            get(JSON.parse(data.properties || '{}'), 'isAggregation') ===
              'true') ||
          (data.status === DataMap.copydata_validStatus.invalid.value &&
            properties?.isMemberDeleted === 'true'),
        hidden:
          !includes(
            [
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.NASFileSystem.value
            ],
            data.resource_sub_type
          ) ||
          includes(
            [
              DataMap.CopyData_generatedType.replicate.value,
              DataMap.CopyData_generatedType.cascadedReplication.value,
              DataMap.CopyData_generatedType.reverseReplication.value,
              DataMap.CopyData_generatedType.tapeArchival.value,
              DataMap.CopyData_generatedType.import.value,
              DataMap.CopyData_generatedType.cloudArchival.value,
              DataMap.CopyData_generatedType.liveMount.value
            ],
            data.generated_by
          ),
        onClick: () => this.copyActionService.exportFile(data)
      },
      {
        id: 'manualIndexing',
        label: this.i18n.get('protection_enable_recovery_label'),
        permission: OperateItems.ManualIndex,
        disabled:
          this.isEncrypted(data) ||
          !includes(
            [
              DataMap.CopyData_fileIndex.unIndexed.value,
              DataMap.CopyData_fileIndex.deletedFailed.value
            ],
            data.indexed
          ) ||
          data.status !== DataMap.copydata_validStatus.normal.value ||
          (includes(
            [DataMap.Resource_Type.ObjectSet.value],
            data.resource_sub_type
          ) &&
            JSON.parse(data.resource_properties).ext_parameters
              .multiNodeBackupSwitch) ||
          !hasCopyIndexPermission(data),
        tips: this.isEncrypted(data)
          ? this.i18n.get('protection_hcs_encryption_index_tip_label')
          : '',
        hidden:
          !includes(
            [
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.volume.value,
              DataMap.Resource_Type.FusionCompute.value,
              DataMap.Resource_Type.fusionOne.value,
              DataMap.Resource_Type.HCSCloudHost.value,
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.ObjectSet.value,
              DataMap.Resource_Type.APSCloudServer.value,
              DataMap.Resource_Type.cNwareVm.value,
              DataMap.Resource_Type.hyperVVm.value,
              DataMap.Resource_Type.ndmp.value,
              DataMap.Resource_Type.virtualMachine.value
            ],
            data.resource_sub_type
          ) ||
          includes(
            [
              DataMap.CopyData_generatedType.cascadedReplication.value,
              DataMap.CopyData_generatedType.reverseReplication.value,
              DataMap.CopyData_generatedType.tapeArchival.value,
              DataMap.CopyData_generatedType.import.value,
              DataMap.CopyData_generatedType.cloudArchival.value,
              DataMap.CopyData_generatedType.liveMount.value
            ],
            data.generated_by
          ) ||
          (DataMap.CopyData_generatedType.replicate.value ===
            data.generated_by &&
            !includes(
              [
                DataMap.Resource_Type.FusionCompute.value,
                DataMap.Resource_Type.fusionOne.value,
                DataMap.Resource_Type.HCSCloudHost.value,
                DataMap.Resource_Type.openStackCloudServer.value,
                DataMap.Resource_Type.APSCloudServer.value,
                DataMap.Resource_Type.cNwareVm.value,
                DataMap.Resource_Type.hyperVVm.value
              ],
              data.resource_sub_type
            )) ||
          (includes(
            [
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.HDFSFileset.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [DataMap.CopyData_generatedType.Imported.value],
              data.generated_by
            )) ||
          this.isHcsUser,
        onClick: () => {
          this.copiesApiService
            .createCopyIndexV1CopiesCopyIdActionCreateIndexPost({
              copyId: data.uuid
            })
            .subscribe(res => this.getCopyData());
        }
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  }

  mount(item) {
    this.manualMountService.create({
      item,
      resType: this.resType,
      onOk: () => {
        this.getCopyData();
      }
    });
  }

  isEncrypted(data) {
    // 判断副本是否为包含加密卷的HCSCloud副本
    return (
      data.resource_sub_type === DataMap.Resource_Type.HCSCloudHost.value &&
      JSON.parse(data.properties || '{}')?.volList?.some(
        item => JSON.parse(item.extendInfo || '{}')?.systemEncrypted === '1'
      )
    );
  }

  trackByUuid = (index, item) => {
    return item.uuid;
  };

  _getCopyVerifyStatus(item) {
    if (
      includes(
        [
          DataMap.CopyData_generatedType.cloudArchival.value,
          DataMap.CopyData_generatedType.tapeArchival.value
        ],
        item.generated_by
      )
    ) {
      return;
    }
    const properties = JSON.parse(get(item, 'properties', '{}'));
    const verifyStatus = get(properties, 'verifyStatus', '');

    assign(item, {
      copy_verify_status: verifyStatus
    });
  }

  onColumnsChange() {
    const showColumnsNum = this.columns.filter(
      item => item.isShow && !item.hidden
    ).length;
    this.tableScrollParams['x'] =
      showColumnsNum > 10 ? `${100 + 100 * showColumnsNum}px` : undefined;
  }

  verifyCopy(rowCopy) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-db-two-instance',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('common_copies_verification_label'),
        lvContent: CopyVerifyComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowCopy
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CopyVerifyComponent;
          const modalIns = modal.getInstance();

          content.formGroup.statusChanges.subscribe(val => {
            modalIns.lvOkDisabled = !(val === 'VALID');
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CopyVerifyComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  assignNewAttribute(data) {
    if (this.resType === DataMap.Resource_Type.HBaseBackupSet.value) {
      each(data, item => {
        assign(item, { environment_uuid: this.rowData.environment_uuid });
      });
    } else if (this.resType === DataMap.Resource_Type.ActiveDirectory.value) {
      each(data, item => {
        const resourceProperties = isString(item.resource_properties)
          ? JSON.parse(item.resource_properties)
          : item.resource_properties;
      });
    }
  }
}
