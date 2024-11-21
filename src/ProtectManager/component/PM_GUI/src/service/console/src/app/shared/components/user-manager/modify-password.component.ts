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
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn,
  AbstractControl
} from '@angular/forms';
import { SecurityApiService } from 'app/shared';
import {
  I18NService,
  BaseUtilService,
  CookieService,
  GlobalService
} from '../../services';
import { isUndefined, assign } from 'lodash';
import { ModalRef } from '@iux/live';

@Component({
  selector: 'aui-modify-password',
  templateUrl: 'modify-password.component.html'
})
export class ModifyPasswordComponent implements OnInit {
  passwdFormGroup: FormGroup;
  passLenVal = 8;
  passComplexVal = 4;
  maxLenVal = 64;

  pwdComplexTipLabel = this.i18n.get('common_pwdtip_label');
  originalPasswordErrorTip = assign({}, this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });
  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    public modal: ModalRef,
    public baseUtilService: BaseUtilService,
    private securityApiService: SecurityApiService,
    private cookieService: CookieService,
    public globalService: GlobalService
  ) {}

  ngOnInit() {
    this.initData();
    this.initPwd();
  }

  initPwd() {
    this.securityApiService.getUsingGET1({}).subscribe(res => {
      this.passLenVal = res.passLenVal;
      this.passComplexVal = res.passComplexVal;
      this.passwdFormGroup.controls['newPassword'].setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.password(
          this.passLenVal,
          this.passComplexVal,
          this.maxLenVal
        ),
        this.validPwdAndOldpwd(),
        this.validConfirmPwdIsSame()
      ]);
      this.passwdFormGroup.controls['confirmPassword'].setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.password(
          this.passLenVal,
          this.passComplexVal,
          this.maxLenVal
        ),
        this.validPwdAndOldpwd(),
        this.validNewPwdIsSame()
      ]);
      this.passwdFormGroup.controls['newPassword'].updateValueAndValidity();
      this.passwdFormGroup.controls['confirmPassword'].updateValueAndValidity();
      this.pwdComplexTipLabel = this.i18n.get('common_pwdtip_label', [
        this.passLenVal,
        64,
        this.passComplexVal === 2
          ? this.i18n.get('common_pwd_complex_label')
          : '',
        2,
        ''
      ]);
    });
  }

  validPwdAndOldpwd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.passwdFormGroup)) {
        return null;
      }

      const originalPwd = this.passwdFormGroup.value.originalPassword;
      if (originalPwd === control.value) {
        return { sameHistoryPwd: { value: control.value } };
      }
      return null;
    };
  }

  validConfirmPwdIsSame(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.passwdFormGroup)) {
        return null;
      }

      if (
        !!this.passwdFormGroup.value.confirmPassword &&
        this.passwdFormGroup.value.confirmPassword !== control.value
      ) {
        return { diffPwd: { value: control.value } };
      }

      if (!!this.passwdFormGroup.value.confirmPassword) {
        this.passwdFormGroup.get('confirmPassword').setErrors(null);
      }

      return null;
    };
  }

  validNewPwdIsSame(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.passwdFormGroup)) {
        return null;
      }

      if (
        !!this.passwdFormGroup.value.newPassword &&
        this.passwdFormGroup.value.newPassword !== control.value
      ) {
        return { diffPwd: { value: control.value } };
      }

      if (!!this.passwdFormGroup.value.newPassword) {
        this.passwdFormGroup.get('newPassword').setErrors(null);
      }
      return null;
    };
  }

  initData() {
    this.passwdFormGroup = this.fb.group({
      originalPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ],
        updateOn: 'change'
      }),
      newPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.password(8, 4, 64)
        ],
        updateOn: 'change'
      }),
      confirmPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.password(8, 4, 64),
          (control: FormControl) => {
            if (
              control.parent &&
              control.parent.value.newPassword === control.value
            ) {
              return null;
            }
            return { diffPwd: true };
          }
        ],
        updateOn: 'change'
      })
    });
    this.passwdFormGroup.get('originalPassword').valueChanges.subscribe(res => {
      if (
        res !== this.passwdFormGroup.value.newPassword &&
        !!this.passwdFormGroup.value.newPassword
      ) {
        this.passwdFormGroup.get('newPassword').setErrors(null);
      }
      if (
        res === this.passwdFormGroup.value.newPassword &&
        !!this.passwdFormGroup.value.newPassword
      ) {
        this.passwdFormGroup.get('newPassword').updateValueAndValidity();
      }
      if (
        res !== this.passwdFormGroup.value.confirmPassword &&
        !!this.passwdFormGroup.value.confirmPassword
      ) {
        this.passwdFormGroup.get('confirmPassword').setErrors(null);
      }
      if (
        res === this.passwdFormGroup.value.confirmPassword &&
        !!this.passwdFormGroup.value.confirmPassword
      ) {
        this.passwdFormGroup.get('confirmPassword').updateValueAndValidity();
      }
    });
  }
}
