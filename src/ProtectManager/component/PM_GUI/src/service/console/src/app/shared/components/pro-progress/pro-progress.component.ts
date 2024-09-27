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
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import { I18NService, TypeUtils } from '@iux/live';
import { merge as _merge, isNil as _isNil } from 'lodash';

const DEFAULT_CONFIG = {
  // progress 配置
  value: 0,
  width: 0,
  colors: [],
  status: 'normal',
  showLabel: true,
  label: null,
  extra: null,
  // tooltip 配置
  tooltip: null,
  tooltipTheme: 'light'
};

@Component({
  selector: 'lv-pro-progress',
  templateUrl: './pro-progress.component.html',
  styleUrls: ['./pro-progress.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ProProgressComponent implements OnInit {
  @Input() value;
  @Input() config;

  initConfig;
  public typeUtils = TypeUtils;

  constructor() {}

  isNull() {
    return _isNil(this.value) || this.value === '' || false;
  }

  ngOnInit() {
    this.initConfig = _merge({}, DEFAULT_CONFIG, this.config);
    this.initConfig.showLabel = this.initConfig.label === false ? false : true;
  }
}
