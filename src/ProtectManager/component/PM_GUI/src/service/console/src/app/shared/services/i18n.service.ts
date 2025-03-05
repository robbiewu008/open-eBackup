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
import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { each, find, includes, isArray, isEmpty, isString, size } from 'lodash';
import { of } from 'rxjs';
import { tap } from 'rxjs/operators';
import {
  cyberEngineMap,
  dataBackupX3000Map,
  DataMap,
  distributedMap
} from '../consts';
import { CookieService } from './cookie.service';

@Injectable({
  providedIn: 'root'
})
export class I18NService {
  // 语言key值
  languageKey = 'livelanguage';

  //OP服务化key值
  opLanguageKey = 'lvpLanguage';

  // 语言, 默认支持zh-cn, en-us
  language: string;

  // 国际化资源
  resource = {};

  // 二次国际化对象资源
  secondResource = {};

  // 支持的语言
  supportLanguages = ['zh-cn', 'en-us'];

  // 默认语言
  defaultLanguage = 'zh-cn';

  // 安全一体机错误码
  cyberDoradoResource = {};
  cyberPacificResource = {};

  SPECIAL_KEY = '1677931542';

  REPLACE_KEY = ['job_status_success_label', 'job_status_fail_label'];

  get isEn() {
    return ['en-us'].includes(this.language);
  }

  constructor(private http: HttpClient, private cookie: CookieService) {
    this.initLanguage();
  }

  getQueryLanguage(): string {
    // 查询参数
    let queryLang: string;
    let url = window.location.href.split('?')[0];
    try {
      const urlParams = window.location.href.split('?')[1];
      const queryParams = urlParams ? urlParams.split('&') : [];
      const paramsArr = [];
      for (let index = 0; index < queryParams.length; index++) {
        const query = queryParams[index].split('=');
        if (query[0] === 'language') {
          queryLang = query[1];
        } else {
          paramsArr.push(`${query[0]}=${query[1]}`);
        }
      }
      if (!isEmpty(paramsArr)) {
        url += `?${paramsArr.join('&')}`;
      }
    } catch (error) {
      queryLang = '';
    }
    if (queryLang) {
      window.location.href = url;
      const returnLang = queryLang === 'en' ? 'en-us' : 'zh-cn';
      this.cookie.set(this.languageKey, returnLang);
      localStorage.setItem(this.languageKey, returnLang);
      return returnLang;
    }
    return queryLang;
  }

  /**
   * 语言初始化
   * 优先级: 1.cookie 2.浏览器语言 3.默认zh-cn
   */
  initLanguage() {
    // 英文浏览器英文环境是en
    let browserLanguage = navigator.language;
    if (browserLanguage !== 'zh-CN') {
      browserLanguage = 'en-us';
    }
    this.language = (
      localStorage.getItem(this.languageKey) ||
      this.cookie.get(this.opLanguageKey) ||
      browserLanguage
    ).toLowerCase();

    // url中携带的语言优先级最高
    const urlQueryLang = this.getQueryLanguage();
    if (urlQueryLang) {
      this.language = urlQueryLang;
    }

    if (this.supportLanguages.indexOf(this.language) === -1) {
      this.language = this.defaultLanguage;
    }
  }

  /**
   * 切换语言, 会重新加载页面
   * @param language 语言
   */
  changeLanguage(language) {
    this.language = language;
    this.cookie.set(this.languageKey, this.language);
    localStorage.setItem(this.languageKey, this.language);
    window.location.reload();
  }

  /**
   * 加载国际化资源
   * @param resource 远端资源地址(en/i18n.json)或资源对象({xxkeyid:xxkeyvalue})
   */
  load(resource) {
    if (typeof resource === 'string') {
      return this.http
        .get(resource, { params: { $prefix: 'none', akLoading: 'false' } })
        .pipe(
          tap(r => {
            if (
              resource ===
              `assets/i18n/${this.language}/error-code/dorado_616.json`
            ) {
              Object.assign(this.cyberDoradoResource, r);
            } else if (
              resource ===
              `assets/i18n/${this.language}/error-code/pacific.json`
            ) {
              Object.assign(this.cyberPacificResource, r);
            } else if (
              resource === `assets/i18n/${this.language}/alarm/dorado_v6.json`
            ) {
              this.resource = Object.assign(r, this.resource);
            } else {
              Object.assign(this.resource, r);
            }

            if (
              [
                `assets/i18n/${this.language}/params.json`,
                `assets/i18n/${this.language}/common.json`,
                `assets/i18n/${this.language}/search.json`,
                `assets/i18n/${this.language}/system.json`,
                `assets/i18n/${this.language}/explore.json`,
                `assets/i18n/${this.language}/insight.json`,
                `assets/i18n/${this.language}/protection.json`,
                `assets/i18n/${this.language}/task/common.json`,
                `assets/i18n/${this.language}/operation/common.json`
              ].includes(resource)
            ) {
              Object.assign(this.secondResource, r);
            }
          })
        );
    } else {
      return of(resource).pipe(
        tap(r => {
          Object.assign(this.resource, resource);
        })
      );
    }
  }

  getJobTargetType(value) {
    const config = DataMap.Job_Target_Type;
    const item = find(config, item => item.value === value);
    return item?.label || value;
  }

  /**
   * 获取国际化信息
   * @param key     i18n资源的key
   * @param params  i18n中占位符内要填充的内容
   * @param colon   是否要带冒号输出，默认不带冒号（false）
   * @param isEncode   是否需要编码转义，防XSS，默认不编码（false）
   * @param subType   区分一体机任务执行类型
   */
  get(key, params = [], colon?, isEncode = false, subType?) {
    if (key === '-1000000000') {
      return this.stringWithMultipleParams(params);
    }
    let i18nStr = this.resource[key] || key;

    if (key === this.SPECIAL_KEY && !isEmpty(params[0])) {
      params[0] = this.getJobTargetType(params[0]);
    }

    // 安全一体机错误码处理
    if (
      !this.resource[key] &&
      this.resource['deploy_type'] === DataMap.Deploy_Type.cyberengine.value
    ) {
      i18nStr =
        subType === DataMap.Job_Target_Type.OceanStorPacific.value
          ? this.cyberPacificResource[key]
          : this.cyberDoradoResource[key] ||
            this.cyberPacificResource[key] ||
            key;
    }

    if (
      this.resource['deploy_type'] === DataMap.Deploy_Type.cyberengine.value &&
      cyberEngineMap[key]
    ) {
      i18nStr = this.resource[cyberEngineMap[key]] || cyberEngineMap[key];
    }

    if (
      this.resource['deploy_type'] === DataMap.Deploy_Type.e6000.value &&
      distributedMap[key]
    ) {
      i18nStr = this.resource[distributedMap[key]] || distributedMap[key];
    }

    if (
      this.resource['deploy_type'] === DataMap.Deploy_Type.x3000.value &&
      dataBackupX3000Map[key]
    ) {
      i18nStr =
        this.resource[dataBackupX3000Map[key]] || dataBackupX3000Map[key];
    }

    i18nStr = this.stringFormat(
      i18nStr,
      isEmpty(params) ? [] : params,
      isEncode
    );

    i18nStr += colon ? (this.language === 'zh-cn' ? '：' : ': ') : '';

    // 替换指定key
    i18nStr = i18nStr.replace(
      new RegExp(this.REPLACE_KEY.join('|'), 'g'),
      match => {
        return includes(params, match) ? this.resource[match] || match : match;
      }
    );

    // E6000屏蔽部分应用时修改词条
    if (this.resource['deploy_type'] === DataMap.Deploy_Type.e6000.value) {
      if (i18nStr.includes('/openGauss')) {
        i18nStr = i18nStr.replace('/openGauss', '');
      }
      if (i18nStr.includes('Informix/')) {
        i18nStr = i18nStr.replace('Informix/', '');
      }
    }

    return i18nStr;
  }

  encodeHtml(html: any): any {
    if (!isString(html)) {
      return html;
    }
    const stringMap = {
      '<': '&lt;',
      '>': '&gt;',
      '"': '&quot;',
      "'": '&apos;'
    };
    return html.replace(/[&<>'"]/g, item => stringMap[item] || item);
  }

  stringFormat(str: string, params = [], isEncode = false) {
    if (isEmpty(str)) {
      return '';
    }

    return str.replace(/\{\d+\}/gi, (match, index) => {
      const i = +match.replace(/\D/g, (m, n) => {
        return '';
      });
      if (!isNaN(i) && typeof params[i] !== 'undefined') {
        return params[i] === 'protection_without_backup_label'
          ? ''
          : isEncode
          ? this.encodeHtml(this.secondResource[params[i]] || params[i])
          : this.secondResource[params[i]] || params[i];
      } else {
        return '--';
      }
    });
  }

  stringWithMultipleParams(params) {
    if (!isArray(params) || !size(params)) {
      return '';
    }
    const strs = [];
    each(params, item => {
      if (item.match(/^i18n:/)) {
        const error = JSON.parse(item.substr(5));
        const errorContent = this.get(
          error.shift()?.replace('lego.err.', ''),
          error
        );
        if (!includes(strs, errorContent)) {
          strs.push(errorContent);
        }
      }
    });
    return strs.join('<br>');
  }
}
