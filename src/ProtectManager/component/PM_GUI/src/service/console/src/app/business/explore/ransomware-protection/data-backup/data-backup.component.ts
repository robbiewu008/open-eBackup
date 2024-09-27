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
import { Component, OnDestroy, OnInit } from '@angular/core';
import {
  CommonConsts,
  DataMap,
  GlobalService,
  I18NService,
  ProtectedResourceApiService,
  SlaApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign } from 'lodash';
import { Subject } from 'rxjs';
import { takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-data-backup',
  templateUrl: './data-backup.component.html',
  styleUrls: ['./data-backup.component.less']
})
export class DataBackupComponent implements OnInit, OnDestroy {
  activeIndex = 'fileSystem';
  destroy$ = new Subject();
  protectedFileSystem = 0;
  totalFileSystem = 0;
  totalPolicy = 0;
  isEn = this.i18n.isEn;

  constructor(
    public slaApiService: SlaApiService,
    private globalService: GlobalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    if (this.appUtilsService.getCacheValue('copyDetectionTab')) {
      this.activeIndex = 'backupPolicy';
    }
    this.getState();
    this.getFilesystem();
    this.getFilesystem(true);
    this.getPolicy();
  }

  getState() {
    this.globalService
      .getState('toFilesystem')
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        this.activeIndex = 'fileSystem';
      });
  }

  getFilesystem(protectedFlag = false, mask = true) {
    const params = {
      pageNum: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      akLoading: mask
    };

    const defaultConditions = {
      subType: [DataMap.Resource_Type.LocalFileSystem.value]
    };

    if (protectedFlag) {
      assign(defaultConditions, {
        protectionStatus: [DataMap.Protection_Status.protected.value]
      });
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (protectedFlag) {
        this.protectedFileSystem = res.totalCount;
      } else {
        this.totalFileSystem = res.totalCount;
      }
    });
  }

  getPolicy(mask = true) {
    const params = {
      pageNum: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      akLoading: mask
    };
    this.slaApiService.pageQueryUsingGET(params).subscribe(res => {
      this.totalPolicy = res.total;
    });
  }

  refreshFileSystem() {
    this.getFilesystem(false, false);
    this.getFilesystem(true, false);
  }

  refreshPolicy() {
    this.getPolicy(false);
  }
}
