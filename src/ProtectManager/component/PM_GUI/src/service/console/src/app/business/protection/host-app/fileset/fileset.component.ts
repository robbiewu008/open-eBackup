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
  OnDestroy,
  OnInit
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  DATE_PICKER_MODE,
  extendSlaInfo,
  getLabelList,
  getPermissionMenuItem,
  GlobalService,
  GROUP_COMMON,
  hasBackupPermission,
  hasProtectPermission,
  hasRecoveryPermission,
  hasResourcePermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProjectedObjectApiService,
  ProtectedResourceApiService,
  ProtectResourceAction,
  ProtectResourceCategory,
  ResourceOperationType,
  RoleOperationMap,
  SetTagType,
  WarningMessageService
} from 'app/shared';
import { WarningBatchConfirmsService } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { RememberColumnsService } from 'app/shared/services/remember-columns.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  clone,
  cloneDeep,
  each,
  filter,
  find,
  first,
  forEach,
  get,
  includes,
  isArray,
  isEmpty,
  isUndefined,
  map,
  mapValues,
  size,
  some,
  toString,
  trim
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { CreateFilesetTemplateComponent } from './fileset-template-list/create-fileset-template/create-fileset-template.component';

@Component({
  selector: 'aui-fileset',
  templateUrl: './fileset.component.html',
  styleUrls: ['./fileset.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class FilesetComponent implements OnInit, OnDestroy {
  queryUuid;
  queryName;
  queryTemplate;
  queryHostName;
  querySlaName;
  protectBtnTooltip;
  queryIp;
  orders = [];
  selection = [];
  tableData = [];
  filterParams: any = {
    subType: [DataMap.Resource_Type.fileset.value]
  };

  deleteBtnDisabled = true;
  protectBtnDisabled = true;
  protectResourceAction = ProtectResourceAction;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  moreMenus = [];
  columns = [
    {
      label: this.i18n.get('protection_resource_id_label'),
      key: 'uuid',
      isShow: false
    },
    {
      label: this.i18n.get('common_name_label'),
      key: 'name',
      isShow: true
    },
    {
      label: this.i18n.get('protection_host_name_label'),
      key: 'environment_name',
      isShow: true
    },
    {
      label: this.i18n.get('common_ip_address_label'),
      key: 'environment_endpoint',
      isShow: true
    },
    {
      label: this.i18n.get('protection_os_type_label'),
      key: 'osType',
      filter: true,
      filterMap: this.dataMapService.toArray('Os_Type').filter(item => {
        return includes(
          [
            DataMap.Os_Type.windows.value,
            DataMap.Os_Type.linux.value,
            DataMap.Os_Type.aix.value,
            DataMap.Os_Type.solaris.value
          ],
          item.value
        );
      }),
      isShow: true
    },
    {
      label: this.i18n.get('protection_volume_advanced_backup_label'),
      key: 'osBackup',
      filter: true,
      filterMap: this.dataMapService.toArray('copyDataVolume'),
      isShow: true
    },
    {
      label: this.i18n.get('protection_associate_template_label'),
      key: 'template_name',
      isShow: true
    },
    {
      label: this.i18n.get('common_sla_label'),
      key: 'sla_name',
      isShow: true
    },
    {
      label: this.i18n.get('common_sla_compliance_label'),
      key: 'sla_compliance',
      filter: true,
      filterMap: this.dataMapService.toArray('Sla_Compliance'),
      isShow: true
    },
    {
      label: this.i18n.get('protection_protected_status_label'),
      key: 'protection_status',
      filter: true,
      filterMap: this.dataMapService.toArray('Protection_Status'),
      isShow: true
    },
    // 新增标签
    {
      key: 'labelList',
      label: this.i18n.get('common_tag_label'),
      isShow: true
    }
  ];
  optItems = [];
  tableColumnKey = 'protection_fileset_table';
  columnStatus = this.rememberColumnsService.getColumnsStatus(
    this.tableColumnKey
  );
  currentDetailItemUuid;
  filesetListSub$: Subscription;
  destroy$ = new Subject();

  groupCommon = GROUP_COMMON;
  activeItem;

  roleOperationMap = RoleOperationMap;

  registerTipShow = false;

  constructor(
    public i18n: I18NService,
    private slaService: SlaService,
    private detailService: ResourceDetailService,
    private drawModalService: DrawModalService,
    private protectService: ProtectService,
    private dataMapService: DataMapService,
    public globalService: GlobalService,
    public warningMessageService: WarningMessageService,
    public projectedObjectApiService: ProjectedObjectApiService,
    private takeManualBackupService: TakeManualBackupService,
    public batchOperateService: BatchOperateService,
    public cookieService: CookieService,
    public rememberColumnsService: RememberColumnsService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private warningBatchConfirmsService: WarningBatchConfirmsService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.getColumnStatus();
    this.getFilesets();
    this.getMoreMenus();
    this.virtualScroll.getScrollParam(400);
    this.getUserGuideState();
    this.showRegisterTip();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  getUserGuideState() {
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        this.showRegisterTip();
      });
  }

  showRegisterTip() {
    if (USER_GUIDE_CACHE_DATA.active && USER_GUIDE_CACHE_DATA.showTips) {
      setTimeout(() => {
        this.registerTipShow = true;
        USER_GUIDE_CACHE_DATA.showTips = false;
        this.cdr.detectChanges();
      });
    }
  }

  lvPopoverBeforeClose = () => {
    this.registerTipShow = false;
    this.cdr.detectChanges();
  };

  getColumnStatus() {
    if (!isEmpty(this.columnStatus)) {
      each(this.columns, col => {
        col.isShow = this.columnStatus[col.key];
      });
    }
  }

  getMoreMenus() {
    const menus = [
      {
        id: 'batchModifyProtect',
        label: this.i18n.get('protection_modify_protection_label'),
        disabled:
          !size(this.selection) ||
          !isUndefined(
            find(
              this.selection,
              data =>
                !data.sla_id ||
                data.protection_status !==
                  DataMap.Protection_Status.protected.value
            )
          ),
        permission: OperateItems.ModifyFilesetProtection,
        onClick: () =>
          this.protect(
            this.selection,
            ProtectResourceAction.Modify,
            this.i18n.get('protection_modify_protection_label')
          )
      },
      {
        id: 'batchRemoveProtect',
        label: this.i18n.get('protection_remove_protection_label'),
        disabled:
          !size(this.selection) ||
          !isUndefined(
            find(
              this.selection,
              data =>
                !data.sla_id ||
                data.protection_status !==
                  DataMap.Protection_Status.protected.value
            )
          ),
        permission: OperateItems.RemoveFilesetProtection,
        onClick: () => this.batchRemoveProtect(this.selection)
      },
      {
        id: 'activeProtection',
        label: this.i18n.get('protection_active_protection_label'),
        tips: '',
        disabled:
          !size(this.selection) ||
          !isUndefined(
            find(this.selection, data => !(data.sla_id && !data.sla_status))
          ),
        permission: OperateItems.ActivateFilesetProtection,
        onClick: () => this.activeProtection(this.selection)
      },
      {
        id: 'deactiveProtection',
        label: this.i18n.get('protection_deactive_protection_label'),
        tips: '',
        disabled:
          !size(this.selection) ||
          !isUndefined(
            find(this.selection, data => !(data.sla_id && data.sla_status))
          ),
        permission: OperateItems.DeactivateFilesetProtection,
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
        permission: OperateItems.ManuallyBackDatabase,
        onClick: () => this.manualBackup(this.selection)
      },
      {
        id: 'deleteFileset',
        label: this.i18n.get('common_delete_label'),
        tips: '',
        disabled:
          size(
            filter(this.selection, val => {
              return (
                isEmpty(val.sla_id) &&
                val.protection_status !==
                  DataMap.Protection_Status.creating.value
              );
            })
          ) !== size(this.selection) || !size(this.selection),
        permission: OperateItems.DeleteHostFileset,
        onClick: () => this.deleteFileset(this.selection)
      },
      {
        id: 'addTag',
        permission: OperateItems.AddTag,
        disabled:
          !size(this.selection) ||
          some(this.selection, v => !hasResourcePermission(v)),
        label: this.i18n.get('common_add_tag_label'),
        onClick: data => this.addTag(this.selection)
      },
      {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        disabled:
          !size(this.selection) ||
          some(this.selection, v => !hasResourcePermission(v)),
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(this.selection)
      }
    ];
    this.moreMenus = getPermissionMenuItem(menus, this.cookieService.role);
  }

  refresh() {
    this.getFilesets();
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data,
      type: SetTagType.Resource,
      onOk: () => {
        this.selection = [];
        this.getFilesets();
      }
    });
  }

  removeTag(data) {
    this.setResourceTagService.setTag({
      isAdd: false,
      rowDatas: data,
      type: SetTagType.Resource,
      onOk: () => {
        this.selection = [];
        this.getFilesets();
      }
    });
  }

  getFilesets(refreshData?, name?, keepSelection?) {
    if (!keepSelection) {
      each(this.moreMenus, item => {
        item.disabled = true;
      });
    }
    const params = this.getParams(name);
    if (this.filesetListSub$) {
      this.filesetListSub$.unsubscribe();
    }
    let manualRefresh = true;
    this.filesetListSub$ = timer(0, CommonConsts.TIME_INTERVAL_RESOURCE)
      .pipe(
        switchMap(index => {
          manualRefresh = !index;
          return this.protectedResourceApiService.ListResources({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        each(res.records, item => {
          // 获取标签数据
          const { showList, hoverList } = getLabelList(item);

          assign(item, {
            sub_type: item.subType,
            environment_name: item.environment?.name,
            environment_endpoint: item.environment?.endpoint,
            template_name: item.extendInfo?.templateName,
            showLabelList: showList,
            hoverLabelList: hoverList,
            osBackup: get(item.extendInfo, 'is_OS_backup', 'false') === 'true'
          });
          extendSlaInfo(item);
        });
        this.tableData = res.records;
        this.total = res.totalCount;
        this.selectionChange(undefined);
        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'detail-modal'
          ) &&
          refreshData &&
          manualRefresh
        ) {
          this.refreshDetail(refreshData, res.records);
        }
        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'detail-modal'
          ) &&
          this.currentDetailItemUuid
        ) {
          this.globalService.emitStore({
            action: 'autoReshResource',
            state: find(res.records, { uuid: this.currentDetailItemUuid })
          });
        }
        this.cdr.detectChanges();
      });
  }

  searchFilesets(name) {
    this.getFilesets(null, name);
  }
  // 刷新资源详情
  refreshDetail(target, tableData) {
    if (find(tableData, { uuid: target.uuid })) {
      this.getDetail(find(tableData, { uuid: target.uuid }));
    } else {
      this.drawModalService.destroyModal('detail-modal');
    }
  }

  getParams(name?) {
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize
    };
    if (!isUndefined(name)) {
      this.queryName = name;
      assign(this.filterParams, {
        name: [['~~'], trim(this.queryName)]
      });
    }
    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
      if (includes(['templateName'], key) && isArray(value) && !value[1]) {
        delete this.filterParams[key];
      }
    });
    if (!isEmpty(this.filterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.filterParams)
      });
    }
    return params;
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

    this.detailService.openDetailModal(
      DataMap.Resource_Type.fileset.value,
      {
        data: assign(
          item,
          { optItems: this.getOptsItems(item) },
          {
            optItemsFn: v => {
              return this.getOptsItems(v);
            }
          }
        )
      },
      () => {
        this.activeItem = {};
        this.cdr.detectChanges();
      }
    );
    this.currentDetailItemUuid = item.uuid;
    this.activeItem = item;
  }

  getSlaDetail(item) {
    this.slaService.getDetail({ uuid: item.sla_id, name: item.sla_name });
  }

  optsCallback = data => {
    return this.getOptsItems(data);
  };

  getOptsItems(item) {
    const menus = [
      {
        id: 'protectFileset',
        disabled:
          item.sla_id ||
          item.protection_status === DataMap.Protection_Status.creating.value ||
          item.protection_status ===
            DataMap.Protection_Status.protected.value ||
          !hasProtectPermission(item),
        label: this.i18n.get('common_protect_label'),
        permission: OperateItems.ProtectHostFileset,
        onClick: () =>
          this.protect(
            [item],
            ProtectResourceAction.Create,
            this.i18n.get('common_protect_label'),
            item
          )
      },
      {
        id: 'modifyProtection',
        disabled: !item.sla_id || !hasProtectPermission(item),
        label: this.i18n.get('protection_modify_protection_label'),
        permission: OperateItems.ModifyFilesetProtection,
        onClick: () =>
          this.protect(
            [item],
            ProtectResourceAction.Modify,
            this.i18n.get('protection_modify_protection_label'),
            item
          )
      },
      {
        id: 'removeProtection',
        divide: true,
        disabled:
          (!item.sla_id &&
            item.protection_status !==
              DataMap.Protection_Status.protected.value) ||
          !hasProtectPermission(item),
        label: this.i18n.get('protection_remove_protection_label'),
        permission: OperateItems.RemoveFilesetProtection,
        onClick: () => this.removeProtection([item], item)
      },
      {
        id: 'activeProtection',
        label: this.i18n.get('protection_active_protection_label'),
        permission: OperateItems.ActivateFilesetProtection,
        disabled:
          !(item.sla_id && !item.sla_status) || !hasProtectPermission(item),
        onClick: () => this.activeProtection([item], item)
      },
      {
        id: 'deactiveProtection',
        divide: true,
        label: this.i18n.get('protection_deactive_protection_label'),
        permission: OperateItems.DeactivateFilesetProtection,
        disabled:
          !(item.sla_id && item.sla_status) || !hasProtectPermission(item),
        onClick: () => this.deactiveProtection([item], item)
      },
      {
        id: 'recovery',
        disabled: !hasRecoveryPermission(item),
        label: this.i18n.get('common_restore_label'),
        permission: OperateItems.RestoreCopy,
        onClick: () =>
          this.getDetail({
            ...item,
            activeId: 'copydata',
            datePickerMode: DATE_PICKER_MODE.DATE
          })
      },
      {
        id: 'manualBackup',
        divide: true,
        disabled: !item.sla_id || !hasBackupPermission(item),
        label: this.i18n.get('common_manual_backup_label'),
        permission: OperateItems.ManuallyBackFileset,
        onClick: () => this.manualBackup(item)
      },
      {
        id: 'modifyFileset',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyHostFileset,
        disabled: !hasResourcePermission(item),
        onClick: () => this.modifyFileset(item)
      },
      {
        id: 'deleteFileset',
        divide: true,
        disabled:
          !isEmpty(item.sla_id) ||
          item.protection_status === DataMap.Protection_Status.creating.value ||
          !hasResourcePermission(item),
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteHostFileset,
        onClick: () => this.deleteFileset([item], item)
      },
      {
        id: 'addTag',
        permission: OperateItems.AddTag,
        disabled: !hasResourcePermission(item),
        label: this.i18n.get('common_add_tag_label'),
        onClick: () => this.addTag([item])
      },
      {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        disabled: !hasResourcePermission(item),
        label: this.i18n.get('common_remove_tag_label'),
        onClick: () => this.removeTag([item])
      }
    ];
    this.optItems = getPermissionMenuItem(menus, this.cookieService.role);
    return this.optItems;
  }

  filterChange = e => {
    if (e.key === 'sla_compliance') {
      assign(this.filterParams, {
        protectedObject: {
          ...this.filterParams.protectedObject,
          sla_compliance: [['in'], ...e.value]
        }
      });
    } else if (e.key === 'sla_status') {
      assign(this.filterParams, {
        protectedObject: {
          ...this.filterParams.protectedObject,
          status: [['in'], ...map(e.value, v => +v)]
        }
      });
    } else if (e.key === 'protection_status') {
      assign(this.filterParams, {
        protectionStatus: e.value
      });
    } else if (e.key === 'osType') {
      assign(this.filterParams, {
        environment: {
          ...this.filterParams?.environment,
          osType: [['in'], ...e.value]
        }
      });
    } else if (e.key === 'osBackup') {
      if (e.value.length !== 2) {
        assign(this.filterParams, {
          is_OS_backup: e.value.includes(false)
            ? [['!='], 'true']
            : [['in'], ...e.value.map(item => String(item))]
        });
      }
    } else {
      assign(this.filterParams, {
        [e.key]: e.value
      });
    }
    this.getFilesets();
  };

  searchByUuid(uuid) {
    assign(this.filterParams, {
      uuid: [['~~'], trim(uuid)]
    });
    this.getFilesets();
  }

  searchByName(name) {
    assign(this.filterParams, {
      name: [['~~'], trim(name)]
    });
    this.getFilesets();
  }

  searchByTemplate(templateName) {
    assign(this.filterParams, {
      templateName: [['~~'], trim(templateName)]
    });
    this.getFilesets();
  }

  searchByHostName(hostName) {
    assign(this.filterParams, {
      environment: {
        ...this.filterParams?.environment,
        name: [['~~'], trim(hostName)]
      }
    });
    this.getFilesets();
  }

  searchByIp(ip) {
    assign(this.filterParams, {
      environment: {
        ...this.filterParams?.environment,
        endpoint: [['~~'], trim(ip)]
      }
    });
    this.getFilesets();
  }

  searchBySlaName(slaName) {
    assign(this.filterParams, {
      protectedObject: {
        ...this.filterParams?.protectedObject,
        slaName: [['~~'], trim(slaName)]
      }
    });
    this.getFilesets();
  }

  searchByLabel(label) {
    assign(this.filterParams, {
      labelCondition: {
        labelName: trim(label)
      }
    });
    this.getFilesets();
  }

  selectionChange(source) {
    each(this.moreMenus, item => {
      if (!size(this.selection)) {
        item.tips = '';
        return (item.disabled = true);
      }

      if (item.id === 'manualBackup') {
        item.disabled =
          size(
            filter(this.selection, val => {
              return !isEmpty(val.sla_id) && hasBackupPermission(val);
            })
          ) !== size(this.selection) || !size(this.selection);
      } else if (
        item.id === 'batchRemoveProtect' ||
        item.id === 'batchModifyProtect'
      ) {
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
      } else if (item.id === 'deleteFileset') {
        item.disabled =
          size(
            filter(this.selection, val => {
              return (
                isEmpty(val.sla_id) &&
                val.protection_status !==
                  DataMap.Protection_Status.creating.value &&
                hasResourcePermission(val)
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
                val.sla_status &&
                hasProtectPermission(val)
              );
            })
          ) !== size(this.selection);
        item.tips = item.disabled
          ? this.i18n.get('protection_partial_resources_deactive_label')
          : '';
      }
    });
    // 批量删除按钮
    this.deleteBtnDisabled =
      size(
        filter(this.selection, val => {
          return (
            isEmpty(val.sla_id) &&
            val.protection_status !==
              DataMap.Protection_Status.creating.value &&
            hasResourcePermission(val)
          );
        })
      ) !== size(this.selection) || !size(this.selection);
    this.protectBtnDisabled =
      size(
        filter(this.selection, val => {
          return (
            isEmpty(val.sla_id) &&
            val.protection_status !==
              DataMap.Protection_Status.creating.value &&
            val.protection_status !==
              DataMap.Protection_Status.protected.value &&
            hasProtectPermission(val)
          );
        })
      ) !== size(this.selection) ||
      !size(this.selection) ||
      size(
        filter(this.selection, val => {
          return (
            val.environment?.osType === this.selection[0].environment?.osType
          );
        })
      ) !== size(this.selection);
    this.protectBtnTooltip = '';
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getFilesets('', '', true);
  }

  manualBackup(datas) {
    if (isArray(datas) && size(datas) > 1) {
      each(datas, item => {
        assign(item, {
          host_ip: item.environment_endpoint,
          resource_id: item.uuid,
          resource_type: datas[0].sub_type
        });
      });
      this.takeManualBackupService.batchExecute(datas, () => {
        this.selection = [];
        this.getFilesets(datas);
      });
    } else if (isArray(datas) && size(datas) === 1) {
      assign(datas[0], {
        host_ip: datas[0].environment_endpoint,
        resource_id: datas[0].uuid,
        resource_type: datas[0].sub_type
      });
      this.takeManualBackupService.execute(datas[0], () => {
        this.selection = [];
        this.getFilesets(datas);
      });
    } else {
      assign(datas, {
        host_ip: datas.environment_endpoint,
        resource_id: datas.uuid,
        resource_type: datas.sub_type
      });
      this.takeManualBackupService.execute(datas, () => {
        this.selection = [];
        this.getFilesets(datas);
      });
    }
  }

  createFileset(item?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('protection_create_fileset_label'),
      lvContent: CreateFilesetTemplateComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.largeWidth + 320,
      lvAfterOpen: modal => {},
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateFilesetTemplateComponent;
          content.onOK().subscribe(
            res => {
              resolve(true);
              this.getFilesets();
            },
            error => resolve(false)
          );
        });
      }
    });
  }

  protect(datas, action: ProtectResourceAction, header?: string, refreshData?) {
    const data = size(datas) > 1 ? datas : datas[0];
    const type =
      size(datas) > 1
        ? ProtectResourceCategory.filesets
        : ProtectResourceCategory.fileset;
    this.protectService.openProtectModal(type, action, {
      width: 780,
      data,
      onOK: () => {
        this.selection = [];
        this.getFilesets(refreshData);
      }
    });
  }

  modifyFileset(res) {
    const item = cloneDeep(res);
    assign(item, {
      environment_os_type:
        item?.environment?.osType || item?.environment_os_type
    });
    if (item && item.extendInfo?.paths) {
      item.extendInfo.paths = JSON.stringify(
        map(JSON.parse(item.extendInfo.paths), 'name')
      );
    }
    if (item && item.extendInfo?.filters) {
      assign(item, {
        filters: JSON.parse(item.extendInfo?.filters)
      });
    }
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_modify_label'),
      lvContent: CreateFilesetTemplateComponent,
      lvOkDisabled: true,
      lvComponentParams: {
        rowItem: item
      },
      lvWidth: MODAL_COMMON.largeWidth + 320,
      lvAfterOpen: modal => {},
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateFilesetTemplateComponent;
          content.onOK().subscribe(
            res => {
              resolve(true);
              this.getFilesets(item);
            },
            error => resolve(false)
          );
        });
      }
    });
  }

  deleteFileset(selection: any[], refreshData?) {
    if (selection.length === 1) {
      this.warningMessageService.create({
        content: this.i18n.get('protection_resource_delete_label', [
          toString(map(selection, 'name'))
        ]),
        onOK: () => {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: first(selection).uuid
            })
            .subscribe(res => {
              this.selection = [];
              this.getFilesets();
            });
        }
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get('protection_resource_batch_delete_label'),
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item => {
              return this.protectedResourceApiService.DeleteResource({
                resourceId: item.uuid,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            cloneDeep(selection),
            () => {
              this.selection = [];
              this.getFilesets();
            }
          );
        }
      });
    }
  }

  removeProtection(datas, refreshData?) {
    const resource_ids = [];
    const resource_names = [];
    forEach(datas, data => {
      resource_ids.push(data.uuid);
      resource_names.push(data.name);
    });
    this.warningMessageService.create({
      content: this.i18n.get('protection_resource_delete_protect_label', [
        toString(resource_names)
      ]),
      onOK: () => {
        this.projectedObjectApiService
          .deleteV1ProtectedObjectsDelete({
            body: {
              resource_ids
            }
          })
          .subscribe({
            next: () => {
              this.selection = [];
              this.getFilesets(refreshData);
            },
            error: () => {
              this.selection = [];
              this.cdr.detectChanges();
            }
          });
      }
    });
  }

  batchRemoveProtect(selection) {
    if (selection.length === 1) {
      this.removeProtection(selection, selection[0]);
    } else {
      this.warningBatchConfirmsService.create({
        data: selection,
        selection: clone(selection),
        message: this.i18n.get(
          'protection_resource_batch_delete_protect_label'
        ),
        operationType: ResourceOperationType.protection,
        onOK: () => {
          this.selection = [];
          this.getFilesets();
        }
      });
    }
  }

  activeProtection(datas, refreshData?) {
    const resource_ids = [];
    forEach(datas, data => {
      resource_ids.push(data.uuid);
    });
    this.projectedObjectApiService
      .activeV1ProtectedObjectsStatusActionActivatePut({
        body: {
          resource_ids
        }
      })
      .subscribe({
        next: () => {
          this.selection = [];
          this.getFilesets(refreshData);
        },
        error: () => {
          this.selection = [];
          this.cdr.detectChanges();
        }
      });
  }

  deactiveProtection(datas, refreshData?) {
    const resource_ids = [];
    const resource_names = [];
    forEach(datas, data => {
      resource_ids.push(data.uuid);
      resource_names.push(data.name);
    });
    this.warningMessageService.create({
      content: this.i18n.get('protection_deactivate_resource_tip_label', [
        toString(resource_names)
      ]),
      onOK: () => {
        this.projectedObjectApiService
          .deactivateV1ProtectedObjectsStatusActionDeactivatePut({
            body: {
              resource_ids
            }
          })
          .subscribe({
            next: () => {
              this.selection = [];
              this.getFilesets(refreshData);
            },
            error: () => {
              this.selection = [];
              this.cdr.detectChanges();
            }
          });
      }
    });
  }

  trackByUuid(index: number, list: any) {
    return list.uuid;
  }

  isActive(item): boolean {
    return item.uuid === this.activeItem?.uuid;
  }
}
