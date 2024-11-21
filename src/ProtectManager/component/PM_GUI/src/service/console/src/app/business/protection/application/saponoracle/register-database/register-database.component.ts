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
import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  AppService,
  BaseUtilService,
  DataMap,
  DataMapService,
  getMultiHostOps,
  I18NService,
  InstanceType,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { each, filter, find, get, isEmpty, isNumber, map, set } from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { AppUtilsService } from '../../../../../shared/services/app-utils.service';

@Component({
  selector: 'aui-saponoracle-register-database',
  templateUrl: './register-database.component.html',
  styleUrls: ['./register-database.component.less']
})
export class SaponoracleRegisterDatabaseComponent implements OnInit {
  item;
  dataDetail;
  optsConfig;
  hostOptions = [];
  dataMap = DataMap;
  tableData = {
    data: [],
    total: 0
  };
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  authKeyErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  @Input() rowData;
  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private appService: AppService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getHost();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.name()]
      }),
      sapsId: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      oraclesId: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      host: new FormControl([]),
      authType: new FormControl(DataMap.Database_Auth_Method.db.value), // 接口需要，页面隐藏
      authKey: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      authPwd: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      })
    });
    if (this.rowData) {
      this.getDataDetail();
    }
  }

  getHost() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.saponoracleDatabase.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            item.environment.extendInfo.scenario ===
            DataMap.proxyHostType.external.value
        );
        if (MultiCluster.isMulti) {
          resource = getMultiHostOps(resource);
        } else {
          resource = filter(
            resource,
            item =>
              item.environment.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
        }
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true
          });
        });
        this.hostOptions = hostArray;
      }
    );
  }

  getParams() {
    const params = {
      name: this.formGroup.get('name').value,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.saponoracleDatabase.value,
      parentUuid: this.formGroup.get('host').value,
      extendInfo: {
        hostId: this.formGroup.get('host').value,
        sapsid: this.formGroup.get('sapsId').value,
        oraclesid: this.formGroup.get('oraclesId').value
      },
      auth: {
        authType: this.formGroup.value.authType,
        authKey: this.formGroup.value.authKey,
        authPwd: this.formGroup.value.authPwd
      },
      dependencies: {
        agents: [
          {
            uuid: this.formGroup.get('host').value
          }
        ]
      }
    };
    return params;
  }

  getDataDetail() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe(res => {
        const logBackup = JSON.parse(
          get(res, 'extendInfo.logBackupExtInfo', '{}')
        );
        this.formGroup.patchValue({
          name: res.name,
          host: get(res, 'extendInfo.hostId', ''),
          sapsId: get(res, 'extendInfo.sapsid', ''),
          oraclesId: get(res, 'extendInfo.oraclesid', ''),
          authType: get(res, 'auth.authType'),
          authKey: get(res, 'auth.authKey', ''),
          authPwd: get(res, 'auth.authPwd', '')
        });
        this.formGroup.get('sapsId').disable();
        this.formGroup.get('oraclesId').disable();
        this.dataDetail = res;
      });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.rowData) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowData.uuid,
            UpdateResourceRequestBody: params
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      } else {
        this.protectedResourceApiService
          .CreateResource({
            CreateResourceRequestBody: params
          })
          .subscribe(
            res => {
              observer.next();
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
