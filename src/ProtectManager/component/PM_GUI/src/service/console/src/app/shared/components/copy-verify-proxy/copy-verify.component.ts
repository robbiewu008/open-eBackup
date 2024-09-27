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
import {
  BaseUtilService,
  CopyControllerService,
  DataMap,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, each, join } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-copy-verify',
  templateUrl: './copy-verify.component.html',
  styleUrls: ['./copy-verify.component.less']
})
export class CopyVerifyComponent implements OnInit {
  dataDetails;
  proxyOptions = [];
  dataMap = DataMap;
  formGroup: FormGroup;

  @Input() rowCopy;
  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private copyControllerService: CopyControllerService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      proxy: new FormControl([])
    });
  }

  getProxyOptions() {
    let conditions = {
      environment: {
        linkStatus: [['in'], DataMap.resource_LinkStatus.normal.value]
      },
      type: 'Plugin'
    };
    if (
      this.rowCopy.resource_sub_type ===
      DataMap.Resource_Type.openStackCloudServer.value
    ) {
      assign(conditions, {
        subType: [
          `${DataMap.globalResourceType.openStackContainer.value}Plugin`
        ]
      });
    } else if (
      this.rowCopy.resource_sub_type ===
      DataMap.Resource_Type.HCSCloudHost.value
    ) {
      assign(conditions, {
        subType: [`HCScontainerPlugin`]
      });
    } else if (
      this.rowCopy.resource_sub_type ===
      DataMap.Resource_Type.FusionCompute.value
    ) {
      assign(conditions, {
        subType: [`${DataMap.Application_Type.FusionCompute.value}Plugin`]
      });
    } else if (
      this.rowCopy.resource_sub_type ===
      DataMap.Resource_Type.KubernetesStatefulset.value
    ) {
      assign(conditions, {
        subType: [`${DataMap.Resource_Type.Kubernetes.value}Plugin`]
      });
    } else if (
      this.rowCopy.resource_sub_type ===
      DataMap.Resource_Type.APSCloudServer.value
    ) {
      assign(conditions, {
        subType: [`${ResourceType.ApsaraStack}Plugin`]
      });
    } else if (
      this.rowCopy.resource_sub_type === DataMap.Resource_Type.cNwareVm.value
    ) {
      assign(conditions, {
        subType: [`${ResourceType.CNWARE}Plugin`]
      });
    } else if (
      this.rowCopy.resource_sub_type === DataMap.Resource_Type.fusionOne.value
    ) {
      assign(conditions, {
        subType: [`${ResourceType.FUSION_ONE}Plugin`]
      });
    }

    const extParams = {
      conditions: JSON.stringify(conditions)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value &&
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          }
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      this.copyControllerService
        .ExecuteCopyVerifyTask({
          copyId: this.rowCopy.uuid,
          copyVerifyRequest: {
            agents: join(this.formGroup.value.proxy, ';')
          }
        })
        .subscribe({
          next: res => {
            observer.next();
            observer.complete();
          },
          error: err => {
            observer.error(err);
            observer.complete();
          }
        });
    });
  }
}
