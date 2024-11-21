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
import {
  Component,
  OnInit,
  AfterViewInit,
  ViewChild,
  ChangeDetectorRef,
  TemplateRef
} from '@angular/core';
import {
  TableCols,
  TableData,
  TableConfig,
  Filters,
  ProTableComponent
} from 'app/shared/components/pro-table';
import { I18NService, SnmpApiService } from 'app/shared';
import { size } from 'lodash';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';

@Component({
  selector: 'aui-snmp-v3-engine',
  templateUrl: './snmp-v3-engine.component.html',
  styleUrls: ['./snmp-v3-engine.component.less']
})
export class SnmpV3EngineComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('engineIdTpl', { static: true }) engineIdTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private snmpApiService: SnmpApiService,
    private virtualScroll: VirtualScrollService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'nodeName',
        name: this.i18n.get('common_home_node_label'),
        width: 160
      },
      {
        key: 'engineId',
        name:
          this.i18n.get('system_snmp_v3_engine_id_label', ['/']) +
          this.i18n.get('system_snmp_v3_engine_id_label', ['(HEX)']),
        cellRender: this.engineIdTpl
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'engineId',
        columns: cols,
        virtualScroll: true,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        }
      },
      pagination: {
        showPageSizeOptions: false,
        winTablePagination: false,
        mode: 'simple'
      }
    };
  }

  getData(filters?: Filters) {
    this.snmpApiService.queryAllSnmpSecurityAgentUsingGET({}).subscribe(res => {
      this.tableData = {
        data: res,
        total: size(res)
      };
      this.cdr.detectChanges();
    });
  }
}
