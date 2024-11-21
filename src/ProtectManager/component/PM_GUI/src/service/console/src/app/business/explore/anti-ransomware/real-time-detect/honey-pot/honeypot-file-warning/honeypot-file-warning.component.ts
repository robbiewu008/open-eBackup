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
import { DataMapService, I18NService } from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { size } from 'lodash';

@Component({
  selector: 'aui-honeypot-file-warning',
  templateUrl: './honeypot-file-warning.component.html',
  styleUrls: ['./honeypot-file-warning.component.less']
})
export class HoneypotFileWarningComponent implements OnInit {
  tableConfig: TableConfig;
  tableData: TableData;
  data;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private modal: ModalRef,
    private dataMapService: DataMapService
  ) {}

  ngOnInit(): void {
    this.getModalHeader();
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'fsName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'vstoreName',
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Batch_Result_Status')
        }
      },
      {
        key: 'desc',
        name: this.i18n.get('common_desc_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('fileHoneypotErrorStatus')
        }
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        async: false,
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };

    this.tableData = {
      data: this.data,
      total: size(this.data)
    };
  }

  getModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }
}
