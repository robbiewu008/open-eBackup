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
import {
  BaseUtilService,
  DataMap,
  ProtectedResourceApiService
} from 'app/shared';
import { ProtectFilterComponent } from 'app/shared/components/protect-filter/protect-filter.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  filter,
  includes,
  isArray,
  isEmpty,
  isString,
  map,
  reject
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-advanced',
  templateUrl: './advanced.component.html',
  styleUrls: ['./advanced.component.less']
})
export class AdvancedComponent implements OnInit {
  resourceData;
  resourceType;
  selectedNode;
  hostOptions = [];
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  dataMap = DataMap;
  hasArchive = false;
  extParams;
  @ViewChild(ProtectFilterComponent, { static: false })
  ProtectFilterComponent: ProtectFilterComponent;

  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getProxyOptions();
    this.updateData();
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [
          `${DataMap.globalResourceType.openStackContainer.value}Plugin`
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
        // 过滤1.2.1版本
        resource = this.baseUtilService.rejectAgentsByVersion(
          resource,
          '1.2.1'
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
      }
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      proxyHost: new FormControl([]),
      open_consistent_snapshots: new FormControl(false),
      slaOverwrite: new FormControl(false),
      slaPolicy: new FormControl(['APPLY_TO_ALL', 'APPLY_TO_NEW'])
    });
    this.formGroup.statusChanges.subscribe(() => {
      this.valid$.next(this.formGroup.valid);
    });
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
      open_consistent_snapshots:
        extParameters.open_consistent_snapshots === 'true',
      slaOverwrite: extParameters.overwrite || false,
      slaPolicy: extParameters.binding_policy || [
        'APPLY_TO_ALL',
        'APPLY_TO_NEW'
      ]
    });
    this.extParams = extParameters;
    this.formGroup.patchValue(extParameters);
    setTimeout(() => {
      this.valid$.next(this.formGroup.valid);
    }, 500);
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
          .join(';') || null,
      open_consistent_snapshots: `${this.formGroup.value.open_consistent_snapshots}`
    });

    if (this.resourceType === DataMap.Resource_Type.openStackProject.value) {
      const cloudFilters = this.ProtectFilterComponent.getAllFilters();
      assign(ext_parameters, {
        resource_filters: !isEmpty(cloudFilters) ? cloudFilters : null,
        overwrite: this.formGroup.value.slaOverwrite,
        binding_policy: this.formGroup.value.slaPolicy
      });
    } else {
      each(
        [
          'backup_res_auto_index',
          'archive_res_auto_index',
          'enable_security_archive'
        ],
        key => {
          if (this.formGroup.get(key)) {
            assign(ext_parameters, {
              [key]: this.formGroup.get(key).value
            });
          }
        }
      );
    }

    return {
      ext_parameters
    };
  }
}
