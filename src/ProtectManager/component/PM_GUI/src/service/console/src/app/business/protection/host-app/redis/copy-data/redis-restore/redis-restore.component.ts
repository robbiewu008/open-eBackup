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
  RestoreV2LocationType
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { find, includes, isNumber, isString, map } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-redis-restore',
  templateUrl: './redis-restore.component.html',
  styleUrls: ['./redis-restore.component.less']
})
export class RedisRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  clusterOptions = [];
  dataMap = DataMap;
  restoreLocationType = RestoreV2LocationType;
  formGroup: FormGroup;
  resourceData;
  location = this.i18n.get('common_location_label');
  disabledOrigin = false;
  restoreToNewLocationOnly = false;
  resourceIsExist = true;

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.getResourceData();
    this.initForm();
    this.getClusters();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      cluster: new FormControl(
        this.rowCopy.resource_id ? this.rowCopy.resource_id : ''
      )
    });
    this.watch();
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) || this.rowCopy.is_replicated;

    if (this.restoreToNewLocationOnly) {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
      this.disabledOrigin = true;
    }
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.updateOld();
        this.location = this.i18n.get('common_location_label');
      } else {
        this.updateNew();
        this.location = this.i18n.get('common_target_to_cluster_label');
      }
      this.formGroup.get('cluster').updateValueAndValidity();
      this.formGroup.updateValueAndValidity();
    });
  }

  updateOld() {
    this.formGroup.get('cluster').clearValidators();
    this.formGroup
      .get('cluster')
      .setValue(this.rowCopy.resource_id ? this.rowCopy.resource_id : '');
  }

  updateNew() {
    this.formGroup.get('cluster').setValue('');
    this.formGroup
      .get('cluster')
      .setValidators([this.baseUtilService.VALID.required()]);
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
  }

  getClusters(recordsTemp?: any[], startPage?: number) {
    const conditions = {
      subType: [DataMap.Resource_Type.Redis.value],
      resourceType: [['=='], 'cluster']
    };

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        this.clusterOptions = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          };
        });
        this.formGroup.updateValueAndValidity();
        this.formGroup.get('cluster').setValue(this.rowCopy.resource_id);
        if (
          !find(
            this.clusterOptions,
            item => item.uuid === this.formGroup.value.cluster
          )
        ) {
          this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
          this.resourceIsExist = false;
          this.disabledOrigin = true;
        }
        return;
      }
      this.getClusters(recordsTemp, startPage);
    });
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const targetObj = find(this.clusterOptions, {
        value: this.formGroup.value.cluster
      });
      const params = {
        copyId: this.rowCopy.uuid,
        targetEnv: targetObj?.uuid,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        targetObject: this.formGroup.value.cluster
      };
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

  getTargetPath() {
    const selectedCluster = find(this.clusterOptions, {
      uuid: this.formGroup.value.cluster
    });
    return `${selectedCluster.label}`;
  }
}
