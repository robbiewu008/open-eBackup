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
  DataMap,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { BaseUtilService, I18NService } from 'app/shared/services';
import { assign, map } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-auth-info',
  templateUrl: './auth.component.html',
  styleUrls: ['./auth.component.less']
})
export class AuthComponent implements OnInit {
  data;
  formGroup: FormGroup;
  nameErrorTip = assign(this.baseUtilService.nameErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [60])
  });
  pwdErrorTip = assign(this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [60])
  });
  asmAuthType = DataMap.Database_Auth_Method;
  authTypeOptions = [
    {
      key: DataMap.Database_Auth_Method.db.value,
      value: DataMap.Database_Auth_Method.db.value,
      label: this.i18n.get('protection_database_auth_label'),
      isLeaf: true
    },
    {
      key: DataMap.Database_Auth_Method.os.value,
      value: DataMap.Database_Auth_Method.os.value,
      label: this.i18n.get('protection_os_auth_label'),
      isLeaf: true
    }
  ];

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      uuid: new FormControl(this.data.uuid),
      auth_type: new FormControl(
        this.data.auth?.authType || DataMap.Database_Auth_Method.db.value,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      db_username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(60)
        ]
      }),
      db_password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(60)
        ]
      })
    });
    this.formGroup.get('auth_type').valueChanges.subscribe(res => {
      if (res === DataMap.Database_Auth_Method.db.value) {
        this.formGroup.addControl(
          'db_username',
          new FormControl('', {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(60)
            ]
          })
        );
        this.formGroup.addControl(
          'db_password',
          new FormControl('', {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(60)
            ]
          })
        );
      } else {
        this.formGroup.removeControl('db_username');
        this.formGroup.removeControl('db_password');
      }
      this.formGroup.updateValueAndValidity();
    });
  }

  getParams() {
    const params = {
      name: this.data.name,
      type: ResourceType.DATABASE,
      subType: this.data.subType,
      parentUuid: this.data.environment?.uuid,
      dependencies: {
        agents:
          this.data.subType === DataMap.oracleType.single.value
            ? [{ uuid: this.data.environment?.uuid }]
            : map(this.data?.dependencies?.agents, item => {
                return { uuid: item.uuid };
              })
      },
      extendInfo: {
        hostId: this.data.environment?.uuid,
        installUsername: this.data.extendInfo?.installUsername
      },
      auth: {
        authType: this.formGroup.value.auth_type,
        authKey:
          this.formGroup.value.auth_type ===
          DataMap.Database_Auth_Method.db.value
            ? this.formGroup.value.db_username
            : '',
        authPwd:
          this.formGroup.value.auth_type ===
          DataMap.Database_Auth_Method.db.value
            ? this.formGroup.value.db_password
            : '',
        extendInfo: {}
      }
    };
    if (this.data.subType === DataMap.oracleType.cluster.value) {
      delete params.parentUuid;
    }
    return params;
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const params = this.getParams();
      if (this.data.subType === DataMap.oracleType.single.value) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.data?.uuid,
            UpdateResourceRequestBody: params
          })
          .subscribe(
            res => {
              observer.next(res);
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      } else {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.data.uuid,
            UpdateProtectedEnvironmentRequestBody: params
          })
          .subscribe(
            res => {
              observer.next(res);
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      }
    });
  }
}
