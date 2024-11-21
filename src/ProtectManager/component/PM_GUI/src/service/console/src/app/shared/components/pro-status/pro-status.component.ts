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
  Input,
  OnChanges,
  OnInit,
  SimpleChanges
} from '@angular/core';
import { LvConfig } from '@iux/live';
import {
  merge as _merge,
  cloneDeep as _cloneDeep,
  find as _find
} from 'lodash';

export interface StatusConfig {
  value: string | number | boolean;
  label?: string;
  color?: string;
  icon?: string;
  animation?: boolean;
  tooltipTheme?: 'light' | 'dark';
}

@Component({
  selector: 'lv-pro-status',
  templateUrl: './pro-status.component.html',
  styleUrls: ['./pro-status.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ProStatusComponent implements OnInit, OnChanges {
  @Input() value: string | number | boolean;

  @Input() config: { [key: string]: StatusConfig };

  statusConfig: StatusConfig = {
    value: '',
    // color: '#499DF2',
    label: this.unknownLabel(),
    animation: false,
    tooltipTheme: 'light'
  };

  unknownLabel() {
    return (
      {
        'zh-cn': '--',
        'en-us': '--'
      }[LvConfig.language] || 'unknown'
    );
  }

  constructor() {}

  ngOnChanges(changes: SimpleChanges) {
    if (changes.value || changes.config) {
      this.init();
    }
  }

  ngOnInit() {
    this.init();
  }

  init() {
    const config = this.getValueConfig(this.config, this.value, false);

    if (!config) {
      this.statusConfig = {
        value: '',
        label: this.unknownLabel(),
        animation: false,
        tooltipTheme: 'light'
      };
      return;
    }

    this.statusConfig = _merge({}, this.statusConfig, config);
  }

  /**
   * 获取值对应的配置
   * @param configKey 模型
   * @param value 值
   * @param clone 是否深拷贝
   * @param isCaseSensitive 是否大小写敏感，默认否
   */
  protected getValueConfig(
    configKey,
    value,
    clone = true,
    isCaseSensitive = false
  ) {
    const config = clone ? _cloneDeep(configKey) : configKey;

    const item = _find(config, (data: { value: any }) =>
      this.isEqualWithCaseSensitive(data.value, value, isCaseSensitive)
    );

    return clone ? _cloneDeep(item) : item;
  }

  /**
   * 是否相等,地址比较
   * @param source 源数据
   * @param target 比较数据
   * @param isCaseSensitive 是否大小写敏感
   */
  protected isEqualWithCaseSensitive(
    source: any,
    target: any,
    isCaseSensitive = false
  ) {
    return isCaseSensitive
      ? source === target
      : this.caseInsensitiveValue(source) === this.caseInsensitiveValue(target);
  }

  /**
   * 根据数据返回大小写不敏感结果
   */
  protected caseInsensitiveValue(value: any) {
    if (typeof value === 'string') {
      return value && value.toLowerCase();
    } else {
      return value;
    }
  }
}
