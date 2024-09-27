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
import {
  assign,
  each,
  filter,
  find,
  first,
  get,
  isEmpty,
  isNumber,
  isString,
  set,
  size
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-goldendb-restore',
  templateUrl: './goldendb-restore.component.html',
  styleUrls: ['./goldendb-restore.component.less']
})
export class GoldendbRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = VmFileReplaceStrategy;
  clusterOptions = [];
  instanceOptions = [];
  originInstanceOptions = [];
  resourceData;
  copyGroupNum;
  copyResourceGroupNum;
  selectedInstance;
  originInstance;
  instanceTips = '';
  disableOriginLocation = false;
  readonly PAGESIZE = CommonConsts.PAGE_SIZE * 10;
  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  textAreaErrorTips = {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1000])
  };
  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    public i18n: I18NService,
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
    this.getResourceData();
    this.initForm();
    this.getOriginInstance();
    this.getClusterOptions();
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getClusterOptions(null, null, event);
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('cluster')?.setValue(config.targetEnv);
      this.formGroup.get('instance')?.setValue(config.targetObject);
    }
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    const instance = JSON.parse(
      get(this.resourceData, 'extendInfo.clusterInfo', '{}')
    );
    this.copyGroupNum = size(get(instance, 'group', []));
    this.instanceTips = this.i18n.get(
      'protection_goldendb_restore_tips_label',
      [this.copyGroupNum]
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl(''),
      cluster: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      instance: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      )
    });

    this.listenForm();
    this.modal.getInstance().lvOkDisabled = false;
    if (this.disableOriginLocation) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('cluster').disable();
        this.formGroup.get('instance').disable();
      } else {
        this.formGroup.get('cluster').enable();
        this.formGroup.get('instance').enable();
      }
    });

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      this.formGroup.get('instance').setValue('');
      if (isEmpty(res)) {
        return;
      }

      this.getInstanceOptions(res);
    });

    this.formGroup.get('instance').valueChanges.subscribe(res => {
      this.selectedInstance = find(this.instanceOptions, { uuid: res });
    });
  }

  getOriginInstance() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        uuid: this.resourceData.uuid,
        subType: [DataMap.Resource_Type.goldendbInstance.value]
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!size(res.records)) {
        return;
      }
      const instanceArray = [];
      each(res.records, item => {
        const instance = JSON.parse(get(item, 'extendInfo.clusterInfo', '{}'));
        const groupNum = size(get(instance, 'group', []));
        instanceArray.push({
          ...item,
          groupNum: groupNum,
          key: item.uuid,
          value: item.uuid,
          label: item.name,
          isLeaf: true
        });
      });
      this.originInstanceOptions = instanceArray;
      this.originInstance = first(this.originInstanceOptions);
      this.formGroup.get('originLocation').setValue(this.resourceData.uuid);
    });
  }

  getClusterOptions(recordsTemp?, startPage?, labelParams?: any) {
    const conditions = {
      subType: [DataMap.Resource_Type.goldendbCluter.value]
    };
    extendParams(conditions, labelParams);
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: this.PAGESIZE,
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
        startPage === Math.ceil(res.totalCount / this.PAGESIZE) ||
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
        this.clusterOptions = clusterArray;

        if (
          find(this.clusterOptions, { uuid: this.resourceData.parent_uuid })
        ) {
          this.formGroup.get('cluster').setValue(this.resourceData.parent_uuid);
        }
        this.updateDrillData();
        return;
      }
      this.getClusterOptions(recordsTemp, startPage, labelParams);
    });
  }

  getInstanceOptions(cluster, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: this.PAGESIZE,
      conditions: JSON.stringify({
        parentUuid: cluster,
        subType: [DataMap.Resource_Type.goldendbInstance.value]
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
        startPage === Math.ceil(res.totalCount / this.PAGESIZE) ||
        res.totalCount === 0
      ) {
        const instanceArray = [];
        each(recordsTemp, item => {
          const instance = JSON.parse(
            get(item, 'extendInfo.clusterInfo', '{}')
          );
          const groupNum = size(get(instance, 'group', []));
          instanceArray.push({
            ...item,
            groupNum: groupNum,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });

        this.instanceOptions = filter(instanceArray, item => {
          return item.uuid !== this.resourceData?.uuid;
        });

        return;
      }
      this.getInstanceOptions(recordsTemp, startPage);
    });
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData?.environment_uuid ||
            this.rowCopy?.environment_uuid
          : this.formGroup.value.cluster,
      restoreType:
        this.restoreType === RestoreV2Type.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      filters: [],
      agents: []
    };
    if (this.rowCopy.restoreTimeStamp) {
      assign(params, {
        extendInfo: {
          restoreTimestamp: this.rowCopy.restoreTimeStamp
        }
      });
    }

    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW) {
      set(params, 'targetObject', this.formGroup.value.instance);
    } else {
      set(params, 'targetObject', this.resourceData?.uuid);
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
              find(this.instanceOptions, {
                value: this.formGroup.value.instance
              }),
              {
                name: find(this.instanceOptions, {
                  value: this.formGroup.value.instance
                })?.label
              }
            ),
      restoreLocation: this.formGroup.value.restoreLocation,
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.resourceData?.name
      : `${
          find(this.instanceOptions, {
            value: this.formGroup.value.instance
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
