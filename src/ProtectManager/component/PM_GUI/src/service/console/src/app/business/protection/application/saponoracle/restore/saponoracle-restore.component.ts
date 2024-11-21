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
  DataMapService,
  extendParams,
  getMultiHostOps,
  I18NService,
  InstanceType,
  MultiCluster,
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
  isEmpty,
  isNumber,
  isString,
  set
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { AppUtilsService } from '../../../../../shared/services/app-utils.service';

@Component({
  selector: 'aui-saponoracle-restore',
  templateUrl: './saponoracle-restore.component.html',
  styleUrls: ['./saponoracle-restore.component.less']
})
export class SaponoracleRestoreComponent implements OnInit {
  resourceData;
  targetOptions = [];
  databaseOptions = [];
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = VmFileReplaceStrategy;
  numberOfChannelRangeErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 254])
  };
  dataMap = DataMap;
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private appUtilsService: AppUtilsService,
    public i18n: I18NService,
    public dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.initForm();
    this.getDatabaseOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl({
        value: this.resourceData?.name,
        disabled: true
      }),
      target: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      numberOfChannelOpen: new FormControl(false),
      numberOfChannels: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 254)
        ]
      })
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
    this.formGroup.get('numberOfChannelOpen').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('numberOfChannels')
          .setValidators([
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 254),
            this.baseUtilService.VALID.required()
          ]);
      } else {
        this.formGroup.get('numberOfChannels').clearValidators();
      }
      this.formGroup.get('numberOfChannels').updateValueAndValidity();
    });
    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('target').disable();
      } else {
        this.formGroup.get('target').enable();
      }
    });
  }

  getDatabaseOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.saponoracleDatabase.value]
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
        const databaseArray = [];

        each(recordsTemp, item => {
          databaseArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });

        this.targetOptions = databaseArray;
        return;
      }
      this.getDatabaseOptions(recordsTemp, startPage);
    });
  }
  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData?.parentUuid || this.resourceData?.parent_uuid
          : this.resourceData?.parentUuid || this.resourceData?.parent_uuid,
      restoreType:
        this.restoreType === RestoreV2Type.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData?.uuid
          : this.formGroup.value.target,
      extendInfo: {
        CHANNELS: this.formGroup.get('numberOfChannelOpen').value
          ? this.formGroup.get('numberOfChannels').value
          : '',
        UUID: this.rowCopy.dbUuid || this.rowCopy.resource_id || ''
      }
    };

    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      assign(params, {
        extendInfo: {
          restoreTimestamp: this.rowCopy.restoreTimeStamp
        }
      });
    }
    return params;
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.resourceData?.name
      : `${
          find(this.targetOptions, {
            value: this.formGroup.value.target
          }).label
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
