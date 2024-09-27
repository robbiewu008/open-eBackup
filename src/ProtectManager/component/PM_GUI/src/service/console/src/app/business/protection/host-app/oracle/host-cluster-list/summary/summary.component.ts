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
  extendSlaInfo,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, defer, each, isEmpty, size } from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-summary-cluster-list',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit, AfterViewInit {
  rowItem: any;

  nodeTableConfig: TableConfig;
  nodeTableData: TableData;
  databaseTableConfig: TableConfig;
  databaseTableData: TableData;

  @ViewChild('clusterNode', { static: false }) clusterNode: ProTableComponent;
  @ViewChild('databaseTable', { static: false })
  databaseTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit() {
    if (this.databaseTable) {
      this.databaseTable.fetchData();
    }
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
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
              config: this.dataMapService.toArray('resource_LinkStatus_Special')
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

    defer(() => {
      this.nodeTableData = {
        data: this.rowItem.dependencies?.agents,
        total: size(this.rowItem.dependencies?.agents)
      };
    });

    this.databaseTableConfig = {
      table: {
        compareWith: 'uuid',
        size: 'small',
        colDisplayControl: false,
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label')
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
        ],
        fetchData: (filters: Filters) => this.getDatabase(filters)
      },
      pagination: {
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getDatabase(filters: Filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const defaultConditions = {
      subType: [
        DataMap.Resource_Type.oracle.value,
        DataMap.Resource_Type.oracleCluster.value
      ],
      parentUuid: this.rowItem?.uuid
    };
    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      assign(defaultConditions, conditionsTemp);
    }
    assign(params, {
      conditions: JSON.stringify(defaultConditions)
    });
    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            extendSlaInfo(item);
          });
          return res;
        })
      )
      .subscribe(res => {
        this.databaseTableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }
}
