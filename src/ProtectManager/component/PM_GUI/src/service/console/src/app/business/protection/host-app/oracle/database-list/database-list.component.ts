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
  OnInit,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import {
  CommonConsts,
  CookieService,
  DATE_PICKER_MODE,
  DataMap,
  DataMapService,
  DatabasesService,
  GROUP_COMMON,
  GlobalService,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  PROTECTION_NAVIGATE_STATUS,
  ProjectedObjectApiService,
  ProtectResourceAction,
  ProtectResourceCategory,
  ProtectedResourceApiService,
  ResourceOperationType,
  RoleOperationAuth,
  RoleOperationMap,
  SetTagType,
  WarningMessageService,
  extendSlaInfo,
  getLabelList,
  getPermissionMenuItem,
  hasBackupPermission,
  hasProtectPermission,
  hasRecoveryPermission,
  hasResourcePermission
} from 'app/shared';
import { WarningBatchConfirmsService } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.component';
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
  includes,
  isArray,
  isEmpty,
  isNumber,
  isUndefined,
  mapValues,
  size,
  some,
  toString,
  trim
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { map, switchMap, takeUntil } from 'rxjs/operators';
import { AuthComponent } from './auth/auth.component';
import { RegisterComponent } from './register/register.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-database-list',
  templateUrl: './database-list.component.html',
  styleUrls: ['./database-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class DatabaseListComponent implements OnInit, OnDestroy {
  queryUuid;
  queryName;
  queryIp;
  querySlaName;
  queryVersionName;
  environment_name;
  inst_name;
  selection = [];
  tableData = [];
  filterParams: any = {};
  protectBtnDisabled = true;
  activeBtnDisabled = true;
  deactiveBtnDisabled = true;
  protectResourceAction = ProtectResourceAction;
  pageNo = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  activeBtnTip;
  deactiveBtnTip;
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
      label: this.i18n.get('common_status_label'),
      key: 'link_status',
      filter: true,
      filterMap: this.dataMapService.toArray('resource_LinkStatus_Special'),
      isShow: true
    },
    {
      label: this.i18n.get('common_ip_address_label'),
      key: 'path',
      isShow: true
    },
    {
      label: this.i18n.get('common_type_label'),
      key: 'subType',
      filter: true,
      filterMap: this.dataMapService.toArray('oracleType'),
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
            DataMap.Os_Type.aix.value
          ],
          item.value
        );
      }),
      isShow: true
    },
    {
      label: this.i18n.get('common_version_label'),
      key: 'version',
      isShow: true
    },
    {
      label: this.i18n.get('common_auth_status_label'),
      key: 'verify_status',
      filter: true,
      filterMap: this.dataMapService.toArray('Verify_Status'),
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
    {
      key: 'labelList',
      label: this.i18n.get('common_tag_label'),
      isShow: true
    }
  ];
  tableColumnKey = 'protection_oracle_database_table';
  columnStatus = this.rememberColumnsService.getColumnsStatus(
    this.tableColumnKey
  );
  currentDetailItemUuid;
  oracleListSub$: Subscription;
  destroy$ = new Subject();

  groupCommon = GROUP_COMMON;
  activeItem;

  roleOperationMap = RoleOperationMap;
  roleOperationAuth = RoleOperationAuth;

  registerTipShow = false;

  constructor(
    public i18n: I18NService,
    public dataMapService: DataMapService,
    public drawModalService: DrawModalService,
    public protectService: ProtectService,
    public warningMessageService: WarningMessageService,
    public projectedObjectApiService: ProjectedObjectApiService,
    public takeManualBackupService: TakeManualBackupService,
    public databasesService: DatabasesService,
    public slaService: SlaService,
    public detailService: ResourceDetailService,
    public cookieService: CookieService,
    public rememberColumnsService: RememberColumnsService,
    public globalService: GlobalService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private warningBatchConfirmsService: WarningBatchConfirmsService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private batchOperateService: BatchOperateService,
    private messageService: MessageService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.getColumnStatus();
    this.getMoreMenus();
    this.getDatabase();
    this.virtualScroll.getScrollParam(400, 3);
    this.showRegisterTip();
    this.getUserGuideState();
  }

  getUserGuideState() {
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(res => {
        if (res.showTips) {
          this.showRegisterTip();
        }
      });
  }

  showRegisterTip() {
    if (
      USER_GUIDE_CACHE_DATA.active &&
      USER_GUIDE_CACHE_DATA.showTips &&
      includes(this.roleOperationAuth, this.roleOperationMap.manageResource)
    ) {
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

  getMoreMenus() {
    const menus = [
      {
        id: 'batchRemoveProtect',
        label: this.i18n.get('protection_remove_protection_label'),
        disabled:
          !size(this.selection) ||
          !isUndefined(find(this.selection, data => !data.sla_id)),
        permission: OperateItems.RemoveFilesetProtection,
        onClick: () => this.batchRemoveProtect(this.selection)
      },
      {
        id: 'activeProtection',
        tips: '',
        disabled:
          size(
            filter(this.selection, val => {
              return !isEmpty(val.sla_id) && !val.sla_status;
            })
          ) !== size(this.selection) || !size(this.selection),
        label: this.i18n.get('protection_active_protection_label'),
        permission: OperateItems.ActivateHostProtection,
        onClick: () => this.activeProtection(this.selection)
      },
      {
        id: 'deactiveProtection',
        tips: '',
        disabled:
          size(
            filter(this.selection, val => {
              return !isEmpty(val.sla_id) && val.sla_status;
            })
          ) !== size(this.selection) || !size(this.selection),
        label: this.i18n.get('protection_deactive_protection_label'),
        permission: OperateItems.DeactivateHostProtection,
        onClick: () => this.deactiveProtection(this.selection)
      },
      {
        id: 'manualBackup',
        divide: true,
        disabled:
          size(
            filter(this.selection, val => {
              return (
                !isEmpty(val.sla_id) &&
                val.link_status ===
                  DataMap.resource_LinkStatus_Special.normal.value
              );
            })
          ) !== size(this.selection) || !size(this.selection),
        label: this.i18n.get('common_manual_backup_label'),
        permission: OperateItems.ManuallyBackDatabase,
        onClick: () => this.manualBackup(this.selection)
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteDatabase,
        disabled: !size(this.selection),
        onClick: () => this.deleteResource(this.selection)
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
        this.selection = [];
        this.getDatabase();
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
        this.getDatabase();
      }
    });
  }

  getColumnStatus() {
    if (!isEmpty(this.columnStatus)) {
      each(this.columns, col => {
        col.isShow = this.columnStatus[col.key];
      });
    }
  }

  register(rowData?) {
    const openWin = (res?: any) => {
      this.drawModalService.create({
        ...MODAL_COMMON.generateDrawerOptions(),
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvOkDisabled: isEmpty(res),
        lvHeader: isEmpty(res)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterComponent,
        lvComponentParams: { rowData: res },
        lvOk: modal => {
          const content = modal.getContentComponent() as RegisterComponent;
          return new Promise(resolve => {
            content.onOK().subscribe(
              () => {
                resolve(true);
                this.getDatabase();
              },
              () => resolve(false)
            );
          });
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterComponent;
          const instance = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            instance.lvOkDisabled = res !== 'VALID';
          });
          content.patchForm();
        }
      });
    };
    if (rowData) {
      this.protectedResourceApiService
        .ShowResource({ resourceId: rowData.uuid })
        .subscribe(res => {
          openWin(res);
        });
    } else {
      openWin();
    }
  }

  deleteResource(data) {
    if (isArray(data)) {
      this.warningMessageService.create({
        content: this.i18n.get('protection_nas_share_delete_label'),
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
            cloneDeep(data),
            () => {
              this.selection = [];
              this.getDatabase();
            }
          );
        }
      });
    }
    this.warningMessageService.create({
      content: this.i18n.get('protection_nas_share_delete_label'),
      onOK: () => {
        this.protectedResourceApiService
          .DeleteResource({
            resourceId: data.uuid
          })
          .subscribe(() => this.getDatabase());
      }
    });
  }

  refresh() {
    this.getDatabase();
  }

  getDatabase(refreshData?) {
    each(this.moreMenus, item => {
      item.disabled = true;
    });
    if (
      !isEmpty(PROTECTION_NAVIGATE_STATUS.protectionStatus) ||
      isNumber(PROTECTION_NAVIGATE_STATUS.protectionStatus)
    ) {
      each(this.columns, item => {
        if (item.key === 'protection_status') {
          item.filterMap = filter(
            this.dataMapService.toArray('Protection_Status'),
            val => {
              if (val.value === PROTECTION_NAVIGATE_STATUS.protectionStatus) {
                val.selected = true;
              }
              return true;
            }
          );
        }
      });
      assign(this.filterParams, {
        protection_status: [PROTECTION_NAVIGATE_STATUS.protectionStatus]
      });
      PROTECTION_NAVIGATE_STATUS.protectionStatus = '';
    }
    const params = this.getParams() as any;
    if (this.oracleListSub$) {
      this.oracleListSub$.unsubscribe();
    }
    let manualRefresh = true;
    this.oracleListSub$ = timer(0, CommonConsts.TIME_INTERVAL_RESOURCE)
      .pipe(
        switchMap(index => {
          manualRefresh = !index;
          return this.protectedResourceApiService.ListResources({
            ...params,
            akLoading: !index
          });
        }),
        map(res => {
          each(res.records, (item: any) => {
            // 获取标签数据
            const { showList, hoverList } = getLabelList(item);
            item.showLabelList = showList;
            item.hoverLabelList = hoverList;
            item.sub_type = item.subType;
            item.link_status = item.extendInfo?.linkStatus;
            item.verify_status = item.extendInfo?.verify_status === 'true';
            extendSlaInfo(item);
          });
          return res;
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.total = res.totalCount;
        this.tableData = res.records;
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

  refreshDetail(target, tableData) {
    if (find(tableData, { uuid: target.uuid })) {
      this.getDetail(find(tableData, { uuid: target.uuid }));
    } else {
      this.drawModalService.destroyModal('detail-modal');
    }
  }

  getParams() {
    const params = {
      pageNo: this.pageNo,
      pageSize: this.pageSize
    };

    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });

    if (isEmpty(this.filterParams.subType)) {
      assign(this.filterParams, {
        subType: [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value
        ]
      });
    }

    if (!isEmpty(this.filterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.filterParams)
      });
    }

    return params;
  }

  searchByUuid(uuid) {
    assign(this.filterParams, {
      uuid: trim(uuid)
    });
    this.getDatabase();
  }

  searchByName(name) {
    assign(this.filterParams, {
      name: [['~~'], trim(name)]
    });
    if (!trim(name)) {
      delete this.filterParams.name;
    }
    this.getDatabase();
  }

  searchByIp(ip) {
    assign(this.filterParams, {
      path: [['~~'], trim(ip)]
    });
    if (!trim(ip)) {
      delete this.filterParams.path;
      delete this.filterParams.environment;
    }
    this.getDatabase();
  }

  searchBySlaName(slaName) {
    assign(this.filterParams, {
      protectedObject: {
        ...this.filterParams.protectedObject,
        slaName: [['~~'], trim(slaName)]
      }
    });
    if (!trim(slaName)) {
      delete this.filterParams.protectedObject?.slaName;
    }
    this.getDatabase();
  }

  searchByVersion(versionName) {
    assign(this.filterParams, {
      version: [['~~'], trim(versionName)]
    });
    if (!trim(versionName)) {
      delete this.filterParams.version;
    }
    this.getDatabase();
  }

  searchByLabel(label) {
    assign(this.filterParams, {
      labelCondition: {
        labelName: trim(label)
      }
    });
    if (!trim(label)) {
      delete this.filterParams.labelCondition;
    }
    this.getDatabase();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageNo = page.pageIndex;
    this.getDatabase();
  }

  selectionChange(source) {
    this.protectBtnDisabled =
      size(
        filter(this.selection, val => {
          return (
            isEmpty(val.sla_id) &&
            val.link_status ===
              DataMap.resource_LinkStatus_Special.normal.value &&
            val.verify_status &&
            val.protection_status !==
              DataMap.Protection_Status.creating.value &&
            val.protection_status !== DataMap.Protection_Status.protected.value
          );
        })
      ) !== size(this.selection) || !size(this.selection);
    each(this.moreMenus, item => {
      if (!size(this.selection)) {
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
          ) !== size(this.selection) || !size(this.selection);
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
          ) !== size(this.selection) || !size(this.selection);
        item.tips = item.disabled
          ? this.i18n.get('protection_partial_resources_deactive_label')
          : '';
      } else if (item.id === 'manualBackup') {
        item.disabled =
          size(
            filter(this.selection, val => {
              return (
                !isEmpty(val.sla_id) &&
                val.link_status ===
                  DataMap.resource_LinkStatus_Special.normal.value &&
                hasBackupPermission(val)
              );
            })
          ) !== size(this.selection) || !size(this.selection);
      } else if (item.id === 'delete') {
        item.disabled =
          !size(this.selection) ||
          size(
            filter(this.selection, val => {
              return (
                isEmpty(val.sla_id) &&
                val.protection_status !==
                  DataMap.Protection_Status.creating.value &&
                hasResourcePermission(val)
              );
            })
          ) !== size(this.selection);
      } else if (item.id === 'addTag') {
        item.disabled =
          !size(this.selection) ||
          some(this.selection, v => !hasResourcePermission(v));
      } else if (item.id === 'removeTag') {
        item.disabled =
          !size(this.selection) ||
          some(this.selection, v => !hasResourcePermission(v));
      }
    });
    if (!size(this.selection)) {
      this.activeBtnTip = '';
      this.deactiveBtnTip = '';
    }
  }

  filterChange(source) {
    const keyMap = {
      sub_type: 'subType',
      link_status: 'linkStatus',
      protection_status: 'protectionStatus'
    };

    if (source.key === 'verify_status') {
      each(source.value, (v, index) => {
        source.value[index] = trim(v);
      });
    }

    if (source.key === 'labelList') {
      assign(this.filterParams, {
        labelCondition: {
          labelName: source.value
        }
      });
      if (isEmpty(source.value)) {
        delete this.filterParams.labelCondition;
      }
    }

    if (source.key === 'sla_compliance') {
      assign(this.filterParams, {
        protectedObject: {
          ...this.filterParams.protectedObject,
          sla_compliance: [['in'], ...source.value]
        }
      });
      if (isEmpty(source.value)) {
        delete this.filterParams.protectedObject?.sla_compliance;
      }
    } else {
      if (source.key === 'osType') {
        assign(this.filterParams, {
          environment: {
            osType: [['in'], ...source.value]
          }
        });
        if (isEmpty(source.value)) {
          delete this.filterParams.environment;
        }
      } else {
        assign(this.filterParams, {
          [keyMap[source.key] ? keyMap[source.key] : source.key]: [
            ['in'],
            ...source.value
          ]
        });
        if (isEmpty(source.value)) {
          delete this.filterParams[
            keyMap[source.key] ? keyMap[source.key] : source.key
          ];
        }
      }
    }

    this.getDatabase();
  }

  getSlaDetail(item) {
    this.slaService.getDetail({ uuid: item.sla_id, name: item.sla_name });
  }

  optsCallback = data => {
    return this.getOptItems(data);
  };

  getOptItems = data => {
    const menus = [
      {
        id: 'protect',
        disabled:
          data.sla_id ||
          data.link_status ===
            DataMap.resource_LinkStatus_Special.offline.value ||
          !data.verify_status ||
          data.protection_status === DataMap.Protection_Status.creating.value ||
          data.protection_status ===
            DataMap.Protection_Status.protected.value ||
          !hasProtectPermission(data),
        label: this.i18n.get('common_protect_label'),
        permission: OperateItems.ProtectDatabases,
        onClick: () =>
          this.protect(
            [data],
            ProtectResourceAction.Create,
            this.i18n.get('common_protect_label')
          )
      },
      {
        id: 'modifyProtect',
        disabled:
          !data.sla_id ||
          data.link_status ===
            DataMap.resource_LinkStatus_Special.offline.value ||
          !data.verify_status ||
          !hasProtectPermission(data),
        label: this.i18n.get('protection_modify_protection_label'),
        permission: OperateItems.ModifyDatabaseProtection,
        onClick: () => {
          this.protect(
            [data],
            ProtectResourceAction.Modify,
            this.i18n.get('protection_modify_protection_label')
          );
        }
      },
      {
        id: 'removeProtection',
        divide: true,
        disabled:
          (!data.sla_id &&
            data.protection_status !==
              DataMap.Protection_Status.protected.value) ||
          !hasProtectPermission(data),
        label: this.i18n.get('protection_remove_protection_label'),
        permission: OperateItems.RemoveDatabaseProtection,
        onClick: () => this.removeProtection([data])
      },
      {
        id: 'activeProtection',
        disabled:
          !(data.sla_id && !data.sla_status) || !hasProtectPermission(data),
        label: this.i18n.get('protection_active_protection_label'),
        permission: OperateItems.ActivateDatabaseProtection,
        onClick: () => this.activeProtection([data])
      },
      {
        divide: true,
        id: 'deactiveProtection',
        disabled:
          !(data.sla_id && data.sla_status) || !hasProtectPermission(data),
        label: this.i18n.get('protection_deactive_protection_label'),
        permission: OperateItems.DeactivateDatabseProtection,
        onClick: () => this.deactiveProtection([data])
      },
      {
        id: 'recovery',
        disabled: !hasRecoveryPermission(data),
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
        divide: true,
        disabled:
          !data.sla_id ||
          data.link_status ===
            DataMap.resource_LinkStatus_Special.offline.value ||
          !hasBackupPermission(data),
        label: this.i18n.get('common_manual_backup_label'),
        permission: OperateItems.ManuallyBackDatabase,
        onClick: () => this.manualBackup(data)
      },
      {
        id: 'rescan',
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('common_rescan_label'),
        disabled: !hasResourcePermission(data),
        onClick: () => {
          this.protectedResourceApiService
            .ScanProtectedResources({
              resId: data.uuid
            })
            .subscribe(() => this.getDatabase());
        }
      },
      {
        id: 'connectTest',
        divide: true,
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('protection_connectivity_test_label'),
        disabled: !hasResourcePermission(data),
        onClick: () => this.connectTest(data)
      },
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyDatabaseAuth,
        disabled: !hasResourcePermission(data),
        onClick: () => this.register(data)
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        disabled:
          data.sla_id ||
          data.protection_status === DataMap.Protection_Status.creating.value ||
          !hasResourcePermission(data),
        permission: OperateItems.DeleteDatabase,
        onClick: () => this.deleteResource(data)
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
  };

  removeProtection(datas) {
    const resource_ids = [];
    const resource_names = [];
    each(datas, data => {
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
              this.getDatabase(size(datas) === 1 ? datas[0] : '');
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
      this.removeProtection(selection);
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
          this.getDatabase();
        }
      });
    }
  }

  protect(datas, action: ProtectResourceAction, header?: string) {
    const data = size(datas) > 1 ? datas : datas[0];
    const type =
      size(datas) > 1
        ? ProtectResourceCategory.oracles
        : ProtectResourceCategory.oracle;
    this.protectService.openProtectModal(type, action, {
      width: 780,
      data,
      onOK: () => {
        this.selection = [];
        this.getDatabase(size(datas) === 1 ? datas[0] : '');
      }
    });
  }

  activeProtection(datas) {
    const resource_ids = [];
    each(datas, data => {
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
          this.getDatabase(size(datas) === 1 ? datas[0] : '');
        },
        error: () => {
          this.selection = [];
          this.cdr.detectChanges();
        }
      });
  }

  deactiveProtection(datas) {
    const resource_ids = [];
    const resource_names = [];
    each(datas, data => {
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
              this.getDatabase(size(datas) === 1 ? datas[0] : '');
            },
            error: () => {
              this.selection = [];
              this.cdr.detectChanges();
            }
          });
      }
    });
  }

  manualBackup(datas) {
    if (isArray(datas) && size(datas) > 1) {
      each(datas, item => {
        assign(item, {
          host_ip: item.path,
          resource_id: item.uuid,
          resource_type: datas[0].sub_type
        });
      });
      this.takeManualBackupService.batchExecute(datas, () => {
        this.selection = [];
        this.getDatabase();
      });
    } else if (isArray(datas) && size(datas) === 1) {
      assign(datas[0], {
        host_ip: datas[0].path,
        resource_id: datas[0].uuid,
        resource_type: datas[0].sub_type
      });
      this.takeManualBackupService.execute(datas[0], () => {
        this.selection = [];
        this.getDatabase();
      });
    } else {
      assign(datas, {
        host_ip: datas.path,
        resource_id: datas.uuid,
        resource_type: datas.sub_type
      });
      this.takeManualBackupService.execute(datas, () => {
        this.selection = [];
        this.getDatabase();
      });
    }
  }

  getDetail(item) {
    this.activeItem = item;
    this.protectedResourceApiService
      .ShowResource({ resourceId: item.uuid })
      .subscribe(res => {
        extendSlaInfo(res);
        assign(res, {
          sub_type: res.subType,
          link_status: res.extendInfo?.linkStatus,
          verify_status: res.extendInfo?.verify_status === 'true'
        });
        this.detailService.openDetailModal(
          DataMap.Resource_Type.oracle.value,
          {
            data: assign(
              {
                ...res,
                ip: item.path || item.endpoint
              },
              { optItems: this.getOptItems(item) },
              {
                optItemsFn: v => {
                  return this.getOptItems(v);
                }
              }
            )
          },
          () => {
            this.activeItem = {};
            this.cdr.detectChanges();
          }
        );
      });
    this.currentDetailItemUuid = item.uuid;
  }

  auth(data, header) {
    this.protectedResourceApiService
      .ShowResource({ resourceId: data.uuid })
      .subscribe(res => {
        this.drawModalService.create({
          ...MODAL_COMMON.generateDrawerOptions(),
          lvWidth: MODAL_COMMON.normalWidth,
          lvOkDisabled:
            data.auth?.authType &&
            data.auth?.authType === DataMap.Database_Auth_Method.db.value,
          lvHeader: header,
          lvContent: AuthComponent,
          lvOk: modal => {
            const content = modal.getContentComponent() as AuthComponent;
            return new Promise(resolve => {
              content.onOK().subscribe({
                next: () => {
                  resolve(true);
                  this.getDatabase();
                },
                error: () => resolve(false)
              });
            });
          },
          lvComponentParams: {
            data: res
          },
          lvAfterOpen: modal => {
            const content = modal.getContentComponent() as AuthComponent;
            const instance = modal.getInstance();
            content.formGroup.statusChanges.subscribe(res => {
              instance.lvOkDisabled = res !== 'VALID';
            });
          }
        });
      });
  }

  connectTest(data) {
    this.protectedResourceApiService
      .CheckProtectedResource({
        resourceId: data.uuid
      })
      .subscribe((res: any) => {
        const returnRes = JSON.parse(res);
        const idx = returnRes.findIndex(item => item.code !== 0);
        if (idx !== -1) {
          this.messageService.error(this.i18n.get(returnRes[idx].code), {
            lvMessageKey: 'errorKey',
            lvShowCloseButton: true
          });
        } else {
          this.messageService.success(
            this.i18n.get('common_operate_success_label'),
            {
              lvMessageKey: 'successKey',
              lvShowCloseButton: true
            }
          );
        }
      });
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
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }
}
