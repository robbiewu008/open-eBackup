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
import { CookieService } from './cookie.service';
import { Injectable } from '@angular/core';
import { cloneDeep, find, includes, reduce, size } from 'lodash';
import { DataMap } from '../consts/data-map.config';
import { I18NService } from './i18n.service';
import { BaseUtilService } from './base-util.service';

@Injectable({
  providedIn: 'root'
})
export class DataMapService {
  constructor(
    private i18n: I18NService,
    private cookieService: CookieService,
    private baseUtilService: BaseUtilService
  ) {}

  // 根据configKey拿到配置项
  getConfig(configKey, clone = true) {
    const config = DataMap[configKey];
    if (!config) {
      throw new Error(`${configKey} is not defined in data-map.config`);
    }

    return clone ? cloneDeep(config) : config;
  }

  // 获取值对应的配置
  getValueConfig(configKey, value, clone = true) {
    const config = this.getConfig(configKey, clone);
    const item = find(config, item => item.value === value);

    return clone ? cloneDeep(item) : item;
  }

  // 通过value, 拿到Label
  getLabel(configKey, value) {
    const config = this.getValueConfig(configKey, value, false);
    return this.i18n.get(config ? config.label : '--');
  }

  // 把配置转换成数组
  /*
  [{
    value: '1',
    label: 'Full + Incremental',
    key: objKey
  }]
  */
  toArray(configKey, includesList = []) {
    const config = this.getConfig(configKey, false);

    return reduce(
      config,
      (arr, item, key) => {
        const cloneItem = cloneDeep(item);
        cloneItem.key = key;
        cloneItem.label = this.i18n.get(cloneItem.label);
        arr.push(cloneItem);
        return arr;
      },
      []
    ).filter(item => {
      return this.cookieService.isCloudBackup && !!size(includesList)
        ? includes(includesList, item.value)
        : !!size(includesList)
        ? !includes([DataMap.Resource_Type.LocalFileSystem.value], item.value)
        : true;
    });
  }

  /**
   * 把配置转换成对象
   * @param configKey
   * {
   * [value]: { value, label, ...}
   * }
   */
  toMap(configKey) {
    const arr = this.toArray(configKey);
    return reduce(
      arr,
      (map, o) => {
        map[o.value] = o;
        return map;
      },
      {}
    );
  }
}
