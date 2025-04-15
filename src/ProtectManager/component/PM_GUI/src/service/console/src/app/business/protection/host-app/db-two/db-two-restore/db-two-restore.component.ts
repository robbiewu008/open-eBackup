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
  OverWriteOption,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  each,
  filter,
  find,
  get,
  isEmpty,
  isNumber,
  isString,
  map,
  set,
  size,
  split,
  uniqBy
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-db-two-restore',
  templateUrl: './db-two-restore.component.html',
  styleUrls: ['./db-two-restore.component.less']
})
export class DbTwoRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  copyLocation;
  clusterType;
  hostData = [];
  instData = [];
  filterParams = [];
  options = [];
  instanceOptions = [];
  databaseOptions = [];
  dataMap = DataMap;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = OverWriteOption;
  formGroup: FormGroup;
  resourceData;
  location = this.i18n.get('common_location_label');
  isClusterInstance = false;
  originalLocation;
  disableOriginLocation = false;
  connectTipsLabel = null;
  readonly PAGESIZE = CommonConsts.PAGE_SIZE * 10;

  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  newNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('protection_new_name_error_tips_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64]),
    specialNameInvaild: this.i18n.get(
      'protection_sqlserver_new_name_error_tips_label'
    )
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    pathError: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048])
  };

  constructor(
    public i18n: I18NService,
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
      originalHost: new FormControl({
        value:
          this.resourceData.environment_name ||
          this.resourceData.environment?.name,
        disabled: true
      }),
      host: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      instance: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      database: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      failOnPit: new FormControl(false),
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
    if (this.disableOriginLocation) {
      if (
        this.rowCopy?.resource_sub_type ===
        DataMap.Resource_Type.dbTwoTableSet.value
      ) {
        this.modal.getInstance().lvOkDisabled = true;
      } else {
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
      }
    } else {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.ORIGIN);
    }
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      this.instanceOptions = [];
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('host').disable();
        this.formGroup.get('instance').disable();
        this.formGroup.get('database').disable();
        this.location = this.i18n.get('common_location_label');
      } else {
        this.formGroup.get('host').enable();
        this.formGroup.get('instance').enable();
        this.formGroup.get('database').enable();
        this.location = this.i18n.get('explore_target_host_cluster_label');
      }

      this.formGroup.get('host').setValue('');
    });

    this.formGroup.get('host').valueChanges.subscribe(res => {
      this.formGroup.get('instance').setValue('');
      this.formGroup.get('database').setValue('');
      this.getInstance();
    });

    this.formGroup.get('instance').valueChanges.subscribe(res => {
      this.formGroup.get('database').setValue('');
      this.getDatabase();
    });

    this.formGroup.statusChanges.subscribe(res => {
      this.disableOkBtn();
    });
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    if (
      this.rowCopy?.resource_sub_type ===
      DataMap.Resource_Type.dbTwoDatabase.value
    ) {
      this.connectTipsLabel = this.i18n.get(
        'protection_db_two_retsore_connect_database_tips_label'
      );
    } else if (
      this.rowCopy?.resource_sub_type ===
      DataMap.Resource_Type.dbTwoTableSet.value
    ) {
      this.connectTipsLabel = this.i18n.get(
        'protection_db_two_retsore_connect_tablespace_tips_label'
      );
    }
  }

  getClusters(recordsTemp?, startPage?, labelParams?: any) {
    const conditions = {
      subType: [
        DataMap.Resource_Type.dbTwoClusterInstance.value,
        DataMap.Resource_Type.dbTwoInstance.value
      ],
      isTopInstance: InstanceType.TopInstance
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
            key: item?.environment.uuid,
            value: item?.environment.uuid,
            label:
              item?.subType === DataMap.Resource_Type.dbTwoClusterInstance.value
                ? item?.environment.name
                : `${item?.environment.name}(${item?.environment.endpoint})`,
            isLeaf: true
          });
        });

        this.options = uniqBy(clusterArray, 'key');

        // 过滤掉类型，操作系统和版本不同的集群或主机
        this.clusterType = get(this.resourceData, 'extendInfo.clusterType');
        const deployOperatingSystem = get(
          this.resourceData,
          'extendInfo.deployOperatingSystem'
        );
        const version = get(this.resourceData, 'version');

        if (this.clusterType && deployOperatingSystem && version) {
          this.options = filter(this.options, item => {
            if (this.clusterType === 'single') {
              return (
                !get(item, 'environment.extendInfo.clusterType') &&
                get(item, 'extendInfo.deployOperatingSystem') ===
                  deployOperatingSystem &&
                item.version === version
              );
            } else {
              return (
                get(item, 'environment.extendInfo.clusterType') ===
                  this.clusterType &&
                get(item, 'extendInfo.deployOperatingSystem') ===
                  deployOperatingSystem &&
                item.version === version
              );
            }
          });
        }

        // 过滤掉节点数不一致的集群或主机
        const nodeSize = size(
          split(
            get(this.resourceData, 'environment_endpoint', '') ||
              get(this.resourceData, 'environment.endpoint', ''),
            ','
          )
        );

        this.options = filter(this.options, item => {
          return (
            size(split(get(item, 'environment.endpoint', ''), ',')) === nodeSize
          );
        });
        this.updateDrillData();
        return;
      }
      this.getClusters(recordsTemp, startPage, labelParams);
    });
  }

  getInstance(recordsTemp?: any[], startPage?: number) {
    if (!this.formGroup.value.host) {
      this.instanceOptions = [];
      return;
    }
    const conditions = {
      subType: [
        DataMap.Resource_Type.dbTwoClusterInstance.value,
        DataMap.Resource_Type.dbTwoInstance.value
      ],
      isTopInstance: InstanceType.TopInstance,
      rootUuid: this.formGroup.value.host
    };

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
        this.instanceOptions = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          };
        });

        // 只保留同名实例
        const instanceName =
          get(this.resourceData, 'parent_name') ||
          get(this.resourceData, 'parentName');
        this.instanceOptions = filter(this.instanceOptions, item => {
          return item.name === instanceName;
        });
        return;
      }
      this.getInstance(recordsTemp, startPage);
    });
  }

  getDatabase(recordsTemp?, startPage?) {
    if (!this.formGroup.value.instance) {
      this.databaseOptions = [];
      return;
    }

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: this.PAGESIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.dbTwoDatabase.value,
        parentUuid: this.formGroup.value.instance
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
        this.databaseOptions = databaseArray;

        // 过滤掉原位置数据库
        const originalDatabase = get(this.resourceData, 'uuid');
        this.databaseOptions = filter(this.databaseOptions, item => {
          return item.uuid !== originalDatabase;
        });

        // HADR类型只保留同名数据库
        const clusterType = get(this.resourceData, 'extendInfo.clusterType');
        if (clusterType === DataMap.dbTwoType.hadr.value) {
          const databaseName = get(this.resourceData, 'name');
          this.databaseOptions = filter(this.databaseOptions, item => {
            return item.name === databaseName;
          });
        }

        //
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
          ? this.resourceData.environment_uuid ||
            this.resourceData.environment?.uuid
          : this.formGroup.value.host,
      restoreType:
        this.restoreType === RestoreV2Type.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreTo,
      scripts: {
        preScript: this.formGroup.value.preScript,
        postScript: this.formGroup.value.postScript,
        failPostScript: this.formGroup.value.executeScript
      }
    };

    if (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW) {
      set(params, 'targetObject', this.formGroup.value.database);
    }

    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      set(params, 'extendInfo.restoreTimestamp', this.rowCopy.restoreTimeStamp);

      if (
        this.clusterType === DataMap.dbTwoType.dpf.value &&
        !!this.rowCopy?.restoreTimeStamp
      ) {
        set(
          params,
          'extendInfo.failOnPit',
          this.formGroup.value.failOnPit ? 1 : 0
        );
      }
    }

    return params;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      originalHost: this.formGroup.get('originalHost')?.value,
      resource:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? {
              name:
                this.resourceData.environment_name ||
                this.resourceData.environment?.name,
              value:
                this.resourceData.environment_uuid ||
                this.resourceData.environment?.uuid
            }
          : {
              name: find(
                this.options,
                item => item.uuid === this.formGroup.value.host
              )['label'],
              value: this.formGroup.value.host
            },
      restoreLocation: this.formGroup.value.restoreTo,
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? this.resourceData.environment_name ||
          this.resourceData.environment?.name
      : `${
          find(this.options, item => item.value === this.formGroup.value.host)[
            'label'
          ]
        }/${
          find(
            this.instanceOptions,
            item => item.uuid === this.formGroup.value.instance
          )['label']
        }`;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.restoreV2Service
        .CreateRestoreTask({
          CreateRestoreTaskRequestBody: this.getParams() as any
        })
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

    this.getResourceData();
    this.initForm();
    this.getClusters();
    this.disableOkBtn();
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('host').setValue(config.targetEnv);
      // 查询数据库获取目标实例
      this.protectedResourceApiService
        .ShowResource({ resourceId: config.targetObject, akDoException: false })
        .subscribe(res => {
          this.formGroup.get('instance').setValue(res.parentUuid);
          this.formGroup.get('database').setValue(config.targetObject);
        });
      if (
        this.clusterType === this.dataMap.dbTwoType.dpf.value &&
        this.rowCopy?.backup_type ===
          this.dataMap.CopyData_Backup_Type.log.value &&
        !!this.rowCopy?.restoreTimeStamp
      ) {
        this.formGroup
          .get('failOnPit')
          .setValue(config.extendInfo?.failOnPit === 1);
      }
    }
  }
}
