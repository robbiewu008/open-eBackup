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
  DataMapService,
  extendParams,
  I18NService,
  InstanceType,
  OverWriteOption,
  RestoreV2LocationType
} from 'app/shared';
import {
  HostService,
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  assign,
  defer,
  endsWith,
  filter,
  find,
  first,
  get,
  includes,
  isEmpty,
  isNumber,
  isString,
  map,
  set,
  size,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-mysql-restore',
  templateUrl: './mysql-restore.component.html',
  styleUrls: ['./mysql-restore.component.less']
})
export class MysqlRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  copyLocation;
  hostData = [];
  instData = [];
  filterParams = [];
  clusterOptions = [];
  databaseOptions = [];
  dataMap = DataMap;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = OverWriteOption;
  formGroup: FormGroup;
  resourceData;
  location = this.i18n.get('common_location_label');
  isClusterInstance = false;
  showForbiddenStrictMode = false;
  originalLocation;
  disabledOrigin;

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
      'protection_mysql_new_name_error_tips_label'
    )
  };

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    private modal: ModalRef,
    public baseUtilService: BaseUtilService,
    private dataMapService: DataMapService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private hostApiService: HostService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      cluster: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      database: new FormControl(this.rowCopy.resource_id),
      replaceDatabase: new FormControl(false),
      newName: new FormControl(''),
      forceRecovery: new FormControl(false),
      forbiddenStrictMode: new FormControl(false),
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

    if (
      this.rowCopy.resource_sub_type !==
      DataMap.Resource_Type.MySQLDatabase.value
    ) {
      this.formGroup.get('replaceDatabase').disable();
      this.formGroup.get('newName').disable();
      this.formGroup.get('database').disable();
    }
    this.watch();

    if (this.isDrill) {
      defer(() =>
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW)
      );
    }
  }

  validNewName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return { required: { value: control.value } };
      }
      const specialCharacter = [
        '[',
        '@',
        '|',
        ';',
        '&',
        '$',
        '>',
        '<',
        '`',
        '!',
        '+',
        '\\',
        ']',
        '#'
      ];

      if (find(specialCharacter, item => includes(trim(control.value), item))) {
        return { invalidName: { value: control.value } };
      }

      if (
        includes(
          ['information_schema', 'mysql', 'performance_schema', 'sys'],
          trim(control.value)
        )
      ) {
        return { specialNameInvaild: { value: control.value } };
      }

      return null;
    };
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        if (
          this.rowCopy.resource_sub_type ===
          DataMap.Resource_Type.MySQLDatabase.value
        ) {
          this.formGroup.get('database').setValue(this.rowCopy.resource_id);
        }
        this.updateOld();
        this.location = this.i18n.get('common_location_label');
      } else {
        this.updateNew();
        this.location = this.i18n.get('protect_target_instance_label');
      }
    });

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      if (
        this.rowCopy.resource_sub_type ===
        DataMap.Resource_Type.MySQLDatabase.value
      ) {
        if (this.isClusterInstance) {
          this.getClusterInstanceDetail(res);
        } else {
          this.getDatabase(res);
        }
      }
    });

    this.formGroup.get('replaceDatabase').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('newName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64),
            this.validNewName()
          ]);
      } else {
        this.formGroup.get('newName').clearValidators();
      }

      this.formGroup.get('newName').setValue('');
    });
  }

  updateOld() {
    this.formGroup.get('database').clearValidators();
    this.formGroup.get('cluster').setValue(this.originalLocation);
  }

  updateNew() {
    this.formGroup
      .get('database')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup.get('database').setValue('');
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};

    this.showForbiddenStrictMode =
      this.rowCopy.resource_sub_type ===
        DataMap.Resource_Type.MySQLDatabase.value &&
      get(this.resourceData, 'extendInfo.clusterType') ===
        DataMap.Mysql_Cluster_Type.pxc.value;
  }

  getInstanceType() {
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType: [DataMap.Resource_Type.MySQLDatabase.value],
          uuid: [['~~'], this.rowCopy.resource_id]
        })
      })
      .subscribe(res => {
        this.formGroup
          .get('cluster')
          .setValue(get(first(res.records), 'environment.uuid'));

        if (get(first(res.records), 'environment.extendInfo.clusterType')) {
          this.isClusterInstance = true;
        } else {
          this.isClusterInstance = false;
        }
        if (!size(res.records)) {
          this.isClusterInstance =
            this.resourceData?.environment_sub_type ===
            DataMap.Resource_Type.MySQLCluster.value;
        }

        this.getClusters();
      });
  }

  getClusterInstanceDetail(uuid) {
    if (!uuid) {
      return;
    }

    this.protectedResourceApiService
      .ShowResource({ resourceId: uuid })
      .subscribe(res => {
        // 查询children中任意一个节点都可以，结果中的数据库id可能不同，名称相同就没问题
        this.getDatabase(first(get(res, 'dependencies.children', []))['uuid']);
      });
  }

  getClusters(recordsTemp?: any[], startPage?: number, labelParams?: any) {
    const conditions = {
      subType: this.isClusterInstance
        ? [DataMap.Resource_Type.MySQLClusterInstance.value]
        : [DataMap.Resource_Type.MySQLInstance.value]
    };

    set(conditions, 'isTopInstance', InstanceType.TopInstance);
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
        this.clusterOptions = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.path ? `${item.name}(${item.path})` : item.name,
            isLeaf: true
          };
        });

        if (
          this.rowCopy.resource_sub_type ===
            DataMap.Resource_Type.MySQLClusterInstance.value &&
          this.resourceData.extendInfo?.clusterType ===
            DataMap.Mysql_Cluster_Type.eapp.value
        ) {
          this.clusterOptions = filter(this.clusterOptions, item => {
            return (
              size(this.resourceData.path?.split(',')) ===
              size(item.path?.split(','))
            );
          });
        }

        this.clusterOptions = filter(this.clusterOptions, item => {
          let sameVaild = true;

          if (!!this.resourceData?.version) {
            if (
              this.resourceData?.version !== item.version &&
              !endsWith(this.resourceData.version, item.version) &&
              !endsWith(item.version, this.resourceData.version)
            ) {
              sameVaild = false;
            }
          }

          if (
            !!this.resourceData?.extendInfo?.deployOperatingSystem &&
            this.resourceData?.extendInfo?.deployOperatingSystem !==
              item?.extendInfo?.deployOperatingSystem
          ) {
            sameVaild = false;
          }

          if (this.isClusterInstance) {
            if (
              this.rowCopy.resource_sub_type ===
                DataMap.Resource_Type.MySQLClusterInstance.value &&
              !!this.resourceData?.extendInfo?.clusterType &&
              this.resourceData?.extendInfo?.clusterType !==
                item.environment?.extendInfo?.clusterType
            ) {
              sameVaild = false;
            }

            if (
              this.rowCopy.resource_sub_type ===
                DataMap.Resource_Type.MySQLDatabase.value &&
              find(
                this.clusterOptions,
                item => item.parentUuid === this.resourceData?.root_uuid
              ) &&
              get(
                find(
                  this.clusterOptions,
                  item => item.parentUuid === this.resourceData?.root_uuid
                ),
                'extendInfo.clusterType'
              ) !== item.environment?.extendInfo?.clusterType
            ) {
              sameVaild = false;
            }
          }

          return sameVaild;
        });

        this.formGroup.updateValueAndValidity();

        if (
          this.rowCopy.resource_sub_type ===
          DataMap.Resource_Type.MySQLDatabase.value
        ) {
          this.originalLocation = get(
            find(
              this.clusterOptions,
              item => item.parentUuid === this.formGroup.value.cluster
            ),
            'value'
          );
          this.formGroup.get('cluster').setValue(this.originalLocation);
        } else {
          this.originalLocation = this.rowCopy.resource_id;
        }
        this.formGroup.get('cluster').setValue(this.originalLocation);

        if (
          !find(
            this.clusterOptions,
            item => item.uuid === this.formGroup.value.cluster
          ) ||
          this.rowCopy?.generated_by ===
            DataMap.CopyData_generatedType.cascadedReplication.value
        ) {
          this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
          this.formGroup.get('cluster').setValue('');
          this.disabledOrigin = true;
        }
        this.updateDrillData();
        return;
      }
      this.getClusters(recordsTemp, startPage, labelParams);
    });
  }

  getDatabase(parentUuid, recordsTemp?: any[], startPage?: number) {
    if (isEmpty(parentUuid)) {
      return;
    }
    const conditions = {
      subType: [DataMap.Resource_Type.MySQLDatabase.value],
      parentUuid: parentUuid
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
        this.databaseOptions = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          };
        });

        const sameDatabase = find(
          this.databaseOptions,
          item => item.name === this.rowCopy.resource_name
        );

        if (!!sameDatabase) {
          this.databaseOptions = [sameDatabase];
        }
        return;
      }
      this.getDatabase(parentUuid, recordsTemp, startPage);
    });
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? find(
          this.clusterOptions,
          item => item.uuid === this.formGroup.value.cluster
        )['label']
      : `${
          find(
            this.clusterOptions,
            item => item.uuid === this.formGroup.value.cluster
          )['label']
        }`;
  }

  getParams() {
    const tagetObj = find(this.clusterOptions, {
      value: this.formGroup.value.cluster
    });
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv: includes(
        [
          DataMap.Resource_Type.MySQLInstance.value,
          DataMap.Resource_Type.MySQLClusterInstance.value
        ],
        this.rowCopy.resource_sub_type
      )
        ? tagetObj?.parentUuid
        : tagetObj?.rootUuid,
      scripts: {
        preScript: this.formGroup.value.preScript,
        postScript: this.formGroup.value.postScript,
        failPostScript: this.formGroup.value.executeScript
      },
      restoreType: this.restoreType,
      targetLocation: this.formGroup.value.restoreTo,
      targetObject:
        this.rowCopy.resource_sub_type ===
        DataMap.Resource_Type.MySQLDatabase.value
          ? this.formGroup.value.database
          : this.formGroup.value.cluster
    };
    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      assign(params, {
        extendInfo: {
          restoreTimestamp: this.rowCopy.restoreTimeStamp
        }
      });
    }

    if (this.formGroup.value.replaceDatabase) {
      set(params, 'extendInfo.newDatabaseName', this.formGroup.value.newName);
    }

    set(
      params,
      'extendInfo.forceRecovery',
      this.formGroup.value.forceRecovery ? 1 : 0
    );

    if (this.showForbiddenStrictMode) {
      set(
        params,
        'extendInfo.forbiddenStrictMode',
        this.formGroup.value.forbiddenStrictMode ? 1 : 0
      );
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
    this.getResourceData();
    this.initForm();
    if (
      includes(
        [
          DataMap.Resource_Type.MySQLInstance.value,
          DataMap.Resource_Type.MySQLClusterInstance.value
        ],
        this.rowCopy.resource_sub_type
      )
    ) {
      this.rowCopy.resource_sub_type ===
      DataMap.Resource_Type.MySQLClusterInstance.value
        ? (this.isClusterInstance = true)
        : (this.isClusterInstance = false);

      this.getClusters();
    } else {
      this.getInstanceType();
    }
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      if (
        includes(
          [
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.MySQLClusterInstance.value
          ],
          this.rowCopy.resource_sub_type
        )
      ) {
        this.formGroup.get('cluster').setValue(config.targetObject);
      } else {
        const cluster = find(this.clusterOptions, {
          rootUuid: config.targetEnv
        });
        this.formGroup.get('cluster').setValue(cluster?.uuid);
        this.formGroup.get('database').setValue(config.targetObject);
        this.formGroup
          .get('replaceDatabase')
          .setValue(!isEmpty(config?.extendInfo.newDatabaseName));
        this.formGroup
          .get('newName')
          .setValue(config?.extendInfo.newDatabaseName);
        this.formGroup
          .get('forceRecovery')
          .setValue(config.extendInfo?.forceRecovery === 1);
        if (this.showForbiddenStrictMode) {
          this.formGroup
            .get('forbiddenStrictMode')
            .setValue(config.extendInfo?.forbiddenStrictMode === 1);
        }
      }
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getClusters(null, null, event);
  }
}
