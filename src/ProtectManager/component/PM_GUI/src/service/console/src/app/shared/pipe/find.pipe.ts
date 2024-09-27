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
import { find } from 'lodash';

@Pipe({ name: 'find', pure: false })
export class FindPipe implements PipeTransform {
  /**
   * 查找集合中指定属性的值
   *
   * @param   {any[]}   items  目标集合
   * @param   {string}  key    属性名称
   * @param   {string}  value  属性值
   *
   * @return  {[]}             [return description]
   */
  transform(items: any[], key: string, value: string | number | boolean) {
    return find(items, item => item[key] === value);
  }
}
