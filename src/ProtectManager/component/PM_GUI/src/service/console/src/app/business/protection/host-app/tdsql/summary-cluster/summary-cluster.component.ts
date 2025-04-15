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
import { Component, OnInit, ChangeDetectorRef } from '@angular/core';
import {
  I18NService,
  DataMapService,
  ProtectedResourceApiService,
  DataMap
} from 'app/shared';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';
import { find, get, map, size, toString as _toString } from 'lodash';

@Component({
  selector: 'aui-summary-tdsql-cluster',
  templateUrl: './summary-cluster.component.html',
  styleUrls: ['./summary-cluster.component.less']
})
export class SummaryClusterComponent implements OnInit {
  ossTableData;
  schedulerTableData;
  resSubType;
  formItems = [];
  data = {} as any;
  dataMap = DataMap;
  ossTableConfig: TableConfig;
  schedulerTableConfig: TableConfig;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initFormItems();
    this.initConfig();
    this.getDetails();
  }

  initDetailData(data: any) {
    this.data = data;
  }

  initFormItems() {
    this.formItems = [
      [
        {
          key: 'name',
          value: this.data.name,
          label: this.i18n.get('common_name_label')
        }
      ]
    ];
  }

  initConfig() {
    const ossCols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('protection_clients_label')
      },
      {
        key: 'ip',
        name: this.i18n.get('common_dataplane_ip_label')
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label')
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      }
    ];
    const schedulerCols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('protection_clients_label')
      },
      {
        key: 'ip',
        name: this.i18n.get('common_dataplane_ip_label')
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      }
    ];

    this.ossTableConfig = {
      table: {
        compareWith: 'endpoint',
        columns: ossCols,
        colDisplayControl: false
      },
      pagination: null
    };

    this.schedulerTableConfig = {
      table: {
        compareWith: 'endpoint',
        columns: schedulerCols,
        colDisplayControl: false
      },
      pagination: null
    };
  }

  getDetails() {
    if (!this.data) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.data.uuid })
      .subscribe(res => {
        const clusterInfo = JSON.parse(get(res, 'extendInfo.clusterInfo'));
        const ossNodes = get(clusterInfo, 'ossNodes');
        const schedulerNodes = get(clusterInfo, 'schedulerNodes');
        const agents = get(res, 'dependencies.agents');

        this.ossTableData = {
          data: map(ossNodes, item => {
            const agent: any = find(agents, { uuid: item.parentUuid });
            return {
              name: `${agent?.name}(${agent?.endpoint})`,
              ip: item.ip,
              port: item.port,
              linkStatus: item?.linkStatus
            };
          }),
          total: size(ossNodes)
        };
        this.schedulerTableData = {
          data: map(schedulerNodes, item => {
            const agent: any = find(agents, { uuid: item.parentUuid });
            return {
              name: `${agent?.name}(${agent?.endpoint})`,
              ip: item.ip,
              linkStatus: item?.linkStatus
            };
          }),
          total: size(schedulerNodes)
        };
        this.cdr.detectChanges();
      });
  }
}
