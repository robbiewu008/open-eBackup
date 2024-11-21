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
import { HTTP_INTERCEPTORS } from '@angular/common/http';
import { CollectionDataInterceptor } from './collection-data.interceptor';
import { DownloadInterceptor } from './download.interceptor';
import { ExceptionInterceptor } from './exception.interceptor';
import { HeaderInterceptor } from './header.interceptor';
import { LoadingInterceptor } from './loading.interceptor';
import { OperationTipsInterceptor } from './operation-tips.interceptor';
import { TimeoutInterceptor } from './timeout.interceptor';
import { UrlInterceptor } from './url.interceptor';
import { PermissionInterceptor } from './permission.interceptor';
import { SummaryInterceptor } from './summary.interceptor';

export const HttpInterceptorProviders = [
  {
    provide: HTTP_INTERCEPTORS,
    useClass: LoadingInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: CollectionDataInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: ExceptionInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: TimeoutInterceptor,
    multi: true
  },
  { provide: HTTP_INTERCEPTORS, useClass: UrlInterceptor, multi: true },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: HeaderInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: DownloadInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: OperationTipsInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: PermissionInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: SummaryInterceptor,
    multi: true
  }
];

export * from './http.params';
