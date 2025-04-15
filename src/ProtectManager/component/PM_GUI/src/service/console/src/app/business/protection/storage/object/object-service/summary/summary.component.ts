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
import { DataMap, GlobalService, I18NService } from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { each, size } from 'lodash';

@Component({
  selector: 'aui-object-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  dbInfo;
  type;
  dataMap = DataMap;
  tableConfig: TableConfig;
  tableData: TableData;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(private i18n: I18NService, public globalService: GlobalService) {}

  ngOnInit(): void {
    this.initConfig();
    this.initData();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'path',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'rule',
        name: this.i18n.get('protection_object_filter_rules_label'),
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
        async: false,
        virtualScroll: true,
        virtualItemHeight: 32,
        scrollFixed: true,
        scroll: { y: '420px' }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  initDetailData(data) {
    this.source = data;
    this.type = DataMap.Resource_Type.ObjectSet.value;
  }

  initData() {
    const data = JSON.parse(this.source.extendInfo.bucketList);
    let path = [];
    each(data, item => {
      let prefix = item?.prefix;
      if (prefix) {
        prefix = prefix.join(', ');
      }
      path.push({
        path: item.name,
        rule: prefix ? `Prefix: ${prefix}` : ''
      });
    });
    this.tableData = {
      data: path,
      total: size(data)
    };
  }
}
