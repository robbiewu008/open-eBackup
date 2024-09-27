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
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  LDAPAPIService,
  WarningMessageService
} from 'app/shared';
import { LdapConfigRequest } from 'app/shared/api/models/ldap-config-request';
import {
  assign,
  defer,
  each,
  filter,
  includes,
  isEmpty,
  size,
  toUpper,
  trim
} from 'lodash';

@Component({
  selector: 'aui-ldap-config',
  templateUrl: './ldap-config.component.html',
  styleUrls: ['./ldap-config.component.less']
})
export class LdapConfigComponent implements OnInit {
  viewSettingFlag = true;
  ldapServiceStatus = false;
  formGroup: FormGroup;
  dataMap = DataMap;
  ctrls;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  serviceTypeOptions = this.dataMapService.toArray('ldapService').map(item => {
    item['isLeaf'] = true;
    return item;
  });
  protocolOptions = this.dataMapService.toArray('ldapProtocol').map(item => {
    item['isLeaf'] = true;
    return item;
  });
  domainName;

  serviceItems = [];
  userItems = [];
  userGroupItems = [];
  userGroupOn = true;
  ladapCheckCn = false;
  isDomain = false;

  hasTest = false;

  hasConfig = false;

  dnErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidDn: this.i18n.get('system_dn_error_label'),
    invalidSpecialKey: this.i18n.get('system_ldap_special_key_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [63])
  };
  ipErrorTip = {
    ...this.baseUtilService.ipErrorTip
  };
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  domainErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_invalid_input_label')
  };
  directoryErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidDn: this.i18n.get('system_ldap_directory_error_label'),
    invalidSpecialKey: this.i18n.get('system_ldap_special_key_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };
  nameAttributeErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidSpecialKey: this.i18n.get('system_ldap_special_key_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [63])
  };
  projectTypeErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidSpecialKey: this.i18n.get('system_ldap_special_key_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [63])
  };
  memberAttributeErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidSpecialKey: this.i18n.get('system_ldap_special_key_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [63])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private ldapApiService: LDAPAPIService,
    private dataMapService: DataMapService,
    private messageService: MessageService,
    private baseUtilService: BaseUtilService,
    private warningMessageService: WarningMessageService
  ) {}

  onChange() {
    this.initLdap();
  }

  ngOnInit(): void {
    this.initForm();
    this.initLdap();
  }

  initForm() {
    this.formGroup = this.fb.group({
      serviceType: new FormControl(DataMap.ldapService.ldap.value),
      protocol: new FormControl(DataMap.ldapProtocol.ldap.value),
      dn: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validDn(),
          this.validSpecialKey(),
          this.baseUtilService.VALID.maxLength(255)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(63)
        ]
      }),
      checkCN: new FormControl(false),
      addressType: new FormControl(DataMap.ldapAddressType.ip.value),
      ips: this.fb.array([new FormControl('')]),
      port: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      }),
      domain: new FormControl(''),
      directory: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validDn(),
          this.validSpecialKey(),
          this.baseUtilService.VALID.maxLength(255)
        ]
      }),
      nameAttribute: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validSpecialKey(),
          this.baseUtilService.VALID.maxLength(63)
        ]
      }),
      projectType: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validSpecialKey(),
          this.baseUtilService.VALID.maxLength(63)
        ]
      }),
      userGroup: new FormControl(false),
      groupDirectory: new FormControl(''),
      groupNameAttribute: new FormControl(''),
      memberAttribute: new FormControl(''),
      groupProjectType: new FormControl('')
    });
    this.ctrls = (this.formGroup.get('ips') as FormArray).controls;
    this.listenForm();
  }

  validDnItem(item: string): boolean {
    const itemArr = item.split('=');
    return (
      itemArr.length === 2 && !filter(itemArr, val => isEmpty(trim(val))).length
    );
  }

  validDn(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const dnArr = control.value.split(',');
      if (filter(dnArr, item => !this.validDnItem(item)).length) {
        return {
          invalidDn: { value: control.value }
        };
      }
      return null;
    };
  }

  validIP(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const ipArr = control.value.split('.');
      if (
        ['0', '127', '244'].includes(ipArr[0]) ||
        control.value === '255.255.255.255'
      ) {
        return {
          invalidName: { value: control.value }
        };
      }
      return null;
    };
  }

  validSpecialKey(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      if (includes(control.value, "'")) {
        return {
          invalidSpecialKey: { value: control.value }
        };
      }
      return null;
    };
  }

  listenForm() {
    this.formGroup.get('addressType').valueChanges.subscribe(res => {
      if (res === DataMap.ldapAddressType.ip.value) {
        each((this.formGroup.get('ips') as FormArray).controls, ctrl => {
          ctrl?.setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip(),
            this.validIP()
          ]);
          ctrl?.updateValueAndValidity();
        });
        this.formGroup.get('domain').clearValidators();
      } else {
        each((this.formGroup.get('ips') as FormArray).controls, ctrl => {
          ctrl?.clearValidators();
          ctrl?.updateValueAndValidity();
        });
        this.formGroup
          .get('domain')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.nasshareDomain)
          ]);
      }
      this.formGroup.get('domain').updateValueAndValidity();
    });
    this.formGroup.get('userGroup').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('groupDirectory')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validDn(),
            this.validSpecialKey(),
            this.baseUtilService.VALID.maxLength(255)
          ]);
        this.formGroup
          .get('groupNameAttribute')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validSpecialKey(),
            this.baseUtilService.VALID.maxLength(63)
          ]);
        this.formGroup
          .get('memberAttribute')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validSpecialKey(),
            this.baseUtilService.VALID.maxLength(63)
          ]);
        this.formGroup
          .get('groupProjectType')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validSpecialKey(),
            this.baseUtilService.VALID.maxLength(63)
          ]);
      } else {
        this.formGroup.get('groupDirectory').clearValidators();
        this.formGroup.get('groupNameAttribute').clearValidators();
        this.formGroup.get('memberAttribute').clearValidators();
        this.formGroup.get('groupProjectType').clearValidators();
      }
      this.formGroup.get('groupDirectory').updateValueAndValidity();
      this.formGroup.get('groupNameAttribute').updateValueAndValidity();
      this.formGroup.get('memberAttribute').updateValueAndValidity();
      this.formGroup.get('groupProjectType').updateValueAndValidity();
    });
    this.formGroup.get('dn').valueChanges.subscribe(res => {
      if (!this.isDomain) {
        defer(() => this.getDomain(res));
      }
    });
  }

  addIp() {
    (this.formGroup.get('ips') as FormArray).push(
      new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ]
      })
    );
  }

  deleteIp(i) {
    (this.formGroup.get('ips') as FormArray).removeAt(i);
  }

  initLdap() {
    this.ldapApiService.queryLdapConfig({}).subscribe(
      (res: any) => {
        this.hasConfig = !isEmpty(res);
        this.ldapServiceStatus = res?.ldapEnable;
        this.ladapCheckCn = res?.ldapCheckCn;
        this.isDomain =
          res?.ldapAddrType === DataMap.ldapAddressType.domain.value;
        this.serviceItems = [
          {
            label: this.i18n.get('system_service_type_label'),
            content: this.dataMapService.getLabel('ldapService', res?.ldapType)
          },
          {
            label: this.i18n.get('common_protocol_label'),
            content: res?.ldapProtocol
          },
          {
            label: this.i18n.get('system_bind_dn_label'),
            content: res?.ldapBindDn
          },
          {
            label: this.i18n.get('system_bind_password_label'),
            content: res?.ldapBindPswd
          },
          {
            key: 'ladapCheckCn',
            hide: res?.ldapProtocol !== DataMap.ldapProtocol.ldaps.value
          },
          {
            label: this.i18n.get('system_address_type_label'),
            content: this.dataMapService.getLabel(
              'ldapAddressType',
              res?.ldapAddrType || DataMap.ldapAddressType.ip.value
            ),
            hide: this.isCyberEngine
          },
          {
            label: this.i18n.get('common_ip_address_label'),
            content: res?.ldapAddr,
            hide: res?.ldapAddrType === DataMap.ldapAddressType.domain.value
          },
          {
            label: this.i18n.get('common_domain_label'),
            content: res?.ldapAddr,
            hide: res?.ldapAddrType !== DataMap.ldapAddressType.domain.value
          },
          {
            label: this.i18n.get('common_port_label'),
            content: res?.ldapPort
          }
        ];
        this.userItems = [
          {
            label: this.i18n.get('system_belong_directory_label'),
            content: res?.ldapUserPath
          },
          {
            label: this.i18n.get('system_name_attribute_label'),
            content: res?.ldapUserName
          },
          {
            label: this.i18n.get('system_project_type_label'),
            content: res?.ldapUserObjectType
          }
        ];
        this.userGroupOn = res?.ldapEnableGroup;
        if (this.userGroupOn) {
          this.userGroupItems = [
            {
              label: this.i18n.get('system_belong_directory_label'),
              content: res?.ldapGroupPath
            },
            {
              label: this.i18n.get('system_name_attribute_label'),
              content: res?.ldapGroupName
            },
            {
              label: this.i18n.get('system_member_attribute_label'),
              content: res?.ldapGroupMember
            },
            {
              label: this.i18n.get('system_project_type_label'),
              content: res?.ldapGroupObjectType
            }
          ];
        } else {
          this.userGroupItems = [];
        }
        (this.formGroup.get('ips') as FormArray).clear();
        if (
          !isEmpty(res?.ldapAddr) &&
          res?.ldapAddrType === DataMap.ldapAddressType.ip.value
        ) {
          each(res?.ldapAddr?.split(','), () => this.addIp());
        } else {
          this.addIp();
        }
        const formParams = {
          serviceType: res?.ldapType,
          protocol: res?.ldapProtocol,
          dn: res?.ldapBindDn,
          password: '',
          checkCN: res?.ldapCheckCn,
          addressType: res?.ldapAddrType || DataMap.ldapAddressType.ip.value,
          ips:
            res?.ldapAddrType === DataMap.ldapAddressType.domain.value
              ? []
              : res?.ldapAddr?.split(',') || [],
          port: res?.ldapPort,
          domain:
            res?.ldapAddrType === DataMap.ldapAddressType.domain.value
              ? res?.ldapAddr
              : '',
          directory: res?.ldapUserPath,
          nameAttribute: res?.ldapUserName,
          projectType: res?.ldapUserObjectType,
          userGroup: res?.ldapEnableGroup,
          groupDirectory: res?.ldapGroupPath,
          groupNameAttribute: res?.ldapGroupName,
          memberAttribute: res?.ldapGroupMember,
          groupProjectType: res?.ldapGroupObjectType
        };
        this.formGroup.patchValue(formParams);
      },
      () => {
        this.serviceItems = [
          {
            label: this.i18n.get('system_service_type_label'),
            content: ''
          },
          {
            label: this.i18n.get('common_protocol_label'),
            content: ''
          },
          {
            label: this.i18n.get('system_bind_dn_label'),
            content: ''
          },
          {
            label: this.i18n.get('system_bind_password_label'),
            content: ''
          },
          {
            label: this.i18n.get('system_address_type_label'),
            content: this.dataMapService.getLabel(
              'ldapAddressType',
              DataMap.ldapAddressType.ip.value
            ),
            hide: this.isCyberEngine
          },
          {
            label: this.i18n.get('common_ip_address_label'),
            content: ''
          },
          {
            label: this.i18n.get('common_port_label'),
            content: ''
          }
        ];
        this.userItems = [
          {
            label: this.i18n.get('system_belong_directory_label'),
            content: ''
          },
          {
            label: this.i18n.get('system_name_attribute_label'),
            content: ''
          },
          {
            label: this.i18n.get('system_project_type_label'),
            content: ''
          }
        ];
        this.userGroupOn = false;
      }
    );
  }

  ldapServiceChange() {
    if (this.ldapServiceStatus) {
      if (this.hasConfig) {
        this.warningMessageService.create({
          content: this.i18n.get('system_disable_ldap_warn_label'),
          onOK: () => {
            this.ldapApiService
              .changeLdapConfig({
                isLdapEnable: false
              })
              .subscribe(() => {
                this.viewSettingFlag = true;
                this.ldapServiceStatus = false;
              });
          }
        });
      } else {
        this.viewSettingFlag = true;
        this.ldapServiceStatus = false;
      }
    } else {
      if (this.hasConfig) {
        this.ldapApiService
          .changeLdapConfig({
            isLdapEnable: true
          })
          .subscribe(() => {
            this.viewSettingFlag = true;
            this.ldapServiceStatus = true;
            this.initLdap();
          });
      } else {
        this.viewSettingFlag = true;
        this.ldapServiceStatus = true;
      }
    }
  }

  modify() {
    if (!this.ldapServiceStatus) {
      return;
    }
    this.viewSettingFlag = false;
    if (!this.hasConfig) {
      return;
    }
    this.initLdap();
  }

  cancel() {
    this.viewSettingFlag = true;
    if (!this.hasConfig) {
      return;
    }
    this.initLdap();
  }

  getDomain(dn) {
    if (!dn) {
      return '';
    }
    const dcArr = dn
      .split(',')
      .filter(item => toUpper(item.split('=')[0]) === 'DC');
    const domainArr = [];
    each(dcArr, item => {
      if (item && item.split('=')[1]) {
        domainArr.push(item.split('=')[1]);
      }
    });
    if (size(domainArr) > 1) {
      const domainName = domainArr.join('.');
      this.formGroup.get('domain').setValue(domainName);
      return domainName;
    }
    return '';
  }

  getParams(): LdapConfigRequest {
    const params = {
      ldapType: this.formGroup.value.serviceType,
      ldapEnable: this.ldapServiceStatus,
      ldapProtocol: this.formGroup.value.protocol,
      ldapBindDn: this.formGroup.value.dn,
      ldapBindPswd: this.formGroup.value.password,
      ldapAddrType: this.formGroup.value.addressType,
      ldapAddr:
        this.formGroup.value.addressType === DataMap.ldapAddressType.ip.value
          ? this.formGroup.value.ips?.join(',')
          : this.formGroup.value.domain,
      ldapPort: this.formGroup.value.port,
      ldapUserPath: this.formGroup.value.directory,
      ldapUserName: this.formGroup.value.nameAttribute,
      ldapUserObjectType: this.formGroup.value.projectType,
      ldapEnableGroup: this.formGroup.value.userGroup,
      ldapCheckCn:
        this.formGroup.value.protocol === DataMap.ldapProtocol.ldaps.value
          ? this.formGroup.value.checkCN
          : false
    };
    if (this.formGroup.value.userGroup) {
      assign(params, {
        ldapGroupPath: this.formGroup.value.groupDirectory,
        ldapGroupName: this.formGroup.value.groupNameAttribute,
        ldapGroupMember: this.formGroup.value.memberAttribute,
        ldapGroupObjectType: this.formGroup.value.groupProjectType
      });
    }
    return params as any;
  }

  test() {
    this.ldapApiService
      .testLdapConfig({ ldapConfig: this.getParams(), akOperationTips: false })
      .subscribe(
        () => {
          this.hasTest = true;
          this.messageService.success(
            this.i18n.get('common_operate_success_label'),
            {
              lvMessageKey: 'test_successful_key',
              lvShowCloseButton: true
            }
          );
        },
        () => {
          this.hasTest = false;
        }
      );
  }

  save() {
    this.ldapApiService
      .modifyLdapConfig({
        ldapConfig: this.getParams()
      })
      .subscribe(() => {
        this.viewSettingFlag = true;
        this.initLdap();
      });
  }
}
