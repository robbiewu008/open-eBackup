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
import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { OptionItem } from '@iux/live';
import {
  BaseUtilService,
  ClustersApiService,
  CollectInfoApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  LogManagerApiService
} from 'app/shared';
import { StorageUnitService } from 'app/shared/api/services/storage-unit.service';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  every,
  find,
  get,
  isEmpty,
  isUndefined,
  split
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-storage-device',
  templateUrl: './create-storage-device.component.html',
  styleUrls: ['./create-storage-device.component.less']
})
export class CreateStorageDeviceComponent implements OnInit {
  formGroup: FormGroup;
  isEdit: boolean;
  drawData: any;
  changedName = true;
  deviceTypeOptions: OptionItem[];
  dataMap = DataMap;
  ipOptions = [];
  isDistributed = this.appUtilsService.isDistributed;
  isDecouple = this.appUtilsService.isDecouple;

  constructor(
    private baseUtilService: BaseUtilService,
    public fb: FormBuilder,
    public i18n: I18NService,
    public clusterApiService: ClustersApiService,
    public dataMapService: DataMapService,
    public appUtilsService: AppUtilsService,
    public storageUnitService: StorageUnitService,
    public collectInfoService: CollectInfoApiService,
    public logManagerService: LogManagerApiService
  ) {}

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_storage_pool_name_invalid_label')
  };
  ipErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.ipErrorTip
  };
  portErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  userNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [5, 64])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [8, 64])
  };

  ngOnInit(): void {
    this.initForm();
    this.initOptionItems();
  }

  // 校验多个IP以英文逗号形式分隔
  validMultiIpv4() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || isEmpty(control.value)) {
        return null;
      }
      const ips = control.value.split(',');

      const ipv4 = CommonConsts.REGEX.ipv4;
      if (!every(ips, item => ipv4.test(item))) {
        return { invalidName: { value: control.value } };
      }
      return null;
    };
  }
  validLength(min, max): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || isEmpty(control.value)) {
        return null;
      }
      const value = control.value;
      if (!new RegExp(`^.{${min},${max}}$`).test(value)) {
        return { invalidNameLength: { value: control.value } };
      }
      return null;
    };
  }

  initForm() {
    this.formGroup = this.fb.group({
      clusterName: new FormControl(
        {
          value: get(this.drawData, 'clusterName', ''),
          disabled: !!this.drawData
        },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.storagePoolName)
          ]
        }
      ),
      deviceType: new FormControl(get(this.drawData, 'deviceType', ''), {
        validators: [this.baseUtilService.VALID.required()]
      }),
      ip: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ]
      }),
      port: new FormControl(get(this.drawData, 'port', '8088'), {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      }),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validLength(5, 64)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validLength(8, 64)
        ]
      })
    });

    !isEmpty(this.drawData) &&
      this.formGroup.get('username').valueChanges.subscribe(res => {
        if (this.drawData && res === this.drawData.username) {
          this.formGroup
            .get('password')
            .setValidators([this.validLength(8, 64)]);
          this.changedName = false;
        } else {
          this.formGroup
            .get('password')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.validLength(8, 64)
            ]);
          this.changedName = true;
        }
        this.formGroup.get('password').updateValueAndValidity();
      });

    this.formGroup.get('deviceType').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      // 切换时清理ip
      this.formGroup.get('ip').setValue('');
      if (res === DataMap.poolStorageDeviceType.Server.value) {
        this.getIpData();
        this.formGroup.get('port').clearValidators();
        this.formGroup.get('username').clearValidators();
        this.formGroup.get('password').clearValidators();
      } else {
        this.formGroup
          .get('port')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]);
        this.formGroup
          .get('username')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validLength(5, 64)
          ]);
        this.formGroup
          .get('password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validLength(8, 64)
          ]);
      }
      this.formGroup.get('port').updateValueAndValidity();
      this.formGroup.get('username').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
    });

    if (this.drawData) {
      this.formGroup.get('deviceType').updateValueAndValidity();
      this.formGroup.get('username').setValue(this.drawData.username);
      this.formGroup.get('ip').setValue(split(this.drawData.ip, ',', 1)[0]);
    }

    if (this.isDistributed && !this.drawData) {
      // 分布式只有服务器
      this.formGroup
        .get('deviceType')
        .setValue(DataMap.poolStorageDeviceType.Server.value);
    }
  }

  initOptionItems() {
    this.deviceTypeOptions = this.dataMapService
      .toArray('poolStorageDeviceType')
      .filter((v: OptionItem) => {
        return (
          (v.isLeaf = true) &&
          v.value !== DataMap.poolStorageDeviceType.OceanPacific.value
        );
      });
    if (this.isDistributed) {
      // 分布式不支持oceanProtectx
      this.deviceTypeOptions = this.deviceTypeOptions.filter(
        item => item.value !== DataMap.poolStorageDeviceType.OceanProtectX.value
      );
    }
    if (!this.isDecouple && !this.isDistributed) {
      // 如果不是这两个就没有服务器类型
      this.deviceTypeOptions.pop();
    }
  }

  getIpData() {
    this.logManagerService.collectNodeInfo({}).subscribe(res => {
      const ipArray = [];
      each(res.data, item => {
        if (
          !this.isDecouple ||
          !this.isEdit ||
          this.drawData.storageEsn === item.nodeName
        ) {
          ipArray.push({
            ...item,
            key: item.address,
            value: item.address,
            label: item.address,
            isLeaf: true
          });
        }
      });
      this.ipOptions = ipArray;
    });
  }

  getParams() {
    const params: any = {};
    if (
      this.formGroup.get('deviceType').value ===
      DataMap.poolStorageDeviceType.OceanProtectX.value
    ) {
      assign(params, {
        ...this.formGroup.getRawValue()
      });
      params.role = 3;
    } else {
      const { clusterName, deviceType, ip } = this.formGroup.getRawValue();
      assign(params, {
        clusterName: clusterName,
        deviceType: deviceType,
        ip: ip,
        role: 3,
        deviceId: find(this.ipOptions, item => item.address === ip)?.nodeName
      });
    }

    return params;
  }

  saveUnitInfo(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) return;
      const params = this.getParams();
      if (this.isEdit) {
        this.clusterApiService
          .modifyTargetClusterUsingPUT({
            clusterId: this.drawData.clusterId,
            request: params
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      } else {
        this.clusterApiService
          .createTargetClusterUsingPOST({
            request: params
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      }
    });
  }
}
