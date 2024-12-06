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
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  ApplicationType,
  autoTableScroll,
  CommonConsts,
  CookieService,
  CopyControllerService,
  DatabasesService,
  DataMap,
  DataMapService,
  extendSlaInfo,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectedCopyObjectApiService,
  ProtectedResourceApiService,
  ProtectResourceAction,
  ProtectResourceCategory,
  RoleType,
  VirtualResourceService,
  WarningMessageService
} from 'app/shared';
import { CopiesService } from 'app/shared/api/services/copies.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SlaService } from 'app/shared/services/sla.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  filter,
  find,
  get,
  includes,
  intersection,
  isEmpty,
  isNil,
  isNull,
  isUndefined,
  map,
  reject,
  size,
  toString,
  trim
} from 'lodash';
import { ManualCopyComponent } from './manual-copy/manual-copy.component';
import { finalize } from 'rxjs/operators';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { ModifyOwnedUserComponent } from 'app/shared/components/modify-owned-user/modify-owned-user.component';

@Component({
  selector: 'aui-resource-replica-list',
  templateUrl: './resource-replica-list.component.html',
  styleUrls: ['./resource-replica-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ResourceReplicaListComponent implements OnInit {
  slaName;
  resourceId;
  resourceName;
  resourceLocation;
  protectedSlaName;
  tableData = [];
  isShowOptCol = false;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  orders = [];
  filterParams: any = {};
  columns = [];
  columnSelection = [];
  filter = filter;
  dataMap = DataMap;
  _isUndefined = isUndefined;
  _isNull = isNull;

  clickHouseTypeFilter;
  activeItem;

  isHyperdetect = includes(
    [
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value
    ],
    this.i18n.get('deploy_type')
  );
  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  @Input() resourceType;
  @Input() childResourceType;

  constructor(
    private i18n: I18NService,
    private cookieService: CookieService,
    private protectService: ProtectService,
    private detailService: ResourceDetailService,
    private copiesApiService: CopiesService,
    private dataMapService: DataMapService,
    public warningMessageService: WarningMessageService,
    private protectedCopyObjectApiService: ProtectedCopyObjectApiService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private virtualResourceService: VirtualResourceService,
    private databasesService: DatabasesService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private drawModalService: DrawModalService,
    private slaService: SlaService,
    private copyControllerService: CopyControllerService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.getColumns();
    this.getResource();
    this.initColumnSelection();
    this.virtualScroll.getScrollParam(this.isHyperdetect ? 170 : 270);
    autoTableScroll(
      this.virtualScroll,
      this.isHyperdetect ? 170 : 270,
      null,
      this.cdr
    );
  }

  getColumns() {
    this.columns = [
      {
        label: this.i18n.get('common_resource_label'),
        expanded: true,
        disabled: true,
        show: true,
        children: [
          {
            key: 'resourceId',
            show: false,
            isLeaf: true,
            label: this.i18n.get('protection_resource_id_label')
          },
          {
            key: 'resourceName',
            show: true,
            isLeaf: true,
            label: this.i18n.get('common_name_label')
          },
          {
            key: 'resourceSubType',
            label: this.i18n.get('common_type_label'),
            filter: true,
            show: true,
            disabled: true,
            isLeaf: true,
            width: '130px',
            filterMap: includes(
              this.childResourceType,
              DataMap.Resource_Type.DWS_Cluster.value
            )
              ? this.dataMapService.toArray('CopyData_DWS_Type')
              : includes(
                  this.childResourceType,
                  DataMap.Resource_Type.MySQLClusterInstance.value
                )
              ? this.dataMapService.toArray('copyDataMysqlType')
              : includes(
                  this.childResourceType,
                  DataMap.Resource_Type.OpenGauss.value
                )
              ? this.dataMapService.toArray('CopyDataOpengaussType')
              : includes(
                  this.childResourceType,
                  DataMap.Resource_Type.ClickHouse.value
                )
              ? this.dataMapService.toArray('clickHouseResourceType')
              : includes(
                  this.childResourceType,
                  DataMap.Resource_Type.dbTwoDatabase.value
                )
              ? this.dataMapService.toArray('copyDataDbTwoType')
              : includes(
                  this.childResourceType,
                  DataMap.Resource_Type.OceanBase.value
                )
              ? this.dataMapService.toArray('copyDataOceanBaseType')
              : includes(
                  this.childResourceType,
                  DataMap.Resource_Type.tidb.value
                )
              ? this.dataMapService.toArray('tidbResourceType')
              : this.dataMapService.toArray('CopyData_SQL_Server_Type'),
            resourceType: [
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.OpenGauss.value,
              DataMap.Resource_Type.DWS_Database.value,
              DataMap.Resource_Type.SQLServerDatabase.value,
              DataMap.Resource_Type.MySQLDatabase.value,
              DataMap.Resource_Type.ClickHouseCluster.value,
              DataMap.Resource_Type.ClickHouseDatabase.value,
              DataMap.Resource_Type.ClickHouseTableset.value,
              DataMap.Resource_Type.OceanBase.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.tidbTable.value
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
            key: 'resourceLocation',
            show: !size(
              intersection(this.childResourceType, [
                this.dataMap.Resource_Type.Redis.value
              ])
            ),
            isLeaf: true,
            label: this.i18n.get('common_location_label')
          },
          {
            key: 'resourceStatus',
            filter: true,
            show: !size(
              intersection(this.childResourceType, [
                this.dataMap.Resource_Type.ImportCopy.value
              ])
            ),
            isLeaf: true,
            label: this.i18n.get('common_status_label'),
            filterMap: this.dataMapService.toArray('Resource_Status')
          }
        ]
      },
      {
        label: this.i18n.get('common_copies_label'),
        expanded: true,
        disabled: true,
        show: true,
        children: [
          {
            key: 'copyCount',
            show: true,
            isLeaf: true,
            label: this.i18n.get('common_amount_label'),
            sort: true
          },
          {
            key: 'protectedSlaName',
            show: true,
            isLeaf: true,
            label: this.i18n.get('common_copy_data_sla_label')
          },
          {
            key: 'protectedStatus',
            show: true,
            filter: true,
            isLeaf: true,
            filterMap: this.dataMapService.toArray('Resource_Protected_Status'),
            label: this.i18n.get('protection_copy_data_protected_status_label')
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
        each(column.children, col => {
          assign(col, {
            class: 'aui-th-deliver'
          });
        });
        this.columnSelection.push(...column.children);
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

  private _dispatchFilterParams() {
    if (this.childResourceType?.includes('ClickHouse')) {
      assign(this.filterParams, {
        resourceSubType: this.childResourceType,
        resourceType: !isNil(this.clickHouseTypeFilter)
          ? this.clickHouseTypeFilter
          : void 0
      });
      return;
    }

    assign(this.filterParams, {
      resourceSubType: !!size(get(this.filterParams, 'resourceSubType'))
        ? get(this.filterParams, 'resourceSubType')
        : this.childResourceType
    });
  }

  getSlaDetail(item) {
    this.slaService.getDetail({
      uuid: item.protectedSlaId,
      name: item.protectedSlaName
    });
  }

  getResource() {
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
    if (!isEmpty(this.filterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.filterParams)
      });
    }
    this.copyControllerService
      .queryCopySummaryResourceV2(params)
      .subscribe(res => {
        this.tableData = map(res.records, item => {
          assign(item, {
            resource_id: item.resourceId,
            resource_name: item.resourceName,
            protected_sla_id: item.protectedSlaId,
            sla_id: item.protectedSlaId,
            resource_script: get(
              JSON.parse(item.resourceProperties || '{}'),
              'extendInfo.databaseTypeDisplay'
            )
          });
          return item;
        });
        this.total = res.totalCount;
        this.cdr.detectChanges();
      });
  }

  optsCallback = data => {
    return this.getOptItems(data);
  };

  getOptItems(item) {
    const menus = [
      {
        id: 'protectResource',
        label: this.i18n.get('common_protect_label'),
        disabled: item.protectedSlaId,
        permission: OperateItems.Protection,
        onClick: () =>
          this.protectResource(
            item,
            ProtectResourceAction.Create,
            this.i18n.get('common_protect_label')
          )
      },
      {
        id: 'modifyProtect',
        disabled: !item.protectedSlaId,
        permission: OperateItems.ModifyProtection,
        label: this.i18n.get('protection_modify_protection_label'),
        onClick: () =>
          this.modifyProtect(
            item,
            ProtectResourceAction.Modify,
            this.i18n.get('protection_modify_protection_label')
          )
      },
      {
        id: 'removeProtection',
        divide: true,
        disabled: !item.protectedSlaId,
        permission: OperateItems.RemoveProtection,
        label: this.i18n.get('protection_remove_protection_label'),
        onClick: () => this.removeProtection(item)
      },
      {
        id: 'activeProtection',
        disabled: item.protectedStatus || !item.protectedSlaId,
        permission: OperateItems.ActivateProtection,
        label: this.i18n.get('protection_active_protection_label'),
        onClick: () => this.activeProtection(item)
      },
      {
        id: 'deactiveProtection',
        divide: true,
        disabled: !item.protectedStatus || !item.protectedSlaId,
        permission: OperateItems.DeactivateProtection,
        label: this.i18n.get('protection_deactive_protection_label'),
        onClick: () => this.deactiveProtection(item)
      },
      {
        id: 'manualCopy',
        disabled: !item.protectedSlaId,
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('protection_manual_copy_label'),
        onClick: () => this.manualCopy(item)
      },
      {
        id: 'modifyOwnedUser',
        disabled: this.cookieService.role !== RoleType.SysAdmin,
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('common_modify_owned_user_label'),
        onClick: () => this.modifyOwnedUser(item)
      }
    ];

    return getPermissionMenuItem(menus, this.cookieService.role);
  }

  protectResource(data, action: ProtectResourceAction, header?: string) {
    let hasReplicaCopy = false;
    const type = ProtectResourceCategory.Replica;
    const params = {
      pageSize: CommonConsts.PAGE_SIZE,
      pageNo: CommonConsts.PAGE_START,
      conditions: JSON.stringify({
        resource_id: data.resource_id,
        generated_by: [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value,
          DataMap.CopyData_generatedType.reverseReplication.value
        ]
      })
    };
    this.copiesApiService
      .queryResourcesV1CopiesGet(params)
      .pipe(
        finalize(() => {
          this.protectService.openProtectModal(type, action, {
            width: 780,
            data: {
              ...data,
              sub_type: ApplicationType.Replica,
              disableSla: this.isOceanProtect && !hasReplicaCopy
            },
            header,
            onOK: () => this.getResource()
          });
        })
      )
      .subscribe(res => {
        hasReplicaCopy = res.total > 0;
      });
  }

  modifyProtect(data, action: ProtectResourceAction, header?: string) {
    let hasReplicaCopy = false;
    const type = ProtectResourceCategory.Replica;
    const params = {
      pageSize: CommonConsts.PAGE_SIZE,
      pageNo: CommonConsts.PAGE_START,
      conditions: JSON.stringify({
        resource_id: data.resource_id,
        generated_by: [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value,
          DataMap.CopyData_generatedType.reverseReplication.value
        ]
      })
    };
    this.copiesApiService
      .queryResourcesV1CopiesGet(params)
      .pipe(
        finalize(() => {
          this.protectService.openProtectModal(type, action, {
            width: 780,
            data: {
              ...data,
              sub_type: ApplicationType.Replica,
              disableSla: this.isOceanProtect && !hasReplicaCopy
            },
            header,
            onOK: () => this.getResource()
          });
        })
      )
      .subscribe(res => {
        hasReplicaCopy = res.total > 0;
      });
  }

  removeProtection(data) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_resource_delete_protect_label', [
        toString(data.resourceName)
      ]),
      onOK: () => {
        this.protectedCopyObjectApiService
          .deleteV1ProtectedCopyObjectsDelete({
            body: {
              resource_ids: [data.resourceId]
            }
          })
          .subscribe(res => this.getResource());
      }
    });
  }

  activeProtection(data) {
    this.protectedCopyObjectApiService
      .activeV1ProtectedCopyObjectsStatusActionActivatePut({
        body: {
          resource_ids: [data.resourceId]
        }
      })
      .subscribe(res => this.getResource());
  }

  deactiveProtection(data) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_deactivate_resource_tip_label', [
        toString(data.resourceName)
      ]),
      onOK: () => {
        this.protectedCopyObjectApiService
          .deactivateV1ProtectedCopyObjectsStatusActionDeactivatePut({
            body: {
              resource_ids: [data.resourceId]
            }
          })
          .subscribe(res => this.getResource());
      }
    });
  }

  manualCopy(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('protection_manual_copy_label'),
      lvContent: ManualCopyComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth,
      lvComponentParams: {
        rowItem: assign({}, data)
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as ManualCopyComponent;
        const modalIns = modal.getInstance();
        content.formGroup.statusChanges.subscribe(res => {
          modalIns.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as ManualCopyComponent;
          content.onOK().subscribe(
            res => {
              resolve(true);
              this.getResource();
            },
            () => resolve(false)
          );
        });
      }
    });
  }

  modifyOwnedUser(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_modify_owned_user_label'),
      lvContent: ModifyOwnedUserComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth,
      lvComponentParams: {
        rowItem: assign({}, data)
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as ModifyOwnedUserComponent;
        const modalIns = modal.getInstance();
        content.formGroup.statusChanges.subscribe(res => {
          modalIns.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as ModifyOwnedUserComponent;
          content.onOK().subscribe(
            res => {
              resolve(true);
              this.getResource();
            },
            () => resolve(false)
          );
        });
      }
    });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getResource();
  }

  searchById(resourceId) {
    assign(this.filterParams, {
      resourceId: trim(resourceId)
    });
    this.getResource();
  }

  searchByName(name) {
    assign(this.filterParams, {
      resourceName: trim(name)
    });
    this.getResource();
  }

  searchByLocation(location) {
    assign(this.filterParams, {
      resourceLocation: trim(location)
    });
    this.getResource();
  }

  searchByProtectedSlaName(slaName) {
    assign(this.filterParams, {
      protectedSlaName: trim(slaName)
    });
    this.getResource();
  }

  openResourceDetail(item) {
    const resource = JSON.parse(item.resourceProperties);
    this.detailService.openDetailModal(
      item.resourceSubType,
      {
        lvHeader: item.name,
        data: assign(
          {
            uuid: item.resourceId,
            ip:
              resource.sub_type === DataMap.Resource_Type.virtualMachine.value
                ? ''
                : resource.environment_endpoint
          },
          item,
          resource,
          { name: item.resourceName }
        ),
        formCopyDataList: true
      },
      () => {
        this.activeItem = {};
        this.cdr.detectChanges();
      }
    );
  }

  getResourceDetail(item) {
    const params = {
      pageSize: CommonConsts.PAGE_SIZE,
      pageNo: CommonConsts.PAGE_START,
      conditions: JSON.stringify({ uuid: item.resourceId })
    };
    this.activeItem = item;
    if (item.resourceSubType === DataMap.Resource_Type.virtualMachine.value) {
      this.virtualResourceService
        .queryResourcesV1VirtualResourceGet(params)
        .subscribe({
          next: res => {
            if (res.items && res.items[0]) {
              this.detailService.openDetailModal(
                item.resourceSubType,
                {
                  data: { ...res.items[0], name: item.resourceName },
                  formCopyDataList: true
                },
                () => {
                  this.activeItem = {};
                  this.cdr.detectChanges();
                }
              );
            } else {
              this.openResourceDetail(item);
            }
          },
          error: err => {
            this.openResourceDetail(item);
          }
        });
      return;
    }
    if (item.resourceSubType === DataMap.Resource_Type.oracle.value) {
      this.databasesService
        .queryResourcesV1DatabasesGet({ ...params, akDoException: false })
        .subscribe({
          next: res => {
            if (res.items && res.items[0]) {
              this.detailService.openDetailModal(
                item.resourceSubType,
                {
                  data: { ...res.items[0], name: item.resourceName },
                  formCopyDataList: true
                },
                () => {
                  this.activeItem = {};
                  this.cdr.detectChanges();
                }
              );
            } else {
              this.openResourceDetail(item);
            }
          },
          error: err => {
            this.openResourceDetail(item);
          }
        });
      return;
    }
    if (item.resourceSubType === DataMap.Resource_Type.Redis.value) {
      this.protectedResourceApiService
        .ShowResource({
          resourceId: item.resourceId,
          akDoException: false
        })
        .subscribe({
          next: (res: any) => {
            if (res?.dependencies?.children) {
              extendSlaInfo(res);
              this.detailService.openDetailModal(
                item.resourceSubType,
                {
                  data: { ...res, name: item.resourceName },
                  formCopyDataList: true
                },
                () => {
                  this.activeItem = {};
                  this.cdr.detectChanges();
                }
              );
            } else {
              this.openResourceDetail(item);
            }
          },
          error: err => {
            this.openResourceDetail(item);
          }
        });
      return;
    }

    this.openResourceDetail(item);
  }

  filterChange(e) {
    if (
      this.childResourceType?.includes('ClickHouse') &&
      e.key === 'resourceSubType'
    ) {
      this.clickHouseTypeFilter = e.value;
    }
    assign(this.filterParams, {
      [e.key]: e.value
    });
    this.getResource();
  }

  sortChange(source) {
    this.orders = [];
    this.orders.push((source.direction === 'asc' ? '+' : '-') + source.key);
    this.getResource();
  }

  isActive(item): boolean {
    return item.resource_id === this.activeItem?.resource_id;
  }
}
