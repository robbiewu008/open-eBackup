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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  Optional,
  Pipe,
  PipeTransform,
  QueryList,
  SimpleChanges,
  ViewChild,
  ViewChildren
} from '@angular/core';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import { ModalRef, TypeUtils, CheckboxComponent } from '@iux/live';
import { merge as _merge } from 'lodash';
import { Observable } from 'rxjs';
import { ProTableComponent, TableConfig } from '../pro-table';

@Pipe({
  name: 'safehtml'
})
export class SafeHtmlPipe implements PipeTransform {
  constructor(protected dom: DomSanitizer) {}
  public transform(value): SafeHtml {
    return this.dom.bypassSecurityTrustHtml(value);
  }
}

@Component({
  selector: 'pro-messagebox',
  templateUrl: './pro-messagebox.component.html',
  styleUrls: ['./pro-messagebox.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ProMessageboxComponent implements OnInit {
  @Input() data: any;

  public typeUtils = TypeUtils;
  public selected;
  public tableConfig: TableConfig;
  public tableData = [];
  @ViewChildren(CheckboxComponent) checkboxs: QueryList<CheckboxComponent>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    @Optional() private modal: ModalRef,
    private cdr: ChangeDetectorRef
  ) {}

  ngOnInit() {
    this.data.description &&
      (this.data.description.type = this.data.description?.type
        ? this.data.description?.type
        : 'text');
    this.selected = this.data.checkbox?.value;

    if (this.data.list) {
      const DEFAULT_CONFIG = {
        pagination: {
          mode: 'simple',
          showTotal: true,
          showPageSizeOptions: false,
          pageSize: 5
        },
        table: {
          async: false,
          size: 'small',
          colDisplayControl: false
        }
      };
      this.tableConfig = _merge({}, DEFAULT_CONFIG, this.data.list);
      if (this.tableConfig.table.fetchData) {
        this.tableConfig.table.fetchData = (filters, ...args) =>
          this.tableFetchData(filters, ...args);
      }
    }
  }

  ngAfterViewInit() {
    // 初始化列表数据
    this.dataTable && this.dataTable.fetchData();
  }

  tableFetchData(filters?, ...args: any[]) {
    // 加载表格数据
    const fetchData = this.data.list?.table.fetchData(
      filters,
      this.modal,
      ...args
    );
    if (fetchData instanceof Promise) {
      fetchData
        .then(res => {
          this.tableData = res;
          this.cdr.markForCheck();
        })
        .catch(error => {
          console.error('Failed to request data.');
        });
    } else if (fetchData instanceof Observable) {
      fetchData.subscribe(
        res => {
          this.tableData = res;
          this.cdr.markForCheck();
        },
        error => {
          console.error('Failed to request data.');
        }
      );
    } else {
      this.tableData = fetchData;
    }
  }

  checkboxModelChange(e) {
    this.data.checkbox.valueChange &&
      this.data.checkbox.valueChange(e, this.modal);
  }
}
