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
  RestoreV2LocationType,
  RestoreV2Type,
  VmFileReplaceStrategy
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
  get,
  includes,
  isEmpty,
  isNumber,
  isString,
  set,
  size,
  startsWith
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-general-database-restore',
  templateUrl: './general-database-restore.component.html',
  styleUrls: ['./general-database-restore.component.less']
})
export class GeneralDatabaseRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = VmFileReplaceStrategy;
  hostOptions = [];
  targetOptions = [];
  resourceData;
  disableOriginLocation = false;
  disableOriginal = false;
  disableNew = false;
  isGbase = false;

  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  textAreaErrorTips = {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1000])
  };
  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.disableOriginLocation =
      this.rowCopy?.resource_status ===
        DataMap.Resource_Status.notExist.value ||
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.cascadedReplication.value;
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    const type = JSON.parse(this.rowCopy.resource_properties).extendInfo
      ?.databaseTypeDisplay;
    if (type.indexOf('GBase 8a') !== -1) {
      this.isGbase = true;
    }
    this.initForm();
    this.getHostOptions();
  }

  findTargetHost(config) {
    const target = find(this.hostOptions, item => {
      const resourceList = [];
      each(item?.extendInfo, (value, key) => {
        if (startsWith(key, '$citations_hosts_')) {
          resourceList.push(value);
        }
      });
      return includes(resourceList, config.targetObject);
    });
    return target;
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getHostOptions(event);
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      const target = this.findTargetHost(config);
      this.formGroup.get('host').setValue(target?.uuid);
      this.formGroup.get('target').setValue(config.targetObject);
      if (this.resourceData?.extendInfo?.script !== 'saphana') {
        this.formGroup
          .get('customParams')
          .setValue(config.extendInfo?.customParams);
      }
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl({
        value: this.resourceData?.environment_name || this.resourceData?.name,
        disabled: true
      }),
      host: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      target: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      metadataPath: new FormControl(''),
      customParams: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(1000)]
      })
    });
    this.getScriptConf();

    this.listenForm();
    this.modal.getInstance().lvOkDisabled = false;
    if (this.disableOriginLocation || this.disableOriginal || this.isDrill) {
      if (this.disableNew) {
        this.formGroup.get('restoreLocation').setValue('');
        this.modal.getInstance().lvOkDisabled = true;
      } else {
        this.formGroup
          .get('restoreLocation')
          .setValue(RestoreV2LocationType.NEW);
      }
    }
  }

  getScriptConf() {
    const scriptConf = JSON.parse(
      get(this.resourceData, 'extendInfo.scriptConf', '{}')
    );
    const restoreLocation = get(
      find(
        get(scriptConf, 'restore.support'),
        item => get(item, 'restoreType') === this.restoreType
      ),
      'targetLocation',
      []
    );

    if (!!size(restoreLocation)) {
      this.disableOriginal = !find(
        restoreLocation,
        val => val === RestoreV2LocationType.ORIGIN
      );
      this.disableNew = !find(
        restoreLocation,
        val => val === RestoreV2LocationType.NEW
      );
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('host').disable();
        this.formGroup.get('target').disable();
        this.formGroup.get('metadataPath').disable();
      } else {
        this.formGroup.get('host').enable();
        this.formGroup.get('target').enable();
        this.formGroup.get('metadataPath').enable();
      }
    });

    this.formGroup.get('host').valueChanges.subscribe(res => {
      this.targetOptions = [];
      this.formGroup.get('target').setValue('');

      if (!res) {
        return;
      }
      this.getDatabaseOptions(res);
    });
  }

  getHostOptions(labelParams?: any) {
    const conditions = {
      type: 'Plugin',
      subType: [`${DataMap.Resource_Type.generalDatabase.value}Plugin`]
    };
    extendParams(conditions, labelParams);

    const extParams = {
      conditions: JSON.stringify(conditions)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        if (this.isGbase) {
          const relateIp = this.resourceData.extendInfo.relatedHostIds.split(
            ','
          );
          resource = filter(resource, item => {
            return includes(relateIp, item.environment.uuid);
          });
        }
        each(resource, item => {
          const tmp = item.environment;
          if (
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
        this.hostOptions = hostArray;
        this.updateDrillData();
      }
    );
  }

  getDatabaseOptions(uuid, recordsTemp?, startPage?) {
    const host = find(this.hostOptions, host => host.uuid === uuid);
    const resourceList = [];

    each(host?.extendInfo, (value, key) => {
      if (startsWith(key, '$citations_hosts_')) {
        resourceList.push(value);
      }
    });

    if (!size(resourceList)) {
      return;
    }

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.generalDatabase.value,
        firstClassification: '2',
        uuid: resourceList
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
        startPage ===
          Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
        res.totalCount === 0
      ) {
        const clusterArray = [];
        each(recordsTemp, item => {
          if (item.name === this.resourceData.name || !this.isGbase) {
            clusterArray.push({
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true
            });
          }
        });
        this.targetOptions = filter(clusterArray, item => {
          return (
            item.extendInfo?.script === this.resourceData?.extendInfo?.script
          );
        });
        return;
      }
      this.getDatabaseOptions(uuid, recordsTemp, startPage);
    });
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData?.environment_uuid || this.resourceData?.uuid
          : this.formGroup.value.target,
      restoreType:
        this.restoreType === RestoreV2Type.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      filters: [],
      agents: [],
      extendInfo: {
        customParams: this.formGroup.value.customParams
      }
    };

    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      assign(params, {
        extendInfo: {
          restoreTimestamp: this.rowCopy.restoreTimeStamp
        }
      });
    }

    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW) {
      set(params, 'targetObject', this.formGroup.value.target);
    }
    return params;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? {
              name: this.formGroup.value.originLocation,
              value:
                this.resourceData?.environment_uuid || this.resourceData?.uuid
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
      restoreLocation: this.formGroup.value.restoreLocation,
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.resourceData?.environment_name || this.resourceData?.name
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

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }
}
