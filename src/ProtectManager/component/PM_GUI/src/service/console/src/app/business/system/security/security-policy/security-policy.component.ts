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
import { DatePipe } from '@angular/common';
import { Component, OnInit, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  DatatableComponent,
  MessageService,
  OptionItem,
  PaginatorComponent
} from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  LocalStorageApiService,
  WarningMessageService
} from 'app/shared';
import {
  ClusterSecurityApiService,
  KmcApiService,
  SecurityApiService
} from 'app/shared/api/services';
import { I18NService } from 'app/shared/services/i18n.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { IpUtilService } from 'app/shared/services/ip-util.service';
import {
  assign,
  cloneDeep,
  each,
  extend,
  filter,
  find,
  includes,
  isEmpty,
  isFunction,
  isString,
  isUndefined,
  map,
  pick,
  remove,
  size,
  trim,
  uniqueId
} from 'lodash';

@Component({
  selector: 'aui-securit-ypolicy',
  templateUrl: './security-policy.component.html',
  styleUrls: ['./security-policy.component.less'],
  providers: [DatePipe]
})
export class SecuritypolicyComponent implements OnInit {
  enableCtrl;
  passCtrlLabel;
  passLenValLabel;
  passComplexValLabel;

  isViewLoginPolicy = true;
  isViewSecurityIp = true;
  isViewTimeoutPolicy = true;
  isViewPasswordPolicy = true;
  isViewKeyUpdatePolicy = true;
  passLenValOptions: OptionItem[];
  passComplexOptions: OptionItem[];
  pwdFormGroup: FormGroup;
  timeoutFormGroup: FormGroup;
  loginFormGroup: FormGroup;
  keyUpdateFormGroup: FormGroup;
  internalFormGroup: FormGroup;

  formGroupMap: { [key: string]: FormGroup } = {};
  ipFormGroupMap: { [key: string]: FormGroup } = {};
  passwordData = [];
  ipData = [];
  cachePasswordData = [];
  cacheIpData = [];
  passwordTotal = 0;
  sizeOptions = [5, 10, 20];
  addPasswordEnable = false;
  addAccessIpEnable = true;
  weakPassowrd = '';
  securityIp = '';
  visiblePasswordErrorTip = false;
  visibleIpErrorTip = false;
  currentErrorTip = '';
  currentIpErrorTip = '';
  MAX_USER_DEFINED_INFO_EN_LENGTH = 511;
  MAX_USER_DEFINED_INFO_CN_LENGTH = 170;
  encoder = new TextEncoder();
  policyData = {
    minLifetime: 0,
    passComplexVal: 0,
    passCtrl: false,
    passErrNum: 0,
    passLenVal: 0,
    passLockTime: 0,
    sessionTime: 0,
    usefulLife: 0
  };
  keyUpdatePolicy = {};
  internalUpdatePolicy = {};
  ipTableScroll = {
    view: {
      y: '320px',
      autosize: true
    },
    edit: {
      y: '280px',
      autosize: true
    }
  };
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isHyperdetect = includes(
    [
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.hyperdetect.value
    ],
    this.i18n.get('deploy_type')
  );
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );
  isDecouple =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.decouple.value;
  isDistributed =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.e6000.value;
  updatePwdTip = this.isCyberEngine
    ? this.i18n.get('system_password_update_cycle_cyberengin_label')
    : this.isHyperdetect
    ? this.i18n.get('system_password_update_cycle_hyperdetect_label')
    : this.isDistributed
    ? this.i18n.get('system_password_update_cycle_distributed_label')
    : this.i18n.get('system_password_update_cycle_label');

  sessionTimeErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 100])
  });
  keyPolicyErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [30, 180])
  });
  internalPolicyErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 1825])
  });
  passErrNumErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 9])
  });
  passLockTimeErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [3, 2000])
  });
  usefulLifeErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [60, 360])
  });
  minLifeTimeErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 9999])
  });

  passwordsNumberErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [0, 12])
  });
  passwordsDurationErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [0, 365])
  });
  customTipsErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLen: this.i18n.get(
      'common_bongding_port_name_length_tips_label',
      [this.MAX_USER_DEFINED_INFO_EN_LENGTH]
    )
  };

  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidRepeat: this.i18n.get('system_weak_name_repeat_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64]),
    invalidMinLength: this.i18n.get('common_valid_minlength_label', [8])
  };

  ipErrorTip = {
    ...this.baseUtilService.ipErrorTip,
    ipFormat: this.i18n.get('system_ipaddress_format_example_label', [
      'system_ipsegmentexample_value_label'
    ]),
    ipType: this.i18n.get('system_task_ip_para_endip_principle_label'),
    ipRange: this.i18n.get('system_iprule_para_startip_range_label'),
    ipRepeat: this.i18n.get('system_ip_notequall_msg_label')
  };

  loginPolicyLabel = this.i18n.get('system_policy_login_label');
  timeoutPolicyLabel = this.i18n.get('system_policy_session_timeout_label');
  passwordPolicyLabel = this.i18n.get('system_policy_password_label');
  minLengthLabel = this.i18n.get('system_policy_min_length_label');
  complexityLabel = this.i18n.get('system_policy_complexity_label');
  pwdStatusLabel = this.i18n.get('system_policy_pwd_status_label');
  pwdPeriodLabel = this.i18n.get('system_policy_pwd_period_label');
  errorCountLabel = this.i18n.get('system_policy_error_count_label');
  lockDurationLabel = this.i18n.get('system_policy_lock_duration_label');
  pwdSavePeriodLabel = this.i18n.get('system_policy_pwd_save_period_label');
  sessionTimeoutLabel = this.i18n.get('system_policy_session_timeout_label');
  complex2Label = this.i18n.get('system_complex_2_field_label');
  complex4Label = this.i18n.get('system_complex_4_field_label');
  minlength8Label = this.i18n.get('system_minlength_8_char_label');
  minlength10Label = this.i18n.get('system_minlength_10_char_label');
  minlength14Label = this.i18n.get('system_minlength_14_char_label');
  enableLabel = this.i18n.get('common_enable_label');
  disableLabel = this.i18n.get('common_disable_label');
  minuteLabel = this.i18n.get('common_minute_label');
  dayLabel = this.i18n.get('common_day_label');
  userDefinedInfoPlaceholder = this.i18n.get(
    'system_policy_user_defined_info_placeholder_label',
    [
      1,
      this.MAX_USER_DEFINED_INFO_EN_LENGTH,
      1,
      this.MAX_USER_DEFINED_INFO_CN_LENGTH
    ]
  );
  @ViewChild(DatatableComponent, { static: true })
  passwordTable: DatatableComponent;
  @ViewChild(DatatableComponent, { static: true })
  ipTable: DatatableComponent;
  @ViewChild('weakPassowrdPopover', { static: false }) weakPassowrdPopover;
  @ViewChild('ipPopover', { static: false }) ipPopover;
  @ViewChild('page', { static: false }) lvPage: PaginatorComponent;
  @ViewChild('page2', { static: false }) lvPage2: PaginatorComponent;

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    private datePipe: DatePipe,
    private message: MessageService,
    private kmcApiService: KmcApiService,
    public securityApiService: SecurityApiService,
    public baseUtilService: BaseUtilService,
    public infoMessageService: InfoMessageService,
    private warningMessageService: WarningMessageService,
    private LocalStorageApiService: LocalStorageApiService,
    private clusterSecurityApiService: ClusterSecurityApiService
  ) {}

  ngOnInit() {
    this.initPolicyData();
    this.initData();
    this.initKeyData();
    this.getWeakPasswords();
    this.getFeatureSwitch();
    this.getSecurityIp();
  }

  onChange() {
    this.ngOnInit();
  }

  updatePolicyNow() {
    this.infoMessageService.create({
      content: this.i18n.get('system_update_new_confirm_label'),
      onOK: () => {
        this.kmcApiService.updateKeyUsingPUT({}).subscribe(res => {
          this.initKeyData();
        });
      }
    });
  }

  initPolicyData() {
    // 一体机的密码策略需要额外的几个参数
    if (this.isCyberEngine) {
      assign(this.policyData, {
        isEnableLoginNotes: true,
        isEnableUserDefNotes: false,
        userDefNodes: null
      });
      // 因为不同的场景“访问控制”模块的高度不同
      // 所以动态控制scroll的高度
      assign(this.ipTableScroll, {
        view: {
          y: '380px'
        },
        edit: {
          y: '330px'
        }
      });
    }
  }

  initData() {
    this.getData(res => {
      this.passCtrlLabel = res.passCtrl ? this.enableLabel : this.disableLabel;
      (this.passLenValLabel =
        res.passLenVal === 8
          ? this.minlength8Label
          : res.passLenVal === 10
          ? this.minlength10Label
          : res.passLenVal === 14
          ? this.minlength14Label
          : ''),
        (this.passComplexValLabel =
          res.passComplexVal === 2 ? this.complex2Label : this.complex4Label);
      this.policyData = res;
      if (this.isCyberEngine) {
        this.policyData['userDefNodes'] = this.i18n.encodeHtml(
          res?.userDefNodes
        );
      }
    });
  }

  initKeyData() {
    this.getKeyUpdatePolicy(keyLifetime => {
      assign(this.keyUpdatePolicy, {
        keyLifetime
      });
    });
  }

  getKeyUpdatePolicy(callback) {
    this.kmcApiService.getKeyLifetimeUsingGET({}).subscribe(res => {
      callback(res);
    });
  }

  getData(callback) {
    this.securityApiService.getUsingGET1({}).subscribe(
      res => {
        callback(res);
      },
      err => {
        callback(err);
      }
    );
  }

  passCtrlChange(passCtrl) {
    if (!passCtrl) {
      this.pwdFormGroup.get('usefulLife').disable();
      this.pwdFormGroup.get('minLifetime').disable();
    } else {
      this.pwdFormGroup.get('usefulLife').enable();
      this.pwdFormGroup.get('minLifetime').enable();
    }
  }

  modifyPasswordPolicy() {
    this.passLenValOptions = this.initPassLenValOptions();
    this.passComplexOptions = this.initPassComplexOptions();
    this.getData(res => {
      this.policyData = res;
      this.pwdFormGroup = this.fb.group({
        passLenVal: [res.passLenVal],
        passComplexVal: [res.passComplexVal],
        passCtrl: [res.passCtrl],
        usefulLife: new FormControl(res.usefulLife, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(60, 360)
          ],
          updateOn: 'change'
        }),
        minLifetime: new FormControl(res.minLifetime, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 9999)
          ],
          updateOn: 'change'
        }),
        passHistoryNum: new FormControl(res.passHistoryNum, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(0, 12)
          ],
          updateOn: 'change'
        }),
        passHistoryDay: new FormControl(res.passHistoryDay, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(0, 365)
          ],
          updateOn: 'change'
        })
      });
      this.isViewPasswordPolicy = !this.isViewPasswordPolicy;
      this.passCtrlChange(res.passCtrl);
    });
  }

  savePasswordPolicy() {
    if (!this.pwdFormGroup.valid) {
      return;
    }
    this.save(
      extend({ ...this.policyData, ...this.pwdFormGroup.value }),
      res => {
        this.cancelPasswordPolicy();
      }
    );
  }

  cancelPasswordPolicy() {
    this.isViewPasswordPolicy = !this.isViewPasswordPolicy;
    this.initData();
  }

  modifyTimeoutPolicy() {
    this.passLenValOptions = this.initPassLenValOptions();
    this.passComplexOptions = this.initPassComplexOptions();
    this.getData(res => {
      this.timeoutFormGroup = this.fb.group({
        sessionTime: new FormControl(res.sessionTime, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 100)
          ],
          updateOn: 'change'
        })
      });
      this.isViewTimeoutPolicy = !this.isViewTimeoutPolicy;
    });
  }

  modifyLoginPolicy() {
    this.getData(res => {
      this.loginFormGroup = this.fb.group({
        passErrNum: new FormControl(res.passErrNum, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 9)
          ],
          updateOn: 'change'
        }),
        passLockTime: new FormControl(res.passLockTime, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(3, 2000)
          ],
          updateOn: 'change'
        })
      });
      this.addExtraLoginFormControl(res);
      this.isViewLoginPolicy = !this.isViewLoginPolicy;
    });
  }

  addExtraLoginFormControl(res) {
    // 根据部署形态添加额外的登录控件
    if (this.isCyberEngine) {
      this.loginFormGroup.addControl(
        'isEnableLoginNotes',
        new FormControl(!!res.isEnableLoginNotes)
      );
      this.loginFormGroup.addControl(
        'isEnableUserDefNotes',
        new FormControl(!!res?.isEnableUserDefNotes)
      );
      this.loginFormGroup.addControl(
        'userDefNodes',
        new FormControl(res?.userDefNodes || '')
      );
      this.loginFormGroup
        .get('isEnableUserDefNotes')
        .valueChanges.subscribe(res => {
          if (res) {
            this.loginFormGroup
              .get('userDefNodes')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.validCustomizeTips(this.MAX_USER_DEFINED_INFO_EN_LENGTH)
              ]);
          } else {
            this.loginFormGroup.get('userDefNodes').clearValidators();
          }
          this.loginFormGroup.get('userDefNodes').updateValueAndValidity();
        });
      this.loginFormGroup.get('isEnableUserDefNotes').updateValueAndValidity();
    }
  }

  modifyKeyUpdatePolicy() {
    this.getKeyUpdatePolicy(keyLifetime => {
      this.keyUpdateFormGroup = this.fb.group({
        keyLifetime: new FormControl(keyLifetime, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(30, 180)
          ]
        })
      });
      this.isViewKeyUpdatePolicy = !this.isViewKeyUpdatePolicy;
    });
  }

  saveTimeoutPolicy() {
    if (!this.timeoutFormGroup.valid) {
      return;
    }
    const params = { ...this.policyData, ...this.timeoutFormGroup.value };
    this.save(extend(params), res => {
      this.cancelTimeoutPolicy();
    });
  }

  saveLoginPolicy() {
    if (!this.loginFormGroup.valid) {
      return;
    }
    const params = { ...this.policyData, ...this.loginFormGroup.value };
    this.save(extend(params), res => {
      this.cancelLoginPolicy();
    });
  }

  saveKeyUpdatePolicy() {
    if (!this.keyUpdateFormGroup.valid) {
      return;
    }
    const keyLifetime = this.keyUpdateFormGroup.value.keyLifetime;
    this.saveKeyPolicy({ keyLifetime }, res => {
      this.cancelKeyUpdatePolicy();
    });
  }

  saveKeyPolicy(params, callback) {
    this.kmcApiService.modifyKeyLifetimeUsingPUT(params).subscribe(res => {
      callback();
    });
  }

  getSystemTime() {
    return this.datePipe.transform(new Date().getTime(), 'yyyyMMddHHmmss');
  }

  save(params, callback) {
    /**
     * OceanCyber的自定义提示userDefNodes不需要转成数字
     * 其余输入的数字串都需要转为number类型
     */
    each(params, (value, key) => {
      // tslint:disable-next-line: no-unused-expression
      if (!this.isCyberEngine && key !== 'userDefNodes') {
        isString(value) && (params[key] = +value);
      }
    });
    if (!params.passCtrl) {
      delete params.usefulLife;
      delete params.minLifetime;
    }
    this.securityApiService.modifyUsingPUT({ vo: params }).subscribe(res => {
      callback(res);
    });
  }

  cancelTimeoutPolicy() {
    this.isViewTimeoutPolicy = !this.isViewTimeoutPolicy;
    this.initData();
  }

  cancelLoginPolicy() {
    this.isViewLoginPolicy = !this.isViewLoginPolicy;
    this.initData();
  }

  cancelKeyUpdatePolicy() {
    this.isViewKeyUpdatePolicy = !this.isViewKeyUpdatePolicy;
    this.initKeyData();
  }

  initPassLenValOptions() {
    return [
      {
        passLenVal: 8,
        label: this.minlength8Label,
        isLeaf: true
      },
      {
        passLenVal: 10,
        label: this.minlength10Label,
        isLeaf: true
      },
      {
        passLenVal: 14,
        label: this.minlength14Label,
        isLeaf: true
      }
    ];
  }

  initPassComplexOptions() {
    return [
      {
        passComplexVal: 2,
        label: this.complex2Label,
        isLeaf: true
      },
      {
        passComplexVal: 4,
        label: this.complex4Label,
        isLeaf: true
      }
    ];
  }

  getWeakPasswords() {
    this.securityApiService.getWeekPasswordsUsingGET({}).subscribe(res => {
      this.passwordTotal = res.total;
      this.passwordData = res.weakPasswords;
      this.cachePasswordData = res.weakPasswords;
      this.searchByName();
      this.addPasswordEnable = this.passwordTotal < 256;
    });
  }

  searchByName() {
    if (this.weakPassowrdPopover) {
      this.weakPassowrdPopover.hide();
    }
    if (!this.weakPassowrd) {
      this.passwordData = cloneDeep(this.cachePasswordData);
    }
    this.passwordData = filter(this.cachePasswordData, item => {
      return includes(item.password, this.weakPassowrd);
    });
  }

  searchByIp() {
    if (this.ipPopover) {
      this.ipPopover.hide();
    }
    if (!this.securityIp) {
      this.ipData = cloneDeep(this.cacheIpData);
    }
    this.ipData = filter(this.cacheIpData, item => {
      return includes(item.securityIp, this.securityIp);
    });
  }

  saveData(data) {
    this.securityApiService
      .addWeakPasswordUsingPOST({
        weakPasswordAddRequest: {
          password: this.formGroupMap[data.id].value.password
        }
      })
      .subscribe(res => this.getWeakPasswords());
  }

  saveIpData(data) {
    each(this.ipData, item => {
      if (item.id !== data.id) {
        return;
      }
      item.isEditing = false;
      item.securityIp = this.ipFormGroupMap[data.id].value.securityIp;
    });
    this.addAccessIpEnable = true;
  }

  addTableData() {
    const params = {
      password: '',
      id: uniqueId(),
      isEditing: true
    };
    this.formGroupMap[params.id] = this.fb.group({
      password: new FormControl(params.password, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(8),
          this.baseUtilService.VALID.maxLength(64),
          this.validNameRepeat()
        ]
      })
    });
    this.passwordData = [...[params], ...this.passwordData];
    this.addPasswordEnable = false;
    this.lvPage.jumpToFisrtPage();
  }

  validNameRepeat(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }

      const result = filter(this.cachePasswordData, item => {
        return includes(item.password, trim(control.value));
      });

      return !!size(result)
        ? { invalidRepeat: { value: control.value } }
        : null;
    };
  }

  addIpTableData() {
    const params = {
      securityIp: '',
      id: uniqueId(),
      isEditing: true
    };
    this.ipFormGroupMap[params.id] = this.fb.group({
      securityIp: new FormControl(params.securityIp, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validSecurityIp(params.id)
        ]
      })
    });
    this.ipData = [...[params], ...this.ipData];
    this.addAccessIpEnable = false;
    this.lvPage2.jumpToFisrtPage();
  }

  validSecurityIp(id): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.ipFormGroupMap[id])) {
        return null;
      }

      const value = control.value;
      if (!value) {
        return null;
      }

      const ipArr = value.split('-');
      if (ipArr.length > 3) {
        return { ipFormat: true };
      }

      for (const ip of ipArr) {
        // IP为空，仅有分隔符
        if (!ip) {
          return { ipFormat: true };
        }

        // 校验IP格式
        const ipControl = new FormControl(ip);
        if (!BaseUtilService.ipWithMulticast(ipControl.value)) {
          return { ipFormat: true };
        }
      }

      if (ipArr.length === 2) {
        // 校验IP段中的两个IP是否为同一类型
        if (
          IpUtilService.getType(ipArr[0]) !== IpUtilService.getType(ipArr[1])
        ) {
          return { ipType: true };
        }

        // 校验IP范围是否合理，结束IP大于必须开始IP
        if (IpUtilService.compare(ipArr[0], ipArr[1]) >= 0) {
          return { ipRange: true };
        }
      }

      // 校验IP是否已存在
      if (this.checkIpRepeat(value)) {
        return { ipRepeat: true };
      }

      return null;
    };
  }

  /**
   * 检查IP是否和已有IP重复
   */
  checkIpRepeat(ip: string): boolean {
    const repeatIpRules: string[] = [];

    for (const ipRule of this.ipData) {
      const existIP = ipRule.securityIp;
      if (IpUtilService.compareIPSegment(ip, existIP) >= 0) {
        repeatIpRules.push(existIP);
      }
    }

    return repeatIpRules.length > 0;
  }

  /**
   * 检查输入的内容长度
   * 中文占1个字符 英文占3个字符
   */
  validCustomizeTips(maxLen: number): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      const str = control.value;
      const length = this.encoder.encode(str).length;
      if (length > maxLen) {
        return { invalidMaxLen: { value: str } };
      }
    };
  }

  deleteRow(source) {
    if (source.isEditing === true) {
      const removedRow = remove(
        this.passwordData,
        item => item.id === source.id
      );
      this.passwordData = [...this.passwordData];
      this.passwordTable.deleteSelection(removedRow);
      this.addPasswordEnable = true;
    } else {
      this.securityApiService
        .deleteWeakPasswordUsingDELETE({ id: source.id })
        .subscribe(res => {
          this.getWeakPasswords();
          this.addPasswordEnable = true;
        });
    }
  }

  deleteIpRow(source) {
    const removedRow = remove(this.ipData, item => item.id === source.id);
    this.ipData = [...this.ipData];
    this.ipTable.deleteSelection(removedRow);
    this.addAccessIpEnable = isUndefined(
      find(this.ipData, item => item.isEditing)
    );
  }

  checkInvalid(control) {
    this.visiblePasswordErrorTip =
      control.invalid && (control.dirty || control.touched);

    if (control.errors) {
      this.currentErrorTip = this.passwordErrorTip[
        Object.keys(control.errors)[0]
      ];
    }
  }

  checkIpInvalid(control) {
    this.visibleIpErrorTip =
      control.invalid && (control.dirty || control.touched);

    if (control.errors) {
      this.currentIpErrorTip = this.ipErrorTip[Object.keys(control.errors)[0]];
    }
  }

  focus(item) {
    this.checkInvalid(this.formGroupMap[item.id].controls.password);
  }

  ipFocus(item) {
    this.checkIpInvalid(this.ipFormGroupMap[item.id].controls.securityIp);
  }

  getFeatureSwitch() {
    this.securityApiService
      .getFeatureSwitchUsingGET({ featureName: 'access_control' })
      .subscribe(res => {
        this.enableCtrl = res.enable;
      });
  }

  getSecurityIp(callback?) {
    this.securityApiService.getSecurityIpsUsingGET({}).subscribe(res => {
      this.ipData = res.securityIpList;
      this.cacheIpData = this.ipData;
      this.addAccessIpEnable = res.securityIpList.length <= 32;
      isFunction(callback) && callback(res);
    });
  }

  initSecurityIp() {
    this.getSecurityIp(() => {
      this.isViewSecurityIp = !this.isViewSecurityIp;
    });
  }

  accessControlChange(enable) {
    if (!this.ipData.length) {
      this.message.error(this.i18n.get('system_control_ip_save_desc_label'), {
        lvShowCloseButton: true,
        lvMessageKey: 'lvMessageKey_noData'
      });
      return;
    }
    const featureSwitchRequest = {
      enable,
      featureName: 'access_control'
    };
    this.warningMessageService.create({
      content: enable
        ? this.i18n.get('system_access_control_enable_label')
        : this.i18n.get('system_access_control_disable_label'),
      onOK: () => {
        this.securityApiService
          .updateFeatureSwitchUsingPOST({ featureSwitchRequest })
          .subscribe(() => {
            this.getFeatureSwitch();
          });
      },
      onCancel: () => {
        this.getFeatureSwitch();
      },
      lvAfterClose: result => {
        if (result && result.trigger === 'close') {
          this.getFeatureSwitch();
        }
      }
    });
  }

  saveSecurityIp() {
    const params = map(this.ipData, item => {
      const _item = pick(item, 'securityIp');
      return _item;
    });
    this.securityApiService
      .updateSecurityIpsUsingPOST({
        securityIpListRequest: params
      })
      .subscribe(() => this.cancelSecurityIp());
  }

  cancelSecurityIp() {
    this.initSecurityIp();
  }

  modifySecurityIp() {
    this.initSecurityIp();
  }

  updateStoragePwd() {
    this.warningMessageService.create({
      content: this.i18n.get('system_password_update_warning_label'),
      onOK: () => {
        this.LocalStorageApiService.updateLocalStorageUserUsingPUT(
          {}
        ).subscribe(res => {});
      }
    });
  }

  updateComponentPwd() {
    this.warningMessageService.create({
      content: this.isHyperdetect
        ? this.i18n.get('system_password_update_hyperdetect_warn_label')
        : this.i18n.get('system_password_update_warn_label'),
      onOK: () => {
        this.clusterSecurityApiService
          .updateComponentPassword({})
          .subscribe(res => {});
      }
    });
  }
}
