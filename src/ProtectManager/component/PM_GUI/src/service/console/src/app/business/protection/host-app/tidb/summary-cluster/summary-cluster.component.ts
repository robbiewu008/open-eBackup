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
import { Component, OnInit, AfterViewInit, ViewChild } from '@angular/core';
import {
  DataMap,
  I18NService,
  GlobalService,
  DataMapService,
  ResourceType,
  ProtectedResourceApiService
} from 'app/shared';
import {
  TableCols,
  TableConfig,
  ProTableComponent,
  Filters
} from 'app/shared/components/pro-table';
import { get, isArray, isNil, map as __map, size } from 'lodash';

@Component({
  selector: 'aui-summary-tidb-cluster',
  templateUrl: './summary-cluster.component.html',
  styleUrls: ['./summary-cluster.component.less']
})
export class SummaryClusterComponent implements OnInit, AfterViewInit {
  source;
  sourceType;
  dataMap = DataMap;
  tableConfig;
  tableData;
  firstformItems = [];
  secondformItems = [];
  data;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private globalService: GlobalService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.getData();
    this.initTable();
    this.globalService.getState('registerTidbCluster').subscribe(res => {
      this.source = { ...this.source, ...res };
      this.dataTable.fetchData();
    });
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  getData() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.source.extendInfo.tiupUuid
      })
      .subscribe(res => {
        this.data = res;
        this.initFormItems();
      });
  }

  initFormItems() {
    this.firstformItems = [
      [
        {
          key: 'name',
          value: this.source.name,
          label: this.i18n.get('common_name_label')
        }
      ],
      [
        {
          key: 'version',
          value: this.source.extendInfo.version,
          label: this.i18n.get('common_version_label')
        }
      ],
      [
        {
          key: 'owner',
          value: this.source.extendInfo.owner,
          label: this.i18n.get('common_owned_user_label')
        }
      ],
      [
        {
          key: 'clusterName',
          value: this.source.extendInfo.clusterName,
          label: this.i18n.get('protection_internal_cluster_label')
        }
      ]
    ];
    this.secondformItems = [
      [
        {
          key: 'host',
          value: `${this.data.name}(${this.data.endpoint})`,
          label: this.i18n.get('protection_proxy_host_label')
        }
      ],
      [
        {
          key: 'node',
          value: this.data.endpoint,
          label: this.i18n.get('protection_tiup_node_label')
        }
      ]
    ];
  }

  initDetailData(data) {
    this.source = data;
    this.sourceType = data.resourceType;
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'id',
        name: this.i18n.get('ID'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'role',
        name: this.i18n.get('common_role_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'host',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        fetchData: (filters: Filters) => {
          this.getNodeList(filters);
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getNodeList(filters: Filters) {
    const params = {
      resourceId: this.source.uuid
    };
    this.protectedResourceApiService.ShowResource(params).subscribe(res => {
      const nodes = this.getShowTableData(
        JSON.parse(get(res, ['extendInfo', 'clusterInfoList']))
      ); // 总数据
      if (filters.conditions) {
        const conditions = JSON.parse(filters.conditions);
        const _nodes = nodes.filter(item => {
          for (const key in conditions) {
            if (Object.prototype.hasOwnProperty.call(conditions, key)) {
              if (key === 'status') {
                if (isArray(conditions[key])) {
                  if (!conditions[key].includes(item[key])) {
                    return false;
                  }
                }
              } else {
                if (!item[key].includes(conditions[key])) {
                  return false;
                }
              }
            }
          }
          return true;
        });
        this.tableData = {
          data: _nodes,
          total: size(_nodes)
        };
      } else {
        this.tableData = {
          data: nodes,
          total: size(nodes)
        };
      }
    });
  }

  getShowTableData(data) {
    if (isNil(data)) return [];

    return data?.map(item => {
      return {
        id: item.id,
        host: item.hostManagerIp,
        role: item.role
      };
    });
  }
}
