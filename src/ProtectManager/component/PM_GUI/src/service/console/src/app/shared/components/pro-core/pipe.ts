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
import { Pipe, PipeTransform } from '@angular/core';
import { isNil as _isNil, filter as _filter } from 'lodash';

/**
 * 按需保留小数位
 * @param decimals 保留小数点几位数，默认2
 * @param method 保留小数的方式，默认四舍五入
 */
export function NumberToFixed(value, decimals = 0, method = 'round') {
  const k = Math.pow(10, decimals);
  let _value;
  switch (method) {
    // 取向下整
    case 'floor':
      _value = Math.floor(value * k) / k;
    // 向上取整
    case 'ceil':
      _value = Math.ceil(value * k) / k;
    // 四舍五入
    default:
      _value = Math.round(value * k) / k;
  }

  if (decimals === 0) {
    return _value;
  } else {
    const values = _value.toString().split('.');
    const v_int = values[0];
    let v_float = values[1] || '0';
    for (let i = v_float.length; i < decimals; i++) {
      v_float = v_float + '0';
    }
    return v_int + '.' + v_float;
  }
}

/**
 * 数字转容量
 * @param decimals 保留小数点几位数，默认2
 * @param k 系数，默认1024
 */
@Pipe({ name: 'capacity', pure: false })
export class ProCapacityPipe implements PipeTransform {
  constructor() {}

  transform(value: any): any {
    if (_isNil(value) || value === '') {
      return '--';
    } else {
      const [decimals, k, method] = Array.from(arguments).slice(1);
      return this.numberToCapacity(value, decimals, k, method);
    }
  }

  /**
   *  数字转容量
   * @param data 源数据，单位b
   * @param decimals 保留小数点几位数，默认2
   * @param k 系数，默认1024
   */
  numberToCapacity(
    data: number,
    decimals: number = 2,
    k: number = 1024,
    method = 'round'
  ): string {
    if (data === 0) {
      return '0 B';
    }
    const CAPACITY_UNIT = {
      BYTE: 'B',
      KB: 'KB',
      MB: 'MB',
      GB: 'GB',
      TB: 'TB',
      PB: 'PB'
    };
    const dm = decimals <= 0 ? 0 : decimals,
      sizes = this.enumToKeyValueMap(CAPACITY_UNIT, false).values,
      i = Math.floor(Math.log(data) / Math.log(k)),
      value = data / Math.pow(k, i);

    return NumberToFixed(value, dm, method) + ' ' + sizes[i];
  }

  /**
   * 枚举转{keys, values}， 仅支持全是number | string
   * @param data 枚举
   * @param isNumber 枚举值是否是number，默认是
   */
  enumToKeyValueMap(data, isNumber = true) {
    let keys = Object.keys(data);
    if (isNumber) {
      keys = keys.filter(item => typeof data[item] !== 'number');
    }
    const values = keys.map(item => data[item]);
    return { keys, values };
  }
}

@Pipe({ name: 'percent' })
export class ProPercentPipe implements PipeTransform {
  transform(value: number, decimals: number = 0, method = 'round') {
    if (_isNil(value)) {
      return '--';
    } else {
      return NumberToFixed(value * 100, decimals, method) + '%';
    }
  }
}

@Pipe({ name: 'number' })
export class ProNumberPipe implements PipeTransform {
  transform(
    value: number,
    decimals: number = 0,
    method = 'round',
    format: boolean = true
  ) {
    if (_isNil(value)) {
      return '--';
    } else {
      const valStr = NumberToFixed(value, decimals, method).toString();
      const values = valStr.split('.');
      let _value = values[0];
      if (format) {
        for (let i = values[0].length; i > 0; i -= 3) {
          if (i < values[0].length) {
            _value = _value.slice(0, i) + ', ' + _value.slice(i);
          }
        }
      }
      return decimals === 0 ? _value : _value + '.' + values[1];
    }
  }
}
