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
import { Component } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  ProtectedResourceApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, filter, get, isArray, isEmpty, map } from 'lodash';

@Component({
  selector: 'aui-advanced-parameter',
  templateUrl: './advanced-parameter.component.html',
  styleUrls: ['./advanced-parameter.component.less']
})
export class AdvancedParameterComponent {
  resourceData;
  resourceType;
  formGroup: FormGroup;
  isDetail = false;
  hostOptions = [];
  constructor(
    private fb: FormBuilder,
    private clientManagerApiService: ClientManagerApiService,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getHostOptions();
  }

  initData(data: any, resourceType: string) {
    this.resourceData = data;
    this.resourceType = resourceType;
  }

  initForm() {
    const { protectedObject } = this.resourceData;
    const extParameters = protectedObject?.extParameters || {};
    this.formGroup = this.fb.group({
      proxyHost: new FormControl(
        !isEmpty(extParameters?.agents) ? extParameters?.agents.split(';') : [],
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      )
    });
  }

  getHostOptions() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.resourceData?.parentUuid
      })
      .subscribe(res => {
        const dataList =
          get(res, 'dependencies.agents') ||
          get(res, 'dependencies.children') ||
          [];
        let arr = map(dataList, item => {
          return {
            ...item,
            label: `${item?.name}(${item?.endpoint || item?.path})`,
            key: item?.uuid,
            value: item?.uuid,
            isLeaf: true
          };
        });
        this.hostOptions = arr;
      });
  }

  onOK() {
    const resourceData = isArray(this.resourceData)
      ? this.resourceData[0]
      : this.resourceData;
    return assign(resourceData, {
      ext_parameters: {
        agents: this.formGroup.value.proxyHost?.join(';')
      }
    });
  }
}
