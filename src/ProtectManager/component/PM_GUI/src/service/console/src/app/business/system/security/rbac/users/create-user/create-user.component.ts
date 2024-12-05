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
import {
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  ValidatorFn
} from '@angular/forms';
import { DomSanitizer } from '@angular/platform-browser';
import { Router } from '@angular/router';
import { MessageboxService, OptionItem } from '@iux/live';
import {
  ApiService,
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  DefaultRoles,
  I18NService,
  MODAL_COMMON,
  ResourceSetApiService,
  RoleApiService,
  SecurityApiService,
  UsersApiService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  defer,
  difference,
  each,
  filter,
  find,
  first,
  includes,
  isUndefined,
  last,
  map,
  trim,
  upperCase
} from 'lodash';
import { finalize } from 'rxjs';
import { AddRoleComponent } from './add-role/add-role.component';

@Component({
  selector: 'aui-create-user',
  templateUrl: './create-user.component.html',
  styleUrls: ['./create-user.component.less']
})
export class CreateUserComponent implements OnInit {
  @Input() openPage;
  @Input() data;
  stepIndex = 0;
  step2Invalid = true;
  nextBtnDisabled = true;

  tableConfig: TableConfig;
  tableData: TableData = {
    data: [],
    total: 0
  };
  selectionData = [];
  resourceSetMap = new Map();
  roleList = [];
  specialDefaultRoleIdList = [
    DefaultRoles.rdAdmin.roleId,
    DefaultRoles.drAdmin.roleId,
    DefaultRoles.audit.roleId,
    DefaultRoles.sysAdmin.roleId
  ];

  formGroup;
  rolesOptsConfig: ProButton[];
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
          [
            DataMap.loginUserType.saml.value,
            DataMap.loginUserType.hcs.value,
            DataMap.loginUserType.dme.value
          ],
          item.value
        )
    );
  methodTypeOptions = this.dataMapService.toArray('loginMethod').map(item => {
    item.isLeaf = true;
    return item;
  });
  notLoginRoleTypes = [
    DefaultRoles.rdAdmin.roleId,
    DefaultRoles.drAdmin.roleId
  ];

  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );

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
  openStorageGroupTip;

  @ViewChild('dpAdminTipTpl', { static: true }) dpAdminTipTpl;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    public router: Router,
    public fb: FormBuilder,
    public cdr: ChangeDetectorRef,
    public usersApiService: UsersApiService,
    public roleApiService: RoleApiService,
    public apiService: ApiService,
    public baseUtilService: BaseUtilService,
    public cookieService: CookieService,
    private messageBox: MessageboxService,
    private securityApiService: SecurityApiService,
    private sanitizer: DomSanitizer,
    private dataMapService: DataMapService,
    private resourceSetService: ResourceSetApiService,
    public appUtilsService: AppUtilsService
  ) {
    this.dynamicCodeHelp = this.sanitizer.bypassSecurityTrustHtml(
      i18n.get('system_open_email_config_label')
    );
    this.openStorageGroupTip = this.sanitizer.bypassSecurityTrustHtml(
      i18n.get('system_open_storage_group_tip_label')
    );
  }

  openStorageGroupLink() {
    const openStorageGroup = document.querySelector('#open-storage-group');
    if (!openStorageGroup) {
      return;
    }
    openStorageGroup.addEventListener('click', () => {
      this.router.navigateByUrl('/system/infrastructure/nas-backup-storage');
    });
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
    this.initConfig();
    this.initForm();
    if (this.data) {
      this.getData();
    } else {
      this.getSecuritPolicy();
    }
  }

  getData() {
    this.formGroup.patchValue(this.data);
    this.formGroup.get('roleId').setValue(this.data.rolesSet[0]?.roleId);
    this.getRoleData();
    this.getResourceData();
  }

  private getRoleData() {
    let pageNo = 0;
    const pageSize = CommonConsts.PAGE_SIZE_MAX;
    let roleList = [];
    let total = 1;
    while (pageSize * pageNo < total) {
      this.roleApiService
        .getUsingGET({
          pageNo: pageNo,
          pageSize: pageSize,
          conditions: JSON.stringify({
            userId: this.data.userId
          })
        })
        .subscribe(res => {
          this.roleDataProcess(res);
          const defaultRoleData = find(res.records, {
            roleId: this.formGroup.get('roleId').value
          });
          if (defaultRoleData) {
            defaultRoleData['disabled'] = true;
          }
          total = res.totalCount;
          roleList = roleList.concat(res.records);
          this.updateRoleList(roleList);
        });
      pageNo++;
    }
  }

  private roleDataProcess(res) {
    each(res.records, item => {
      if (item['is_default']) {
        item.roleName = this.dataMapService.getLabel(
          'defaultRoleName',
          item.roleName
        );
        item['roleDescription'] = this.dataMapService.getLabel(
          'defaultRoleDescription',
          item['roleDescription']
        );
      }
    });
  }

  private getResourceData() {
    this.resourceSetService
      .queryResourceSetByUserId({
        userId: this.data.userId
      })
      .subscribe((res: any) => {
        const notDefaultRes = res.filter(item => !item.isDefault);
        notDefaultRes.forEach(item => {
          const resouceData = {
            uuid: item.resourceSetId,
            name: item.resourceSetName,
            description: item.resourceSetDescription
          };
          if (this.resourceSetMap.get(item.roleId)) {
            this.resourceSetMap.get(item.roleId).push(resouceData);
          } else {
            this.resourceSetMap.set(item.roleId, [resouceData]);
          }
        });
      });
  }

  previous() {
    this.stepIndex--;
    this.nextBtnDisabledCheck();
  }

  next() {
    this.stepIndex++;
    this.nextBtnDisabledCheck();
  }

  nextBtnDisabledCheck(step2Invalid?) {
    if (step2Invalid !== undefined) {
      this.step2Invalid = step2Invalid;
    }
    this.nextBtnDisabled =
      this.stepIndex === 0
        ? this.formGroup.status !== 'VALID'
        : this.step2Invalid;
  }

  initConfig() {
    this.rolesOptsConfig = [
      {
        id: 'addRole',
        type: 'primary',
        label: this.i18n.get('common_add_label'),
        onClick: () => {
          this.addRole();
        }
      },
      {
        id: 'deleteRole',
        label: this.i18n.get('common_delete_label'),
        onClick: () => {
          this.deleteRole();
        },
        disableCheck: data => !data.length
      }
    ];
    const cols: TableCols[] = [
      {
        key: 'roleName',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'userNum',
        name: this.i18n.get('system_associated_users_num_label')
      },
      {
        key: 'roleDescription',
        name: this.i18n.get('common_desc_label')
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'roleId',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: selection => {
          this.selectionData = selection;
        }
      },
      pagination: null
    };
  }

  initForm() {
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
        validators: [this.validUserPasswordIsSame()],
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
    if (this.data) {
      this.formGroup.get('userPassword').clearValidators();
      this.formGroup.get('confirmPassword').clearValidators();
      this.formGroup.get('userPassword').updateValueAndValidity();
      this.formGroup.get('confirmPassword').updateValueAndValidity();
    }
    this.listenForm();
    this.formGroup.statusChanges.subscribe(() => {
      this.nextBtnDisabledCheck();
    });
  }

  listenForm() {
    this.formGroup.get('userType').valueChanges.subscribe(res => {
      if (res === DataMap.loginUserType.local.value) {
        if (!this.data) {
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
        }
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
        // 其他类型的用户不选远端和灾备角色
        this.tableData.data = this.tableData.data.filter(
          item =>
            ![
              DataMap.defaultRoleName.rdAdmin.value,
              DataMap.defaultRoleName.drAdmin.value
            ].includes(item?.originRoleName)
        );
        this.updateRoleList(this.tableData.data);
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
          return !includes(this.notLoginRoleTypes, item.roleId);
        });
        if (includes(this.notLoginRoleTypes, this.formGroup.value.roleId)) {
          this.formGroup.get('roleId').setValue('', { emitEvent: false });
        }
        this.formGroup
          .get('userName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validLdapName(),
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
          return !includes(this.notLoginRoleTypes, item.roleId);
        });
        if (includes(this.notLoginRoleTypes, this.formGroup.value.roleId)) {
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

    this.formGroup.get('sessionControl').valueChanges.subscribe(res => {
      if (res) {
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
    });
  }

  validLdapName(): ValidatorFn {
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
    this.securityApiService
      .getUsingGET1({})
      .pipe(
        finalize(() => {
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
        })
      )
      .subscribe(res => {
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

  addRole() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('system_add_role_label'),
      lvContent: AddRoleComponent,
      lvOkDisabled: false,
      lvWidth: MODAL_COMMON.xLargeWidth,
      lvComponentParams: {
        userType: this.formGroup.get('userType').value,
        selectionData: cloneDeep(this.tableData.data),
        data: this.data
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddRoleComponent;
        const modalIns = modal.getInstance();
        content.modalInValid.subscribe(res => {
          modalIns.lvOkDisabled = res;
        });
      },
      lvOk: modal => {
        const content = modal.getContentComponent() as AddRoleComponent;
        content.onOK().subscribe(res => {
          this.updateRoleList([...res]);
        });
      }
    });
  }

  updateRoleList(roleList) {
    this.roleList = roleList;
    this.tableData = {
      data: roleList,
      total: roleList.length
    };
    // 根据roleId的差异，更新resourceSetMap
    const newRoleIdSet = new Set(map(roleList, 'roleId'));
    each(this.resourceSetMap.keys(), roleId => {
      if (!newRoleIdSet.has(roleId)) {
        this.resourceSetMap.delete(roleId);
      }
    });
    roleList.forEach(item => {
      if (!this.resourceSetMap.has(item.roleId)) {
        this.resourceSetMap.set(item.roleId, []);
      }
    });
    this.updateRoleOptions();
  }

  updateRoleOptions() {
    const roleSet = [];
    each(this.tableData.data, role => {
      roleSet.push({
        isLeaf: true,
        key: role.roleId,
        value: role.roleId,
        label: role.roleName
      });
    });
    this.roleOptions = roleSet;
    this.cacheRoleOptions = cloneDeep(roleSet);
    if (!find(roleSet, { value: this.formGroup.get('roleId').value })) {
      this.formGroup.get('roleId').setValue('');
    }
  }

  deleteRole() {
    this.updateRoleList(difference(this.roleList, this.selectionData));
    this.selectionData = [];
    this.dataTable.setSelections([]);
  }

  onOK() {
    const userParams = {
      ...this.formGroup.value,
      resourceSetAuthorizationSets: this.roleList.map(item => {
        return {
          roleId: item.roleId,
          resourceSetIds: map(this.resourceSetMap.get(item.roleId), 'uuid')
        };
      })
    };

    if (userParams.sessionControl) {
      userParams.sessionLimit = parseInt(userParams.sessionLimit);
    } else {
      delete userParams.sessionLimit;
    }

    if (!this.data) {
      assign(userParams, { rolesIdsSet: [userParams.roleId] });
    }
    delete userParams['roleId'];

    if (this.data) {
      this.usersApiService
        .updateUsingPUT({
          userId: this.data.userId,
          userRequest: userParams
        })
        .subscribe(() => {
          this.openPage.emit();
        });
    } else {
      this.usersApiService
        .createUsingPOST({
          user: userParams
        })
        .subscribe(() => {
          this.dpAdminTipBox(
            !this.specialDefaultRoleIdList.includes(
              this.formGroup.get('roleId').value
            )
          );
          this.openPage.emit();
        });
    }
  }

  dpAdminTipBox(isDPAdmin) {
    // 数据保护管理员或自定义用户需要展示弹窗
    if (!isDPAdmin) {
      return;
    }
    this.messageBox.info({
      lvContent: this.dpAdminTipTpl
    });
    defer(() => this.openStorageGroupLink());
  }

  back() {
    this.openPage.emit();
  }
}
