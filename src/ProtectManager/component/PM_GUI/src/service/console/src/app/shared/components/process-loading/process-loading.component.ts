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
  Component,
  EventEmitter,
  OnDestroy,
  OnInit,
  Output
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  I18NService,
  SftpManagerApiService,
  ThemeEnum,
  getAppTheme
} from 'app/shared';
import { includes } from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-process-loading',
  templateUrl: './process-loading.component.html',
  styleUrls: ['./process-loading.component.less']
})
export class ProcessLoadingComponent implements OnInit, OnDestroy {
  timeSub$: Subscription;
  destroy$ = new Subject();
  result = {} as any;
  dataMap = DataMap;
  status = DataMap.Standard_Service_Status.running.value;

  @Output() onComplete = new EventEmitter<any>();
  @Output() onTetry = new EventEmitter<any>();

  constructor(
    private i18n: I18NService,
    public sftpManagerApiService: SftpManagerApiService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {}

  getStatus() {
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }
    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.sftpManagerApiService.queryServiceUsingGET({
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.status = res.status;
        if (includes([0, 1], res.status)) {
          this.timeSub$.unsubscribe();
        }
        this.onComplete.emit(includes([0, 1], res.status));
      });
  }

  isLight() {
    return getAppTheme(this.i18n) === ThemeEnum.light;
  }

  getInitLoadingImg(): string {
    return this.isLight()
      ? 'assets/img/init_loading.gif'
      : 'assets/img/init_loading_dark.gif';
  }
}
