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
import { assign, each, filter, includes, isNumber, set } from 'lodash';
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
  isCopySetLog = false;

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
          const clusterArray = [];
          this.parseCluster(recordsTemp, clusterArray);
          this.clusterOptions = clusterArray;
          return;
        }
        this.getCluster(recordsTemp, startPage);
      });
  }

  private parseCluster(recordsTemp: any[], clusterArray: any[]) {
    each(recordsTemp, item => {
      // 副本集的日志副本不能恢复到非副本集的单实例去,原位置也不行
      if (
        this.isCopySetLog &&
        item.uuid === this.resource.uuid &&
        item.extendInfo?.singleType !==
          DataMap.mongoDBSingleInstanceType.copySet.value
      ) {
        this.restoreToNewLocationOnly = true;
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
      }
      if (
        (this.isCopySetLog &&
          item.extendInfo?.singleType ===
            DataMap.mongoDBSingleInstanceType.copySet.value) ||
        !this.isCopySetLog
      ) {
        clusterArray.push(
          assign(item, {
            value: item.uuid,
            key: item.uuid,
            label: item.name,
            isLeaf: true
          })
        );
      }
    });
  }

  initForm() {
    this.resource = JSON.parse(this.rowCopy?.resource_properties || '{}');
    this.isCopySetLog =
      this.resource.sub_type ===
        DataMap.Resource_Type.MongodbSingleInstance.value &&
      this.resource.extendInfo?.singleType ===
        DataMap.mongoDBSingleInstanceType.copySet.value &&
      this.rowCopy.source_copy_type === DataMap.CopyData_Backup_Type.log.value;
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
    if (this.rowCopy?.restoreTimeStamp) {
      set(params, 'extendInfo.restoreTimestamp', this.rowCopy.restoreTimeStamp);
    }
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
