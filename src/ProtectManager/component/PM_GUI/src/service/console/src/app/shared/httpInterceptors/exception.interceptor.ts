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
  HttpRequest,
  HttpResponse
} from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import { tap } from 'rxjs/operators';
import { ExceptionService } from '../services';

@Injectable()
export class ExceptionInterceptor implements HttpInterceptor {
  constructor(private exceptionService: ExceptionService) {}

  intercept(
    req: HttpRequest<any>,
    next: HttpHandler
  ): Observable<HttpEvent<any>> {
    const doException = req.params.get('akDoException') !== 'false';

    // 删除akDoException参数
    req = req.clone({
      params: req.params.delete('akDoException')
    });

    return next.handle(req).pipe(
      tap(
        data => {
          if (data instanceof HttpResponse) {
            if (doException && this.exceptionService.isException(data)) {
              this.exceptionService.doException(data);
            }
          }
        },
        error => {
          if (doException) {
            this.exceptionService.doException(error);
          }
        }
      )
    );
  }
}
