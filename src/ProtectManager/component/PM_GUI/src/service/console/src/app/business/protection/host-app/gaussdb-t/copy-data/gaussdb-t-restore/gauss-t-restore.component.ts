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
  OverWriteOption,
  RestoreV2LocationType
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  assign,
  filter,
  find,
  includes,
  isEmpty,
  isNumber,
  isString,
  map,
  set,
  startsWith
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-gaussdb-t-restore',
  templateUrl: './gaussdb-t-restore.component.html',
  styleUrls: ['./gaussdb-t-restore.component.less']
})
export class GaussDBTRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  copyLocation;
  hostData = [];
  instData = [];
  filterParams = [];
  clusterOptions = [];
  displayClusterOptions = [];
  databaseOptions = [];
  dataMap = DataMap;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = OverWriteOption;
  formGroup: FormGroup;
  resourceData;
  location = this.i18n.get('common_location_label');
  isClusterInstance = false;
  originalLocation;
  disabledOrigin;

  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  parallelNumberPlaceHolder = '1~16';
  parallelNumberErrorTip = {
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 16])
  };

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      cluster: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      parallel_process: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 16)
        ]
      })
    });
    this.watch();
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.updateOld();
        this.location = this.i18n.get('common_location_label');
        this.displayClusterOptions = [...this.clusterOptions];
      } else {
        this.displayClusterOptions = filter(this.clusterOptions, item => {
          return item.uuid !== this.originalLocation;
        });
        if (this.formGroup.value.cluster === this.originalLocation) {
          this.formGroup.get('cluster').setValue('');
        }
        this.location = this.i18n.get('protection_volume_restore_target_label');
      }
    });
  }
  updateOld() {
    this.formGroup.get('cluster').setValue(this.originalLocation);
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
  }

  getClusters(recordsTemp?: any[], startPage?: number, labelParams?: any) {
    const conditions = {
      subType: [this.rowCopy.resource_sub_type],
      deployType: [['=='], this.resourceData.extendInfo.deployType]
    };
    extendParams(conditions, labelParams);
    const version = this.resourceData.version;

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
        recordsTemp = filter(recordsTemp, item => {
          return startsWith(item.version, version);
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
        this.displayClusterOptions = [...this.clusterOptions];
        this.formGroup.updateValueAndValidity();

        this.originalLocation = this.rowCopy.resource_id;
        this.formGroup.get('cluster').setValue(this.originalLocation);

        if (
          !find(
            this.clusterOptions,
            item => item.uuid === this.formGroup.value.cluster
          ) ||
          includes(
            [
              DataMap.CopyData_generatedType.cascadedReplication.value,
              DataMap.CopyData_generatedType.replicate.value
            ],
            this.rowCopy.generated_by
          ) ||
          this.rowCopy.is_replicated
        ) {
          this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
          this.disabledOrigin = true;
        }
        this.updateDrillData();
        return;
      }
      this.getClusters(recordsTemp, startPage, labelParams);
    });
  }

  getParams() {
    const tagetObj = find(this.clusterOptions, {
      value: this.formGroup.value.cluster
    });
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv: tagetObj.rootUuid,
      restoreType: this.restoreType,
      targetLocation: this.formGroup.value.restoreTo,
      targetObject: this.formGroup.value.cluster,
      extendInfo: {}
    };
    if (this.formGroup.value.parallel_process) {
      set(
        params,
        'extendInfo.parallel_process',
        Number(this.formGroup.value.parallel_process)
      );
    }
    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      set(params, 'extendInfo.restoreTimestamp', this.rowCopy.restoreTimeStamp);
    }
    return params;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: this.getParams() })
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

  ngOnInit() {
    this.getResourceData();
    this.initForm();
    this.getClusters();
  }

  getTargetPath() {
    let targetPath;
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      targetPath = find(this.clusterOptions, item => {
        return item.uuid === this.originalLocation;
      }).name;
    } else {
      targetPath = find(this.clusterOptions, item => {
        return item.uuid === this.formGroup.value.cluster;
      }).name;
    }
    return targetPath;
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('cluster').setValue(config.targetObject);
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getClusters(null, null, event);
  }
}
