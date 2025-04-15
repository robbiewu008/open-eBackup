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
import { Observable, Observer } from 'rxjs';
import { SystemApiService } from '../api/services';

@Injectable({
  providedIn: 'root'
})
export class SystemTimeService {
  constructor(private systemApiService: SystemApiService) {}

  getSystemTime(load = true, cluster?): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      this.systemApiService
        .getSystemTimeUsingGET({
          akLoading: load,
          akDoException: false,
          memberEsn: cluster?.memberEsn
        })
        .subscribe({
          next: res => {
            observer.next(res);
            observer.complete();
          },
          error: err => {
            observer.error(err);
            observer.complete();
          }
        });
    });
  }
}
