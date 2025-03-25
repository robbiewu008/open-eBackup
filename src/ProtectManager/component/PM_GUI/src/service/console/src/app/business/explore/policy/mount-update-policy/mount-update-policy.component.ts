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
  EventEmitter,
  Input,
  OnInit,
  Output
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  getPermissionMenuItem,
  hasLivemountPolicyPermission,
  I18NService,
  LANGUAGE,
  LiveMountPolicyApiService,
  MODAL_COMMON,
  OperateItems,
  ResourceSetApiService,
  ResourceSetType,
  RetentionPolicy,
  RoleOperationMap,
  SchedulePolicy,
  SYSTEM_TIME,
  WarningMessageService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, cloneDeep, each, isEmpty, map, set, trim } from 'lodash';
import { CreateUpdatePolicyComponent } from './create-update-policy/create-update-policy.component';

@Component({
  selector: 'aui-mount-update-policy',
  templateUrl: './mount-update-policy.component.html',
  styleUrls: ['./mount-update-policy.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class MountUpdatePolicyComponent implements OnInit {
  @Input() isResourceSet = false; // 用于资源集判断
  @Input() allSelectionMap;
  @Input() data;
  @Input() isDetail;
  @Output() allSelectChange = new EventEmitter<any>();

  name;
  filterParams = {};
  tableData = [];
  selection = []; // 只用于资源集
  orders = ['-created_time'];
  schedulePolicy = SchedulePolicy;
  retentionPolicy = RetentionPolicy;
  startPage = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  columns = [
    {
      key: 'name',
      label: this.i18n.get('common_name_label')
    },
    {
      key: 'copyDataSelectionPolicy',
      label: this.i18n.get('common_copy_data_label')
    },
    {
      key: 'scheduleInterval',
      label: this.i18n.get('common_scheduled_label')
    },
    {
      key: 'retentionValue',
      label: this.i18n.get('common_retention_label')
    },
    {
      key: 'liveMountCount',
      label: this.i18n.get('explore_account_of_object_label')
    }
  ];
  isAllSelect = false; // 用来标记是否全选
  buttonLabel = this.i18n.get('system_resourceset_all_select_label');

  spaceLabel = this.i18n.language === LANGUAGE.CN ? '' : ' ';
  executionPeriodLabel = this.i18n.get(
    'protection_execution_period_label',
    [],
    true
  );
  firstExecuteTimeLabel = this.i18n.get(
    'explore_first_execute_label',
    [],
    true
  );

  roleOperationMap = RoleOperationMap;
  timeZone = SYSTEM_TIME.timeZone;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private liveMountPolicyApiService: LiveMountPolicyApiService,
    private cookieService: CookieService,
    public appUtilsService: AppUtilsService,
    public virtualScroll: VirtualScrollService,
    private resourceSetService: ResourceSetApiService,
    private cdr: ChangeDetectorRef
  ) {}

  ngOnInit() {
    if (
      this.isResourceSet &&
      !!this.allSelectionMap[ResourceSetType.LiveMount]?.isAllSelected
    ) {
      this.isAllSelect = true;
    }
    this.getPolicyList();
    this.virtualScroll.getScrollParam(243, 3);
  }

  onChange() {
    this.ngOnInit();
  }

  allSelect(turnPage?) {
    const isAllSelected = !!turnPage ? !this.isAllSelect : this.isAllSelect;
    set(this.allSelectionMap, ResourceSetType.LiveMount, { isAllSelected });
    this.selection = isAllSelected ? [...this.tableData] : [];
    each(this.tableData, item => {
      item.disabled = isAllSelected;
    });
    this.isAllSelect = isAllSelected;
    this.buttonLabel = this.i18n.get(
      isAllSelected
        ? 'system_resourceset_cancel_all_select_label'
        : 'system_resourceset_all_select_label'
    );
    this.allSelectChange.emit();
    this.cdr.detectChanges();
  }

  selectionChange(e) {
    set(this.allSelectionMap, ResourceSetType.LiveMount, {
      data: this.selection
    });
    this.allSelectChange.emit();
  }

  getPolicyList() {
    const params = {
      page: this.startPage,
      size: this.pageSize
    };
    if (!!this.appUtilsService.getCacheValue('jobToMount', false)) {
      this.name = this.appUtilsService.getCacheValue('jobToMount');
      assign(this.filterParams, {
        name: trim(this.name)
      });
    }
    if (this.isResourceSet && this.isDetail) {
      assign(this.filterParams, {
        resourceSetId: this.data[0].uuid
      });
    }
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
    this.liveMountPolicyApiService
      .getPoliciesUsingGET(params)
      .subscribe(res => {
        this.total = res.total;
        this.tableData = res.items;
        if (
          this.isResourceSet &&
          !isEmpty(this.allSelectionMap[ResourceSetType.LiveMount]?.data)
        ) {
          this.selection = cloneDeep(
            this.allSelectionMap[ResourceSetType.LiveMount].data
          );
        }

        if (
          this.isResourceSet &&
          !!this.allSelectionMap[ResourceSetType.LiveMount]?.isAllSelected
        ) {
          this.allSelect(false);
        }

        if (
          this.isResourceSet &&
          !!this.data &&
          isEmpty(this.allSelectionMap[ResourceSetType.Agent]?.data) &&
          !this.isDetail
        ) {
          this.getSelectedData();
        }

        this.cdr.detectChanges();
      });
  }

  getSelectedData() {
    // 用于修改时回显
    const params: any = {
      resourceSetId: this.data[0].uuid,
      scopeModule: ResourceSetType.LiveMount,
      type: ResourceSetType.LiveMount
    };
    this.resourceSetService.queryResourceObjectIdList(params).subscribe(res => {
      set(this.allSelectionMap, ResourceSetType.LiveMount, {
        data: map(res, item => {
          return { policyId: item };
        })
      });
      this.selection = cloneDeep(
        this.allSelectionMap[ResourceSetType.LiveMount].data
      );
      this.allSelectChange.emit();
    });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.startPage = page.pageIndex;
    this.getPolicyList();
  }

  sortChange(source) {
    this.orders = [];
    this.orders.push(
      (source.direction === 'asc' ? '+' : '-') +
        (source.key === 'liveMountCount' ? 'live_mount_count' : source.key)
    );
    this.getPolicyList();
  }

  searchByName(name) {
    assign(this.filterParams, {
      name: trim(name)
    });
    this.getPolicyList();
  }

  optsCallback = item => {
    return this.getOptsItems(item);
  };

  create() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'updating-policy-create',
        lvWidth: MODAL_COMMON.smallModal,
        lvHeader: this.i18n.get('common_create_label'),
        lvContent: CreateUpdatePolicyComponent,
        lvComponentParams: {},
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateUpdatePolicyComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateUpdatePolicyComponent;
            content.onCreate().subscribe({
              next: res => {
                resolve(true);
                this.getPolicyList();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  clone(item) {
    this.liveMountPolicyApiService
      .getPolicyUsingGET({
        policyId: item.policyId
      })
      .subscribe(data => {
        this.drawModalService.create(
          assign({}, MODAL_COMMON.generateDrawerOptions(), {
            lvModalKey: 'updating-policy-clone',
            lvHeader: this.i18n.get('common_clone_label'),
            lvContent: CreateUpdatePolicyComponent,
            lvWidth: MODAL_COMMON.normalWidth,
            lvComponentParams: {
              data
            },
            lvOkDisabled: true,
            lvAfterOpen: modal => {
              const content = modal.getContentComponent() as CreateUpdatePolicyComponent;
              const modalIns = modal.getInstance();
              content.formGroup.statusChanges.subscribe(res => {
                modalIns.lvOkDisabled = res !== 'VALID';
              });
            },
            lvOk: modal => {
              return new Promise(resolve => {
                const content = modal.getContentComponent() as CreateUpdatePolicyComponent;
                content.onCreate().subscribe({
                  next: () => {
                    resolve(true);
                    this.getPolicyList();
                  },
                  error: () => resolve(false)
                });
              });
            }
          })
        );
      });
  }

  modify(item) {
    this.liveMountPolicyApiService
      .getPolicyUsingGET({
        policyId: item.policyId
      })
      .subscribe(data => {
        this.drawModalService.create(
          assign({}, MODAL_COMMON.generateDrawerOptions(), {
            lvModalKey: 'updating-policy-modify',
            lvHeader: this.i18n.get('common_modify_label'),
            lvContent: CreateUpdatePolicyComponent,
            lvWidth: MODAL_COMMON.normalWidth,
            lvComponentParams: {
              data
            },
            lvOkDisabled: true,
            lvAfterOpen: modal => {
              const content = modal.getContentComponent() as CreateUpdatePolicyComponent;
              const modalIns = modal.getInstance();
              content.formGroup.statusChanges.subscribe(res => {
                modalIns.lvOkDisabled = res !== 'VALID';
              });
            },
            lvOk: modal => {
              return new Promise(resolve => {
                const content = modal.getContentComponent() as CreateUpdatePolicyComponent;
                content.onModify().subscribe({
                  next: () => {
                    resolve(true);
                    this.getPolicyList();
                  },
                  error: () => resolve(false)
                });
              });
            }
          })
        );
      });
  }

  delete(item) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_updating_policy_delete_label', [
        item.name
      ]),
      onOK: () => {
        this.liveMountPolicyApiService
          .deletePolicyUsingDELETE({ policyId: item.policyId })
          .subscribe(res => this.getPolicyList());
      }
    });
  }

  getOptsItems(item) {
    const menus = [
      {
        id: 'clone',
        label: this.i18n.get('common_clone_label'),
        permission: OperateItems.CloneUpdatingPolicy,
        disabled: !hasLivemountPolicyPermission(item),
        onClick: () => this.clone(item)
      },
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyUpdatingPolicy,
        disabled: !hasLivemountPolicyPermission(item),
        onClick: () => this.modify(item)
      },
      {
        id: 'delete',
        disabled:
          (item.liveMountCount && item.liveMountCount > 0) ||
          !hasLivemountPolicyPermission(item),
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteUpdatingPolicy,
        onClick: () => this.delete(item)
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  }
}
