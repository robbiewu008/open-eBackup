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
  TemplateRef,
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
import { OptionItem } from '@iux/live';
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
  SpecialRoleIds,
  StorageUserAuthService,
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
  find,
  first,
  includes,
  isUndefined,
  last,
  map,
  size,
  trim,
  uniqBy,
  upperCase
} from 'lodash';
import { combineLatest, finalize } from 'rxjs';
import { RoleAuthTreeComponent } from '../../roles/role-detail/role-auth-tree/role-auth-tree.component';
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
  step3Invalid = true;
  nextBtnDisabled = true;

  tableConfig: TableConfig;
  tableData: TableData = {
    data: [],
    total: 0
  };
  selectionData = [];
  unitAuthData = {};
  resourceSetMap = new Map();
  roleList = [];

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

  includes = includes;
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
  isRoleAddable = false; // 用于表明能够添加更多角色
  roleType = [
    {
      value: true,
      label: this.i18n.get('system_default_role_label'),
      isLeaf: true
    },
    {
      value: false,
      label: this.i18n.get('system_none_default_role_label'),
      isLeaf: true
    }
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
  @ViewChild('roleColTpl', { static: true }) roleColTpl: TemplateRef<any>;
  @ViewChild('roleOptsTpl', { static: true }) roleOptsTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public router: Router,
    public fb: FormBuilder,
    public i18n: I18NService,
    public apiService: ApiService,
    public cdr: ChangeDetectorRef,
    public cookieService: CookieService,
    public roleApiService: RoleApiService,
    public appUtilsService: AppUtilsService,
    public usersApiService: UsersApiService,
    public baseUtilService: BaseUtilService,
    public drawModalService: DrawModalService,
    private securityApiService: SecurityApiService,
    private sanitizer: DomSanitizer,
    private dataMapService: DataMapService,
    private resourceSetService: ResourceSetApiService,
    private storageUserAuthService: StorageUserAuthService
  ) {
    this.dynamicCodeHelp = this.sanitizer.bypassSecurityTrustHtml(
      i18n.get('system_open_email_config_label')
    );
    this.openStorageGroupTip = this.sanitizer.bypassSecurityTrustHtml(
      i18n.get('system_open_storage_group_tip_label')
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
    if (this.data) {
      this.userTypeOptions.push({
        value: 'SAML',
        label: 'system_saml_service_user_label',
        isLeaf: true
      });
    }
    this.initConfig();
    this.initForm();
    this.getRoleList();
    if (!this.data) {
      this.tableData.data.push({
        isDefaultRole: true,
        disabled: true
      });
      this.getSecuritPolicy();
    }
  }

  getRoleList() {
    this.appUtilsService.getResourceByRecursion(
      null,
      params => this.roleApiService.getUsingGET(params),
      resource => {
        this.roleOptions = resource.map(item => {
          item.originRoleName = item.roleName;
          item.isDefaultRole = false;
          if (
            this.formGroup.get('userType').value !==
              DataMap.loginUserType.local.value &&
            [
              DataMap.defaultRoleName.rdAdmin.value,
              DataMap.defaultRoleName.drAdmin.value
            ].includes(item.roleName)
          ) {
            item.disabled = true;
          }
          if (item.is_default) {
            item.roleName = this.dataMapService.getLabel(
              'defaultRoleName',
              item.roleName
            );
            item.roleDescription = this.dataMapService.getLabel(
              'defaultRoleDescription',
              item.roleDescription
            );
          } else {
            // 升级场景会有自动创造的角色，使用词条做描述
            item.roleDescription = this.i18n.get(item.roleDescription);
          }
          return assign(item, {
            value: item.roleId,
            label: item.roleName,
            isLeaf: true
          });
        });
        if (!!this.data) {
          this.getData();
        }
      }
    );
  }

  defaultRoleChange(e) {
    // 这里要把原来的默认角色判断清掉
    if (this.formGroup.get('roleId').value) {
      const lastDefaultRole = find(this.roleOptions, {
        roleId: this.formGroup.get('roleId').value
      });
      assign(lastDefaultRole, {
        isDefaultRole: false,
        disabled: false
      });
    }
    const tmpRole = assign(find(this.roleOptions, { roleId: e }), {
      isDefaultRole: true,
      disabled: true
    });
    this.roleOptions = [...this.roleOptions];
    this.isRoleAddable = !SpecialRoleIds.includes(tmpRole.roleId);
    this.formGroup.get('roleId').setValue(tmpRole.roleId);
    if (!this.data) {
      this.tableData.data[0] = tmpRole;
      // 默认角色可以选择非默认角色里面的角色，所以选择时要清掉下面重复的
      this.tableData.data = uniqBy(this.tableData.data, 'roleId');
    } else {
      // 默认角色放第一个
      const lastIndex = this.tableData.data.findIndex(
        item => item.roleId === e
      );
      [this.tableData.data[0], this.tableData.data[lastIndex]] = [
        tmpRole,
        this.tableData.data[0]
      ];
    }
    // 切换为其他内置角色则清空其他角色
    if (!this.isRoleAddable) {
      this.tableData = {
        data: [cloneDeep(tmpRole)],
        total: 1
      };
    } else {
      this.tableData = {
        data: cloneDeep(this.tableData.data),
        total: size(this.tableData.data)
      };
    }

    this.roleList = this.tableData.data;
    this.updateResourceSetMapRole(this.tableData.data);
    this.nextBtnDisabledCheck();
  }

  clearDefaultRole() {
    // 只有在是灾备和远端有这个场景，所以不用考虑其他参数，重新初始化就行了
    this.tableData.data = [
      {
        isDefaultRole: true,
        disabled: true
      }
    ];
    this.formGroup.get('roleId').setValue('');
  }

  getData() {
    this.formGroup.patchValue(this.data);
    this.formGroup.get('roleId').setValue(this.data.rolesSet[0]?.roleId);
    this.isRoleAddable = !SpecialRoleIds.includes(
      this.data.rolesSet[0]?.roleId
    );
    this.getRoleData();
    this.getResourceData();
    if (this.isRoleAddable) {
      this.getStorageData();
    }
  }

  private getRoleData() {
    let pageNo = 0;
    const pageSize = CommonConsts.PAGE_SIZE_MAX;
    let roleList = [];
    let total = 1;
    while (pageSize * pageNo < total) {
      // 注意这里有超过200个角色的话就不能只放执行一次的东西在这里
      this.roleApiService
        .getUsingGET({
          pageNo: pageNo,
          pageSize: pageSize,
          conditions: JSON.stringify({
            userId: this.data.userId
          })
        })
        .subscribe((res: any) => {
          this.roleDataProcess(res);
          const defaultRoleData = find(res.records, {
            roleId: this.formGroup.get('roleId').value
          });
          if (defaultRoleData) {
            defaultRoleData['disabled'] = true;
            defaultRoleData.isDefaultRole = true;
          }
          total = res.totalCount;
          roleList = roleList.concat(res.records);
          this.tableData = {
            data: roleList,
            total: roleList.length
          };
          if (defaultRoleData) {
            this.defaultRoleChange(this.formGroup.get('roleId').value);
          }
          this.updateResourceSetMapRole(roleList);
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
      item.isDefaultRole = false;
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

  getStorageData() {
    this.resourceSetMap = new Map();
    combineLatest(this.callStorageData(2), this.callStorageData(1)).subscribe(
      res => {
        this.resourceSetMap.set(
          DataMap.storagePoolBackupStorageType.group.value,
          res[0].records.map(item => {
            return assign(item, {
              uuid: item.storageId
            });
          })
        );
        this.resourceSetMap.set(
          DataMap.storagePoolBackupStorageType.unit.value,
          res[1].records.map(item => {
            return assign(item, {
              id: item.storageId
            });
          })
        );
      }
    );
  }

  callStorageData(type) {
    return this.storageUserAuthService.getStorageUserAuthRelationsByUserId({
      userId: this.data.userId,
      authType: type
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

  nextBtnDisabledCheck(step3Invalid?) {
    if (step3Invalid !== undefined) {
      this.step3Invalid = step3Invalid;
    }
    switch (this.stepIndex) {
      case 0:
        this.nextBtnDisabled = this.formGroup.status !== 'VALID';
        break;
      case 1:
        this.nextBtnDisabled = false;
        break;
      default:
        this.nextBtnDisabled = this.step3Invalid;
        break;
    }
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'roleName',
        name: this.i18n.get('common_name_label'),
        width: 280,
        cellRender: this.roleColTpl
      },
      {
        key: 'isDefaultRole',
        name: this.i18n.get('common_type_label'),
        cellRender: {
          type: 'status',
          config: this.roleType
        }
      },
      {
        key: 'userNum',
        name: this.i18n.get('system_associated_users_num_label')
      },
      {
        key: 'roleDescription',
        name: this.i18n.get('common_desc_label')
      },
      {
        key: 'operation',
        width: 130,
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        cellRender: this.roleOptsTpl
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'roleId',
        columns: cols,
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
      } else if (
        includes(
          [
            DataMap.loginUserType.ldap.value,
            DataMap.loginUserType.ldapGroup.value
          ],
          res
        )
      ) {
        this.formGroup.get('userPassword').clearValidators();
        this.formGroup.get('confirmPassword').clearValidators();
        if (
          [
            DataMap.defaultRoleName.rdAdmin.value,
            DataMap.defaultRoleName.drAdmin.value
          ].includes(this.tableData.data[0]?.originRoleName)
        ) {
          this.clearDefaultRole();
        }
        this.updateRoleList(this.tableData.data);
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
        if (
          [
            DataMap.defaultRoleName.rdAdmin.value,
            DataMap.defaultRoleName.drAdmin.value
          ].includes(this.tableData.data[0]?.originRoleName)
        ) {
          this.clearDefaultRole();
        }
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
        this.roleOptions = map(this.roleOptions, item => {
          return assign(item, {
            disabled: includes(this.notLoginRoleTypes, item.roleId)
          });
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
        this.roleOptions = map(this.roleOptions, item => {
          return assign(item, {
            disabled: includes(this.notLoginRoleTypes, item.roleId)
          });
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
        this.roleOptions = this.roleOptions.map(item => {
          return assign(item, {
            disabled: false
          });
        });
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
      // saml这部分只有在修改用户的时候才会有，所以不会考虑其他情况
      if (res === DataMap.loginUserType.saml.value) {
        this.formGroup
          .get('sessionLimit')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 100)
          ]);
        this.sessionErrorTip.invalidRang = this.i18n.get(
          'common_valid_rang_label',
          [1, 100]
        );
        this.formGroup.get('sessionLimit').updateValueAndValidity();
      }
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
            this.baseUtilService.VALID.rangeValue(
              1,
              this.formGroup.get('userType').value ===
                DataMap.loginUserType.saml.value
                ? 100
                : 8
            )
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
        data: cloneDeep(this.tableData.data)
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
      data: roleList.map(item => {
        return assign(item, {
          isDefaultRole: item?.isDefaultRole || false
        });
      }),
      total: roleList.length
    };
    // 根据roleId的差异，更新resourceSetMap
    this.updateResourceSetMapRole(roleList);
  }

  private updateResourceSetMapRole(roleList: any) {
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
  }

  deleteRole(item?) {
    this.updateRoleList(difference(this.roleList, item ?? this.selectionData));
    this.selectionData = [];
    this.dataTable.setSelections([]);
  }

  getRoleDetail(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'view-role-detail',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: data.roleName,
        lvContent: RoleAuthTreeComponent,
        lvComponentParams: {
          data
        },
        lvFooter: [
          {
            id: 'close',
            label: this.i18n.get('common_close_label'),
            onClick: modal => {
              modal.close();
            }
          }
        ]
      })
    );
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

    if (this.isRoleAddable) {
      userParams.storageAuthArray = [
        {
          storageIds: map(
            this.resourceSetMap.get(
              DataMap.storagePoolBackupStorageType.group.value
            ),
            'uuid'
          ),
          authType: 2
        },
        {
          storageIds: map(
            this.resourceSetMap.get(
              DataMap.storagePoolBackupStorageType.unit.value
            ),
            'id'
          ),
          authType: 1
        }
      ];
    }

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
          this.openPage.emit();
        });
    }
  }

  back() {
    this.openPage.emit();
  }
}
