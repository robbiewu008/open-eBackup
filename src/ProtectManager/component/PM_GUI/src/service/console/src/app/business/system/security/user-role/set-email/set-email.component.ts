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
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn,
  Validators
} from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  I18NService,
  UsersApiService
} from 'app/shared';
import { assign, filter, isEmpty, reject } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-set-email',
  templateUrl: './set-email.component.html'
})
export class SetEmailComponent implements OnInit {
  showType = 'password';
  userId;
  formGroup: FormGroup;
  emailAddress;
  encryptedEmail = '';

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    public usersApiService: UsersApiService
  ) {}

  emailErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [254])
    },
    {
      invalidEmail: this.i18n.get('system_error_email_label')
    }
  );

  maskEmail(email) {
    if (isEmpty(email)) {
      return '';
    }
    const prefixStr = email.split('@')[0];
    return prefixStr.slice(0, 3) + '***' + '@' + email.split('@')[1];
  }

  ngOnInit(): void {
    this.encryptedEmail = this.maskEmail(this.emailAddress);
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      email: new FormControl(this.encryptedEmail, {
        validators: [
          this.validEmail(),
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(254)
        ]
      })
    });
  }

  validEmail(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const email = control.value;
      if (!CommonConsts.REGEX.email.test(email)) {
        return { invalidEmail: { value: control.value } };
      }
      return null;
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = {
        emailAddress: this.emailAddress
      };
      this.usersApiService
        .updateEmailUsingPUT({
          pwdRetrievalEmailRequest: params as any,
          userId: this.userId
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }
  emailBlur() {
    if (this.formGroup.controls.email.status !== 'VALID') {
      return;
    }
    this.emailAddress = this.formGroup.value.email;
    this.formGroup.get('email').setValue(this.maskEmail(this.emailAddress));
  }

  emailFocus() {
    if (this.emailAddress) {
      this.formGroup.get('email').setValue(this.emailAddress);
    }
  }
}
