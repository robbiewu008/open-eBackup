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
import { SystemApiService } from '../api/services/system-api.service';
import { Observable, Observer, finalize } from 'rxjs';
import { SYSTEM_TIME } from '../consts/common.const';

@Injectable({
  providedIn: 'root'
})
export class TimeZoneService {
  constructor(private systemApiService: SystemApiService) {}

  getTimeZone(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (SYSTEM_TIME.hasLoad) {
        observer.next();
        observer.complete();
        return;
      }
      this.systemApiService
        .getSystemTimeUsingGET({
          akDoException: false
        })
        .pipe(
          finalize(() => {
            observer.next();
            observer.complete();
          })
        )
        .subscribe(res => {
          SYSTEM_TIME.hasLoad = true;
          SYSTEM_TIME.timeZone = res.displayName;
          SYSTEM_TIME.offset = res.offset / (3600 * 1e3);
          // 系统时间
          SYSTEM_TIME.sysTime = res.time;
          // 用户访问客户端时间
          SYSTEM_TIME.userSystemTime = new Date().getTime();
        });
    });
  }
}
