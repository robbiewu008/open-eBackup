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
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  MultiCluster,
  ClientManagerApiService,
  getMultiHostOps
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  get,
  has,
  isNumber,
  map,
  set,
  size,
  toString as _toString,
  isUndefined,
  isEmpty
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-register-cluster',
  templateUrl: './register-cluster.component.html',
  styleUrls: ['./register-cluster.component.less']
})
export class RegisterClusterComponent implements OnInit {
  dataDetails;
  proxyOptions = [];
  dataMap = DataMap;
  testLoading = false;
  formGroup: FormGroup;
  clusterAgentOptions = [];
  valid$ = new Subject<boolean>();
  tempPath = '/opt/huawei/Bigdata/mppdb/.mppdbgs_profile';

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048]),
    invalidPath: this.i18n.get('common_path_error_label'),
    samePathError: this.i18n.get('protection_same_path_error_label')
  };

  @Input() rowData;
  @Input() clusterData;
  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      path: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(2048)
        ]
      }),
      cluster: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      agents: new FormControl([])
    });

    this.listenForm();

    if (this.rowData) {
      this.formGroup.patchValue({
        name: this.rowData.name,
        userName: this.rowData.auth.authKey,
        path: this.rowData.extendInfo.envFile
      });
    }
  }

  disableUsedHost() {
    each(this.clusterData, item => {
      if (!!this.rowData && this.rowData.uuid === item.uuid) {
        return;
      }

      this.protectedResourceApiService
        .ShowResource({ resourceId: item.uuid })
        .subscribe((res: any) => {
          let agents = [];

          if (has(res, 'dependencies.clusterAgent')) {
            agents = [...get(res, 'dependencies.clusterAgent')];
          }
          if (has(res, 'dependencies.hostAgent')) {
            agents = [...get(res, 'dependencies.hostAgent'), ...agents];
          }

          each(agents, item => {
            const clusterAgent = find(
              this.clusterAgentOptions,
              agent => agent.uuid === item.uuid
            );
            const hostAgent = find(
              this.proxyOptions,
              agent => agent.uuid === item.uuid
            );

            if (!!clusterAgent) {
              set(clusterAgent, 'disabled', true);
              set(clusterAgent, 'used', true);
              set(hostAgent, 'disabled', true);
              set(hostAgent, 'used', true);
            }
          });

          this.clusterAgentOptions = [...this.clusterAgentOptions];
          this.proxyOptions = [...this.proxyOptions];
        });
    });
  }

  getDataDetail() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe((res: any) => {
        if (has(res, 'dependencies.clusterAgent')) {
          const node = [];

          each(get(res, 'dependencies.clusterAgent'), (item: any) => {
            node.push(item.uuid);
          });
          this.formGroup.get('cluster').setValue(node);
        }

        if (has(res, 'dependencies.hostAgent')) {
          const agent = [];

          each(get(res, 'dependencies.hostAgent'), (item: any) => {
            agent.push(item.uuid);
          });
          this.formGroup.get('agents').setValue(agent);
        }

        this.dataDetails = res;
      });
  }

  listenForm() {
    this.formGroup.valueChanges.subscribe(res => {
      this.valid$.next(this.formGroup.valid);
    });

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      this.proxyOptions = map(this.proxyOptions, item => {
        const checked = find(res, val => val === item.value);

        if (has(item, 'used')) {
          return item;
        }
        if (checked) {
          return assign(item, {
            disabled: true
          });
        } else {
          return assign(item, {
            disabled: false
          });
        }
      });
    });

    this.formGroup.get('agents').valueChanges.subscribe(res => {
      this.clusterAgentOptions = map(this.clusterAgentOptions, item => {
        const checked = find(res, val => val === item.value);

        if (has(item, 'used')) {
          return item;
        }
        if (checked) {
          return assign(item, {
            disabled: true
          });
        } else {
          return assign(item, {
            disabled: false
          });
        }
      });
    });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        pluginType: `${DataMap.Resource_Type.DWS_Cluster.value}Plugin`,
        scenario: '0'
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        const hostArray = [];
        if (MultiCluster.isMulti && !this.rowData) {
          resource = getMultiHostOps(resource, true);
        }
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
        this.clusterAgentOptions = cloneDeep(this.proxyOptions);
        this.disableUsedHost();
        if (this.rowData) {
          this.getDataDetail();
        }
      }
    );
  }

  getParams() {
    const deletedCluster = filter(
      get(this.dataDetails, 'dependencies.clusterAgent'),
      item => !find(this.formGroup.value.cluster, val => val === item.uuid)
    );
    const deletedHost = filter(
      get(this.dataDetails, 'dependencies.hostAgent'),
      item => !find(this.formGroup.value.agents, val => val === item.uuid)
    );
    const params = {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.DWS_Cluster.value,
      extendInfo: {
        envFile: this.formGroup.value.path
      },
      auth: {
        authType: DataMap.Database_Auth_Method.db.value,
        authKey: this.formGroup.value.userName
      },
      dependencies: {
        clusterAgent: map(this.formGroup.value.cluster, item => {
          return {
            uuid: item
          };
        }),
        hostAgent: map(this.formGroup.value.agents, item => {
          return {
            uuid: item
          };
        })
      }
    };

    if (size(deletedCluster)) {
      set(
        params.dependencies,
        '-clusterAgent',
        map(deletedCluster, item => {
          return {
            uuid: item.uuid
          };
        })
      );
    }

    if (size(deletedHost)) {
      set(
        params.dependencies,
        '-hostAgent',
        map(deletedHost, item => {
          return {
            uuid: item.uuid
          };
        })
      );
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
            UpdateProtectedEnvironmentRequestBody: params as any
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
            RegisterProtectedEnviromentRequestBody: params as any
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
