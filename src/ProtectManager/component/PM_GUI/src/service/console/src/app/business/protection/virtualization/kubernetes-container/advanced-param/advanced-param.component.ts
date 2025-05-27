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
import { DataMap, I18NService, ProtectedResourceApiService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  isArray,
  isString,
  assign,
  filter,
  includes,
  reject,
  isEmpty,
  each,
  map
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-advanced-param',
  templateUrl: './advanced-param.component.html',
  styleUrls: ['./advanced-param.component.less']
})
export class AdvancedParamComponent implements OnInit {
  resourceData;
  resourceType;
  formGroup: FormGroup;
  dataMap = DataMap;

  backupHelp = this.i18n.get('protetion_kubernetes_advanced_help_label');

  valid$ = new Subject<boolean>();
  hostOptions = [];

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
    this.updateForm();
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [
          `${DataMap.Resource_Type.kubernetesClusterCommon.value}Plugin`
        ]
      })
    };
    if (!this.resourceData.sla_id) {
      assign(extParams.conditions, {
        environment: {
          linkStatus: [['=='], DataMap.resource_LinkStatus_Special.normal.value]
        }
      });
    }
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        let bindAgents;
        try {
          bindAgents = this.resourceData.protectedObject?.extParameters?.agents?.split(
            ';'
          );
        } catch (error) {
          bindAgents = [];
        }
        resource = reject(
          resource,
          item =>
            item.environment.linkStatus !==
              DataMap.resource_LinkStatus_Special.normal.value &&
            !includes(bindAgents, item.environment.uuid)
        );
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              linkStatus: tmp.linkStatus,
              isLeaf: true
            });
          }
        });
        this.hostOptions = hostArray;
        if (!isEmpty(this.formGroup.value.proxyHost)) {
          this.formGroup
            .get('proxyHost')
            .setValue(
              filter(this.formGroup.value.proxyHost, item =>
                includes(map(hostArray, 'value'), item)
              )
            );
        }
      }
    );
  }

  backupHelpHover() {
    const url = this.i18n.isEn
      ? '/console/assets/help/a8000/en-us/index.html#en-us_topic_0000002164589986.html'
      : '/console/assets/help/a8000/zh-cn/index.html#kubernetes_CSI_00108.html';
    this.appUtilsService.openSpecialHelp(url);
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
  }

  initForm() {
    this.formGroup = this.fb.group({
      is_consistent: new FormControl(false),
      proxyHost: new FormControl([])
    });
    this.listenForm();
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(() =>
      this.valid$.next(this.formGroup.valid)
    );
  }

  updateForm() {
    if (!this.resourceData.protectedObject?.extParameters) {
      return;
    }
    const extParameters = isString(
      this.resourceData.protectedObject?.extParameters
    )
      ? JSON.parse(this.resourceData.protectedObject?.extParameters)
      : this.resourceData.protectedObject?.extParameters;
    this.formGroup.patchValue({
      is_consistent: extParameters.is_consistent,
      proxyHost: extParameters?.agents?.split(';') || []
    });
  }

  onOK() {
    const ext_parameters = {
      is_consistent: this.formGroup.value.is_consistent,
      agents:
        this.formGroup.value.proxyHost
          ?.filter(item => {
            return includes(map(this.hostOptions, 'value'), item);
          })
          .join(';') || null
    };

    return {
      ext_parameters
    };
  }
}
