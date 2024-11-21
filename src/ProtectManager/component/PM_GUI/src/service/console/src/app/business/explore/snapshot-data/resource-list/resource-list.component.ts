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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  OnDestroy,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import { MessageboxService } from '@iux/live';
import {
  ApiStorageBackupPluginService,
  CAPACITY_UNIT,
  CommonConsts,
  CookieService,
  CopiesDetectReportService,
  DATE_PICKER_MODE,
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectedResourceApiService,
  autoTableScroll,
  extendSlaInfo,
  getPermissionMenuItem
} from 'app/shared';
import { ProtectedResourcePageListResponse } from 'app/shared/api/models';
import { CyberSnapshotDataComponent } from 'app/shared/components/cyber-snapshot-data/cyber-snapshot-data.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { CopyActionService } from 'app/shared/services/copy-action.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  defer,
  each,
  filter,
  find,
  first,
  includes,
  isArray,
  isEmpty,
  isNumber,
  isUndefined,
  last,
  map,
  reject,
  set,
  size,
  trim
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-ransomware-resource-list',
  templateUrl: './resource-list.component.html',
  styleUrls: ['./resource-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ResourceListComponent implements OnInit, OnDestroy, AfterViewInit {
  resourceName;
  storageDeviceName;
  tenantName;
  tableData: any = [];
  columns;
  columnSelection = [];
  filter = filter;
  _isNumber = isNumber;
  dataMap = DataMap;

  filterParams = {};
  orders = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  unitconst = CAPACITY_UNIT;
  optItems: any[];

  timeSub$: Subscription;
  destroy$ = new Subject();

  @Output() refreshResource = new EventEmitter();

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    private copyActionService: CopyActionService,
    private detailService: ResourceDetailService,
    private messageBox: MessageboxService,
    private copiesDetectReportService: CopiesDetectReportService,
    private apiStorageBackupPluginService: ApiStorageBackupPluginService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    defer(() => this.initColGroup());
  }

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit(): void {
    this.virtualScroll.getScrollParam(400);
    this.getColumns();
    this.initColumnSelection();
    this.initFilterMap();
    autoTableScroll(this.virtualScroll, 400, null, this.cdr);
    this.getResource();
  }

  initColGroup() {
    const table = first(document.getElementsByTagName('table'));
    const tableFirstNode = table.firstElementChild;
    if (tableFirstNode.nodeName === 'COLGROUP') {
      table.removeChild(tableFirstNode);
    }
    const theader = first(document.getElementsByTagName('thead'));
    const colGroup = document.createElement('colgroup');
    colGroup.innerHTML = last(
      document.getElementsByTagName('colgroup')
    ).innerHTML;
    if (table && theader && colGroup) {
      table.insertBefore(colGroup, theader);
    }
  }

  initFilterMap() {
    const cacheName = this.appUtilsService.getCacheValue('abnormalDeviceName');
    const resourceName = this.appUtilsService.getCacheValue(
      'resourceName',
      false
    );
    if (cacheName) {
      set(this.filterParams, 'status', [
        DataMap.detectionSnapshotStatus.infected.value
      ]);
      set(this.filterParams, 'device_name', [cacheName]);
      each(this.columns, item => {
        if (isArray(item.children)) {
          each(item.children, col => {
            if (col.key === 'status') {
              col.filterMap = filter(
                this.dataMapService.toArray('detectionSnapshotStatus'),
                val => {
                  if (
                    val.value === DataMap.detectionSnapshotStatus.infected.value
                  ) {
                    val.selected = true;
                  }
                  return true;
                }
              );
            }
          });
        }
      });
    }
    if (resourceName) {
      this.resourceName = resourceName;
      set(this.filterParams, 'resource_name', resourceName);
    }
    this.appUtilsService
      .getCyberEngineStorage()
      .subscribe((res: ProtectedResourcePageListResponse) => {
        each(this.columns[0].children, col => {
          if (col.key === 'device_name') {
            col.filterMap = map(res.records, item => {
              const device = {
                key: item.uuid,
                label: item.name,
                value: item.name
              };
              if (cacheName === item.name) {
                assign(device, {
                  selected: true
                });
              }
              return device;
            });
          }
        });
        this.columns = [...this.columns];
        this.cdr.detectChanges();
      });
  }

  getColumns() {
    this.columns = [
      {
        label: this.i18n.get('explore_resource_object_label'),
        expanded: true,
        disabled: true,
        show: true,
        children: [
          {
            key: 'resource_id',
            show: false,
            isLeaf: true,
            label: this.i18n.get('protection_resource_id_label')
          },
          {
            key: 'name',
            disabled: true,
            show: true,
            isLeaf: true,
            label: this.i18n.get('common_name_label'),
            width: '280px'
          },
          {
            key: 'device_name',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_belong_device_label'),
            filter: true,
            filterMap: []
          },
          {
            key: 'tenant_name',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_belong_tenant_label')
          },
          {
            key: 'total_copy_num',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_snapshot_count_label'),
            width: '120px'
          }
        ]
      },
      {
        label: this.i18n.get('explore_latest_detection_result_label'),
        expanded: true,
        disabled: true,
        show: true,
        children: [
          {
            key: 'latest_snapshot_time',
            show: true,
            isLeaf: true,
            disabled: true,
            label: this.i18n.get('explore_snapshot_create_time_label'),
            width: '180px'
          },
          {
            key: 'status',
            label: this.i18n.get('explore_safe_status_label'),
            filter: true,
            show: true,
            isLeaf: true,
            filterMap: this.dataMapService.toArray('detectionSnapshotStatus')
          },
          {
            key: 'generate_type',
            show: true,
            isLeaf: true,
            label: this.i18n.get('common_generated_type_label'),
            filter: true,
            filterMap: this.dataMapService.toArray('snapshotGeneratetype')
          },
          {
            key: 'total_file_size',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_total_file_size_label')
          },
          {
            key: 'added_file_count',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_new_file_num_label')
          },
          {
            key: 'changed_file_count',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_modify_file_count_label')
          },
          {
            key: 'deleted_file_count',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_delete_file_count_label')
          },
          {
            key: 'infected_file_count',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_suspicious_file_num_label')
          },
          {
            key: 'latest_detection_time',
            show: false,
            isLeaf: true,
            label: this.i18n.get('explore_detect_end_time_label'),
            width: '180px'
          }
        ]
      }
    ];
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
            class: includes(['name'], col.key)
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
    defer(() => this.initColGroup());
  }

  optsCallback = data => {
    return this.getOptItems(data);
  };

  getOptItems(data) {
    const menus = [
      {
        id: 'latestResult',
        label: this.i18n.get('explore_latest_detection_result_label'),
        permission: OperateItems.AddReport,
        disabled: !includes(
          [
            DataMap.detectionSnapshotStatus.infected.value,
            DataMap.detectionSnapshotStatus.uninfected.value
          ],
          data.status
        ),
        onClick: () => {
          if (
            data.generate_type === DataMap.snapshotGeneratetype.ioDetect.value
          ) {
            this.copiesDetectReportService
              .ShowDetectionDetails({
                pageNo: CommonConsts.PAGE_START,
                pageSize: CommonConsts.PAGE_SIZE,
                resourceId: data.resource_id,
                conditions: JSON.stringify({
                  uuid: data.latest_copy_id
                })
              })
              .subscribe(snapshot => {
                this.copyActionService.detectionReport(first(snapshot.items));
              });
          } else {
            this.copyActionService.detectionReport({
              ...data,
              uuid: data.latest_copy_id
            });
          }
        }
      },
      {
        id: 'historyResult',
        permission: OperateItems.AddReport,
        label: this.i18n.get('explore_history_detection_result_label'),
        disabled: false,
        divide: true,
        onClick: () => this.getHistoryData(data)
      },
      {
        id: 'errorHandle',
        permission: OperateItems.AddReport,
        label: this.i18n.get('explore_error_feedbac_label'),
        disabled: !includes(
          [DataMap.detectionSnapshotStatus.infected.value],
          data.status
        ),
        divide: true,
        onClick: () =>
          this.copyActionService.dealMisreport({
            copyId: data.latest_copy_id,
            snapshotTime: data.latest_snapshot_time,
            generate_type: data.generate_type
          })
      },
      {
        id: 'recovery',
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('common_restore_label'),
        onClick: () => {
          if (data.file_sub_type === 2) {
            this.apiStorageBackupPluginService
              .QueryPacificFileSystemWormPolicy({
                fileSystemId: data.resource_id
              })
              .subscribe(res => {
                if (res === 'true') {
                  this.messageBox.confirm({
                    lvContent: this.i18n.get(
                      'common_cyberengine_worm_disrestore_label'
                    ),
                    lvOk: () => {
                      this.getResource();
                      return;
                    }
                  });
                } else {
                  this.copyActionService.snapshotRestore(data, true);
                }
              });
          } else if (data.file_sub_type === 1) {
            this.messageBox.confirm(
              this.i18n.get('explore_worm_recovery_tip_label', [data.name])
            );
          } else {
            this.copyActionService.snapshotRestore(data, true);
          }
        }
      }
    ];
    const btns = getPermissionMenuItem(menus, this.cookieService.role);
    this.optItems = [...btns];
    return btns;
  }

  getParams() {
    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize,
      resourceSubType: DataMap.Resource_Type.LocalFileSystem.value
    };
    if (!isEmpty(this.orders)) {
      assign(params, {
        orders: this.orders
      });
    }
    if (!isEmpty(this.filterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.filterParams)
      });
    }
    return params;
  }

  getResource(name?: string) {
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }
    if (!isUndefined(name)) {
      this.resourceName = name;
      assign(this.filterParams, {
        resource_name: trim(name)
      });
    }
    const params = this.getParams();
    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL_RESOURCE)
      .pipe(
        switchMap(index => {
          return this.copiesDetectReportService.ShowDetectionStatistics({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.tableData = res.items;
        this.total = res.total;
        this.refreshResource.emit();
        this.cdr.detectChanges();
        const cacheResourceName = this.appUtilsService.getCacheValue(
          'resourceName'
        );
        const findResoure = find(res.items, { name: cacheResourceName });
        if (cacheResourceName && findResoure) {
          if (
            findResoure['generate_type'] ===
            DataMap.snapshotGeneratetype.ioDetect.value
          ) {
            this.copiesDetectReportService
              .ShowDetectionDetails({
                pageNo: CommonConsts.PAGE_START,
                pageSize: CommonConsts.PAGE_SIZE,
                resourceId: findResoure.resource_id,
                conditions: JSON.stringify({
                  uuid: findResoure.latest_copy_id
                })
              })
              .subscribe(snapshot => {
                this.copyActionService.detectionReport(first(snapshot.items));
              });
          } else {
            this.copyActionService.detectionReport({
              ...findResoure,
              uuid: findResoure.latest_copy_id
            });
          }
        }
      });
  }

  searchByResourceName(resourceName) {
    assign(this.filterParams, {
      resource_name: trim(resourceName)
    });
    this.getResource();
  }

  searchBytenantName(tenantName) {
    assign(this.filterParams, {
      tenant_name: trim(tenantName)
    });
    this.getResource();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getResource();
  }

  sortChange(source) {
    this.orders = [];
    this.orders.push((source.direction === 'asc' ? '+' : '-') + source.key);
    this.getResource();
  }

  filterChange(e) {
    assign(this.filterParams, {
      [e.key]: e.value
    });
    this.getResource();
  }

  trackByUuid = (_, item) => {
    return item.resource_id;
  };

  getResourceDetail(item, isCopy?) {
    if (isCopy && !item.total_copy_num) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: item.resource_id })
      .subscribe(res => {
        extendSlaInfo(res);
        assign(res, {
          file_sub_type: item.file_sub_type
        });
        if (isCopy) {
          assign(res, {
            activeId: 'copydata',
            datePickerMode: DATE_PICKER_MODE.DATE
          });
        }
        this.detailService.openDetailModal(res.subType, {
          data: assign(
            res,
            {
              optItems: this.getOptItems(item)
            },
            {
              optItemsFn: v => {
                return this.getOptItems(v);
              }
            }
          )
        });
      });
  }

  getHistoryData(rowData) {
    // 副本只查看已感染和未感染
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('explore_history_detection_result_label'),
        lvContent: CyberSnapshotDataComponent,
        lvModalKey: 'history-data-draw',
        lvWidth: MODAL_COMMON.xLargeWidth,
        lvModality: false,
        lvComponentParams: {
          isHistory: true,
          id: rowData.resource_id,
          status: [
            DataMap.detectionSnapshotStatus.infected.value,
            DataMap.detectionSnapshotStatus.uninfected.value
          ]
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
}
