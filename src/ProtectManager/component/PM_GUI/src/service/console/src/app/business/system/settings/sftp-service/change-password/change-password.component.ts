import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  SFTP_USERNAME_BLACKLIST,
  SftpManagerApiService
} from 'app/shared';
import { I18NService } from 'app/shared/services/i18n.service';
import { isUndefined, assign, isEmpty } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'cdm-change-password',
  templateUrl: './change-password.component.html'
})
export class ChangePasswordComponent implements OnInit {
  user;
  node;
  formGroup: FormGroup;
  passLenVal = 8;
  passComplexVal = 3;
  maxLenVal = 64;
  passContiunVal = 3;
  passRepeatVal = 3;
  nameRepeatVal = 3;

  pwdComplexTipLabel = this.i18n.get('system_sftp_pwdtip_label');
  originalPasswordErrorTip = assign({}, this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });
  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public sftpManagerApiService: SftpManagerApiService
  ) {}

  ngOnInit() {
    this.initData();
    this.initPwd();
  }

  initPwd() {
    this.formGroup.controls['newPassword'].setValidators([
      this.baseUtilService.VALID.required(),
      this.validSftpPwd(),
      this.validUserNamePwd(),
      this.validPwdAndOldpwd(),
      this.validConfirmPwdIsSame()
    ]);
    this.formGroup.controls['confirmPassword'].setValidators([
      this.baseUtilService.VALID.required(),
      this.validSftpPwd(),
      this.validUserNamePwd(),
      this.validPwdAndOldpwd(),
      this.validNewPwdIsSame()
    ]);
    this.formGroup.controls['newPassword'].updateValueAndValidity();
    this.formGroup.controls['confirmPassword'].updateValueAndValidity();
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
      if (isUndefined(this.formGroup) || isEmpty(this.user.username)) {
        return null;
      }

      const reverseName = this.user.username.split('').reverse();
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
            item => this.user.username.toLowerCase().indexOf(item) !== -1
          )
        )
      ) {
        return { invalidPwd: { value: control.value } };
      }

      return null;
    };
  }
  validPwdAndOldpwd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      const originalPwd = this.formGroup.value.originalPassword;
      if (originalPwd === control.value) {
        return { sameHistoryPwd: { value: control.value } };
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

  initData() {
    this.formGroup = this.fb.group({
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
    this.formGroup.get('originalPassword').valueChanges.subscribe(res => {
      if (
        res !== this.formGroup.value.newPassword &&
        !!this.formGroup.value.newPassword
      ) {
        this.formGroup.get('newPassword').setErrors(null);
      }
      if (
        res === this.formGroup.value.newPassword &&
        !!this.formGroup.value.newPassword
      ) {
        this.formGroup.get('newPassword').updateValueAndValidity();
      }
      if (
        res !== this.formGroup.value.confirmPassword &&
        !!this.formGroup.value.confirmPassword
      ) {
        this.formGroup.get('confirmPassword').setErrors(null);
      }
      if (
        res === this.formGroup.value.confirmPassword &&
        !!this.formGroup.value.confirmPassword
      ) {
        this.formGroup.get('confirmPassword').updateValueAndValidity();
      }
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const sftpModifyPasswordRequest = {
        id: this.user.id,
        newPassword: this.formGroup.value.newPassword,
        password: this.formGroup.value.originalPassword
      };
      this.sftpManagerApiService
        .changePasswordUsingPOST({
          sftpModifyPasswordRequest,
          id: this.user.id,
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
