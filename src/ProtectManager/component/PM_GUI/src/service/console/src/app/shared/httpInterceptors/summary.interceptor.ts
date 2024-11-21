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
  HttpRequest,
  HttpHandler,
  HttpEvent,
  HttpInterceptor,
  HttpResponse
} from '@angular/common/http';
import { Observable, Observer, combineLatest, finalize, mergeMap } from 'rxjs';
import { CookieService } from '../services/cookie.service';
import { ProtectedResourceApiService } from '../api/services';
import { CommonConsts } from '../consts/common.const';
import { assign, each, find, includes, isEmpty } from 'lodash';
import { DataMap } from '../consts/data-map.config';

@Injectable()
export class SummaryInterceptor implements HttpInterceptor {
  const;
  constructor(
    private cookieService: CookieService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  private _setSummaryCount(resources, protectedCount = 0, total = 0) {
    each(resources, item => {
      if (item.resource_sub_type === DataMap.Resource_Type.HCSCloudHost.value) {
        assign(item, {
          protected_count: item.protected_count - protectedCount,
          unprotected_count: item.unprotected_count - (total - protectedCount)
        });
      }
    });
  }

  private _hasHcsCloudCount(resources) {
    if (isEmpty(resources)) {
      return false;
    }
    return !!find(
      resources,
      item =>
        item.resource_sub_type === DataMap.Resource_Type.HCSCloudHost.value
    );
  }

  private _getSummaryCount(event, req: HttpRequest<unknown>): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const resources = event.body?.summary;
      if (
        this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE &&
        includes(
          [
            '/console/rest/v1/resource/protection/summary',
            '/cbs/op/rest/v1/resource/protection/summary'
          ],
          req.url
        ) &&
        event instanceof HttpResponse &&
        this._hasHcsCloudCount(resources)
      ) {
        const conditions = {
          type: 'CloudHost',
          subType: DataMap.Resource_Type.HCSCloudHost.value,
          environment: {
            subType: ['HcsEnvOp']
          }
        };
        combineLatest([
          this.protectedResourceApiService.ListResources({
            pageNo: CommonConsts.PAGE_START,
            pageSize: CommonConsts.PAGE_SIZE,
            conditions: JSON.stringify({
              ...conditions,
              protectionStatus: [
                ['in'],
                DataMap.Protection_Status.protected.value
              ]
            }),
            akOperationTips: false,
            akDoException: false,
            akLoading: false
          }),
          this.protectedResourceApiService.ListResources({
            pageNo: CommonConsts.PAGE_START,
            pageSize: CommonConsts.PAGE_SIZE,
            conditions: JSON.stringify(conditions),
            akOperationTips: false,
            akDoException: false,
            akLoading: false
          })
        ])
          .pipe(
            finalize(() => {
              observer.next(event);
              observer.complete();
            })
          )
          .subscribe(res => {
            const [protectedCount, total] = res;
            this._setSummaryCount(
              resources,
              protectedCount?.totalCount,
              total?.totalCount
            );
          });
      } else {
        observer.next(event);
        observer.complete();
      }
    });
  }

  intercept(
    request: HttpRequest<unknown>,
    next: HttpHandler
  ): Observable<HttpEvent<unknown>> {
    return next
      .handle(request)
      .pipe(mergeMap(event => this._getSummaryCount(event, request)));
  }
}
