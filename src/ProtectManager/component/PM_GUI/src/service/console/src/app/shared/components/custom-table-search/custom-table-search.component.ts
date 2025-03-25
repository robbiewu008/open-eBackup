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
  ElementRef,
  EventEmitter,
  Input,
  OnChanges,
  Output,
  SimpleChanges,
  ViewChild
} from '@angular/core';
import { trim } from 'lodash';

@Component({
  selector: 'aui-custom-table-search',
  templateUrl: './custom-table-search.component.html',
  styleUrls: ['./custom-table-search.component.less']
})
export class CustomTableSearchComponent implements OnChanges {
  @Input() filterTitle: string;
  @Input() value; // 用于传入数据重置
  @Output() search = new EventEmitter<any>();

  nameFilterValue: string;
  nameFilterTrueValue: string;
  nameSearchPanelVisible = false;
  @ViewChild('primaryButton', { read: ElementRef }) primaryButton: ElementRef<
    HTMLElement
  >;
  @ViewChild('nameFilterInput') nameFilterInput: ElementRef<HTMLElement>;

  ngOnChanges(changes: SimpleChanges): void {
    if (changes.value && !changes.value.currentValue) {
      this.nameFilterValue = '';
      this.nameSearchPanelVisible = false;
      this.nameFilterTrueValue = '';
    }

    // 用于右侧搜索框触发filterchange时同步表格里搜索框状态
    if (
      changes.value &&
      !!changes.value.currentValue &&
      changes.value.currentValue !== changes.value.previousValue
    ) {
      this.filterByName(changes.value.currentValue, false);
    }
  }

  popoverBeforeOpen = () => {
    this.nameFilterValue = this.nameFilterTrueValue;
  };

  openNameSearchPanel() {
    this.nameSearchPanelVisible = !this.nameSearchPanelVisible;
    setTimeout(() => {
      this.nameFilterInput?.nativeElement?.focus();
    }, 300);
  }

  confirmName(e: KeyboardEvent) {
    this.primaryButton.nativeElement.click();
  }

  reset() {
    this.nameFilterValue = '';
    this.filterByName(this.nameFilterValue);
    this.nameSearchPanelVisible = false;
  }

  filterByName(value: string, isManual = true) {
    this.nameFilterTrueValue = value;
    if (value === null || value === undefined) {
      return;
    }
    if (isManual) {
      this.search.emit(trim(value));
    }
    this.nameSearchPanelVisible = false;
  }
}
