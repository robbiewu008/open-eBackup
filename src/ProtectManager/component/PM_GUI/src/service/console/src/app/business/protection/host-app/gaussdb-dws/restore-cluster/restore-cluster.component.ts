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
  compareVersion,
  DataMap,
  HdfsFilesetReplaceOptions,
  I18NService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import { assign, filter, find, get, isNumber, map, set } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-cluster-restore',
  templateUrl: './restore-cluster.component.html',
  styleUrls: ['./restore-cluster.component.less']
})
export class ClusterRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = HdfsFilesetReplaceOptions;
  restoreToNewLocationOnly = false;
  clusterOptions = [];
  resourceObj;
  clusterUuid: any;
  originalClusterUuid;
  resourceData;
  disableOriginLocation = false;
  isOriginAllowRestore = true;
  tip = this.i18n.get('protection_cloud_origin_restore_disabled_label');

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
    this.resourceData = JSON.parse(this.rowCopy.resource_properties) || {};
    this.initForm();
    if (!this.disableOriginLocation) {
      // 存在原资源才去获取原位置是否允许恢复
      this.getRestoreLimit();
    } else {
      this.getClusters();
    }
    this.disableOkBtn();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originCluster: new FormControl({
        value:
          this.rowCopy?.resource_environment_name ||
          this.rowCopy?.resource_location,
        disabled: true
      }),
      cluster: new FormControl(
        { value: '', disabled: true },
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      )
    });

    this.listenForm();

    if (this.disableOriginLocation) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('cluster').disable();
      } else {
        this.formGroup.get('cluster').enable();
      }

      if (
        this.restoreType === RestoreType.CommonRestore ||
        this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
      ) {
        this.formGroup.get('cluster').setValue('');
      } else {
        this.formGroup
          .get('cluster')
          .setValue(get(this.resourceData, 'root_uuid'));
      }
    });
  }

  getRestoreLimit() {
    this.protectedResourceApiService
      .CheckAllowRestore({
        resourceIds: String(this.resourceData?.uuid)
      })
      .subscribe(res => {
        const isAllowRestore = get(res[0], 'isAllowRestore', 'false');
        if (isAllowRestore === 'false') {
          this.disableOriginLocation = true;
          this.tip = this.i18n.get('protection_origin_disable_restore_label');
          this.isOriginAllowRestore = false;
          this.formGroup
            .get('restoreLocation')
            .setValue(RestoreV2LocationType.NEW);
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
        if (this.restoreType === RestoreType.CommonRestore) {
          recordsTemp = filter(
            recordsTemp,
            item => item?.version === this.resourceData?.version
          );

          this.clusterOptions = filter(
            map(recordsTemp, item => {
              return {
                ...item,
                key: item.uuid,
                value: item.uuid,
                label: item.name,
                isLeaf: true,
                disabled:
                  get(item, 'extendInfo.isAllowRestore', 'false') === 'false'
              };
            }),
            item => item.uuid !== get(this.resourceData, 'root_uuid')
          );
        } else {
          // 表级恢复的新位置恢复：目标集群只展示版本号大于等于9.1.0版本的，且保留原集群因为需要能新建schema恢复
          recordsTemp = filter(
            recordsTemp,
            item =>
              compareVersion(item?.version, '9.1.0') !== -1 ||
              item.uuid === get(this.resourceData, 'root_uuid')
          );
          this.clusterOptions = map(recordsTemp, item => {
            return {
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true,
              disabled:
                get(item, 'extendInfo.isAllowRestore', 'false') === 'false'
            };
          });
        }

        this.formGroup.get('originCluster').setValue(
          get(
            find(
              this.clusterOptions,
              item => item.uuid === get(this.resourceData, 'root_uuid')
            ),
            'name'
          ) ||
            this.rowCopy?.resource_environment_name ||
            this.rowCopy?.resource_location
        );
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
          ? get(this.resourceData, 'root_uuid')
          : this.formGroup.value.cluster,
      restoreType:
        this.restoreType === RestoreType.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      extendInfo: {
        resourceSubType: this.rowCopy.resource_sub_type
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
              name:
                this.rowCopy?.resource_environment_name ||
                this.rowCopy?.resource_location,
              value: get(this.resourceData, 'root_uuid')
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
      ? this.rowCopy?.resource_environment_name ||
          this.rowCopy?.resource_location
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

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }
}
