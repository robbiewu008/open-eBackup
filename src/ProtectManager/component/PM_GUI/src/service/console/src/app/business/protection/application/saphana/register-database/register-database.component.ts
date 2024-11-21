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
import { each, filter, find, get, isNumber, map, set } from 'lodash';
import {
  concatMap,
  from,
  Observable,
  Observer,
  of,
  Subject,
  takeWhile
} from 'rxjs';

@Component({
  selector: 'aui-register-database',
  templateUrl: './register-database.component.html',
  styleUrls: ['./register-database.component.less']
})
export class RegisterDatabaseComponent implements OnInit {
  item;
  dataDetail;
  optsConfig;
  optItems = [];
  instanceOptions = [];
  hostOptions = [];
  authOptions = this.dataMapService.toArray('saphanaAuthMethod').map(item => {
    item['isLeaf'] = true;
    return item;
  });
  instanceDatabaseOpts = [];
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
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };

  @Input() rowData;
  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private appService: AppService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getInstanceOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.saphanaDatabaseType.systemdb.value),
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.name()]
      }),
      instance: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      host: new FormControl([]),
      authMode: new FormControl(''),
      userName: new FormControl(''),
      password: new FormControl(''),
      port: new FormControl(''),
      hdbuserstoreKey: new FormControl('')
    });

    this.watchFormGroup();

    if (this.rowData) {
      this.getDataDetail();
    }
  }

  watchFormGroup() {
    this.formGroup.get('type').valueChanges.subscribe(res => {
      this.formGroup.get('instance').setValue('');
      this.instanceDatabaseOpts = [];
      if (res === DataMap.saphanaDatabaseType.systemdb.value) {
        this.formGroup.get('host').clearValidators();
        this.formGroup.get('authMode').clearValidators();
      } else {
        this.formGroup
          .get('host')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('authMode')
          .setValidators([this.baseUtilService.VALID.required()]);
      }

      this.formGroup.get('host').updateValueAndValidity();
      this.formGroup.get('authMode').updateValueAndValidity();
      this.formGroup.get('userName').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
      this.formGroup.get('hdbuserstoreKey').updateValueAndValidity();
    });
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
        this.formGroup.get('hdbuserstoreKey').clearValidators();
        this.formGroup
          .get('port')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]);
      } else if (res === DataMap.saphanaAuthMethod.hdbuserstore.value) {
        this.formGroup.get('userName').clearValidators();
        this.formGroup.get('password').clearValidators();
        this.formGroup
          .get('hdbuserstoreKey')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]);
        this.formGroup.get('port').clearValidators();
      }

      if (
        this.formGroup.value.type === DataMap.saphanaDatabaseType.systemdb.value
      ) {
        this.formGroup.get('userName').clearValidators();
        this.formGroup.get('password').clearValidators();
        this.formGroup.get('hdbuserstoreKey').clearValidators();
        this.formGroup.get('port').clearValidators();
      }

      this.formGroup.get('userName').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
      this.formGroup.get('hdbuserstoreKey').updateValueAndValidity();
      this.formGroup.get('port').updateValueAndValidity();
    });

    this.formGroup.get('instance').valueChanges.subscribe(res => {
      this.formGroup.get('host').setValue([]);
      this.hostOptions = [];

      if (res) {
        this.getProxyOptions(res);
        this.getDatabaseNameByInstanceId(res);
      }
    });
  }

  getDataDetail() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe(res => {
        const databaseType = get(res, 'extendInfo.sapHanaDbType');
        if (databaseType === DataMap.saphanaDatabaseType.systemdb.value) {
          this.formGroup.patchValue({
            type: databaseType,
            name: res.name,
            instance: get(res, 'parentUuid')
          });
        } else {
          this.formGroup.patchValue({
            type: databaseType,
            name: res.name,
            instance: get(res, 'parentUuid'),
            host: map(get(res, 'dependencies.agents'), 'uuid'),
            authMode: get(res, 'auth.authType'),
            userName: get(res, 'auth.authKey', ''),
            password: get(res, 'auth.authPwd', ''),
            port: get(res, 'extendInfo.systemDbPort', ''),
            hdbuserstoreKey: get(res, 'auth.extendInfo.hdbUserStoreKey', '')
          });
        }
        this.formGroup.get('name').disable();
        this.dataDetail = res;
      });
  }

  getInstanceOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      queryDependency: true,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.saphanaInstance.value,
        isTopInstance: InstanceType.TopInstance
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        const instanceArray = [];
        each(recordsTemp, item => {
          instanceArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.instanceOptions = instanceArray;
        return;
      }
      this.getInstanceOptions(recordsTemp, startPage);
    });
  }

  getProxyOptions(res) {
    this.protectedResourceApiService
      .ShowResource({ resourceId: res })
      .subscribe((res: any) => {
        const agents = get(res, 'dependencies.agents');
        const hostArray = [];
        each(agents, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item?.name}(${item?.endpoint})`,
            isLeaf: true
          });
        });
        this.hostOptions = hostArray;
      });
  }

  getDatabaseNameByInstanceId(instanceId: string) {
    this.instanceDatabaseOpts = [];
    let agents = [];
    let found = false; // 用于跟踪流程是否结束
    let targetInstance;
    if (this.rowData) {
      agents = map(JSON.parse(this.rowData.extendInfo.nodes), 'uuid');
      targetInstance = this.rowData.environment;
    } else {
      targetInstance = find(this.instanceOptions, { value: instanceId });
      agents = map(get(targetInstance, 'dependencies.agents', []), 'uuid');
    }
    from(agents)
      .pipe(
        concatMap(agentId => {
          if (found) {
            return of(null);
          }
          return this.appService
            .ListResourcesDetails(
              this.getRequestParam(agentId, targetInstance.uuid)
            )
            .pipe(
              takeWhile(response => {
                if (
                  response.records.length > 0 &&
                  response.records[0].extendInfo
                ) {
                  found = true;
                  this.formatDatabaseOpts(response.records[0]);
                  return false;
                }
                return true;
              })
            );
        })
      )
      .subscribe();
  }

  getRequestParam(agentId, resourceId) {
    return {
      envId: agentId,
      agentId: agentId,
      pageNo: CommonConsts.PAGE_START_EXTRA,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      resourceIds: [resourceId],
      appType: DataMap.Resource_Type.saphanaInstance.value,
      conditions: JSON.stringify({
        queryType:
          this.formGroup.get('type').value ===
          DataMap.saphanaDatabaseType.systemdb.value
            ? 'systemDb'
            : 'notSystemDb'
      })
    };
  }

  formatDatabaseOpts(data) {
    const dbArr = JSON.parse(get(data, 'extendInfo.db', '[]'));
    this.instanceDatabaseOpts = map(dbArr, item => ({
      key: item,
      value: item,
      label: item,
      isLeaf: true
    }));
  }

  getParams() {
    const params = {
      name: this.formGroup.get('name').value,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.saphanaDatabase.value,
      parentUuid: this.formGroup.value.instance,
      extendInfo: {
        sapHanaDbType: this.formGroup.value.type
      },
      dependencies: {
        agents: []
      }
    };

    if (
      this.formGroup.value.type === DataMap.saphanaDatabaseType.tenantdb.value
    ) {
      const deletedNode = filter(
        get(this.dataDetail, 'dependencies.agents'),
        item => !find(this.formGroup.value.host, val => val === item.uuid)
      );

      set(
        params.dependencies,
        'agents',
        map(this.formGroup.value.host, item => {
          return {
            uuid: item
          };
        })
      );
      set(
        params.dependencies,
        '-agents',
        map(deletedNode, item => {
          return {
            uuid: item.uuid
          };
        })
      );

      if (
        this.formGroup.value.authMode === DataMap.saphanaAuthMethod.db.value
      ) {
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
