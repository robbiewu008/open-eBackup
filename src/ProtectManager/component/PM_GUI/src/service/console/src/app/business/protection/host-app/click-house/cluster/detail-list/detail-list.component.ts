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
  OnInit,
  ChangeDetectorRef,
  AfterViewInit,
  ViewChild,
  TemplateRef
} from '@angular/core';
import {
  DataMap,
  I18NService,
  DataMapService,
  ResourceType,
  extendSlaInfo,
  ProtectedResourceApiService
} from 'app/shared';
import {
  TableCols,
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { SlaService } from 'app/shared/services/sla.service';
import { assign, each, eq, isEmpty, isNil, size } from 'lodash';
import { map } from 'rxjs/operators';

enum ACTIVE_IDS {
  DATABASE = 'database',
  TABLE_SET = 'tableset'
}

@Component({
  selector: 'aui-detail-list',
  templateUrl: './detail-list.component.html',
  styleUrls: ['./detail-list.component.css']
})
export class DetailListComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  activeId: string;
  sourceType: string;
  source;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    public dataMapService: DataMapService,
    private slaService: SlaService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initTable();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  initDetailData(data, { activeId }) {
    this.source = data;
    this.sourceType = data.resourceType;
    this.activeId = activeId;
  }

  initTable() {
    const cols: TableCols[] = eq(this.activeId, ACTIVE_IDS.DATABASE)
      ? [
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
                iconPos: 'flow-text'
              }
            }
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
          }
        ]
      : [
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
                iconPos: 'flow-text'
              }
            }
          },
          {
            key: 'parentName',
            name: this.i18n.get('protection_host_database_name_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
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
          }
        ];

    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        fetchData: (filters: Filters, args) => {
          eq(this.activeId, ACTIVE_IDS.DATABASE)
            ? this.getDatabase(filters, args)
            : this.getTableset(filters, args);
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getData(filters, args, defaultConditions) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading: !isNil(args) && args.isAutoPolling ? !args.isAutoPolling : true
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
            assign(item, {
              sub_type: item.subType,
              parentName: eq(this.activeId, ACTIVE_IDS.DATABASE)
                ? item.environment?.name
                : item.parentName,
              auth_status: DataMap.Verify_Status.true.value
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
        this.cdr.detectChanges();
      });
  }

  getDatabase(filters: Filters, args) {
    const defaultConditions = {
      subType: DataMap.Resource_Type.ClickHouse.value,
      type: ResourceType.DATABASE,
      parentUuid: this.source.uuid
    };
    this.getData(filters, args, defaultConditions);
  }

  getTableset(filters: Filters, args) {
    const defaultConditions = {
      subType: DataMap.Resource_Type.ClickHouse.value,
      type: ResourceType.TABLE_SET,
      rootUuid: this.source.uuid
    };
    this.getData(filters, args, defaultConditions);
  }
}
