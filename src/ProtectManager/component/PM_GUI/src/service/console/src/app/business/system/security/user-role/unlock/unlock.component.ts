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
import {
  I18NService,
  UsersApiService,
  BaseUtilService,
  WarningMessageService
} from 'app/shared';
import { FormGroup, FormControl, FormBuilder } from '@angular/forms';
import { Observable, Observer } from 'rxjs';
import { assign } from 'lodash';

@Component({
  selector: 'aui-unlock',
  templateUrl: './unlock.component.html',
  styleUrls: ['./unlock.component.less']
})
export class UnlockComponent implements OnInit {
  user;
  formGroup: FormGroup;
  passwordErrorTip = assign({}, this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });

  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public usersApiService: UsersApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ],
        updateOn: 'change'
      })
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const userId = this.user.userId;
      const password = this.formGroup.value.password;
      this.usersApiService.unlockUsingPUT({ userId, password }).subscribe({
        next: () => {
          observer.next();
          observer.complete();
        },
        error: error => {
          observer.error(error);
          observer.complete();
        }
      });
    });
  }
}
