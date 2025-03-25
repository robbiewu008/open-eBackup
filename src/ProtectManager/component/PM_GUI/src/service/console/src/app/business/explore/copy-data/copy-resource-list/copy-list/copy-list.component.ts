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
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnDestroy,
  OnInit,
  Output,
  Pipe,
  PipeTransform
} from '@angular/core';
import { MessageboxService, MessageService } from '@iux/live';
import { EXCLUDE_RESOURCE_TYPES } from 'app/business/explore/policy/anti-policy-setting/anti-policy/create-anti-policy/create-anti-policy.component';
import {
  autoTableScroll,
  CommonConsts,
  CommonShareRestoreApiService,
  CookieService,
  CopyControllerService,
  DataMap,
  DataMapService,
  disableOpenstackVmRestore,
  disableOracleRestoreNewLocation,
  disableOracleRestoreOriginalLocation,
  disableValidCopyBtn,
  filterBackupType,
  filterGeneratedBy,
  GenerationType,
  getCommonRestoreLabel,
  getCopyStatusFilterOptions,
  getGeneralDatabaseConf,
  getLabelList,
  getPermissionMenuItem,
  GlobalService,
  hasArchivePermission,
  hasCopyDeletePermission,
  hasCopyIndexPermission,
  hasLivemountPermission,
  hasRecoveryPermission,
  hasReplicationPermission,
  hiddenDwsFileLevelRestore,
  hiddenHcsUserFileLevelRestore,
  hiddenOracleFileLevelRestore,
  hideE6000Func,
  I18NService,
  isHideOracleInstanceRestore,
  isHideOracleMount,
  isHideWorm,
  isIncompleteOracleCopy,
  isIndexedFilesetCLoudArchival,
  MODAL_COMMON,
  OperateItems,
  ResourceType,
  RestoreType,
  RestoreV2Type,
  StorageUnitService,
  TapeCopyApiService,
  WarningMessageService,
  WormStatusEnum
} from 'app/shared';
import { CopiesService } from 'app/shared/api/services/copies.service';
import {
  ModifyRetentionPolicyComponent,
  WormSetComponent
} from 'app/shared/components';
import { CopyDataDetailComponent } from 'app/shared/components/copy-data-detail/copy-data-detail.component';
import { CopyVerifyComponent } from 'app/shared/components/copy-verify-proxy/copy-verify.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { CopyActionService } from 'app/shared/services/copy-action.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ManualMountService } from 'app/shared/services/manual-mount.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { RestoreService } from 'app/shared/services/restore.service';
import { TakeManualArchiveService } from 'app/shared/services/take-manual-archive.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  findIndex,
  get,
  includes,
  intersection,
  isBoolean,
  isEmpty,
  isFunction,
  isNil,
  isString,
  isUndefined,
  map,
  mapValues,
  reject,
  size,
  trim
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';

@Pipe({ name: 'filterTable' })
export class FilterPipe implements PipeTransform {
  constructor(private cookieService: CookieService) {}

  transform(value: any[]) {
    return filter(value, item =>
      this.cookieService.isCloudBackup
        ? item['backup_type'] === DataMap.CopyData_Backup_Type.full.value
        : true
    );
  }
}

@Component({
  selector: 'aui-copy-list',
  templateUrl: './copy-list.component.html',
  styleUrls: ['./copy-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [DatePipe]
})
export class CopyListComponent implements OnInit, OnDestroy, AfterViewInit {
  copyName;
  slaName;
  resourceName;
  resourceSubType;
  resourceLocation;
  copyLocation;
  copyUuid;
  backupCopyUuid;
  copyClusterName;
  clickHouseTypeFilter;
  orders = ['-display_timestamp'];
  selection = [];
  tableData = [];
  timeSub$: Subscription;
  destroy$ = new Subject();
  fileLevelMountSubscription;
  dataMap = DataMap;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  columns = [];
  filterParams = {};
  activeSort = {};
  columnSelection = [];
  filter = filter;
  _parse = JSON.parse;
  _get = get;
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
  isExpirationTips = !includes(
    // 是否加过期时间提示，因为提示词涉及worm，对这些形态屏蔽
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value,
      DataMap.Deploy_Type.e6000.value
    ],
    this.i18n.get('deploy_type')
  );
  isVirtualizationAndCloud = false;
  activeItem;
  storageUnitOptions = [];

  subTypeTextMapKey = 'Job_Target_Type';

  @Input() resourceType;
  @Input() childResourceType;
  @Input() isResourceSet = false; // 用于资源集判断
  @Input() data;
  @Output() onSelectionChange = new EventEmitter<any>();
  @Output() onNumChange = new EventEmitter<any>();

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private detailService: ResourceDetailService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private copiesApiService: CopiesService,
    private restoreService: RestoreService,
    private warningMessageService: WarningMessageService,
    private manualMountService: ManualMountService,
    private messageBox: MessageboxService,
    private message: MessageService,
    private tapeCopyApiService: TapeCopyApiService,
    private takeManualArchiveService: TakeManualArchiveService,
    public cookieService: CookieService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private copyActionService: CopyActionService,
    private copyControllerService: CopyControllerService,
    private globalService: GlobalService,
    private commonShareRestoreApiService: CommonShareRestoreApiService,
    public appUtilsService: AppUtilsService,
    private storageUnitService: StorageUnitService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
    if (this.fileLevelMountSubscription) {
      this.fileLevelMountSubscription.unsubscribe();
    }
  }

  ngAfterViewInit() {
    if (this.isVirtualizationAndCloud) {
      this.fileLevelMountSubscription = this.globalService
        .getState('fileLevelExploreMount')
        .subscribe(res => {
          if (res === 'mount') {
            this.getCopies(false);
          }
        });
    }
  }

  ngOnInit() {
    this.preProcessData();
    this.getColumns();
    if (this.appUtilsService.isDecouple || this.appUtilsService.isDistributed) {
      this.getStorageUnit();
    } else {
      this.getCopies();
    }
    this.initColumnSelection();
    this.virtualScroll.getScrollParam(this.isOceanProtect ? 270 : 170);
    autoTableScroll(
      this.virtualScroll,
      this.isOceanProtect ? 270 : 170,
      null,
      this.cdr
    );
  }

  preProcessData() {
    this.isVirtualizationAndCloud =
      !!size(
        intersection(this.childResourceType, [
          DataMap.Resource_Type.virtualMachine.value,
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.fusionOne.value,
          DataMap.Resource_Type.HCSCloudHost.value,
          DataMap.Resource_Type.openStackCloudServer.value,
          DataMap.Resource_Type.APSCloudServer.value,
          DataMap.Resource_Type.cNwareVm.value,
          DataMap.Resource_Type.hyperVVm.value,
          DataMap.Resource_Type.nutanixVm.value
        ])
      ) && !this.isHcsUser;
  }

  getStorageUnit() {
    this.storageUnitService
      .queryBackUnitGET({
        pageNo: 0,
        pageSize: 200
      })
      .subscribe(res => {
        this.storageUnitOptions = res.records.map(item => {
          return assign(item, {
            deviceType:
              item.deviceType === 'BasicDisk'
                ? DataMap.poolStorageDeviceType.Server.value
                : item.deviceType
          });
        });
        this.getCopies();
      });
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
      case WormStatusEnum.EXPIRATION:
        return 'common_active_expired_label';
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

  getResourceTypeMap() {
    const sourceMap = {
      [DataMap.Resource_Type.DWS_Cluster.value]: 'CopyData_DWS_Type',
      [DataMap.Resource_Type.MySQLClusterInstance.value]: 'copyDataMysqlType',
      [DataMap.Resource_Type.OpenGauss.value]: 'CopyDataOpengaussType',
      [DataMap.Resource_Type.ClickHouse.value]: 'clickHouseResourceType',
      [DataMap.Resource_Type.dbTwoDatabase.value]: 'copyDataDbTwoType',
      [DataMap.Resource_Type.OceanBase.value]: 'copyDataOceanBaseType',
      [DataMap.Resource_Type.AntDBInstance.value]: 'AntDB_Instance_Type'
    };
    let filterMap = [];
    if (
      !isEmpty(
        intersection(this.childResourceType, [
          DataMap.Resource_Type.ExchangeDataBase.value,
          DataMap.Resource_Type.tidb.value
        ])
      )
    ) {
      filterMap = this.dataMapService
        .toArray('Job_Target_Type')
        .filter(item => includes(this.childResourceType, item.value));
    } else {
      each(sourceMap, (value, key) => {
        if (includes(this.childResourceType, key)) {
          filterMap = this.dataMapService.toArray(value);
          this.subTypeTextMapKey = value;
          return false;
        }
      });
    }

    if (!isEmpty(filterMap)) {
      return filterMap;
    }

    this.subTypeTextMapKey = 'CopyData_SQL_Server_Type';
    return this.dataMapService.toArray('CopyData_SQL_Server_Type');
  }

  getColumns() {
    const getGeneratedTypeOpts = () => {
      const opts = this.dataMapService
        .toArray('CopyData_generatedType', [
          DataMap.CopyData_generatedType.backup.value,
          DataMap.CopyData_generatedType.Imported.value
        ])
        .filter(
          item =>
            filterGeneratedBy(item, this.childResourceType) &&
            item.value !== DataMap.CopyData_generatedType.import.value
        );

      if (
        this.resourceType === DataMap.Resource_Type.fileset.value ||
        !!size(
          intersection(
            [
              DataMap.Resource_Type.MySQLDatabase.value,
              DataMap.Resource_Type.tdsqlDistributedInstance.value
            ],
            this.childResourceType
          )
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
          this.childResourceType,
          DataMap.Resource_Type.ElasticsearchBackupSet.value
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_generatedType.download.value,
              DataMap.CopyData_generatedType.snapshot.value
            ],
            item.value
          )
        );
      }

      if (
        !!size(
          intersection(this.childResourceType, [
            DataMap.Resource_Type.KubernetesStatefulset.value,
            DataMap.Resource_Type.DWS_Cluster.value,
            DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
            DataMap.Resource_Type.informixInstance.value,
            DataMap.Resource_Type.informixClusterInstance.value,
            DataMap.Resource_Type.lightCloudGaussdbInstance.value,
            DataMap.Resource_Type.ClickHouse.value,
            DataMap.Resource_Type.SQLServerDatabase.value,
            DataMap.Resource_Type.generalDatabase.value,
            DataMap.Resource_Type.dbTwoTableSet.value,
            DataMap.Resource_Type.tidb.value,
            DataMap.Resource_Type.OceanBase.value,
            DataMap.Resource_Type.ActiveDirectory.value,
            DataMap.Resource_Type.ObjectSet.value,
            DataMap.Resource_Type.Exchange.value,
            DataMap.Resource_Type.goldendbInstance.value,
            DataMap.Resource_Type.saphanaDatabase.value,
            DataMap.Resource_Type.saponoracleDatabase.value
          ])
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
        !!size(
          intersection(this.childResourceType, [
            DataMap.Resource_Type.MongodbSingleInstance.value,
            DataMap.Resource_Type.MongodbClusterInstance.value
          ])
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
        .filter(item =>
          filterBackupType(item, this.childResourceType, this.i18n)
        );

      if (this.isResourceSet) {
        return reject(opts, item =>
          includes([DataMap.CopyData_Backup_Type.snapshot.value], item.value)
        );
      }

      if (this.resourceType === DataMap.Resource_Type.fileset.value) {
        if (
          includes(this.childResourceType, DataMap.Resource_Type.volume.value)
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
        includes(this.childResourceType, DataMap.Resource_Type.ClickHouse.value)
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.log.value,
              DataMap.CopyData_Backup_Type.diff.value,
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.incremental.value,
              DataMap.CopyData_Backup_Type.permanent.value
            ],
            item.value
          )
        );
      }
      // 全量 增量
      if (
        !!size(
          intersection(this.childResourceType, [
            DataMap.Resource_Type.ElasticsearchBackupSet.value
          ])
        )
      ) {
        return reject(opts, item => {
          return includes(
            [
              DataMap.CopyData_Backup_Type.log.value,
              DataMap.CopyData_Backup_Type.diff.value,
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.permanent.value
            ],
            item.value
          );
        });
      }

      // 全量 增量 差异
      if (
        includes(
          this.childResourceType,
          DataMap.Resource_Type.DWS_Cluster.value
        )
      ) {
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

      // 全量 永久增量
      if (
        includes(
          this.childResourceType,
          DataMap.Resource_Type.KubernetesStatefulset.value
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

      // 全量 日志 差异
      if (
        !!size(
          intersection(this.childResourceType, [
            DataMap.Resource_Type.SQLServerDatabase.value,
            DataMap.Resource_Type.gaussdbForOpengaussInstance.value
          ])
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
        !!size(
          intersection(this.childResourceType, [
            DataMap.Resource_Type.MySQLDatabase.value,
            DataMap.Resource_Type.generalDatabase.value,
            DataMap.Resource_Type.dbTwoDatabase.value,
            DataMap.Resource_Type.informixInstance.value,
            DataMap.Resource_Type.informixClusterInstance.value
          ])
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
      // 全量 增量 日志 永久增量
      if (
        !!size(
          intersection(this.childResourceType, [
            DataMap.Resource_Type.Exchange.value
          ])
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.diff.value
            ],
            item.value
          )
        );
      }
      // 全量 增量 日志
      if (
        !!size(
          intersection(this.childResourceType, [
            DataMap.Resource_Type.OceanBase.value,
            DataMap.Resource_Type.goldendbInstance.value
          ])
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.diff.value,
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.permanent.value
            ],
            item.value
          )
        );
      }
      // 全量 日志 永久增量
      if (
        !!size(
          intersection(this.childResourceType, [
            DataMap.Resource_Type.tdsqlInstance.value
          ])
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.diff.value,
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.incremental.value
            ],
            item.value
          )
        );
      }
      // 全量 日志
      if (
        !!size(
          intersection(this.childResourceType, [
            DataMap.Resource_Type.tidb.value,
            DataMap.Resource_Type.saponoracleDatabase.value,
            DataMap.Resource_Type.AntDBInstance.value,
            DataMap.Resource_Type.AntDBClusterInstance.value
          ])
        )
      ) {
        return reject(opts, item =>
          includes(
            [
              DataMap.CopyData_Backup_Type.diff.value,
              DataMap.CopyData_Backup_Type.snapshot.value,
              DataMap.CopyData_Backup_Type.permanent.value,
              DataMap.CopyData_Backup_Type.incremental.value
            ],
            item.value
          )
        );
      }

      // 全量 增量 永久增量
      if (
        !!size(
          intersection(this.childResourceType, [
            DataMap.Resource_Type.ObjectSet.value
          ])
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

      return opts;
    };

    this.columns = [
      {
        label: this.i18n.get('common_copies_label'),
        expanded: true,
        disabled: true,
        show: true,
        children: [
          {
            key: 'uuid',
            label: 'ID',
            show: false,
            isLeaf: true
          },
          {
            key: 'origin_backup_id',
            label: this.i18n.get('common_backup_copy_label'),
            show: false,
            isLeaf: true,
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
              return (
                this.isOceanProtect &&
                !this.appUtilsService.isDistributed &&
                !this.appUtilsService.isDecouple
              );
            }
          },
          {
            key: 'name',
            label: this.i18n.get('common_name_label'),
            disabled: false,
            show: false,
            isLeaf: true
          },
          {
            key: 'userName',
            label: this.i18n.get('common_owned_user_label'),
            disabled: false,
            show: false,
            isLeaf: true
          },
          {
            key: 'origin_copy_time_stamp',
            label: this.i18n.get('common_time_origin_copy_generate_label'),
            showSort: true,
            show: true,
            isLeaf: true,
            width: '180px',
            displayCheck: () => {
              return this.isOceanProtect;
            }
          },
          {
            key: 'timestamp',
            label: this.i18n.get('common_copy_standard_time_label'),
            show: false,
            isLeaf: true
          },
          {
            key: 'generation',
            label: this.i18n.get('explore_generation_label'),
            show: false,
            isLeaf: true
          },
          {
            key: 'status',
            label: this.i18n.get('common_status_label'),
            filter: true,
            filterMap: getCopyStatusFilterOptions(
              this.dataMapService.toArray('copydata_validStatus', [
                DataMap.copydata_validStatus.normal.value,
                DataMap.copydata_validStatus.invalid.value,
                DataMap.copydata_validStatus.deleting.value
              ]),
              this.childResourceType
            ),
            show: true,
            isLeaf: true
          },
          {
            key: 'location',
            label: this.i18n.get('common_location_label'),
            show: true,
            isLeaf: true,
            width: '110px'
          },
          {
            key: 'storage_unit_status',
            label: this.i18n.get('common_storage_media_status_label'),
            show: true,
            isLeaf: true,
            filter: true,
            filterMap: this.dataMapService.toArray('storageUnitStatus')
          },
          {
            key: 'generated_by',
            filter: true,
            label: this.i18n.get('common_generated_type_label'),
            filterMap: this.filterGeneratedOps(getGeneratedTypeOpts()),
            show: true,
            isLeaf: true
          },
          {
            key: 'display_timestamp',
            label: this.i18n.get('common_time_copy_generate_label'),
            showSort: true,
            show: true,
            isLeaf: true,
            width: '180px'
          },
          {
            key: 'backup_type',
            filter: true,
            label: this.i18n.get('common_copy_type_label'),
            filterMap: getBackupTypeOpts(),
            resourceType: map(
              reject(this.dataMapService.toArray('Resource_Type'), {
                value: DataMap.Resource_Type.ImportCopy.value
              }),
              'value'
            ),
            show: true,
            isLeaf: true
          },
          {
            key: 'expiration_time',
            width: '170px',
            label: this.i18n.get('common_copy_expriration_time_label'),
            show: true,
            isLeaf: true
          },
          {
            key: 'browse_mounted',
            label: this.i18n.get('common_exploration_mount_status_label'),
            displayCheck: () => this.isVirtualizationAndCloud,
            filter: true,
            filterMap: this.dataMapService.toArray('Browse_LiveMount_Status'),
            isLeaf: true,
            show: this.isVirtualizationAndCloud
          },
          {
            key: 'extend_type',
            label: this.i18n.get('common_backup_data_label'),
            displayCheck: () => {
              return includes(
                this.childResourceType,
                DataMap.Resource_Type.ActiveDirectory.value
              );
            },
            filter: true,
            filterMap: this.dataMapService.toArray('objectBackupLevel'),
            isLeaf: true,
            show: true
          },
          {
            key: 'storage_snapshot_flag',
            label: this.i18n.get('protection_snapshot_backup_copy_label'),
            filter: true,
            filterMap: this.dataMapService.toArray('isBusinessOptions'),
            displayCheck: () => {
              return (
                !!size(
                  intersection(this.childResourceType, [
                    DataMap.Resource_Type.oracle.value,
                    DataMap.Resource_Type.oracleCluster.value
                  ])
                ) && !this.isHcsUser
              );
            },
            isLeaf: true,
            show: true
          },
          {
            key: 'can_table_restore',
            label: this.i18n.get('protection_copy_contains_table_info_label'),
            displayCheck: () => {
              return (
                !!size(
                  intersection(this.childResourceType, [
                    DataMap.Resource_Type.oracle.value,
                    DataMap.Resource_Type.oracleCluster.value
                  ])
                ) && !this.isHcsUser
              );
            },
            isLeaf: true,
            show: true
          },
          {
            key: 'copy_format',
            label: this.i18n.get('protection_copy_format_label'),
            resourceType: [],
            show: true,
            isLeaf: true
          },
          {
            key: 'worm_status',
            label: this.i18n.get('explore_worm_th_label'),
            resourceType: map(
              this.dataMapService.toArray('Resource_Type'),
              'value'
            ),
            filter: true,
            filterMap: this.dataMapService.toArray('copyDataWormStatus'),
            show: !this.appUtilsService.isDistributed,
            hidden: this.appUtilsService.isDistributed,
            isLeaf: true
          },
          {
            key: 'worm_expiration_time',
            label: this.i18n.get('common_worm_expiration_time_label'),
            show: !this.appUtilsService.isDistributed,
            hidden: this.appUtilsService.isDistributed,
            isLeaf: true
          },
          {
            key: 'copy_verify_status',
            label: this.i18n.get('common_copy_verify_status_label'),
            resourceType: [
              DataMap.Resource_Type.KubernetesStatefulset.value,
              DataMap.Resource_Type.HCSCloudHost.value,
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.FusionCompute.value,
              DataMap.Resource_Type.FusionComputeVM.value,
              DataMap.Resource_Type.fusionOne.value,
              DataMap.Resource_Type.cNwareVm.value,
              DataMap.Resource_Type.APSCloudServer.value,
              DataMap.Resource_Type.nutanixVm.value
            ],
            show: true,
            isLeaf: true
          },
          {
            key: 'indexed',
            label: this.i18n.get('common_index_label'),
            resourceType: [
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.volume.value,
              DataMap.Resource_Type.virtualMachine.value,
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.ndmp.value,
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.LocalFileSystem.value,
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.FusionCompute.value,
              DataMap.Resource_Type.fusionOne.value,
              DataMap.Resource_Type.HCSCloudHost.value,
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.ObjectSet.value,
              DataMap.Resource_Type.APSCloudServer.value,
              DataMap.Resource_Type.cNwareVm.value,
              DataMap.Resource_Type.hyperVVm.value,
              DataMap.Resource_Type.nutanixVm.value,
              DataMap.Resource_Type.tdsqlInstance.value,
              DataMap.Resource_Type.tdsqlDistributedInstance.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.MongodbClusterInstance.value,
              DataMap.Resource_Type.MongodbSingleInstance.value
            ],
            filter: true,
            filterMap: this.dataMapService.toArray('CopyData_fileIndex'),
            show: true,
            isLeaf: true
          },
          {
            key: 'isSanClient',
            label: this.i18n.get('explore_sanclient_copy_label'),
            resourceType: [
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.dbTwoTableSet.value,
              DataMap.Resource_Type.informixInstance.value,
              DataMap.Resource_Type.informixClusterInstance.value
            ],
            filter: false,
            filterMap: this.dataMapService.toArray('copyDataSanclient'),
            show: false,
            isLeaf: true
          },
          {
            key: 'system_backup_flag',
            label: this.i18n.get('protection_volume_advanced_backup_label'),
            resourceType: [
              DataMap.Resource_Type.volume.value,
              DataMap.Resource_Type.fileset.value
            ],
            filter: false,
            filterMap: this.dataMapService.toArray('copyDataVolume'),
            show: true,
            isLeaf: true
          },
          {
            key: 'isBackupAcl',
            label: this.i18n.get('explore_acl_backup_label'),
            resourceType: [DataMap.Resource_Type.ObjectSet.value],
            filter: false,
            filterMap: this.dataMapService.toArray('aclType'),
            show: true,
            isLeaf: true
          },
          {
            key: 'canRestore',
            label: this.i18n.get('explore_is_copy_complete_label'),
            resourceType: [
              DataMap.Resource_Type.lightCloudGaussdbInstance.value
            ],
            filter: false,
            show: true,
            isLeaf: true
          }
        ]
      },
      {
        label: this.i18n.get('common_resource_label'),
        expanded: true,
        disabled: true,
        show: true,
        children: [
          {
            key: 'resource_name',
            label: this.i18n.get('common_name_label'),
            show: true,
            disabled: true,
            isLeaf: true,
            width: '130px'
          },
          {
            key: 'resource_sub_type',
            label: this.i18n.get('common_type_label'),
            filter: true,
            show: true,
            disabled: true,
            isLeaf: true,
            width: '130px',
            filterMap: this.getResourceTypeMap(),
            resourceType: [
              DataMap.Resource_Type.AntDBInstance.value,
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.OpenGauss.value,
              DataMap.Resource_Type.DWS_Database.value,
              DataMap.Resource_Type.SQLServerDatabase.value,
              DataMap.Resource_Type.MySQLDatabase.value,
              DataMap.Resource_Type.ClickHouseCluster.value,
              DataMap.Resource_Type.ClickHouseDatabase.value,
              DataMap.Resource_Type.ClickHouseTableset.value,
              DataMap.Resource_Type.OceanBaseCluster.value,
              DataMap.Resource_Type.OceanBaseTenant.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.tidbTable.value,
              DataMap.Resource_Type.ExchangeDataBase.value
            ]
          },
          {
            key: 'resource_script',
            label: this.i18n.get('protection_database_type_label'),
            show: true,
            isLeaf: true,
            width: '130px',
            resourceType: [DataMap.Resource_Type.generalDatabase.value]
          },
          {
            key: 'resource_location',
            label: this.i18n.get('common_location_label'),
            show: !size(
              intersection(this.childResourceType, [
                this.dataMap.Resource_Type.Redis.value
              ])
            ),
            isLeaf: true,
            width: '130px'
          },
          {
            key: 'resource_status',
            filter: true,
            label: this.i18n.get('common_status_label'),
            show: true,
            isLeaf: true,
            filterMap: this.dataMapService.toArray('Resource_Status')
          },
          {
            key: 'version',
            label: this.i18n.get('protection_database_version_label'),
            show: true,
            isLeaf: true,
            resourceType: [DataMap.Resource_Type.oracle.value]
          },
          {
            key: 'labelList',
            label: this.i18n.get('common_tag_label'),
            show: true,
            disabled: true,
            isLeaf: true,
            width: '130px'
          }
        ]
      }
    ];
    const deployType = this.i18n.get('deploy_type');
    if (
      [
        DataMap.Deploy_Type.cloudbackup2.value,
        DataMap.Deploy_Type.hyperdetect.value,
        DataMap.Deploy_Type.cloudbackup.value
      ].includes(deployType)
    ) {
      this.columns[0].children = this.columns[0].children.filter(
        item =>
          !['worm_status', 'worm_expiration_time', 'labelList'].includes(
            item.key
          )
      );
    }
    if (this.isResourceSet) {
      this.columns[1].children.unshift({
        key: 'resource_set_type',
        label: this.i18n.get('common_resource_type_label'),
        filter: true,
        filterMap: this.dataMapService.toArray('Job_Target_Type').filter(
          item =>
            !includes(
              EXCLUDE_RESOURCE_TYPES.map(item => item.value),
              item.value
            )
        ),
        show: true,
        disabled: true,
        isLeaf: true,
        width: '130px'
      });
    }
    // 如果配置项中写了displayCheck，就用对应的bool值 columns是二维数组
    each(this.columns, child => {
      child.children = child.children.filter(data =>
        isFunction(data.displayCheck) ? data.displayCheck() : true
      );
    });
    each(this.columns, column => {
      if (!column.children) {
        return;
      }
      column.children = filter(column.children, children => {
        if (
          !children.resourceType ||
          (children.key === 'backup_type' && this.isResourceSet)
        ) {
          return true;
        }
        if (
          children.key === 'indexed' &&
          (this.isHyperdetect || this.isHcsUser)
        ) {
          return false;
        }

        return !!size(
          intersection(this.childResourceType, children.resourceType)
        );
      });
    });
  }

  filterGeneratedOps(opts) {
    if (this.isHcsUser) {
      opts = reject(opts, item =>
        includes(
          [
            DataMap.CopyData_generatedType.cloudArchival.value,
            DataMap.CopyData_generatedType.tapeArchival.value,
            DataMap.CopyData_generatedType.cascadedReplication.value,
            DataMap.CopyData_generatedType.reverseReplication.value
          ],
          item.value
        )
      );
    }

    if (this.appUtilsService.isDistributed) {
      opts = reject(opts, item =>
        includes(
          [
            DataMap.CopyData_generatedType.tapeArchival.value,
            DataMap.CopyData_generatedType.cascadedReplication.value,
            DataMap.CopyData_generatedType.reverseReplication.value
          ],
          item.value
        )
      );
    }
    return opts;
  }

  initColumnSelection() {
    each(this.columns, column => {
      assign(column, {
        class: 'aui-th-deliver'
      });
      this.columnSelection.push(column);
      if (column.children) {
        this.columnSelection.push(...column.children);
        each(column.children, col => {
          assign(col, {
            class: includes(
              [
                'uuid',
                'cluster_name',
                'name',
                'display_timestamp',
                'generation',
                'status',
                'location',
                'generated_by',
                'backup_type',
                'copy_format',
                'indexed',
                'resource_name',
                'worm_status'
              ],
              col.key
            )
              ? 'timestamp-padding'
              : 'aui-th-deliver'
          });
        });
      }
    });
    this.columnSelection = reject(
      this.columnSelection,
      item => item.isLeaf && !item.show
    );
  }

  columnCheck(source) {
    if (!source.node.children) {
      source.node.show = !source.node.show;
      source.node.parent.show = !!size(
        filter(source.node.parent.children, item => {
          return item.show;
        })
      );
    } else {
      if (
        find(source.node.children, { show: true }) &&
        find(source.node.children, { show: false })
      ) {
        source.node.show = true;
      } else {
        source.node.show = !source.node.show;
      }
      each(source.node.children, item => {
        item.show = source.node.show;
      });
    }
  }

  getCopies(refreshData?) {
    this.selection = [];
    this.onSelectionChange.emit(this.selection);
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }
    let manualRefresh = true;
    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL_RESOURCE)
      .pipe(
        switchMap(index => {
          manualRefresh = !index;
          const params = this.getParams();
          return this.copiesApiService.queryResourcesV1CopiesGet({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.tableData = map(res.items, item => {
          this._getCopyVerifyStatus(item);
          this.assignNewAttribute(item);
          // 获取标签数据
          const { showList, hoverList } = getLabelList(
            JSON.parse(item.properties || '{}')
          );
          assign(item, {
            resource_script: get(
              JSON.parse(item.resource_properties || '{}'),
              'extendInfo.databaseTypeDisplay'
            ),
            labelList: get(JSON.parse(item.properties || '{}'), 'labelList'),
            showLabelList: showList,
            hoverLabelList: hoverList
          });
          return item;
        });
        this.total = res.total;

        if (this.isResourceSet) {
          this.onNumChange.emit(res.total);
        }

        if (
          manualRefresh &&
          refreshData &&
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'explore-copy-list'
          ) &&
          find(res.items, { uuid: refreshData.uuid })
        ) {
          this.getCopyDetail(find(res.items, { uuid: refreshData.uuid }));
        }
        this.cdr.detectChanges();

        this.globalService.emitStore({
          action: 'emitRefreshApp',
          state: true
        });
      });
  }

  private _dispatchFilterParams() {
    if (this.childResourceType?.includes('ClickHouse')) {
      assign(this.filterParams, {
        resource_sub_type: this.childResourceType,
        resource_type: !isNil(this.clickHouseTypeFilter)
          ? this.clickHouseTypeFilter
          : void 0
      });
      return;
    }

    assign(this.filterParams, {
      resource_sub_type: !!size(get(this.filterParams, 'resource_sub_type'))
        ? get(this.filterParams, 'resource_sub_type')
        : this.childResourceType
    });
  }

  getParams() {
    this._dispatchFilterParams();
    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize
    };
    if (!isEmpty(this.orders)) {
      assign(params, {
        orders: this.orders
      });
    }
    if (this.isResourceSet) {
      // 资源集需要根据资源集ID获取
      if (!isEmpty(this.filterParams['resource_set_type'])) {
        assign(this.filterParams, {
          resource_sub_type: this.filterParams['resource_set_type']
        });
        delete this.filterParams['resource_set_type'];
      } else {
        delete this.filterParams['resource_set_type'];
        delete this.filterParams['resource_sub_type'];
      }
      assign(this.filterParams, {
        resource_set_id: this.data[0].uuid
      });
    }
    if (!isEmpty(this.filterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.filterParams)
      });
    }

    return params;
  }

  getVersion(item) {
    const properties = JSON.parse(item.resource_properties || '{}') || {};
    return properties?.version || '--';
  }

  getGeneration(item) {
    const properties = JSON.parse(item.properties) || {};
    return properties.replicate_count || 0;
  }

  pageChange(source) {
    this.pageIndex = source.pageIndex;
    this.pageSize = source.pageSize;
    this.getCopies();
  }

  filterChange(e) {
    if (
      this.childResourceType?.includes('ClickHouse') &&
      e.key === 'resource_sub_type'
    ) {
      this.clickHouseTypeFilter = e.value;
    }

    if (
      ([
        DataMap.Resource_Type.fusionComputeVirtualMachine.value,
        DataMap.Resource_Type.FusionCompute.value,
        DataMap.Resource_Type.fusionOne.value,
        DataMap.Resource_Type.HCS.value
      ].includes(this.resourceType) ||
        intersection(this.childResourceType, [
          DataMap.Resource_Type.hyperVVm.value,
          DataMap.Resource_Type.hyperVCluster.value,
          DataMap.Resource_Type.hyperVHost,
          DataMap.Resource_Type.hyperVScvmm,
          DataMap.Resource_Type.tdsqlInstance.value,
          DataMap.Resource_Type.openStackCloudServer.value
        ]).length > 0) &&
      e.key === 'backup_type'
    ) {
      e.value = e.value.map(item => (item === 5 ? 2 : item));
    }
    assign(this.filterParams, {
      [e.key === 'backup_type' ? 'source_copy_type' : e.key]: e.value
    });
    this.getCopies();
  }

  searchByCopyUuid(copyUuid) {
    assign(this.filterParams, {
      uuid: trim(copyUuid)
    });
    this.getCopies();
  }

  searchByBackupCopyUuid(backupCopyUuid) {
    assign(this.filterParams, {
      origin_backup_id: trim(backupCopyUuid)
    });
    this.getCopies();
  }

  searchByCopyClusterName(copyClusterName) {
    assign(this.filterParams, {
      cluster_name: trim(copyClusterName)
    });
    this.getCopies();
  }

  searchByCopyName(copyName) {
    assign(this.filterParams, {
      name: trim(copyName)
    });
    this.getCopies();
  }

  searchByCopyLocation(copyLocation) {
    assign(this.filterParams, {
      location: trim(copyLocation)
    });
    this.getCopies();
  }

  searchByResourceLocation(resourceLocation) {
    assign(this.filterParams, {
      resource_location: trim(resourceLocation)
    });
    this.getCopies();
  }

  searchByName(resourceName) {
    assign(this.filterParams, {
      resource_name: trim(resourceName)
    });
    this.getCopies();
  }

  searchBySla(slaName) {
    assign(this.filterParams, {
      sla_name: trim(slaName)
    });
    this.getCopies();
  }

  searchByLabel(label) {
    label = label.map(e => e.value);
    assign(this.filterParams, {
      labelList: label
    });
    this.getCopies();
  }

  sortChange(source) {
    this.orders = [];
    this.orders.push((source.direction === 'asc' ? '+' : '-') + source.key);
    this.getCopies();
  }

  selectionChange() {
    this.onSelectionChange.emit(this.selection);
  }

  private _isDirCopyData(data) {
    return (
      parseInt(JSON.parse(data.properties)?.format, 10) === 1 &&
      includes(
        [GenerationType.BY_REPLICATED, GenerationType.BY_CASCADED_REPLICATION],
        data.generated_by
      )
    );
  }

  modifyRetentionPolicy(data) {
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
                this.getCopies(data);
              },
              error: error => resolve(false)
            });
          });
        }
      })
    );
  }
  wormSet(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('common_worm_setting_label'),
        lvModalKey: 'worm-set',
        lvOkLoadingText: this.i18n.get('common_loading_label'),
        lvWidth: MODAL_COMMON.smallModal,
        lvContent: WormSetComponent,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as WormSetComponent;
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
            const content = modal.getContentComponent() as WormSetComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.getCopies(data);
              },
              error: error => resolve(false)
            });
          });
        }
      })
    );
  }
  getCopyDetail(item, activeFileTab = false) {
    if (this.isResourceSet) {
      return;
    }
    this.activeItem = item;
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'explore-copy-list',
        lvWidth: MODAL_COMMON.largeWidth,
        lvContent: CopyDataDetailComponent,
        lvComponentParams: {
          data: {
            ...item,
            optItems: this.getOptItems(item),
            name: this.datePipe.transform(
              item.display_timestamp,
              'yyyy-MM-dd HH:mm:ss'
            )
          },
          activeFileTab
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ],
        lvAfterClose: () => {
          // 关闭详情框，取消激活
          this.activeItem = {};
          this.cdr.detectChanges();
        }
      })
    );
  }

  getResourceDetail(item) {
    if (this.isResourceSet) {
      // 资源集没有跳转功能
      return;
    }
    this.detailService.openDetailModal(item.resource_sub_type, {
      lvHeader: item.resource_name,
      data: isString(item.resource_properties)
        ? JSON.parse(item.resource_properties)
        : item.resource_properties,
      formCopyDataList: true
    });
  }

  manualMount(item) {
    this.manualMountService.create({
      item,
      resType: this.resourceType,
      onOk: () => {
        this.getCopies();
      }
    });
  }

  fileLevelExplorationUnMount(item) {
    this.copyControllerService
      .CloseCopyGuestSystem({
        copyId: item.uuid
      })
      .subscribe({
        next: res => {
          this.getCopies();
        },
        error: err => {}
      });
  }

  optsCallback = data => {
    return this.getOptItems(data);
  };

  getOptItems(data) {
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
        return (
          get(properties, 'canRestore', true) === false ||
          get(properties, 'can_table_restore', true) === false
        );
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
        label: getCommonRestoreLabel(data, this.i18n),
        tips:
          includes(
            [
              DataMap.Resource_Type.oracle.value,
              DataMap.Resource_Type.lightCloudGaussdbInstance.value
            ],
            data.resource_sub_type
          ) && get(JSON.parse(data.properties), 'canRestore', true) === false
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
                DataMap.Resource_Type.nutanixVm.value
              ],
              data.resource_sub_type
            ) &&
              data.status === DataMap.copydata_validStatus.invalid.value)
          ) ||
          (includes(
            [DataMap.Resource_Type.lightCloudGaussdbInstance.value],
            data.resource_sub_type
          ) &&
            data.resource_status === DataMap.Resource_Status.notExist.value &&
            !includes(
              [
                DataMap.CopyData_generatedType.replicate.value,
                DataMap.CopyData_generatedType.cascadedReplication.value,
                DataMap.CopyData_generatedType.reverseReplication.value
              ],
              data.generated_by
            )) ||
          (includes(
            [
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.dbTwoTableSet.value,
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
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
          (includes(
            [
              DataMap.Resource_Type.lightCloudGaussdbInstance.value,
              DataMap.Resource_Type.oracle.value,
              DataMap.Resource_Type.oracleCluster.value
            ],
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
          this.resourceType === ResourceType.HOST ||
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
          ) ||
          (includes(
            [
              DataMap.Resource_Type.saphanaDatabase.value,
              DataMap.Resource_Type.saponoracleDatabase.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [
                DataMap.CopyData_Backup_Type.incremental.value,
                DataMap.CopyData_Backup_Type.diff.value
              ],
              data.source_copy_type
            )),
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          this.restoreService.restore({
            childResType: data.resource_sub_type,
            copyData:
              data.resource_sub_type ===
              DataMap.Resource_Type.HBaseBackupSet.value
                ? assign(data, {
                    environment_uuid: JSON.parse(data.resource_properties)
                      ?.environment_uuid
                  })
                : data,
            isMessageBox: !includes(
              [
                DataMap.Resource_Type.oracle.value,
                DataMap.Resource_Type.oracleCluster.value
              ],
              data.resource_sub_type
            ),
            restoreType: RestoreType.CommonRestore,
            onOk: () => {
              this.getCopies();
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
              this.getCopies();
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
              this.getCopies();
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
                  this.getCopies();
                });
            }
          });
        }
      },
      {
        id: 'objectLevelRestore',
        label: objectLevelRestoreLabel(data.resource_sub_type),
        hidden: isObjectLevelRestoreHidden(),
        disabled:
          isObjectLevelRestoreDisabled() ||
          !hasRecoveryPermission(data) ||
          data.storage_unit_status === DataMap.storageUnitStatus.offline.value,
        onClick: () => {
          this.restoreService.restore({
            childResType: data.resource_sub_type,
            copyData: data,
            restoreType: RestoreType.FileRestore,
            isMessageBox: true,
            onOk: () => {
              this.getCopies();
            }
          });
        }
      },
      {
        id: 'singleFileRestore',
        label: this.i18n.get('common_single_file_level_restore_label'),
        disabled:
          data.status !== DataMap.copydata_validStatus.normal.value ||
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
          !hasRecoveryPermission(data) ||
          isIncompleteOracleCopy(
            data,
            properties,
            resourceProperties?.sub_type
          ),
        hidden:
          resourceProperties?.environment_os_type ===
            DataMap.Os_Type.aix.value ||
          !!data?.storage_snapshot_flag ||
          !includes(
            [
              DataMap.Resource_Type.oracle.value,
              DataMap.Resource_Type.oracleCluster.value
            ],
            data.resource_sub_type
          ) ||
          (!data?.storage_snapshot_flag &&
            [
              DataMap.CopyData_generatedType.replicate.value,
              DataMap.CopyData_generatedType.cloudArchival.value,
              DataMap.CopyData_generatedType.tapeArchival.value,
              DataMap.CopyData_generatedType.cascadedReplication.value
            ].includes(data.generated_by)),
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          this.restoreService.singleFileRestore({
            header: this.i18n.get('common_single_file_level_restore_label'),
            childResType: data.resource_sub_type,
            copyData: assign(data, { singleFileRestore: true }),
            restoreType: RestoreType.FileRestore,
            onOk: () => {
              this.getCopies();
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
                DataMap.Resource_Type.hyperVHost.value,
                DataMap.Resource_Type.hyperVVm.value
              ],
              data.resource_sub_type
            ) &&
              data.status === DataMap.copydata_validStatus.invalid.value)
          ) ||
          disableValidCopyBtn(data, properties) ||
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
          !hasRecoveryPermission(data),
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
            DataMap.Resource_Type.hyperVVm.value,
            DataMap.Resource_Type.hyperVHost.value
          ],
          data.resource_sub_type
        ),
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          if (
            data.resource_sub_type ===
            DataMap.Resource_Type.virtualMachine.value
          ) {
            this.getCopyDetail(data);
            return;
          }
          this.restoreService.restore({
            childResType: data.resource_sub_type,
            copyData: assign(cloneDeep(data), { diskRestore: true }),
            isMessageBox: true,
            restoreType: RestoreType.FileRestore,
            onOk: () => {
              this.getCopies();
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
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
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
              DataMap.Resource_Type.hyperVVm.value,
              DataMap.Resource_Type.nutanixVm.value
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
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
          (data.status === DataMap.copydata_validStatus.invalid.value &&
            properties?.isMemberDeleted === 'true') ||
          !hasRecoveryPermission(data),
        hidden: isSchemaLevelRestoreHideen(),
        permission: OperateItems.SchemaLevelRestore,
        onClick: () =>
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
              this.getCopies();
            },
            restoreLevel: 'schema'
          })
      },
      {
        id: 'fileLevelRestore',
        label: includes(
          [
            DataMap.Resource_Type.HBaseBackupSet.value,
            DataMap.Resource_Type.HiveBackupSet.value,
            DataMap.Resource_Type.DWS_Cluster.value,
            DataMap.Resource_Type.DWS_Database.value,
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
              [DataMap.Resource_Type.ElasticsearchBackupSet.value],
              data.resource_sub_type
            )
          ? this.i18n.get('explore_index_level_restore_label')
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
              [DataMap.Resource_Type.ObjectSet.value],
              data.resource_sub_type
            )
          ? this.i18n.get('common_object_level_restore_label')
          : this.i18n.get('common_file_level_restore_label'),
        disabled:
          !hasRecoveryPermission(data) ||
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
          (includes(
            [
              DataMap.Resource_Type.LocalFileSystem.value,
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.ndmp.value
            ],
            data.resource_sub_type
          )
            ? !(
                data.status === DataMap.copydata_validStatus.normal.value &&
                includes(
                  [DataMap.CopyData_fileIndex.indexed.value],
                  data.indexed
                )
              )
            : data.status !== DataMap.copydata_validStatus.normal.value ||
              (includes(
                [
                  DataMap.Resource_Type.NASShare.value,
                  DataMap.Resource_Type.NASFileSystem.value
                ],
                data.resource_sub_type
              ) &&
                includes(
                  [
                    DataMap.CopyData_generatedType.cloudArchival.value,
                    DataMap.CopyData_generatedType.tapeArchival.value
                  ],
                  data.generated_by
                ) &&
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
                properties?.isMemberDeleted === 'true') ||
              (includes(
                [
                  DataMap.Resource_Type.fileset.value,
                  DataMap.Resource_Type.ObjectStorage.value
                ],
                data.resource_sub_type
              ) &&
                DataMap.CopyData_generatedType.tapeArchival.value ===
                  data.generated_by &&
                data.indexed !== DataMap.CopyData_fileIndex.indexed.value)),
        hidden:
          !includes(
            [
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.volume.value,
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.LocalFileSystem.value,
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.HBaseBackupSet.value,
              DataMap.Resource_Type.HiveBackupSet.value,
              DataMap.Resource_Type.ElasticsearchBackupSet.value,
              DataMap.Resource_Type.DWS_Cluster.value,
              DataMap.Resource_Type.DWS_Database.value,
              DataMap.Resource_Type.DWS_Schema.value,
              DataMap.Resource_Type.DWS_Table.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerGroup.value,
              DataMap.Resource_Type.Dameng_singleNode.value,
              DataMap.Resource_Type.FusionCompute.value,
              DataMap.Resource_Type.fusionOne.value,
              DataMap.Resource_Type.HCSCloudHost.value,
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.OceanBaseCluster.value,
              DataMap.Resource_Type.ObjectSet.value,
              DataMap.Resource_Type.APSCloudServer.value,
              DataMap.Resource_Type.cNwareVm.value,
              DataMap.Resource_Type.hyperVVm.value,
              DataMap.Resource_Type.ndmp.value,
              DataMap.Resource_Type.virtualMachine.value,
              DataMap.Resource_Type.nutanixVm.value
            ],
            data.resource_sub_type
          ) ||
          (includes(
            [
              DataMap.Resource_Type.Dameng_singleNode.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.OceanBaseCluster.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerGroup.value // 屏蔽SQL server日志备份的数据库级恢复选项
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
          (includes(
            [
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.virtualMachine.value
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
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.tidbTable.value,
              DataMap.Resource_Type.OceanBaseCluster.value,
              DataMap.Resource_Type.ObjectSet.value,
              DataMap.Resource_Type.virtualMachine.value,
              DataMap.Resource_Type.APSCloudServer.value,
              DataMap.Resource_Type.FusionCompute.value,
              DataMap.Resource_Type.fusionOne.value,
              DataMap.Resource_Type.hyperVVm.value,
              DataMap.Resource_Type.cNwareVm.value
            ],
            data.resource_sub_type
          ) &&
            includes(
              [DataMap.CopyData_generatedType.cloudArchival.value],
              data.generated_by
            )) ||
          hiddenDwsFileLevelRestore(data) ||
          (includes(
            [DataMap.Resource_Type.DWS_Cluster.value],
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
            [DataMap.Resource_Type.DWS_Cluster.value],
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
          (includes(
            [DataMap.Resource_Type.openStackCloudServer.value],
            data.resource_sub_type
          ) &&
            !includes(
              [
                DataMap.CopyData_generatedType.replicate.value,
                DataMap.CopyData_generatedType.backup.value
              ],
              data.generated_by
            )) ||
          ([DataMap.Resource_Type.volume.value].includes(
            data.resource_sub_type
          ) &&
            includes(
              [
                DataMap.CopyData_generatedType.cascadedReplication.value,
                DataMap.CopyData_generatedType.reverseReplication.value
              ],
              data.generated_by
            )) ||
          hideE6000Func(this.appUtilsService, data, false),
        permission: OperateItems.FileLevelRestore,
        onClick: () => {
          // 说白了，vmware文件级恢复是打开副本详情
          if (
            data.resource_sub_type ===
            DataMap.Resource_Type.virtualMachine.value
          ) {
            this.getCopyDetail(data, true);
            return;
          }
          this.restoreService.fileLevelRestore({
            header: includes(
              [
                DataMap.Resource_Type.HBaseBackupSet.value,
                DataMap.Resource_Type.HiveBackupSet.value,
                DataMap.Resource_Type.DWS_Cluster.value,
                DataMap.Resource_Type.DWS_Database.value,
                DataMap.Resource_Type.DWS_Schema.value,
                DataMap.Resource_Type.DWS_Table.value,
                DataMap.Resource_Type.SQLServerDatabase.value,
                DataMap.Resource_Type.tidbDatabase.value,
                DataMap.Resource_Type.tidbCluster.value,
                DataMap.Resource_Type.OceanBaseCluster.value,
                DataMap.Resource_Type.OceanBaseTenant.value
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
                  [DataMap.Resource_Type.ObjectSet.value],
                  data.resource_sub_type
                )
              ? this.i18n.get('common_object_level_restore_label')
              : this.i18n.get('common_file_level_restore_label'),
            childResType: data.resource_sub_type,
            copyData:
              data.resource_sub_type ===
              DataMap.Resource_Type.HBaseBackupSet.value
                ? assign(data, {
                    environment_uuid: JSON.parse(data.resource_properties)
                      ?.environment_uuid
                  })
                : data,
            restoreType: RestoreType.FileRestore,
            onOk: () => {
              this.getCopies();
            }
          });
        }
      },
      {
        id: 'instantRestore',
        label: this.i18n.get('common_live_restore_job_label'),
        disabled:
          !(
            data.status === DataMap.copydata_validStatus.normal.value ||
            (includes(
              [DataMap.Resource_Type.cNwareVm.value],
              data.resource_sub_type
            ) &&
              data.status === DataMap.copydata_validStatus.invalid.value)
          ) ||
          !hasLivemountPermission(data) ||
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
          isIncompleteOracleCopy(
            data,
            properties,
            resourceProperties?.sub_type
          ),
        hidden:
          includes(
            [
              DataMap.Resource_Type.Dameng_singleNode.value,
              DataMap.Resource_Type.Dameng_cluster.value,
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.ImportCopy.value,
              DataMap.Resource_Type.LocalFileSystem.value,
              DataMap.Resource_Type.Redis.value,
              DataMap.Resource_Type.HCSCloudHost.value,
              DataMap.Resource_Type.FusionCompute.value,
              DataMap.Resource_Type.fusionOne.value,
              DataMap.Resource_Type.PostgreSQLInstance.value,
              DataMap.Resource_Type.PostgreSQLClusterInstance.value,
              DataMap.Resource_Type.KingBaseInstance.value,
              DataMap.Resource_Type.KingBaseClusterInstance.value,
              DataMap.Resource_Type.ElasticsearchBackupSet.value,
              DataMap.Resource_Type.KubernetesStatefulset.value,
              DataMap.Resource_Type.ClickHouse.value,
              DataMap.Resource_Type.HiveBackupSet.value,
              DataMap.Resource_Type.DWS_Cluster.value,
              DataMap.Resource_Type.DWS_Database.value,
              DataMap.Resource_Type.DWS_Schema.value,
              DataMap.Resource_Type.DWS_Table.value,
              DataMap.Resource_Type.OpenGauss_database.value,
              DataMap.Resource_Type.OpenGauss_instance.value,
              DataMap.Resource_Type.GaussDB_T.value,
              DataMap.Resource_Type.gaussdbTSingle.value,
              DataMap.Resource_Type.generalDatabase.value,
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.dbTwoTableSet.value,
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.goldendbInstance.value,
              DataMap.Resource_Type.informixInstance.value,
              DataMap.Resource_Type.informixClusterInstance.value,
              DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
              DataMap.Resource_Type.MongodbClusterInstance.value,
              DataMap.Resource_Type.MongodbSingleInstance.value,
              DataMap.Resource_Type.lightCloudGaussdbInstance.value,
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
              DataMap.Resource_Type.saponoracleDatabase.value,
              DataMap.Resource_Type.ObjectSet.value,
              DataMap.Resource_Type.ndmp.value,
              DataMap.Resource_Type.nutanixVm.value,
              DataMap.Resource_Type.AntDBInstance.value,
              DataMap.Resource_Type.AntDBClusterInstance.value,
              DataMap.Resource_Type.hyperVVm.value
            ],
            data.resource_sub_type
          ) ||
          this.appUtilsService.isDistributed ||
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
              DataMap.Resource_Type.OceanBaseTenant.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.tidbTable.value
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
              DataMap.Resource_Type.volume.value,
              DataMap.Resource_Type.APSCloudServer.value
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
          isHideOracleInstanceRestore(data, resourceProperties, properties),
        permission: OperateItems.InstanceRecovery,
        onClick: () => {
          this.restoreService.restore({
            childResType: data.resource_sub_type,
            isMessageBox: includes(
              [DataMap.Resource_Type.cNwareVm.value],
              data.resource_sub_type
            ),
            copyData: data,
            restoreType: RestoreType.InstanceRestore,
            onOk: () => {
              this.getCopies();
            }
          });
        }
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
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
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
          !hasLivemountPermission(data) ||
          isIncompleteOracleCopy(
            data,
            properties,
            resourceProperties?.sub_type
          ),
        hidden:
          includes(
            [
              DataMap.Resource_Type.Dameng_cluster.value,
              DataMap.Resource_Type.Dameng_singleNode.value,
              DataMap.Resource_Type.ImportCopy.value,
              DataMap.Resource_Type.LocalFileSystem.value,
              DataMap.Resource_Type.Redis.value,
              DataMap.Resource_Type.HCSCloudHost.value,
              DataMap.Resource_Type.FusionCompute.value,
              DataMap.Resource_Type.fusionOne.value,
              DataMap.Resource_Type.PostgreSQLInstance.value,
              DataMap.Resource_Type.PostgreSQLClusterInstance.value,
              DataMap.Resource_Type.KingBaseInstance.value,
              DataMap.Resource_Type.KingBaseClusterInstance.value,
              DataMap.Resource_Type.ElasticsearchBackupSet.value,
              DataMap.Resource_Type.ClickHouse.value,
              DataMap.Resource_Type.KubernetesStatefulset.value,
              DataMap.Resource_Type.HiveBackupSet.value,
              DataMap.Resource_Type.DWS_Cluster.value,
              DataMap.Resource_Type.DWS_Database.value,
              DataMap.Resource_Type.DWS_Schema.value,
              DataMap.Resource_Type.DWS_Table.value,
              DataMap.Resource_Type.OpenGauss_database.value,
              DataMap.Resource_Type.OpenGauss_instance.value,
              DataMap.Resource_Type.GaussDB_T.value,
              DataMap.Resource_Type.gaussdbTSingle.value,
              DataMap.Resource_Type.generalDatabase.value,
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.dbTwoTableSet.value,
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.goldendbInstance.value,
              DataMap.Resource_Type.informixInstance.value,
              DataMap.Resource_Type.informixClusterInstance.value,
              DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
              DataMap.Resource_Type.MongodbClusterInstance.value,
              DataMap.Resource_Type.MongodbSingleInstance.value,
              DataMap.Resource_Type.lightCloudGaussdbInstance.value,
              DataMap.Resource_Type.OceanBaseCluster.value,
              DataMap.Resource_Type.OceanBaseTenant.value,
              DataMap.Resource_Type.kubernetesDatasetCommon.value,
              DataMap.Resource_Type.kubernetesNamespaceCommon.value,
              DataMap.Resource_Type.tdsqlDistributedInstance.value,
              DataMap.Resource_Type.ActiveDirectory.value,
              DataMap.Resource_Type.commonShare.value,
              DataMap.Resource_Type.saphanaDatabase.value,
              DataMap.Resource_Type.saponoracleDatabase.value,
              DataMap.Resource_Type.ObjectSet.value,
              DataMap.Resource_Type.ndmp.value,
              DataMap.Resource_Type.nutanixVm.value,
              DataMap.Resource_Type.AntDBClusterInstance.value,
              DataMap.Resource_Type.AntDBInstance.value,
              DataMap.Resource_Type.hyperVVm.value
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
              DataMap.Resource_Type.SQLServerCluster.value,
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerDatabase.value,
              DataMap.Resource_Type.SQLServerGroup.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.tidbTable.value,
              DataMap.Resource_Type.APSCloudServer.value
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
            [DataMap.Resource_Type.MySQLInstance.value],
            data.resource_sub_type
          ) &&
            includes(
              [DataMap.CopyData_generatedType.liveMount.value],
              data.generated_by
            )) ||
          (includes(
            [
              DataMap.Resource_Type.tdsqlInstance.value,
              DataMap.Resource_Type.OceanBaseCluster.value,
              DataMap.Resource_Type.OceanBaseTenant.value,
              DataMap.Resource_Type.MySQLDatabase.value,
              DataMap.Resource_Type.MySQLClusterInstance.value
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
            )) ||
          (this.appUtilsService.isDistributed &&
            includes(
              [
                DataMap.Resource_Type.fileset.value,
                DataMap.Resource_Type.NASShare.value,
                DataMap.Resource_Type.NASFileSystem.value,
                DataMap.Resource_Type.ndmp.value,
                DataMap.Resource_Type.volume.value
              ],
              data.resource_sub_type
            )) ||
          hideE6000Func(this.appUtilsService, data, true) ||
          isHideOracleMount(resourceProperties) ||
          this.hideBasicDiskFuction(data, this.appUtilsService.isDistributed),
        permission: OperateItems.MountingCopy,
        onClick: () => this.manualMount(data)
      },
      {
        id: 'copy',
        disabled:
          +data.generation === 3 ||
          !includes([DataMap.copydata_validStatus.normal.value], data.status) ||
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
          !hasReplicationPermission(data),
        tips:
          +data.generation === 3
            ? this.i18n.get('protection_copy_disable_tip_label')
            : '',
        label: this.i18n.get('common_replicate_label'),
        permission: OperateItems.CopyDuplicate,
        hidden:
          (this.appUtilsService.isDistributed &&
            includes(
              [DataMap.CopyData_generatedType.replicate.value],
              data.generated_by
            )) ||
          !includes(
            [
              DataMap.CopyData_generatedType.cascadedReplication.value,
              DataMap.CopyData_generatedType.replicate.value
            ],
            data.generated_by
          ) ||
          includes(
            [DataMap.Resource_Type.oraclePDB.value],
            data.resource_sub_type
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
            this.getCopies();
          })
      },
      {
        id: 'archive',
        label: this.i18n.get('common_manual_archive_label'),
        disabled:
          !hasArchivePermission(data) ||
          data.storage_unit_status === DataMap.storageUnitStatus.offline.value,
        hidden:
          !this.isOceanProtect ||
          !includes(
            [
              DataMap.CopyData_generatedType.replicate.value,
              DataMap.CopyData_generatedType.cascadedReplication.value,
              DataMap.CopyData_generatedType.backup.value,
              DataMap.CopyData_generatedType.Imported.value
            ],
            data.generated_by
          ) ||
          (includes(
            [
              DataMap.CopyData_generatedType.replicate.value,
              DataMap.CopyData_generatedType.cascadedReplication.value
            ],
            data.generated_by
          ) &&
            includes(
              [DataMap.Resource_Type.ActiveDirectory.value],
              data.resource_sub_type
            )) ||
          includes(
            [DataMap.Resource_Type.oraclePDB.value],
            data.resource_sub_type
          ) ||
          includes(
            [
              DataMap.copydata_validStatus.deleteFailed.value,
              DataMap.copydata_validStatus.deleting.value,
              DataMap.copydata_validStatus.invalid.value
            ],
            data.status
          ) ||
          this.validManualArchive(data) ||
          this.hideBasicDiskFuction(
            data,
            this.appUtilsService.isDecouple ||
              this.appUtilsService.isDistributed
          ) ||
          this.isHcsUser,
        onClick: () =>
          this.takeManualArchiveService.manualArchive(data, () => {
            this.getCopies();
          })
      },
      {
        id: 'modifyRetentionPolicy',
        disabled:
          !includes(
            [
              DataMap.copydata_validStatus.normal.value,
              DataMap.copydata_validStatus.deleteFailed.value
            ],
            data.status
          ) ||
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
          (data.status === DataMap.copydata_validStatus.invalid.value &&
            properties?.isMemberDeleted === 'true'),
        label: this.i18n.get('common_modify_retention_policy_label'),
        permission: OperateItems.ModifyingCopyRetentionPolicy,
        hidden:
          (includes(
            [DataMap.Resource_Type.ImportCopy.value],
            data.resource_sub_type
          ) &&
            data.generated_by ===
              DataMap.CopyData_generatedType.download.value) ||
          data.generated_by ===
            DataMap.CopyData_generatedType.tapeArchival.value,
        onClick: () => {
          this.modifyRetentionPolicy(data);
        }
      },
      {
        id: 'wormSet',
        disabled:
          !includes(
            [
              DataMap.copydata_validStatus.normal.value,
              DataMap.copydata_validStatus.deleteFailed.value
            ],
            data.status
          ) ||
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
          (data.status === DataMap.copydata_validStatus.invalid.value &&
            properties?.isMemberDeleted === 'true'),
        label: this.i18n.get('common_worm_setting_label'),
        permission: OperateItems.WormSet,
        hidden: isHideWorm(data, this.i18n),
        onClick: () => {
          this.wormSet(data);
        }
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
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
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
                this.getCopies();
              }
            }
          });
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeletingCopy,
        divide: includes(
          [
            DataMap.Resource_Type.NASShare.value,
            DataMap.Resource_Type.NASFileSystem.value,
            DataMap.Resource_Type.HDFSFileset.value
          ],
          data.resource_sub_type
        ),
        disabled:
          !includes(
            [
              DataMap.copydata_validStatus.normal.value,
              DataMap.copydata_validStatus.invalid.value,
              DataMap.copydata_validStatus.deleteFailed.value
            ],
            data.status
          ) || !hasCopyDeletePermission(data),
        hidden:
          (data.resource_sub_type === DataMap.Resource_Type.ImportCopy.value &&
            DataMap.CopyData_generatedType.import.value ===
              data.generated_by) ||
          data.backup_type === DataMap.CopyData_Backup_Type.log.value ||
          data.generated_by ===
            DataMap.CopyData_generatedType.tapeArchival.value,
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
            rowData: data,
            actionId: OperateItems.DeletingCopy,
            content: this.i18n.get('common_copy_delete_label', [
              this.datePipe.transform(
                data.display_timestamp,
                'yyyy-MM-dd HH:mm:ss'
              )
            ]),
            onOK: modal => {
              this.copiesApiService
                .deleteCopyV1CopiesCopyIdDelete({
                  copyId: data.uuid,
                  isForced: get(
                    modal,
                    'contentInstance.forciblyDeleteCopy',
                    null
                  )
                })
                .subscribe(res => this.getCopies());
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
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
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
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.NASShare.value,
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
          (!includes(
            [
              DataMap.CopyData_fileIndex.unIndexed.value,
              DataMap.CopyData_fileIndex.deletedFailed.value
            ],
            data.indexed
          ) &&
            !isIndexedFilesetCLoudArchival(data)) ||
          data.storage_unit_status ===
            DataMap.storageUnitStatus.offline.value ||
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
              DataMap.Resource_Type.virtualMachine.value,
              DataMap.Resource_Type.nutanixVm.value
            ],
            data.resource_sub_type
          ) ||
          includes(
            [
              DataMap.CopyData_generatedType.tapeArchival.value,
              DataMap.CopyData_generatedType.import.value,
              DataMap.CopyData_generatedType.liveMount.value
            ],
            data.generated_by
          ) ||
          (includes(
            [DataMap.CopyData_generatedType.cloudArchival.value],
            data.generated_by
          ) &&
            !includes(
              [DataMap.Resource_Type.fileset.value],
              data.resource_sub_type
            )) ||
          (includes(
            [
              DataMap.CopyData_generatedType.cascadedReplication.value,
              DataMap.CopyData_generatedType.reverseReplication.value
            ],
            data.generated_by
          ) &&
            !includes(
              [DataMap.Resource_Type.virtualMachine.value],
              data.resource_sub_type
            )) ||
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
                DataMap.Resource_Type.hyperVVm.value,
                DataMap.Resource_Type.virtualMachine.value,
                DataMap.Resource_Type.ndmp.value,
                DataMap.Resource_Type.nutanixVm.value,
                DataMap.Resource_Type.volume.value
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
          this.isHcsUser ||
          hideE6000Func(this.appUtilsService, data, false),
        onClick: () => {
          this.copiesApiService
            .createCopyIndexV1CopiesCopyIdActionCreateIndexPost({
              copyId: data.uuid
            })
            .subscribe(res => this.getCopies());
        }
      }
    ];
    this.appendAdditionalOptsItems(menus, data);
    return getPermissionMenuItem(menus, this.cookieService.role);
  }

  private hideBasicDiskFuction(data: any, isCorrectDeployType): boolean {
    return (
      isCorrectDeployType &&
      find(this.storageUnitOptions, {
        name: data.storage_unit_name
      })?.deviceType === DataMap.poolStorageDeviceType.Server.value
    );
  }

  appendAdditionalOptsItems(menus: any[], data) {
    if (this.isVirtualizationAndCloud) {
      const index = findIndex(menus, { id: 'fileLevelRestore' });
      if (index === -1) {
        return;
      }
      const menu = menus[index];
      const newMenuItem = {
        id: 'fileExploreUnMount',
        label: this.i18n.get('common_file_exploration_unmount_label'),
        disabled:
          menu.disabled ||
          !includes(
            [
              DataMap.Browse_LiveMount_Status.mounted.value,
              DataMap.Browse_LiveMount_Status.unmountFail.value
            ],
            data?.browse_mounted
          ),
        hidden: !this.isVirtualizationAndCloud || menu.hidden,
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          this.fileLevelExplorationUnMount(data);
        }
      };
      menus.splice(index + 1, 0, newMenuItem);
    }
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

  validManualArchive(data) {
    const copyFormat = get(JSON.parse(data?.properties), 'format');
    if (
      includes(
        [DataMap.CopyData_Backup_Type.full.value],
        data.source_copy_type
      ) ||
      isUndefined(copyFormat)
    ) {
      return false;
    }

    if ([0, 2].includes(copyFormat)) {
      return false;
    }

    return true;
  }

  getSystemBackupStatus(item) {
    const resourcePro = JSON.parse(item.resource_properties || '{}');
    if (!resourcePro) {
      return '';
    }

    if (this.childResourceType.includes(DataMap.Resource_Type.fileset.value)) {
      return resourcePro?.extendInfo?.is_OS_backup === 'true' || false;
    }

    if (resourcePro?.environment_os_type === DataMap.Os_Type.windows.value) {
      return (
        Number(resourcePro?.extendInfo?.system_backup_type) ===
        DataMap.windowsVolumeBackupType.bareMetal.value
      );
    }

    return resourcePro?.ext_parameters?.system_backup_flag;
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

  assignNewAttribute(item) {
    if (this.isVirtualizationAndCloud && isEmpty(item?.browse_mounted)) {
      item.browse_mounted = DataMap.Browse_LiveMount_Status.unmount.value;
    }
    if (
      !!size(
        intersection(this.childResourceType, [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value
        ])
      )
    ) {
      const properties = JSON.parse(item?.properties || '{}');
      if (item.backup_type === DataMap.CopyData_Backup_Type.log.value) {
        item.can_table_restore = ''; // 老副本日志副本的表级恢复信息不完整;
        return;
      }
      item.can_table_restore = isUndefined(properties?.can_table_restore)
        ? ''
        : properties.can_table_restore;
    }
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

  isActive(item): boolean {
    return item.uuid === this.activeItem?.uuid;
  }
}
