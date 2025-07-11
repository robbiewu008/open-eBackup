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
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  disableDeactiveProtectionTips,
  extendSlaInfo,
  getPermissionMenuItem,
  I18NService,
  OperateItems,
  ProtectedResourceApiService,
  ProtectResourceAction,
  ResourceType
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { ProtectService } from 'app/shared/services/protect.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  includes,
  isEmpty,
  isString,
  last,
  map as _map,
  reject,
  size,
  values
} from 'lodash';

@Component({
  selector: 'aui-fusion-cluster-summary',
  templateUrl: './fusion-cluster-summary.component.html',
  styleUrls: ['./fusion-cluster-summary.component.less']
})
export class FusionClusterSummaryComponent implements OnInit, AfterViewInit {
  resourceType = ResourceType;
  dataMap = DataMap;
  tableConfig: TableConfig;
  CNATableConfig: TableConfig;
  source;
  optItems = [];

  type = ResourceType.CNA;
  tableData = {
    data: [],
    total: 0
  };

  CNATableData = {
    data: [],
    total: 0
  };

  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('CNADataTable', { static: false }) CNADataTable: ProTableComponent;
  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private protectService: ProtectService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private takeManualBackupService: TakeManualBackupService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    if (this.dataTable) {
      this.dataTable.fetchData();
    }
    if (this.CNADataTable) {
      this.CNADataTable.fetchData();
    }
  }

  refreshData() {
    if (this.dataTable) {
      this.dataTable.fetchData();
    }
    if (this.CNADataTable) {
      this.CNADataTable.fetchData();
    }
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      protect: {
        id: 'protect',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  isEmpty(val.sla_id) &&
                  val.protection_status !==
                    DataMap.Protection_Status.creating.value
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.Protection,
        label: this.i18n.get('common_protect_label'),
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
                return !isEmpty(val.sla_id);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ModifyProtection,
        label: this.i18n.get('common_resource_protection_modify_label'),
        onClick: data => {
          this.protect(
            data,
            ProtectResourceAction.Modify,
            this.i18n.get('protection_modify_protection_label'),
            data
          );
        }
      },
      removeProtection: {
        id: 'removeProtection',
        divide: true,
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return !isEmpty(val.sla_id);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.RemoveProtection,
        label: this.i18n.get('protection_remove_protection_label'),
        onClick: data => {
          this.protectService
            .removeProtection(_map(data, 'uuid'), _map(data, 'name'))
            .subscribe(() => this.refreshData());
        }
      },
      activeProtection: {
        id: 'activeProtection',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
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
          this.protectService
            .activeProtection(_map(data, 'uuid'))
            .subscribe(() => this.refreshData());
        }
      },
      deactiveProtection: {
        id: 'deactiveProtection',
        divide: true,
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return !isEmpty(val.sla_id) && val.sla_status;
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
            .subscribe(res => this.refreshData());
        }
      },
      manualBackup: {
        id: 'manualBackup',
        divide: true,
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return !isEmpty(val.sla_id);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('common_manual_backup_label'),
        onClick: data => {
          this.takeManualBackupService.execute(
            assign(data[0], {
              resource_id: data[0].uuid,
              resource_type: data[0].sub_type
            }),
            () => this.refreshData()
          );
        }
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

    const cols1: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];
    const cols2: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'vmId',
        name: this.i18n.get('common_virtual_machine_id_label')
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('fcVMLinkStatus')
            .filter(item => {
              return [
                DataMap.fcVMLinkStatus.running.value,
                DataMap.fcVMLinkStatus.stopped.value,
                DataMap.fcVMLinkStatus.unknown.value
              ].includes(item.value);
            })
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('fcVMLinkStatus').filter(item => {
            return [
              DataMap.fcVMLinkStatus.running.value,
              DataMap.fcVMLinkStatus.stopped.value,
              DataMap.fcVMLinkStatus.unknown.value
            ].includes(item.value);
          })
        }
      }
    ];
    const cols: TableCols[] = [
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
        key: 'operation',
        width: 130,
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

    this.CNATableConfig = {
      table: {
        async: true,
        size: 'small',
        columns: [...cols1, ...cols],
        compareWith: 'uuid',
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.uuid;
        },
        fetchData: (filter: Filters, args: {}) => {
          this.getCNAData(filter, args);
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
    this.tableConfig = {
      table: {
        async: true,
        size: 'small',
        columns: [...cols2, ...cols],
        compareWith: 'uuid',
        scroll: {
          y: '300px'
        },
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.uuid;
        },
        fetchData: (filter: Filters, args: {}) => {
          this.getData(filter, args);
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  selectIndexChange(event) {
    if (event === 'cnaTable') {
      this.type = ResourceType.CNA;
      defer(() => {
        this.CNADataTable.fetchData();
      });
    } else {
      this.type = ResourceType.VM;
      defer(() => {
        this.dataTable.fetchData();
      });
    }
  }

  initDetailData(data: any) {
    this.source = data;
  }

  getData(filters: Filters, args: any) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const defaultConditions = {
      path: [['=~'], this.source.path],
      type: ['VM']
    };
    if (!isEmpty(filters.conditions_v2) && isString(filters.conditions_v2)) {
      assign(defaultConditions, JSON.parse(filters.conditions_v2));
    }
    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        assign(item, {
          sub_type: item.subType,
          cluster: item.environment?.name,
          endpoint: item.environment?.endpoint,
          parentName: item.parentName,
          status: item.extendInfo?.status,
          vmId: last(item.extendInfo?.moReference?.split('/'))
        });
        extendSlaInfo(item);
      });
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
    });
  }

  getCNAData(filters: Filters, args: any) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const defaultConditions = {
      path: [['=~'], this.source.path],
      type: ResourceType.HOST
    };
    if (!isEmpty(filters.conditions_v2) && isString(filters.conditions_v2)) {
      assign(defaultConditions, JSON.parse(filters.conditions_v2));
    }
    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        assign(item, {
          sub_type: item.subType,
          cluster: item.environment?.name,
          endpoint: item.environment?.endpoint,
          parentName: item.parentName,
          status: item.extendInfo?.status
        });
        extendSlaInfo(item);
      });
      this.CNATableData = {
        data: res.records,
        total: res.totalCount
      };
    });
  }

  protect(data, action: ProtectResourceAction, header?: string, refreshData?) {
    if (this.type === ResourceType.CNA) {
      each(data, item => {
        assign(item, { vmNumber: item?.extendInfo?.vmNumber });
      });
    }
    this.protectService.openProtectModal(
      this.type === ResourceType.CNA
        ? DataMap.Resource_Type.FusionComputeCNA.value
        : DataMap.Resource_Type.fusionComputeVirtualMachine.value,
      action,
      {
        width: 780,
        data,
        onOK: () => {
          if (this.type === ResourceType.CNA) {
            this.CNADataTable.fetchData();
          } else {
            this.dataTable.fetchData();
          }
        }
      }
    );
  }
}
