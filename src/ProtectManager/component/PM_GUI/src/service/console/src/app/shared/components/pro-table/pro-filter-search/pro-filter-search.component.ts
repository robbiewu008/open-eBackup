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
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import { Subject } from 'rxjs';
import { debounceTime, takeUntil } from 'rxjs/operators';
import { isString, isArray } from 'lodash';

@Component({
  selector: 'lv-pro-filter-search',
  templateUrl: './pro-filter-search.component.html',
  styleUrls: ['./pro-filter-search.component.less']
})
export class ProFilterSearchComponent implements OnInit {
  @Input() value: string[];
  @Output() search = new EventEmitter<any>();
  @Output() clear = new EventEmitter<any>();

  @ViewChild('popover', { static: false }) popover;
  constructor() {}

  search$ = new Subject<string>();
  destory$ = new Subject<void>();
  searchValue;
  isClear = false;

  isActive() {
    return this.value && this.value[0] && this.value[0] !== '' ? true : false;
  }

  ngOnInit() {
    this.searchSubscribe();
  }

  ngOnDestroy(): void {
    this.destory$.next();
    this.destory$.complete();
  }

  _beforeOpen = () => {
    if (isString(this.value)) {
      this.searchValue = this.value;
    } else if (isArray(this.value)) {
      this.searchValue = this.value[0];
    } else {
      this.searchValue = '';
    }
  };

  _doSearch(e: any) {
    const v = e.replace(/(^\s*)|(\s*$)/g, ''); // 去除左右两边空格
    if (v !== '') {
      this.isClear = false;
      this.search$.next(v);
    } else {
      this._clear();
    }
  }

  _clear() {
    this.isClear = true;
    this.clear.emit();
    this.popover.hide();
  }

  searchSubscribe() {
    this.search$
      .pipe(debounceTime(300), takeUntil(this.destory$))
      .subscribe(value => {
        // If the operation is a clear operation, the search is not triggered.
        if (this.isClear) return;

        this.search.emit([value]);
        // Close search panel after doSearch
        this.popover.hide();
      });
  }
}
