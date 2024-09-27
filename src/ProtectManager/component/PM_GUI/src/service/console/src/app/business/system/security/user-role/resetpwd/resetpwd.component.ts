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
  CookieService,
  UsersApiService,
  WarningMessageService
} from 'app/shared';
import { I18NService } from 'app/shared/services/i18n.service';
import { isUndefined, assign, upperCase } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'cdm-resetpwd',
  templateUrl: './resetpwd.component.html'
})
export class ResetpwdComponent implements OnInit {
  user;
  pwdComplexTipLabel;
  formGroup: FormGroup;
  maxLenVal = 64;
  adminPasswordErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public cookieService: CookieService,
    public baseUtilService: BaseUtilService,
    public usersApiService: UsersApiService,
    public warningMessageService: WarningMessageService
  ) {}

  ngOnInit(): void {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      adminPassword: new FormControl('', {
        validators: [
          Validators.required,
          this.baseUtilService.VALID.maxLength(64)
        ],
        updateOn: 'change'
      }),
      newPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.password(
            this.user.passLenVal,
            this.user.passComplexVal,
            this.maxLenVal
          ),
          this.validPwdAndUsername(),
          this.validConfirmPwdIsSame()
        ],
        updateOn: 'change'
      }),
      confirmPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.password(
            this.user.passLenVal,
            this.user.passComplexVal,
            this.maxLenVal
          ),
          this.validPwdAndUsername(),
          this.validNewPwdIsSame()
        ],
        updateOn: 'change'
      })
    });

    this.pwdComplexTipLabel = this.i18n.get('common_pwdtip_label', [
      this.user.passLenVal,
      64,
      this.user.passComplexVal === 2
        ? this.i18n.get('common_pwd_complex_label')
        : '',
      2,
      this.i18n.get('common_pwdtip_five_six_label')
    ]);
  }

  validPwdAndUsername(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      const reverseName = this.user.userName.split('').reverse();
      const _reverseName = reverseName.join('');
      if (
        control.value === this.user.userName ||
        control.value === _reverseName ||
        upperCase(control.value).indexOf(upperCase(this.user.userName)) !== -1
      ) {
        return { invalidPwd: { value: control.value } };
      }

      return null;
    };
  }

  validConfirmPwdIsSame(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      if (
        !!this.formGroup.value.confirmPassword &&
        this.formGroup.value.confirmPassword !== control.value
      ) {
        return { diffPwd: { value: control.value } };
      }

      if (!!this.formGroup.value.confirmPassword) {
        this.formGroup.get('confirmPassword').setErrors(null);
      }

      return null;
    };
  }

  validNewPwdIsSame(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      if (
        !!this.formGroup.value.newPassword &&
        this.formGroup.value.newPassword !== control.value
      ) {
        return { diffPwd: { value: control.value } };
      }

      if (!!this.formGroup.value.newPassword) {
        this.formGroup.get('newPassword').setErrors(null);
      }

      return null;
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.warningMessageService.create({
        content: this.i18n.get('system_user_restpwd_tip_label', [
          this.user.userName
        ]),
        onOK: () => {
          const password = this.formGroup.value;
          this.usersApiService
            .resetPasswordUsingPUT({ password, userId: this.user.userId })
            .subscribe({
              next: () => {
                observer.next();
                observer.complete();
              },
              error: error => {
                observer.error(error);
                observer.complete();
              }
            });
        },
        onCancel: () => {
          observer.error({});
          observer.complete();
        },
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            observer.error({});
            observer.complete();
          }
        }
      });
    });
  }
}
