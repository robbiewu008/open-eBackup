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
  Component,
  EventEmitter,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  extendSlaInfo,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProjectedObjectApiService,
  ProtectedResourceApiService,
  ProtectResourceAction,
  WarningMessageService
} from 'app/shared';
import { ProtectedResourcePageListResponse } from 'app/shared/api/models';
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
import { SlaService } from 'app/shared/services/sla.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  clone,
  cloneDeep,
  each,
  filter as _filter,
  find,
  isEmpty,
  isUndefined,
  map as _map,
  size,
  tail,
  values,
  isNumber
} from 'lodash';
import { map } from 'rxjs/operators';
import { ManualDetecteComponent } from './manual-detecte/manual-detecte.component';

@Component({
  selector: 'aui-ransomware-file-system',
  templateUrl: './file-system.component.html',
  styleUrls: ['./file-system.component.less']
})
export class FileSystemComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  dataMap = DataMap;

  @Output() refreshFileSystem = new EventEmitter();

  optItems: any = [];

  colors = [[0, '#6C92FA']];

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('storageDeviceTpl', { static: true })
  storageDeviceTpl: TemplateRef<any>;
  @ViewChild('policyTpl', { static: true })
  policyTpl: TemplateRef<any>;
  @ViewChild('learningTpl', { static: true })
  learningTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private protectService: ProtectService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    public virtualScroll: VirtualScrollService,
    private drawModalService: DrawModalService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private projectedObjectApiService: ProjectedObjectApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
    this.getStorage();
  }

  initStatusFilter() {
    const status = this.appUtilsService.getCacheValue('protectionStatus');
    if (isNumber(status) || !isEmpty(status)) {
      this.dataTable.setFilterMap(
        assign(this.dataTable.filterMap, {
          filters: [
            {
              caseSensitive: false,
              key: 'protectionStatus',
              value: [status]
            }
          ]
        })
      );
    }
  }

  getStorage() {
    this.appUtilsService
      .getCyberEngineStorage()
      .subscribe((res: ProtectedResourcePageListResponse) => {
        const deviceFilterMap = _map(res.records, item => {
          return {
            key: item.uuid,
            label: item.name,
            value: item.name
          };
        });
        each(this.tableConfig.table.columns, item => {
          if (item.key === 'parentName') {
            assign(item, {
              filter: {
                type: 'select',
                isMultiple: true,
                showCheckAll: true,
                options: deviceFilterMap
              }
            });
          }
        });
        this.dataTable.init();
        this.initStatusFilter();
      });
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      protect: {
        id: 'protect',
        type: 'primary',
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return (
                  isEmpty(val.sla_id) &&
                  val.protection_status !==
                    DataMap.Protection_Status.creating.value &&
                  val.protection_status !==
                    DataMap.Protection_Status.protected.value
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
                return !isEmpty(val.sla_id);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ModifyProtection,
        label: this.i18n.get('common_resource_protection_modify_label'),
        onClick: data => this.protect(data, ProtectResourceAction.Modify)
      },
      removeProtection: {
        id: 'removeProtection',
        divide: true,
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return (
                  !isEmpty(val.sla_id) ||
                  val.protection_status ===
                    DataMap.Protection_Status.protected.value
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.RemoveProtection,
        label: this.i18n.get('protection_remove_protection_label'),
        onClick: data => {
          this.warningMessageService.create({
            content: this.i18n.get('protection_resource_delete_protect_label', [
              _map(data, 'name').join(',')
            ]),
            onOK: () => {
              this.batchOperateService.selfGetResults(
                item => {
                  return this.projectedObjectApiService.deleteV1ProtectedObjectsCyberDelete(
                    {
                      body: {
                        resource_ids: [item.uuid]
                      },
                      akDoException: false,
                      akOperationTips: false,
                      akLoading: false
                    }
                  );
                },
                cloneDeep(data),
                () => {
                  this.selectionData = [];
                  this.dataTable.setSelections([]);
                  this.dataTable.fetchData();
                },
                '',
                true
              );
            }
          });
        }
      },
      activeProtection: {
        id: 'activeProtection',
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return !isEmpty(val.sla_id) && !val.sla_status;
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
          this.batchOperateService.selfGetResults(
            item => {
              return this.projectedObjectApiService.activeV1ProtectedObjectsStatusActionActivateCyberPut(
                {
                  body: {
                    resource_ids: [item.uuid]
                  },
                  akDoException: false,
                  akOperationTips: false,
                  akLoading: false
                }
              );
            },
            cloneDeep(data),
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            },
            '',
            true
          );
        }
      },
      deactiveProtection: {
        id: 'deactiveProtection',
        divide: true,
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return !isEmpty(val.sla_id) && val.sla_status;
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.DeactivateProtection,
        disabledTips: this.i18n.get(
          'protection_partial_resources_deactive_label'
        ),
        label: this.i18n.get('protection_deactive_protection_label'),
        onClick: data => {
          this.warningMessageService.create({
            content: this.i18n.get('protection_deactivate_resource_tip_label', [
              _map(data, 'name').join(',')
            ]),
            onOK: () => {
              this.batchOperateService.selfGetResults(
                item => {
                  return this.projectedObjectApiService.deactivateV1ProtectedObjectsStatusActionDeactivateCyebrPut(
                    {
                      body: {
                        resource_ids: [item.uuid]
                      },
                      akDoException: false,
                      akOperationTips: false,
                      akLoading: false
                    }
                  );
                },
                cloneDeep(data),
                () => {
                  this.selectionData = [];
                  this.dataTable.setSelections([]);
                  this.dataTable.fetchData();
                },
                '',
                true
              );
            }
          });
        }
      },
      manualDetecte: {
        id: 'manualDetecte',
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return !isEmpty(val.sla_id);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('explore_manual_detecte_label'),
        onClick: data => this.manualDetecte(data)
      }
    };

    this.optItems = cloneDeep(getPermissionMenuItem(values(opts)));
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
        }
      },
      {
        key: 'parentName',
        name: this.i18n.get('protection_storage_device_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: []
        },
        cellRender: this.storageDeviceTpl
      },
      {
        key: 'tenantName',
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'sla_name',
        name: this.i18n.get('explore_intelligent_detection_policy_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.policyTpl
      },
      {
        key: 'sla_compliance',
        name: this.i18n.get('explore_compliance_label'),
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
        key: 'learningStatus',
        name: this.i18n.get('explore_learning_status_label'),
        cellRender: this.learningTpl
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
        autoPolling: CommonConsts.TIME_INTERVAL,
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
      opts.protect,
      removeProtectionBtn,
      opts.activeProtection,
      deactiveBtn,
      opts.manualDetecte
    ]);
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType: [DataMap.Resource_Type.LocalFileSystem.value]
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditions = JSON.parse(filters.conditions_v2);
      if (conditions.parentName) {
        assign(defaultConditions, {
          environment: {
            name: tail(conditions.parentName)
          }
        });
        delete conditions.parentName;
      }
      assign(defaultConditions, conditions);
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
            assign(item, {
              tenantName: item.extendInfo?.tenantName,
              sub_type: item.subType
            });
            extendSlaInfo(item);
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          total: res.totalCount,
          data: res.records
        };
        this.refreshFileSystem.emit();
      });
  }

  protect(datas, action: ProtectResourceAction) {
    const data = size(datas) > 1 ? datas : datas[0];
    this.protectService.openProtectModal(
      DataMap.Resource_Type.LocalFileSystem.value,
      action,
      {
        width: 780,
        data: data,
        onOK: () => {
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.dataTable.fetchData();
        }
      }
    );
  }

  // 手动侦测
  manualDetecte(rowData) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_manual_detecte_label'),
      lvModalKey: '_manual-detecte-policy',
      lvWidth: this.i18n.isEn
        ? MODAL_COMMON.normalWidth + 100
        : MODAL_COMMON.normalWidth,
      lvContent: ManualDetecteComponent,
      lvOkDisabled: false,
      lvComponentParams: {
        rowData
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as ManualDetecteComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as ManualDetecteComponent;
          content.onOK().subscribe(
            () => {
              resolve(true);
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            },
            () => resolve(false)
          );
        });
      }
    });
  }

  getPolicyDetail(item) {
    this.slaService.getAntiDetail({ uuid: item.sla_id }, []);
  }
}
