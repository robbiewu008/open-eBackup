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
import { Component, OnInit, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { DatatableComponent, MessageService } from '@iux/live';
import {
  ALARM_SEVERITY,
  BaseUtilService,
  CommonConsts,
  CookieService,
  ENCRYPTION_METHOD,
  LANGUAGR_TYPE,
  SWITCH_TYPE
} from 'app/shared';
import { AlarmEmailNotifyApiService } from 'app/shared/api/services';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { I18NService } from 'app/shared/services/i18n.service';
import {
  assign,
  cloneDeep,
  forEach,
  includes,
  isArray,
  isEmpty,
  isNull,
  map,
  upperCase
} from 'lodash';

@Component({
  selector: 'alarm-notify',
  templateUrl: './alarm-notify.component.html',
  styles: [
    `
      .notify-page {
        height: 100%;
        display: flex;
        justify-content: space-between;
      }

      .notify-page .aui-block {
        width: calc(50% - 8px);
      }

      .notify-page .lv-input.lv-input-size-default {
        width: 100%;
      }

      .notify-column-lg {
        margin-top: 24px;
      }

      .notify-add-recive-btn {
        text-align: center !important;
      }
      .alarm-icon-mg {
        margin-right: 24px;
      }
      .recipient-label > h2 {
        margin-bottom: 12px;
      }

      .tip-info {
        display: inline-block;
      }

      .icon-info {
        float: left;
        vertical-align: middle;
        margin-right: 8px;
        margin-top: 12px;
      }

      .encription-tip {
        width: 95%;
        float: left;
        margin-top: 8px;
        line-height: 24px;
      }
      .max-content {
        max-width: 420px;
      }
      .icon-eye-open {
        position: relative;
        top: 2px;
      }
      .table-email-label {
        max-width: 80%;
      }
      .lv-alert {
        margin-bottom: 0px !important;
      }
    `
  ]
})
export class AlarmNotifyComponent implements OnInit {
  isModifyRecipient = false;
  isModifySender = false;
  isModifyNotifyObject = false;
  isArray = isArray;

  sendeForm;
  senderFormItms = [];
  recipitentFormItms;
  recipientForm;

  formGroupMap: { [key: string]: FormGroup } = {};

  selection = [];
  tableData = [];
  emailSelection = [];

  languageMethods;
  encryptionMethods;
  severityItems;
  addAble = true; // 控制是否可添加收件人，解决不能批量保存的问题
  enableRecipientAdd = false; // 收件设置未初始化不能添加
  asteriskLabel = '********';
  hasTested = false;

  testLabel = this.i18n.get('common_test_label');
  usernameLabel = this.i18n.get('common_username_label');
  passwordLabel = this.i18n.get('common_password_label');
  notifyLabel = this.i18n.get('system_alarm_notification_label');
  senderSettingLabel = this.i18n.get('system_sender_settings_label');
  recipientLabel = this.i18n.get('system_recipient_settings_label');
  smtpServerLabel = this.i18n.get('system_smtp_server_label');
  smtpPortLabel = this.i18n.get('system_smtp_port_label');
  senderEmailLabel = this.i18n.get('system_sender_email_label');
  recipientEmailLabel = this.i18n.get('system_recipient_email_label');
  testEmailLabel = this.i18n.get('system_test_email_label');
  encryptionMethodLabel = this.i18n.get('system_encryption_method_label');
  sslPortLabel = this.i18n.get('system_ssl_port_label');
  smtpServerAuthLabel = this.i18n.get('system_smtp_server_auth_label');
  proxyServerLabel = this.i18n.get('system_proxy_server_label');
  encriptionTipLabel = this.i18n.get('system_encription_tip_label');
  recipitionTipLabel = this.i18n.get('system_recipition_tip_label');
  maintenanceStatusLabel = this.i18n.get('system_maintenance_status_label');
  alarmSeverityLabel = this.i18n.get('system_alarm_severity_label');
  languageLabel = this.i18n.get('system_language_type_label');
  notifyObjectLabel = this.i18n.get('common_recipient_label');
  emailLabel = this.i18n.get('common_email_label');
  ipAdressLabel = this.i18n.get('common_ip_address_label');
  portLabel = this.i18n.get('common_port_label');
  editLabel = this.i18n.get('common_modify_label');
  saveLabel = this.i18n.get('common_save_label');
  cancelLabel = this.i18n.get('common_cancel_label');
  addLabel = this.i18n.get('common_add_label');
  descLabel = this.i18n.get('common_desc_label');
  optLabel = this.i18n.get('common_operation_label');
  deleteLabel = this.i18n.get('common_delete_label');
  encriptionNoLabel = this.i18n.get('system_not_encrypted_label');
  notifyObjectTipLabel = this.i18n.get('system_nofify_object_tip_label');
  recipientDescLabel = this.i18n.get('system_recipent_desc_label');

  @ViewChild(DatatableComponent, { static: true }) lvTable: DatatableComponent;

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public messageService: MessageService,
    public baseUtilService: BaseUtilService,
    public emailApiService: AlarmEmailNotifyApiService,
    public drawModalService: DrawModalService,
    public batchOperateService: BatchOperateService,
    private cookieService: CookieService
  ) {}

  rangeErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  });

  requiredErrorTip = assign(this.baseUtilService.emailErrorTip, {
    asterisk: this.baseUtilService.invalidInputLabel
  });

  passwordErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    asterisk: this.i18n.get('common_invalid_asterisk_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });

  emailAddressError = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [254])
    },
    {
      invalidName: this.i18n.get('system_error_email_label')
    },
    {
      invalidEmail: this.i18n.get('system_same_email_error_label')
    }
  );

  emailErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
    },
    {
      invalidName: this.i18n.get('system_error_email_label')
    }
  );

  testEmailErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
    },
    {
      invalidName: this.i18n.get('system_error_email_label')
    }
  );

  userNameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });

  descErrorTip = assign({}, this.baseUtilService.lengthErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
  });

  passWordChanged = false;

  recipientOpts = [];

  originalEmailFrom;
  originalTestEmail;
  showOriginalEmailFrom = false;
  showOriginalTestEmail = false;

  ngOnInit() {
    this.initSenderData();
  }

  onChange() {
    this.ngOnInit();
  }

  maskEmail(email) {
    if (isEmpty(email)) {
      return '';
    }
    const prefixStr = email.split('@')[0];
    return prefixStr.slice(0, 3) + '***' + '@' + email.split('@')[1];
  }

  clearOriginalEmail() {
    this.originalEmailFrom = '';
    this.originalTestEmail = '';
    this.showOriginalEmailFrom = false;
    this.showOriginalTestEmail = false;
  }

  emailBlur() {
    if (this.sendeForm.controls.emailFrom.status !== 'VALID') {
      return;
    }
    this.originalEmailFrom = this.sendeForm.value.emailFrom;
    this.sendeForm
      .get('emailFrom')
      .setValue(this.maskEmail(this.originalEmailFrom));
  }

  emailFocus() {
    if (this.originalEmailFrom) {
      this.sendeForm.get('emailFrom').setValue(this.originalEmailFrom);
    }
  }

  testEmailBlur() {
    if (this.sendeForm.controls.testEmail.status !== 'VALID') {
      return;
    }
    this.originalTestEmail = this.sendeForm.value.testEmail;
    this.sendeForm
      .get('testEmail')
      .setValue(this.maskEmail(this.originalTestEmail));
  }

  testEmailFocus() {
    if (this.originalTestEmail) {
      this.sendeForm.get('testEmail').setValue(this.originalTestEmail);
    }
  }

  changeEmail(item) {
    if (item.label == this.senderEmailLabel) {
      item.content = !this.showOriginalEmailFrom
        ? this.originalEmailFrom
        : this.maskEmail(this.originalEmailFrom);
      this.showOriginalEmailFrom = !this.showOriginalEmailFrom;
      item.icon = this.showOriginalEmailFrom
        ? 'aui-icon-eye-open'
        : 'aui-icon-eye-close';
    } else {
      item.content = !this.showOriginalTestEmail
        ? this.originalTestEmail
        : this.maskEmail(this.originalTestEmail);
      this.showOriginalTestEmail = !this.showOriginalTestEmail;
      item.icon = this.showOriginalTestEmail
        ? 'aui-icon-eye-open'
        : 'aui-icon-eye-close';
    }
  }

  getSenderData(callback) {
    this.emailApiService.queryEmailServerUsingGET({}).subscribe(
      res => {
        callback(res);
      },
      err => {
        callback(err);
      }
    );
  }

  getEncryptionMethod = item => {
    if (isNull(item.isSslEnable) && isNull(item.isTlsEnable)) {
      return '2';
    }
    if (item.isSslEnable) {
      return '1';
    } else if (item.isTlsEnable) {
      return '2';
    } else {
      return '0';
    }
  };

  encryptionMethodChange = e => {
    if (e === '1') {
      this.sendeForm.patchValue({ isSSLEnable: true, isTLSEnable: false });
      this.sendeForm.get('sslSmtpPort').enable();
      this.encriptionTipLabel = this.i18n.get(
        'system_encription_ssl_tip_label'
      );
      return;
    }
    this.sendeForm.get('sslSmtpPort').disable();
    if (e === '2') {
      this.sendeForm.patchValue({ isSSLEnable: false, isTLSEnable: true });
      this.encriptionTipLabel = this.i18n.get(
        'system_encription_starttls_tip_label'
      );
      return;
    }
    this.encriptionTipLabel = this.i18n.get('system_encription_tip_label');
    this.sendeForm.patchValue({ isSSLEnable: false, isTLSEnable: false });
  };

  initSenderData() {
    this.getSenderData(data => {
      if (data.emailFrom) {
        this.originalEmailFrom = data.emailFrom;
      }
      if (data.testEmail) {
        this.originalTestEmail = data.testEmail;
      }
      this.senderFormItms = [
        {
          label: this.smtpServerLabel,
          content: data.server || '--'
        },
        {
          label: this.smtpPortLabel,
          content: data.port || '--'
        },
        {
          label: this.senderEmailLabel,
          content: this.maskEmail(data.emailFrom) || '--',
          icon: 'aui-icon-eye-close'
        },
        {
          label: this.testEmailLabel,
          content: this.maskEmail(data.testEmail) || '--',
          icon: 'aui-icon-eye-close'
        },
        {
          label: this.encryptionMethodLabel,
          content: this.i18n.get(
            `system_${ENCRYPTION_METHOD[this.getEncryptionMethod(data)]}_label`
          )
        },
        {
          label: this.sslPortLabel,
          content: data.sslSmtpPort || '--'
        },
        {
          label: this.smtpServerAuthLabel,
          content: this.i18n.get(
            `common_${SWITCH_TYPE[+data.validateEnable]}_label`
          )
        },
        {
          label: this.usernameLabel,
          content: data.userName || '--'
        },
        {
          label: this.passwordLabel,
          content: !!data.userName ? this.asteriskLabel : '--'
        },
        {
          label: this.proxyServerLabel,
          content: this.i18n.get(
            `common_${SWITCH_TYPE[+data.proxyEnable]}_label`
          )
        },
        {
          label: this.ipAdressLabel,
          content: data.proxyServer || '--'
        },
        {
          label: this.portLabel,
          content: data.proxyPort || '--'
        }
      ];
      if (!data.validateEnable) {
        this.senderFormItms = this.senderFormItms.reduce((arr, item) => {
          if (
            item.label !== this.usernameLabel &&
            item.label !== this.passwordLabel
          ) {
            arr.push(item);
          }
          return arr;
        }, []);
      }
      if (!data.proxyEnable) {
        this.senderFormItms = this.senderFormItms.reduce((arr, item) => {
          if (
            item.label !== this.ipAdressLabel &&
            item.label !== this.portLabel
          ) {
            arr.push(item);
          }
          return arr;
        }, []);
      }

      // 是否选择ssl
      if (!data.isSslEnable) {
        this.senderFormItms = this.senderFormItms.reduce((arr, item) => {
          if (item.label !== this.sslPortLabel) {
            arr.push(item);
          }
          return arr;
        }, []);
      }
    });
  }

  initSenderForm() {
    this.getSenderData(data => {
      const protocol = isEmpty(data.server)
        ? '1'
        : this.getEncryptionMethod(data);
      if (data.emailFrom) {
        this.originalEmailFrom = data.emailFrom;
      }
      if (data.testEmail) {
        this.originalTestEmail = data.testEmail;
      }
      this.sendeForm = this.fb.group({
        server: new FormControl(data.server, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip(CommonConsts.REGEX.severIp)
          ],
          updateOn: 'change'
        }),
        port: new FormControl(data.port || 25, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ],
          updateOn: 'change'
        }),
        emailFrom: new FormControl(this.maskEmail(data.emailFrom) || '', {
          validators: [
            this.baseUtilService.VALID.name(CommonConsts.REGEX.email),
            this.baseUtilService.VALID.maxLength(255)
          ],
          updateOn: 'change'
        }),
        testEmail: new FormControl(this.maskEmail(data.testEmail) || '', {
          validators: [
            this.baseUtilService.VALID.name(CommonConsts.REGEX.email),
            this.baseUtilService.VALID.maxLength(255)
          ],
          updateOn: 'change'
        }),
        isSSLEnable: new FormControl(data.isSslEnable),
        protocol: new FormControl(protocol),
        isTLSEnable: new FormControl(data.isTlsEnable),
        sslSmtpPort: new FormControl(
          { value: data.sslSmtpPort, disabled: data.isTlsEnable },
          {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 65535)
            ],
            updateOn: 'change'
          }
        ),
        validateEnable: new FormControl(data.validateEnable),
        userName: new FormControl(
          { value: data.userName, disabled: !data.validateEnable },
          {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(64)
            ],
            updateOn: 'change'
          }
        ),
        password: new FormControl(
          {
            value: '',
            disabled: !data.validateEnable
          },
          {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(64)
            ],
            updateOn: 'change'
          }
        ),
        proxyEnable: new FormControl(data.proxyEnable),
        proxyServer: new FormControl(
          { value: data.proxyServer, disabled: !data.proxyEnable },
          {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.ip()
            ],
            updateOn: 'change'
          }
        ),
        proxyPort: new FormControl(
          { value: data.proxyPort, disabled: !data.proxyEnable },
          {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 65535)
            ],
            updateOn: 'change'
          }
        )
      });
      this.sendeForm.valueChanges.subscribe(res => {
        // 如果测试通过一次后，只要再次修改表单的值就要重新测试才行
        this.hasTested = this.hasTested && false;
      });
      this.isModifySender = true;
      this.encryptionMethodChange(protocol);
    });
  }

  validateChange = e => {
    if (e) {
      this.sendeForm.get('password').enable();
      this.sendeForm.get('userName').enable();
      this.sendeForm
        .get('password')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.asterisk(),
          this.baseUtilService.VALID.maxLength(64)
        ]);
    } else {
      this.sendeForm.get('password').disable();
      this.sendeForm.get('userName').disable();
      this.sendeForm.get('password').clearValidators();
    }
    this.sendeForm.get('password').updateValueAndValidity();
  };

  proxyChange = e => {
    if (e) {
      this.sendeForm.get('proxyServer').enable();
      this.sendeForm.get('proxyPort').enable();
      return;
    }
    this.sendeForm.get('proxyServer').disable();
    this.sendeForm.get('proxyPort').disable();
  };

  cancelSenderData() {
    this.isModifySender = false;
    this.passWordChanged = false;
    this.initSenderData();
    this.clearOriginalEmail();
  }

  modifySenderData() {
    this.hasTested = false;
    this.initSelectMethod();
    this.initSenderForm();
  }

  saveSenderData() {
    if (!this.sendeForm.valid) {
      return;
    }
    this.passWordChanged = false;
    if (this.asteriskLabel === this.sendeForm.value.password) {
      this.sendeForm.get('password').setValue('');
    }
    if (isEmpty(this.sendeForm.value.testEmail)) {
      this.sendeForm.value.testEmail = null;
    }
    const params = cloneDeep(this.sendeForm.value);
    assign(params, {
      emailFrom: this.originalEmailFrom
    });
    if (this.sendeForm.value.testEmail) {
      assign(params, {
        testEmail: this.originalTestEmail
      });
    }
    this.emailApiService
      .modifyEmailServerUsingPUT({
        notifyEmailServerRequest: params
      })
      .subscribe(res => {
        this.cancelSenderData();
      });
  }

  initSelectMethod() {
    this.languageMethods = [];
    this.encryptionMethods = [];
    forEach(LANGUAGR_TYPE, (item: any) => {
      if (isNaN(item)) {
        this.languageMethods.push({
          label: this.i18n.get(`common_${item}_label`),
          value: LANGUAGR_TYPE[item],
          isLeaf: true
        });
      }
    });
    forEach(ENCRYPTION_METHOD, item => {
      if (isNaN(item)) {
        this.encryptionMethods.push({
          label: this.i18n.get(`system_${item}_label`),
          value: String(ENCRYPTION_METHOD[item]),
          isLeaf: true
        });
      }
    });
  }

  sortAlarm(arr) {
    const sortArr = [];
    forEach(ALARM_SEVERITY, item => {
      if (includes(arr, upperCase(item))) {
        sortArr.push(upperCase(item));
      }
    });
    return sortArr;
  }

  getSeverity(data: any[]) {
    data = map(data, val => {
      return this.i18n.get(`common_alarms_${ALARM_SEVERITY[val]}_label`);
    });
    return data;
  }

  testSenderData() {
    if (!this.sendeForm.valid) {
      return;
    }
    if (this.asteriskLabel === this.sendeForm.value.password) {
      this.sendeForm.get('password').setValue('');
    }
    const params = cloneDeep(this.sendeForm.value);
    assign(params, {
      emailFrom: this.originalEmailFrom,
      testEmail: this.originalTestEmail
    });
    this.emailApiService
      .testEmailServerUsingPUT({ request: params })
      .subscribe({
        next: () => {
          this.hasTested = true;
          this.afterTest();
        },
        error: () => {
          this.hasTested = false;
          this.afterTest();
        }
      });
  }

  // 完成测试密码回填
  afterTest() {
    if ('' === this.sendeForm.value.password) {
      this.sendeForm.get('password').setValue(this.asteriskLabel);
      this.sendeForm
        .get('password')
        .setValidators([this.baseUtilService.VALID.required()]);
    }
  }

  // email 添加 不能重复
  addEmailValid(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      return this.tableData.find(
        el => control.value === el.originalEmailAddress && !el.isEditing
      )
        ? { invalidEmail: { value: control.value } }
        : null;
    };
  }

  emailAddressBlur(item) {
    if (this.formGroupMap[item.id].controls.emailAddress.status !== 'VALID') {
      return;
    }
    assign(item, {
      originalEmailAddress: this.formGroupMap[item.id].value.emailAddress
    });
    this.formGroupMap[item.id]
      .get('emailAddress')
      .setValue(this.maskEmail(item.originalEmailAddress));
  }

  emailAddressFocus(item) {
    if (item.originalEmailAddress) {
      this.formGroupMap[item.id]
        .get('emailAddress')
        .setValue(item.originalEmailAddress);
    }
  }

  changeEmailAddress(item) {
    item.emailAddress = item.showOriginal
      ? this.maskEmail(item.originalEmailAddress)
      : item.originalEmailAddress;
    item.showOriginal = !item.showOriginal;
    item.icon = item.showOriginal ? 'aui-icon-eye-open' : 'aui-icon-eye-close';
  }

  /**
   * 8*校验 密码不允许输入8个*
   *
   * @return  {ValidatorFn}         [return description]
   */
  asterisk(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      return control.value === this.asteriskLabel
        ? { asterisk: { value: control.value } }
        : null;
    };
  }
}
