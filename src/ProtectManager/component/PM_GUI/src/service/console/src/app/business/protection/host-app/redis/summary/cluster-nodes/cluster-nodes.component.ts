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
  ChangeDetectionStrategy,
  ViewChild,
  AfterViewInit,
  Input
} from '@angular/core';
import {
  TableConfig,
  TableData,
  ProTableComponent,
  TableCols,
  Filters
} from 'app/shared/components/pro-table';
import { I18NService, DataMapService } from 'app/shared';
import { isArray, size } from 'lodash';

@Component({
  selector: 'aui-redis-cluster-nodes',
  templateUrl: './cluster-nodes.component.html',
  styleUrls: ['./cluster-nodes.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ClusterNodesComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  @Input() source;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'hostname',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Redis_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Redis_Status')
        }
      },
      {
        key: 'clientPath',
        name: this.i18n.get('common_client_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'ip',
        name: this.i18n.get('common_business_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'role',
        name: this.i18n.get('protection_running_mode_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Redis_Node_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Redis_Node_Type')
        }
      },
      {
        key: 'slot',
        name: this.i18n.get('common_slot_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        async: false,
        size: 'small',
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
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

  getShowTableData(data) {
    return data.map(item => {
      return {
        uuid: item.uuid,
        hostname: item.dependencies.agents[0].name,
        endpoint: item.dependencies.agents[0].endpoint,
        status: item.extendInfo.status,
        clientPath: item.extendInfo.clientPath,
        ip: item.extendInfo.ip,
        port: item.extendInfo.port,
        role: item.extendInfo.role,
        slot: item.extendInfo.slot
      };
    });
  }

  // 构造下页面所需要的展示数据
  getData(filters: Filters, args) {
    let nodes =
      this.getShowTableData(this.source?.dependencies?.children || []) || []; // 总数据
    let params;
    if (filters.conditions) {
      params = JSON.parse(filters.conditions);
      nodes = nodes.filter(item => {
        for (const key in params) {
          if (Object.prototype.hasOwnProperty.call(params, key)) {
            if (key === 'status' || key === 'role') {
              if (isArray(params[key])) {
                if (!params[key].includes(item[key])) {
                  return false;
                }
              }
            } else {
              if (item[key] !== params[key]) {
                return false;
              }
            }
          }
        }
        return true;
      });
    }
    this.tableData = {
      data: nodes,
      total: size(nodes)
    };
  }
}
