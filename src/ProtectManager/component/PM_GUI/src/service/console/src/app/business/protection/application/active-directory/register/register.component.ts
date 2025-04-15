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
  DataMap,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { filter, map } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register',
  templateUrl: './register.component.html',
  styleUrls: ['./register.component.less']
})
export class RegisterComponent implements OnInit {
  formGroup: FormGroup;
  clientOptions = [];
  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_fileset_name_vaild_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
  };
  @Input() rowData;

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getClientOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256),
          this.baseUtilService.VALID.name(
            /^[\u4e00-\u9fa5a-zA-Z_][\u4e00-\u9fa5a-zA-Z_0-9-]*$/
          )
        ]
      }),
      client: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
    if (!!this.rowData) {
      this.updateData();
    }
  }

  getClientOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`ADDSPlugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(
          resource,
          item =>
            item.environment.linkStatus ===
            DataMap.resource_LinkStatus_Special.normal.value
        );
        this.clientOptions = map(resource, item => {
          const environment = item.environment;
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${environment.name}(${environment.endpoint})`,
            isLeaf: true
          };
        });
      }
    );
  }

  updateData() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe(res => {
        this.formGroup.patchValue({
          name: res.name,
          client: res.parentUuid
        });
      });
  }

  getParams() {
    return {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.ActiveDirectory.value,
      parentUuid: this.formGroup.value.client
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams() as any;
      if (!!this.rowData) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowData.uuid,
            UpdateResourceRequestBody: params
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
        this.protectedResourceApiService
          .CreateResource({
            CreateResourceRequestBody: params
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
