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
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  ClustersApiService,
  ColorConsts,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  getAccessibleViewList,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService
} from 'app/shared';
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
import { assign, cloneDeep, includes, isUndefined, map, size } from 'lodash';
import { AddBackupNodeComponent } from '../../cluster-management/add-backup-node/add-backup-node.component';
import { CreateStorageDeviceComponent } from '../create-storage-device/create-storage-device.component';

@Component({
  selector: 'aui-backup-storage-device',
  templateUrl: './backup-storage-device.component.html',
  styleUrls: ['./backup-storage-device.component.less']
})
export class BackupStorageDeviceComponent implements OnInit {
  dataMap = DataMap;
  unitOptsConfig;
  unitTableConfig: TableConfig;
  unitTableData: TableData;
  activeIndex: string = 'unitGroup';
  selectionData = [];
  backupUnitStatus: any = this.dataMapService.toArray('Cluster_Status');
  isDistributed = this.appUtilsService.isDistributed;
  isDecouple = this.appUtilsService.isDecouple;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('statusTpl', { static: true }) statusTpl: TemplateRef<any>;
  @ViewChild('clusterTpl', { static: true }) clusterTpl: TemplateRef<any>;
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
    private cookieService: CookieService,
    public dataMapService: DataMapService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {
    this.backupUnitStatus.push({
      value: 25,
      key: 'upgrating',
      label: this.i18n.get('system_upgrating_label'),
      color: ColorConsts.RUNNING
    });
    this.backupUnitStatus = this.backupUnitStatus.filter(
      item =>
        item.value !== DataMap.Cluster_Status.unknown.value &&
        item.value !== DataMap.Cluster_Status.partOffline.value
    );
    this.initUnitConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  getData(filters: Filters, args) {
    const params = {
      startPage: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      roleList: [3],
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    const { conditions } = filters;
    if (conditions) {
      const {
        clusterName,
        status,
        generatedType,
        clusterIp,
        port,
        deviceType
      } = JSON.parse(conditions);
      assign(params, {
        clusterName,
        statusList: status,
        generatedType,
        clusterIp,
        port,
        deviceType
      });
    }

    this.clusterApiService.getClustersInfoUsingGET(params).subscribe(res => {
      const newArr = [];
      res.records.forEach(item => {
        if (item.generatedType === 2 || item.status === 25) {
          newArr.push(Object.assign(item, { disabled: true }));
        } else {
          newArr.push(Object.assign(item, { disabled: false }));
        }

        if (item.deviceType === DataMap.poolStorageDeviceType.Server.value) {
          item.port = '';
        }
      });
      this.unitTableData = {
        data: newArr || [],
        total: res.totalCount
      };
      this.cdr.detectChanges();
    });
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
        label: this.i18n.get('common_delete_label'),
        displayCheck: () => {
          return getAccessibleViewList(this.cookieService.role)[
            OperateItems.DeleteBackupStorageUnit
          ];
        },
        disableCheck: () => {
          return !size(this.selectionData);
        },
        onClick: () => {
          this.deleteUnit('multiple');
        }
      }
    ];
    this.unitOptsConfig = opts;

    const cols: TableCols[] = [
      {
        key: 'clusterId',
        name: 'clusterId',
        hidden: true
      },
      {
        key: 'clusterName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.backupUnitStatus
        },
        cellRender: this.statusTpl
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
                options: this.dataMapService
                  .toArray('poolStorageDeviceType')
                  .filter(
                    item =>
                      item.value !==
                      (this.isDistributed
                        ? DataMap.poolStorageDeviceType.OceanProtectX.value
                        : DataMap.poolStorageDeviceType.OceanPacific.value)
                  )
              }
            : null,
        cellRender: this.deviceTypeTPl
      },
      {
        key: 'generatedType',
        name: this.i18n.get('system_backup_local_cluster_label'),
        hidden: true,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: [
            {
              key: 'no',
              value: 1,
              label: this.i18n.get('common_no_label')
            },
            {
              key: 'yes',
              value: 2,
              label: this.i18n.get('common_yes_label')
            }
          ]
        },
        cellRender: this.clusterTpl
      },
      {
        key: 'clusterIp',
        name: 'IP',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        width: 144,
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'modify',
                label: this.i18n.get('common_modify_label'),
                onClick: data => {
                  this.editUnit(data[0]);
                },
                displayCheck: data => {
                  if (
                    data[0].deviceType ===
                    DataMap.poolStorageDeviceType.OceanPacific.value
                  ) {
                    return false;
                  }
                  if (
                    getAccessibleViewList(this.cookieService.role)[
                      OperateItems.ModifyBackupStorageUnit
                    ]
                  ) {
                    return includes([1, 2], data[0].generatedType);
                  }
                  return false;
                },
                disableCheck: data => {
                  return data[0].status === DataMap.Node_Status.upgrating.value;
                }
              },
              {
                id: 'delete',
                label: this.i18n.get('common_delete_label'),
                onClick: data => {
                  this.deleteUnit('single', data);
                },
                displayCheck: data => {
                  if (
                    getAccessibleViewList(this.cookieService.role)[
                      OperateItems.ModifyBackupStorageUnit
                    ]
                  ) {
                    return data[0].generatedType === 1;
                  }
                  return false;
                },
                disableCheck: data => {
                  return data[0].status === DataMap.Node_Status.upgrating.value;
                }
              },
              {
                id: 'upgrating',
                label: this.i18n.get(
                  'system_upgrate_to_backup_member_node_label'
                ),
                onClick: data => {
                  this.upgratingUnit(data[0]);
                },
                displayCheck: data => {
                  // 有升级权限，且是手动生成的，且设备类型是ProtectX系列，才可以升级
                  if (
                    getAccessibleViewList(this.cookieService.role)[
                      OperateItems.upgrateBackupStorageUnit
                    ] &&
                    data[0].generatedType === 1 &&
                    data[0].deviceType ===
                      this.dataMap.poolStorageDeviceType.OceanProtectX.value &&
                    !this.appUtilsService.isDecouple
                  ) {
                    return true;
                  }
                  return false;
                },
                disableCheck: data => {
                  return (
                    data[0].status === DataMap.Node_Status.upgrating.value ||
                    data[0].status === DataMap.Node_Status.offline.value
                  );
                }
              }
            ]
          }
        }
      }
    ];

    this.unitTableConfig = {
      table: {
        compareWith: 'clusterId',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        autoPolling: CommonConsts.TIME_INTERVAL,
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.storageEsn;
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
        lvContent: CreateStorageDeviceComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateStorageDeviceComponent;
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
            this.dealCreatUnit(modal, resolve);
          });
        }
      }
    });
  }

  dealCreatUnit(modal: any, resolve: (value: unknown) => void) {
    const content = modal.getContentComponent() as CreateStorageDeviceComponent;
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
        lvContent: CreateStorageDeviceComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateStorageDeviceComponent;
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
    const content = modal.getContentComponent() as CreateStorageDeviceComponent;
    content.saveUnitInfo().subscribe({
      next: () => {
        this.dataTable.fetchData();
        resolve(true);
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
      nameArr = [map(this.selectionData, item => item.clusterName).join(',')];
      selectedArr = this.selectionData;
    } else {
      nameArr = [data[0].clusterName];
      selectedArr = data;
    }

    this.warningMessageService.create({
      content: this.i18n.get(
        'system_backup_storage_device_delete_label',
        nameArr
      ),
      onOK: () => {
        if (size(selectedArr) === 1) {
          this.clusterApiService
            .deleteTargetClusterUsingDELETE({
              clusterId: selectedArr[0].clusterId
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
            return this.clusterApiService.deleteTargetClusterUsingDELETE({
              clusterId: item.clusterId,
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            });
          },
          map(cloneDeep(this.selectionData), item => {
            return assign(item, {
              name: item.clusterName,
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

  upgratingUnit(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'addTargetCluster',
        lvHeader: this.i18n.get('system_upgrate_to_backup_member_node_label'),
        lvWidth: MODAL_COMMON.normalWidth,
        lvContent: AddBackupNodeComponent,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddBackupNodeComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvComponentParams: {
          unitData: data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddBackupNodeComponent;
            this.warningMessageService.create({
              content: this.i18n.get(
                'system_upgrating_backup_node_warning_label'
              ),
              width: 700,
              onOK: () => {
                const content = modal.getContentComponent() as AddBackupNodeComponent;
                content.upgrateBackupStorageUnit().subscribe(
                  () => {
                    resolve(true);
                    this.dataTable.fetchData();
                  },
                  () => {
                    resolve(false);
                  }
                );
              },
              onCancel: () => resolve(false),
              lvAfterClose: result => {
                if (result && result.trigger === 'close') {
                  resolve(false);
                }
              }
            });
          });
        }
      })
    );
  }
}
