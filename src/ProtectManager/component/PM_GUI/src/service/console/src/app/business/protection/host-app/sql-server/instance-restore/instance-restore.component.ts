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
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  extendParams,
  I18NService,
  OverWriteOption,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  filter,
  find,
  first,
  includes,
  isEmpty,
  isNumber,
  isString,
  set,
  split,
  trim
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

  dataMap = DataMap;
  isFileLevelRestore;
  isFileLevelAndClusterRestore = false;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = OverWriteOption;
  formGroup: FormGroup;
  resourceData;
  options;
  locationLabel = this.i18n.get('common_location_label');
  disableOriginLocation = false;

  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    pathError: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048])
  };

  constructor(
    public i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private modal: ModalRef,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getClusters(null, null, event);
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      position: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      path: new FormControl('', {
        validators: [
          this.validPath(),
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(2048)
        ]
      })
    });
    // sqlserver数据库级恢复无需填写路径项
    if (
      this.rowCopy.resource_sub_type ===
        DataMap.Resource_Type.SQLServerInstance.value &&
      this.isFileLevelRestore
    ) {
      this.formGroup.get('path').clearValidators();
      this.formGroup.get('path').updateValueAndValidity();
    }
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (
        this.restoreType === RestoreType.FileRestore &&
        res === RestoreV2LocationType.NEW
      ) {
        this.formGroup.get('path').enable();
        this.locationLabel = this.i18n.get('explore_target_host_cluster_label');
        this.getHosts();
      } else {
        this.formGroup.get('path').disable();
        this.locationLabel = this.i18n.get('common_location_label');
        if (
          includes(
            [DataMap.Resource_Type.SQLServerInstance.value],
            this.rowCopy.resource_sub_type
          )
        ) {
          this.getHosts();
        } else {
          this.getClusters();
        }
      }
    });
    this.formGroup.valueChanges.subscribe(res => {
      this.disableOkBtn();
    });

    if (this.disableOriginLocation && !this.isDrill) {
      if (this.isFileLevelRestore) {
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
      } else {
        this.modal.getInstance().lvOkDisabled = true;
      }
    } else {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.ORIGIN);
    }
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return { required: { value: control.value } };
      }

      if (!CommonConsts.REGEX.windowsPath.test(control.value)) {
        return { pathError: { value: control.value } };
      }

      const path = split(control.value, '\\');

      if (find(path, item => !trim(item))) {
        return { pathError: { value: control.value } };
      }

      if (!/[a-zA-Z]:/.test(first(path))) {
        return { pathError: { value: control.value } };
      }

      path.shift();
      let vaild = true;
      each(path, item => {
        if (!/[\w\u4e00-\u9fa5\s]+/.test(item)) {
          vaild = false;
        }
      });

      if (!vaild) {
        return { pathError: { value: control.value } };
      }

      return null;
    };
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
  }

  setDrillHost() {
    if (this.isDrill) {
      this.protectedResourceApiService
        .ShowResource({
          resourceId: this.resourceData?.uuid,
          akDoException: false
        })
        .subscribe(res => {
          if (res) {
            this.formGroup.get('position').setValue(res.rootUuid);
          }
        });
    }
  }

  getClusters(recordsTemp?: any[], startPage?: number, labelParams?: any) {
    const conditions = {
      subType: DataMap.Resource_Type.SQLServerCluster.value
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
        const hostArray = [];
        each(recordsTemp, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.options = hostArray;

        if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
          this.formGroup
            .get('position')
            .setValue(
              this.resourceData.environment_uuid ||
                this.rowCopy.environment_uuid
            );
          this.setDrillHost();
        } else {
          this.formGroup.get('position').setValue('');
        }
        return;
      }
      this.getClusters(recordsTemp, startPage, labelParams);
    });
  }

  getHosts() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.SQLServerInstance.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.osType === DataMap.Os_Type.windows.value &&
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          }
        });
        if (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW) {
          this.options = filter(
            hostArray,
            item =>
              this.resourceData.path !== item?.endpoint &&
              this.resourceData.environment_endpoint !== item?.endpoint
          );
        } else {
          this.options = hostArray;
        }

        const location = find(
          this.options,
          item => item.uuid === this.resourceData.environment_uuid
        );

        if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
          this.formGroup
            .get('position')
            .setValue(
              this.resourceData.environment_uuid ||
                this.rowCopy.environment_uuid
            );
          this.setDrillHost();
        } else {
          this.formGroup.get('position').setValue('');
        }
      }
    );
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? {
              name:
                this.rowCopy?.resource_environment_name ||
                this.rowCopy?.resource_location,
              value: this.resourceData?.root_uuid
            }
          : {
              name: find(
                this.options,
                item => item.uuid === this.formGroup.value.position
              )['label'],
              value: this.formGroup.value.position,
              endpoint: find(
                this.options,
                item => item.uuid === this.formGroup.value.position
              )['endpoint']
            },
      restoreLocation: this.formGroup.value.restoreTo,
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? this.rowCopy?.resource_environment_name ||
          this.rowCopy?.resource_location
      : find(this.options, item => item.uuid === this.formGroup.value.position)[
          'label'
        ];
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv: this.formGroup.value.position,
      restoreType:
        this.restoreType === RestoreType.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreTo
    };
    if (this.formGroup.value.restoreTo !== RestoreV2LocationType.ORIGIN) {
      assign(params, {
        extendInfo: {
          newDatabasePath: this.formGroup.value.path
        }
      });
      if (
        this.rowCopy.resource_sub_type ===
        DataMap.Resource_Type.SQLServerInstance.value
      ) {
        delete params['extendInfo'].newDatabasePath;
      }
    }

    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      set(
        params,
        'extendInfo.restoreTimestamp',
        this.rowCopy.restoreTimeStamp || ''
      );
    }

    return params;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
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

  ngOnInit() {
    this.disableOriginLocation =
      this.rowCopy?.resource_status ===
        DataMap.Resource_Status.notExist.value ||
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.cascadedReplication.value;
    this.isFileLevelRestore = this.restoreType === RestoreType.FileRestore;

    this.initForm();
    this.getResourceData();
    if (
      includes(
        [DataMap.Resource_Type.SQLServerInstance.value],
        this.rowCopy.resource_sub_type
      )
    ) {
      this.getHosts();
    } else {
      // 是数据库级并且是集群的恢复
      this.isFileLevelAndClusterRestore = this.isFileLevelRestore;
      this.getClusters();
    }
  }
}
