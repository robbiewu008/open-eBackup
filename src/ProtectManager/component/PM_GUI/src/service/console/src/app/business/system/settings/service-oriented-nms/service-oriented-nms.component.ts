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
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  DmeApiService,
  I18NService,
  WarningMessageService
} from 'app/shared';
import { isEmpty } from 'lodash';
import { SetDmeConfigUsingPOSTRequestBody } from '../../../../shared/api/models/set-dme-config-using-postrequest-body';

@Component({
  selector: 'aui-service-oriented-nms',
  templateUrl: './service-oriented-nms.component.html',
  styleUrls: ['./service-oriented-nms.component.less']
})
export class ServiceOrientedNMSComponent implements OnInit {
  viewSettingFlag = true;
  dmeServiceStatus = false;
  hasConfig = false;
  formGroup: FormGroup;
  dataMap = DataMap;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  serviceItems = [];
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [63])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [50])
  };
  portErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
  };
  portValid = [
    this.baseUtilService.VALID.required(),
    this.baseUtilService.VALID.maxLength(256)
  ];

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private demApiService: DmeApiService,
    private baseUtilService: BaseUtilService,
    private warningMessageService: WarningMessageService
  ) {}

  onChange() {
    this.initDme();
  }

  ngOnInit(): void {
    this.initForm();
    this.initDme();
  }

  initForm() {
    this.formGroup = this.fb.group({
      north: new FormControl('', {
        validators: this.portValid
      }),
      south: new FormControl('', {
        validators: this.portValid
      }),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(50)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(63)
        ]
      }),
      verify: new FormControl(false)
    });
    this.listenForm();
  }

  listenForm() {}

  initDme() {
    this.demApiService.getDmeConfigUsingGET({}).subscribe(
      (res: any) => {
        this.hasConfig = !isEmpty(res?.nbiScUrl);
        if (!this.dmeServiceStatus) {
          this.dmeServiceStatus = !!res?.nbiScUrl;
        }
        this.serviceItems = [
          {
            label: this.i18n.get('system_north_dem_label'),
            content: res?.nbiScUrl
          },
          {
            label: this.i18n.get('system_south_dem_label'),
            content: res?.sbiScUrl
          },
          {
            label: this.i18n.get('common_username_label'),
            content: res?.serviceUserName
          },
          {
            label: this.i18n.get('common_password_label'),
            content: '******'
          },
          {
            label: this.i18n.get('protection_register_vm_cert_label'),
            content: res?.isVerifyCert
              ? this.i18n.get('common_yes_label')
              : this.i18n.get('common_no_label')
          }
        ];
        const formParams = {
          north: res?.nbiScUrl,
          south: res?.sbiScUrl,
          username: res.serviceUserName,
          password: '',
          verify: res.isVerifyCert
        };
        this.formGroup.patchValue(formParams);
      },
      () => {
        this.serviceItems = [
          {
            label: this.i18n.get('system_north_dem_label'),
            content: ''
          },
          {
            label: this.i18n.get('system_south_dem_label'),
            content: ''
          },
          {
            label: this.i18n.get('common_username_label'),
            content: ''
          },
          {
            label: this.i18n.get('common_password_label'),
            content: ''
          },
          {
            label: this.i18n.get('protection_register_vm_cert_label'),
            content: this.i18n.get('common_no_label')
          }
        ];
      }
    );
  }

  delFormData() {
    this.warningMessageService.create({
      content: this.i18n.get('system_disable_dme_warn_label'),
      onOK: () => {
        this.demApiService
          .setDmeConfigUsingPOST({
            DmeInitConfigReq: {
              isVerifyCert: false,
              nbiScUrl: '',
              sbiScUrl: '',
              serviceUserName: '',
              serviceUserPassWord: ''
            }
          })
          .subscribe(() => {
            this.viewSettingFlag = true;
            this.dmeServiceStatus = false;
          });
      }
    });
  }

  dmeServiceChange() {
    if (this.dmeServiceStatus) {
      if (this.hasConfig) {
        this.delFormData();
      } else {
        this.viewSettingFlag = true;
        this.dmeServiceStatus = false;
      }
    } else {
      if (this.hasConfig) {
        this.viewSettingFlag = true;
        this.dmeServiceStatus = true;
        this.initDme();
      } else {
        this.viewSettingFlag = true;
        this.dmeServiceStatus = true;
      }
    }
  }

  modify() {
    if (!this.dmeServiceStatus) {
      return;
    }
    this.viewSettingFlag = false;
    if (!this.hasConfig) {
      return;
    }
    this.initDme();
  }

  cancel() {
    this.viewSettingFlag = true;
    if (!this.hasConfig) {
      return;
    }
    this.initDme();
  }

  getParams(): SetDmeConfigUsingPOSTRequestBody {
    return {
      isVerifyCert: this.formGroup.value.verify,
      nbiScUrl: this.formGroup.value.north,
      sbiScUrl: this.formGroup.value.south,
      serviceUserName: this.formGroup.value.username,
      serviceUserPassWord: this.formGroup.value.password
    };
  }

  save() {
    this.demApiService
      .setDmeConfigUsingPOST({
        DmeInitConfigReq: this.getParams()
      })
      .subscribe(() => {
        this.viewSettingFlag = true;
        this.initDme();
      });
  }
}
