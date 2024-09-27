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
import {
  CommonConsts,
  EXPORT_URL_WHITE_LIST
} from 'app/shared/consts/common.const';
import { Observable } from 'rxjs';
import { CookieService } from '../services/cookie.service';

@Injectable()
export class DownloadInterceptor implements HttpInterceptor {
  constructor(private cookieService: CookieService) {}

  intercept(
    req: HttpRequest<any>,
    next: HttpHandler
  ): Observable<HttpEvent<any>> {
    const downloadUrl = EXPORT_URL_WHITE_LIST.some(url => {
      // HCS服务嵌套处理
      if (this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE) {
        return new RegExp(url.replace('/console/rest/', '/cbs/op/rest/')).test(
          req.url
        );
      }
      // DME服务嵌套处理
      const azId = this.cookieService.get('az-id');
      if (this.cookieService.get('userType') === CommonConsts.DME_USER_TYPE) {
        return new RegExp(
          url.replace(
            '/console/rest/',
            `/dmebackupdfxwebsite/rest/csbs/v1/op/forwards/${azId}/console/rest/`
          )
        ).test(req.url);
      }

      return new RegExp(url).test(req.url);
    });

    if (downloadUrl) {
      req = req.clone({
        responseType: 'blob'
      });
    }
    return next.handle(req);
  }
}
