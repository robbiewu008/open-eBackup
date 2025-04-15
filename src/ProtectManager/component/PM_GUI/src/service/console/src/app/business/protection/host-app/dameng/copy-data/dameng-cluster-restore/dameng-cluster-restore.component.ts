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
  extendParams,
  I18NService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType
} from 'app/shared';
import {
  assign,
  find,
  includes,
  isEmpty,
  isNumber,
  isString,
  map
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-dameng-cluster-restore',
  templateUrl: './dameng-cluster-restore.component.html',
  styleUrls: ['./dameng-cluster-restore.component.less']
})
export class DamengClusterRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  isReplicated = false;
  filterParams = [];
  clusterOptions = [];
  dataMap = DataMap;
  restoreLocationType = RestoreV2LocationType;
  restoLocation = RestoreV2LocationType.ORIGIN;
  formGroup: FormGroup;
  resourceData;
  displayDifferentDatabaseTip = false;
  targetVersion: string;
  currentVersion;

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.getResourceData();
    this.initForm();
    this.getClusters();
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('targetCuster').setValue(config.targetEnv);
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getClusters(null, null, event);
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(this.restoLocation),
      cluster: new FormControl({
        value: this.resourceData.name,
        disabled: true
      }),
      targetCuster: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });

    this.watch();
    this.formGroup.get('restoreTo').updateValueAndValidity();
    this.formGroup.get('cluster').updateValueAndValidity();
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.updateOld();
      } else {
        this.updateNew();
      }
      this.formGroup.get('cluster').updateValueAndValidity();
      this.formGroup.get('targetCuster').updateValueAndValidity();
    });

    this.formGroup.get('targetCuster').valueChanges.subscribe(res => {
      if (!isEmpty(res)) {
        const targetCusterObject = find(this.clusterOptions, {
          uuid: res
        });
        const properties = JSON.parse(this.rowCopy.resource_properties);
        this.currentVersion = properties.extendInfo.version;
        this.targetVersion = targetCusterObject.extendInfo.version;
        this.displayDifferentDatabaseTip =
          this.targetVersion !== this.currentVersion;
      }
    });
  }

  updateOld() {
    this.formGroup.get('cluster').clearValidators();
    this.formGroup.get('cluster').setValue(this.resourceData.name);
    this.formGroup.get('targetCuster').clearValidators();
  }

  updateNew() {
    this.formGroup.get('cluster').clearValidators();
    this.formGroup
      .get('targetCuster')
      .setValidators([this.baseUtilService.VALID.required()]);
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    if (
      (includes(
        [
          DataMap.CopyData_generatedType.cloudArchival.value,
          DataMap.CopyData_generatedType.tapeArchival.value
        ],
        this.rowCopy.generated_by
      ) &&
        this.rowCopy.is_replicated) ||
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      )
    ) {
      this.isReplicated = true;
    }
    if (this.isReplicated) {
      this.restoLocation = RestoreV2LocationType.NEW;
    }
  }
  getClusters(recordsTemp?: any[], startPage?: number, labelParams?: any) {
    const conditions = {
      subType: [DataMap.Resource_Type.Dameng_cluster.value],
      isTopInstance: [['=='], 1]
    };
    extendParams(conditions, labelParams);
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
        this.updateDrillData();
        this.formGroup.updateValueAndValidity();
        return;
      }
      this.getClusters(recordsTemp, startPage, labelParams);
    });
  }

  getParams() {
    const params = {};
    if (this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN) {
      assign(params, {
        copyId: this.rowCopy.uuid,
        targetEnv: this.resourceData.uuid,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        targetObject: this.resourceData.uuid
      });
    } else {
      assign(params, {
        copyId: this.rowCopy.uuid,
        targetEnv: this.formGroup.value.targetCuster,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        targetObject: this.formGroup.value.targetCuster
      });
    }
    return params;
  }

  getTargetPath() {
    let targetPath = this.resourceData.name;
    if (this.formGroup.value.restoreTo === this.restoreLocationType.NEW) {
      targetPath = find(this.clusterOptions, item => {
        return item.uuid === this.formGroup.value.targetCuster;
      }).name;
    }
    return targetPath;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
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
