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
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  DATE_PICKER_MODE,
  disableDeactiveProtectionTips,
  extendSlaInfo,
  getLabelList,
  getPermissionMenuItem,
  getTableOptsItems,
  GROUP_COMMON,
  hasBackupPermission,
  hasProtectPermission,
  hasRecoveryPermission,
  hasResourcePermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectedResourceApiService,
  ProtectResourceAction,
  RoleOperationMap,
  SetTagType,
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
import { ProtectService } from 'app/shared/services/protect.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  clone,
  cloneDeep,
  each,
  filter as _filter,
  first,
  includes,
  isEmpty,
  isUndefined,
  map as _map,
  mapValues,
  omit,
  reject,
  size,
  trim,
  values,
  some
} from 'lodash';
import { map } from 'rxjs/operators';
import { InstanceRegisterComponent } from '../instance-register/instance-register.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { GetLabelOptionsService } from '../../../../../shared/services/get-labels.service';

@Component({
  selector: 'aui-table-template',
  templateUrl: './table-template.component.html',
  styleUrls: ['./table-template.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class TableTemplateComponent implements OnInit, AfterViewInit {
  name;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  optItems = [];
  dataMap = DataMap;
  currentDetailUuid = '';
  groupCommon = GROUP_COMMON;

  @Input() activeIndex;
  @Input() sourceType;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('deployTypeTpl', { static: true })
  deployTypeTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private protectService: ProtectService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    public virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private batchOperateService: BatchOperateService,
    private takeManualBackupService: TakeManualBackupService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService,
    private getLabelOptionsService: GetLabelOptionsService,
    private appUtilsService: AppUtilsService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(340);
    this.initConfig();
  }

  onChange() {
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      register: {
        id: 'register',
        type: 'primary',
        permission: RoleOperationMap.manageResource,
        label: this.i18n.get('common_register_label'),
        onClick: () => this.register(),
        popoverContent: this.i18n.get('protection_guide_database_tip_label'),
        popoverShow: USER_GUIDE_CACHE_DATA.active
      },
      protect: {
        id: 'protect',
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return (
                  isEmpty(val.sla_id) &&
                  val.protection_status !==
                    DataMap.Protection_Status.creating.value &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.Protection,
        label: this.i18n.get('common_protect_label'),
        onClick: data => this.protect(data, ProtectResourceAction.Create)
      },
      modifyProtect: {
        id: 'modifyProtect',
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return !isEmpty(val.sla_id) && hasProtectPermission(val);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ModifyProtection,
        label: this.i18n.get('common_resource_protection_modify_label'),
        onClick: data =>
          this.protect(
            data,
            ProtectResourceAction.Modify,
            this.i18n.get('protection_modify_protection_label'),
            data
          )
      },
      removeProtection: {
        id: 'removeProtection',
        divide: true,
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return !isEmpty(val.sla_id) && hasProtectPermission(val);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.RemoveProtection,
        label: this.i18n.get('protection_remove_protection_label'),
        onClick: data => {
          this.protectService
            .removeProtection(_map(data, 'uuid'), _map(data, 'name'))
            .subscribe(() => this.dataTable.fetchData());
        }
      },
      activeProtection: {
        id: 'activeProtection',
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return (
                  !isEmpty(val.sla_id) &&
                  !val.sla_status &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ActivateProtection,
        disabledTips: this.i18n.get(
          'protection_partial_resources_active_label'
        ),
        label: this.i18n.get('protection_active_protection_label'),
        onClick: data => {
          this.protectService
            .activeProtection(_map(data, 'uuid'))
            .subscribe(() => this.dataTable.fetchData());
        }
      },
      deactiveProtection: {
        id: 'deactiveProtection',
        divide: true,
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return (
                  !isEmpty(val.sla_id) &&
                  val.sla_status &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) ||
            !size(data) ||
            size(data) > CommonConsts.DEACTIVE_PROTECTION_MAX
          );
        },
        permission: OperateItems.DeactivateProtection,
        disabledTips: this.i18n.get(
          'protection_partial_resources_deactive_label'
        ),
        disabledTipsCheck: data =>
          disableDeactiveProtectionTips(data, this.i18n),
        label: this.i18n.get('protection_deactive_protection_label'),
        onClick: data => {
          this.protectService
            .deactiveProtection(_map(data, 'uuid'), _map(data, 'name'))
            .subscribe(res => this.dataTable.fetchData());
        }
      },
      recovery: {
        id: 'recovery',
        disableCheck: data => {
          return (
            !size(data) || some(data, item => !hasRecoveryPermission(item))
          );
        },
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('common_restore_label'),
        onClick: data =>
          this.getResourceDetail({
            ...data[0],
            activeId: 'copydata',
            datePickerMode: DATE_PICKER_MODE.DATE
          })
      },
      manualBackup: {
        id: 'manualBackup',
        divide: true,
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return !isEmpty(val.sla_id) && hasBackupPermission(val);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('common_manual_backup_label'),
        onClick: data => {
          this.manualBackup(data);
        }
      },
      connectivityTest: {
        id: 'connectivityTest',
        divide: true,
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('protection_connectivity_test_label'),
        onClick: data => {
          this.protectedResourceApiService
            .CheckProtectedResource({ resourceId: data[0].uuid })
            .subscribe({
              next: res => {
                this.messageService.success(
                  this.i18n.get('job_status_success_label'),
                  {
                    lvMessageKey: 'successKey',
                    lvShowCloseButton: true
                  }
                );
                this.dataTable.fetchData();
              },
              error: error => {
                this.dataTable.fetchData();
              }
            });
        }
      },
      modify: {
        id: 'modify',
        permission: OperateItems.RegisterNasShare,
        label: this.i18n.get('common_modify_label'),
        onClick: data => {
          this.register(first(data));
        },
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return (
            size(
              _filter(data, item => {
                return (
                  item['rootUuid'] === first(data)['rootUuid'] &&
                  item['shareMode'] === first(data)['shareMode'] &&
                  hasResourcePermission(item)
                );
              })
            ) !== size(data) || !size(data)
          );
        }
      },
      deleteResource: {
        id: 'deleteResource',
        permission: OperateItems.DeleteResource,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return (
                  isEmpty(val.sla_id) &&
                  val.protection_status !==
                    DataMap.Protection_Status.creating.value &&
                  hasResourcePermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        label: this.i18n.get('common_delete_label'),
        onClick: data => this.deleteRes(data)
      },
      addTag: {
        id: 'addTag',
        permission: OperateItems.AddTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        label: this.i18n.get('common_add_tag_label'),
        onClick: data => this.addTag(data)
      },
      removeTag: {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(data)
      }
    };
    this.optItems = cloneDeep(
      getPermissionMenuItem(
        values(reject(opts, item => includes(['register'], item.id)))
      )
    );
    each(this.optItems, item => {
      if (item.disabledTips) {
        item.disabledTips = '';
      }
    });

    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
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
              this.getResourceDetail(data);
            }
          }
        }
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('resource_LinkStatus_Special')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      },
      {
        key: 'deployType',
        name: this.i18n.get('common_type_label'),
        hidden: true,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('generalDbClusterType')
        },
        cellRender: this.deployTypeTpl
      },
      {
        key: 'databaseTypeDisplay',
        name: this.i18n.get('protection_database_type_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'relatedHostIps',
        name: this.i18n.get('protection_node_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'version',
        name: this.i18n.get('common_version_label')
      },
      {
        key: 'sla_name',
        name: this.i18n.get('common_sla_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            overflow: true,
            click: data => {
              this.slaService.getDetail({
                uuid: data.sla_id,
                name: data.sla_name
              });
            }
          }
        }
      },
      {
        key: 'sla_compliance',
        name: this.i18n.get('common_sla_compliance_label'),
        thExtra: this.slaComplianceExtraTpl,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Sla_Compliance')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Sla_Compliance')
        }
      },
      {
        key: 'protectionStatus',
        name: this.i18n.get('protection_protected_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Protection_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Protection_Status')
        }
      },
      {
        key: 'labelList',
        name: this.i18n.get('common_tag_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: false,
          showSearch: true,
          options: () => this.getLabelOptionsService.getLabelOptions()
        },
        cellRender: this.resourceTagTpl
      },
      {
        key: 'operation',
        width: 130,
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: this.optItems
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'uuid',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };

    const removeProtectionBtn = clone(opts.removeProtection);
    removeProtectionBtn.divide = false;
    const deactiveBtn = clone(opts.deactiveProtection);
    deactiveBtn.divide = false;
    this.optsConfig = getPermissionMenuItem([
      opts.register,
      opts.protect,
      removeProtectionBtn,
      opts.activeProtection,
      deactiveBtn,
      assign(cloneDeep(opts.manualBackup), {
        divide: false
      }),
      opts.deleteResource,
      opts.addTag,
      opts.removeTag
    ]);
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data ? data : this.selectionData,
      type: SetTagType.Resource,
      onOk: () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      }
    });
  }

  removeTag(data) {
    this.setResourceTagService.setTag({
      isAdd: false,
      rowDatas: data ? data : this.selectionData,
      type: SetTagType.Resource,
      onOk: () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      }
    });
  }

  search() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(this.name)
        }
      ]
    });
    this.dataTable.fetchData();
  }

  manualBackup(datas) {
    if (size(datas) > 1) {
      each(datas, item => {
        assign(item, {
          host_ip: item.environment_endpoint,
          resource_id: item.uuid,
          resource_type: datas[0].sub_type
        });
      });
      this.takeManualBackupService.batchExecute(datas, () =>
        this.dataTable.fetchData()
      );
    } else {
      assign(datas[0], {
        host_ip: datas[0].environment_endpoint,
        resource_id: datas[0].uuid,
        resource_type: datas[0].sub_type
      });
      this.takeManualBackupService.execute(datas[0], () =>
        this.dataTable?.fetchData()
      );
    }
  }

  register(item?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-nas-shared',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: InstanceRegisterComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          item,
          sourceType: this.sourceType
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as InstanceRegisterComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(formGroupStatus => {
            modalIns.lvOkDisabled = formGroupStatus === 'INVALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as InstanceRegisterComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                if (
                  !isEmpty(item) &&
                  includes(
                    mapValues(this.drawModalService.modals, 'key'),
                    'detail-modal'
                  )
                ) {
                  this.getResourceDetail(content);
                } else {
                  this.dataTable.fetchData();
                }
              },
              error: () => resolve(false)
            });
          });
        },
        lvCancel: modal => {
          const content = modal.getContentComponent() as InstanceRegisterComponent;
          if (
            !isEmpty(item) &&
            includes(
              mapValues(this.drawModalService.modals, 'key'),
              'detail-modal'
            )
          ) {
            this.getResourceDetail(content);
          }
        }
      })
    );
  }

  deleteRes(data) {
    if (size(data) === 1) {
      this.warningMessageService.create({
        content: this.i18n.get('protection_nas_share_delete_label'),
        onOK: () => {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: data[0].uuid
            })
            .subscribe(() => {
              this.selectionData = reject(
                this.dataTable.getAllSelections(),
                item => {
                  return item.uuid === data[0].uuid;
                }
              );
              this.dataTable.setSelections(this.selectionData);
              this.dataTable.fetchData();
            });
        }
      });
    } else {
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
            cloneDeep(this.selectionData),
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

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType: DataMap.Resource_Type.generalDatabase.value,
      firstClassification:
        this.sourceType === DataMap.Resource_Type.generalDatabase.value
          ? '2'
          : '1'
    };
    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.equipment) {
        assign(conditionsTemp, {
          environment: {
            name: conditionsTemp.equipment
          }
        });
        delete conditionsTemp.equipment;
      }
      if (conditionsTemp.equipmentType) {
        if (isEmpty(conditionsTemp.environment)) {
          assign(conditionsTemp, {
            environment: {
              subType: conditionsTemp.equipmentType
            }
          });
        } else {
          assign(conditionsTemp.environment, {
            subType: conditionsTemp.equipmentType
          });
        }
        delete conditionsTemp.equipmentType;
      }
      if (conditionsTemp.labelList) {
        conditionsTemp.labelList.shift();
        assign(conditionsTemp, {
          labelCondition: {
            labelList: conditionsTemp.labelList
          }
        });
        delete conditionsTemp.labelList;
      }
      assign(defaultConditions, conditionsTemp);
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            // 获取标签数据
            const { showList, hoverList } = getLabelList(item);
            assign(item, {
              sub_type: item.subType,
              deployType: item.extendInfo?.deployType,
              databaseTypeDisplay: item.extendInfo?.databaseTypeDisplay,
              cluster_name: item.environment?.name,
              auth_status: item.extendInfo?.authStatus,
              relatedHostIps: item.extendInfo?.relatedHostIps,
              showLabelList: showList,
              hoverLabelList: hoverList
            });
            extendSlaInfo(item);
          });

          return res;
        })
      )
      .subscribe(res => {
        this.appUtilsService.openDetailModalAfterQueryData(
          {
            autoPolling: args?.isAutoPolling,
            records: res.records
          },
          this
        );
        this.tableData = {
          total: res.totalCount,
          data: res.records
        };
        if (!(!isUndefined(args) && args.isAutoPolling)) {
          this.selectionData = [];
          this.dataTable.setSelections([]);
        }
        this.cdr.detectChanges();
      });
  }

  getResourceDetail(res) {
    this.currentDetailUuid = res.uuid;
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType: [DataMap.Resource_Type.generalDatabase.value],
          uuid: [['~~'], res.uuid]
        })
      })
      .pipe(
        map(result => {
          return first(result.records) || {};
        })
      )
      .subscribe((item: any) => {
        if (!item || isEmpty(item)) {
          this.messageService.error(
            this.i18n.get('common_resource_not_exist_label'),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'resNotExistMesageKey'
            }
          );
          return;
        }
        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'slaDetailModalKey'
          )
        ) {
          this.drawModalService.destroyModal('slaDetailModalKey');
        }
        extendSlaInfo(item);
        this.detailService.openDetailModal(
          DataMap.Resource_Type.generalDatabase.value,
          {
            data: assign(
              omit(cloneDeep(res), ['sla_id']),
              item,
              {
                optItems: getTableOptsItems(
                  cloneDeep(this.optItems),
                  assign(omit(cloneDeep(res), ['sla_id']), item),
                  this
                )
              },
              {
                optItemsFn: v => {
                  return getTableOptsItems(cloneDeep(this.optItems), v, this);
                }
              }
            ),
            lvHeader: item?.name
          }
        );
      });
  }

  protect(datas, action: ProtectResourceAction, header?: string, refreshData?) {
    const data = size(datas) > 1 ? datas : datas[0];
    this.protectService.openProtectModal(
      DataMap.Resource_Type.generalDatabase.value,
      action,
      {
        width: 780,
        data,
        onOK: () => this.dataTable.fetchData()
      }
    );
  }
}
