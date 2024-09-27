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
  ClusterEnvironment,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  getMultiHostOps
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  difference,
  each,
  filter,
  includes,
  isEmpty,
  map
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { MultiCluster } from 'app/shared';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';
@Component({
  selector: 'aui-register',
  templateUrl: './register.component.html',
  styleUrls: ['./register.component.less']
})
export class RegisterComponent implements OnInit {
  rowData: any;
  formGroup: FormGroup;
  typeOptions = this.dataMapService
    .toArray('oracleClusterType')
    .filter(item => {
      return (item.isLeaf = true);
    });
  hostOptions: any = [];
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_name_label'),
    invalidSameName: this.i18n.get('common_duplicate_name_label')
  };
  nodeErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };

  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getHost();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(this.rowData?.name || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      type: new FormControl(this.rowData?.extendInfo?.clusterType || '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      node: new FormControl(
        map(this.rowData?.dependencies?.agents, 'uuid') || [],
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      )
    });
  }

  getHost() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.oracle.value}Plugin`]
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
    let reduceAgents = [];
    if (!isEmpty(this.rowData)) {
      reduceAgents = difference(
        this.rowData?.dependencies?.agents.map(item => item.uuid),
        this.formGroup.value.node
      );
    }
    const params = {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: ClusterEnvironment.oralceClusterEnv,
      parentUuid: this.formGroup.value.host,
      dependencies: {
        agents: map(this.formGroup.value.node, item => {
          return { uuid: item };
        })
      },
      extendInfo: {
        clusterType: this.formGroup.value.type
      }
    };
    if (!isEmpty(this.rowData)) {
      assign(params.dependencies, {
        '-agents': reduceAgents.map(item => {
          return { uuid: item };
        })
      });
    }
    return params;
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const params = this.getParams();
      if (!isEmpty(this.rowData)) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.rowData.uuid,
            UpdateProtectedEnvironmentRequestBody: params
          })
          .subscribe({
            next: res => {
              observer.next(res);
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next(res);
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      }
    });
  }
}
