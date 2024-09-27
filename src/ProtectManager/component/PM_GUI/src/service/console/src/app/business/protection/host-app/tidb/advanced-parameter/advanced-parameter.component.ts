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
  QosService,
  ResourceType
} from 'app/shared';
import { ProtectFilterComponent } from 'app/shared/components/protect-filter/protect-filter.component';
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
  find = find;
  resourceData;
  resourceType;
  selectedNode;
  qosNames = [];
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
    public message: MessageService,
    public baseUtilService: BaseUtilService,
    private i18n: I18NService,
    private qosServiceApi: QosService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getQosNames();
    this.updateData();
  }

  getQosNames() {
    this.qosServiceApi
      .queryResourcesV1QosGet({
        pageNo: 0,
        pageSize: 100
      })
      .subscribe(res => {
        this.qosNames = map(res.items, (item: any) => {
          item['isLeaf'] = true;
          item['label'] = item.name;
          return item;
        });
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
      qos_id: extParameters.qos_id
    });
    this.formGroup.patchValue(extParameters);
    setTimeout(() => {
      this.valid$.next(this.formGroup.valid);
    }, 500);
  }

  initForm() {
    this.formGroup = this.fb.group({
      qos_id: new FormControl([])
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
      qos_id: this.formGroup.value.qos_id
    });

    const vmFilters = this.ProtectFilterComponent.getAllFilters();
    assign(ext_parameters, {
      resource_filters: !isEmpty(vmFilters) ? vmFilters : null
    });

    return {
      ext_parameters
    };
  }
}
