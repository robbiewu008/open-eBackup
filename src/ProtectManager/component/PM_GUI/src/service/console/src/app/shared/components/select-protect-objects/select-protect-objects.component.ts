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
  AfterViewInit,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  Pipe,
  PipeTransform,
  ViewChild
} from '@angular/core';
import { I18NService } from 'app/shared/services';
import {
  assign,
  cloneDeep,
  each,
  filter,
  isArray,
  isEmpty,
  reject,
  size
} from 'lodash';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from '../pro-table';

@Pipe({ name: 'selectionPipe' })
export class SelectionPipe implements PipeTransform {
  transform(value: any[], exponent: string = 'sla_id') {
    return filter(value, item => isEmpty(item[exponent]));
  }
}

@Component({
  selector: 'aui-select-protect-objects',
  templateUrl: './select-protect-objects.component.html',
  styleUrls: ['./select-protect-objects.component.less']
})
export class SelectProtectObjectsComponent implements OnInit, AfterViewInit {
  @Input() title;
  @Input() allTableData: TableData;
  @Input() columns;
  @Input() resourceData;
  @Output() updateTable = new EventEmitter();
  @Output() dataChange = new EventEmitter();

  isMultipleSelected = false;
  allTableConfig: TableConfig;
  selectedTableConfig: TableConfig;
  selectedTableData: TableData;

  @ViewChild('allTable', { static: false }) allTable: ProTableComponent;
  @ViewChild('selectedTable', { static: false })
  selectedTable: ProTableComponent;

  constructor(private i18n: I18NService) {}

  ngAfterViewInit() {
    if (!this.isMultipleSelected) {
      return;
    }
    this.allTable.fetchData();
  }

  initData() {
    this.isMultipleSelected =
      isArray(this.resourceData) && this.resourceData.length > 1;
    if (this.isMultipleSelected) {
      this.selectedTableData = {
        data: cloneDeep(this.resourceData),
        total: size(this.resourceData)
      };
      setTimeout(() => {
        this.allTable.setSelections(cloneDeep(this.selectedTableData.data));
      });
    }
  }

  ngOnInit() {
    this.initData();
    this.initTableConfig();
  }

  indexChange(lvId) {
    if (!lvId) {
      this.allTable.setSelections(cloneDeep(this.selectedTableData.data));
      this.dataChange.emit(cloneDeep(this.selectedTableData.data));
    }
  }

  initTableConfig() {
    this.allTableConfig = {
      table: {
        async: true,
        compareWith: 'uuid',
        columns: this.columns,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        size: 'small',
        colDisplayControl: false,
        fetchData: (filters: Filters) => {
          this.updateTable.emit(filters);
        },
        selectionChange: (selection: any[]) => {
          this.selectedTableData = {
            data: cloneDeep(selection),
            total: size(selection)
          };
          this.dataChange.emit(selection);
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
    const columns = cloneDeep(this.columns);
    each(columns, item => {
      if (item.filter) {
        delete item.filter;
      }
    });
    columns.push({
      key: 'operation',
      name: this.i18n.get('common_operation_label'),
      cellRender: {
        type: 'operation',
        config: {
          maxDisplayItems: 2,
          items: [
            {
              id: 'remove',
              label: this.i18n.get('common_remove_label'),
              onClick: data => {
                const checkedData = reject(
                  cloneDeep(this.selectedTableData.data),
                  { uuid: data[0].uuid }
                );
                this.selectedTableData = {
                  data: checkedData,
                  total: size(checkedData)
                };
                this.dataChange.emit(this.selectedTableData.data);
              }
            }
          ]
        }
      }
    });
    this.selectedTableConfig = {
      table: {
        size: 'small',
        compareWith: 'uuid',
        showLoading: false,
        columns,
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }
}
