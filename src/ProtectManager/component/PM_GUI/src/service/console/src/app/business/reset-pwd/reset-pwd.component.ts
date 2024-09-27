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
  ValidatorFn
} from '@angular/forms';
import { Router } from '@angular/router';
import {
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMap,
  GlobalService,
  GROUP_COMMON,
  I18NService,
  LANGUAGE,
  RESET_PSWD_NAVIGATE_STATUS,
  UsersApiService
} from 'app/shared';
import { assign, includes } from 'lodash';

@Component({
  selector: 'aui-reset-pwd',
  templateUrl: './reset-pwd.component.html',
  styleUrls: ['./reset-pwd.component.less']
})
export class ResetPwdComponent implements OnInit {
  groupOptions = GROUP_COMMON;
  formGroup: FormGroup;
  userId;
  randomCode;
  isSend = false;
  vertifycodeTip = this.i18n.get('sysadmin_check_vertifycode_tip_label');
  languageLabel =
    this.i18n.language.toLowerCase() === LANGUAGE.CN
      ? this.i18n.get('common_english_label')
      : this.i18n.get('common_chinese_label');
  isCloudBackup = includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value
    ],
    this.i18n.get('deploy_type')
  );
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  countdownLabel = this.i18n.get('common_countdown_label', [900]);

  constructor(
    private router: Router,
    private i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    public usersApiService: UsersApiService,
    private cookieService: CookieService
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
  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [254])
  };

  ngOnInit(): void {
    this.initForm();
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
      if (email.split('@')[1] && email.split('@')[1].length > 255) {
        return { invalidEmail: { value: control.value } };
      }
      return null;
    };
  }

  initForm() {
    this.formGroup = this.fb.group({
      userName: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      emailAdress: new FormControl('', {
        validators: [this.validEmail(), this.baseUtilService.VALID.required()]
      }),
      verificationCode: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
  }

  checkVerifyCode() {
    const params = {
      userName: this.formGroup.value.userName,
      emailAddress: this.formGroup.value.emailAdress,
      language: this.i18n.language === LANGUAGE.CN ? 1 : 2
    };
    this.usersApiService
      .sendVerifyCodeUsingPOST({
        verifyCodeSendRequest: params as any
      })
      .subscribe(res => {
        this.userId = JSON.parse(res);
        this.isSend = true;
        let time = 900;
        const interval = setInterval(() => {
          time--;
          this.countdownLabel = this.i18n.get('common_countdown_label', [time]);
        }, 1e3);
        const timeOut = setTimeout(() => {
          clearInterval(interval);
          clearTimeout(timeOut);
          this.isSend = false;
        }, 900 * 1e3);
      });
  }

  submit() {
    const params = {
      userId: this.userId?.uuid,
      userName: this.formGroup.value.userName,
      randomCode: this.formGroup.value.verificationCode
    };
    this.usersApiService
      .checkVerifyCodeUsingPOST({
        verifyCodeDto: params as any
      })
      .subscribe((res: any) => {
        res = JSON.parse(res);
        this.userId = res.userId;
        this.randomCode = res.randomCode;
        RESET_PSWD_NAVIGATE_STATUS.randomCode = this.randomCode;
        RESET_PSWD_NAVIGATE_STATUS.userId = this.userId;
        RESET_PSWD_NAVIGATE_STATUS.userName = this.formGroup.value.userName;
        this.router.navigate(['/login']);
      });
  }

  toggleLanguage() {
    this.cookieService.remove('userId');
    this.i18n.changeLanguage(
      this.i18n.language.toLowerCase() === LANGUAGE.CN
        ? LANGUAGE.EN
        : LANGUAGE.CN
    );
  }
}
