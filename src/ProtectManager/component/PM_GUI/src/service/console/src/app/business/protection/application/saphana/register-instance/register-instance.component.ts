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
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  InstanceType,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { filter, find, get, map, set } from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-register-instance',
  templateUrl: './register-instance.component.html',
  styleUrls: ['./register-instance.component.less']
})
export class RegisterInstanceComponent implements OnInit {
  item;
  dataDetail;
  optsConfig;
  optItems = [];
  hostOptions = [];
  authOptions = this.dataMapService.toArray('saphanaAuthMethod').map(item => {
    item['isLeaf'] = true;
    return item;
  });
  unitOptions = this.dataMapService
    .toArray('Detecting_During_Unit')
    .filter(item => {
      item.isLeaf = true;
      return [
        DataMap.Detecting_During_Unit.second.value,
        DataMap.Detecting_During_Unit.minute.value,
        DataMap.Detecting_During_Unit.hour.value
      ].includes(item.value);
    });
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
  instanceIdErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  hostsErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_host_number_least_2_label')
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
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('protection_logbackup_interval_range_label')
  };

  @Input() rowData;
  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private clientManagerApiService: ClientManagerApiService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.name()]
      }),
      instanceId: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      host: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      authMode: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      userName: new FormControl(''),
      password: new FormControl(''),
      port: new FormControl(''),
      hdbuserstoreKey: new FormControl(''),
      logBackup: new FormControl(false),
      logBackupInterval: new FormControl(1),
      logBackupUnit: new FormControl(
        DataMap.Detecting_During_Unit.second.value
      ),
      logBackupPath: new FormControl('')
    });

    this.watchFormGroup();

    if (this.rowData) {
      this.getDataDetail();
    }
  }

  watchFormGroup() {
    this.formGroup.get('authMode').valueChanges.subscribe(res => {
      if (res === DataMap.saphanaAuthMethod.db.value) {
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
        this.formGroup
          .get('port')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]);
        this.formGroup.get('hdbuserstoreKey').clearValidators();
      } else if (res === DataMap.saphanaAuthMethod.hdbuserstore.value) {
        this.formGroup.get('userName').clearValidators();
        this.formGroup.get('password').clearValidators();
        this.formGroup.get('port').clearValidators();
        this.formGroup
          .get('hdbuserstoreKey')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]);
      }

      this.formGroup.get('userName').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
      this.formGroup.get('port').updateValueAndValidity();
      this.formGroup.get('hdbuserstoreKey').updateValueAndValidity();
    });

    this.formGroup.get('logBackup').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('logBackupInterval')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.integer(),
            this.getTimeRangeValid()
          ]);
        this.formGroup
          .get('logBackupPath')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]);
      } else {
        this.formGroup.get('logBackupInterval').clearValidators();
        this.formGroup.get('logBackupPath').clearValidators();
      }

      this.formGroup.get('logBackupInterval').updateValueAndValidity();
      this.formGroup.get('logBackupPath').updateValueAndValidity();
    });

    this.formGroup.get('logBackupUnit').valueChanges.subscribe(res => {
      if (!this.formGroup.value.logBackup) {
        return;
      }

      this.formGroup
        .get('logBackupInterval')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.getTimeRangeValid(res)
        ]);
      this.formGroup.get('logBackupInterval').updateValueAndValidity();
    });
  }

  integer(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      return !/^[1-9]\d*$/.test(control.value) && '0' !== control.value
        ? { invalidInteger: { value: control.value } }
        : null;
    };
  }

  getTimeRangeValid(val?) {
    const ONE_MONTH = 30;
    const UNIT = val || this.formGroup.value.logBackupUnit;
    switch (UNIT) {
      case DataMap.Detecting_During_Unit.second.value:
        return this.baseUtilService.VALID.rangeValue(1, ONE_MONTH * 24 * 3600);
      case DataMap.Detecting_During_Unit.minute.value:
        return this.baseUtilService.VALID.rangeValue(1, ONE_MONTH * 24 * 60);
      case DataMap.Detecting_During_Unit.hour.value:
        return this.baseUtilService.VALID.rangeValue(1, ONE_MONTH * 24);
      default:
        return this.baseUtilService.VALID.rangeValue(1, ONE_MONTH * 24);
    }
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
          instanceId: get(res, 'extendInfo.systemId'),
          host: map(get(res, 'dependencies.agents'), 'uuid'),
          authMode: get(res, 'auth.authType'),
          userName: get(res, 'auth.authKey', ''),
          password: get(res, 'auth.authPwd', ''),
          port: get(res, 'extendInfo.systemDbPort', ''),
          hdbuserstoreKey: get(res, 'auth.extendInfo.hdbUserStoreKey', ''),
          logBackup: get(res, 'extendInfo.enableLogBackup', '') === 'true',
          logBackupInterval: get(logBackup, 'logBackupInterval', ''),
          logBackupUnit:
            get(logBackup, 'logBackupIntervalUnit') ||
            DataMap.Detecting_During_Unit.second.value,
          logBackupPath: get(logBackup, 'logBackupPath', '')
        });
        this.formGroup.get('instanceId').disable();
        this.dataDetail = res;
      });
  }

  getProxyOptions() {
    const extParams = {
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      conditions: JSON.stringify({
        pluginType: `${DataMap.Resource_Type.saphanaInstance.value}Plugin`
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        let filterResource = resource;
        if (!this.rowData) {
          filterResource = filter(
            resource,
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
        }
        this.hostOptions = map(filterResource, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          };
        });
      }
    );
  }

  getParams() {
    const deletedNode = filter(
      get(this.dataDetail, 'dependencies.agents'),
      item => !find(this.formGroup.value.host, val => val === item.uuid)
    );
    const params = {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.saphanaInstance.value,
      extendInfo: {
        isTopInstance: InstanceType.TopInstance,
        systemId: this.formGroup.get('instanceId').value,
        enableLogBackup: this.formGroup.value.logBackup,
        logBackupExtInfo: JSON.stringify({
          logBackupInterval: this.formGroup.value.logBackup
            ? this.formGroup.value.logBackupInterval
            : '',
          logBackupIntervalUnit: this.formGroup.value.logBackup
            ? this.formGroup.value.logBackupUnit
            : '',
          logBackupPath: this.formGroup.value.logBackup
            ? this.formGroup.value.logBackupPath
            : ''
        })
      },
      dependencies: {
        agents: map(this.formGroup.value.host, item => {
          return {
            uuid: item
          };
        }),
        '-agents': map(deletedNode, item => {
          return {
            uuid: item.uuid
          };
        })
      }
    };

    if (this.formGroup.value.authMode === DataMap.saphanaAuthMethod.db.value) {
      set(params, 'auth', {
        authType: this.formGroup.value.authMode,
        authKey: this.formGroup.value.userName,
        authPwd: this.formGroup.value.password
      });
      set(params, 'extendInfo.systemDbPort', this.formGroup.value.port);
    } else {
      set(params, 'auth', {
        authType: this.formGroup.value.authMode,
        extendInfo: {
          hdbUserStoreKey: this.formGroup.value.hdbuserstoreKey
        }
      });
    }

    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.rowData) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.rowData.uuid,
            UpdateProtectedEnvironmentRequestBody: params
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
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params
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
