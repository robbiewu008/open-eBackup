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
import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService } from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { get } from 'lodash';

@Component({
  selector: 'aui-pdb-set-summary',
  templateUrl: './pdb-set-summary.component.html',
  styleUrls: ['./pdb-set-summary.component.less']
})
export class PdbSetSummaryComponent implements OnInit {
  source;
  dbInfo;
  type;
  dataMap = DataMap;
  tableConfig: TableConfig;
  tableData: TableData;

  constructor(private i18n: I18NService) {}

  ngOnInit() {
    this.getPDBs();
  }

  initDetailData(data) {
    this.type = data.subType || data.resourceType;
    this.source = data;
    this.source.linkStatus = data.extendInfo?.linkStatus;
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
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
        scroll: {
          y: '380px'
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getPDBs() {
    const pdb = JSON.parse(get(this.source, 'extendInfo.pdb', '[]')).map(
      item => ({
        name: item
      })
    );
    this.tableData = {
      data: pdb,
      total: pdb.length
    };
  }
}
