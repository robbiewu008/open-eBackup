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
import { Injectable } from '@angular/core';
import {
  ActivatedRouteSnapshot,
  Resolve,
  RouterStateSnapshot
} from '@angular/router';
import { forEach as _forEach, indexOf as _indexOf } from 'lodash';
import { Observable, of, throwError } from 'rxjs';
import { catchError, mergeMap, retry } from 'rxjs/operators';
import { I18NService } from '../services/i18n.service';

/**
 * 获取I18n资源
 *
 * @export
 * @param {I18NService} i18n I18n服务
 * @param {string} moduleName I18n资源模块名
 */
export function getI18nResource(i18n: I18NService, moduleName: any) {
  if (!moduleName) {
    throw throwError('I18n Module No Config!');
  }

  // 适配LiveUI平台国际化文件夹不同情况
  const urlPrefix = 'assets/i18n/';
  let resourceUrl = `${urlPrefix}${i18n.language}/${moduleName}.json`;
  if (moduleName === 'deploy') {
    resourceUrl = 'assets/deploy/deploy.json';
  }
  return i18n.load(resourceUrl).pipe(
    retry(3),
    catchError(error => {
      console.error('i18n error: %o', error);
      return of('Load I18n Module Error!');
    })
  );
}

// I18n模块地址映射表
const i18nMapToUrl = {
  '^/protection': 'protection,insight,explore',
  '^/explore': 'explore,protection,insight',
  '^/insight': 'insight,protection',
  '^/system': 'system',
  '^/search': 'search,protection'
};

@Injectable({
  providedIn: 'root'
})
export class PreloadI18nResolver implements Resolve<any> {
  // 已加载的资源模块
  loadedModules = [];

  constructor(private i18n: I18NService) {}

  resolve(
    route: ActivatedRouteSnapshot,
    state: RouterStateSnapshot
  ): Observable<any> {
    const url = state.url;
    const requestModule: string[] = [];

    // 过滤出未加载的模块名
    _forEach(i18nMapToUrl, (value: string, key: string) => {
      if (!new RegExp(key, 'gi').test(url)) {
        return true;
      }

      _forEach(value.split(','), item => {
        // 避免重复加载相同资源模块
        if (this.has(item)) {
          return true;
        }

        this.loadedModules.push(item);

        requestModule.push(item);
      });
    });

    // 批量获取I18n资源
    return !requestModule.length
      ? of(null)
      : of(...requestModule).pipe(
          mergeMap(module => getI18nResource(this.i18n, module))
        );
  }

  // 是否已经存在
  has(item: string) {
    return _indexOf(this.loadedModules, item) !== -1;
  }

  // 添加已加载的国家化模块
  append(item: any) {
    // 避免重复加载相同资源模块
    if (this.has(item)) {
      return;
    }

    this.loadedModules.push(item);
  }

  // 清空已加载的国际化模块
  clear() {
    this.loadedModules.length = 0;
  }
}
