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
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType
} from 'app/shared';
import { assign, filter, get, includes, isNumber, map } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-restore',
  templateUrl: './restore.component.html',
  styleUrls: ['./restore.component.less']
})
export class RestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;

  resource;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  restoreToNewLocationOnly = false;
  clusterOptions = [];
  instanceOptions = [];

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getCluster();
  }

  getCluster(recordsTemp?: any[], startPage?: number) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: 200,
        conditions: JSON.stringify({
          subType: [this.resource?.sub_type],
          isTopInstance: [['=='], '1']
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          // 只能同版本恢复
          recordsTemp = filter(
            recordsTemp,
            item => item.version === this.resource.version
          );
          this.clusterOptions = map(recordsTemp, item => {
            return assign(item, {
              value: item.uuid,
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });
          return;
        }
        this.getCluster(recordsTemp, startPage);
      });
  }

  initForm() {
    this.resource = JSON.parse(this.rowCopy?.resource_properties || '{}');
    this.restoreToNewLocationOnly =
      includes(
        [DataMap.CopyData_generatedType.replicate.value],
        this.rowCopy.generated_by
      ) || this.rowCopy.is_replicated;

    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      originalLocation: new FormControl({
        value: `${this.resource.path}/${this.resource.name}`,
        disabled: true
      }),
      targetCuster: new FormControl(''),
      startInstanceUser: new FormControl('')
    });

    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('targetCuster').clearValidators();
      } else {
        this.formGroup
          .get('targetCuster')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('targetCuster').updateValueAndValidity();
    });

    if (this.restoreToNewLocationOnly) {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
    }
  }

  getParams() {
    const params = {
      copyId: this.rowCopy?.uuid,
      restoreType: this.restoreType,
      targetEnv:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resource?.uuid
          : this.formGroup.value.targetCuster,
      targetLocation: this.formGroup.value.restoreTo,
      targetObject:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resource?.uuid
          : this.formGroup.value.targetCuster,
      extendInfo: {
        start_instance_user: this.formGroup.value.startInstanceUser || ''
      }
    };
    return params;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
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
    });
  }
}
