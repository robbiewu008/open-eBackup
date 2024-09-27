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
import { StatusConfig } from '../pro-status/index';
import {
  merge as _merge,
  cloneDeep as _cloneDeep,
  find as _find,
  isNil as _isNil
} from 'lodash';

export interface TextConfig {
  id?: string;
  label?: string;
  icon?: string;
  iconColor?: string;
  align?: 'left' | 'right';
  iconPos?: 'left' | 'right' | 'flow-text';
  overflow?: boolean; // 内容超出显示方式
  map?: { [key: string]: StatusConfig };
  click?: (args: []) => void;
}

export interface TextShowConfig extends TextConfig {
  color?: string;
}

@Component({
  selector: 'lv-pro-text',
  templateUrl: './pro-text.component.html',
  styleUrls: ['./pro-text.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ProTextComponent implements OnInit, OnChanges {
  @Input() value: string;

  @Input() data: any;

  @Input() config: TextConfig;

  textConfig: TextShowConfig = { iconPos: 'left' as const, overflow: true };

  _icon;
  _iconColor;
  _value;

  constructor() {}

  isNull() {
    return _isNil(this.value) || this.value === '' || false;
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.value || changes.config) {
      changes.value && (this.value = changes.value.currentValue);
      changes.config && (this.config = changes.config.currentValue);

      this.init();
    }
  }

  ngOnInit() {
    this.init();
  }

  init() {
    let config;
    if (this.config?.map) {
      config = this.getValueConfig(this.config.map, this.value, false);

      if (!config) {
        return;
      }

      config.click = this.config.click;
      this.config.iconPos && (config.iconPos = this.config.iconPos);
    } else {
      config = this.config;
    }

    this.textConfig = _merge({}, this.textConfig, config);
    this._icon = this.textConfig.icon;
    this._iconColor = this.textConfig.color || this.textConfig.iconColor;
    this._value = this.textConfig.label || this.value;
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
    let config = {};

    config = clone ? _cloneDeep(configKey) : configKey;

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
