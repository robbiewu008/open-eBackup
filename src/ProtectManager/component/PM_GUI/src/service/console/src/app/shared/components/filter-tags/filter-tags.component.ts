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
import { DatePipe } from '@angular/common';
import {
  Component,
  EventEmitter,
  Input,
  OnChanges,
  Output,
  SimpleChanges
} from '@angular/core';
import { TypeUtils } from '@iux/live';
import { SYSTEM_TIME } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { first, isArray, merge as _merge } from 'lodash';

@Component({
  selector: 'aui-filter-tags',
  templateUrl: './filter-tags.component.html',
  styleUrls: ['./filter-tags.component.less'],
  providers: [DatePipe]
})
export class FilterTagsComponent implements OnChanges {
  @Input() filterMap;
  @Input() tableCols;
  @Output() clearCertainTag = new EventEmitter<any>();
  @Output() clearAllTag = new EventEmitter<any>();
  filterTags = [];
  typeUtils = TypeUtils;

  constructor(
    public datePipe: DatePipe,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnChanges(changes: SimpleChanges): void {
    this.getFilterTags();
  }

  getFilterTags() {
    const _filterTags = Array.from(this.filterMap).map(item => {
      const arr = {
        label: this._formatTagItem(item),
        removeable: true
      };
      return _merge(arr, item);
    });
    this.filterTags = _filterTags.filter((item: any) => {
      if (isArray(item.value)) {
        item.value = item.value.filter(
          v => v !== '' && v !== null && v !== undefined
        );
        return item.value.length > 0;
      }
      return true;
    });
  }

  /**
   * 获取Tag名称
   */
  _getTagName(key) {
    const col = this.tableCols.filter(item => item.key === key);
    return (
      col.length > 0 &&
      (this.typeUtils.isRealString(col[0].name)
        ? col[0].name
        : this.typeUtils.isRealString(col[0].label)
        ? col[0].label
        : col[0].auxiliary)
    );
  }

  _formatTagItem(item) {
    const col = this.tableCols.filter(i => i.key === item.key)[0];
    let labels: any[] = [];
    if (col?.filterMap && col.filter) {
      this.dealArrayFilter(item, col, labels);
    } else if (item.key === 'alarmTimeStr') {
      if (first(item.value) == null) return null;
      labels = item.value
        .map(item =>
          this.getAlarmTimeStr(
            this.appUtilsService.toSystemTimeLong(item),
            false
          )
        )
        .join(' ~ ');
    } else {
      labels = item.value;
    }
    return labels;
  }

  private dealArrayFilter(item: any, col: any, labels: any[]) {
    item.value.map(v => {
      col.filterMap.map(o => {
        if (v === o.value) {
          labels.push(o.label);
        }
      });
    });
  }

  private getAlarmTimeStr(timestamp, isSeconds = true) {
    if (isSeconds) timestamp = timestamp * 1000;
    return this.datePipe.transform(
      timestamp,
      'yyyy-MM-dd HH:mm:ss',
      SYSTEM_TIME.timeZone
    );
  }

  /**
   * 清除过滤标签
   * @param event 无参数时清除所有
   */
  _clearFilterTag(event?) {
    if (event) {
      this.clearCertainTag.emit(event);
    } else {
      this.clearAllTag.emit();
    }
  }
}
