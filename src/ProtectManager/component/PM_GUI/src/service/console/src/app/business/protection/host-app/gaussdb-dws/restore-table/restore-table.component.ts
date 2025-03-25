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
import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import {
  filter,
  find,
  get,
  isEmpty,
  isNumber,
  isString,
  map,
  set
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-table-restore',
  templateUrl: './restore-table.component.html',
  styleUrls: ['./restore-table.component.less']
})
export class TableRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  @Output() onStatusChange = new EventEmitter<any>();

  dataMap = DataMap;
  typeRestore = RestoreType;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  clusterOptions = [];
  databaseOptions = [];
  resourceData;
  isGDSBackup;
  placeHolder = this.i18n.get('common_select_label');
  disableOriginLocation = false;
  tip = this.i18n.get('protection_cloud_origin_restore_disabled_label');
  schemaTip = this.i18n.get('explore_cluster_not_exist_label');

  constructor(
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
    this.getResourceData();
    this.initForm();

    if (!this.disableOriginLocation) {
      // 存在原位置资源才获取原位置是否允许恢复
      this.getRestoreLimit();
    } else {
      this.getClusters();
    }

    this.disableOkBtn();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      position: new FormControl({
        value:
          this.rowCopy?.resource_environment_name ||
          this.rowCopy?.resource_location,
        disabled: true
      }),
      cluster: new FormControl(''),
      database: new FormControl('')
    });

    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      this.formGroup.get('cluster').setValue(this.resourceData?.root_uuid);
      if (res === RestoreV2LocationType.ORIGIN) {
        this.placeHolder = this.i18n.get(
          'protection_cloud_origin_restore_disabled_label'
        );
        this.formGroup.get('database').clearValidators();
        this.formGroup.get('cluster').clearValidators();
      } else {
        this.placeHolder = this.i18n.get('common_select_label');
        this.formGroup
          .get('cluster')
          .setValidators([this.baseUtilService.VALID.required()]);

        if (this.rowCopy.resource_sub_type === 'DWS-table') {
          this.formGroup
            .get('database')
            .setValidators([this.baseUtilService.VALID.required()]);
        }
      }
      this.formGroup.get('database').updateValueAndValidity();
      this.formGroup.get('cluster').updateValueAndValidity();
    });

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.formGroup.get('database').setValue('');
      this.getDatabase();
    });

    this.formGroup.statusChanges.subscribe(res => {
      this.disableOkBtn();
      if (this.restoreType !== RestoreType.CommonRestore) {
        this.onStatusChange.emit();
      }
    });

    if (this.disableOriginLocation) {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
    }
    if (
      this.restoreType !== RestoreType.CommonRestore &&
      this.rowCopy.resource_sub_type === DataMap.Resource_Type.DWS_Schema.value
    ) {
      this.formGroup.patchValue({
        cluster: this.resourceData?.root_uuid,
        database: this.resourceData?.parent_uuid
      });
    }
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};

    this.isGDSBackup =
      this.resourceData.ext_parameters.backup_tool_type ===
      DataMap.Backup_Type.GDS.value;

    if (
      !this.isGDSBackup &&
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.placeHolder = this.i18n.get(
        'protection_cloud_origin_restore_disabled_label'
      );
    }
  }

  getRestoreLimit() {
    this.protectedResourceApiService
      .CheckAllowRestore({
        resourceIds: String(this.resourceData?.uuid)
      })
      .subscribe(res => {
        const isAllowRestore = get(res[0], 'isAllowRestore', 'false');
        if (isAllowRestore === 'false') {
          this.tip = this.i18n.get('protection_origin_disable_restore_label');
          this.schemaTip = this.i18n.get('explore_disable_old_cluster_label');
          this.disableOriginLocation = true;
          this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
          this.formGroup.get('cluster').setValue('');
        }
        this.getClusters();
      });
  }

  getClusters(recordsTemp?: any[], startPage?: number) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.DWS_Cluster.value
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
        const version =
          get(
            find(
              recordsTemp,
              item => item.uuid === this.resourceData?.root_uuid
            ),
            'version'
          ) ||
          JSON.parse(this.rowCopy.properties)?.version ||
          JSON.parse(this.rowCopy.properties)?.extendInfo?.version ||
          JSON.parse(this.rowCopy.properties)?.extendInfo?.extendInfo?.version;

        if (
          this.rowCopy.resource_sub_type !==
          DataMap.Resource_Type.DWS_Schema.value
        ) {
          recordsTemp = filter(recordsTemp, item => item?.version === version);
        }
        this.clusterOptions = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name || '',
            isLeaf: true,
            disabled:
              get(item, 'extendInfo.isAllowRestore', 'false') === 'false'
          };
        });

        return;
      }
      this.getClusters(recordsTemp, startPage);
    });
  }

  getDatabase(recordsTemp?: any[], startPage?: number) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.DWS_Database.value,
        parentUuid: [['=='], this.formGroup.value.cluster]
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
        this.databaseOptions = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name || '',
            isLeaf: true
          };
        });
        return;
      }
      this.getDatabase(recordsTemp, startPage);
    });
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resourceData?.root_uuid
          : this.formGroup.value.cluster,
      restoreType:
        this.restoreType === RestoreType.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreTo,
      extendInfo: {
        resourceSubType: this.rowCopy.resource_sub_type
      }
    };

    if (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW) {
      set(
        params,
        'targetObject',
        get(
          find(
            this.clusterOptions,
            item => item.uuid === this.formGroup.value.cluster
          ),
          'name'
        )
      );
    }

    if (
      this.formGroup.value.restoreTo === RestoreV2LocationType.NEW &&
      this.childResType === DataMap.Resource_Type.DWS_Table.value
    ) {
      set(
        params,
        'extendInfo.database',
        find(
          this.databaseOptions,
          item => item.uuid === this.formGroup.value.database
        )['label']
      );
    }
    return params;
  }

  getTargetParams() {
    if (isEmpty(this.clusterOptions) || isEmpty(this.databaseOptions)) {
      return;
    }
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
                this.clusterOptions,
                item => item.uuid === this.formGroup.value.cluster
              )['label'],
              value: this.formGroup.value.cluster
            },
      restoreLocation: this.formGroup.value.restoreTo,
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? this.rowCopy?.resource_environment_name ||
          this.rowCopy?.resource_location
      : find(
          this.clusterOptions,
          item => item.uuid === this.formGroup.value.cluster
        )['label'];
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
    if (
      this.rowCopy?.resource_status ===
        DataMap.Resource_Status.notExist.value &&
      this.rowCopy.resource_sub_type === DataMap.Resource_Type.DWS_Table.value
    ) {
      this.modal.getInstance().lvOkDisabled = true;
      return;
    }
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }
}
