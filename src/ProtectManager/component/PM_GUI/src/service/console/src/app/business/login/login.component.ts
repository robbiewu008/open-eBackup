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
import { Component, OnDestroy, OnInit } from '@angular/core';
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
  CookieService,
  getAccessibleViewList,
  GlobalService,
  LANGUAGE,
  RoleType,
  ErrorCode,
  LogoutType,
  DataMap,
  RESET_PSWD_NAVIGATE_STATUS,
  DataMapService,
  timeZones,
  ExceptionService,
  SYSTEM_TIME
} from 'app/shared';
import {
  SecurityApiService,
  UsersApiService,
  AuthApiService,
  ADFSService,
  RsaService
} from 'app/shared/api/services';
import {
  AuthApiService as AuthLoginApiService,
  LoginResponseBody
} from 'app/shared/services/auth-api.service';
import { I18NService } from 'app/shared/services/i18n.service';
import {
  IoemInfo,
  WhiteboxService,
  IMAGE_PATH_PREFIX as WITHEBOX_PATH_PREFIX
} from 'app/shared/services/whitebox.service';
import {
  assign,
  cloneDeep,
  eq,
  find,
  first,
  get,
  includes,
  isEmpty,
  isNil,
  isUndefined,
  omit,
  pick,
  set,
  toString,
  upperCase
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { map, switchMap } from 'rxjs/operators';
import { MessageService } from '@iux/live';
import { SendDynamicCodeRequest } from 'app/shared/api/models';
import { DatePipe } from '@angular/common';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

type ILastLoginInfo = Pick<
  LoginResponseBody,
  'lastLoginIp' | 'lastLoginTime' | 'lastLoginZone'
>;

@Component({
  selector: 'aui-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.less'],
  providers: [DatePipe]
})
export class LoginComponent implements OnInit, OnDestroy {
  resetPwdParams;
  language;
  passLenVal;
  passComplexVal;
  isResetPassword = false;
  maxLenVal = 64;
  isLogging = false;
  isAdfsLogging = false;
  isModifyPasswd = false;
  hasVerificationCode = false;
  loginFormGroup: FormGroup;
  passwdFormGroup: FormGroup;

  okLabel = this.i18n.get('common_ok_label');
  loginLabel = this.i18n.get('common_login_label');
  loggingLabel = this.i18n.get('common_logging_label');
  modifyLabel = this.i18n.get('common_modify_label');
  cancelLabel = this.i18n.get('common_cancel_label');
  userNameLabel = this.i18n.get('common_username_label');
  passwordLabel = this.i18n.get('common_password_label');
  newPasswordLabel = this.i18n.get('common_newpwd_label');
  verifyCodeLabel = this.i18n.get('common_verify_code_label');
  confirmPasswordLabel = this.i18n.get('common_confirmpwd_label');
  originalPasswordLabel = this.i18n.get('common_originalpwd_label');
  verifyCodeRefreshLabel = this.i18n.get('common_verify_refresh_label');
  passwordDescLabel = this.i18n.get('common_modify_password_desc_label');
  adfsLoginLabel = this.i18n.get('common_adfs_login_label');
  adfsLoginingLabel = this.i18n.get('common_adfs_logining_label');
  pwdErrorTip = this.i18n.get('common_pwdtip_label');
  copyRightLabel = this.i18n.get('common_copy_right_label', [
    new Date().getFullYear() === 2021
      ? 2021
      : `2021-${new Date().getFullYear()}`
  ]);
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
  userTypeOptions = this.dataMapService
    .toArray('loginUserType')
    .filter(item => {
      item.isLeaf = true;
      return includes(
        [DataMap.loginUserType.local.value, DataMap.loginUserType.ldap.value],
        item.value
      );
    });
  needDynamicCode = false;
  showSendDynamicCodeBtn = true;
  nextSendDynamicCodeTime = 60;
  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  DYNAMIC_CODE_ERROR_CODE = '1677752064';

  productName = this.whitebox.isWhitebox
    ? (this.whitebox.oem as IoemInfo).vendor
    : this.baseUtilService.getProductName();

  loginSendDisable = false;
  modifySendDisable = false;
  adfsLoginSendDisable = false;
  dataMap = DataMap;
  _includes = includes;

  // 登录错误信息
  loginErrorMsg: string;

  constructor(
    public router: Router,
    public fb: FormBuilder,
    public i18n: I18NService,
    public datePipe: DatePipe,
    public whitebox: WhiteboxService,
    private cookieService: CookieService,
    private baseUtilService: BaseUtilService,
    private usersApiService: UsersApiService,
    private authApiService: AuthLoginApiService,
    private globalService: GlobalService,
    private securityApiService: SecurityApiService,
    private message: MessageService,
    private dataMapService: DataMapService,
    private authService: AuthApiService,
    private adfsService: ADFSService,
    private exceptionService: ExceptionService,
    private rsaService: RsaService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnDestroy(): void {
    document.body.classList?.remove('light-loading');
  }

  ngOnInit() {
    document.body.classList?.add('light-loading');
    this.initBackgroundImage();
    this.initLoginForm();
    this.initVerifyCode();
    this.checkLoginFormStates();
    this.resetPassword();
    this.initCopyRight();
    this.getAdfsErrorCode('errorCode');
  }

  getAdfsErrorCode(name) {
    const errorUrl = this.router.url;
    const reg = new RegExp('(^|&)' + name + '=([^&]*)(&|$)', 'i');
    const num = errorUrl.indexOf('?');
    const str = errorUrl.substring(num + 1);
    const r = str.match(reg);
    if (r !== null) {
      const resCode = decodeURIComponent(r[2]);
      this.message.error(this.i18n.get(resCode), {
        lvMessageKey: resCode,
        lvShowCloseButton: true
      });
      if (resCode === '1677929520' || resCode === '1677929492') {
        this.adfsService.adfsLoginForward({}).subscribe((resData: any) => {
          window.open(
            resData?.logoutForwardUrl,
            '_blank',
            'scrollbars=yes,resizable=yes,statebar=no,width=400,height=400,left=200, top=100'
          );
        });
      }
    }
  }

  initCopyRight() {
    const isZh = this.i18n.language === LANGUAGE.CN;
    this.copyRightLabel = this.whitebox.isWhitebox
      ? this.whitebox.oem[`copyright_${isZh ? 'zh' : 'en'}`]
      : this.i18n.get('common_copy_right_label', [
          new Date().getFullYear() === 2021
            ? 2021
            : `2021-${new Date().getFullYear()}`
        ]);
  }

  private _despatchBgImg() {
    if (this.whitebox.isWhitebox) {
      return WITHEBOX_PATH_PREFIX + 'main.png';
    }
    if (this.isCyberEngine) {
      return 'assets/img/cyber_login.png';
    }
    return 'assets/img/bg_login.png';
  }

  initBackgroundImage() {
    const container = first(document.getElementsByClassName('login-container'));

    set(
      container,
      'style',
      `background:url(${this._despatchBgImg()}) no-repeat, linear-gradient(to right, #0a0c14, #1b2a3f);background-size:cover;`
    );
  }

  initLoginForm() {
    this.loginFormGroup = this.fb.group({
      userType: new FormControl(DataMap.loginUserType.local.value),
      userName: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    });

    this.globalService.getState('isLoginView').subscribe(res => {
      this.isModifyPasswd && this.cancel();
    });

    this.globalService.getState(ErrorCode.UserStatusAbnormal).subscribe(res => {
      this.toLoginView();
    });
  }

  watchName() {
    this.loginErrorMsg = '';
    if (!this.isOceanProtect) {
      return;
    }
    if (this.needDynamicCode) {
      this.clearDynamicCodeValidator();
    }
  }

  resetErrorMessage() {
    this.loginErrorMsg = '';
  }

  initModifyForm(
    passLenVal: number,
    passComplexVal: number,
    maxLenVal: number = 64
  ) {
    this.passwdFormGroup = this.fb.group({
      userName: new FormControl({
        value: this.isResetPassword
          ? this.resetPwdParams?.userName
          : this.loginFormGroup.value.userName,
        disabled: true
      }),
      originalPassword: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      newPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validUserNamePwd(),
          this.baseUtilService.VALID.password(
            passLenVal,
            passComplexVal,
            maxLenVal
          ),
          this.validPwdAndOldpwd(),
          this.validConfirmPwdIsSame()
        ],
        updateOn: 'change'
      }),
      confirmPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validUserNamePwd(),
          this.baseUtilService.VALID.password(
            passLenVal,
            passComplexVal,
            maxLenVal
          ),
          this.validPwdAndOldpwd(),
          this.validNewPwdIsSame()
        ],
        updateOn: 'change'
      })
    });
    if (this.isResetPassword) {
      this.passwdFormGroup.get('originalPassword').clearValidators();
      this.passwdFormGroup.get('originalPassword').updateValueAndValidity();
      this.passwdFormGroup
        .get('newPassword')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.password(
            passLenVal,
            passComplexVal,
            maxLenVal
          ),
          this.validConfirmPwdIsSame()
        ]);
      this.passwdFormGroup
        .get('confirmPassword')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.password(
            passLenVal,
            passComplexVal,
            maxLenVal
          ),
          this.validNewPwdIsSame()
        ]);
      this.passwdFormGroup.get('newPassword').updateValueAndValidity();
      this.passwdFormGroup.get('confirmPassword').updateValueAndValidity();
    }
  }

  checkLoginFormStates() {
    const currentInfo = this.getBrowserInfo() as any;
    const brower = currentInfo.browser.toLowerCase();
    const isIE = brower.indexOf('IE:11.0');
    const isIEAndZh = isIE && this.i18n.language === LANGUAGE.CN;
    const formGroup = this.loginFormGroup;
    let markPristineCountUsername = 0;
    let markPristineCountPassword = 0;
    let maxCountNeeded = 0;

    // IE环境下，中文的placehoder会触发change事件，username和password各一次，
    // IE环境下英文，focus也会触发change，这个时候处理一次即可
    // 这两次触发的change需要置回pristine状态，避免一开始就触发输入框校验报不能为空的错误
    if (isIEAndZh) {
      maxCountNeeded = 2;
    } else if (isIE) {
      maxCountNeeded = 1;
    }

    if (isIE) {
      const usernameChange$ = formGroup
        .get('userName')
        .valueChanges.subscribe(() => {
          if (markPristineCountUsername < maxCountNeeded) {
            markPristineCountUsername++;
            formGroup.get('userName').markAsPristine();
          } else if (usernameChange$) {
            usernameChange$.unsubscribe();
          }
        });

      const passwordChange$ = formGroup
        .get('password')
        .valueChanges.subscribe(() => {
          if (markPristineCountPassword < maxCountNeeded) {
            markPristineCountPassword++;
            formGroup.get('password').markAsPristine();
          } else if (passwordChange$) {
            passwordChange$.unsubscribe();
          }
        });
    }
  }

  getBrowserInfo() {
    const Msie = /(msie\s|trident.*rv:)([\w.]+)/;
    const Firefox = /(firefox)\/([\w.]+)/;
    const Chrome = /(chrome)\/([\w.]+)/;
    const Opera = /(opera).+version\/([\w.]+)/;
    const Safari = /version\/([\w.]+).*(safari)/;
    const agent = navigator.userAgent.toLowerCase();
    let match = Msie.exec(agent);
    if (null != match) {
      return {
        browser: 'IE',
        version: match[2] || '0'
      };
    }
    match = Firefox.exec(agent);
    if (null != match) {
      return {
        browser: match[1] || '',
        version: match[2] || '0'
      };
    }
    match = Chrome.exec(agent);
    if (null != match) {
      return {
        browser: match[1] || '',
        version: match[2] || '0'
      };
    }
    match = Opera.exec(agent);
    if (null != match) {
      return {
        browser: match[1] || '',
        version: match[2] || '0'
      };
    }
    match = Safari.exec(agent);
    if (null != match) {
      return {
        browser: match[2] || '',
        version: match[1] || '0'
      };
    }
    return 'false';
  }

  validUserNamePwd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.loginFormGroup)) {
        return null;
      }

      const reverseName = this.loginFormGroup.value.userName
        .split('')
        .reverse();
      const _reverseName = reverseName.join('');
      if (
        control.value === this.loginFormGroup.value.userName ||
        control.value === _reverseName ||
        upperCase(control.value).indexOf(
          upperCase(this.loginFormGroup.value.userName)
        ) !== -1
      ) {
        return { invalidPwd: { value: control.value } };
      }

      return null;
    };
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

  clearPasswordValidator() {
    const ctrl = this.loginFormGroup.get('password');
    if (!ctrl) {
      return;
    }
    ctrl.setValue('');
    ctrl.clearValidators();
    ctrl.updateValueAndValidity();
  }

  clearOriginalPasswordValidator() {
    const ctrl = this.passwdFormGroup.get('originalPassword');
    if (!ctrl) {
      return;
    }
    ctrl.setValue('');
    ctrl.clearValidators();
    ctrl.updateValueAndValidity();
  }

  clearNewPasswordValidator() {
    const ctrl = this.passwdFormGroup.get('newPassword');
    if (!ctrl) {
      return;
    }
    ctrl.setValue('');
    ctrl.clearValidators();
    ctrl.updateValueAndValidity();
  }

  clearConfirmPasswordValidator() {
    const ctrl = this.passwdFormGroup.get('confirmPassword');
    if (!ctrl) {
      return;
    }
    ctrl.setValue('');
    ctrl.clearValidators();
    ctrl.updateValueAndValidity();
  }

  clearVerifyCodeValidator() {
    const ctrl = this.loginFormGroup.get('verifyCode');
    if (!ctrl) {
      return;
    }
    ctrl.setValue('');
    ctrl.clearValidators();
    ctrl.updateValueAndValidity();
  }

  setPasswordValidator() {
    const ctrl = this.loginFormGroup.get('password');
    if (!ctrl) {
      return;
    }
    ctrl.setValidators([this.baseUtilService.VALID.required()]);
    this.loginFormGroup.get('password').updateValueAndValidity();
  }

  setOriginalPasswordValidator() {
    const ctrl = this.passwdFormGroup.get('originalPassword');
    if (!ctrl) {
      return;
    }
    if (this.isResetPassword) {
      ctrl.clearValidators();
    } else {
      ctrl.setValidators([this.baseUtilService.VALID.required()]);
    }

    this.passwdFormGroup.get('originalPassword').updateValueAndValidity();
  }

  setNewPasswordValidator() {
    const ctrl = this.passwdFormGroup.get('newPassword');
    if (!ctrl) {
      return;
    }
    if (this.isResetPassword) {
      ctrl.setValidators([
        this.baseUtilService.VALID.required(),
        this.validConfirmPwdIsSame(),
        this.baseUtilService.VALID.password(8, 4, 64)
      ]);
    } else {
      ctrl.setValidators([
        this.baseUtilService.VALID.required(),
        this.validUserNamePwd(),
        this.baseUtilService.VALID.password(
          this.passLenVal,
          this.passComplexVal,
          this.maxLenVal
        ),
        this.validConfirmPwdIsSame(),
        this.validPwdAndOldpwd()
      ]);
    }
    this.passwdFormGroup.get('newPassword').updateValueAndValidity();
  }

  setConfirmPasswordValidator() {
    const ctrl = this.passwdFormGroup.get('confirmPassword');
    if (!ctrl) {
      return;
    }
    if (this.isResetPassword) {
      ctrl.setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.password(8, 4, 64),
        this.validNewPwdIsSame()
      ]);
    } else {
      ctrl.setValidators([
        this.baseUtilService.VALID.required(),
        this.validUserNamePwd(),
        this.baseUtilService.VALID.password(
          this.passLenVal,
          this.passComplexVal,
          this.maxLenVal
        ),
        this.validNewPwdIsSame(),
        this.validPwdAndOldpwd()
      ]);
    }
    this.passwdFormGroup.get('confirmPassword').updateValueAndValidity();
  }

  setVerifyCodeValidator() {
    const ctrl = this.loginFormGroup.get('verifyCode');
    if (!ctrl) {
      return;
    }
    ctrl.setValidators([this.baseUtilService.VALID.required()]);
    this.loginFormGroup.get('verifyCode').updateValueAndValidity();
  }

  changeVerifyCode() {
    const node: any = document.getElementById('verifyCodeImg');
    if (node) {
      const url = node.src;
      if (url.indexOf('?') >= 0) {
        node.src = url.split('?')[0] + '?id=' + this.getUuid();
      } else {
        node.src = url + '?id=' + this.getUuid();
      }
    }
  }

  getUuid() {
    let d = new Date().getTime();
    const uuid = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, c => {
      // tslint:disable-next-line: no-bitwise
      const r = (d + Math.random() * 16) % 16 | 0;
      d = Math.floor(d / 16);
      // tslint:disable-next-line: no-bitwise
      return (c === 'x' ? r : (r & 0x3) | 0x8).toString(16);
    });
    return uuid;
  }

  getSecurityPolicy(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      this.securityApiService.getUsingGET1({}).subscribe(res => {
        this.passLenVal = res.passLenVal;
        this.passComplexVal = res.passComplexVal;
        this.initModifyForm(res.passLenVal, res.passComplexVal);
        this.passwdFormGroup.updateValueAndValidity();
        this.loginFormGroup.removeControl('password');
        this.pwdErrorTip = this.i18n.get('common_pwdtip_label', [
          res.passLenVal,
          64,
          res.passComplexVal === 2
            ? this.i18n.get('common_pwd_complex_label')
            : '',
          2,
          this.i18n.get('common_pwdtip_five_six_label')
        ]);
        observer.next(res);
        observer.complete();
      });
    });
  }

  getUser(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const userId = this.cookieService.get('userId');
      if (isEmpty(userId)) {
        return;
      }
      this.usersApiService
        .getUsingGET2({ userId })
        .pipe(map(res => res || { rolesSet: [{ roleId: RoleType.Null }] }))
        .subscribe(res => {
          observer.next(res);
          observer.complete();
        });
    });
  }

  resetPassword() {
    this.isResetPassword =
      !!RESET_PSWD_NAVIGATE_STATUS.userId ||
      !!RESET_PSWD_NAVIGATE_STATUS.randomCode ||
      !!RESET_PSWD_NAVIGATE_STATUS.userName;

    this.resetPwdParams = cloneDeep(RESET_PSWD_NAVIGATE_STATUS);
    RESET_PSWD_NAVIGATE_STATUS.userId = '';
    RESET_PSWD_NAVIGATE_STATUS.userName = '';
    RESET_PSWD_NAVIGATE_STATUS.randomCode = '';
    this.isModifyPasswd = this.isResetPassword;
    if (this.isResetPassword) {
      this.initModifyForm(8, 4);
      this.passwdFormGroup.updateValueAndValidity();
      this.loginFormGroup.removeControl('password');
      this.pwdErrorTip = this.i18n.get('common_pwdtip_label', [
        8,
        64,
        '',
        2,
        this.i18n.get('common_pwdtip_five_six_label')
      ]);
    }
  }

  setPermission(res): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.cookieService.setRole(res.rolesSet[0].roleId);
      this.cookieService.set('role', res.rolesSet[0].roleId);
      // 发布用户信息流
      this.globalService.emitBehaviorStore({
        action: 'userInfo',
        state: res
      });

      // 发布用户操作权限
      this.globalService.setViewPermission({
        action: 'viewPermission',
        state: getAccessibleViewList(this.cookieService.role)
      });

      observer.next();
      observer.complete();
    });
  }

  goHome(lastLoginInfo?: ILastLoginInfo) {
    this.getUser()
      .pipe(switchMap(res => this.setPermission(res)))
      .subscribe(() => {
        this.router
          .navigateByUrl('/home')
          .then(() => {
            this.isLogging = false;
            this.globalService.emitStore({
              action: 'isLogin',
              state: false
            });
            this.loginSendDisable = false;
            if (
              isNil(lastLoginInfo.lastLoginIp) ||
              isNil(lastLoginInfo.lastLoginTime)
            ) {
              return;
            }
            const currentTimeZones = get(timeZones, [
              eq(this.i18n.language, LANGUAGE.CN) ? 'zh' : 'en'
            ]);
            const lastLoginZone =
              lastLoginInfo.lastLoginZone &&
              get(
                find(currentTimeZones, ['value', lastLoginInfo.lastLoginZone]),
                'label'
              );
            const timeZoneArea = !isEmpty(lastLoginZone)
              ? lastLoginZone.split(' ')[1]
              : '';
            const loginTime = this.datePipe.transform(
              lastLoginInfo.lastLoginTime,
              'yyyy-MM-dd HH:mm:ss',
              SYSTEM_TIME.timeZone
            );
            const lastLoginTimeContent = isEmpty(lastLoginZone)
              ? loginTime
              : `${loginTime} ${SYSTEM_TIME.timeZone} ${timeZoneArea}`;
            let loginInfoContent = `${this.i18n.get(
              'common_last_login_time_label'
            )}: ${lastLoginTimeContent}\n${this.i18n.get(
              'common_last_login_ip_label'
            )}: ${lastLoginInfo.lastLoginIp}`;
            this.processLoginInfoContent(loginInfoContent);
          })
          .catch(() => {
            this.isLogging = false;
            this.loginSendDisable = false;
          });
      });
  }

  processLoginInfoContent(loginInfoContent: string) {
    if (this.isCyberEngine) {
      // 安全一体机是否弹出登录提示，需要在安全策略里进行配置
      this.getSecurityPolicy().subscribe(res => {
        if (!!res?.isEnableUserDefNotes) {
          loginInfoContent += `\n${res?.userDefNodes}`;
        }
        if (!!res?.isEnableLoginNotes) {
          this.popLogInfoComponent(loginInfoContent, 15 * 1e3);
        }
      });
    } else {
      this.popLogInfoComponent(loginInfoContent);
    }
  }

  popLogInfoComponent(loginInfoContent: string, lvDuration = 5 * 1e3) {
    this.message.info(loginInfoContent, {
      lvMessageKey: 'lastLoginInfoMsg',
      lvPosition: 'topRight',
      lvShowCloseButton: true,
      lvMaxWidth: '400px',
      lvDuration
    });
  }

  sendLogin() {
    localStorage.setItem(
      'doRefresh',
      JSON.stringify(this.cookieService.get('userId'))
    );
    localStorage.removeItem('doRefresh');
  }

  setDynamicCode() {
    this.needDynamicCode = true;
    this.loginFormGroup.addControl(
      'dynamicCode',
      new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    );
    this.loginFormGroup.updateValueAndValidity();
  }

  async encryptParam(target: string, publicKey: string): Promise<string> {
    const key = await this.appUtilsService.importRsaPublicKey(
      publicKey,
      'SHA-256'
    );
    let encryptedStr = this.appUtilsService.arrayBufferToB64(
      await this.appUtilsService.rsaEncrypt(
        this.appUtilsService.utf8ToB64(target),
        key
      )
    );
    return encryptedStr;
  }

  login() {
    this.setPasswordValidator();
    this.setVerifyCodeValidator();
    if (this.updateTreeValidity(this.loginFormGroup)) {
      return;
    }

    this.isLogging = true;
    // 防止多次触发登录
    if (this.loginSendDisable || this.adfsLoginSendDisable) {
      return;
    }
    this.loginSendDisable = true;
    this.rsaService.GetPublicKey({}).subscribe(async res => {
      const authParams = this.isOceanProtect
        ? assign({}, this.loginFormGroup.value, {
            language: this.i18n.isEn ? 2 : 1
          })
        : omit(
            this.loginFormGroup.value,
            this.isCyberEngine ? ['dynamicCode'] : ['userType', 'dynamicCode']
          );
      // RSA加密
      assign(authParams, {
        userName: await this.encryptParam(
          this.loginFormGroup.value.userName,
          res.publicKey
        ),
        password: await this.encryptParam(
          this.loginFormGroup.value.password,
          res.publicKey
        )
      });
      this.authApiService
        .loginUsingPOST({
          authRequest: authParams,
          akOperationTips: false,
          akDoException: false
        })
        .subscribe({
          next: res => {
            this.loginErrorMsg = '';
            this.cookieService.set('userId', res.userId);
            this.cookieService.set('serviceProduct', res.serviceProduct);
            if (toString(res.modifyPassword) !== 'true') {
              if (+res.expireDay > 0 && +res.expireDay < 6) {
                this.message.warning(
                  this.i18n.get('common_expire_day_label', [+res.expireDay]),
                  {
                    lvMessageKey: 'lvMsg_key_expireDay',
                    lvShowCloseButton: true
                  }
                );
              }
              const lastLoginInfo: ILastLoginInfo = pick(res, [
                'lastLoginIp',
                'lastLoginTime',
                'lastLoginZone'
              ]);
              this.goHome(lastLoginInfo);
              this.sendLogin();
              return;
            }
            this.getSecurityPolicy().subscribe({
              next: () => {
                this.isLogging = false;
                this.isModifyPasswd = true;
                this.loginSendDisable = false;
                this.clearDynamicCodeValidator();
              },
              error: () => {
                this.loginSendDisable = false;
              }
            });
          },
          error: err => {
            if (err.error?.errorCode === this.DYNAMIC_CODE_ERROR_CODE) {
              this.setDynamicCode();
            }
            if (!this.needDynamicCode) {
              this.clearPasswordValidator();
            }
            this.clearVerifyCodeValidator();
            this.loginSendDisable = false;
            this.isLogging = false;
            this.initVerifyCode();
            this.loginErrorMsg = this.exceptionService.getErrorMessage(
              err.error
            );
          }
        });
    });
  }

  adfsLogin() {
    this.isAdfsLogging = true;
    // 防止多次触发登录
    if (this.loginSendDisable || this.adfsLoginSendDisable) {
      return;
    }
    this.adfsLoginSendDisable = true;
    this.adfsService.adfsLoginForward({ akDoException: false }).subscribe({
      next: (res: any) => {
        this.loginErrorMsg = '';
        window.open(res.forwardUrl, '_self');
      },
      error: err => {
        this.isAdfsLogging = false;
        this.adfsLoginSendDisable = false;
        this.loginErrorMsg = this.exceptionService.getErrorMessage(err.error);
      }
    });
  }

  modify() {
    this.setOriginalPasswordValidator();
    this.setNewPasswordValidator();
    this.setConfirmPasswordValidator();
    if (this.updateTreeValidity(this.passwdFormGroup)) {
      return;
    }
    this.isLogging = true;
    // 防止多次触发
    if (this.modifySendDisable) {
      return;
    }
    this.modifySendDisable = true;
    if (this.isResetPassword) {
      this.usersApiService
        .resetForgottenPasswordUsingPut({
          retrievePasswordDto: {
            ...this.resetPwdParams,
            confirmPassword: this.passwdFormGroup.value.confirmPassword,
            newPassword: this.passwdFormGroup.value.newPassword
          },
          akDoException: false
        })
        .subscribe({
          next: () => {
            this.loginErrorMsg = '';
            this.toLoginView();
            this.modifySendDisable = false;
          },
          error: err => {
            this.modifySendDisable = false;
            this.isLogging = false;
            this.clearOriginalPasswordValidator();
            this.clearNewPasswordValidator();
            this.clearConfirmPasswordValidator();
            this.loginErrorMsg = this.exceptionService.getErrorMessage(
              err.error
            );
          }
        });
    } else {
      const userId = this.cookieService.get('userId');
      this.usersApiService
        .updatePasswordUsingPUT({
          userId,
          passwordRequest: omit(this.passwdFormGroup.value, 'userName') as any,
          akOperationTips: false,
          akDoException: false
        })
        .subscribe({
          next: () => {
            this.loginErrorMsg = '';
            this.toLoginView();
            this.modifySendDisable = false;
          },
          error: err => {
            this.modifySendDisable = false;
            this.isLogging = false;
            this.clearOriginalPasswordValidator();
            this.clearNewPasswordValidator();
            this.clearConfirmPasswordValidator();
            this.loginErrorMsg = this.exceptionService.getErrorMessage(
              err.error
            );
          }
        });
    }
  }

  updateTreeValidity(group: FormGroup): boolean {
    Object.keys(group.controls).forEach((key: string) => {
      const abstractControl = group.controls[key];

      if (abstractControl instanceof FormGroup) {
        this.updateTreeValidity(abstractControl);
      } else {
        abstractControl.markAllAsTouched();
        abstractControl.updateValueAndValidity();
      }
    });

    return group.invalid;
  }

  cancel() {
    this.authApiService
      .logoutUsingPOST({
        akOperationTips: false,
        logoutType: LogoutType.Manual,
        clustersType: toString(DataMap.Cluster_Type.local.value),
        clustersId: toString(DataMap.Cluster_Type.local.value),
        akDoException: false
      })
      .subscribe({
        next: () => {
          this.loginErrorMsg = '';
          this.toLoginView();
        },
        error: err => {
          this.loginErrorMsg = this.exceptionService.getErrorMessage(err.error);
        }
      });
  }

  toLoginView() {
    if (!this.isResetPassword) {
      this.passwdFormGroup.removeControl('originalPassword');
    }
    this.passwdFormGroup.removeControl('newPassword');
    this.passwdFormGroup.removeControl('confirmPassword');
    this.loginFormGroup.addControl(
      'password',
      new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    );
    this.passwdFormGroup.updateValueAndValidity();
    this.loginFormGroup.updateValueAndValidity();
    this.cookieService.removeAll(this.i18n.languageKey);
    this.hasVerificationCode = false;
    this.isModifyPasswd = false;
    this.isLogging = false;
  }

  toggleLanguage() {
    this.cookieService.remove('userId');
    this.i18n.changeLanguage(
      this.i18n.language.toLowerCase() === LANGUAGE.CN
        ? LANGUAGE.EN
        : LANGUAGE.CN
    );
  }

  initVerifyCode() {
    this.hasVerificationCode =
      this.cookieService.get('needVerificationCode') === 'true';
    const verifyCodeCtrl = new FormControl('', {
      validators: [this.baseUtilService.VALID.required()],
      updateOn: 'change'
    });
    if (!this.hasVerificationCode) {
      this.loginFormGroup.removeControl('verifyCode');
    } else {
      this.loginFormGroup.addControl('verifyCode', verifyCodeCtrl);
    }
    this.loginFormGroup.updateValueAndValidity();
    setTimeout(() => {
      this.changeVerifyCode();
    });
  }

  restPassword() {
    this.router.navigateByUrl('/reset-pwd');
  }

  clearDynamicCodeValidator() {
    this.needDynamicCode = false;
    const ctrl = this.loginFormGroup.get('dynamicCode');
    if (!ctrl) {
      return;
    }
    ctrl.setValue('');
    ctrl.clearValidators();
    ctrl.updateValueAndValidity();
  }

  sendDynamicCode() {
    const params = {
      ...omit(this.loginFormGroup.value, 'dynamicCode'),
      language: this.i18n.isEn ? 2 : 1
    };
    this.authService
      .sendDynamicCode({
        authRequest: <SendDynamicCodeRequest>params,
        akDoException: false
      })
      .subscribe({
        next: () => {
          this.loginErrorMsg = '';
          this.showSendDynamicCodeBtn = false;
          this.nextSendDynamicCodeTime = 60;
          const timer = setInterval(() => {
            this.nextSendDynamicCodeTime--;
            if (this.nextSendDynamicCodeTime === 0) {
              this.showSendDynamicCodeBtn = true;
              clearInterval(timer);
            }
          }, 1e3);
          this.initVerifyCode();
        },
        error: err => {
          this.initVerifyCode();
          this.loginErrorMsg = this.exceptionService.getErrorMessage(err.error);
        }
      });
  }
}
