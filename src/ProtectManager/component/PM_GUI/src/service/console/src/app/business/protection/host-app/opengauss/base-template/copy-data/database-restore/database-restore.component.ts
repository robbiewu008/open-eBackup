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
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
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
  each,
  find,
  includes,
  isEmpty,
  isNumber,
  isString,
  map
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-database-restore',
  templateUrl: './database-restore.component.html',
  styleUrls: ['./database-restore.component.less']
})
export class DatabaseRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;

  isReplicated = false;
  restoLocation = RestoreV2LocationType.ORIGIN;
  dataMap = DataMap;
  formGroup: FormGroup;
  dataBaseOptions = [];
  clusterOptions = [];
  instanceOptions = [];
  resourceData: any;
  isDistributed = false; // 判断是否为CMDB分布式集群数据库

  restoreLocationType = RestoreV2LocationType;

  constructor(
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  dataBaseErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidNameBegin: this.i18n.get('system_valid_sftp_username_begin_label'),
    invalidDataBaseName: this.i18n.get('common_valid_database_name_label'),
    invalidDataName: this.i18n.get(
      'common_valid_opengauss_database_name_label'
    ),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [63])
  });

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(this.restoLocation),
      cluster: new FormControl({
        value: `${this.resourceData.environment_name}/${this.resourceData.parent_name}`,
        disabled: true
      }),
      coverDataBase: new FormControl(),
      newDataBaseName: new FormControl(''),
      targetCuster: new FormControl(''),
      targetInstance: new FormControl(''),
      replaceDatabase: new FormControl(),
      newName: new FormControl()
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
            this.rowCopy?.generated_by ===
              DataMap.CopyData_generatedType.cascadedReplication.value ||
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
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === this.restoreLocationType.ORIGIN) {
        if (this.formGroup.value.coverDataBase) {
          this.formGroup
            .get('newDataBaseName')
            .setValidators([
              this.validDataBaseName(),
              this.baseUtilService.VALID.maxLength(63),
              this.baseUtilService.VALID.required()
            ]);
        } else {
          this.formGroup.get('newDataBaseName').clearValidators();
        }
        this.formGroup.get('targetCuster').clearValidators();
        this.formGroup.get('targetInstance').clearValidators();
        this.formGroup.get('newName').clearValidators();
      } else {
        if (this.formGroup.value.replaceDatabase) {
          this.formGroup
            .get('newName')
            .setValidators([
              this.validDataBaseName(),
              this.baseUtilService.VALID.maxLength(63),
              this.baseUtilService.VALID.required()
            ]);
        } else {
          this.formGroup.get('newName').clearValidators();
        }
        this.formGroup
          .get('targetCuster')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('targetInstance')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('newDataBaseName').clearValidators();
      }
      this.formGroup.get('targetCuster').updateValueAndValidity();
      this.formGroup.get('targetInstance').updateValueAndValidity();
      this.formGroup.get('newName').updateValueAndValidity();
      this.formGroup.get('newDataBaseName').updateValueAndValidity();
    });
    this.formGroup.get('coverDataBase').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('newDataBaseName').clearValidators();
      } else {
        this.formGroup
          .get('newDataBaseName')
          .setValidators([
            this.validDataBaseName(),
            this.baseUtilService.VALID.maxLength(63),
            this.baseUtilService.VALID.required()
          ]);
      }
      this.formGroup.get('newDataBaseName').updateValueAndValidity();
    });
    this.formGroup.get('replaceDatabase').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('newName').clearValidators();
      } else {
        this.formGroup
          .get('newName')
          .setValidators([
            this.validDataBaseName(),
            this.baseUtilService.VALID.maxLength(63),
            this.baseUtilService.VALID.required()
          ]);
      }
      this.formGroup.get('newName').updateValueAndValidity();
    });

    this.formGroup.get('targetCuster').valueChanges.subscribe(cluster => {
      this.getInstance(cluster);
    });
    this.formGroup.get('targetInstance').valueChanges.subscribe(instance => {
      this.getDatabase(instance);
    });
  }

  validDataBaseName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const name = control.value;
      if (!CommonConsts.REGEX.opengaussRestoreName.test(name)) {
        return { invalidNameBegin: { value: control.value } };
      }
      if (!CommonConsts.REGEX.dataBaseName.test(name)) {
        return { invalidDataBaseName: { value: control.value } };
      }
      if (includes(['postgres', 'template1', 'vastbase', 'template0'], name)) {
        return { invalidDataName: { value: control.value } };
      }
      return null;
    };
  }

  getDatabase(instance, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.OpenGauss_database.value],
        parentUuid: [['=='], instance]
      })
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
        this.dataBaseOptions = recordsTemp;
        return;
      }
      this.getDatabase(recordsTemp, startPage);
    });
  }

  getParams() {
    const params = {
      extendInfo: {
        newName: ''
      }
    };
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      assign(params, {
        copyId: this.rowCopy.uuid,
        targetEnv: this.resourceData.environment_uuid,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        targetObject: this.resourceData.uuid,
        extendInfo: {
          newName: this.formGroup.value.newDataBaseName
        }
      });
      if (!this.formGroup.value.coverDataBase) {
        delete params.extendInfo.newName;
      }
    } else {
      assign(params, {
        copyId: this.rowCopy.uuid,
        targetEnv: this.formGroup.value.targetCuster,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        targetObject: this.formGroup.value.targetInstance,
        extendInfo: {
          newName: this.formGroup.value.newName
        }
      });
      if (!this.formGroup.value.replaceDatabase) {
        delete params.extendInfo.newName;
      }
    }

    return params;
  }

  getTargetPath() {
    let targetPath = `${this.resourceData.environment_name}/${this.resourceData.parent_name}`;
    if (this.formGroup.value.restoreTo === this.restoreLocationType.NEW) {
      const targetCluster = find(this.clusterOptions, item => {
        return item.uuid === this.formGroup.value.targetCuster;
      }).name;
      const targetInstance = find(this.instanceOptions, item => {
        return item.uuid === this.formGroup.value.targetInstance;
      }).name;
      targetPath = `${targetCluster}/${targetInstance}`;
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

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.isDistributed =
      this.resourceData?.extendInfo?.deployType ===
      DataMap.Deployment_Type.cmdb.value;
  }

  getInstance(cluster, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.OpenGauss_instance.value],
        parentUuid: [['=='], cluster]
      })
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
        const instance = [];
        each(recordsTemp, item => {
          instance.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.instanceOptions = instance;
        return;
      }
      this.getInstance(recordsTemp, startPage);
    });
  }

  getClusters(recordsTemp?: any[], startPage?: number, labelParams?: any) {
    const tmpFilter = this.isDistributed
      ? [DataMap.Deployment_Type.cmdb.value]
      : [
          DataMap.Deployment_Type.single.value,
          DataMap.Deployment_Type.standby.value
        ];
    const conditions = {
      subType: [DataMap.Resource_Type.OpenGauss.value],
      deployType: [['in'], ...tmpFilter]
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

  ngOnInit(): void {
    this.getResourceData();
    this.initForm();
    this.getClusters();
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('targetCuster').setValue(config.targetEnv);
      this.formGroup.get('targetInstance').setValue(config.targetObject);
      this.formGroup
        .get('replaceDatabase')
        .setValue(!isEmpty(config.extendInfo?.newName));
      this.formGroup.get('newName').setValue(config.extendInfo?.newName);
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getClusters(null, null, event);
  }
}
