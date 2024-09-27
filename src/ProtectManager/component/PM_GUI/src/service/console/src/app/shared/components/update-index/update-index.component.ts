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
import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import { FormControl, FormGroup } from '@angular/forms';
import { CommonConsts, PolicyType } from 'app/shared/consts';
import { CookieService, GlobalService } from 'app/shared/services';
import { each, find, isBoolean, isEmpty, isUndefined } from 'lodash';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-update-index',
  templateUrl: './update-index.component.html',
  styleUrls: ['./update-index.component.less']
})
export class UpdateIndexComponent implements OnInit, OnDestroy {
  @Input() formGroup: FormGroup;
  @Input() isDetail;
  @Input() protectData;
  @Input() extParams;

  initSla = false;
  hasBackup = false;
  hasArchive = false;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  _isBoolean = isBoolean;

  destroy$ = new Subject();

  constructor(
    private cookieService: CookieService,
    private globalService: GlobalService
  ) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit(): void {
    this.getState();
  }

  getState(): void {
    this.globalService
      .getState('slaSelectedEvent')
      .pipe(takeUntil(this.destroy$))
      .subscribe(res => {
        this.initSla = true;
        this.getSlaDetail(res);
        this.updateAutoIndex();
      });
  }

  updateAutoIndex() {
    if (!this.extParams) {
      return;
    }
    // 索引
    each(['backup_res_auto_index', 'archive_res_auto_index'], key => {
      if (isBoolean(this.extParams[key])) {
        if (this.formGroup.get(key)) {
          this.formGroup.get(key).setValue(this.extParams[key]);
        } else {
          this.formGroup.addControl(key, new FormControl(this.extParams[key]));
        }
      }
    });
  }

  setAutoIndex(backupPolicy) {
    const indexKey = !isUndefined(
      backupPolicy.ext_parameters.fine_grained_restore
    )
      ? 'fine_grained_restore'
      : 'auto_index';

    if (!this.formGroup?.get('backup_res_auto_index')) {
      this.formGroup.addControl(
        'backup_res_auto_index',
        new FormControl(backupPolicy.ext_parameters[indexKey])
      );
    } else {
      this.formGroup
        ?.get('backup_res_auto_index')
        .setValue(backupPolicy.ext_parameters[indexKey]);
    }
  }

  getSlaDetail(sla): void {
    if (!sla) {
      return;
    }
    const backupPolicy = find(sla.policy_list, { type: PolicyType.BACKUP });
    const archivePolicy = find(sla.policy_list, { type: PolicyType.ARCHIVING });
    // 备份自动索引
    this.hasBackup =
      !isEmpty(backupPolicy) &&
      (!isUndefined(backupPolicy.ext_parameters.fine_grained_restore) ||
        !isUndefined(backupPolicy.ext_parameters.auto_index));

    if (this.hasBackup) {
      this.setAutoIndex(backupPolicy);
    }
    // 归档自动索引
    this.hasArchive =
      !isEmpty(archivePolicy) &&
      !isUndefined(archivePolicy.ext_parameters.auto_index);

    if (this.hasArchive) {
      if (!this.formGroup?.get('archive_res_auto_index')) {
        this.formGroup.addControl(
          'archive_res_auto_index',
          new FormControl(archivePolicy.ext_parameters.auto_index)
        );
      } else {
        this.formGroup
          ?.get('archive_res_auto_index')
          .setValue(archivePolicy.ext_parameters.auto_index);
      }
    }
  }
}
