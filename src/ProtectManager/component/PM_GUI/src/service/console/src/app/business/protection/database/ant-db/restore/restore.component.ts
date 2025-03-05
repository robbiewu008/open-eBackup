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
  InstanceType,
  RestoreV2LocationType
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, find, includes, isEmpty, isString, map } from 'lodash';
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
  isDrill;
  proxyHostOptions = [];
  instanceOptions = [];
  dataMap = DataMap;
  restoreLocationType = RestoreV2LocationType;
  formGroup: FormGroup;
  resourceData;
  isClusterInstance = false;
  originalLocation;
  disabledOrigin;
  restoreToNewLocationOnly = false;
  resourceIsExist = true;

  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
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

  ngOnInit() {
    this.getResourceData();
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      originCluster: new FormControl({
        value: `${this.resourceData?.environment_name}(${this.resourceData?.environment_endpoint})/${this.resourceData?.name}`,
        disabled: true
      }),
      instance: new FormControl(''),
      preScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]
      }),
      postScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]
      }),
      executeScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]
      })
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

    setTimeout(() => {
      if (this.restoreToNewLocationOnly) {
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
        this.disabledOrigin = true;
      } else {
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.ORIGIN);
        this.modal.getInstance().lvOkDisabled = false;
      }
    });
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('instance').setValue('');
        this.formGroup.get('instance').clearValidators();
        this.instanceOptions = [];
        this.proxyHostOptions = [];
      } else {
        this.formGroup
          .get('instance')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.getInstanceOptions();
      }
      this.formGroup.get('instance').updateValueAndValidity();
    });
  }

  getResourceData() {
    this.isClusterInstance =
      this.rowCopy.resource_sub_type ===
      DataMap.Resource_Type.AntDBClusterInstance.value;
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
  }

  getInstanceOptions(event?) {
    const conditions = {
      subType: this.isClusterInstance
        ? [DataMap.Resource_Type.AntDBClusterInstance.value]
        : [DataMap.Resource_Type.AntDBInstance.value],
      isTopInstance: InstanceType.TopInstance
    };
    extendParams(conditions, event);
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      conditions: JSON.stringify(conditions)
    };
    this.appUtilsService.getResourceByRecursion(
      params,
      param => this.protectedResourceApiService.ListResources(param),
      res => {
        const resource = res.filter(
          item => item.uuid !== this.resourceData.uuid
        ); // 新位置不允许选择副本原实例
        this.instanceOptions = map(resource, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          };
        });
      }
    );
  }

  getParams() {
    let targetObj;
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW) {
      targetObj = find(this.instanceOptions, {
        value: this.formGroup.value.instance
      });
    }
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resourceData.environment_uuid
          : targetObj?.parentUuid,
      restoreType: this.restoreType,
      targetLocation: this.formGroup.value.restoreTo,
      scripts: {
        preScript: this.formGroup.value.preScript,
        postScript: this.formGroup.value.postScript,
        failPostScript: this.formGroup.value.executeScript
      },
      targetObject:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resourceData.uuid
          : this.formGroup.value.instance
    };
    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      assign(params, {
        extendInfo: {
          restoreTimestamp: this.rowCopy.restoreTimeStamp || ''
        }
      });
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

  getTargetPath() {
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      return `${this.resourceData?.environment_name}(${this.resourceData?.environment_endpoint})/${this.resourceData?.name}`;
    }
    const selectedInstance = find(this.instanceOptions, {
      uuid: this.formGroup.value.instance
    });
    return `${selectedInstance.name}`;
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('instance').setValue(config.targetObject);
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getInstanceOptions(event);
  }
}
