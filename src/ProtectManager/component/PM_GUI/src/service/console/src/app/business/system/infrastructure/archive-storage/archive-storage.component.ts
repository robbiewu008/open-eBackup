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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  Pipe,
  PipeTransform,
  ViewChild
} from '@angular/core';
import { MenuItem } from '@iux/live';
import {
  BaseUtilService,
  CAPACITY_UNIT,
  ColorConsts,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService,
  CapacityCalculateLabel,
  BackupClustersApiService,
  GROUP_COMMON
} from 'app/shared';
import {
  ClustersApiService,
  StoragesApiService
} from 'app/shared/api/services';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  filter,
  forEach,
  isFunction,
  isNil,
  set,
  size,
  map as _map,
  get,
  includes,
  defer,
  first
} from 'lodash';
import { ArchiveStorageDetailComponent } from './archive-storage-detail/archive-storage-detail.component';
import { CreateArchiveStorageComponent } from './create-archive-storage/create-archive-storage.component';
import { ModifyAlarmThresholdComponent } from './modify-alarm-threshold/modify-alarm-threshold.component';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { map } from 'rxjs/operators';
import { StoragePoolListComponent } from './storage-pool-list/storage-pool-list.component';
import { StorageDeviceListComponent } from './storage-device-list/storage-device-list.component';
import { LinkStatusComponent } from 'app/shared/components/link-status/link-status.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Pipe({ name: 'selectionPipe' })
export class SelectionPipe implements PipeTransform {
  constructor(private dataMapService: DataMapService) {}
  transform(value: any[], exponent: string = 'type') {
    return filter(
      value,
      item =>
        item[exponent] !==
        this.dataMapService.getConfig('Archive_Storage_Type')['local']['value']
    );
  }
}

@Component({
  selector: 'aui-archive-storage',
  templateUrl: './archive-storage.component.html',
  styleUrls: ['./archive-storage.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [SelectionPipe, CapacityCalculateLabel]
})
export class ArchiveStorageComponent implements OnInit {
  dataMap = DataMap;
  selection = [];
  deviceData = [];
  scrollParam = { y: '0px' };
  selectedDevice;
  total = 0;
  tableData: any;
  activeIndex = 'object';
  tapeActiveIndex = 'storagePool';
  unitconst = CAPACITY_UNIT;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  disabledDelete = true;
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );
  showTapeTab =
    !this.appUtilsService.isDistributed && !this.appUtilsService.isDecouple;

  clusterMenus = [];
  nodeName;
  activeNode;
  curClusterName;

  groupCommon = GROUP_COMMON;

  nameLabel = this.i18n.get('common_name_label');
  typeLabel = this.i18n.get('common_type_label');
  statusLabel = this.i18n.get('common_status_label');
  optLabel = this.i18n.get('common_operation_label');
  capacityLabel = this.i18n.get('common_capacity_label');
  lessThanLabel = this.i18n.get('common_less_than_label');
  storageLabel = this.i18n.get('system_storage_management_label');
  deleteArchiveStorageLabel = this.i18n.get('common_delete_label');
  createArchiveStorageLabel = this.i18n.get(
    'common_create_archive_storage_label'
  );
  modifyArchiveStorageLabel = this.i18n.get(
    'system_modify_archive_storage_label'
  );
  importArchiveCopiesLabel = this.i18n.get(
    'system_import_archive_copies_label'
  );
  modifyAlarmThresholdLabel = this.i18n.get(
    'system_modify_alarm_threshold_label'
  );
  addBackupStorageLabel = this.i18n.get('system_add_backup_storage_label');
  modifyBackupStorageLabel = this.i18n.get(
    'system_modify_backup_storage_label'
  );
  importBackupStorageLabel = this.i18n.get(
    'system_import_backup_storage_copy_label'
  );

  @ViewChild(StoragePoolListComponent, { static: false })
  StoragePoolListComponent: StoragePoolListComponent;

  @ViewChild(StorageDeviceListComponent, { static: false })
  StorageDeviceListComponent: StorageDeviceListComponent;

  constructor(
    public appUtilsService: AppUtilsService,
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    public baseUtilService: BaseUtilService,
    public warningMessageService: WarningMessageService,
    public storageApiService: StoragesApiService,
    public dataMapService: DataMapService,
    public cookieService: CookieService,
    private BackupClustersApiService?: BackupClustersApiService,
    public selectionPipe?: SelectionPipe,
    private batchOperateService?: BatchOperateService,
    private infoMessageService?: InfoMessageService,
    private capacityCalculateLabel?: CapacityCalculateLabel,
    public virtualScroll?: VirtualScrollService,
    private cdr?: ChangeDetectorRef
  ) {}

  ngOnInit() {
    if (this.isDataBackup) {
      this.getClusterNodes();
    }
    this.getDevicesData();
    this.getArchiveStorages();
    this.virtualScroll.getScrollParam(360, 3);
    this.setClusterMenuHeight();
    window.addEventListener('resize', () => {
      this.virtualScroll.getScrollParam(360, 3);
      this.setClusterMenuHeight();
    });
  }

  setClusterMenuHeight() {
    defer(() => {
      const clusterMenu = first(
        document.getElementsByClassName('cluster-menus')
      );
      if (clusterMenu) {
        clusterMenu.setAttribute(
          'style',
          `max-height: ${parseInt(this.virtualScroll.scrollParam.y) + 30}px`
        );
      }
    });
  }

  tabChange(e) {
    if (e === 'tape') {
      this.virtualScroll.getScrollParam(300, 3);
      this.setClusterMenuHeight();
      window.addEventListener('resize', () => {
        this.virtualScroll.getScrollParam(300, 3);
        this.setClusterMenuHeight();
      });
    }
  }

  getClusterNodes(nodeName?) {
    const params = {};
    if (nodeName) {
      set(params, 'nodeName', nodeName || '');
    }

    this.BackupClustersApiService.queryAllMembers(params).subscribe(
      (res: any) => {
        // 排序规则：
        // 第一层:按照节点角色：主节点、备、成员节点
        // 第二层：按照节点状态：在线、设置中、离线、删除中
        const rule = [
          DataMap.Node_Status.online.value,
          DataMap.Node_Status.setting.value,
          DataMap.Node_Status.offline.value,
          DataMap.Node_Status.deleting.value
        ];
        res.sort((a, b) => {
          if (a.role !== b.role) {
            return b.role - a.role;
          } else {
            return rule.indexOf(a.status) - rule.indexOf(b.status);
          }
        });
        const nodes = _map(res, item => {
          return {
            ...item,
            id: item.remoteEsn,
            label: item.clusterName,
            disabled: item.status !== DataMap.Cluster_Status.online.value
          };
        });

        this.clusterMenus = nodes;
        this.activeNode = get(this.clusterMenus, '[0].id');
        this.curClusterName = get(this.clusterMenus, '[0].label');
        this.nodeChange({
          data: { remoteEsn: this.activeNode, clusterName: this.curClusterName }
        });
        this.cdr.detectChanges();
      }
    );
  }

  search() {
    this.getClusterNodes(this.nodeName);
  }

  refresh() {
    this.getClusterNodes(this.nodeName);
  }

  nodeChange(event) {
    if (this.StoragePoolListComponent) {
      this.StoragePoolListComponent.tableData = {
        data: [],
        total: 0
      };
      this.StoragePoolListComponent.node = event.data;
      this.StoragePoolListComponent.poolTable.fetchData();
    }
    if (this.StorageDeviceListComponent) {
      this.StorageDeviceListComponent.tableData = {
        data: [],
        total: 0
      };
      this.StorageDeviceListComponent.node = event.data;
      this.StorageDeviceListComponent.deviceTable.fetchData();
    }
  }

  onChange() {
    if (this.StoragePoolListComponent) {
      this.StoragePoolListComponent.tableData = {
        data: [],
        total: 0
      };
      this.StoragePoolListComponent.poolTable.fetchData();
    }
    if (this.StorageDeviceListComponent) {
      this.StorageDeviceListComponent.tableData = {
        data: [],
        total: 0
      };
      this.StorageDeviceListComponent.deviceTable.fetchData();
    }
    this.ngOnInit();
  }

  getArchiveStorages() {
    this.selection = [];
    this.disabledDelete = true;
    this.storageApiService
      .storageUsingGET({
        startPage: this.pageIndex,
        pageSize: this.pageSize
      })
      .pipe(
        map(result => {
          const res = result || ({ records: [], totalCount: 0 } as any);
          forEach(res.records, item => {
            item.progressBarColor = [[0, ColorConsts.NORMAL]];
            item.sizePercent = this.getSizePercent(item);
            item.alarmThreasholdPer = `${item.alarmThreashold * 100}%`;
            if (
              !isNil(item.alarmThreashold) &&
              item.sizePercent / 100 >= item.alarmThreashold
            ) {
              item.progressBarColor = [[0, ColorConsts.ABNORMAL]];
            }
          });
          this.cdr.detectChanges();
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = res.records || [];
        this.total = res.totalCount;
        this.cdr.detectChanges();
      });
  }

  getDevicesData() {
    this.deviceData = [
      { id: 0, type: 'Tape library', num: 0, icon: 'aui-icon-tape' }
    ];
  }

  storagePageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getArchiveStorages();
  }

  getStatusDetail(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: 700,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('common_status_detail_label'),
        lvContent: LinkStatusComponent,
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ],
        lvComponentParams: {
          data: data?.s3StorageStatusResponses,
          type: 'archive'
        }
      }
    });
  }

  createArchiveStorage(callback?: () => void) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: this.i18n.isEn ? 780 : 700,
        lvOkDisabled: true,
        lvHeader: this.cookieService.isCloudBackup
          ? this.addBackupStorageLabel
          : this.createArchiveStorageLabel,
        lvContent: CreateArchiveStorageComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateArchiveStorageComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            this.warningMessageService.create({
              content: this.i18n.get('system_add_storage_warn_desc_label', [
                modal.lvHeader
              ]),
              onOK: () => {
                const content = modal.getContentComponent() as CreateArchiveStorageComponent;
                content.create().subscribe(
                  res => {
                    resolve(true);
                    if (isFunction(callback)) {
                      callback();
                    } else {
                      this.getArchiveStorages();
                    }
                  },
                  () => resolve(false)
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
      }
    });
  }

  deleteArchiveStorage(datas) {
    if (datas.length === 1) {
      this.infoMessageService.create({
        content: this.i18n.get('system_delete_archive_storage_label'),
        onOK: () => {
          this.storageApiService
            .deleteStorageUsingDELETE1({
              storageId: datas[0].repositoryId
            })
            .subscribe(res => {
              this.getArchiveStorages();
            });
        }
      });
      return;
    }
    this.infoMessageService.create({
      content: this.i18n.get('system_delete_archive_storage_label'),
      onOK: () => {
        this.batchOperateService.selfGetResults(
          item => {
            return this.storageApiService.deleteStorageUsingDELETE1({
              storageId: item.repositoryId,
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            });
          },
          datas,
          () => {
            this.getArchiveStorages();
          }
        );
      }
    });
  }

  modifyArchiveStorage(data) {
    this.storageApiService
      .queryStorageByIdUsingGET({ storageId: data.repositoryId })
      .subscribe(res => {
        this.drawModalService.create({
          ...MODAL_COMMON.generateDrawerOptions(),
          ...{
            lvWidth: this.i18n.isEn ? 780 : 700,
            lvOkDisabled: true,
            lvHeader: this.cookieService.isCloudBackup
              ? this.modifyBackupStorageLabel
              : this.modifyArchiveStorageLabel,
            lvContent: CreateArchiveStorageComponent,
            lvOk: modal => {
              return new Promise(resolve => {
                const content = modal.getContentComponent() as CreateArchiveStorageComponent;
                content.modify().subscribe(
                  res => {
                    resolve(true);
                    this.getArchiveStorages();
                  },
                  error => resolve(false)
                );
              });
            },
            lvComponentParams: {
              data: assign({}, data, res)
            },
            lvAfterOpen: modal => {
              const content = modal.getContentComponent() as CreateArchiveStorageComponent;
              const modalIns = modal.getInstance();
              content.formGroup.statusChanges.subscribe(res => {
                modalIns.lvOkDisabled = res !== 'VALID';
              });
            }
          }
        });
      });
  }

  importArchiveCopies(data) {
    this.infoMessageService.create({
      content: this.i18n.get(
        this.i18n.get('deploy_type') ===
          DataMap.Deploy_Type.cloudbackup.value ||
          this.i18n.get('deploy_type') ===
            DataMap.Deploy_Type.cloudbackup2.value
          ? 'system_import_backup_import_warn_label'
          : 'system_import_archive_import_warn_label',
        [data.name, data.name]
      ),
      onOK: () => {
        this.storageApiService
          .importArchiveCopiesByStorageIdUsingPOST({
            storageId: data.repositoryId
          })
          .subscribe(res => {
            this.getArchiveStorages();
          });
      }
    });
  }

  checkAllChange(source) {}

  checkOneChange(source) {}

  getDetail(item) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: item.name,
        lvContent: ArchiveStorageDetailComponent,
        lvComponentParams: {
          rowItem: item
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: (modal, button) => {
              modal.close();
            }
          }
        ]
      }
    });
  }

  selectionChange(source) {
    this.disabledDelete = !size(this.selection);
  }

  stateChange(source: { [key: string]: any }) {}

  optsCallback: (data) => Array<MenuItem> = data => {
    const menus = [
      {
        id: 'importArchiveCopies',
        hidden: data.type === DataMap.Archive_Storage_Type.local.value,
        label: this.cookieService.isCloudBackup
          ? this.importBackupStorageLabel
          : this.importArchiveCopiesLabel,
        permission: OperateItems.ImportingArchiveStorageCopy,
        onClick: () => this.importArchiveCopies(data)
      },
      {
        id: 'modifyArchiveStorage',
        label: this.cookieService.isCloudBackup
          ? this.modifyBackupStorageLabel
          : this.modifyArchiveStorageLabel,
        permission: OperateItems.ModifyingArchiveStorage,
        hidden: data.type === DataMap.Archive_Storage_Type.local.value,
        onClick: () => this.modifyArchiveStorage(data)
      },
      {
        id: 'modifyAlarmThreshold',
        label: this.modifyAlarmThresholdLabel,
        permission: OperateItems.ModifyStorageAlarmThreshold,
        hidden: data.type !== DataMap.Archive_Storage_Type.local.value,
        onClick: () => {
          this.drawModalService.create({
            ...MODAL_COMMON.generateDrawerOptions(),
            ...{
              lvWidth: 700,
              lvOkDisabled: true,
              lvHeader: this.modifyAlarmThresholdLabel,
              lvContent: ModifyAlarmThresholdComponent,
              lvComponentParams: {
                data
              },
              lvOk: modal => {
                return new Promise(resolve => {
                  const content = modal.getContentComponent() as ModifyAlarmThresholdComponent;
                  content.onOK().subscribe({
                    next: () => {
                      resolve(true);
                      this.getArchiveStorages();
                    },
                    error: () => resolve(false)
                  });
                });
              }
            }
          });
        }
      },
      {
        id: 'deleteArchiveStorage',
        label: this.deleteArchiveStorageLabel,
        permission: OperateItems.DeletingArchiveStorage,
        hidden: data.type === DataMap.Archive_Storage_Type.local.value,
        onClick: () => this.deleteArchiveStorage([data])
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  getSizePercent(source): string {
    const sizePercent = parseFloat(
      (source.usedSize / source.totalSize) * 100 + ''
    );
    return this.capacityCalculateLabel.formatDecimalPoint(sizePercent, 3);
  }
}
