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
  InstanceType,
  RestoreV2LocationType
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  find,
  includes,
  isEmpty,
  isNumber,
  isString,
  map,
  set
} from 'lodash';
import { Observable, Observer } from 'rxjs';
@Component({
  selector: 'aui-postgre-sql-restore',
  templateUrl: './postgre-sql-restore.component.html',
  styleUrls: ['./postgre-sql-restore.component.less']
})
export class PostgreSqlRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  clusterOptions = [];
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
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      originCluster: new FormControl({
        value: `${this.resourceData?.environment_name}(${this.resourceData?.environment_endpoint})/${this.resourceData?.name}`,
        disabled: true
      }),
      cluster: new FormControl(''),
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

    if (this.restoreToNewLocationOnly) {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
      this.disabledOrigin = true;
    }
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('cluster').setValue('');
        this.formGroup.get('instance').setValue('');
        this.formGroup.get('cluster').clearValidators();
        this.formGroup.get('instance').clearValidators();
        this.instanceOptions = [];
        this.clusterOptions = [];
      } else {
        this.formGroup
          .get('cluster')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('instance')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.getClusters();
      }
      this.formGroup.get('cluster').updateValueAndValidity();
      this.formGroup.get('instance').updateValueAndValidity();
    });

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      if (isEmpty(res)) {
        return;
      }
      this.formGroup.get('instance').setValue('', { emitEvent: false });
      this.instanceOptions = [];
      if (this.formGroup.get('restoreTo').value === RestoreV2LocationType.NEW) {
        defer(() => this.getInstanceOptions());
      }
    });
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
  }

  getClusters(labelParams?: any) {
    let conditions = {};
    if (this.isClusterInstance) {
      conditions = {
        subType: [DataMap.Resource_Type.PostgreSQLCluster.value]
      };
    } else {
      conditions = {
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.PostgreSQLInstance.value}Plugin`]
      };
    }
    extendParams(conditions, labelParams);

    const extParams = {
      conditions: JSON.stringify(conditions)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        if (this.isClusterInstance) {
          resource = resource.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
        } else {
          resource = resource.filter(
            item =>
              item.environment.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
        }
        each(resource, item => {
          const tmp = this.isClusterInstance ? item : item.environment;
          if (
            this.isClusterInstance ||
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
        this.clusterOptions = hostArray;
        this.updateDrillData();
      }
    );
  }

  getInstanceOptions(recordsTemp?: any[], startPage?: number) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        parentUuid: this.formGroup.value.cluster,
        subType: this.isClusterInstance
          ? DataMap.Resource_Type.PostgreSQLClusterInstance.value
          : DataMap.Resource_Type.PostgreSQLInstance.value
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
        recordsTemp = recordsTemp.filter(
          item => item.version === this.resourceData?.version
        );
        this.instanceOptions = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          };
        });

        return;
      }
      this.getInstanceOptions(recordsTemp, startPage);
    });
  }

  isExistOrigin(recordsTemp?: any[], startPage?: number) {
    const conditions = {
      subType: this.isClusterInstance
        ? [DataMap.Resource_Type.PostgreSQLClusterInstance.value]
        : [DataMap.Resource_Type.PostgreSQLInstance.value]
    };

    set(conditions, 'isTopInstance', InstanceType.TopInstance);

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
        recordsTemp = recordsTemp.filter(
          item => item.version === this.resourceData?.version
        );
        const totalData = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          };
        });
        this.formGroup.updateValueAndValidity();

        this.originalLocation = this.rowCopy.resource_id;
        this.formGroup.get('cluster').setValue(this.originalLocation);

        if (
          !find(totalData, item => item.uuid === this.formGroup.value.cluster)
        ) {
          this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
          this.disabledOrigin = true;
          this.resourceIsExist = false;
        }
        return;
      }
      this.isExistOrigin(recordsTemp, startPage);
    });
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

  ngOnInit() {
    this.rowCopy.resource_sub_type ===
    DataMap.Resource_Type.PostgreSQLClusterInstance.value
      ? (this.isClusterInstance = true)
      : (this.isClusterInstance = false);
    this.getResourceData();
    this.initForm();

    this.isExistOrigin();
  }

  getTargetPath() {
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      return `${this.resourceData?.environment_name}(${this.resourceData?.environment_endpoint})/${this.resourceData?.name}`;
    }

    const selectedCluster = find(this.clusterOptions, {
      uuid: this.formGroup.value.cluster
    });
    const selectedInstance = find(this.instanceOptions, {
      uuid: this.formGroup.value.instance
    });
    return `${selectedCluster.label}/${selectedInstance.name}`;
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('cluster').setValue(config.targetEnv);
      this.formGroup.get('instance').setValue(config.targetObject);
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getClusters(event);
  }
}
