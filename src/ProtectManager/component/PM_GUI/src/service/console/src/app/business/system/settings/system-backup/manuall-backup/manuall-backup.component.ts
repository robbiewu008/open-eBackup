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
import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { BaseUtilService, I18NService, SysbackupApiService } from 'app/shared';
import { assign } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-manuall-backup',
  templateUrl: './manuall-backup.component.html'
})
export class ManuallBackupComponent implements OnInit {
  formGroup: FormGroup;
  descErrorTip = assign({}, this.baseUtilService.lengthErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  });

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private sysbackupApiService: SysbackupApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      desc: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(255)],
        updateOn: 'change'
      })
    });
  }

  onOk(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const rsq = {
        ...this.formGroup.value
      };
      this.sysbackupApiService.backupUsingPOST({ rsq }).subscribe(
        () => {
          observer.next();
          observer.complete();
        },
        error => {
          observer.error(error);
          observer.complete();
        }
      );
    });
  }
}
