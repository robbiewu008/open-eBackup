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
import { DataMap, DataMapService, I18NService } from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, each, map, size } from 'lodash';

@Component({
  selector: 'aui-dameng-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit, AfterViewInit {
  source;
  datas;
  sourceType = DataMap.Resource_Type.Dameng.value;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
    this.updataTable(this.source);
  }
  initDetailData(data) {
    this.source = data;
    assign(this.source, { link_status: this.source.linkStatus });
  }

  updataTable(res) {
    const datas = JSON.parse(res.extendInfo.nodes);
    each(datas, item => {
      assign(item, {
        instanceStatus: item.extendInfo.instanceStatus,
        port: item.extendInfo.port,
        authType: item.extendInfo.authType,
        role: item.extendInfo.role
      });
    });
    this.tableData = {
      data: datas,
      total: size(datas)
    };
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_address_label')
      },
      {
        key: 'instanceStatus',
        name: this.i18n.get('common_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label')
      },
      {
        key: 'authType',
        name: this.i18n.get('protection_auth_method_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Dameng_Auth_Method')
        }
      },
      {
        key: 'role',
        name: this.i18n.get('protection_running_mode_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Dameng_Node_Type')
        }
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }
}
