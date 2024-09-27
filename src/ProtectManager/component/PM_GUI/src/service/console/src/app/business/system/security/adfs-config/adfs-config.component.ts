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
  FormGroup
} from '@angular/forms';
import { MenuItem, MessageService, UploadFile, isEmpty } from '@iux/live';
import {
  ADFSService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  WarningMessageService
} from 'app/shared';
import { set, size, trim } from 'lodash';

@Component({
  selector: 'aui-adfs-config',
  templateUrl: './adfs-config.component.html',
  styleUrls: ['./adfs-config.component.less']
})
export class AdfsConfigComponent implements OnInit {
  adfsStatus = false; // 开关状态
  editFlag = false; // 修改状态
  hasTest = false; // 测试状态
  hasConfig = false;
  items: MenuItem[];
  formItems = [];
  filters = [];
  selectCaCertificateFile;
  editForm: FormGroup;
  isFileValid = false;
  caValidFlag = '';

  providerOptions = this.dataMapService.toArray('adfsProvider').map(item => {
    item['isLeaf'] = true;
    return item;
  });

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_valid_name_with_allowed_dots_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024])
  };
  proUrlErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidUrl: this.i18n.get('system_adfs_provider_url_error_tips_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024])
  };

  clientIdErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };
  clientPasErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private warningMessageService: WarningMessageService,
    private baseUtilService: BaseUtilService,
    private adfsService: ADFSService,
    private dataMapService: DataMapService,
    private message: MessageService
  ) {}

  ngOnInit(): void {
    this.initAdfs();
    this.initForm();
    this.initCaFile();
  }

  validproUrl() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const reg = /^([hH]{1}[tT]{2}[pP]{1}[sS]?):\/\/[\s\S]*$/;
      if (!reg.test(control.value)) {
        return { invalidUrl: { value: control.value } };
      }
      return null;
    };
  }

  validName() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const reg = CommonConsts.REGEX.nameWithAllowedDots;
      if (!reg.test(control.value)) {
        return { invalidName: { value: control.value } };
      }
    };
  }

  initForm() {
    this.editForm = this.fb.group({
      provider: new FormControl(DataMap.adfsProvider.adfs.value),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validName(),
          this.baseUtilService.VALID.maxLength(1024)
        ]
      }),
      proUrl: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validproUrl(),
          this.baseUtilService.VALID.maxLength(1024)
        ]
      }),
      calUrl: new FormControl({ value: '', disabled: true }),
      clientID: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(255)
        ]
      }),
      clientPas: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(1024)
        ]
      })
    });
  }

  // 初始化表单展示
  initAdfs() {
    // 调用查询表单接口
    this.adfsService.getADFSConfig({}).subscribe(
      (res: any) => {
        this.hasConfig = !isEmpty(res);
        this.adfsStatus = !isEmpty(res);
        this.isFileValid = !isEmpty(res);
        let callbackUrl = '';
        if (this.isFileValid) {
          this.caValidFlag = 'optional';
        }
        this.adfsService.getCallBackUrl({}).subscribe((resData: any) => {
          callbackUrl = resData?.callbackUrl;
          this.formItems = [
            {
              label: this.i18n.get('system_adfs_select_provider_label'),
              content: DataMap.adfsProvider.adfs.value
            },
            {
              label: this.i18n.get('common_name_label'),
              content: res?.configName
            },
            {
              label: this.i18n.get('system_adfs_provider_url_label'),
              content: res?.providerUrl
            },
            {
              label: this.i18n.get('system_adfs_callback_url_label'),
              content: callbackUrl
            },
            {
              label: this.i18n.get('system_adfs_client_id_label'),
              content: res?.clientId
            }
          ];
          // 将从接口获取的表单数据临时存起来赋值给表单
          let editFormParams = {
            name: res?.configName,
            proUrl: res?.providerUrl,
            calUrl: callbackUrl,
            clientID: res?.clientId,
            clientPas: ''
          };
          this.editForm.patchValue(editFormParams);
        });
      },
      () => {
        this.formItems = [
          {
            label: this.i18n.get('system_adfs_select_provider_label'),
            content: DataMap.adfsProvider.adfs.value
          },
          {
            label: this.i18n.get('common_name_label'),
            content: ''
          },
          {
            label: this.i18n.get('system_adfs_provider_url_label'),
            content: ''
          },
          {
            label: this.i18n.get('system_adfs_callback_url_label'),
            content: ''
          },
          {
            label: this.i18n.get('system_adfs_client_id_label'),
            content: ''
          }
        ];
      }
    );
  }

  initCaFile() {
    this.filters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          this.selectCaCertificateFile = '';
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            const is1mb = file.size / 1024 / 1024 < 1;
            if (supportSuffix.includes(suffix) && is1mb) {
              return true;
            } else {
              return false;
            }
          });

          const fileType = files[0].name.split('.').pop();
          if (!supportSuffix.includes(fileType)) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.caValidFlag = 'incorrect';
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.caValidFlag = 'incorrect';
            return validFiles;
          }
          this.selectCaCertificateFile = files[0].originFile;
          this.caValidFlag = 'correct';
          return validFiles;
        }
      }
    ];
  }

  filesChange(file) {
    if (!size(file)) {
      this.selectCaCertificateFile = '';
      if (this.hasConfig) {
        this.caValidFlag = 'optional';
      }
    }
    if (!this.hasConfig) {
      if (this.selectCaCertificateFile) {
        this.isFileValid = true;
      } else {
        this.isFileValid = false;
      }
    } else {
      if (this.caValidFlag === 'incorrect') {
        this.isFileValid = false;
      } else {
        this.isFileValid = true;
      }
    }
  }

  // 开关点击事件
  switchChange() {
    this.editFlag = false;
    if (this.adfsStatus) {
      if (this.hasConfig) {
        this.warningMessageService.create({
          content: this.i18n.get('system_disable_adfs_warn_label'),
          onOK: () => {
            this.adfsService.closeADFSConfig({}).subscribe(() => {
              this.adfsStatus = false;
              this.editFlag = false;
              this.initAdfs();
            });
          }
        });
      } else {
        this.adfsStatus = false;
      }
    } else {
      if (this.hasConfig) {
        this.initAdfs();
      }
      this.adfsStatus = true;
    }
  }

  // 修改按钮事件
  modify() {
    if (!this.adfsStatus) {
      return;
    }
    this.editFlag = true;
    this.hasTest = false;
    if (!this.hasConfig) {
      return;
    }
    this.initAdfs();
  }

  // 保存按钮
  save() {
    const params = this.getParams();
    this.adfsService.setADFSConfig(params as any).subscribe(() => {
      this.editFlag = false;
      this.initAdfs();
    });
  }

  // 取消按钮
  cancel() {
    this.editFlag = false;
    if (!this.hasConfig) {
      return;
    }
    this.initAdfs();
  }

  // 测试按钮
  test() {
    const params = this.getParams();
    this.adfsService.checkADFSConfig(params as any).subscribe(
      () => {
        this.hasTest = true;
      },
      () => {
        this.hasTest = false;
      }
    );
  }

  getParams() {
    const params = {
      adfsEnable: this.adfsStatus,
      configName: this.editForm.value.name,
      providerUrl: this.editForm.value.proUrl,
      clientId: this.editForm.value.clientID,
      clientPwd: this.editForm.value.clientPas
    };
    if (this.selectCaCertificateFile) {
      set(params, 'caFile', this.selectCaCertificateFile);
    }
    return params;
  }
}
