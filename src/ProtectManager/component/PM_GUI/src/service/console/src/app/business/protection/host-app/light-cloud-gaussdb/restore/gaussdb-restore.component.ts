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
  compareVersion,
  DataMap,
  extendParams,
  I18NService,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  each,
  filter,
  find,
  get,
  includes,
  isEmpty,
  isNumber,
  set,
  size
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-gaussdb-restore',
  templateUrl: './gaussdb-restore.component.html',
  styleUrls: ['./gaussdb-restore.component.less']
})
export class GaussdbRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  formGroup: FormGroup;
  valid$: Subject<boolean> = new Subject();
  resourceData;
  targetEnv;
  disabledOrigin = false;
  projectOptions = [];
  instanceOptions = [];
  restoreLocationType = RestoreV2LocationType;
  readonly PAGE_SIZE = CommonConsts.PAGE_SIZE * 10;
  versionVerifyFailed = false;
  TARGET_DATABASE_VERSION = '24.7.30.10';
  originProjectUuid = '';
  originProjectVersion = '';
  restoreToNewLocationOnly = false;
  tip = this.i18n.get('protection_cloud_origin_restore_disabled_label');

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private appUtilService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.resourceData = JSON.parse(
      get(this.rowCopy, 'resource_properties', '{}')
    );
    this.originProjectUuid = this.resourceData.parent_uuid;
    this.originProjectVersion = get(
      this.resourceData,
      'extendInfo.tpopsVersion',
      ''
    );
    this.restoreToNewLocationOnly = includes(
      [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ],
      this.rowCopy?.generated_by
    );
    this.setInstanceDisabledStatus(false);
    this.initForm();
    this.listenForm();
    if (
      !this.restoreToNewLocationOnly &&
      this.rowCopy?.resource_status !== DataMap.Resource_Status.notExist.value
    ) {
      // 原位置存在才会去获取是否允许恢复
      this.getRestoreLimit();
    }

    this.getOriginProject();
    this.getProjectOptions();
    this.disableOkBtn();
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('targetProject').setValue(config.targetEnv);
      this.formGroup.get('targetInstance').setValue(config.targetObject);
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getProjectOptions(null, null, event);
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originCluster: new FormControl({
        value: this.resourceData?.name,
        disabled: true
      }),
      versionVerify: new FormControl(true),
      targetProject: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      targetInstance: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      )
    });
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('targetProject').disable();
        this.formGroup.get('targetInstance').disable();
      } else {
        this.formGroup.get('targetProject').enable();
        this.formGroup.get('targetInstance').enable();
      }
    });

    this.formGroup.get('targetProject').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.getInstanceOptions(res);
      this.formGroup.get('targetInstance').setValue('');
    });

    this.formGroup.get('versionVerify').valueChanges.subscribe(res => {
      this.checkRestoreConditions();
    });

    this.formGroup.get('targetInstance').valueChanges.subscribe(res => {
      this.checkRestoreConditions();
    });
  }

  getRestoreLimit() {
    this.protectedResourceApiService
      .CheckAllowRestore({
        resourceIds: String(this.resourceData?.uuid)
      })
      .subscribe(res => {
        const isAllowRestore = get(res[0], 'isAllowRestore', 'true');
        if (isAllowRestore === 'false') {
          this.tip = this.i18n.get('protection_origin_disable_restore_label');
          this.restoreToNewLocationOnly = true;
          this.formGroup
            .get('restoreLocation')
            .setValue(RestoreV2LocationType.NEW);
        } else {
          this.setInstanceDisabledStatus(this.formGroup.invalid);
        }
      });
  }

  getOriginProject() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.lightCloudGaussdbProject.value,
        uuid:
          this.resourceData?.environment?.uuid ||
          this.resourceData?.environment_uuid
      }),
      akDoException: false
    };
    this.protectedResourceApiService.ListResources(params).subscribe(
      res => {
        this.disabledOrigin =
          this.rowCopy?.resource_status ===
            DataMap.Resource_Status.notExist.value ||
          !size(res.records) ||
          this.restoreToNewLocationOnly;

        if (this.disabledOrigin) {
          this.formGroup
            .get('restoreLocation')
            .setValue(RestoreV2LocationType.NEW);
        }
      },
      error => {
        this.disabledOrigin =
          this.rowCopy?.resource_status ===
          DataMap.Resource_Status.notExist.value;

        if (this.disabledOrigin) {
          this.setInstanceDisabledStatus(true);
        }
      }
    );
  }

  getProjectOptions(recordsTemp?, startPage?, labelParams?: any) {
    const conditions = {
      subType: [DataMap.Resource_Type.lightCloudGaussdbProject.value]
    };
    extendParams(conditions, labelParams);
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: this.PAGE_SIZE,
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
        startPage === Math.ceil(res.totalCount / this.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        const projectArray = [];
        each(recordsTemp, item => {
          projectArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true,
            disabled: get(item, 'extendInfo.isAllowRestore', 'true') === 'false'
          });
        });
        this.projectOptions = projectArray;
        this.updateDrillData();
        return;
      }
      this.getProjectOptions(recordsTemp, startPage, labelParams);
    });
  }

  getInstanceOptions(uuid, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: this.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.lightCloudGaussdbInstance.value],
        pmAddress: find(this.projectOptions, { key: uuid })?.extendInfo
          ?.pmAddress
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
        startPage === Math.ceil(res.totalCount / this.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        const instanceArray = [];
        each(recordsTemp, item => {
          if (
            get(item, 'extendInfo.instanceStatus', '1') ===
            DataMap.airgapDeviceStatus.online.value
          ) {
            instanceArray.push({
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true
            });
          }
        });
        this.instanceOptions = filter(
          instanceArray,
          item => item.uuid !== this.resourceData.uuid
        );
        if (!!this.instanceOptions.length) {
          this.appUtilService.getRestoreOptions(this.instanceOptions, res => {
            this.instanceOptions = res;
          });
        }
        return;
      }
      this.getInstanceOptions(uuid, recordsTemp, startPage);
    });
  }

  setInstanceDisabledStatus(disabled: boolean) {
    this.valid$.next(!disabled);
  }

  checkRestoreConditions() {
    const isOrigin =
      this.formGroup.get('restoreLocation').value ===
      RestoreV2LocationType.ORIGIN;
    const isVerifyVersion = this.formGroup.get('versionVerify').value;
    const targetProjectUuid = this.formGroup.get('targetProject').value;
    const targetProjectUuidIsEmpty = isEmpty(targetProjectUuid);
    // 原位置无需校验，新位置没有开启版本校验时无需校验
    if (!isVerifyVersion || isOrigin || targetProjectUuidIsEmpty) {
      this.setInstanceDisabledStatus(this.formGroup.invalid);
      this.versionVerifyFailed = false;
      return;
    }

    if (targetProjectUuid !== this.originProjectUuid) {
      // 新项目恢复，需要继续比较目标实例版本
      const targetProject = find(this.projectOptions, {
        key: targetProjectUuid
      });
      const targetProjectVersion = get(
        targetProject,
        'extendInfo.tpopsVersion',
        null
      );
      if (
        isEmpty(targetProjectVersion) ||
        isEmpty(this.originProjectVersion) ||
        compareVersion(targetProjectVersion, this.TARGET_DATABASE_VERSION) ===
          -1 ||
        compareVersion(
          this.originProjectVersion,
          this.TARGET_DATABASE_VERSION
        ) === -1
      ) {
        this.setInstanceDisabledStatus(true);
        this.versionVerifyFailed = true;
        return;
      }
    }

    this.setInstanceDisabledStatus(this.formGroup.invalid);
    this.versionVerifyFailed = false;
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData.environment_uuid ||
            this.resourceData.environment?.uuid
          : this.formGroup.value.targetProject,
      restoreType:
        this.restoreType === RestoreV2Type.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData.uuid
          : this.formGroup.value.targetInstance
    };

    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      set(params, 'extendInfo.restoreTimestamp', this.rowCopy.restoreTimeStamp);
    }

    return params;
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.resourceData.name
      : `${
          find(
            this.projectOptions,
            item => item.value === this.formGroup.value.targetProject
          )['label']
        }/${
          find(
            this.instanceOptions,
            item => item.uuid === this.formGroup.value.targetInstance
          )['label']
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
    this.checkRestoreConditions();
  }
}
