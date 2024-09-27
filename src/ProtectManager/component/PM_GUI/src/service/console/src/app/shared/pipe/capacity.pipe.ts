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
import { DecimalPipe } from '@angular/common';
import { Pipe, PipeTransform } from '@angular/core';
import { isNaN, isNumber } from 'lodash';
import { CAPACITY_UNIT, CAPACITY_UNIT_EXTRA } from '../consts/common.const';

@Pipe({ name: 'capacityCalculateLabel' })
export class CapacityCalculateLabel extends DecimalPipe
  implements PipeTransform {
  /**
   *
   * 例如： value为: 100098.2645， digists为：1.3-3 计算后的结果为：97.752MB
   *       minUnit最小的起始计算单位，默认是KB，isFixed: 不进行四舍五入，默认false
   *       radix:表示容量进制，默认为1024
   *
   * @param digists 数字展现的选项，通过如下格式的字符串指定，格式为{minIntegerDigits}.{minFractionDigits}-{maxFractionDigits}
   *   minIntegerDigits：在小数点前的最小位数，默认为1
   *   minFractionDigits：小数点后的最小位数，默认为 0
   *   maxFractionDigits：小数点后的最大为数，默认为 3
   *
   * 构造方法使用介绍：
   * 第一步：@Component通过providers: [CapacityCalculateLabel]引入
   * 第二步：constructor(public capacityCalculateLabel:CapacityCalculateLabel)中依赖注入
   * 第三步：调用capacityCalculateLabel.transform(100098.2645, '1.3-3')，返回97.752MB
   *
   */
  transform(
    value: any,
    digits?: any,
    minUnit?: string,
    isFixed = true,
    isUnitExtra = false,
    radix = 1024
  ) {
    return this.convertCapUnit(
      value,
      0,
      digits,
      minUnit,
      isFixed,
      isUnitExtra,
      radix
    );
  }

  // 递归从最小单位KB依次转换为最大容量单位EB,
  private convertCapUnit(
    value: any,
    index: number,
    digits?: string,
    minUnit?: string,
    isFixed = false,
    isUnitExtra = false,
    radix = 1024
  ) {
    const unitconst = isUnitExtra ? CAPACITY_UNIT_EXTRA : CAPACITY_UNIT,
      unitsList = Object.keys(unitconst).map(item => unitconst[item]),
      firstUnit = minUnit || unitconst.KB,
      i = unitsList.indexOf(firstUnit),
      units = unitsList.slice(i);

    value = parseFloat(value);
    if (!isNumber(value) || isNaN(value)) {
      return '0.000' + ' ' + units[0];
    }

    if (value / radix >= 1 && index < units.length - 1) {
      return this.convertCapUnit(
        value / radix,
        index + 1,
        digits,
        minUnit,
        isFixed,
        isUnitExtra
      );
    }

    if (isFixed) {
      return this.formatDecimalPoint(value, 3) + ' ' + units[index];
    }

    return super.transform(value, digits) + ' ' + units[index];
  }

  // 截取小数点位数
  public formatDecimalPoint(num: any, decimal: number = 3) {
    const reg = '^[+-]?((\\d+\\.?\\d*)|(\\.\\d+))[Ee][+-]?\\d+$',
      isScientificCount = new RegExp(reg).test(num);
    if (isScientificCount) {
      const m = num.toExponential().match(/\d(?:\.(\d*))?[Ee]([+-]\d+)/);
      num = num.toFixed(Math.max(0, (m[1] || '').length - m[2]));
    }
    num = num.toString();
    const index = num.indexOf('.');
    if (index !== -1) {
      num = num.substring(0, decimal + index + 1);
    } else {
      num = num.substring(0);
    }
    return parseFloat(num).toFixed(decimal);
  }
}
