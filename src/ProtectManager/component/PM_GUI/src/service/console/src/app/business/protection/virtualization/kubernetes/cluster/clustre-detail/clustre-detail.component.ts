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
import { AfterViewInit, Component, OnInit, ViewChild } from '@angular/core';
import {
  CommonConsts,
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
import {
  assign,
  each,
  filter,
  includes,
  isEmpty,
  map,
  set,
  size
} from 'lodash';

@Component({
  selector: 'aui-clustre-detail',
  templateUrl: './clustre-detail.component.html',
  styleUrls: ['./clustre-detail.component.less']
})
export class ClustreDetailComponent implements OnInit, AfterViewInit {
  rowItem: any;
  subType;
  nodeTableConfig: TableConfig;
  tableConfigN: TableConfig;
  tableConfigF: TableConfig;
  tableDataN: TableData;
  tableDataF: TableData;
  tableConfigS: TableConfig;
  tableDataS: TableData;
  nodeTableData: TableData;
  dataMap = DataMap;

  @ViewChild('dataTableN', { static: false }) dataTableN: ProTableComponent;
  @ViewChild('dataTableF', { static: false }) dataTableF: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    if (this.dataTableN) {
      this.dataTableN.fetchData();
    }
    if (this.dataTableF) {
      this.dataTableF.fetchData();
    }
  }

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        sort: true,
        filter: {
          type: 'search'
        }
      },
      {
        key: 'parentName',
        name: this.i18n.get('protection_belong_namespace_label'),
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
            overflow: true
          }
        }
      },
      {
        key: 'sla_compliance',
        name: this.i18n.get('common_sla_compliance_label'),
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
      }
    ];
    this.tableConfigN = {
      table: {
        columns: filter(cols, item => {
          return item.key !== 'parentName';
        }),
        async: true,
        colDisplayControl: false,
        fetchData: (filter: Filters, args: {}) => {
          this.getSource(filter, {
            ...args,
            subType:
              this.subType ===
              DataMap.Resource_Type.kubernetesClusterCommon.value
                ? [DataMap.Resource_Type.kubernetesNamespaceCommon.value]
                : [DataMap.Resource_Type.KubernetesNamespace.value]
          });
        }
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS
      }
    };
    this.tableConfigF = {
      table: {
        columns: cols,
        async: true,
        colDisplayControl: false,
        fetchData: (filter: Filters, args: {}) => {
          this.getSource(filter, {
            ...args,
            subType:
              this.subType ===
              DataMap.Resource_Type.kubernetesClusterCommon.value
                ? [DataMap.Resource_Type.kubernetesDatasetCommon.value]
                : [DataMap.Resource_Type.KubernetesStatefulset.value]
          });
        }
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS
      }
    };
    this.tableConfigS = {
      table: {
        async: false,
        colDisplayControl: false,
        columns: [
          {
            key: 'ip',
            name: this.i18n.get('common_ip_domain_name_label'),
            sort: true,
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          }
        ]
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
    if (this.subType === DataMap.Resource_Type.kubernetesClusterCommon.value) {
      return;
    }
    const storages = [];
    for (let key in this.rowItem?.extendInfo) {
      if (key.indexOf('storage_') !== -1) {
        storages.push(JSON.parse(this.rowItem.extendInfo[key]));
      }
    }
    this.tableDataS = {
      data: storages,
      total: size(storages)
    };
    this.nodeTableConfig = {
      table: {
        async: false,
        colDisplayControl: false,
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label')
          },
          {
            key: 'endpoint',
            name: this.i18n.get('common_ip_address_label')
          },
          {
            key: 'linkStatus',
            name: this.i18n.get('common_status_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('resource_Host_LinkStatus')
            }
          }
        ]
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
    this.nodeTableData = {
      data: map(this.rowItem.dependencies?.agents, item => {
        return assign(item, {
          linkStatus: +item.linkStatus
        });
      }),
      total: size(this.rowItem.dependencies?.agents)
    };
  }

  getSource(filters: Filters, args: any) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    const defaultConditions = {
      ...args,
      path: [['=~'], this.rowItem?.path]
    };

    if (this.subType === DataMap.Resource_Type.kubernetesClusterCommon.value) {
      delete defaultConditions.path;
      set(defaultConditions, 'parentUuid', this.rowItem.uuid);
    }

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

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        assign(item, {
          sub_type: item.subType,
          cluster: item.environment?.name,
          endpoint: item.environment?.endpoint,
          parentName: item.parentName
        });
        extendSlaInfo(item);
      });
      if (
        includes(
          [
            DataMap.Resource_Type.KubernetesNamespace.value,
            DataMap.Resource_Type.kubernetesNamespaceCommon.value
          ],
          args.subType[0]
        )
      ) {
        this.tableDataN = {
          data: res.records,
          total: res.totalCount
        };
      } else {
        this.tableDataF = {
          data: res.records,
          total: res.totalCount
        };
      }
    });
  }
}
