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
  I18NService,
  DataMapService,
  ProtectedResourceApiService,
  DataMap
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { get, includes, map, map as _map, toString as _toString } from 'lodash';

@Component({
  selector: 'aui-light-cloud-gaussdb-project-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  data = {} as any;
  formItems = [];
  tableConfig: TableConfig;
  tableData;
  resSubType;
  dataMap = DataMap;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  constructor(
    private i18n: I18NService,
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
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
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
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.data.uuid })
      .subscribe(res => {
        const dataList =
          get(res, 'dependencies.agents') ||
          get(res, 'dependencies.children') ||
          [];
        this.tableData = {
          data: map(dataList, item => {
            return {
              name: item.name,
              endpoint: item.endpoint || item.path,
              linkStatus: item.linkStatus || item.extendInfo?.linkStatus
            };
          }),
          total: dataList.length
        };
      });
  }
}
