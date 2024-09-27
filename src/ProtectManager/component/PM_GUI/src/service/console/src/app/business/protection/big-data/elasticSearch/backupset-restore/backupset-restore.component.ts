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
  OverWriteOption,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreMode,
  RestoreV2LocationType,
  VmFileReplaceStrategy
} from 'app/shared';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  get,
  includes,
  isNumber,
  map,
  set
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
  fileReplaceStrategy = VmFileReplaceStrategy;
  restoreToNewLocationOnly = false;
  clusterOptions = [];
  metadataPathData = [];
  resourceObj;
  clusterUuid;
  constructor(
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.clusterUuid = get(
      JSON.parse(this.rowCopy.resource_properties),
      'root_uuid'
    );
    this.initForm();
    this.getClusters();
  }

  initForm() {
    this.restoreToNewLocationOnly = includes(
      [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ],
      this.rowCopy.generated_by
    );
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originCluster: new FormControl({
        value: this.rowCopy.resource_location,
        disabled: true
      }),
      cluster: new FormControl('', {
        validators: this.baseUtilService.VALID.required()
      }),
      overwriteType: new FormControl(VmFileReplaceStrategy.Skip, {
        validators: this.baseUtilService.VALID.required()
      })
    });

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('cluster').disable();
      } else {
        this.formGroup.get('cluster').enable();
        this.formGroup.get('cluster').setValue('');
      }
    });

    if (this.restoreToNewLocationOnly) {
      defer(() => {
        this.formGroup.patchValue({
          restoreLocation: RestoreV2LocationType.NEW
        });
      });
    }
  }

  getClusters(recordsTemp?: any[], startPage?: number) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.Elasticsearch.value
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
        this.clusterOptions = map(recordsTemp, item => {
          return {
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true,
            ...item
          };
        });

        this.clusterOptions = filter(
          this.clusterOptions,
          item => item.uuid !== this.clusterUuid
        );
        if (!this.restoreToNewLocationOnly) {
          this.formGroup
            .get('restoreLocation')
            .setValue(RestoreV2LocationType.ORIGIN);
        }
        return;
      }
      this.getClusters(recordsTemp, startPage);
    });
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.clusterUuid
          : this.formGroup.value.cluster,
      restoreType:
        this.restoreType === 'CR' ? 'normalRestore' : 'fineGrainedRestore',
      targetLocation: this.formGroup.value.restoreLocation,
      filters: [],
      agents: [],
      extendInfo: {
        restoreOption: this.formGroup.value.overwriteType
      }
    };

    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW) {
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
    return params;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? {
              name: this.rowCopy.resource_location,
              value: this.formGroup.value.originCluster
            }
          : assign(
              {},
              find(this.clusterOptions, {
                value: this.formGroup.value.cluster
              }),
              {
                name: find(this.clusterOptions, {
                  value: this.formGroup.value.cluster
                })?.label
              }
            ),
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.rowCopy.resource_location
      : find(this.clusterOptions, {
          value: this.formGroup.value.cluster
        })?.label;
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
}
