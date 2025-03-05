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
  CommonConsts
} from 'app/shared';
import {
  assign,
  cloneDeep,
  each,
  find,
  includes,
  isEqual,
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
  isEapp: boolean;
  formGroup: FormGroup;

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
  charsetErrorTip = {
    invalidName: this.baseUtilService.invalidInputLabel,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  confPathErrorTip = {
    invalidName: this.baseUtilService.invalidInputLabel,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048])
  };

  constructor(
    private fb: FormBuilder,
    public modal: ModalRef,
    public i18n: I18NService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      host: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      instanceIp: new FormControl('', {
        validators: this.isEapp
          ? [
              this.baseUtilService.VALID.ip(),
              this.baseUtilService.VALID.required()
            ]
          : [this.baseUtilService.VALID.ip()]
      }),
      port: new FormControl(this.isEapp ? '3307' : '3306', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ],
        updateOn: 'change'
      }),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      charset: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.charset, false),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      configPath: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.templatLinuxPath,
            false
          ),
          this.baseUtilService.VALID.maxLength(2048)
        ]
      }),
      toolPath: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.templatLinuxPath,
            false
          ),
          this.baseUtilService.VALID.maxLength(2048)
        ]
      }),
      libraryPath: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.templatLinuxPath,
            false
          ),
          this.baseUtilService.VALID.maxLength(2048)
        ]
      })
    });

    if (this.rowData) {
      this.formGroup.patchValue({
        host: [this.rowData.extendInfo?.hostId],
        instanceIp: this.rowData.extendInfo?.instanceIp,
        port: this.rowData.extendInfo?.instancePort,
        userName: this.rowData.auth?.authKey,
        charset: this.rowData.extendInfo?.charset,
        configPath: this.rowData.extendInfo?.myCnfPath,
        toolPath: this.rowData.extendInfo?.toolPath,
        libraryPath: this.rowData.extendInfo?.libraryPath
      });
    }
  }

  getProxyOptions() {
    const params = {
      resourceId: this.parentUuid
    };
    this.protectedResourceApiService.ShowResource(params).subscribe(res => {
      const hostArray = [];
      each(res['dependencies']['agents'], item => {
        hostArray.push({
          ...item,
          key: item.uuid,
          value: item.uuid,
          label: `${item.name}(${item.endpoint})`,
          isLeaf: true
        });
      });
      if (this.rowData) {
        this.hostOptions = [
          find(
            hostArray,
            item => item.value === this.rowData.extendInfo?.hostId
          )
        ];
        return;
      }
      if (!!size(this.children)) {
        this.hostOptions = hostArray.filter(item =>
          isUndefined(
            find(
              this.children,
              child => child.extendInfo?.hostId === item.value
            )
          )
        );
        return;
      }
      this.hostOptions = hostArray;
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

          set(
            this.data[0],
            'extendInfo.instanceIp',
            this.formGroup.value.instanceIp
          );
          set(
            this.data[0],
            'extendInfo.instancePort',
            this.formGroup.value.port
          );
          set(this.data[0], 'extendInfo.charset', this.formGroup.value.charset);
          set(
            this.data[0],
            'extendInfo.myCnfPath',
            this.formGroup.value.configPath
          );
          set(
            this.data[0],
            'extendInfo.toolPath',
            this.formGroup.value.toolPath
          );
          set(
            this.data[0],
            'extendInfo.libraryPath',
            this.formGroup.value.libraryPath
          );
          this.data[0].port = this.formGroup.value.port;
        } else {
          let name = `${this.name}${index++}`;

          while (!!find(this.children, item => startsWith(item.name, name))) {
            name = `${this.name}${index++}`;
          }

          this.data.push({
            parentUuid: '',
            name: `${name}(${host?.endpoint})`,
            type: ResourceType.DATABASE,
            subType: DataMap.Resource_Type.MySQLInstance.value,
            extendInfo: {
              hostId: item,
              instanceIp: this.formGroup.value.instanceIp,
              instancePort: this.formGroup.value.port,
              isTopInstance: InstanceType.NotTopinstance,
              charset: this.formGroup.value.charset,
              myCnfPath: this.formGroup.value.configPath,
              toolPath: this.formGroup.value.toolPath,
              libraryPath: this.formGroup.value.libraryPath
            },
            dependencies: {
              agents: [{ uuid: item }]
            },
            auth: {
              authType: DataMap.Database_Auth_Method.db.value,
              authKey: this.formGroup.value.userName,
              authPwd: this.formGroup.value.password,
              extendInfo: { instancePort: this.formGroup.value.port }
            },
            port: this.formGroup.value.port,
            hostName: host?.name,
            ip: host?.endpoint
          });
        }
      });
      observer.next();
      observer.complete();
    });
  }
}
