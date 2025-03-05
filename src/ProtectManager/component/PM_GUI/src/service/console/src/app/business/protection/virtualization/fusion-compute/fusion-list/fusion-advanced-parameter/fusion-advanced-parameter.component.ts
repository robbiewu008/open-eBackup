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
  ClientManagerApiService,
  DataMap,
  Features,
  I18NService,
  ProtectedResourceApiService,
  ResourceType,
  Scene
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
  selector: 'aui-fusion-advanced-parameter',
  templateUrl: './fusion-advanced-parameter.component.html',
  styleUrls: ['./fusion-advanced-parameter.component.less']
})
export class FusionAdvancedParameterComponent implements OnInit {
  includes = includes;
  ResourceType = ResourceType;
  resourceData;
  resourceType;
  selectedNode;
  hostOptions = [];
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  dataMap = DataMap;
  @ViewChild(ProtectFilterComponent, { static: false })
  ProtectFilterComponent: ProtectFilterComponent;
  extParams;
  isSupport = true;

  isFusionOne = false;

  speedErrorTip = {
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [10, 500])
  };

  constructor(
    public fb: FormBuilder,
    private appUtilsService: AppUtilsService,
    private i18n: I18NService,
    public message: MessageService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private clientManagerApiService: ClientManagerApiService
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
        subType: [
          `${
            this.isFusionOne
              ? DataMap.Application_Type.FusionOne.value
              : DataMap.Application_Type.FusionCompute.value
          }Plugin`
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
      isConsistent: extParameters.is_consistent ?? false,
      snapDeleteSpeed: extParameters.snap_delete_speed || '',
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

  initForm() {
    this.formGroup = this.fb.group({
      isConsistent: new FormControl(false),
      snapDeleteSpeed: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(10, 500)
        ]
      }),
      proxyHost: new FormControl([]),
      slaOverwrite: new FormControl(false),
      slaPolicy: new FormControl(['APPLY_TO_ALL', 'APPLY_TO_NEW'])
    });
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(this.formGroup.valid);
    });
    this.listenForm();
  }

  listenForm() {
    this.formGroup.get('proxyHost').valueChanges.subscribe(res => {
      if (isEmpty(res)) {
        this.isSupport = true;
        return;
      }

      const params = {
        hostUuidsAndIps: res,
        applicationType: 'FusionCompute',
        scene: Scene.Protect,
        buttonNames: [Features.ConsistencySnapshot]
      };
      this.clientManagerApiService
        .queryAgentApplicationUsingPOST({
          AgentCheckSupportParam: params,
          akOperationTips: false
        })
        .subscribe(res => {
          this.isSupport = res?.ConsistencySnapshot;
          if (!this.isSupport) {
            this.formGroup.get('isConsistent').setValue(false);
          }
        });
    });
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
    this.isFusionOne =
      this.resourceData?.subType === DataMap.Resource_Type.fusionOne.value ||
      this.resourceData?.sub_type === DataMap.Resource_Type.fusionOne.value;
  }

  onOK() {
    const ext_parameters = {};
    assign(ext_parameters, {
      is_consistent: this.formGroup.value.isConsistent,
      snap_delete_speed: +this.formGroup.value.snapDeleteSpeed || 0,
      agents:
        this.formGroup.value.proxyHost
          ?.filter(item => {
            return includes(map(this.hostOptions, 'value'), item);
          })
          .join(';') || null
    });

    if (
      !includes(
        [
          DataMap.Resource_Type.fusionComputeVirtualMachine.value,
          DataMap.Resource_Type.vmGroup.value
        ],
        this.resourceType
      )
    ) {
      const stkFilters = this.ProtectFilterComponent.getAllFilters();
      assign(ext_parameters, {
        resource_filters: !isEmpty(stkFilters) ? stkFilters : null,
        overwrite: this.formGroup.value.slaOverwrite,
        binding_policy: this.formGroup.value.slaPolicy
      });
    }

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
