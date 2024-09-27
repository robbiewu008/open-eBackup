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
import { AfterViewInit, Component, OnInit, Renderer2 } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { DomSanitizer } from '@angular/platform-browser';
import { Router } from '@angular/router';
import { OptionItem } from '@iux/live';
import {
  BaseUtilService,
  CookieService,
  I18NService,
  UserRoleType,
  UserRoleI18nMap,
  CommonConsts,
  DataMapService,
  DataMap
} from 'app/shared';
import {
  RoleApiService,
  UsersApiService,
  SecurityApiService
} from 'app/shared/api/services';
import {
  assign,
  extend,
  forEach,
  isUndefined,
  filter,
  trim,
  upperCase,
  includes,
  defer,
  cloneDeep,
  last,
  first
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'cdm-createuser',
  templateUrl: './createuser.component.html',
  styleUrls: ['./createuser.component.less']
})
export class CreateuserComponent implements OnInit {
  formGroup: FormGroup;
  roleOptions: OptionItem[];
  cacheRoleOptions: OptionItem[];
  passLenVal = 0;
  passComplexVal = 0;
  maxLenVal = 64;
  pwdComplexTipLabel = this.i18n.get('common_pwdtip_label');
  sessionErrorTip = assign(
    { ...this.baseUtilService.integerErrorTip },
    { ...this.baseUtilService.rangeErrorTip },
    {
      invalidRang: this.i18n.get('common_valid_rang_label', [1, 8])
    }
  );
  userNameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_username_label'),
    nameExistError: this.i18n.get('system_username_exist_label'),
    invalidMaxLength: this.i18n.get('common_valid_length_rang_label', [5, 64]),
    invalidMinLength: this.i18n.get('common_valid_length_rang_label', [5, 64]),
    invalidAdfsName: this.i18n.get('common_adfs_name_error_label')
  };
  nameToolTips = this.i18n.get('common_valid_username_label');

  dataMap = DataMap;
  userTypeOptions = this.dataMapService
    .toArray('loginUserType')
    .map(item => {
      item.isLeaf = true;
      return item;
    })
    .filter(
      item =>
        !includes(
          [DataMap.loginUserType.saml.value, DataMap.loginUserType.hcs.value],
          item.value
        )
    );
  methodTypeOptions = this.dataMapService.toArray('loginMethod').map(item => {
    item.isLeaf = true;
    return item;
  });

  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  isCyberengine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

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

  dynamicCodeHelp;

  constructor(
    public router: Router,
    public fb: FormBuilder,
    public i18n: I18NService,
    public usersApiService: UsersApiService,
    public roleApiService: RoleApiService,
    public baseUtilService: BaseUtilService,
    public cookieService: CookieService,
    private securityApiService: SecurityApiService,
    private dataMapService: DataMapService,
    private sanitizer: DomSanitizer,
    public appUtilsService?: AppUtilsService
  ) {
    this.dynamicCodeHelp = this.sanitizer.bypassSecurityTrustHtml(
      i18n.get('system_open_email_config_label')
    );
  }

  openEmailLink() {
    const openEmail = document.querySelector('#open-email-settings');
    if (!openEmail) {
      return;
    }
    openEmail.addEventListener('click', () => {
      this.router.navigateByUrl('/system/settings/alarm-notify');
    });
  }

  ngOnInit() {
    this.initForm();
    this.initRoles();
    this.getSecuritPolicy();
  }

  initForm() {
    // 安全一体机新增ldap用户类型
    if (this.isCyberengine) {
      this.userTypeOptions = filter(this.userTypeOptions, item =>
        includes(
          [
            DataMap.loginUserType.local.value,
            DataMap.loginUserType.ldap.value,
            DataMap.loginUserType.ldapGroup.value
          ],
          item.value
        )
      );
    }
    this.formGroup = this.fb.group({
      userType: new FormControl(DataMap.loginUserType.local.value),
      loginType: new FormControl(DataMap.loginMethod.password.value),
      dynamicCodeEmail: new FormControl(''),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.nameBegin),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.nameCombination),
          this.baseUtilService.VALID.maxLength(64),
          this.baseUtilService.VALID.minLength(5)
        ],
        updateOn: 'change'
      }),
      userPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.password(
            this.passLenVal,
            this.passComplexVal,
            this.maxLenVal
          ),
          this.validUserNamePwd(),
          this.validConfirmPwdIsSame()
        ],
        updateOn: 'change'
      }),
      confirmPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.password(
            this.passLenVal,
            this.passComplexVal,
            this.maxLenVal
          ),
          this.validUserNamePwd(),
          this.validUserPasswordIsSame()
        ],
        updateOn: 'change'
      }),
      roleId: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      description: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(255)],
        updateOn: 'change'
      }),
      sessionControl: [true],
      sessionLimit: new FormControl(5, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 8)
        ],
        updateOn: 'change'
      }),
      neverExpire: new FormControl(false)
    });
    this.listenForm();
  }

  listenForm() {
    this.formGroup.get('userType').valueChanges.subscribe(res => {
      if (res === DataMap.loginUserType.local.value) {
        this.formGroup
          .get('userPassword')
          .setValidators([
            this.baseUtilService.VALID.password(
              this.passLenVal,
              this.passComplexVal,
              this.maxLenVal
            ),
            this.validUserNamePwd(),
            this.validConfirmPwdIsSame()
          ]);
        this.formGroup
          .get('confirmPassword')
          .setValidators([
            this.baseUtilService.VALID.password(
              this.passLenVal,
              this.passComplexVal,
              this.maxLenVal
            ),
            this.validUserNamePwd(),
            this.validUserPasswordIsSame()
          ]);
        if (
          this.formGroup.value.loginType === DataMap.loginMethod.password.value
        ) {
          this.formGroup.get('dynamicCodeEmail').clearValidators();
        } else {
          this.formGroup
            .get('dynamicCodeEmail')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.validEmail()
            ]);
        }
      } else {
        this.formGroup.get('userPassword').clearValidators();
        this.formGroup.get('confirmPassword').clearValidators();
        this.formGroup.get('dynamicCodeEmail').clearValidators();
      }
      this.formGroup.get('userPassword').updateValueAndValidity();
      this.formGroup.get('confirmPassword').updateValueAndValidity();
      this.formGroup.get('dynamicCodeEmail').updateValueAndValidity();
      if (
        includes(
          [
            DataMap.loginUserType.ldap.value,
            DataMap.loginUserType.ldapGroup.value
          ],
          res
        )
      ) {
        this.roleOptions = filter(this.cacheRoleOptions, item => {
          return !includes([6, 7], item.roleId);
        });
        if (includes([6, 7], this.formGroup.value.roleId)) {
          this.formGroup.get('roleId').setValue('', { emitEvent: false });
        }
        this.formGroup
          .get('userName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validLadpName(),
            this.baseUtilService.VALID.maxLength(64),
            this.baseUtilService.VALID.minLength(1)
          ]);
        this.nameToolTips = this.i18n.get('system_ldap_name_valid_label');
        assign(this.userNameErrorTip, {
          invalidName: this.i18n.get('system_ldap_name_valid_label'),
          invalidMaxLength: this.i18n.get('common_valid_length_rang_label', [
            1,
            64
          ]),
          invalidMinLength: this.i18n.get('common_valid_length_rang_label', [
            1,
            64
          ])
        });
      } else if (res === DataMap.loginUserType.adfs.value) {
        this.roleOptions = filter(this.cacheRoleOptions, item => {
          return !includes([6, 7], item.roleId);
        });
        if (includes([6, 7], this.formGroup.value.roleId)) {
          this.formGroup.get('roleId').setValue('', { emitEvent: false });
        }
        this.nameToolTips = this.i18n.get('common_valid_adfs_username_label');
        this.formGroup
          .get('userName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validAdfsName(),
            this.baseUtilService.VALID.maxLength(254)
          ]);
        this.userNameErrorTip.invalidMaxLength = this.i18n.get(
          'common_valid_length_rang_label',
          [1, 254]
        );
      } else {
        this.roleOptions = cloneDeep(this.cacheRoleOptions);
        this.formGroup
          .get('userName')
          .setValidators([
            this.baseUtilService.VALID.name(CommonConsts.REGEX.nameBegin),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.nameCombination),
            this.baseUtilService.VALID.maxLength(64),
            this.baseUtilService.VALID.minLength(5)
          ]);
        this.nameToolTips = this.i18n.get('common_valid_username_label');
        assign(this.userNameErrorTip, {
          invalidName: this.i18n.get('common_valid_username_label'),
          invalidMaxLength: this.i18n.get('common_valid_length_rang_label', [
            5,
            64
          ]),
          invalidMinLength: this.i18n.get('common_valid_length_rang_label', [
            5,
            64
          ])
        });
      }
      this.formGroup.get('userName').updateValueAndValidity();
    });

    this.formGroup.get('loginType').valueChanges.subscribe(res => {
      if (res === DataMap.loginMethod.email.value) {
        defer(() => this.openEmailLink());
        this.formGroup
          .get('dynamicCodeEmail')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validEmail()
          ]);
      } else {
        this.formGroup.get('dynamicCodeEmail').clearValidators();
      }
      this.formGroup.get('dynamicCodeEmail').updateValueAndValidity();
    });

    this.formGroup.get('userName').valueChanges.subscribe(() => {
      defer(() => {
        if (this.formGroup.value.userPassword) {
          this.formGroup.get('userPassword').updateValueAndValidity();
        }
      });
    });
  }

  validLadpName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      if (last(control.value) === ' ' || first(control.value) === ' ') {
        return { invalidName: { value: control.value } };
      }
      return null;
    };
  }

  validAdfsName() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }

      if (!CommonConsts.REGEX.email.test(control.value)) {
        return { invalidAdfsName: { value: control.value } };
      }
      return null;
    };
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

  getSecuritPolicy() {
    this.securityApiService.getUsingGET1({}).subscribe(res => {
      this.passLenVal = res.passLenVal;
      this.passComplexVal = res.passComplexVal;
      this.pwdComplexTipLabel = this.i18n.get('common_pwdtip_label', [
        this.passLenVal,
        64,
        this.passComplexVal === 2
          ? this.i18n.get('common_pwd_complex_label')
          : '',
        2,
        this.i18n.get('common_pwdtip_five_six_label')
      ]);
    });
  }

  validUserNamePwd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || !this.formGroup.value?.userName) {
        return null;
      }

      const reverseName = this.formGroup.value.userName.split('').reverse();
      const _reverseName = reverseName.join('');
      if (
        control.value === this.formGroup.value.userName ||
        control.value === _reverseName ||
        upperCase(control.value).indexOf(
          upperCase(this.formGroup.value.userName)
        ) !== -1
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

  asyncValidUserName() {
    return (
      control: AbstractControl
    ): Promise<{ [key: string]: any } | null> => {
      return new Promise(resolve => {
        this.usersApiService
          .getAllUserUsingGET({
            startIndex: 1,
            pageSize: 70,
            akLoading: false
          })
          .subscribe(res => {
            const sameName = filter(res.userList, item => {
              return item.userName === control.value;
            });
            if (sameName.length > 0) {
              resolve({ nameExistError: { value: control.value } });
            }
            resolve(null);
          });
      });
    };
  }

  initRoles() {
    this.roleApiService.getUsingGET({}).subscribe(roles => {
      const roleSet = [];
      forEach(
        filter(roles.records, item => {
          const deployType = this.i18n.get('deploy_type');
          if (
            (['d3', 'd4', 'cloudbackup'].includes(deployType) &&
              ['Role_DP_Admin', 'Role_DR_Admin'].includes(item.roleName)) ||
            (this.appUtilsService.isDistributed &&
              ['Role_RD_Admin', 'Role_DR_Admin'].includes(item.roleName)) ||
            ([
              DataMap.Deploy_Type.decouple.value,
              DataMap.Deploy_Type.openServer.value
            ].includes(deployType) &&
              item.roleName === 'Role_DR_Admin')
          ) {
            return false;
          }
          return this.cookieService.isCloudBackup
            ? item.roleName !== 'Role_RD_Admin'
            : true;
        }),
        role => {
          if (role.roleId && role.roleId !== 4) {
            roleSet.push({
              isLeaf: true,
              key: role.roleId,
              roleId: role.roleId,
              value: UserRoleType[role.roleId + ''],
              label: this.i18n.get(
                UserRoleI18nMap[UserRoleType[role.roleId + '']]
              )
            });
          }
        }
      );
      this.roleOptions = roleSet;
      this.cacheRoleOptions = cloneDeep(roleSet);
    });
  }

  sessionControlChange(sessionControl) {
    if (sessionControl) {
      this.formGroup
        .get('sessionLimit')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 8)
        ]);
    } else {
      this.formGroup.get('sessionLimit').clearValidators();
    }
    this.formGroup.get('sessionLimit').setValue('');
    this.formGroup.get('sessionLimit').updateValueAndValidity();
  }

  sessionLimitBlur() {
    if (
      !isNaN(+this.formGroup.value.sessionLimit) &&
      trim(this.formGroup.value.sessionLimit) !== ''
    ) {
      this.formGroup.patchValue({
        sessionLimit: +this.formGroup.value.sessionLimit
      });
    }
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }

      if (this.formGroup.value.roleId === 6) {
        this.formGroup.get('sessionControl').setValue(false);
      }

      if (this.formGroup.value.roleId !== 7) {
        this.formGroup.get('neverExpire').setValue(false);
      }

      const user = extend(cloneDeep(this.formGroup.value), {
        rolesIdsSet: [this.formGroup.value.roleId],
        resourceSetAuthorizationSets: [
          { roleId: this.formGroup.value.roleId, resourceSetIds: [] }
        ]
      });
      if (!user.sessionControl) {
        delete user.sessionLimit;
      }
      if (!this.isOceanProtect && !this.isCyberengine) {
        delete user.userType;
        delete user.loginType;
        delete user.dynamicCodeEmail;
      }
      if (this.isCyberengine) {
        delete user.loginType;
        delete user.dynamicCodeEmail;
      }
      this.usersApiService.createUsingPOST({ user }).subscribe({
        next: () => {
          observer.next(this.formGroup.value.roleId === 2);
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
