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
