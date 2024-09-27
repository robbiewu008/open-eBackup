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
import { ModalRef } from '@iux/live';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { Component, OnInit, ViewChild, TemplateRef } from '@angular/core';
import { Observable, Observer } from 'rxjs';
import {
  I18NService,
  BaseUtilService,
  DataMap,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  InstanceType,
  DataMapService
} from 'app/shared';
import {
  assign,
  cloneDeep,
  each,
  find,
  includes,
  isUndefined,
  set,
  size,
  startsWith
} from 'lodash';

@Component({
  selector: 'aui-add-host',
  templateUrl: './add-host.component.html',
  styleUrls: ['./add-host.component.less']
})
export class AddHostComponent implements OnInit {
  rowData;
  name;
  parentUuid;
  data = [];
  children = [];
  isTest = false;
  okLoading = false;
  testLoading = false;
  hostOptions = [];
  formGroup: FormGroup;
  dataMap = DataMap;
  authOptions = this.dataMapService
    .toArray('Database_Auth_Method')
    .map(item => {
      item['isLeaf'] = true;
      return item;
    });

  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  textAreaErrorTips = {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1000])
  };
  constructor(
    private fb: FormBuilder,
    public modal: ModalRef,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      host: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      port: new FormControl('3306', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ],
        updateOn: 'change'
      }),
      authMode: new FormControl(DataMap.Database_Auth_Method.os.value),
      userName: new FormControl(''),
      password: new FormControl(''),
      authExtendInfo: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(1000)]
      }),
      customParams: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(1000)]
      })
    });

    this.formGroup.get('authMode').valueChanges.subscribe(res => {
      if (res === DataMap.Database_Auth_Method.db.value) {
        this.formGroup
          .get('userName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(32)
          ]);
        this.formGroup
          .get('password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(32)
          ]);
      } else {
        this.formGroup.get('userName').clearValidators();
        this.formGroup.get('password').clearValidators();
      }

      this.formGroup.get('userName').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      let index = size(this.children);
      each(this.formGroup.value.host, item => {
        const host = find(this.hostOptions, { uuid: item });
        if (this.rowData) {
          this.data = [cloneDeep(this.rowData)];

          set(this.data[0], 'auth', {
            authType: DataMap.Database_Auth_Method.db.value,
            authKey: this.formGroup.value.userName,
            authPwd: this.formGroup.value.password,
            extendInfo: { instancePort: this.formGroup.value.port }
          });
          this.data[0].port = this.formGroup.value.port;
        } else {
          this.data.push({
            uuid: item,
            name: host.name,
            type: ResourceType.DATABASE,
            extendInfo: {
              hostId: item,
              customParams: this.formGroup.value.customParams
            },
            dependencies: {
              hosts: [{ uuid: item }]
            },
            auth: {
              authType: this.formGroup.value.authMode,
              authKey:
                this.formGroup.value.authMode ===
                DataMap.Database_Auth_Method.db.value
                  ? this.formGroup.value.userName
                  : '',
              authPwd:
                this.formGroup.value.authMode ===
                DataMap.Database_Auth_Method.db.value
                  ? this.formGroup.value.password
                  : '',
              extendInfo: {
                authCustomParams:
                  this.formGroup.value.authMode ===
                  DataMap.Database_Auth_Method.db.value
                    ? this.formGroup.value.authExtendInfo
                    : ''
              }
            },
            port: this.formGroup.value.port,
            hostName: host?.name,
            endpoint: host?.endpoint,
            customParams: this.formGroup.value.customParams
          });
        }
      });
      observer.next();
      observer.complete();
    });
  }
}
