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
  isString,
  map
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-protection-advanced',
  templateUrl: './protection-advanced.component.html',
  styleUrls: ['./protection-advanced.component.less']
})
export class ProtectionAdvancedComponent implements OnInit {
  resourceData;
  resourceType;
  proxyOptions = [];
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  dataMap = DataMap;
  vmFilterShow = false;
  hiddenProxy = false;
  overwriteSlaLable: string;
  notOverwriteSlaLable: string;
  applySlaLabel: string;
  applySlaNewLabel: string;
  extParams;

  @ViewChild(ProtectFilterComponent, { static: false })
  ProtectFilterComponent: ProtectFilterComponent;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.hiddenProxy =
      includes(
        [
          DataMap.Resource_Type.hyperVVm.value,
          DataMap.Resource_Type.hyperVHost.value
        ],
        this.resourceType
      ) ||
      (this.resourceType === DataMap.Resource_Type.vmGroup.value &&
        this?.resourceData?.sourceType === ResourceType.HYPERV);
    this.getLabel();
    this.initForm();
    this.getProxyOptions();
    this.updateData();
  }

  getProxyOptions() {
    if (this.hiddenProxy) return;

    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType:
          this.resourceType === DataMap.Resource_Type.vmGroup.value
            ? [`${this?.resourceData?.sourceType}Plugin`]
            : [`${this?.resourceData?.type}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            !isEmpty(item.environment) &&
            item.environment.extendInfo.scenario ===
              DataMap.proxyHostType.external.value
        );
        if (!this.resourceData.sla_id) {
          resource = filter(
            resource,
            item =>
              item.environment.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
        } else {
          const existAgent = this.resourceData.protectedObject?.extParameters
            ?.agents;
          resource = filter(
            resource,
            item =>
              item.environment.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value ||
              includes(existAgent, item.environment.uuid)
          );
        }
        each(resource, item => {
          hostArray.push({
            ...item.environment,
            key: item.environment.uuid,
            value: item.environment.uuid,
            label: `${item.environment.name}(${item.environment.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
    this.vmFilterShow = includes(
      [
        DataMap.Resource_Type.cNwareCluster.value,
        DataMap.Resource_Type.cNwareHost.value,
        DataMap.Resource_Type.hyperVHost.value,
        DataMap.Resource_Type.nutanixCluster.value,
        DataMap.Resource_Type.nutanixHost.value
      ],
      this.resourceType
    );
  }

  getLabel() {
    this.overwriteSlaLable = this.i18n.get('protection_overwrite_sla_label');
    this.notOverwriteSlaLable = this.i18n.get(
      'protection_not_overwrite_sla_label'
    );
    this.applySlaLabel = this.i18n.get(
      'protection_apply_sla_in_cna_vmware_label'
    );
    this.applySlaNewLabel = this.i18n.get(
      'protection_apply_sla_in_no_cna_vmware_label'
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      proxyHost: new FormControl([]),
      slaOverwrite: new FormControl(false),
      slaPolicy: new FormControl(['APPLY_TO_ALL', 'APPLY_TO_NEW'])
    });
    this.formGroup.statusChanges.subscribe(() => {
      this.valid$.next(this.formGroup.valid);
    });
  }

  updateData() {
    if (!this.resourceData?.protectedObject?.extParameters) {
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
      slaOverwrite: extParameters.overwrite || false,
      slaPolicy: extParameters.binding_policy || [
        'APPLY_TO_ALL',
        'APPLY_TO_NEW'
      ]
    });
    // 索引
    this.extParams = extParameters;
    this.formGroup.patchValue(extParameters);
    setTimeout(() => {
      this.valid$.next(this.formGroup.valid);
    }, 500);
  }

  onOK() {
    const ext_parameters = {};
    assign(ext_parameters, {
      agents:
        this.formGroup.value.proxyHost
          ?.filter(item => {
            return includes(map(this.proxyOptions, 'value'), item);
          })
          .join(';') || null
    });

    if (this.vmFilterShow) {
      const vmFilters = this.ProtectFilterComponent.getAllFilters();
      assign(ext_parameters, {
        resource_filters: !isEmpty(vmFilters) ? vmFilters : null,
        overwrite: this.formGroup.value.slaOverwrite,
        binding_policy: this.formGroup.value.slaPolicy
      });
    }

    // 索引
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

    return {
      ext_parameters
    };
  }
}
