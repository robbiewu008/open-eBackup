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
import { FormGroup } from '@angular/forms';
import {
  ApplicationType,
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  MultiCluster,
  NasDistributionStoragesApiService,
  QosService,
  StorageUnitService,
  StorageUserAuthService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { filter, find, get, isEqual, map, size } from 'lodash';
@Component({
  selector: 'aui-specify-destination-location',
  templateUrl: './specify-destination-location.component.html',
  styleUrls: ['./specify-destination-location.component.less']
})
export class SpecifyDestinationLocationComponent implements OnInit {
  find = find;
  size = size;
  specialClassName;
  backupStorageUnitNames = [];
  backupStorageUnitGroupNames = [];
  applicationType = '';
  showDestination = true;
  @Input() isSlaDetail: boolean;
  @Input() data: any;
  @Input() action: any;
  @Input() formGroup: FormGroup;
  @Input() isRetry: boolean;
  @Input() isUsed: boolean;
  @Output() isDisableQos = new EventEmitter<any>();
  backupStorageTypes = this.dataMapService.toArray(
    'storagePoolBackupStorageType'
  );
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  constructor(
    public baseUtilService: BaseUtilService,
    public i18n: I18NService,
    private qosServiceApi: QosService,
    private cookieService: CookieService,
    public dataMapService: DataMapService,
    private clusterApiService: ClustersApiService,
    private nasDistributionStoragesApiService: NasDistributionStoragesApiService,
    private storageUnitService: StorageUnitService,
    private storageUserAuthService: StorageUserAuthService,
    public appUtilsService?: AppUtilsService
  ) {}

  ngOnInit(): void {
    this.formGroup
      .get('storage_id')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('storage_type')
      .setValidators([this.baseUtilService.VALID.required()]);

    this.getBackupStorageNames();
    if (!this.isSlaDetail) {
      this.updateData();
      this.listenChanges();
      this.applicationType = get(this.data, 'applicationData');
    } else {
      this.applicationType = get(this.data[0], 'applicationData');
    }
  }

  updateData() {
    // 如果是升级场景，则不校验指定目标位置,之前非必选时不同的资源同一SLA可能对应不同存储单元，不能塞值
    if (!this.formGroup.get('storage_id').value && !!this.isUsed) {
      this.formGroup.get('storage_type').clearValidators();
      this.formGroup.get('storage_id').clearValidators();
      this.formGroup.get('storage_type').updateValueAndValidity();
      this.formGroup.get('storage_id').updateValueAndValidity();
    }
  }

  getBackupStorageNames() {
    // 查询备份存储单元组
    this.storageUserAuthService
      .getStorageUserAuthRelationsByUserId({
        userId: this.cookieService.get('userId'),
        authType: 2
      })
      .subscribe(res => {
        this.backupStorageUnitGroupNames = map(res.records, item => {
          return {
            isLeaf: true,
            label: item.storageName,
            disabled: false,
            value: item.storageId,
            ...item
          };
        });
        if (this.applicationType !== ApplicationType.GaussDBDWS) {
          this.backupStorageUnitGroupNames = this.backupStorageUnitGroupNames.filter(
            v => v.hasEnableParallelStorage === false
          );
        }
        if (
          this.applicationType === ApplicationType.GaussDBDWS &&
          !MultiCluster.isMulti
        ) {
          this.backupStorageUnitGroupNames = this.backupStorageUnitGroupNames.filter(
            v => v.hasEnableParallelStorage === true
          );
        }
        this.backupStorageUnitGroupNames = [
          ...this.backupStorageUnitGroupNames
        ];

        // RBAC自定义用户可以修改SLA但是没有授权单元，此时界面上虽然展示不出来但是storage_id是有值的，需要去掉
        if (
          this.formGroup.get('storage_type').value ===
            DataMap.storagePoolBackupStorageType.group.value &&
          !find(this.backupStorageUnitGroupNames, {
            value: this.formGroup.get('storage_id').value
          })
        ) {
          this.formGroup.get('storage_id').setValue('');
        }

        if (!this.isSlaDetail) {
          this.formGroup.get('storage_id').updateValueAndValidity();
        }
      });

    // 查询备份存储单元
    this.storageUserAuthService
      .getStorageUserAuthRelationsByUserId({
        userId: this.cookieService.get('userId'),
        authType: 1
      })
      .subscribe(res => {
        this.backupStorageUnitNames = map(res.records, item => {
          return {
            isLeaf: true,
            label: item.storageName,
            disabled: false,
            value: item.storageId,
            ...item
          };
        });
        // x系列除DWS只能指定本地存储单元
        this.backupStorageUnitNames = filter(
          this.backupStorageUnitNames,
          item => {
            return (
              this.applicationType === ApplicationType.GaussDBDWS ||
              this.appUtilsService.isDecouple ||
              this.appUtilsService.isDistributed ||
              item.generatedType ===
                DataMap.backupStorageGeneratedType.local.value
            );
          }
        );

        // RBAC自定义用户可以修改SLA但是没有授权单元，此时界面上虽然展示不出来但是storage_id是有值的，需要去掉
        if (
          this.formGroup.get('storage_type').value ===
            DataMap.storagePoolBackupStorageType.unit.value &&
          !find(this.backupStorageUnitNames, {
            value: this.formGroup.get('storage_id').value
          })
        ) {
          this.formGroup.get('storage_id').setValue('');
        }

        if (!this.isSlaDetail) {
          this.formGroup.get('storage_id').updateValueAndValidity();
        }
      });
  }
  listenChanges() {
    this.formGroup.get('backupTeams').valueChanges.subscribe(res => {
      this.updateBackupStorageNames(res);
    });

    this.formGroup.get('storage_type').valueChanges.subscribe(res => {
      this.formGroup.get('storage_id').setValue('');
      if (!this.formGroup.value.storage_type) {
        this.formGroup.get('storage_id').clearValidators();
      } else {
        this.formGroup
          .get('storage_id')
          .setValidators(this.baseUtilService.VALID.required());
      }
      this.formGroup.get('storage_id').updateValueAndValidity();
      this.updateBackupStorageNames(this.formGroup.get('backupTeams').value);
    });

    this.formGroup.get('storage_id').valueChanges.subscribe(res => {
      if (!res) {
        this.isDisableQos.emit(false);
        return;
      }
      const flag =
        find(this.backupStorageUnitNames, { value: res })?.storageType ===
          'BasicDisk' ||
        find(this.backupStorageUnitGroupNames, { value: res })?.storageType ===
          DataMap.poolStorageDeviceType.Server.value;
      this.isDisableQos.emit(flag);

      if (flag) {
        // 本地盘存储单元不支持限速策略、源端重删、目标端重删
        this.formGroup.get('qos_id').setValue('');
        if (!!this.formGroup.get('source_deduplication')) {
          this.formGroup.get('source_deduplication').setValue(false);
        }
        if (!!this.formGroup.get('deduplication')) {
          this.formGroup.get('deduplication').setValue(false);
        }
      }
    });
  }
  updateBackupStorageNames(data) {
    if (
      find(data, { action: 'log' }) &&
      this.formGroup.value.storage_type == 'storage_unit_group'
    ) {
      this.backupStorageUnitGroupNames = map(
        this.backupStorageUnitGroupNames,
        item => {
          return {
            isLeaf: true,
            label: item.clusterName,
            ...item,
            disabled: isEqual(
              item.storageStrategyType,
              DataMap.newBackupPolicy.balance.value
            )
          };
        }
      );
    } else {
      this.backupStorageUnitGroupNames = map(
        this.backupStorageUnitGroupNames,
        item => {
          return {
            isLeaf: true,
            label: item.clusterName,
            ...item,
            disabled: false
          };
        }
      );
    }
  }
}
