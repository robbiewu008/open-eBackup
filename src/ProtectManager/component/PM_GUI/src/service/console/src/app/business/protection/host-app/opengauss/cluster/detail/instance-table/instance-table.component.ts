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
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  DataMap,
  DataMapService,
  extendSlaInfo,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, each, filter, isEmpty, isUndefined, map, size } from 'lodash';

@Component({
  selector: 'aui-instance-table',
  templateUrl: './instance-table.component.html',
  styleUrls: ['./instance-table.component.less']
})
export class InstanceTableComponent implements OnInit, AfterViewInit {
  tableData: TableData;
  tableConfig: TableConfig;
  selectionData: any[];

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @Input() data;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig(): void {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'instanceStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('openGauss_InstanceStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('openGauss_InstanceStatus')
        }
      },

      {
        key: 'sla_name',
        name: this.i18n.get('common_sla_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'protectionStatus',
        name: this.i18n.get('protection_protected_status_label'),
        width: 130,
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
        compareWith: 'uuid',
        columns: cols,
        size: 'small',
        colDisplayControl: false,
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
  getData(filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const defaultConditions = {
      subType: [DataMap.Resource_Type.OpenGauss_instance.value]
    };
    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.instanceStatus) {
        delete conditionsTemp.instanceStatus;
      }
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
      .subscribe((res: any) => {
        let resource = filter(res.records, item => {
          return item.parentUuid === this.data.uuid;
        });
        if (
          this.data.clusterState ===
          DataMap.opengauss_Clusterstate.unavailable.value
        ) {
          each(resource, item => {
            assign(item, {
              sub_type: item.subType,
              instanceStatus: DataMap.openGauss_InstanceStatus.offline.value
            });
            extendSlaInfo(item);
          });
        } else {
          each(resource, item => {
            assign(item, {
              sub_type: item.subType,
              instanceStatus: item.extendInfo.instanceState
            });
            extendSlaInfo(item);
          });
        }
        if (filters.conditions) {
          const conditions = JSON.parse(filters.conditions);
          if (size(conditions.instanceStatus) === 1) {
            resource = resource.filter(item => {
              return item.instanceStatus === conditions.instanceStatus[0];
            });
          }
        }
        this.tableData = {
          data: resource,
          total: size(resource)
        };
        if (!(!isUndefined(args) && args.isAutoPolling)) {
          this.selectionData = [];
          this.dataTable.setSelections([]);
        }
      });
  }
}
