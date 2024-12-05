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
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  ClientManagerApiService,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedEnvironmentApiService,
  ResourceType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  differenceBy,
  each,
  filter,
  find,
  get,
  includes,
  isArray,
  isEmpty,
  map,
  reduce,
  replace,
  set,
  toNumber
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-hyper-v-register',
  templateUrl: './register.component.html',
  styleUrls: ['./register.component.less']
})
export class RegisterComponent implements OnInit {
  item;
  formGroup: FormGroup;
  proxyOptions = [];
  typeOptions = this.dataMapService.toArray('Resource_Type').filter(item => {
    item.isLeaf = true;
    return includes(
      [
        DataMap.Resource_Type.hyperVScvmm.value,
        DataMap.Resource_Type.hyperVHost.value,
        DataMap.Resource_Type.hyperVCluster.value
      ],
      item.value
    );
  });
  hideUserInfo = false;
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_name_label'),
    invalidSameName: this.i18n.get('common_duplicate_name_label')
  };
  userNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  dataMap = DataMap;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private modal: ModalRef,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private clientManagerApiService: ClientManagerApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        pluginType: `${DataMap.Resource_Type.hyperVVm.value}Plugin`
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        const hostArray = [];
        const filterArr = filter(
          resource,
          item => item.osType === DataMap.Os_Type.windows.value
        );
        each(filterArr, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(this.item?.subType || '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      name: new FormControl(this.item?.name || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      agent: new FormControl(this.item?.dependencies?.agents[0]?.uuid || '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      agents: new FormControl(
        this.item?.dependencies?.agents.map(item => item.uuid) || []
      ),
      username: new FormControl(this.item?.auth?.authKey || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      password: new FormControl(this.item?.auth?.authPwd || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      })
    });
    this.listenForm();
    if (this.item) {
      // 手动触发type监听，然后会更新status
      this.formGroup.get('type').setValue(this.item.subType);
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => {
      const modalIns = this.modal.getInstance();
      modalIns.lvOkDisabled = this.formGroup.invalid;
    });
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      if (res === DataMap.Resource_Type.hyperVHost.value) {
        this.hideUserInfo = true;
        this.formGroup.get('username').clearValidators();
        this.formGroup.get('password').clearValidators();
        this.formGroup.get('agents').clearValidators();
      } else if (res === DataMap.Resource_Type.hyperVCluster.value) {
        this.hideUserInfo = false;
        this.formGroup
          .get('username')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]);
        this.formGroup
          .get('password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]);
        this.formGroup
          .get('agents')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('agent').clearValidators();
      } else {
        this.hideUserInfo = false;
        this.formGroup
          .get('username')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]);
        this.formGroup
          .get('password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]);
        this.formGroup
          .get('agent')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('agents').clearValidators();
      }
      this.formGroup.get('username').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
      this.formGroup.get('agents').updateValueAndValidity();
      this.formGroup.get('agent').updateValueAndValidity();
    });
  }
  getAllEndpoint() {
    let arr = [];
    reduce(
      this.formGroup.value.agents,
      (acc, item) => {
        acc.push(find(this.proxyOptions, { value: item })?.endpoint);
        return acc;
      },
      arr
    );
    return arr.join(',');
  }
  getEndpoint() {
    if (
      this.formGroup.get('type').value ===
      DataMap.Resource_Type.hyperVCluster.value
    ) {
      return this.getAllEndpoint();
    } else {
      return find(this.proxyOptions, { value: this.formGroup.value.agent })
        ?.endpoint;
    }
  }

  getParams() {
    let reduceAgents = [];
    if (this.item) {
      if (
        this.formGroup.get('type').value ===
        DataMap.Resource_Type.hyperVCluster.value
      ) {
        reduceAgents = differenceBy(
          this.item?.dependencies?.agents.map(item => item.uuid),
          this.formGroup.value.agents
        );
      } else {
        reduceAgents = differenceBy(
          this.item?.dependencies?.agents.map(item => item.uuid),
          [this.formGroup.value.agent]
        );
      }
    }
    return {
      name: this.formGroup.value.name,
      type: ResourceType.Virtualization,
      subType: this.formGroup.value.type,
      endpoint: this.getEndpoint(),
      extendInfo: {
        targetType: replace(this.formGroup.value.type, 'HyperV.', '')
      },
      auth: {
        authType: 1,
        authKey: this.hideUserInfo ? '' : this.formGroup.value.username,
        authPwd: this.hideUserInfo ? '' : this.formGroup.value.password
      },
      dependencies: {
        agents: map(
          this.formGroup.value.type ===
            DataMap.Resource_Type.hyperVCluster.value
            ? this.formGroup.value.agents
            : [this.formGroup.value.agent],
          item => {
            return { uuid: item };
          }
        ),
        '-agents': !isEmpty(this.item)
          ? reduceAgents.map(item => {
              return { uuid: item };
            })
          : []
      }
    };
  }

  onOK(): Observable<void> {
    const params = this.getParams();
    return new Observable<void>((observer: Observer<void>) => {
      if (!isEmpty(this.item)) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            UpdateProtectedEnvironmentRequestBody: params,
            envId: this.item.uuid
          })
          .subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params
          })
          .subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      }
    });
  }
}
