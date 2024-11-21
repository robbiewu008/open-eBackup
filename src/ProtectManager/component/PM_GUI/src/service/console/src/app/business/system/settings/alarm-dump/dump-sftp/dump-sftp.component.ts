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
import { Component, OnInit, Output, EventEmitter } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  AlarmDumpApiService,
  BaseUtilService,
  DataMapService,
  I18NService
} from 'app/shared';
import { isFunction, isUndefined, omit, pick, slice } from 'lodash';

@Component({
  selector: 'aui-dump-sftp',
  templateUrl: './dump-sftp.component.html',
  styleUrls: ['./dump-sftp.component.less']
})
export class DumpSftpComponent implements OnInit {
  isModify = false;
  isFocusPassword = false;

  formItms;
  formGroup: FormGroup;

  userNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255]),
    invalidSpecailChars: this.i18n.get('system_invalid_sftp_name_label')
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  pathErrorTip = {
    ...this.baseUtilService.filePathErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024]),
    invalidSpecailChars: this.i18n.get('common_valid_file_path_label')
  };

  ipErrorTip = {
    ...this.baseUtilService.ipErrorTip
  };

  @Output() getAlarmSftpServer = new EventEmitter<any>();

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private alarmDumpApiService: AlarmDumpApiService
  ) {}

  ngOnInit() {
    this.initData();
  }

  getData(callback?) {
    this.alarmDumpApiService
      .queryAlarmSftpServerConfigUsingGET({})
      .subscribe(res => {
        this.getAlarmSftpServer.emit(res);
        isFunction(callback) && callback(res);
      });
  }

  initData() {
    this.getData(res => {
      let formItms = [
        {
          label: this.i18n.get('common_status_label'),
          content: this.dataMapService.getLabel('Switch_Status', res.useEnable)
        },
        {
          label: this.i18n.get('common_username_label'),
          content: res.userName
        },
        {
          label: this.i18n.get('common_password_label'),
          content: '********'
        },
        {
          label: this.i18n.get('system_service_ip_label'),
          content: res.ipAddress
        },
        {
          label: this.i18n.get('common_port_label'),
          content: res.port
        },
        {
          label: this.i18n.get('system_file_save_path_label'),
          content: res.uploadPath
        }
      ];

      if (!res.useEnable) {
        formItms = slice(formItms, 0, 1);
      }
      this.formItms = formItms;
    });
  }

  initSftpForm() {
    this.getData(res => {
      this.formGroup = this.fb.group({
        useEnable: new FormControl(res.useEnable),
        userName: new FormControl(res.userName, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(255),
            this.invalidSpecailChars()
          ],
          updateOn: 'change'
        }),
        password: new FormControl(res.userName ? '' : '', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(255)
          ],
          updateOn: 'change'
        }),
        ipAddress: new FormControl(res.ipAddress, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID._ip()
          ],
          updateOn: 'change'
        }),
        port: new FormControl(res.port || 22, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ],
          updateOn: 'change'
        }),
        uploadPath: new FormControl(res.uploadPath, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(1024),
            this.invalidSpecailChars(),
            this.baseUtilService.VALID.filePath()
          ],
          updateOn: 'change'
        })
      });
      this.isModify = !this.isModify;
      this.listenStatus();
    });
  }

  invalidSpecailChars(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      const reg = /[':\?\\"<>\|\*;`\$&\-\(\)#\!]+/;
      const isValid = !reg.test(control.value);
      if (isValid) {
        return null;
      }

      return { invalidSpecailChars: { value: control.value } };
    };
  }

  modifyData() {
    this.initSftpForm();
  }

  saveData() {
    if (
      !(
        (this.formGroup.value.useEnable && this.formGroup.valid) ||
        !this.formGroup.value.useEnable
      )
    ) {
      return;
    }

    let request = this.formGroup.value;

    if (!this.formGroup.value.useEnable) {
      request = pick(request, 'useEnable');
    }

    this.alarmDumpApiService
      .modifyAlarmSftpServerConfigUsingPUT({ request })
      .subscribe(() => this.cancelData());
  }

  testData() {
    if (!(this.formGroup.valid && this.formGroup.value.useEnable)) {
      return;
    }

    const request = this.formGroup.value;

    this.alarmDumpApiService
      .checkIfAlarmSftpServerConnectivityUsingPOST({
        request: omit(request, 'useEnable')
      })
      .subscribe(() => {});
  }

  cancelData() {
    this.isFocusPassword = false;
    this.isModify = !this.isModify;
    this.initData();
  }

  focusPwd() {
    if (!this.isFocusPassword) {
      this.formGroup.get('password').setValue('');
      this.isFocusPassword = true;
    }
  }

  listenStatus() {
    this.formGroup.get('useEnable').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('userName').clearValidators();
        this.formGroup.get('password').clearValidators();
        this.formGroup.get('ipAddress').clearValidators();
        this.formGroup.get('port').clearValidators();
        this.formGroup.get('uploadPath').clearValidators();
      } else {
        this.formGroup
          .get('userName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(255),
            this.invalidSpecailChars()
          ]);
        this.formGroup
          .get('password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(255)
          ]);
        this.formGroup
          .get('ipAddress')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID._ip()
          ]);
        this.formGroup
          .get('port')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]);
        this.formGroup
          .get('uploadPath')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(1024),
            this.invalidSpecailChars(),
            this.baseUtilService.VALID.filePath()
          ]);
      }
      this.formGroup.get('userName').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
      this.formGroup.get('ipAddress').updateValueAndValidity();
      this.formGroup.get('port').updateValueAndValidity();
      this.formGroup.get('uploadPath').updateValueAndValidity();
    });
  }
}
