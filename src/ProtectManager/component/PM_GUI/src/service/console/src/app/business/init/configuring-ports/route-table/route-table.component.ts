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
import { ChangeDetectionStrategy, Component, OnInit } from '@angular/core';
import { CommonConsts, DataMapService, I18NService } from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';

@Component({
  selector: 'aui-route-table',
  templateUrl: './route-table.component.html',
  styleUrls: ['./route-table.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class RouteTableComponent implements OnInit {
  data;
  tableConfig: TableConfig;
  tableData: TableData;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'TYPE',
        name: this.i18n.get('common_type_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('initRouteType')
        }
      },
      {
        key: 'DESTINATION',
        name: this.i18n.get('common_target_address_label')
      },
      {
        key: 'MASK',
        name: this.i18n.get('common_subnet_mask_prefix_label')
      },
      {
        key: 'GATEWAY',
        name: this.i18n.get('common_gateway_label')
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        async: false,
        scrollFixed: true,
        scroll: { y: '63vh' }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: true,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE
      }
    };
    this.tableData = {
      data: this.data,
      total: this.data.length
    };
  }
}
