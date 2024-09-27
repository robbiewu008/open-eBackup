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
import {
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CAPACITY_UNIT,
  ClustersApiService,
  ColorConsts,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  getAccessibleViewList,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService
} from 'app/shared';
import { StoragePoolService } from 'app/shared/api/services/storage-pool.service';
import { StorageUnitService } from 'app/shared/api/services/storage-unit.service';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, cloneDeep, isEmpty, isUndefined, map, size } from 'lodash';
import { CreateStorageUnitComponent } from '../create-storage-unit/create-storage-unit.component';
import { BackupStorageUnitDetailComponent } from './backup-storage-unit-detail/backup-storage-unit-detail.component';

@Component({
  selector: 'aui-backup-storage-unit',
  templateUrl: './backup-storage-unit.component.html',
  styleUrls: ['./backup-storage-unit.component.less']
})
export class BackupStorageUnitComponent implements OnInit, AfterViewInit {
  dataMap = DataMap;
  unitOptsConfig;
  unitItemOptConfig;
  unitTableConfig: TableConfig;
  unitTableData: TableData;
  activeIndex: string = 'unitGroup';
  selectionData = [];
  backupUnitStatus: any = this.dataMapService.toArray(
    this.appUtilsService.isDistributed
      ? 'DistributedStoragePoolRunningStatus'
      : 'StoragePoolRunningStatus'
  );
  progressBarColor = [[0, ColorConsts.NORMAL]];
  unitconst = CAPACITY_UNIT;
  lessThanLabel = this.i18n.get('common_less_than_label');
  health_Status = this.dataMapService.toArray('HealthStatus');
  isDistributed = this.appUtilsService.isDistributed;
  isDecouple = this.appUtilsService.isDecouple;

  poolStorageFilter = this.dataMapService
    .toArray('poolStorageDeviceType')
    .filter(
      item =>
        item.value !==
        (this.isDistributed
          ? DataMap.poolStorageDeviceType.OceanProtectX.value
          : DataMap.poolStorageDeviceType.OceanPacific.value)
    )
    .map(item => {
      return assign(item, {
        value:
          item.value !== DataMap.poolStorageDeviceType.Server.value
            ? item.value
            : 'BasicDisk'
      });
    });

  protected readonly Math = Math;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('healthStatusTpl', { static: true }) healthStatusTpl: TemplateRef<
    any
  >;
  @ViewChild('runningStatusTpl', { static: true })
  runningStatusTpl: TemplateRef<any>;
  @ViewChild('capacityTpl', { static: true }) capacityTpl: TemplateRef<any>;
  @ViewChild('thresholdTpl', { static: true }) thresholdTpl: TemplateRef<any>;
  @ViewChild('deviceTypeTPl', { static: true }) deviceTypeTPl: TemplateRef<any>;
  timeSub$: any;
  constructor(
    private i18n: I18NService,
    private virtualScroll: VirtualScrollService,
    private drawModalService: DrawModalService,
    private cdr: ChangeDetectorRef,
    private warningMessageService: WarningMessageService,
    private clusterApiService: ClustersApiService,
    private batchOperateService: BatchOperateService,
    private storageUnitService: StorageUnitService,
    private cookieService: CookieService,
    private storagePoolService: StoragePoolService,
    public dataMapService: DataMapService,
    public appUtilsService?: AppUtilsService
  ) {}

  ngOnInit(): void {
    this.initUnitConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    if (!!size(filters.orders)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }
    const { conditions } = filters;
    if (!isEmpty(conditions)) {
      const {
        name,
        healthStatus,
        runningStatus,
        deviceName,
        poolName,
        deviceType
      } = JSON.parse(conditions);
      assign(params, {
        name,
        healthStatusList: healthStatus,
        runningStatusList: runningStatus,
        deviceName,
        poolName,
        deviceType
      });
    }
    this.storageUnitService.queryBackUnitGET(params).subscribe(res => {
      const newArr = [];
      res.records.map(item => {
        // 由于设备类型中服务器我们本地的数据值和下发的不一样，所以做个转化
        newArr.push(
          Object.assign(item, {
            disabled: !!item.isAutoAdded,
            deviceType:
              item.deviceType === 'BasicDisk'
                ? DataMap.poolStorageDeviceType.Server.value
                : item.deviceType
          })
        );
      });
      this.unitTableData = {
        data: newArr,
        total: res.totalCount
      };
      this.cdr.detectChanges();
    });
  }

  getDetail(item) {
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'distributed-nas-list',
        lvHeader: item.name,
        lvWidth: MODAL_COMMON.xLargeWidth,
        lvContent: BackupStorageUnitDetailComponent,
        lvComponentParams: {
          data: item
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => {
              modal.close();
              this.dataTable.fetchData();
            }
          }
        ]
      })
    );
  }

  initUnitConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_create_label'),
        displayCheck: () => {
          return getAccessibleViewList(this.cookieService.role)[
            OperateItems.AddBackupStorageUnit
          ];
        },
        onClick: () => {
          this.createUnit();
        }
      },
      {
        id: 'delete',
        permission: OperateItems.DeleteBackupStorageUnit,
        label: this.i18n.get('common_delete_label'),
        disableCheck: () => {
          return !size(this.selectionData);
        },
        onClick: () => {
          this.deleteUnit('multiple');
        }
      }
    ];
    this.unitOptsConfig = getPermissionMenuItem(opts);

    const itemOpts: ProButton[] = [
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        onClick: data => {
          this.editUnit(data[0]);
        },
        displayCheck: data => {
          return data[0].isAutoAdded === false;
        },
        permission: OperateItems.ModifyBackupStorageUnit
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        onClick: data => {
          this.deleteUnit('single', data);
        },
        permission: OperateItems.DeleteBackupStorageUnit,
        displayCheck: data => {
          return data[0].isAutoAdded === false;
        }
      }
    ];
    this.unitItemOptConfig = getPermissionMenuItem(itemOpts);

    const cols: TableCols[] = [
      {
        key: 'id',
        name: 'id',
        hidden: true
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: data => {
              this.getDetail(data);
            }
          }
        }
      },
      {
        key: 'healthStatus',
        name: this.i18n.get('common_health_status_label'),
        hidden: this.appUtilsService.isDistributed,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.health_Status
        },
        cellRender: this.healthStatusTpl
      },
      {
        key: 'runningStatus',
        name: this.i18n.get('protection_running_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.backupUnitStatus
        },
        cellRender: this.runningStatusTpl
      },
      {
        key: 'deviceType',
        name: this.i18n.get('common_equipment_type_label'),
        width: 150,
        filter:
          this.isDecouple || this.isDistributed
            ? {
                type: 'select',
                isMultiple: true,
                showCheckAll: true,
                options: this.poolStorageFilter
              }
            : null,
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('poolStorageDeviceType')
        }
      },
      {
        key: 'deviceName',
        name: this.i18n.get('protection_storage_device_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'poolName',
        name: this.i18n.get('common_storage_pool_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'capacity',
        name: this.i18n.get('common_capacity_label'),
        width: 200,
        cellRender: this.capacityTpl
      },
      {
        key: 'threshold',
        name: this.i18n.get('common_alarm_threshold_label'),
        cellRender: this.thresholdTpl
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        width: 144,
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: this.unitItemOptConfig
          }
        }
      }
    ];

    this.unitTableConfig = {
      table: {
        compareWith: 'id',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        scroll: { x: '100%' },
        autoPolling: CommonConsts.TIME_INTERVAL,
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.id;
        }
      }
    };
  }

  onChange() {
    this.ngOnInit();
  }

  createUnit() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('common_create_label'),
        lvContent: CreateStorageUnitComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateStorageUnitComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvComponentParams: {
          isEdit: false
        },
        lvOk: modal => {
          return new Promise(resolve => {
            this.dealCreateUnit(modal, resolve);
          });
        }
      }
    });
  }

  dealCreateUnit(modal: any, resolve: (value: unknown) => void) {
    const content = modal.getContentComponent() as CreateStorageUnitComponent;
    content.saveUnitInfo().subscribe({
      next: () => {
        resolve(true);
        const filter = {
          paginator: {
            pageIndex: 0,
            pageSize: 20
          }
        };
        const args = {
          akLoading: false
        };

        this.getData(filter, args);
      },
      error: err => {
        resolve(false);
      }
    });
  }

  editUnit(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('common_modify_label'),
        lvContent: CreateStorageUnitComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateStorageUnitComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvComponentParams: {
          isEdit: true,
          drawData: data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            this.dealEditUnit(modal, resolve);
          });
        }
      }
    });
  }

  dealEditUnit(modal: any, resolve: (value: unknown) => void) {
    const content = modal.getContentComponent() as CreateStorageUnitComponent;
    content.saveUnitInfo().subscribe({
      next: () => {
        resolve(true);
        this.dataTable.fetchData();
      },
      error: err => {
        resolve(false);
      }
    });
  }

  deleteUnit(type, data?) {
    let nameArr = [];
    let selectedArr = [];
    if (type === 'multiple') {
      nameArr = [map(this.selectionData, item => item.name).join(',')];
      selectedArr = this.selectionData;
    } else {
      nameArr = [data[0].name];
      selectedArr = data;
    }

    this.warningMessageService.create({
      content: this.i18n.get('system_backup_storage_delete_label', nameArr),
      onOK: () => {
        if (size(selectedArr) === 1) {
          this.storageUnitService
            .deleteBackupUnitDELETEResponse({
              id: selectedArr[0].id
            })
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            });
          return;
        }
        this.batchOperateService.selfGetResults(
          item => {
            return this.storageUnitService.deleteBackupUnitDELETEResponse({
              id: item.id,
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            });
          },
          map(cloneDeep(this.selectionData), item => {
            return assign(item, {
              name: item.name,
              isAsyn: false
            });
          }),
          () => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          }
        );
      }
    });
  }
}
