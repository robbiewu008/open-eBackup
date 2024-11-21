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
import { ModalRef } from '@iux/live';
import { I18NService, CommonConsts } from 'app/shared';
import { TableConfig, TableData } from 'app/shared/components/pro-table';
import { size } from 'lodash';

@Component({
  selector: 'aui-tenant-detail',
  templateUrl: './tenant-detail.component.html',
  styleUrls: ['./tenant-detail.component.less']
})
export class TenantDetailComponent implements OnInit {
  rowItem: any;
  nodeTableConfig: TableConfig;
  nodeTableData: TableData;

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(private i18n: I18NService, private modal: ModalRef) {}

  ngOnInit(): void {
    this.initConfig();
    this.updateHeader();
    this.showData();
  }

  updateHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  initConfig() {
    this.nodeTableConfig = {
      table: {
        async: false,
        colDisplayControl: false,
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_username_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          }
        ]
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  showData() {
    this.nodeTableData = {
      data: JSON.parse(this.rowItem?.extendInfo?.vdcNames || '{}').map(item => {
        return { name: item };
      }),
      total: size(this.rowItem?.extendInfo?.vdcNames) || 0
    };
  }
}
