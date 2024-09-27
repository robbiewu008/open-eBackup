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
import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { ProtectFilterComponent } from 'app/shared/components/protect-filter/protect-filter.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  filter,
  find,
  includes,
  isArray,
  isEmpty,
  isNumber,
  isString,
  map,
  reject
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-advanced-parameter',
  templateUrl: './advanced-parameter.component.html',
  styleUrls: ['./advanced-parameter.component.less']
})
export class AdvancedParameterComponent implements OnInit {
  resourceData;
  resourceType;
  selectedNode;
  hostOptions = [];
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  dataMap = DataMap;
  scriptErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };

  @ViewChild(ProtectFilterComponent, { static: false })
  ProtectFilterComponent: ProtectFilterComponent;

  constructor(
    public fb: FormBuilder,
    private appUtilsService: AppUtilsService,
    private i18n: I18NService,
    public message: MessageService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
    this.updateData();
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.Kubernetes.value}Plugin`]
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

  updateData() {
    if (!this.resourceData.protectedObject?.extParameters) {
      return;
    }
    const extParameters = isString(
      this.resourceData.protectedObject?.extParameters
    )
      ? JSON.parse(this.resourceData.protectedObject?.extParameters)
      : this.resourceData.protectedObject?.extParameters;
    if (!isEmpty(extParameters.resource_filters)) {
      defer(() =>
        this.ProtectFilterComponent.setFilter(extParameters.resource_filters)
      );
    }
    assign(extParameters, {
      proxyHost: extParameters.agents?.split(';') || [],
      before_protect_script: extParameters.pre_script
        ? extParameters.pre_script
        : '',
      after_protect_script: extParameters.post_script
        ? extParameters.post_script
        : '',
      protect_failed_script: extParameters.failed_script
        ? extParameters.failed_script
        : '',
      slaOverwrite: extParameters.overwrite || false,
      slaPolicy: extParameters.binding_policy || [
        'APPLY_TO_ALL',
        'APPLY_TO_NEW'
      ]
    });
    this.formGroup.patchValue(extParameters);
    setTimeout(() => {
      this.valid$.next(this.formGroup.valid);
    }, 500);
  }

  initForm() {
    this.formGroup = this.fb.group({
      proxyHost: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      before_protect_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.statefulsetScript,
            false
          )
        ]
      }),
      after_protect_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.statefulsetScript,
            false
          )
        ]
      }),
      protect_failed_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.statefulsetScript,
            false
          )
        ]
      }),
      slaOverwrite: new FormControl(false),
      slaPolicy: new FormControl(['APPLY_TO_ALL', 'APPLY_TO_NEW'])
    });
    this.formGroup.statusChanges.subscribe(() => {
      this.valid$.next(this.formGroup.valid);
    });
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
  }

  onOK() {
    const ext_parameters = {};
    assign(ext_parameters, {
      agents:
        this.formGroup.value.proxyHost
          ?.filter(item => {
            return includes(map(this.hostOptions, 'value'), item);
          })
          .join(';') || null
    });
    if (
      this.resourceType === DataMap.Resource_Type.KubernetesStatefulset.value
    ) {
      assign(ext_parameters, {
        pre_script: this.formGroup.value.before_protect_script || null
      });
      assign(ext_parameters, {
        post_script: this.formGroup.value.after_protect_script || null
      });
      assign(ext_parameters, {
        failed_script: this.formGroup.value.protect_failed_script || null
      });
    }

    if (this.resourceType === DataMap.Resource_Type.KubernetesNamespace.value) {
      const vmFilters = this.ProtectFilterComponent.getAllFilters();
      assign(ext_parameters, {
        resource_filters: !isEmpty(vmFilters) ? vmFilters : null,
        overwrite: this.formGroup.value.slaOverwrite,
        binding_policy: this.formGroup.value.slaPolicy
      });
    }

    return {
      ext_parameters
    };
  }
}
