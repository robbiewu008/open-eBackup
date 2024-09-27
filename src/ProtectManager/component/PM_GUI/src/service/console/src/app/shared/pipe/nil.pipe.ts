/**
 * 数据为null或undefined或空字符时，显示‘--’
 * showZero 特殊场景：0也显示‘--’
 */

import { Pipe, PipeTransform } from '@angular/core';
import { isNil as _isNil } from 'lodash';

@Pipe({ name: 'nil', pure: false })
export class NilPipe implements PipeTransform {
  constructor() {}

  transform(value: any, showZero: boolean = true): any {
    return value === '' || (_isNil(value) && (showZero || value === 0))
      ? '--'
      : value;
  }
}
