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
import { ModalRef } from '@iux/live';
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
  filter,
  find,
  includes,
  isEmpty,
  isNumber,
  isString,
  map
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-instance-restore',
  templateUrl: './instance-restore.component.html',
  styleUrls: ['./instance-restore.component.less']
})
export class InstanceRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  filterParams = [];
  clusterOptions = [];
  instanceOptions = [];
  isReplicated = false;
  dataMap = DataMap;
  restoLocation = RestoreV2LocationType.ORIGIN;
  restoreLocationType = RestoreV2LocationType;
  formGroup: FormGroup;
  resourceData;
  targetPath = '';
  location = this.i18n.get('common_location_label');

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private modal: ModalRef
  ) {}

  ngOnInit() {
    this.getResourceData();
    this.initForm();
    this.getClusters();
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('targetCuster').setValue(config.targetEnv);
      this.formGroup.get('targetInstance').setValue(config.targetObject);
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
      }),
      targetInstance: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
    this.watch();
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.resourceData.uuid,
        akDoException: false
      })
      .subscribe({
        next: res => {
          if (
            includes(
              [
                DataMap.CopyData_generatedType.cascadedReplication.value,
                DataMap.CopyData_generatedType.replicate.value
              ],
              this.rowCopy?.generated_by
            ) ||
            this.rowCopy.is_replicated ||
            this.isDrill
          ) {
            this.formGroup
              .get('restoreTo')
              .setValue(this.restoreLocationType.NEW);
            this.isReplicated = true;
          }
        },
        error: err => {
          this.formGroup
            .get('restoreTo')
            .setValue(this.restoreLocationType.NEW);
          this.isReplicated = true;
        }
      });

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
      this.formGroup.get('targetInstance').updateValueAndValidity();
    });
    this.formGroup.get('targetCuster').valueChanges.subscribe(cluster => {
      const params = {};
      const defaultConditions = {
        subType: [DataMap.Resource_Type.OpenGauss_instance.value]
      };
      assign(params, { conditions: JSON.stringify(defaultConditions) });
      this.protectedResourceApiService
        .ListResources(params)
        .subscribe((res: any) => {
          const instance = filter(res.records, item => {
            return item.parentUuid === cluster;
          });
          this.instanceOptions = map(instance, item => {
            return {
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true
            };
          });
          this.formGroup.updateValueAndValidity();
        });
    });
  }

  updateOld() {
    this.formGroup.get('cluster').clearValidators();
    this.formGroup.get('targetCuster').clearValidators();
    this.formGroup.get('targetInstance').clearValidators();
    this.formGroup.get('cluster').setValue(this.resourceData.name);
  }

  updateNew() {
    this.formGroup.get('cluster').clearValidators();
    this.formGroup
      .get('targetCuster')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('targetInstance')
      .setValidators([this.baseUtilService.VALID.required()]);
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
  }
  getClusters(recordsTemp?: any[], startPage?: number, labelParams?: any) {
    const conditions = {
      subType: [DataMap.Resource_Type.OpenGauss.value]
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
        const properties = JSON.parse(this.rowCopy.properties);
        let nodeNum = 0;
        let version = '';
        if (properties.extendInfo) {
          nodeNum = properties.extendInfo.cluster.nodes.length;
          version = properties.extendInfo.protectObject.type;
        } else {
          nodeNum = properties.cluster.nodes.length;
          version = properties.protectObject.type;
        }
        recordsTemp = recordsTemp.filter(item => {
          return (
            item.extendInfo.clusterVersion === version &&
            JSON.parse(item.extendInfo.nodes).length === nodeNum
          );
        });
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
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      assign(params, {
        copyId: this.rowCopy.uuid,
        targetEnv:
          this.resourceData?.environment_uuid ||
          this.resourceData?.environment?.uuid,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        targetObject: this.resourceData.uuid
      });
    } else {
      assign(params, {
        copyId: this.rowCopy.uuid,
        restoreType: this.restoreType,
        targetEnv: this.formGroup.value.targetCuster,
        targetLocation: this.formGroup.value.restoreTo,
        targetObject: this.formGroup.value.targetInstance
      });
    }
    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      assign(params, {
        extendInfo: {
          restoreTimestamp: this.rowCopy.restoreTimeStamp || ''
        }
      });
    }
    return params;
  }
  getTargetPath() {
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      this.targetPath = `${this.rowCopy.resource_environment_name}/${this.rowCopy.resource_name}`;
    } else {
      const targtCluster = find(this.clusterOptions, item => {
        return item.uuid === this.formGroup.value.targetCuster;
      }).name;
      const targetInstance = this.formGroup.controls.cluster.value;

      this.targetPath = `${targtCluster}/${targetInstance}`;
    }
    return this.targetPath;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const targetObj = find(this.clusterOptions, {
        value: this.formGroup.value.cluster
      });
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
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
