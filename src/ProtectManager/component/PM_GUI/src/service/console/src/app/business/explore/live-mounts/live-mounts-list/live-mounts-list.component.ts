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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnDestroy,
  OnInit,
  ViewChild
} from '@angular/core';
import { DatatableComponent, MessageboxService, OptionItem } from '@iux/live';
import {
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  GROUP_COMMON,
  GlobalService,
  I18NService,
  LiveMountAction,
  LiveMountApiService,
  LiveMountPolicyApiService,
  LiveMountUpdateModal,
  MODAL_COMMON,
  MultiCluster,
  OperateItems,
  ResourceType,
  RoleOperationMap,
  SYSTEM_TIME,
  SystemApiService,
  VirtualResourceService,
  WarningMessageService,
  getPermissionMenuItem
} from 'app/shared';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  map as _map,
  toString as _toString,
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  includes,
  intersection,
  isEmpty,
  isFunction,
  isUndefined,
  reject,
  size,
  trim
} from 'lodash';
import { Subject, Subscription, combineLatest, timer } from 'rxjs';
import { map, switchMap, takeUntil } from 'rxjs/operators';
import { UpdatePolicyDetailComponent } from '../../policy/mount-update-policy/update-policy-detail/update-policy-detail.component';
import { LiveMountMigrateComponent as CnwareLiveMountMigrateComponent } from '../cnware/live-mount-migrate/live-mount-migrate.component';
import { LiveMountSummaryComponent as CnwareLiveMountSummaryComponent } from '../cnware/live-mount-summary/live-mount-summary.component';
import { LiveMountSummaryComponent as FilesetMountSummaryComponent } from '../fileset/live-mount-summary/live-mount-summary.component';
import { LiveMountCreateComponent } from '../live-mount-create/live-mount-create.component';
import { LiveMountSummaryComponent as NasLiveMountSummaryComponent } from '../nas-shared/live-mount-summary/live-mount-summary.component';
import { LiveMountSummaryComponent as OracleLiveMountSummaryComponent } from '../oracle/live-mount-summary/live-mount-summary.component';
import { LiveMountMigrateComponent } from '../vmware/live-mount-migrate/live-mount-migrate.component';
import { LiveMountSummaryComponent as VMwareLiveMountSummaryComponent } from '../vmware/live-mount-summary/live-mount-summary.component';
import { DestroyLiveMountComponent } from './destroy-live-mount/destroy-live-mount.component';
import { LiveMountModifyComponent } from './live-mount-modify/live-mount-modify.component';
import { UpdateCopyDataComponent } from './update-copy-data/update-copy-data.component';
@Component({
  selector: 'aui-live-mounts-list',
  templateUrl: './live-mounts-list.component.html',
  styleUrls: ['./live-mounts-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [DatePipe]
})
export class LiveMountsListComponent implements OnInit, OnDestroy {
  targetResourceName;
  targetResourcePath;
  targetResourceIp;
  resourceName;
  resourcePath;
  resourceIp;
  policyName;
  clusterName;
  selection = [];
  tableData = [];
  updateBtnDisabled = true;
  destroyBtnDisabled = true;
  orders = ['-created_time'];
  page = CommonConsts.PAGE_START;
  size = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  columns = [];
  filterParams = {};
  activeSort = { key: 'created_time', direction: 'desc' };
  columnSelection = [];
  filter = filter;
  timeSub$: Subscription;
  destroy$ = new Subject();
  resourceTypeEnum = ResourceType;
  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  groupCommon = GROUP_COMMON;
  roleOperationMap = RoleOperationMap;

  @Input() header;
  @Input() resourceType;
  @Input() childResourceType;
  @ViewChild('lvTable', { static: false }) lvTable: DatatableComponent;

  constructor(
    private i18n: I18NService,
    private globalService: GlobalService,
    private messageBox: MessageboxService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private systemApiService: SystemApiService,
    private batchOperateService: BatchOperateService,
    private liveMountApiService: LiveMountApiService,
    private warningMessageService: WarningMessageService,
    private liveMountPolicyApiService: LiveMountPolicyApiService,
    private cookieService: CookieService,
    private virtualResourceService: VirtualResourceService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private datePipe: DatePipe
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.getColumns();
    this.getMounts();
    this.initColumnSelection();
    this.getGlobalEmitStore();
    this.virtualScroll.getScrollParam(this.isHcsUser ? 360 : 260);
  }

  onChange() {
    this.ngOnInit();
  }

  getGlobalEmitStore() {
    this.globalService
      .getState(LiveMountAction.Create)
      .subscribe(res => this.getMounts());
  }

  getColumns() {
    this.columns = [
      {
        label: this.i18n.get('common_target_object_label'),
        expanded: true,
        disabled: true,
        show: true,
        children: [
          {
            key: 'target_resource_name',
            label: this.i18n.get('common_name_label'),
            disabled: true,
            show: true,
            isLeaf: true
          },
          {
            key: 'status',
            label: this.i18n.get('common_status_label'),
            show: true,
            isLeaf: true,
            filter: true,
            filterMap: this.dataMapService
              .toArray('LiveMount_Status')
              .filter((v: OptionItem) => {
                return (
                  this.resourceType === ResourceType.VM ||
                  v.value !== DataMap.LiveMount_Status.migrating.value
                );
              })
          },
          {
            key: 'target_resource_ip',
            resourceType: [DataMap.Resource_Type.oracle.value],
            label: this.i18n.get('common_ip_address_label'),
            show: true
          },
          {
            key: 'target_resource_path',
            resourceType: [
              DataMap.Resource_Type.virtualMachine.value,
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.volume.value
            ],
            label: this.i18n.get('common_location_label'),
            show: true,
            isLeaf: true
          },
          {
            key: 'protocol_type',
            resourceType: [
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.NASShare.value
            ],
            label: this.i18n.get('explore_share_protocol_label'),
            show: true,
            isLeaf: true
          },
          {
            key: 'share_name',
            resourceType: [
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.NASShare.value
            ],
            label: this.i18n.get('explore_share_name_or_path_label'),
            show: true,
            isLeaf: true
          }
        ]
      },
      {
        label: this.i18n.get('common_protected_object_label'),
        expanded: true,
        disabled: true,
        show: true,
        children: [
          {
            key: 'resource_name',
            label: this.i18n.get('common_name_label'),
            show: true,
            disabled: true,
            isLeaf: true
          },
          {
            key: 'resource_ip',
            resourceType: [DataMap.Resource_Type.oracle.value],
            label: this.i18n.get('common_ip_address_label'),
            show: true
          },
          {
            key: 'resource_path',
            resourceType: [
              DataMap.Resource_Type.virtualMachine.value,
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.volume.value
            ],
            label: this.i18n.get('common_location_label'),
            show: true,
            isLeaf: true
          }
        ]
      },
      {
        label: this.i18n.get('common_copy_data_label'),
        expanded: true,
        disabled: true,
        show: true,
        children: [
          {
            key: 'cluster_name',
            label: this.i18n.get('system_servers_label'),
            disabled: false,
            show: false,
            isLeaf: true,
            displayCheck: () => {
              return MultiCluster.isMulti;
            }
          },
          {
            key: 'mounted_copy_display_timestamp',
            label: this.i18n.get('common_time_stamp_label'),
            show: true,
            isLeaf: true,
            showSort: true
          },
          {
            key: 'policy_name',
            resourceType: [
              DataMap.Resource_Type.oracle.value,
              DataMap.Resource_Type.virtualMachine.value,
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.volume.value
            ],
            label: this.i18n.get('common_update_policy_name_label'),
            show: true,
            isLeaf: true
          },
          {
            key: 'enable_status',
            label: this.i18n.get('explore_livemount_status_label'),
            show: true,
            isLeaf: true,
            filter: true,
            filterMap: this.dataMapService.toArray(
              'LiveMount_Activation_Status'
            )
          }
        ]
      }
    ];

    each(this.columns, column => {
      if (!column.children) {
        return;
      }
      column.children = filter(column.children, children => {
        if (!children.resourceType) {
          return true;
        }

        return !!size(
          intersection(this.childResourceType, children.resourceType)
        );
      });
    });
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
            class: 'aui-th-deliver'
          });
        });
      }
    });
    each(this.columns, child => {
      child.children = child.children.filter(data =>
        isFunction(data.displayCheck) ? data.displayCheck() : true
      );
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

  getShare(item) {
    if (this.resourceType !== ResourceType.Storage) {
      return '';
    }
    let fileSystemProtocol = DataMap.NasFileSystem_Protocol.none.label;
    try {
      const fileSystem = JSON.parse(item.file_system_share_info) || [];
      if (fileSystem.length === 2) {
        fileSystemProtocol = DataMap.NasFileSystem_Protocol.nfs_cifs.label;
      } else {
        fileSystemProtocol = this.dataMapService.getLabel(
          'NasFileSystem_Protocol',
          _toString(fileSystem[0].type)
        );
      }
    } catch (error) {
      fileSystemProtocol = this.i18n.get(
        DataMap.NasFileSystem_Protocol.none.label
      );
    }
    return fileSystemProtocol;
  }

  getShareName(item) {
    if (this.resourceType !== ResourceType.Storage) {
      return '';
    }
    let fileSystem;
    let shareName = [];
    try {
      fileSystem = JSON.parse(item.file_system_share_info) || [];

      if (size(fileSystem)) {
        each(fileSystem, item => {
          if (item.type === 1) {
            item.advanceParams?.sharePath
              ? shareName.push(item.advanceParams?.sharePath)
              : shareName.push('--');
          }
          if (item.type === 0) {
            shareName.push(item.advanceParams?.shareName);
          }
        });
      }
    } catch (error) {
      fileSystem = {};
    }
    return shareName.join(' , ');
  }

  getMounts() {
    this.selection = [];
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }

    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          const params = this.getParams();
          return this.liveMountApiService.queryLiveMountEntitiesUsingGET({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        if (
          this.resourceType === ResourceType.VM &&
          res.items[0]?.resource_sub_type !==
            DataMap.Resource_Type.cNwareVm.value
        ) {
          each(res.items, item => {
            if (
              find(this.tableData, { id: item.id }) &&
              find(this.tableData, { id: item.id })['vmware_sub_type']
            ) {
              item['vmware_sub_type'] = find(this.tableData, { id: item.id })[
                'vmware_sub_type'
              ];
            }
            this.virtualResourceService
              .queryResourcesV1VirtualResourceGet({
                pageNo: CommonConsts.PAGE_START,
                pageSize: CommonConsts.PAGE_SIZE,
                conditions: JSON.stringify({ uuid: item.mounted_resource_id }),
                akLoading: false
              })
              .subscribe(vm => {
                assign(item, {
                  vmware_sub_type:
                    vm.items[0] && vm.items[0].environment_sub_type
                });
                this.cdr.detectChanges();
              });
          });
        }
        this.total = res.total;
        this.tableData = res.items;
        this.cdr.detectChanges();
      });
  }

  getParams() {
    const params = {
      page: this.page,
      size: this.size
    };

    assign(this.filterParams, {
      resource_sub_type: includes(
        this.childResourceType,
        DataMap.Resource_Type.oracle.value
      )
        ? [
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oracleCluster.value
          ]
        : this.childResourceType
    });

    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });

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

  pageChange(page) {
    this.size = page.pageSize;
    this.page = page.pageIndex;
    this.getMounts();
  }

  filterChange(e) {
    assign(this.filterParams, {
      [e.key]: e.value
    });
    this.getMounts();
  }

  sortChange(source) {
    this.orders = [];
    this.orders.push((source.direction === 'asc' ? '+' : '-') + source.key);
    this.getMounts();
  }

  selectionChange() {
    this.destroyBtnDisabled =
      !size(this.selection) ||
      !isUndefined(
        find(this.selection, item => {
          return !includes(
            [
              DataMap.LiveMount_Status.available.value,
              DataMap.LiveMount_Status.mountFailed.value,
              DataMap.LiveMount_Status.invalid.value
            ],
            item.status
          );
        })
      );
    this.updateBtnDisabled =
      !size(this.selection) ||
      !isUndefined(
        find(this.selection, item => {
          return !(
            includes(
              [
                DataMap.LiveMount_Status.available.value,
                DataMap.LiveMount_Status.mountFailed.value
              ],
              item.status
            ) &&
            item.enable_status ===
              DataMap.LiveMount_Activation_Status.activated.value
          );
        })
      );
  }

  searchByTargetResourceName(value) {
    assign(this.filterParams, {
      target_resource_name: trim(value)
    });
    this.getMounts();
  }

  searchByTargetResourcePath(value) {
    assign(this.filterParams, {
      target_resource_path: trim(value)
    });
    this.getMounts();
  }

  searchByTargetResourceIp(value) {
    assign(this.filterParams, {
      target_resource_ip: trim(value)
    });
    this.getMounts();
  }

  searchByResourceName(value) {
    assign(this.filterParams, {
      resource_name: trim(value)
    });
    this.getMounts();
  }

  searchByResourcePath(value) {
    assign(this.filterParams, {
      resource_path: trim(value)
    });
    this.getMounts();
  }

  searchByResourceIp(value) {
    assign(this.filterParams, {
      resource_ip: trim(value)
    });
    this.getMounts();
  }

  searchByPolicyName(value) {
    assign(this.filterParams, {
      policy_name: trim(value)
    });
    this.getMounts();
  }

  searchByClusterName(value) {
    assign(this.filterParams, {
      cluster_name: trim(value)
    });
    this.getMounts();
  }

  getMountedCopyTime(displayTime) {
    let time: any;
    try {
      time = this.datePipe.transform(displayTime, 'yyyy-MM-dd HH:mm:ss');
    } catch (e) {
      time = displayTime;
    }
    return time;
  }

  getPolicyDetail(item) {
    this.liveMountPolicyApiService
      .getPolicyUsingGET({
        policyId: item.policy_id
      })
      .subscribe(data => {
        this.drawModalService.openDetailModal(
          assign({}, MODAL_COMMON.generateDrawerOptions(), {
            lvModalKey: 'updating-policy-detail',
            lvWidth: MODAL_COMMON.normalWidth,
            lvHeader: data.name,
            lvContent: UpdatePolicyDetailComponent,
            lvComponentParams: {
              data
            },
            lvFooter: [
              {
                label: this.i18n.get('common_close_label'),
                onClick: modal => modal.close()
              }
            ]
          })
        );
      });
  }

  getMountDetail(item) {
    const params = {
      page: CommonConsts.PAGE_START,
      size: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        id: item.id
      })
    };
    this.liveMountApiService
      .queryLiveMountEntitiesUsingGET(params)
      .pipe(
        map(res => {
          return first(res.items);
        })
      )
      .subscribe(res => {
        const component = this.getLiveMountSummaryComponent();
        this.drawModalService.openDetailModal(
          assign({}, MODAL_COMMON.generateDrawerOptions(), {
            lvModalKey: 'live-mount-detail',
            lvWidth: MODAL_COMMON.largeWidth,
            lvHeader: this.i18n.get('common_view_label'),
            lvContent: component,
            lvComponentParams: {
              componentData: {
                liveMountData: res,
                action: LiveMountAction.View,
                resourceType: this.resourceType,
                childResourceType: this.childResourceType
              }
            },
            lvFooter: [
              {
                label: this.i18n.get('common_close_label'),
                onClick: modal => modal.close()
              }
            ]
          })
        );
        this.cdr.detectChanges();
      });
  }

  create() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_create_label'),
      lvFooter: null,
      lvContent: LiveMountCreateComponent,
      lvWidth: MODAL_COMMON.largeWidth + 50,
      lvComponentParams: {
        componentData: {
          resourceType: this.resourceType,
          childResourceType: this.childResourceType
        }
      }
    });
  }

  destroy(items) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-warning-48',
        lvHeader: this.i18n.get('common_danger_label'),
        lvContent: DestroyLiveMountComponent,
        lvComponentParams: {
          items
        },
        lvWidth: 600,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: true,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvAfterOpen: modal => {
          const component = modal.getContentComponent() as DestroyLiveMountComponent;
          const modalIns = modal.getInstance();
          component.isChecked$.subscribe(e => {
            modalIns.lvOkDisabled = !e;
          });
        },
        lvOk: modal => {
          const component = modal.getContentComponent() as DestroyLiveMountComponent;
          this.batchOperateService.selfGetResults(
            item => {
              return this.liveMountApiService.unmountLiveMountUsingDELETE({
                liveMountId: item.id,
                reserveCopy: component.reserveCopy,
                forceDelete: component.forceDelete,
                akOperationTips: false,
                akDoException: false,
                akLoading: false
              });
            },
            _map(cloneDeep(component.items), i => {
              return assign(i, {
                name: i.target_resource_name,
                isAsyn: true
              });
            }),
            () => {
              this.lvTable.clearSelection();
              this.getMounts();
              this.selectionChange();
              this.cdr.detectChanges();
            }
          );
        }
      }
    });
  }

  updateLatestCopyData(items) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_update_live_mount_label'),
      onOK: () => {
        this.batchOperateService.selfGetResults(
          item => {
            return this.liveMountApiService.updateLiveMountUsingPUT({
              akOperationTips: false,
              akDoException: false,
              akLoading: false,
              liveMountId: item.id,
              liveMountUpdate: {
                mode: LiveMountUpdateModal.Latest
              }
            });
          },
          _map(cloneDeep(items), i => {
            return assign(i, { name: i.target_resource_name, isAsyn: true });
          }),
          () => {
            this.lvTable.clearSelection();
            this.getMounts();
            this.selectionChange();
            this.cdr.detectChanges();
          }
        );
      }
    });
  }

  update(item) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_update_label'),
      lvContent: UpdateCopyDataComponent,
      lvWidth: MODAL_COMMON.largeWidth,
      lvComponentParams: {
        item
      },
      lvOkDisabled: false,
      lvAfterOpen: modal => {},
      lvOk: modal => {
        return new Promise(resolve => {
          const component = modal.getContentComponent() as UpdateCopyDataComponent;
          component.onOK().subscribe(
            res => {
              resolve(true);
              this.getMounts();
            },
            err => {
              resolve(false);
            }
          );
        });
      }
    });
  }

  modify(item) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_modify_label'),
      lvContent: LiveMountModifyComponent,
      lvWidth: MODAL_COMMON.largeWidth,
      lvComponentParams: {
        item,
        componentData: {
          resourceType: this.resourceType,
          childResourceType: this.childResourceType
        }
      },
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const component = modal.getContentComponent() as LiveMountModifyComponent;
        const modalIns = modal.getInstance();
        component.formGroup.statusChanges.subscribe(res => {
          modalIns.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const component = modal.getContentComponent() as LiveMountModifyComponent;
          component.onOK().subscribe(
            res => {
              resolve(true);
              if (!component.formGroup.value.policyId) {
                this.getMounts();
                return;
              }
              this.executeMount(component.formGroup.value.policyId, item.id);
            },
            err => resolve(false)
          );
        });
      }
    });
  }

  active(item) {
    this.liveMountApiService
      .activeUsingPUT({ liveMountId: item.id })
      .subscribe(() => this.getMounts());
  }

  disable(item) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_livemount_disable_label'),
      onOK: () => {
        this.liveMountApiService
          .deactivateUsingPUT({ liveMountId: item.id })
          .subscribe(() => this.getMounts());
      }
    });
  }

  migrate(item) {
    let migrateComponent;
    if (item.resource_sub_type === DataMap.Resource_Type.cNwareVm.value) {
      migrateComponent = CnwareLiveMountMigrateComponent;
    } else {
      migrateComponent = LiveMountMigrateComponent;
    }
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_migrate_label'),
      lvContent: migrateComponent,
      lvWidth:
        item.resource_sub_type === DataMap.Resource_Type.cNwareVm.value
          ? MODAL_COMMON.largeWidth + 150
          : MODAL_COMMON.largeWidth - 100,
      lvComponentParams: {
        item
      },
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const component = modal.getContentComponent();
        const modalIns = modal.getInstance();
        const combined: any = combineLatest(
          component.formGroup.statusChanges,
          component.diskValid$
        );
        combined.subscribe(latestValues => {
          const [formGroupStatus, diskValid] = latestValues;
          modalIns.lvOkDisabled = !(formGroupStatus === 'VALID' && diskValid);
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const component = modal.getContentComponent();
          component.onOK().subscribe(
            res => {
              resolve(true);
              this.getMounts();
            },
            err => {
              resolve(false);
            }
          );
        });
      }
    });
  }

  executeMount(policyId, liveMountId) {
    combineLatest([
      this.liveMountPolicyApiService.getPolicyUsingGET({
        policyId
      }),
      this.systemApiService.getSystemTimeUsingGET({})
    ]).subscribe(res => {
      if (
        !!res[0].scheduleStartTime &&
        !!res[1].time &&
        new Date(res[0].scheduleStartTime).getTime() <
          new Date(res[1].time).getTime()
      ) {
        this.messageBox.confirm({
          lvOkType: 'primary',
          lvCancelType: 'default',
          lvContent: this.i18n.get('explore_livemount_first_execute_label'),
          lvOk: () => {
            this.liveMountApiService
              .updateLiveMountUsingPUT({
                liveMountId,
                liveMountUpdate: {
                  mode: LiveMountUpdateModal.Latest
                }
              })
              .subscribe(() => this.getMounts());
          },
          lvCancel: () => this.getMounts()
        });
      } else {
        this.getMounts();
      }
    });
  }

  getLiveMountSummaryComponent() {
    let component;
    if (this.resourceType === ResourceType.DATABASE) {
      component = OracleLiveMountSummaryComponent;
    } else if (this.resourceType === ResourceType.VM) {
      if (
        includes(this.childResourceType, DataMap.Resource_Type.cNwareVm.value)
      ) {
        component = CnwareLiveMountSummaryComponent;
      } else {
        component = VMwareLiveMountSummaryComponent;
      }
    } else if (this.resourceType === ResourceType.Storage) {
      component = NasLiveMountSummaryComponent;
    } else if (this.resourceType === ResourceType.HOST) {
      component = FilesetMountSummaryComponent;
    }

    return component;
  }

  optsCallback = data => {
    return this.getOptItems(data);
  };

  getOptItems(item) {
    const menus = [
      {
        id: 'modify',
        disabled: !(
          includes(
            [
              DataMap.LiveMount_Status.available.value,
              DataMap.LiveMount_Status.mountFailed.value
            ],
            item.status
          ) &&
          item.enable_status ===
            DataMap.LiveMount_Activation_Status.activated.value
        ),
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifiedLiveMount,
        onClick: () => this.modify(item)
      },
      {
        id: 'destroy',
        disabled: !includes(
          [
            DataMap.LiveMount_Status.available.value,
            DataMap.LiveMount_Status.mountFailed.value,
            DataMap.LiveMount_Status.invalid.value
          ],
          item.status
        ),
        label: this.i18n.get('explore_destroy_label'),
        permission: OperateItems.DestroyLiveMount,
        onClick: () => this.destroy([item])
      },
      {
        id: 'update',
        disabled: !(
          includes(
            [
              DataMap.LiveMount_Status.available.value,
              DataMap.LiveMount_Status.mountFailed.value
            ],
            item.status
          ) &&
          item.enable_status ===
            DataMap.LiveMount_Activation_Status.activated.value
        ),
        hidden: this.resourceType === ResourceType.Storage,
        label: this.i18n.get('common_update_label'),
        permission: OperateItems.UpdateLiveMount,
        onClick: () => this.update(item)
      },
      {
        id: 'migrate',
        disabled: !(
          DataMap.LiveMount_Status.available.value === item.status &&
          item.enable_status ===
            DataMap.LiveMount_Activation_Status.activated.value &&
          item.mounted_copy_id &&
          !includes(
            [
              DataMap.Resource_Type.vmwareEsx.value,
              DataMap.Resource_Type.vmwareEsxi.value
            ],
            item.vmware_sub_type
          )
        ),
        tips: includes(
          [
            DataMap.Resource_Type.vmwareEsx.value,
            DataMap.Resource_Type.vmwareEsxi.value
          ],
          item.vmware_sub_type
        )
          ? this.i18n.get('protection_migrate_disable_esx_label')
          : '',
        hidden: this.resourceType !== ResourceType.VM,
        label: this.i18n.get('common_migrate_label'),
        permission: OperateItems.MigrateLiveMount,
        onClick: () => this.migrate(item)
      },
      {
        id: 'active',
        disabled: !(
          includes(
            [
              DataMap.LiveMount_Status.available.value,
              DataMap.LiveMount_Status.mountFailed.value
            ],
            item.status
          ) &&
          item.enable_status ===
            DataMap.LiveMount_Activation_Status.disabled.value
        ),
        label: this.i18n.get('common_active_label'),
        hidden: this.resourceType === ResourceType.Storage,
        permission: OperateItems.ActivateLiveMount,
        onClick: () => this.active(item)
      },
      {
        id: 'disable',
        disabled: !(
          includes(
            [
              DataMap.LiveMount_Status.available.value,
              DataMap.LiveMount_Status.mountFailed.value
            ],
            item.status
          ) &&
          item.enable_status ===
            DataMap.LiveMount_Activation_Status.activated.value
        ),
        label: this.i18n.get('common_disable_label'),
        hidden: this.resourceType === ResourceType.Storage,
        permission: OperateItems.DisableLiveMount,
        onClick: () => this.disable(item)
      },
      {
        id: 'view',
        label: this.i18n.get('common_view_label'),
        permission: OperateItems.ViewLiveMount,
        onClick: () => this.getMountDetail(item)
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  }

  trackById = (index, item) => {
    return item.id;
  };
}
