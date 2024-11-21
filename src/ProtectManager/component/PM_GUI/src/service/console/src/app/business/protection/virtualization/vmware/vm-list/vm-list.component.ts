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
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  SimpleChanges,
  ViewChild,
  OnChanges,
  ChangeDetectorRef
} from '@angular/core';
import { DatatableComponent } from '@iux/live';
import {
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  DATE_PICKER_MODE,
  getPermissionMenuItem,
  GROUP_COMMON,
  hasBackupPermission,
  hasProtectPermission,
  hasRecoveryPermission,
  hasResourcePermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProjectedObjectApiService,
  ProtectResourceAction,
  ProtectResourceCategory,
  ResourceOperationType,
  ResourceType,
  SetTagType,
  VirtualResourceService,
  WarningMessageService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import {
  assign,
  clone,
  each,
  filter,
  find,
  forEach,
  includes,
  isArray,
  isEmpty,
  isUndefined,
  mapValues,
  size,
  some,
  toString,
  trim
} from 'lodash';
import { SummaryComponent } from '../summary/summary.component';
import { RememberColumnsService } from 'app/shared/services/remember-columns.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { WarningBatchConfirmsService } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';

@Component({
  selector: 'aui-vm-list',
  templateUrl: './vm-list.component.html',
  styleUrls: ['./vm-list.component.less']
})
export class VmListComponent implements OnInit, OnChanges {
  @Input() path;
  @Input() tab;
  @Input() tableData;
  @Input() treeSelection;

  ResourceType = ResourceType;
  DataMap = DataMap;

  groupCommon = GROUP_COMMON;
  activeItem;

  activeSort;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  columns;
  searchPlaceHolder;
  filterParams = {
    tabType: ResourceType.VM,
    source: {},
    orders: null,
    paginator: {
      pageSize: this.pageSize,
      pageIndex: this.pageIndex
    }
  };

  selection = [];
  queryUuid;
  queryName;
  queryPath;
  queryIp;
  querySlaName;
  queryTags;

  refreshDetailData;
  deleteBtnDisabled = true;
  protectBtnDisabled = true;
  moreMenus = [];

  tableColumnKey = '';
  disableProtectionTip = '';

  currentDetailItemUuid;

  @Output() updateTable = new EventEmitter();

  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private detailService: ResourceDetailService,
    private protectService: ProtectService,
    private takeManualBackupService: TakeManualBackupService,
    private projectedObjectApiService: ProjectedObjectApiService,
    private virtualResourceService: VirtualResourceService,
    private warningMessageService: WarningMessageService,
    private slaService: SlaService,
    private cookieService: CookieService,
    private rememberColumnsService: RememberColumnsService,
    public virtualScroll: VirtualScrollService,
    private warningBatchConfirmsService: WarningBatchConfirmsService,
    private cdr: ChangeDetectorRef,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngOnInit() {
    this.getMoreMenus();
    this.virtualScroll.getScrollParam(400);
  }

  getMoreMenus() {
    const menus = [
      {
        id: 'batchRemoveProtect',
        label: this.i18n.get('protection_remove_protection_label'),
        disabled:
          !size(this.selection) ||
          !isUndefined(find(this.selection, data => !data.sla_id)) ||
          !isUndefined(find(this.selection, data => data.in_group === 'True')),
        permission: OperateItems.RemoveVMProtection,
        onClick: () => this.batchRemoveProtect(this.selection)
      },
      {
        id: 'activeProtection',
        label: this.i18n.get('protection_active_protection_label'),
        permission: OperateItems.ActivateVMProtection,
        tips: '',
        disabled:
          !size(this.selection) ||
          !isUndefined(
            find(this.selection, data => !(data.sla_id && !data.sla_status))
          ) ||
          !isUndefined(find(this.selection, data => data.in_group === 'True')),
        onClick: () => this.activeProtection(this.selection)
      },
      {
        id: 'deactiveProtection',
        label: this.i18n.get('protection_deactive_protection_label'),
        permission: OperateItems.DeactivateVMProtection,
        tips: '',
        disabled:
          !size(this.selection) ||
          !isUndefined(
            find(this.selection, data => !(data.sla_id && data.sla_status))
          ) ||
          !isUndefined(find(this.selection, data => data.in_group === 'True')),
        onClick: () => this.deactiveProtection(this.selection)
      },
      {
        id: 'manualBackup',
        disabled:
          size(
            filter(this.selection, val => {
              return !isEmpty(val.sla_id);
            })
          ) !== size(this.selection) || !size(this.selection),
        label: this.i18n.get('common_manual_backup_label'),
        permission: OperateItems.ManuallyBackVM,
        onClick: () => this.manualBackup(this.selection)
      },
      {
        id: 'addTag',
        permission: OperateItems.AddTag,
        disabled:
          !size(this.selection) ||
          some(this.selection, v => !hasResourcePermission(v)),
        label: this.i18n.get('common_add_tag_label'),
        onClick: () => this.addTag(this.selection)
      },
      {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        disabled:
          !size(this.selection) ||
          some(this.selection, v => !hasResourcePermission(v)),
        label: this.i18n.get('common_remove_tag_label'),
        onClick: () => this.removeTag(this.selection)
      }
    ];
    this.moreMenus = getPermissionMenuItem(menus, this.cookieService.role);
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data,
      type: SetTagType.Resource,
      onOk: () => {
        this.refresh();
      }
    });
  }

  removeTag(data) {
    this.setResourceTagService.setTag({
      isAdd: false,
      rowDatas: data,
      type: SetTagType.Resource,
      onOk: () => {
        this.refresh();
      }
    });
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.tab) {
      this.columns = [
        {
          key: 'uuid',
          label: this.i18n.get('protection_resource_id_label'),
          isShow: false
        },
        {
          key: 'name',
          label: this.i18n.get('common_name_label'),
          isShow: true
        },
        {
          key: 'path',
          label: this.i18n.get('common_location_label'),
          hidden:
            this.tab.id === ResourceType.CLUSTER ||
            (this.tab.id === ResourceType.HOST &&
              this.tab.resType === ResourceType.HYPERV),
          isShow: true
        },
        {
          key: 'endpoint',
          label: this.i18n.get('common_ip_address_label'),
          hidden: !(
            this.tab.id === ResourceType.HOST &&
            this.tab.resType === ResourceType.HYPERV
          ),
          isShow: true
        },
        {
          key: 'link_status',
          showFilter: true,
          width: '120px',
          label: this.i18n.get('common_status_label'),
          filterMap: this.dataMapService.toArray(
            this.tab.id === ResourceType.VM &&
              this.tab.resType === ResourceType.VM
              ? 'vm_LinkStatus'
              : 'resource_LinkStatus'
          ),
          hidden:
            this.tab.id === ResourceType.CLUSTER ||
            (this.tab.resType === ResourceType.HYPERV &&
              this.tab.id === ResourceType.VM),
          isShow: true
        },
        {
          key: 'os_type',
          label: this.i18n.get('protection_os_type_label'),
          hidden:
            this.tab.id === ResourceType.CLUSTER ||
            this.tab.id === ResourceType.HOST,
          isShow: true,
          showFilter: true,
          filterMap: this.dataMapService.toArray('vmwareOsType')
        },
        {
          key: 'tags',
          label: this.i18n.get('common_mark_label'),
          hidden: !(
            this.tab.id === ResourceType.VM &&
            this.tab.resType === ResourceType.VM
          ),
          isShow: true
        },
        {
          key: 'sla_name',
          label: this.i18n.get('common_sla_label'),
          isShow: true
        },
        {
          key: 'sla_compliance',
          hidden: this.tab.id !== ResourceType.VM,
          width: '140px',
          showFilter: true,
          label: this.i18n.get('common_sla_compliance_label'),
          filterMap: this.dataMapService.toArray('Sla_Compliance'),
          isShow: true
        },
        {
          key: 'protection_status',
          label: this.i18n.get('protection_protected_status_label'),
          showFilter: true,
          filterMap: this.dataMapService
            .toArray('Protection_Status')
            .filter(item => {
              if (item.value === this.tab.protectionStatus) {
                item.selected = true;
              }
              return true;
            }),
          isShow: true
        },
        {
          key: 'children',
          label: this.i18n.get('protection_vms_label'),
          align: 'right',
          hidden: this.tab.id === ResourceType.VM,
          isShow: true
        },
        {
          key: 'resource_group_name',
          label: this.i18n.get('protection_vm_group_label'),
          hidden: this.tab.id !== ResourceType.VM,
          isShow: true
        },
        {
          key: 'labelList',
          label: this.i18n.get('common_tag_label'),
          isShow: true
        }
      ];

      this.filterParams.tabType = this.tab.id;
    }
    if (changes.tableData && this.refreshDetailData) {
      if (
        includes(mapValues(this.drawModalService.modals, 'key'), 'detail-modal')
      ) {
        this.refreshDetail(this.refreshDetailData, this.tab.tableData);
      } else {
        this.refreshDetailData = null;
      }
    }
    // 左树切换，右侧主列表功能重置
    if (
      changes.path &&
      this.lvTable &&
      !isUndefined(changes.path.previousValue)
    ) {
      this.queryName = '';
      this.querySlaName = '';
      this.queryUuid = '';
      this.queryIp = '';
      this.filterParams.source = {};
      this.lvTable.removeFilter();
      this.columns.forEach(col => {
        if (col.filterMap && !col.hidden) {
          col.filterMap.forEach(filter => {
            filter['selected'] = false;
          });
          col.filterMap = [...col.filterMap];
        }
      });
      this.lvTable.removeSort();
      this.activeSort = null;
      this.selection = [];
      this.deleteBtnDisabled = this.protectBtnDisabled = true;
      each(this.moreMenus, item => {
        item.disabled = true;
      });
    }
    this.tableColumnKey = `protection_${this.tab.resType}_${this.tab.id}_table`;
    const columnStatus = this.rememberColumnsService.getColumnsStatus(
      this.tableColumnKey
    );
    if (!isEmpty(columnStatus)) {
      each(this.columns, col => {
        col.isShow = columnStatus[col.key];
      });
    }
  }

  selectionChange(source) {
    each(this.moreMenus, item => {
      if (!size(this.selection)) {
        item.tips = '';
        return (item.disabled = true);
      }
      if (item.id === 'batchRemoveProtect') {
        item.disabled =
          size(
            filter(this.selection, val => {
              return (
                (!isEmpty(val.sla_id) ||
                  val.protection_status ===
                    DataMap.Protection_Status.protected.value) &&
                hasProtectPermission(val)
              );
            })
          ) !== size(this.selection) || !size(this.selection);
      } else if (item.id === 'activeProtection') {
        item.disabled =
          size(
            filter(this.selection, val => {
              return (
                !isEmpty(val.sla_id) &&
                !val.sla_status &&
                hasProtectPermission(val)
              );
            })
          ) !== size(this.selection);
        item.tips = item.disabled
          ? this.i18n.get('protection_partial_resources_active_label')
          : '';
      } else if (item.id === 'deactiveProtection') {
        item.disabled =
          size(
            filter(this.selection, val => {
              return (
                !isEmpty(val.sla_id) &&
                val.sla_status &&
                hasProtectPermission(val)
              );
            })
          ) !== size(this.selection);
        item.tips = item.disabled
          ? this.i18n.get('protection_partial_resources_deactive_label')
          : '';
      } else if (item.id === 'addTag') {
        item.disabled =
          !size(this.selection) ||
          some(this.selection, v => !hasResourcePermission(v));
      } else if (item.id === 'removeTag') {
        item.disabled =
          !size(this.selection) ||
          some(this.selection, v => !hasResourcePermission(v));
      } else {
        item.disabled =
          size(
            filter(this.selection, val => {
              return (
                !isEmpty(val.sla_id) &&
                !this.getVCenterOffine() &&
                hasBackupPermission(val)
              );
            })
          ) !== size(this.selection);
      }
    });
    // 批量删除按钮
    this.protectBtnDisabled =
      size(
        filter(this.selection, val => {
          return (
            isEmpty(val.sla_id) &&
            !this.getHypervOffline(val) &&
            val.protection_status !==
              DataMap.Protection_Status.creating.value &&
            val.protection_status !==
              DataMap.Protection_Status.protected.value &&
            hasResourcePermission(val)
          );
        })
      ) !== size(this.selection) ||
      !isUndefined(find(this.selection, data => data.in_group === 'True')) ||
      !size(this.selection) ||
      size(this.selection) > 100;
    this.deleteBtnDisabled =
      size(
        filter(this.selection, val => {
          return !isEmpty(val.sla_id);
        })
      ) !== size(this.selection) || !size(this.selection);
    this.disableProtectionTip =
      size(this.selection) > 100 && this.tab.id === ResourceType.VM
        ? this.i18n.get('protection_disable_protection_label')
        : '';
  }

  // okCallBack 主机集群详情页面里的虚拟机表格的操作完成后需更新虚拟机表格的数据
  optsCallback = (data, okCallBack?) => {
    return this.getOptsItems(data, okCallBack);
  };

  getOptsItems(data, okCallBack?) {
    const isVCenterOffine = this.getVCenterOffine();
    const isHyperHostOffline = this.getHypervOffline(data);
    const menus = [
      {
        id: 'protect',
        disabled:
          data.sla_id ||
          isHyperHostOffline ||
          isVCenterOffine ||
          data.protection_status === DataMap.Protection_Status.creating.value ||
          data.protection_status ===
            DataMap.Protection_Status.protected.value ||
          data.in_group === 'True' ||
          !hasProtectPermission(data),
        label: this.i18n.get('common_protect_label'),
        permission: OperateItems.ProtectVM,
        onClick: () => this.protect(data, okCallBack)
      },
      {
        id: 'modifyProtection',
        disabled:
          !data.sla_id ||
          isHyperHostOffline ||
          isVCenterOffine ||
          data.in_group === 'True' ||
          !hasProtectPermission(data),
        label: this.i18n.get('protection_modify_protection_label'),
        permission: OperateItems.ModifyVMProtection,
        onClick: () => this.protect(data, okCallBack, true)
      },
      {
        id: 'removeProtection',
        divide: true,
        disabled:
          (!data.sla_id &&
            data.protection_status !==
              DataMap.Protection_Status.protected.value) ||
          data.in_group === 'True' ||
          !hasProtectPermission(data),
        label: this.i18n.get('protection_remove_protection_label'),
        permission: OperateItems.RemoveVMProtection,
        onClick: () => this.removeProtection(data, okCallBack)
      },
      {
        id: 'activeProtection',
        disabled:
          !(data.sla_id && !data.sla_status) ||
          data.in_group === 'True' ||
          !hasProtectPermission(data),
        label: this.i18n.get('protection_active_protection_label'),
        permission: OperateItems.ActivateVMProtection,
        onClick: () => this.activeProtection(data, okCallBack)
      },
      {
        id: 'deactiveProtection',
        divide: true,
        disabled:
          !(data.sla_id && data.sla_status) ||
          data.in_group === 'True' ||
          !hasProtectPermission(data),
        label: this.i18n.get('protection_deactive_protection_label'),
        permission: OperateItems.DeactivateVMProtection,
        onClick: () => this.deactiveProtection(data, okCallBack)
      },
      {
        id: 'recovery',
        disabled: !hasRecoveryPermission(data),
        hidden: this.tab.id !== ResourceType.VM,
        label: this.i18n.get('common_restore_label'),
        permission: OperateItems.RestoreCopy,
        onClick: () =>
          this.getDetail({
            ...data,
            activeId: 'copydata',
            datePickerMode: DATE_PICKER_MODE.DATE
          })
      },
      {
        id: 'manualBackup',
        disabled:
          !data.sla_id ||
          isHyperHostOffline ||
          isVCenterOffine ||
          !hasBackupPermission(data),
        label: this.i18n.get('common_manual_backup_label'),
        permission: OperateItems.ManuallyBackVM,
        divide: true,
        onClick: () => this.manualBackup([data], okCallBack)
      },
      {
        id: 'addTag',
        permission: OperateItems.AddTag,
        disabled: !hasResourcePermission(data),
        label: this.i18n.get('common_add_tag_label'),
        onClick: () => this.addTag([data])
      },
      {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        disabled: !hasResourcePermission(data),
        label: this.i18n.get('common_remove_tag_label'),
        onClick: () => this.removeTag([data])
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  }

  getHypervOffline(data) {
    return (
      this.tab.resType === ResourceType.HYPERV &&
      data.link_status === DataMap.resource_LinkStatus.offline.value
    );
  }

  getVCenterOffine() {
    return (
      this.tab.resType === ResourceType.VM &&
      includes([ResourceType.HOST, ResourceType.CLUSTER], this.tab.id) &&
      this.treeSelection &&
      includes(
        [DataMap.Resource_Type.vmwareVcenterServer.value],
        this.treeSelection.rootNodeSubType
      ) &&
      this.treeSelection.rootNodeLinkStatus ===
        DataMap.resource_LinkStatus.offline.value
    );
  }

  refresh() {
    this.selection = [];
    this.deleteBtnDisabled = this.protectBtnDisabled = true;
    this.disableProtectionTip = '';
    each(this.moreMenus, item => {
      item.disabled = true;
    });
    this.updateTable.emit(this.filterParams);
  }

  refreshDetail(target, tableData) {
    if (find(tableData, { uuid: target.uuid })) {
      this.getDetail(find(tableData, { uuid: target.uuid }));
    } else {
      this.drawModalService.destroyModal('detail-modal');
    }
  }

  getDetail(item) {
    if (
      includes(
        mapValues(this.drawModalService.modals, 'key'),
        'slaDetailModalKey'
      )
    ) {
      this.drawModalService.destroyModal('slaDetailModalKey');
    }
    this.viewDetail(item);
    this.refreshDetailData = null;
  }

  searchByUuid(uuid) {
    assign(this.filterParams.source, { uuid: trim(uuid) });
    this.refresh();
  }

  searchByName(name) {
    assign(this.filterParams.source, { name: trim(name) });
    this.refresh();
  }

  searchByPath(path) {
    assign(this.filterParams.source, { path: trim(path) });
    this.refresh();
  }

  searchByTags(tags) {
    assign(this.filterParams.source, {
      tags: this.encodeChar(trim(tags))
    });
    this.refresh();
  }

  searchByLabel(label) {
    assign(this.filterParams.source, {
      labelName: trim(label)
    });
    this.refresh();
  }

  encodeChar(inputStr) {
    let newStr = '';
    const str = encodeURIComponent(inputStr);
    each(str, char => {
      // tslint:disable-next-line: quotemark
      if (includes(['!', '(', ')', "'"], char)) {
        newStr = newStr + escape(char);
      } else if (includes(['*'], char)) {
        newStr = newStr + '%2A';
      } else {
        newStr = newStr + char;
      }
    });

    return newStr;
  }

  searchByIp(ip) {
    assign(this.filterParams.source, { endpoint: trim(ip) });
    this.refresh();
  }

  searchBySlaName(slaName) {
    assign(this.filterParams.source, { sla_name: trim(slaName) });
    this.refresh();
  }

  pageChange(source) {
    this.pageIndex = source.pageIndex;
    this.pageSize = source.pageSize;
    assign(this.filterParams.paginator, {
      pageSize: this.pageSize,
      pageIndex: this.pageIndex
    });
    // 分页不重置selection
    this.updateTable.emit(this.filterParams);
  }

  filterChange(e) {
    assign(this.filterParams.source, { [e.key]: e.value });
    if (
      e.key === 'sla_compliance' &&
      includes(e.value, true) &&
      includes(e.value, false)
    ) {
      delete this.filterParams.source['sla_compliance'];
    }
    this.refresh();
  }

  sortChange(e) {
    if (e.direction) {
      this.filterParams.orders = [
        `${e.direction === 'asc' ? '+' : '-'}${e.key}`
      ];
    } else {
      this.filterParams.orders = null;
    }
    this.refresh();
  }

  getSlaDetail(item) {
    this.slaService.getDetail({ uuid: item.sla_id, name: item.sla_name });
  }

  protect(datas, okCallBack?, isModify?) {
    const currentType = okCallBack ? datas.type : this.tab.id;
    if (
      isModify &&
      currentType !== ResourceType.VM &&
      this.tab.resType === ResourceType.VM
    ) {
      datas.ext_parameters['disk_info'] =
        datas.ext_parameters.disk_filters[0].values;
    }
    const conditions = {
      path: this.path,
      type: currentType
    };
    if (isArray(datas)) {
      datas[0]['conditions'] = conditions;
      datas[0]['resType'] = this.tab.resType;
    } else {
      datas['conditions'] = conditions;
      datas['resType'] = this.tab.resType;
    }

    this.protectService.openProtectModal(
      {
        [ResourceType.VM]: isArray(datas)
          ? ProtectResourceCategory.vmwares
          : ProtectResourceCategory.vmware,
        [ResourceType.HOST]: ProtectResourceCategory.esix,
        [ResourceType.CLUSTER]: ProtectResourceCategory.cluster
      }[currentType],

      isModify ? ProtectResourceAction.Modify : ProtectResourceAction.Create,
      {
        data: datas,
        width: 780,
        onOK: () => {
          this.optOnOkFunc(okCallBack, datas);
        }
      }
    );
  }

  optOnOkFunc(okCallBack, datas) {
    if (okCallBack) {
      okCallBack();
    } else {
      this.refresh();
      this.refreshDetailData = isArray(datas) ? null : datas;
    }
  }

  activeProtection(datas, okCallBack?) {
    const resource_ids = [];
    forEach(isArray(datas) ? datas : [datas], data => {
      resource_ids.push(data.uuid);
    });
    this.projectedObjectApiService
      .activeV1ProtectedObjectsStatusActionActivatePut({
        body: {
          resource_ids
        }
      })
      .subscribe({
        next: () => this.optOnOkFunc(okCallBack, datas),
        error: () => (this.selection = [])
      });
  }

  batchRemoveProtect(selection) {
    if (selection.length === 1) {
      this.removeProtection(selection);
    } else {
      this.warningBatchConfirmsService.create({
        data: selection,
        selection: clone(selection),
        message: this.i18n.get(
          'protection_resource_batch_delete_protect_label'
        ),
        operationType: ResourceOperationType.protection,
        onOK: () => this.optOnOkFunc(null, selection)
      });
    }
  }

  removeProtection(datas, okCallBack?) {
    const resource_ids = [];
    const resource_names = [];
    forEach(isArray(datas) ? datas : [datas], data => {
      resource_ids.push(data.uuid);
      resource_names.push(data.name);
    });
    this.warningMessageService.create({
      content: this.i18n.get(
        this.tab.id === ResourceType.VM
          ? 'protection_resource_delete_protect_label'
          : 'protection_resource_delete_vm_protect_label',
        [toString(resource_names)]
      ),
      onOK: () => {
        this.projectedObjectApiService
          .deleteV1ProtectedObjectsDelete({
            body: {
              resource_ids
            }
          })
          .subscribe({
            next: () => {
              this.optOnOkFunc(okCallBack, datas);
            },
            error: () => (this.selection = [])
          });
      }
    });
  }

  deactiveProtection(datas, okCallBack?) {
    const resource_ids = [];
    const resource_names = [];
    forEach(isArray(datas) ? datas : [datas], data => {
      resource_ids.push(data.uuid);
      resource_names.push(data.name);
    });
    this.warningMessageService.create({
      content: this.i18n.get(
        this.tab.id === ResourceType.VM
          ? 'protection_deactivate_resource_tip_label'
          : 'protection_deactivate_vm_resource_tip_label',
        [toString(resource_names)]
      ),
      onOK: () => {
        this.projectedObjectApiService
          .deactivateV1ProtectedObjectsStatusActionDeactivatePut({
            body: {
              resource_ids
            }
          })
          .subscribe({
            next: () => this.optOnOkFunc(okCallBack, datas),
            error: () => (this.selection = [])
          });
      }
    });
  }

  manualBackup(datas, okCallBack?) {
    if (size(datas) > 1) {
      each(datas, item => {
        assign(item, {
          host_ip: item.environment_endpoint,
          resource_id: item.uuid,
          resource_type: datas[0].sub_type
        });
      });
      this.takeManualBackupService.batchExecute(datas, () =>
        this.optOnOkFunc(okCallBack, datas)
      );
    } else {
      assign(datas[0], {
        host_ip: datas[0].environment_endpoint,
        resource_id: datas[0].uuid,
        resource_type: datas[0].sub_type
      });
      this.takeManualBackupService.execute(datas[0], () =>
        this.optOnOkFunc(okCallBack, datas)
      );
    }
  }

  viewDetail(item) {
    this.currentDetailItemUuid = item.uuid;
    this.activeItem = item;
    if (this.tab.id === ResourceType.VM) {
      this.detailService.openDetailModal(
        DataMap.Resource_Type.virtualMachine.value,
        {
          data: assign({}, item, {
            optItems: this.getOptsItems(item),
            optItemsFn: v => {
              return this.getOptsItems(v);
            }
          })
        },
        () => {
          this.activeItem = {};
          this.cdr.detectChanges();
        }
      );
    } else {
      this.drawModalService.openDetailModal({
        ...MODAL_COMMON.drawerOptions,
        lvModalKey: 'detail-modal',
        lvOkDisabled: true,
        lvContent: SummaryComponent,
        lvComponentParams: {
          item: {
            ...item,
            resType: this.tab.resType,
            optItemFunc: this.optsCallback,
            optItems: this.getOptsItems(item),
            name: item.name,
            optItemsFn: v => {
              return this.getOptsItems(v);
            }
          },
          type: this.tab.id
        },
        lvWidth: 1150,
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
      });
    }
  }

  trackByUuid(index: number, list: any) {
    return list.uuid;
  }

  isActive(item): boolean {
    return item.uuid === this.activeItem?.uuid;
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.root_uuid)
    );
  }
}
