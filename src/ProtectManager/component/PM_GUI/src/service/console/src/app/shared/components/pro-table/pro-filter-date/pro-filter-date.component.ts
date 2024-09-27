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
  Component,
  EventEmitter,
  Input,
  OnChanges,
  OnInit,
  Output,
  SimpleChanges,
  ViewChild,
  ViewEncapsulation
} from '@angular/core';
import { I18NService } from '@iux/live';
import { merge as _merge } from 'lodash';
import { ProFilterDateConfig } from './interface';

@Component({
  selector: 'lv-pro-filter-date',
  templateUrl: './pro-filter-date.component.html',
  styleUrls: ['./pro-filter-date.component.less'],
  encapsulation: ViewEncapsulation.None,
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ProFilterDateComponent implements OnChanges {
  @Input() value: Date[];
  @Input() config: ProFilterDateConfig;
  @Output() search = new EventEmitter<any>();

  @ViewChild('popover', { static: false }) popover;

  constructor(private i18n: I18NService) {}

  date;
  conf;

  isRange() {
    return this.config.dateRange === true || this.config.dateRange === undefined
      ? true
      : false;
  }

  isNull() {
    return (
      this.date === '' ||
      this.date === undefined ||
      this.date === null ||
      (this.isRange() &&
        (this.date.length === 0 ||
          this.date[0] === null ||
          this.date[1] === null))
    );
  }

  isActive() {
    if (this.isRange()) {
      if (this.value instanceof Array && this.value[0] && this.value[1]) {
        return true;
      } else {
        return false;
      }
    } else {
      return this.value && this.value[0] ? true : false;
    }
  }

  ngOnChanges(changes: SimpleChanges): void {
    if (changes.value && changes.value.currentValue) {
      this.date = this.isRange() ? this.value : this.value[0];
    }
  }

  _ok() {
    const args = this.isRange() ? this.date : [this.date];
    this.search.emit(args);
    this.popover.hide();
  }

  _reset() {
    this.date = this.isRange() ? [null, null] : null;
    const args = this.isRange() ? this.date : [this.date];

    this.search.emit(args);
    this.popover.hide();
  }

  _beforeOpen = () => {
    const DEFAULT = {
      format: 'yyyy-MM-dd',
      dateRange: true,
      showTime: false,
      disabledDate: (date: Date) => {
        return false;
      },
      showTodayButton: false,
      showNowButton: false,
      placeholder: null
    };
    this.conf = _merge({}, DEFAULT, this.config);
    if (this.conf.dateRange) {
      this.conf.placeholder === null &&
        (this.conf.placeholder = [
          this.i18n.get('startDate'),
          this.i18n.get('endDate')
        ]);
    } else {
      this.conf.placeholder === null &&
        (this.conf.placeholder = this.i18n.get('pickDate'));
    }

    if (this.value) {
      this.date = this.isRange() ? this.value : this.value[0];
    } else {
      this.date = this.isRange() ? [null, null] : null;
    }
  };
}
