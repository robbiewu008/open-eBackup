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
  I18NService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType
} from 'app/shared';
import { assign, includes, isNumber, isString, map, trim } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-exchange-copy-data',
  templateUrl: './exchange-copy-data.component.html',
  styleUrls: ['./exchange-copy-data.component.less']
})
export class ExchangeCopyDataComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isReplicated = false;
  filterParams = [];
  clusterOptions = [];
  instanceOptions = [];
  dataMap = DataMap;
  restoLocation = RestoreV2LocationType.ORIGIN;
  restoreLocationType = RestoreV2LocationType;
  formGroup: FormGroup;
  resourceData;

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    pathError: this.i18n.get('common_path_error_label'),
    samePathError: this.i18n.get('protection_same_path_error_label'),
    unsupportPathError: this.i18n.get(
      'protection_unsupport_fileset_linux_path_label'
    )
  };

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

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(this.restoLocation),
      cluster: new FormControl({
        value: `${
          JSON.parse(this.rowCopy.resource_properties).environment_name
        }(${
          JSON.parse(this.rowCopy.resource_properties).environment_endpoint
        })`,
        disabled: true
      }),
      autoReload: new FormControl(false),
      autoUnload: new FormControl(true),
      replaceDatabase: new FormControl(true),
      targetCuster: new FormControl(''),
      dbPath: new FormControl('', {
        validators: [this.validPath(), this.validLinuxPath()]
      }),
      dbName: new FormControl(''),
      newName: new FormControl('')
    });
    this.watch();
    this.formGroup.get('restoreTo').updateValueAndValidity();
    this.formGroup.get('cluster').updateValueAndValidity();
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const paths = control.value;

      if (!CommonConsts.REGEX.unixPath.test(paths) || paths.length > 1024) {
        return { pathError: { value: control.value } };
      }
      return null;
    };
  }
  validLinuxPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const paths = control.value;
      if (includes(['proc', 'dev', 'run'], paths.split('/')[1])) {
        return { unsupportPathError: { value: control.value } };
      }
      return null;
    };
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
      this.formGroup.get('dbPath').updateValueAndValidity();
      this.formGroup.get('dbName').updateValueAndValidity();
    });
  }

  updateOld() {
    this.formGroup.get('cluster').clearValidators();
    this.formGroup
      .get('cluster')
      .setValue(
        `${JSON.parse(this.rowCopy.resource_properties).environment_name}(${
          JSON.parse(this.rowCopy.resource_properties).environment_endpoint
        })`
      );
    this.formGroup.get('targetCuster').clearValidators();
    this.formGroup.get('dbPath').clearValidators();
    this.formGroup.get('dbName').clearValidators();
  }

  updateNew() {
    this.formGroup.get('cluster').clearValidators();
    this.formGroup
      .get('targetCuster')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('dbPath')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.validPath(),
        this.validLinuxPath()
      ]);
    this.formGroup
      .get('dbName')
      .setValidators([
        this.baseUtilService.VALID.name(),
        this.baseUtilService.VALID.maxLength(64)
      ]);
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
      this.rowCopy.generated_by ===
        DataMap.CopyData_generatedType.replicate.value
    ) {
      this.isReplicated = true;
    }
    if (this.isReplicated) {
      this.restoLocation = RestoreV2LocationType.NEW;
    }
  }
  getClusters(recordsTemp?: any[], startPage?: number) {
    const conditions = {
      subType: [DataMap.Resource_Type.Dameng_singleNode.value],
      isTopInstance: [['=='], 1]
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
        const properties = JSON.parse(this.rowCopy.resource_properties);
        let version = properties.extendInfo.version;
        recordsTemp = recordsTemp.filter(item => {
          return item.extendInfo.version === version;
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
        this.formGroup.updateValueAndValidity();
        return;
      }
      this.getClusters(recordsTemp, startPage);
    });
  }

  getParams() {
    const params = {};
    if (this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN) {
      assign(params, {
        copyId: this.rowCopy.uuid,
        targetEnv: JSON.parse(this.rowCopy.resource_properties).parent_uuid,
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
        targetObject: this.formGroup.value.targetCuster,
        extendInfo: {
          dbName: this.formGroup.value.dbName,
          dbPath: this.formGroup.value.dbPath
        }
      });
    }
    return params;
  }
  getTargetPath() {
    let targetPath = JSON.parse(this.rowCopy.resource_properties)
      .environment_name;
    if (this.formGroup.value.restoreTo === this.restoreLocationType.NEW) {
      targetPath = this.formGroup.value.dbPath;
    }
    return targetPath;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      if (
        this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value &&
        this.rowCopy.restoreTimeStamp
      ) {
        if (
          this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN
        ) {
          assign(params, {
            extendInfo: {
              restoreTimestamp: this.rowCopy?.restoreTimeStamp
            }
          });
        } else {
          assign(params, {
            extendInfo: {
              restoreTimestamp: this.rowCopy?.restoreTimeStamp,
              dbName: this.formGroup.value.dbName,
              dbPath: this.formGroup.value.dbPath
            }
          });
        }
      }
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
