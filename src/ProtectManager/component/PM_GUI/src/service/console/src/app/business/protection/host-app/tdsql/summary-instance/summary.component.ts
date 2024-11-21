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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
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
  selector: 'aui-tdsql-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  resourceType = DataMap.Resource_Type;
  subType = DataMap.Resource_Type.tdsqlInstance.value;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initConfig();
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
        key: 'setId',
        name: this.i18n.get('SETID')
      },
      {
        key: 'name',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'type',
        name: this.i18n.get('common_type_label')
      },
      {
        key: 'ip',
        name: this.i18n.get('common_dataplane_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'priority',
        name: this.i18n.get('protection_priority_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('tdsqlNodePriority')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('tdsqlNodePriority')
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

  getDetails() {
    if (!this.source) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.source.uuid })
      .subscribe(res => {
        const clusterInstanceInfo = JSON.parse(
          get(res, 'extendInfo.clusterInstanceInfo')
        );
        let dataNodes = [];
        const agents = get(res, 'dependencies.agents');

        each(get(clusterInstanceInfo, 'groups'), group => {
          dataNodes = [
            ...dataNodes,
            ...map(get(group, 'dataNodes'), node => {
              return {
                ...node,
                setId: group.setId
              };
            })
          ];
        });

        this.tableData = {
          data: map(dataNodes, item => {
            const agent: any = find(agents, { uuid: item.parentUuid });
            return {
              setId: item.setId,
              name: `${agent?.name}(${agent?.endpoint})`,
              ip: item.ip,
              type: this.dataMapService.getLabel(
                'tdsqlDataNodeType',
                Number(item.isMaster)
              ),
              priority: item.priority,
              linkStatus: item?.linkStatus
            };
          }),
          total: size(dataNodes)
        };
      });
  }
}
