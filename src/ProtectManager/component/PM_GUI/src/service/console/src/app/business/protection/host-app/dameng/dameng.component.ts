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
  ChangeDetectorRef,
  Component,
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
  ProtectResourceCategory,
  RoleOperationMap,
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
  filter,
  includes,
  isEmpty,
  isUndefined,
  map,
  omit,
  reject,
  size,
  trim,
  values,
  mapValues,
  some
} from 'lodash';
import { combineLatest } from 'rxjs';
import { RegisterComponent } from './register/register.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-dameng',
  templateUrl: './dameng.component.html',
  styleUrls: ['./dameng.component.less']
})
export class DamengComponent implements OnInit, AfterViewInit {
  name: string;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig: ProButton[];
  clusterName: any;
  selectionData = [];
  opts = [];

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private slaService: SlaService,
    private messageService: MessageService,
    private protectService: ProtectService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private takeManualBackupService: TakeManualBackupService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig(): void {
    const optItems: { [key: string]: ProButton } = {
      register: {
        id: 'register',
        permission: RoleOperationMap.manageResource,
        label: this.i18n.get('common_register_label'),
        type: 'primary',
        popoverContent: this.i18n.get('protection_guide_database_tip_label'),
        popoverShow: USER_GUIDE_CACHE_DATA.active,
        onClick: () => this.register()
      },
      protect: {
        id: 'protect',
        permission: OperateItems.Protection,
        label: this.i18n.get('common_protect_label'),
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  isEmpty(val.sla_id) &&
                  val.protection_status !==
                    DataMap.Protection_Status.creating.value &&
                  val.protection_status !==
                    DataMap.Protection_Status.protected.value &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        onClick: data => {
          this.protect(data, ProtectResourceAction.Create);
        }
      },
      modifyProtect: {
        id: 'modifyProtect',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
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
        permission: OperateItems.RemoveProtection,
        label: this.i18n.get('protection_remove_protection_label'),
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  (!isEmpty(val.sla_id) ||
                    val.protection_status ===
                      DataMap.Protection_Status.protected.value) &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        onClick: data => {
          this.protectService
            .removeProtection(map(data, 'uuid'), map(data, 'name'))
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            });
        }
      },
      activeProtection: {
        id: 'activeProtection',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
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
        label: this.i18n.get('protection_active_protection_label'),
        onClick: data => {
          this.protectService
            .activeProtection(map(data, 'uuid'))
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            });
        }
      },
      deactiveProtection: {
        id: 'deactiveProtection',
        divide: true,
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  !isEmpty(val.sla_id) &&
                  val.sla_status &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.DeactivateProtection,
        label: this.i18n.get('protection_deactive_protection_label'),
        onClick: data => {
          this.protectService
            .deactiveProtection(map(data, 'uuid'), map(data, 'name'))
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            });
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
              filter(data, val => {
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
      rescan: {
        id: 'rescan',
        displayCheck: data => {
          return (
            data[0].sub_type === DataMap.Resource_Type.Dameng_cluster.value
          );
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('common_rescan_label'),
        onClick: data => {
          this.protectedResourceApiService
            .ScanProtectedResources({
              resId: data[0].uuid
            })
            .subscribe(res => {
              this.dataTable.fetchData();
            });
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
        onClick: ([data]) => {
          this.protectedResourceApiService
            .CheckProtectedResource({ resourceId: data.uuid })
            .subscribe(res => {
              this.messageService.success(
                this.i18n.get('job_status_success_label'),
                {
                  lvMessageKey: 'successKey',
                  lvShowCloseButton: true
                }
              );
              this.dataTable.fetchData();
            });
        }
      },
      modify: {
        id: 'modify',
        permission: OperateItems.RegisterDatabase,
        label: this.i18n.get('common_modify_label'),
        onClick: data => {
          this.register(data[0]);
        },
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        }
      },
      deleteResource: {
        id: 'deleteResource',
        permission: OperateItems.DeleteResource,
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
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
        displayCheck: data => {
          return true;
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
          return !size(data);
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
          return !size(data);
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(data)
      }
    };

    this.opts = cloneDeep(
      getPermissionMenuItem(values(reject(optItems, { id: 'register' })))
    );
    each(this.opts, item => {
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
        sort: true,
        name: this.i18n.get('common_name_label'),
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: (data: any) => {
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
        key: 'subType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Dameng_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Dameng_Type')
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
            click: (data: { sla_id: any; sla_name: any }) => {
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
        key: 'protection_status',
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
          type: 'search',
          filterMode: 'contains'
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
            items: this.opts
          }
        }
      }
    ];

    const removeProtectionBtn = clone(optItems.removeProtection);
    removeProtectionBtn.divide = false;
    const deactiveBtn = clone(optItems.deactiveProtection);
    deactiveBtn.divide = false;
    this.optsConfig = getPermissionMenuItem([
      optItems.register,
      optItems.protect,
      removeProtectionBtn,
      optItems.activeProtection,
      deactiveBtn,
      optItems.deleteResource,
      optItems.manualBackup,
      optItems.addTag,
      optItems.removeTag
    ]);

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
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data ? data : this.selectionData,
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
      onOk: () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      }
    });
  }

  onChange() {
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  getData(filters: Filters, args: { isAutoPolling: any }) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType: [
        DataMap.Resource_Type.Dameng_cluster.value,
        DataMap.Resource_Type.Dameng_singleNode.value
      ],
      isTopInstance: [['=='], '1']
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
        assign(conditionsTemp, {
          labelCondition: {
            labelName: conditionsTemp.labelList[1]
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
      .subscribe((res: any) => {
        each(res.records, item => {
          // 获取标签数据
          const { showList, hoverList } = getLabelList(item);
          assign(item, {
            version: item.extendInfo.version,
            sub_type: item.subType,
            showLabelList: showList,
            hoverLabelList: hoverList
          });
          extendSlaInfo(item);
        });
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  search() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(this.clusterName)
        }
      ]
    });
    this.dataTable.fetchData();
  }

  register(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-dameng',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(data)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterComponent;
          combineLatest([
            content.formGroup.statusChanges,
            content.validInstance
          ]).subscribe(res => {
            const [formVaild, instanceVaild] = res;
            modal.getInstance().lvOkDisabled =
              formVaild !== 'VALID' || !instanceVaild;
          });
          content.validInstance.next(true);
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.dataTable.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  protect(datas, action: ProtectResourceAction, header?: string, refreshData?) {
    const data = size(datas) > 1 ? datas : datas[0];
    this.protectService.openProtectModal(
      ProtectResourceCategory.Dameng,
      action,
      {
        width: 780,
        data,
        onOK: () => {
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.dataTable.fetchData();
        },
        restoreWidth: params => this.getResourceDetail(params)
      }
    );
  }

  manualBackup(datas) {
    if (size(datas) > 1) {
      // 当同时手动备份集群和单机时，把集群资源置顶，以避免出现日志备份的选项
      datas.sort((a, b) => {
        if (a.sub_type === DataMap.Resource_Type.Dameng_cluster.value) {
          return -1;
        } else if (b.sub_type === DataMap.Resource_Type.Dameng_cluster.value) {
          return 1;
        } else {
          return 0;
        }
      });
      each(datas, item => {
        assign(item, {
          host_ip: item.environment_endpoint,
          resource_id: item.uuid,
          resource_type: item.sub_type
        });
      });
      this.takeManualBackupService.batchExecute(datas, () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      });
    } else {
      assign(datas[0], {
        host_ip: datas[0].environment_endpoint,
        resource_id: datas[0].uuid,
        resource_type: datas[0].sub_type
      });
      this.takeManualBackupService.execute(datas[0], () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      });
    }
  }

  getResourceDetail(res) {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: res.uuid
      })
      .subscribe(item => {
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
        this.detailService.openDetailModal(DataMap.Resource_Type.Dameng.value, {
          data: assign(
            omit(cloneDeep(res), ['sla_id']),
            item,
            {
              optItems: getTableOptsItems(
                cloneDeep(this.opts),
                assign(omit(cloneDeep(res), ['sla_id']), item),
                this
              )
            },
            {
              optItemsFn: v => {
                return getTableOptsItems(cloneDeep(this.opts), v, this);
              }
            }
          )
        });
      });
  }

  deleteRes(data) {
    if (size(data) === 1) {
      this.warningMessageService.create({
        content: this.i18n.get('protection_resource_delete_label', [
          data[0].name
        ]),
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
        content: this.i18n.get('protection_resource_delete_label', [
          map(data, 'name').join(',')
        ]),
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
}
