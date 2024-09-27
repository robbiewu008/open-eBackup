import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { BaseUtilService, I18NService, IbmcAlarmService } from 'app/shared';
import { isEmpty, isUndefined } from 'lodash';
import { finalize } from 'rxjs/operators';

@Component({
  selector: 'aui-ibmc-config',
  templateUrl: './ibmc-config.component.html',
  styleUrls: ['./ibmc-config.component.less']
})
export class IbmcConfigComponent implements OnInit {
  formGroup: FormGroup;
  isModify = false;
  configType = {
    sync: '1',
    local: '2'
  };
  userNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_length_rang_label', [1, 16]),
    invalidMinLength: this.i18n.get('common_valid_length_rang_label', [1, 16])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidPwd: this.i18n.get('common_invalid_inputtext_label'),
    diffPwd: this.i18n.get('common_diffpwd_label')
  };
  username;
  password;
  pwdComplexTipLabel = this.i18n.get('system_ibmc_password_tip_label');
  hasLoad = false;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private ibmcAlarmService: IbmcAlarmService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getConfig();
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(this.configType.local),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1),
          this.baseUtilService.VALID.maxLength(16)
        ]
      }),
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      newPassword: new FormControl(''),
      confirmPassword: new FormControl('')
    });
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === this.configType.local) {
        this.formGroup.get('newPassword').clearValidators();
        this.formGroup.get('confirmPassword').clearValidators();
      } else {
        this.formGroup
          .get('newPassword')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.password(8, 2, 20, null),
            this.validUserNamePwd(),
            this.validConfirmPwdIsSame()
          ]);
        this.formGroup
          .get('confirmPassword')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.password(8, 2, 20, null),
            this.validUserNamePwd(),
            this.validNewPwdIsSame()
          ]);
      }
      this.formGroup.get('newPassword').updateValueAndValidity();
      this.formGroup.get('confirmPassword').updateValueAndValidity();
    });
  }

  validUserNamePwd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || !this.formGroup.value.username) {
        return null;
      }

      const reverseName = this.formGroup.value.username.split('').reverse();
      const _reverseName = reverseName.join('');
      if (
        control.value === this.formGroup.value.username ||
        control.value === _reverseName
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

  getConfig() {
    this.ibmcAlarmService
      .iBMCUserNameUsingGet({})
      .pipe(
        finalize(() => {
          this.hasLoad = true;
        })
      )
      .subscribe(
        res => {
          try {
            this.username = JSON.parse(res)?.ibmcuserName;
            this.password = JSON.parse(res)?.ibmcuserName ? '******' : '';
            if (isEmpty(this.username)) {
              this.isModify = true;
            }
          } catch (error) {
            this.username = '';
            this.password = '';
            this.isModify = true;
          }
          this.formGroup.patchValue({
            username: this.username,
            password: '',
            newPassword: '',
            confirmPassword: ''
          });
        },
        () => {
          this.isModify = true;
        }
      );
  }

  modify() {
    this.isModify = true;
    this.getConfig();
  }

  cancel() {
    this.isModify = false;
    this.getConfig();
  }

  save() {
    if (this.formGroup.invalid) {
      return;
    }
    if (this.formGroup.value.type === this.configType.sync) {
      this.ibmcAlarmService
        .configureIBMCUserSyncUsingPost({
          iBMCUserRequest: {
            userName: this.formGroup.value.username,
            userPassword: this.formGroup.value.password,
            newPassword: this.formGroup.value.newPassword,
            confirmPassword: this.formGroup.value.confirmPassword
          }
        })
        .subscribe(() => {
          this.isModify = false;
          this.getConfig();
          this.formGroup.patchValue({});
        });
    } else {
      this.ibmcAlarmService
        .configureIBMCUserLocalUsingPost({
          iBMCUserRequest: {
            userName: this.formGroup.value.username,
            userPassword: this.formGroup.value.password,
            newPassword: '',
            confirmPassword: ''
          }
        })
        .subscribe(() => {
          this.isModify = false;
          this.getConfig();
          this.formGroup.patchValue({});
        });
    }
  }
}
