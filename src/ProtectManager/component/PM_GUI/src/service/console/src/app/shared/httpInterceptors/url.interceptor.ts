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
  HttpEvent,
  HttpHandler,
  HttpInterceptor,
  HttpRequest
} from '@angular/common/http';
import { Injectable } from '@angular/core';
import { includes, isEmpty } from 'lodash';
import { Observable } from 'rxjs';
import { CommonConsts } from '../consts/common.const';
import { CookieService } from '../services/cookie.service';
import { HttpExtParams } from './http.params';

@Injectable()
export class UrlInterceptor extends HttpExtParams implements HttpInterceptor {
  constructor(private cookieService: CookieService) {
    super();
  }
  intercept(
    req: HttpRequest<any>,
    next: HttpHandler
  ): Observable<HttpEvent<any>> {
    let _prefix = req.params.get('akPrefix');

    const excludeUrl = [];

    // 删除akPrefix参数
    req = req.clone({
      params: req.params.delete('akPrefix')
    });

    if (!!_prefix && _prefix !== 'none') {
      _prefix = _prefix || this.akPrefix;

      req = req.clone({
        url: _prefix + (req.url.startsWith('/') ? '' : '/') + req.url
      });
    }

    // HCS服务化处理URL
    if (
      this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE &&
      req.url.startsWith('/console/rest/') &&
      !includes(excludeUrl, req.url)
    ) {
      if (req.url === '/console/rest/v1.0/servers') {
        req = req.clone({
          url: req.url.replace('/console/rest/', '/ecm/rest/')
        });
      } else if (
        req.url === '/console/rest/v1/resource-tags/availability_zone'
      ) {
        req = req.clone({
          url: req.url.replace('/console/rest/', '/ecm/rest/')
        });
      } else {
        req = req.clone({
          url: req.url.replace('/console/rest/', '/cbs/op/rest/')
        });
      }
    }

    // DME云备份处理URL
    if (
      this.cookieService.get('userType') === CommonConsts.DME_USER_TYPE &&
      req.url.startsWith('/console/rest/') &&
      !includes(excludeUrl, req.url)
    ) {
      const azId = this.cookieService.get('az-id');
      req = req.clone({
        url: req.url.replace(
          '/console/rest/',
          `/dmebackupdfxwebsite/rest/csbs/v1/op/forwards/${azId}/console/rest/`
        )
      });
    }

    // SLA遵从度处理
    if (
      includes(
        [
          '/console/rest/v2/resources',
          '/console/rest/v2/resource/group',
          '/cbs/op/rest/v2/resources',
          '/cbs/op/rest/v2/resource/group'
        ],
        req.url
      ) &&
      req.method === 'GET' &&
      req.params.get('conditions') &&
      includes(req.params.get('conditions'), 'sla_compliance')
    ) {
      const conditions = JSON.parse(req.params.get('conditions'));
      if (
        conditions.protectedObject &&
        includes(conditions.protectedObject.sla_compliance, true) &&
        includes(conditions.protectedObject.sla_compliance, false)
      ) {
        delete conditions.protectedObject.sla_compliance;
        if (isEmpty(conditions.protectedObject)) {
          delete conditions.protectedObject;
        }
        if (!isEmpty(conditions)) {
          req = req.clone({
            params: req.params.set('conditions', JSON.stringify(conditions))
          });
        }
      }
    }

    return next.handle(req);
  }
}
