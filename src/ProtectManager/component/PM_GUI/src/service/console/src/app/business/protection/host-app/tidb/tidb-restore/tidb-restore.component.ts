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
  RestoreV2LocationType,
  RestoreV2Type,
  VmFileReplaceStrategy
} from 'app/shared';
import {
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  assign,
  each,
  filter,
  find,
  isEmpty,
  isNumber,
  isString
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-tidb-restore',
  templateUrl: './tidb-restore.component.html',
  styleUrls: ['./tidb-restore.component.less']
})
export class TidbRestoreComponent implements OnInit {
  resourceData;
  targetOptions = [];
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = VmFileReplaceStrategy;
  dataMap = DataMap;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE / 2;
  clusterUuid: any;
  version;

  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.initForm();
    this.getClusterOptions();
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('target').setValue(config.targetEnv);
      if (this.rowCopy.backup_type !== DataMap.CopyData_Backup_Type.log.value) {
        this.formGroup
          .get('isDeleteTable')
          .setValue(config.extendInfo?.shouldDeleteTable === '1');
      }
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getClusterOptions(null, null, event);
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl({
        value:
          this.resourceData?.environment_name ||
          this.rowCopy?.resource_environment_name ||
          this.rowCopy?.name,
        disabled: true
      }),
      target: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      isDeleteTable: new FormControl(false)
    });

    this.listenForm();
    this.modal.getInstance().lvOkDisabled = false;
    if (
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('target').disable();
      } else {
        this.formGroup.get('target').enable();
      }
    });
  }

  getClusterOptions(recordsTemp?, startPage?, labelParams?: any) {
    const conditions = {
      subType: [DataMap.Resource_Type.tidbCluster.value]
    };
    extendParams(conditions, labelParams);

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
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
        startPage ===
          Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
        res.totalCount === 0
      ) {
        const clusterArray = [];

        each(recordsTemp, item => {
          clusterArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.targetOptions = clusterArray;
        if (
          this.resourceData.sub_type === DataMap.Resource_Type.tidbCluster.value
        ) {
          this.targetOptions = filter(this.targetOptions, item => {
            return item.uuid !== this.resourceData.uuid;
          });
        } else {
          this.targetOptions = filter(this.targetOptions, item => {
            return item.uuid !== this.resourceData.environment_uuid;
          });
        }
        this.updateDrillData();
        return;
      }
      this.getClusterOptions(recordsTemp, startPage, labelParams);
    });
  }

  getVersion() {
    let tmp: any = filter(this.targetOptions, item => {
      return item.uuid === this.formGroup.value?.target;
    });
    const originVersion = JSON.parse(this.rowCopy.resource_properties).version;
    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN) {
      this.version = 0;
    } else {
      if (tmp[0].version > originVersion) {
        this.version = 1;
      } else {
        this.version = 0;
      }
    }
  }

  getParams() {
    this.getVersion();
    let tmpTargetEnv;
    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN) {
      if (
        this.resourceData.sub_type === DataMap.Resource_Type.tidbCluster.value
      ) {
        tmpTargetEnv = this.resourceData.uuid;
      } else {
        tmpTargetEnv = this.resourceData.environment_uuid;
      }
    } else {
      tmpTargetEnv = this.formGroup.value.target;
    }
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv: tmpTargetEnv,
      restoreType:
        this.restoreType === RestoreV2Type.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject: tmpTargetEnv
    };

    if (this.rowCopy.backup_type !== DataMap.CopyData_Backup_Type.log.value) {
      assign(params, {
        extendInfo: {
          shouldDeleteTable: this.formGroup.value.isDeleteTable ? '1' : '0'
        }
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

  getTargetParams() {
    if (
      this.resourceData.sub_type ===
      this.dataMap.Resource_Type.tidbCluster.value
    ) {
      return {
        ...this.formGroup.value,
        resource:
          this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
            ? {
                name: this.resourceData.name,
                value: this.resourceData.uuid
              }
            : assign(
                {},
                find(this.targetOptions, {
                  value: this.formGroup.value.target
                }),
                {
                  name: find(this.targetOptions, {
                    value: this.formGroup.value.target
                  })?.label
                }
              ),
        requestParams: this.getParams()
      };
    } else {
      return {
        ...this.formGroup.value,
        resource:
          this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
            ? {
                name: this.rowCopy.resource_environment_name,
                value: this.resourceData.environment_uuid
              }
            : assign(
                {},
                find(this.targetOptions, {
                  value: this.formGroup.value.target
                }),
                {
                  name: find(this.targetOptions, {
                    value: this.formGroup.value.target
                  })?.label
                }
              ),
        requestParams: this.getParams()
      };
    }
  }

  getTargetPath() {
    this.getVersion();
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.resourceData?.name
      : `${
          find(this.targetOptions, {
            value: this.formGroup.value.target
          })['label']
        }`;
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

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }
}
