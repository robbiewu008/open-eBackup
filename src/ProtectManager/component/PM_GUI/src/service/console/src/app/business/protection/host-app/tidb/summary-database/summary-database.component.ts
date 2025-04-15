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
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import { TableConfig, TableData } from 'app/shared/components/pro-table';
import {
  toString,
  assign,
  defer,
  size,
  map,
  isEmpty,
  each,
  find
} from 'lodash';

@Component({
  selector: 'aui-summary-database',
  templateUrl: './summary-database.component.html',
  styleUrls: ['./summary-database.component.less']
})
export class SummaryDatabaseComponent implements OnInit {
  source;
  dbInfo;
  type = DataMap.Resource_Type.tidbDatabase.value;
  dataMap = DataMap;

  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('instNameTpl', { static: true }) instNameTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {}

  initDetailData(data) {
    this.source = data;
    this.getInfo(data);
  }

  getInfo(data) {
    this.dbInfo = assign(data, {
      db_type: data.type,
      link_status: toString(data.link_status)
    });
  }
}
