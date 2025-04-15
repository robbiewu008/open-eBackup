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
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, each, find, size } from 'lodash';

@Component({
  selector: 'aui-mongodb-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit, AfterViewInit {
  source;
  subType = DataMap.Resource_Type.MongoDB.value;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initTableConfig();
    this.getNodes();
  }

  initDetailData(data) {
    this.source = data;
  }

  initTableConfig() {
    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label')
          },
          {
            key: 'nodeStatus',
            name: this.i18n.get('common_status_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('resource_LinkStatus_Special')
            }
          },
          {
            key: 'endpoint',
            name: this.i18n.get('common_ip_address_label')
          },
          {
            key: 'port',
            name: this.i18n.get('common_port_label')
          }
        ],
        compareWith: 'uuid',
        colDisplayControl: false,
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showPageSizeOptions: false
      }
    };
  }

  getNodes() {
    const nodes = JSON.parse(this.source?.extendInfo?.clusterNodes);
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.source?.uuid })
      .subscribe((res: any) => {
        const table = [];
        if (
          this.source.subType ===
          DataMap.Resource_Type.MongodbSingleInstance.value
        ) {
          table.push({
            name: res.dependencies?.agents[0]?.name,
            endpoint: this.source.extendInfo?.serviceIp,
            port: this.source.extendInfo?.servicePort,
            nodeStatus: this.source.linkStatus
          });
        } else {
          each(res.dependencies?.children, item => {
            const node = find(
              nodes,
              n =>
                n.agentHost ===
                `${item.extendInfo?.serviceIp}:${item.extendInfo?.servicePort}`
            );
            table.push(
              assign(node, {
                name: item.name,
                endpoint: item.extendInfo?.serviceIp,
                port: item.extendInfo?.servicePort
              })
            );
          });
        }

        this.tableData = {
          data: table,
          total: size(table)
        };
      });
  }
}
