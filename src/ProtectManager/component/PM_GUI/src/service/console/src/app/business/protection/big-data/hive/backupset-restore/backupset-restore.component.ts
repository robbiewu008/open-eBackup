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
  OverWriteOption,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType,
  I18NService,
  ProtectedEnvironmentApiService,
  HdfsFilesetReplaceOptions,
  RestoreMode
} from 'app/shared';
import {
  assign,
  cloneDeep,
  each,
  find,
  first,
  isArray,
  isEmpty,
  isNumber,
  map,
  reject,
  set,
  size
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-backupset-restore',
  templateUrl: './backupset-restore.component.html',
  styleUrls: ['./backupset-restore.component.less']
})
export class BackupsetRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = HdfsFilesetReplaceOptions;
  restoreToNewLocationOnly = false;
  clusterOptions = [];
  backupSetOptions = [];
  metadataPathData = [];
  resourceObj;
  clusterUuid: any;
  resource_properties;
  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.restoreToNewLocationOnly =
      this.rowCopy?.resource_status ===
        DataMap.Resource_Status.notExist.value ||
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.cascadedReplication.value;
    this.resource_properties = JSON.parse(
      this.rowCopy?.resource_properties || '{}'
    );
    this.initForm();
    this.getClusters();
    this.disableOkBtn();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originCluster: new FormControl({
        value: '',
        disabled: true
      }),
      cluster: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      metadataPath: new FormControl([], {
        validators: this.baseUtilService.VALID.required()
      }),
      overwriteType: new FormControl(HdfsFilesetReplaceOptions.Overwrite, {
        validators: this.baseUtilService.VALID.required()
      }),
      tableFileOverwriteType: new FormControl(true),
      tableNewLocationPath: new FormControl([])
    });

    this.listenForm();

    if (this.restoreToNewLocationOnly) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
    } else {
      this.formGroup
        .get('restoreLocation')
        .setValue(RestoreV2LocationType.ORIGIN);
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      this.formGroup.get('cluster').setValue('');

      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('cluster').disable();

        const selectCluster = cloneDeep(
          find(this.clusterOptions, {
            uuid: this.resource_properties?.environment_uuid
          })
        );
        this.formGroup.get('originCluster').setValue(selectCluster?.name);
        assign(selectCluster, {
          label: selectCluster?.name,
          children: [],
          isLeaf: false
        });
        if (!!selectCluster?.name) {
          this.metadataPathData = [selectCluster];
          this.clusterUuid = selectCluster.uuid;
        }
      } else {
        this.formGroup.get('cluster').enable();
        this.metadataPathData = [];
      }
    });

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      this.formGroup.get('metadataPath').setValue([]);
      this.formGroup.get('tableNewLocationPath').setValue([]);

      if (res === '') {
        return;
      }

      const selectCluster = cloneDeep(find(this.clusterOptions, { uuid: res }));
      assign(selectCluster, {
        label: selectCluster?.name,
        children: [],
        isLeaf: false
      });
      if (!!selectCluster.label) {
        this.metadataPathData = [selectCluster];
        this.clusterUuid = selectCluster.uuid;
      }
    });

    this.formGroup.get('tableFileOverwriteType').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup
          .get('tableNewLocationPath')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('tableNewLocationPath').clearValidators();
      }
      this.formGroup.get('tableNewLocationPath').updateValueAndValidity();
    });
  }

  getClusters(recordsTemp?: any[], startPage?: number) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: [
          DataMap.Resource_Type.Hive.value,
          DataMap.Resource_Type.HiveBackupSet.value
        ]
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
        each(recordsTemp, item => {
          if (item.subType === DataMap.Resource_Type.Hive.value) {
            this.clusterOptions.push({
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true
            });
          } else if (
            item.subType === DataMap.Resource_Type.HiveBackupSet.value
          ) {
            this.backupSetOptions.push({
              ...item
            });
          }
        });
        this.clusterOptions = [...this.clusterOptions];
        this.backupSetOptions = [...this.backupSetOptions];
        const selectCluster =
          cloneDeep(
            find(this.clusterOptions, {
              uuid: this.resource_properties?.environment_uuid
            })
          ) || {};
        assign(selectCluster, {
          label: selectCluster?.name,
          children: [],
          isLeaf: false
        });
        if (!!selectCluster.label) {
          this.metadataPathData = [selectCluster];
          this.clusterUuid = selectCluster.uuid;
        }
        if (
          this.formGroup.getRawValue().restoreLocation ===
          RestoreV2LocationType.ORIGIN
        ) {
          const originalName =
            find(this.clusterOptions, {
              uuid: this.resource_properties?.environment_uuid
            })['label'] || this.rowCopy.resource_location;
          this.formGroup.get('originCluster').setValue(originalName);
        }
        return;
      }
      this.getClusters(recordsTemp, startPage);
    });
  }
  expandedChange(node) {
    if (!node.expanded) {
      return;
    }
    this.getClusterResource(node);
  }

  getClusterResource(node, startPage?: number) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      envId: this.clusterUuid,
      parentId: node.extendInfo.path || '/',
      resourceType: 'HiveDirSet'
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        each(res.records, item => {
          assign(item, {
            rootPath: node.rootPath
              ? `${node.rootPath}/${item.extendInfo?.path}`
              : `/${item.extendInfo?.path}`,
            label: item.name,
            isLeaf: false
          });
        });
        if (isArray(node.children) && !isEmpty(node.children)) {
          node.children = [
            ...reject(node.children, n => {
              return n.isMoreBtn;
            }),
            ...res.records
          ];
        } else {
          node.children = [...res.records];
        }
        if (res.totalCount > size(node.children)) {
          const moreClickNode = {
            label: `${this.i18n.get('common_more_label')}...`,
            isMoreBtn: true,
            isLeaf: true,
            disabled: true,
            startPage: Math.floor(size(node.children) / 200) + 1
          };
          node.children = [...node.children, moreClickNode];
        }
        this.metadataPathData = [...this.metadataPathData];
      });
  }

  getParams() {
    const tempPath: any = first(this.formGroup.value.metadataPath) || {};
    const tableLocation: any =
      first(this.formGroup.value.tableNewLocationPath) || {};

    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv: this.clusterUuid,
      restoreType:
        this.restoreType === 'CR' ? 'normalRestore' : 'fineGrainedRestore',
      targetLocation: this.formGroup.value.restoreLocation,
      filters: [],
      agents: [],
      extendInfo: {
        restoreTempPath: tempPath.extendInfo.path || '/',
        restoreOption: this.formGroup.value.overwriteType,
        isDeleteTableFile: this.formGroup.value.tableFileOverwriteType,
        tableNewLocation: !this.formGroup.value.tableFileOverwriteType
          ? tableLocation.extendInfo?.path || '/'
          : ''
      }
    };

    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW) {
      set(
        params,
        'targetObject',
        find(this.clusterOptions, item => item.uuid === this.clusterUuid)[
          'name'
        ]
      );
    }
    return params;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW
          ? find(this.clusterOptions, {
              value: this.clusterUuid
            }) || {}
          : assign(
              {},
              find(this.clusterOptions, {
                value: this.clusterUuid
              }),
              {
                name: find(this.clusterOptions, {
                  value: this.clusterUuid
                })?.label
              }
            ),
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    const tempPath: any = first(this.formGroup.value.metadataPath) || {};

    return `${
      find(this.clusterOptions, {
        value: this.clusterUuid
      })?.label
    }: ${tempPath.extendInfo.path || '/'}`;
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

  // Hive恢复时需要提示恢复的备份集
  getClusterName() {
    const result = find(this.backupSetOptions, {
      uuid: this.resource_properties.uuid
    });
    return result ? [result['name']] : [this.resource_properties.name];
  }
}
