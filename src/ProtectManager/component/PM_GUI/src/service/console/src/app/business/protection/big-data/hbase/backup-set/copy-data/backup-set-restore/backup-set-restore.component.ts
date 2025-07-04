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
import { Component, OnInit, Output, EventEmitter, Input } from '@angular/core';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import {
  BaseUtilService,
  I18NService,
  CommonConsts,
  DataMap,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType,
  AgentsSubType,
  HdfsFilesetReplaceOptions,
  ProtectedEnvironmentApiService,
  MODAL_COMMON
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { Observable, Observer } from 'rxjs';
import {
  each,
  isNumber,
  filter,
  assign,
  map,
  toString,
  isEmpty,
  find,
  size,
  intersectionWith,
  split,
  last,
  isEqual,
  includes,
  set
} from 'lodash';
import { MessageService } from '@iux/live';

@Component({
  selector: 'aui-backup-set-restore',
  templateUrl: './backup-set-restore.component.html',
  styleUrls: ['./backup-set-restore.component.less']
})
export class BackupSetRestoreComponent implements OnInit {
  agents = [];
  dataMap = DataMap;
  clusterOptions = [];
  namespaceOptions = [];
  restoreToNewLocationOnly;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  directoryErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_path_error_label')
  };
  capacityThresholdToolTip = this.i18n.get(
    'protection_bigdata_capacity_threshold_tip_label'
  );
  capacityThresholdErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 100])
  };
  filesetReplaceOptions = HdfsFilesetReplaceOptions;

  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  @Output() restoreParamsChange = new EventEmitter();

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.getAgents();
    this.getClusters();
    this.initForm();
    this.updateData();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(this.restoreLocationType.ORIGIN),
      cluster: new FormControl(
        this.rowCopy.environment_uuid ? this.rowCopy.environment_uuid : ''
      ),
      temporary_directory: new FormControl(''),
      namespace: new FormControl(''),
      originalType: new FormControl(this.filesetReplaceOptions.Skip),
      available_capacity_threshold: new FormControl(20, {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 100)
        ]
      })
    });
    setTimeout(() => {
      if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
        this.formGroup
          .get('temporary_directory')
          .setValidators([
            this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath, true)
          ]);
        this.formGroup.get('temporary_directory').updateValueAndValidity();
      }
    }, 500);
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      setTimeout(() => {
        if (
          !res ||
          this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN
        ) {
          return;
        }
        this.getNameSpace(res);
      }, 0);
    });
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) || this.rowCopy.is_replicated;
    this.listenFormGroup();
  }

  updateData() {
    if (this.restoreToNewLocationOnly) {
      setTimeout(() => {
        this.formGroup.get('restoreTo').setValue(this.restoreLocationType.NEW);
      }, 100);
    }
  }

  getNameSpace(clusterId, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE,
      envId: clusterId,
      parentId: clusterId,
      resourceType: DataMap.Resource_Type.HBaseNameSpace.value
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START + 1;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage ===
            Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) + 1 ||
          res.totalCount === 0
        ) {
          const clusterArray = [];
          each(recordsTemp, item => {
            clusterArray.push({
              ...item,
              key: item.uuid,
              value: item.name,
              label: item.extendInfo.nameSpace,
              isLeaf: true
            });
          });
          this.namespaceOptions = clusterArray;
          return;
        }
        this.getNameSpace(clusterId, recordsTemp, startPage);
      });
  }

  listenFormGroup() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === this.restoreLocationType.ORIGIN) {
        this.formGroup.get('cluster').clearValidators();
        this.formGroup.get('namespace').clearValidators();
        this.formGroup
          .get('cluster')
          .setValue(
            this.rowCopy.environment_uuid ? this.rowCopy.environment_uuid : ''
          );
        this.formGroup.get('namespace').setValue('');
        this.namespaceOptions = [];
      } else {
        this.formGroup
          .get('cluster')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('cluster').setValue('');
        this.formGroup.get('namespace').setValue('');
      }
      this.getClusters();
      this.formGroup.get('cluster').updateValueAndValidity();
    });

    this.formGroup.statusChanges.subscribe(res => {
      this.restoreParamsChange.emit(res === 'VALID');
    });
  }

  getAgents(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: 200,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          type: 'Plugin',
          subType:
            this.rowCopy.resource_sub_type ===
            DataMap.Resource_Type.NASFileSystem.value
              ? [AgentsSubType.NasFileSystem]
              : [AgentsSubType.NasShare],
          environment: {
            linkStatus: [
              ['in'],
              toString(DataMap.resource_LinkStatus.normal.value)
            ]
          }
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          this.agents = recordsTemp;
          return;
        }
        this.getAgents(recordsTemp, startPage);
      });
  }

  getClusters(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.HBase.value
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
        const clusterArray = [];
        each(recordsTemp, item => {
          clusterArray.push({
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.clusterOptions = clusterArray;
        return;
      }
      this.getClusters(recordsTemp, startPage);
    });
  }

  getComponent() {
    const requestParams = this.formGroup.value;
    const recoveryOptions = [
      {
        label: this.i18n.get('protection_restore_to_label'),
        value:
          this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN
            ? this.i18n.get('common_restore_to_origin_location_label')
            : this.i18n.get('common_restore_to_new_location_label')
      },
      {
        label: this.i18n.get('common_location_label'),
        value: this.formGroup.value.cluster
      }
    ];

    return {
      recoveryOptions: filter(recoveryOptions, item => {
        return this.formGroup.value.restoreTo ===
          this.restoreLocationType.ORIGIN
          ? item.label === this.i18n.get('protection_restore_to_label')
          : item;
      }),
      requestParams
    };
  }

  getTargetParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      restoreType: this.restoreType,
      targetLocation: this.formGroup.value.restoreTo,
      targetEnv:
        this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN
          ? this.rowCopy.environment_uuid
          : this.formGroup.value.cluster,
      targetObject: isEmpty(this.formGroup.value.namespace)
        ? this.rowCopy.resource_id
        : this.formGroup.value.namespace,
      agents: [],
      extendInfo: {
        restoreOption: this.formGroup.value.originalType
      }
    };
    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      assign(params.extendInfo, {
        temporary_directory: this.formGroup.value.temporary_directory
      });
    }
    const namespace = find(this.namespaceOptions, {
      value: this.formGroup.value.namespace
    });
    const cluster = find(this.clusterOptions, {
      value: this.formGroup.value.cluster
    });
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreTo === this.restoreLocationType.NEW
          ? namespace
            ? {
                ...namespace,
                label: `${cluster?.label}${namespace.name}`
              }
            : {
                label: cluster?.label
              }
          : assign({}, cluster, {
              name: cluster?.label,
              label: cluster?.label
            }),
      requestParams: params
    };
  }

  getTargetPath() {
    return find(this.clusterOptions, {
      value: this.formGroup.value.cluster
    })?.label;
  }

  restore(body): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      let tables = split(
        JSON.parse(this.rowCopy.resource_properties).extendInfo?.table,
        ','
      );
      tables = filter(tables, item => {
        return size(split(item, '/')) > 2;
      });
      if (
        DataMap.Resource_Type.HBaseBackupSet.value === this.childResType &&
        !isEmpty(this.formGroup.value.namespace) &&
        size(
          intersectionWith(tables, (a, b) => {
            return last(split(a, '/')) === last(split(b, '/'));
          })
        ) !== size(tables) &&
        size(tables) >= 2
      ) {
        this.messageService.error(
          this.i18n.get('common_same_table_to_namespace_label'),
          {
            lvShowCloseButton: true,
            lvMessageKey: 'resSameTableMesageKey'
          }
        );
        observer.error('');
        observer.complete();
        return;
      }
      const params = {
        copyId: this.rowCopy.uuid,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        targetEnv:
          this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN
            ? this.rowCopy.environment_uuid
            : this.formGroup.value.cluster,
        targetObject: isEmpty(this.formGroup.value.namespace)
          ? this.rowCopy.resource_id
          : this.formGroup.value.namespace,
        agents: [],
        extendInfo: {
          restoreOption: this.formGroup.value.originalType
        }
      };
      if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
        assign(params, {
          extendInfo: {
            restoreTimestamp: this.rowCopy.restoreTimeStamp,
            restoreOption: this.formGroup.value.originalType,
            temporary_directory: this.formGroup.value.temporary_directory
          }
        });
      }
      set(
        params,
        'extendInfo.available_capacity_threshold',
        Number(this.formGroup.value.available_capacity_threshold)
      );
      this.beforeRestoreCheckTips(params, observer);
    });
  }

  private beforeRestoreCheckTips(params, observer: Observer<void>) {
    if (
      this.childResType === DataMap.Resource_Type.HBaseBackupSet.value &&
      this.formGroup.value.restoreTo === RestoreV2LocationType.NEW
    ) {
      this.showTips(params, observer);
    } else {
      this.postRestoreTask(params, observer);
    }
  }

  private showTips(params, observer: Observer<void>) {
    let tips = isEmpty(this.formGroup.value.namespace)
      ? this.i18n.get('protection_hbase_restore_no_backup_task_tips_label')
      : this.i18n.get(
          'protection_hbase_restore_target_namespace_no_backup_task_tips_label',
          [
            find(this.namespaceOptions, {
              value: this.formGroup.value.namespace
            }).label
          ]
        );
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'hbase-restore-tips-info',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-danger-48',
        lvHeader: this.i18n.get(
          'protection_hbase_restore_no_backup_task_header_label'
        ),
        lvContent: tips,
        lvWidth: 500,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: false,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvOk: () => this.postRestoreTask(params, observer),
        lvCancel: () => {
          observer.error(null);
          observer.complete();
        },
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            observer.error(null);
            observer.complete();
          }
        }
      }
    });
  }

  private postRestoreTask(params, observer: Observer<void>) {
    this.restoreV2Service
      .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
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
  }
}
