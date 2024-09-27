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
import { OptionItem } from '@iux/live';
import {
  BaseUtilService,
  CookieService,
  I18NService,
  CommonConsts,
  DataMap,
  DataMapService,
  SftpManagerApiService,
  SFTP_USERNAME_BLACKLIST
} from 'app/shared';
import { assign, isUndefined, includes, isEmpty } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'cdm-add-user',
  templateUrl: './add-user.component.html',
  styleUrls: ['./add-user.component.less']
})
export class AddUserComponent implements OnInit {
  node;
  passLenVal = 8;
  maxLenVal = 64;
  passComplexVal = 3;
  passContiunVal = 3;
  passRepeatVal = 3;
  nameRepeatVal = 3;
  formGroup: FormGroup;
  roleOptions: OptionItem[];
  capacityUnits = this.dataMapService
    .toArray('Capacity_Unit')
    .filter(v => {
      return includes(
        [DataMap.Capacity_Unit.gb.value, DataMap.Capacity_Unit.tb.value],
        v.value
      );
    })
    .filter(v => {
      return (v.isLeaf = true);
    });
  invalidNameBeginLabel = this.i18n.get(
    'system_valid_sftp_username_begin_label'
  );
  invalidNameCombinationLabel = this.i18n.get(
    'system_valid_sftp_username_combin_label'
  );
  invalidLengthRangLabel = this.i18n.get('common_valid_length_rang_label', [
    6,
    32
  ]);
  invalidNameCharacters = this.i18n.get(
    'system_valid_sftp_username_characters_label'
  );
  pwdComplexTipLabel = this.i18n.get('system_sftp_pwdtip_label');
  nameComplexTipLabel = this.i18n.get('system_valid_sftp_username_label', [
    SFTP_USERNAME_BLACKLIST.toString()
      .split(',')
      .map((item, idex) => (idex === 0 ? item : ' ' + item))
      .join()
  ]);
  userNameErrorTip = assign(
    {
      invalidNameBegin: this.invalidNameBeginLabel,
      invalidNameLength: this.invalidLengthRangLabel,
      invalidNameCharacters: this.invalidNameCharacters,
      invalidNameCombination: this.invalidNameCombinationLabel
    },
    { ...this.baseUtilService.requiredErrorTip }
  );
  quotaErrorTip = assign(
    { ...this.baseUtilService.integerErrorTip },
    { ...this.baseUtilService.requiredErrorTip },
    {
      invalidMinSize: this.i18n.get('common_valid_minsize_label', [1]),
      invalidMaxQuota: this.i18n.get('system_sftp_max_quota_tip_label', [
        '800TB'
      ])
    }
  );

  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    public cookieService: CookieService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    public sftpManagerApiService: SftpManagerApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      userName: new FormControl('', {
        validators: [this.validSftpUserName()],
        updateOn: 'change'
      }),
      userPassword: new FormControl('', {
        validators: [
          this.validSftpPwd(),
          this.validUserNamePwd(),
          this.validConfirmPwdIsSame()
        ],
        updateOn: 'change'
      }),
      confirmPassword: new FormControl('', {
        validators: [
          this.validSftpPwd(),
          this.validUserNamePwd(),
          this.validUserPasswordIsSame()
        ],
        updateOn: 'change'
      }),
      quota: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.minSize(0),
          this.validMaxQuota()
        ],
        updateOn: 'change'
      }),
      capacity_unit: new FormControl(DataMap.Capacity_Unit.gb.value, {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    });
    this.pwdComplexTipLabel = this.i18n.get('system_sftp_pwdtip_label', [
      this.passLenVal,
      this.maxLenVal,
      this.passComplexVal === 4
        ? ''
        : this.i18n.get('system_sftp_pwd_complex_label', [this.passComplexVal]),
      this.passRepeatVal + 1,
      this.passContiunVal,
      this.nameRepeatVal,
      SFTP_USERNAME_BLACKLIST.toString()
        .split(',')
        .map((item, idex) => (idex === 0 ? item : ' ' + item))
        .join()
    ]);
    this.formGroup.get('userName').valueChanges.subscribe(res => {
      if (!this.formGroup.value.userPassword) {
        return;
      }
      setTimeout(() => {
        this.formGroup.get('userPassword').markAsTouched();
        this.formGroup.get('userPassword').updateValueAndValidity();
      }, 0);
    });
    this.formGroup.get('capacity_unit').valueChanges.subscribe(res => {
      if (!this.formGroup.value.quota) {
        return;
      }
      setTimeout(() => {
        this.formGroup.get('quota').markAsTouched();
        this.formGroup.get('quota').updateValueAndValidity();
      }, 0);
    });
  }

  validSftpUserName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }
      if (!control.value) {
        return { required: { value: control.value } };
      }

      const value = control.value;
      if (!CommonConsts.REGEX.sftpNameBegin.test(value)) {
        return { invalidNameBegin: { value: control.value } };
      }

      if (!CommonConsts.REGEX.sftpNameCombination.test(value)) {
        return { invalidNameCombination: { value: control.value } };
      }

      if (!/^.{6,32}$/.test(value)) {
        return { invalidNameLength: { value: control.value } };
      }

      if (
        !isUndefined(
          SFTP_USERNAME_BLACKLIST.find(
            item => control.value.toLowerCase() === item
          )
        )
      ) {
        return { invalidNameCharacters: { value: control.value } };
      }

      return null;
    };
  }

  validSftpPwd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return { required: { value: control.value } };
      }
      const pwd = control.value;
      if (!pwd) {
        return { invalidPwd: { value: control.value } };
      }

      if (pwd.length < this.passLenVal || pwd.length > this.maxLenVal) {
        return { invalidPwd: { value: control.value } };
      }

      if (
        Array.from(pwd)
          .reverse()
          .join('')
          .toLowerCase() === pwd.toLowerCase()
      ) {
        return { invalidPwd: { value: control.value } };
      }

      if (
        !isUndefined(
          SFTP_USERNAME_BLACKLIST.find(
            item => pwd.toLowerCase().indexOf(item) !== -1
          )
        )
      ) {
        return { invalidPwd: { value: control.value } };
      }

      if (new RegExp('(.)\\1{' + this.passRepeatVal + '}').test(pwd)) {
        return { invalidPwd: { value: control.value } };
      }

      const continuLetter = 'abcdefghijklmnopqrstuvwxyz';
      const reverseContinuLetter = 'zyxwvutsrqponmlkjihgfedcba';
      const continuKeyLetter = 'qwertyuiopasdfghjklzxcvbnm';
      const reverseContinuKeyLetter = 'mnbvcxzlkjhgfdsapoiuytrewq';
      const continuNum = '123456789';
      const reverseContinuNum = '987654321';
      const splitPwds = [];

      for (let i = 0; i <= pwd.length - this.passContiunVal; i++) {
        splitPwds.push(pwd.toLowerCase().substring(i, i + this.passContiunVal));
      }

      if (
        !isUndefined(
          splitPwds.find(item => continuLetter.indexOf(item) !== -1)
        ) ||
        !isUndefined(
          splitPwds.find(item => reverseContinuLetter.indexOf(item) !== -1)
        )
      ) {
        return { invalidPwd: { value: control.value } };
      }

      if (
        !isUndefined(
          splitPwds.find(item => continuKeyLetter.indexOf(item) !== -1) ||
            splitPwds.find(item => reverseContinuKeyLetter.indexOf(item) !== -1)
        )
      ) {
        return { invalidPwd: { value: control.value } };
      }

      if (
        !isUndefined(splitPwds.find(item => continuNum.indexOf(item) !== -1)) ||
        !isUndefined(
          splitPwds.find(item => reverseContinuNum.indexOf(item) !== -1)
        )
      ) {
        return { invalidPwd: { value: control.value } };
      }

      const regUpWord = /[A-Z]+/;
      const regDownWord = /[a-z]+/;
      const regNumber = /[0-9]+/;
      const regSpecialWord = /[~!@%\^\*\_=+\{\}\[\]:\,\.\|\/\?\u0020]+/;
      const regAll = /^[a-zA-Z0-9~!@%\^\*\_=+\{\}\[\]:\,\.\|\/\?\u0020]+$/;

      if (!regAll.test(pwd)) {
        return { invalidPwd: { value: control.value } };
      }

      let weight = 0;

      if (regNumber.test(pwd)) {
        weight += 1;
      }

      if (regDownWord.test(pwd)) {
        weight += 1;
      }

      if (regUpWord.test(pwd)) {
        weight += 1;
      }

      if (regSpecialWord.test(pwd)) {
        weight += 1;
      }

      if (+this.passComplexVal === 3) {
        return weight >= 3 ? null : { invalidPwd: { value: control.value } };
      }

      if (+this.passComplexVal === 4) {
        return weight === 4 ? null : { invalidPwd: { value: control.value } };
      }

      return { invalidName: { value: control.value } };
    };
  }

  validUserNamePwd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        isUndefined(this.formGroup) ||
        isEmpty(this.formGroup.value.userName)
      ) {
        return null;
      }

      const reverseName = this.formGroup.value.userName.split('').reverse();
      const _reverseName = reverseName.join('');
      const splitPwds = [];

      if (
        control.value.toLowerCase().indexOf(_reverseName.toLowerCase()) !== -1
      ) {
        return { invalidPwd: { value: control.value } };
      }

      for (let i = 0; i <= control.value.length - this.nameRepeatVal; i++) {
        splitPwds.push(
          control.value.toLowerCase().substring(i, i + this.nameRepeatVal)
        );
      }

      if (
        !isUndefined(
          splitPwds.find(
            item =>
              this.formGroup.value.userName.toLowerCase().indexOf(item) !== -1
          )
        )
      ) {
        return { invalidPwd: { value: control.value } };
      }

      return null;
    };
  }

  validUserPasswordIsSame(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      if (
        !!this.formGroup.value.userPassword &&
        this.formGroup.value.userPassword !== control.value
      ) {
        return { diffPwd: { value: control.value } };
      }

      if (!!this.formGroup.value.userPassword) {
        this.formGroup.get('userPassword').setErrors(null);
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

  validUserPwd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      const reverseName = this.formGroup.value.userName.split('').reverse();
      const _reverseName = reverseName.join('');
      if (
        control.value === this.formGroup.value.userName ||
        control.value === _reverseName
      ) {
        return { invalidPwd: { value: control.value } };
      }
      return null;
    };
  }

  validConfirmPwd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      const userPassword = this.formGroup.value.userPassword;
      if (userPassword !== control.value) {
        return { diffPwd: { value: control.value } };
      }
      return null;
    };
  }

  validMaxQuota(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }
      const currentCapacity =
        parseInt(control.value, 10) *
        this.dataMapService
          .toArray('Capacity_Unit')
          .find(item => item.value === this.formGroup.value.capacity_unit)
          .convertByte;

      if (currentCapacity > 800 * 1024 * 1024 * 1024 * 1024) {
        return { invalidMaxQuota: { value: control.value } };
      }
      return null;
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const limitSpaceQuota =
        parseInt(this.formGroup.value.quota, 10) *
        this.dataMapService
          .toArray('Capacity_Unit')
          .find(item => item.value === this.formGroup.value.capacity_unit)
          .convertByte;
      const sftpUserRequest = {
        limitSpaceQuota: limitSpaceQuota.toString(),
        unit: this.formGroup.value.capacity_unit,
        password: this.formGroup.value.userPassword,
        username: this.formGroup.value.userName
      };
      this.sftpManagerApiService
        .createSftpNormalUserUsingPOST({
          sftpUserRequest,
          memberEsn: this.node
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
}
