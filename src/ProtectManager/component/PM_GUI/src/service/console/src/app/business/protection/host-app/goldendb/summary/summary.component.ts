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
import { Component, OnInit, ViewChild } from '@angular/core';
import {
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, each, find, get, map, size } from 'lodash';

@Component({
  selector: 'aui-goldendb-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  resourceType = DataMap.Resource_Type;
  subType = DataMap.Resource_Type.goldendb.value;
  tableConfig: TableConfig;
  tableData: TableData;
  dataTableConfig: TableConfig;
  dataTableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.initDataConfig();
    this.getDetails();
  }

  initDetailData(data) {
    this.source = assign(data, {
      subType: data.sub_type || data?.subType
    });
    this.subType = data.sub_type || data.subType;
  }

  initConfig() {
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
        key: 'parentName',
        name: this.i18n.get('protection_clients_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'osUser',
        name: this.i18n.get('common_username_label')
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
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        async: false
      },
      pagination: null
    };
  }

  initDataConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('protection_data_node_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'group',
        name: this.i18n.get('protection_goldendb_part_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'parentName',
        name: this.i18n.get('protection_clients_label')
      },
      {
        key: 'osUser',
        name: this.i18n.get('common_username_label')
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
      }
    ];

    this.dataTableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        async: false
      },
      pagination: null
    };
  }

  getDetails() {
    if (!this.source) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.source.uuid })
      .subscribe(res => {
        const agents = get(res, 'dependencies.agents');
        let tableData = [];
        const instances = JSON.parse(get(res, 'extendInfo.clusterInfo', '{}'));

        each(get(instances, 'group', []), item => {
          each(item.mysqlNodes, node => {
            const nodeStatus = get(
              find(agents, { uuid: node.parentUuid }),
              'linkStatus'
            );

            assign(node, {
              group: `DBGroup${item.groupId}`,
              linkStatus: nodeStatus
            });
          });
          tableData = [...tableData, ...item.mysqlNodes];
        });
        this.dataTableData = {
          data: tableData,
          total: size(tableData)
        };

        const gtm = get(instances, 'gtm', []);

        each(gtm, item => {
          const nodeStatus = get(
            find(agents, { uuid: item.parentUuid }),
            'linkStatus'
          );

          assign(item, {
            linkStatus: nodeStatus
          });
        });
        this.tableData = {
          data: gtm,
          total: size(gtm)
        };
      });
  }
}
