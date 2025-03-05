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
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  HcsResourceServiceService,
  I18NService,
  OpHcsServiceApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  first,
  includes,
  isEmpty,
  join,
  last,
  map,
  reject,
  set,
  size,
  split,
  uniqueId
} from 'lodash';

@Component({
  selector: 'aui-add-host-ingfo',
  templateUrl: './add-host-ingfo.component.html',
  styleUrls: ['./add-host-ingfo.component.less']
})
export class AddHostIngfoComponent implements OnInit {
  formGroup: FormGroup;
  hostFormGroup: FormGroup;
  portTips;
  isHcsEnvir;
  isHcsUser;
  ipData;
  rowData;

  isDistributed = false;
  isWindows = false;
  includes = includes;
  dataMap = DataMap;
  azOptions = [];
  isBusinessOptions = this.dataMapService
    .toArray('isBusinessOptions')
    .filter(item => {
      return (item.isLeaf = true);
    });
  userTypeOptions = this.dataMapService.toArray('userType').filter(v => {
    return (v.isLeaf = true);
  });
  passwordTypeOptions = this.dataMapService
    .toArray('passwordType')
    .filter(v => {
      return (v.isLeaf = true);
    });
  networkOptions = this.dataMapService.toArray('networkPlaneType').filter(v => {
    return (v.isLeaf = true);
  });

  ipsErrorTip = assign({}, this.baseUtilService.ipErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024]),
    repeatIp: this.i18n.get('common_same_ip_tips_label')
  });
  portErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  });
  usernameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  });
  passwordErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  });

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private hcsResourceService: HcsResourceServiceService,
    private opHcsServiceApiService: OpHcsServiceApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getAzOptions();
  }

  getNetworkInfo(isAddVaild?: boolean) {
    return this.fb.group({
      storageIp: new FormControl('', {
        validators: isAddVaild
          ? [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.ip()
            ]
          : null
      }),
      storageGateway: new FormControl('', {
        validators: isAddVaild ? [this.baseUtilService.VALID.ip()] : null
      })
    });
  }

  initForm() {
    this.isWindows =
      this.formGroup.value.osType === DataMap.OS_Type.Windows.value;
    this.hostFormGroup = this.fb.group({
      networkType: new FormControl(DataMap.networkPlaneType.backup.value),
      ip: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(1024),
          this.validIps(),
          this.validReatIp('ip')
        ]
      }),
      manageIp: new FormControl(''),
      businessIp: new FormControl(''),
      port: new FormControl(this.isWindows ? '5985' : '22', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      }),
      az: new FormControl(''),
      sftpPort: new FormControl('22', {
        validators: !this.isWindows
          ? [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 65535)
            ]
          : null
      }),
      businessIpFlags: new FormControl(true),
      userType: new FormControl(DataMap.userType.admin.value),
      passwordType: new FormControl(false),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(255)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(255)
        ]
      }),
      sudoPassword: new FormControl(''),
      isDpcNode: new FormControl(false),
      networkInfo: this.fb.array([this.getNetworkInfo()])
    });

    this.listenForm();

    if (!isEmpty(this.rowData)) {
      if (!isEmpty(this.rowData.networkInfo)) {
        (this.hostFormGroup.get('networkInfo') as FormArray).clear();
        each(this.rowData.networkInfo, () => this.addNetworkInfo());
      }
      this.hostFormGroup.patchValue({ ...this.rowData });
    }
  }

  listenForm() {
    this.hostFormGroup.get('userType').valueChanges.subscribe(res => {
      if (
        res === DataMap.userType.common.value &&
        !this.hostFormGroup.value.passwordType
      ) {
        this.hostFormGroup
          .get('sudoPassword')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(255)
          ]);
      } else {
        this.hostFormGroup.get('sudoPassword').clearValidators();
      }
      this.hostFormGroup.get('sudoPassword').updateValueAndValidity();
    });

    this.hostFormGroup.get('passwordType').valueChanges.subscribe(res => {
      if (
        !res &&
        this.hostFormGroup.value.userType === DataMap.userType.common.value
      ) {
        this.hostFormGroup
          .get('sudoPassword')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(255)
          ]);
      } else {
        this.hostFormGroup.get('sudoPassword').clearValidators();
      }
      this.hostFormGroup.get('sudoPassword').updateValueAndValidity();
    });

    this.hostFormGroup.get('isDpcNode').valueChanges.subscribe(res => {
      if (res) {
        each(
          (this.hostFormGroup.get('networkInfo') as FormArray).controls,
          form => {
            form
              .get('storageIp')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.ip()
              ]);
            form
              .get('storageGateway')
              .setValidators([this.baseUtilService.VALID.ip()]);
            form.get('storageIp').updateValueAndValidity();
            form.get('storageGateway').updateValueAndValidity();
          }
        );
      } else {
        each(
          (this.hostFormGroup.get('networkInfo') as FormArray).controls,
          form => {
            form.get('storageIp').clearValidators();
            form.get('storageGateway').clearValidators();
            form.get('storageIp').updateValueAndValidity();
            form.get('storageGateway').updateValueAndValidity();
          }
        );
      }
    });

    this.hostFormGroup.get('networkType').valueChanges.subscribe(res => {
      if (res === DataMap.networkPlaneType.management.value) {
        this.hostFormGroup
          .get('manageIp')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.setIpTypeValidator();
        this.hostFormGroup.get('ip').clearValidators();
        this.hostFormGroup.get('ip').setValue('');
      } else {
        this.hostFormGroup
          .get('ip')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(1024),
            this.validIps(),
            this.validReatIp('ip')
          ]);
        this.hostFormGroup.get('manageIp').clearValidators();
        this.hostFormGroup.get('businessIp').clearValidators();
        this.hostFormGroup.get('manageIp').setValue('');
        this.hostFormGroup.get('businessIp').setValue('');
      }
      this.hostFormGroup.get('ip').updateValueAndValidity();
      this.hostFormGroup.get('manageIp').updateValueAndValidity();
      this.hostFormGroup.get('businessIp').updateValueAndValidity();
    });
  }

  // 根据IP类型对IP地址做校验
  setIpTypeValidator() {
    const ipTypeValidator =
      this.formGroup.get('ipType').value === DataMap.IP_Type.ipv4.value
        ? this.baseUtilService.VALID._ipv4()
        : this.baseUtilService.VALID._ipv6();
    ['manageIp', 'businessIp'].forEach(item => {
      this.hostFormGroup
        .get(item)
        .addValidators([ipTypeValidator, this.validReatIp(item)]);
    });
  }

  get networkInfo() {
    return (this.hostFormGroup.get('networkInfo') as FormArray).controls;
  }

  addNetworkInfo() {
    (this.hostFormGroup.get('networkInfo') as FormArray).push(
      this.getNetworkInfo(true)
    );
  }

  deleteNetworkInfo(i) {
    if (this.networkInfo?.length === 1) {
      return;
    }
    (this.hostFormGroup.get('networkInfo') as FormArray).removeAt(i);
  }

  validIps(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }
      const ips = [control.value];
      const ipType = this.formGroup.get('ipType').value;

      if (!this.validIp(ips, ipType)) {
        return {
          invalidName: {
            value: control.value
          }
        };
      }

      return null;
    };
  }

  validIp(ips, ipType) {
    let isValid = true;
    for (const ip of ips) {
      if (!isValid) {
        break;
      }
      if (ipType === DataMap.IP_Type.ipv4.value) {
        if (includes(ip, '-')) {
          const ipSegment = split(ip, '-');
          if (size(ipSegment) > 2) {
            isValid = false;
            break;
          }
          const ipSize = this.validIpSize(ipSegment[0], ipSegment[1]);
          if (ipSize === 0 || ipSize === 1) {
            isValid = false;
            break;
          }

          if (!this.validIp(ipSegment, ipType)) {
            isValid = false;
            break;
          }
        } else {
          isValid = BaseUtilService.validateIpv4(ip);
        }
      } else {
        isValid = BaseUtilService.validateIpv6(ip);
      }
    }
    return isValid;
  }

  validIpSize(ipBegin, ipEnd) {
    let temp1;
    let temp2;
    temp1 = ipBegin.split('.');
    temp2 = ipEnd.split('.');
    if (size(temp1) !== size(temp2)) {
      return 1;
    }
    for (let i = 0; i < 4; i++) {
      if (i < 3 && Number(temp1[i]) !== Number(temp2[i])) {
        return 1;
      }
      if (Number(temp1[i]) > Number(temp2[i])) {
        return 1;
      } else if (Number(temp1[i]) < Number(temp2[i])) {
        return -1;
      }
    }
    return 0;
  }

  compareIp(ipA, ipB) {
    if (this.formGroup.value.ipType === DataMap.IP_Type.ipv6.value) {
      return ipA >= ipB;
    } else {
      let temp1;
      let temp2;
      temp1 = ipA.split('.');
      temp2 = ipB.split('.');
      for (let i = 0; i < 4; i++) {
        if (Number(temp1[i]) > Number(temp2[i])) {
          return true;
        } else if (Number(temp1[i]) < Number(temp2[i])) {
          return false;
        }
      }
      return true;
    }
  }

  validReatIp(keyId): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }
      // 对同类型Ip做重复校验
      const obj = {};
      set(obj, keyId, control.value);
      let ips = [
        ...reject(
          this.ipData,
          v => v?.networkType !== this.hostFormGroup.get('networkType').value
        ),
        obj
      ];
      if (!isEmpty(this.rowData)) {
        ips = [...reject(ips, v => v?.infoId === this.rowData.infoId)];
      }
      const ipLists = map(map(ips, keyId), val => {
        if (includes(val, '-')) {
          return split(val, '-');
        } else {
          return [val, val];
        }
      });
      ipLists.sort((a, b) => {
        if (this.compareIp(first(a), first(b))) {
          return 1;
        } else {
          return -1;
        }
      });

      for (let i = 0; i < ipLists.length - 1; i++) {
        if (this.compareIp(last(ipLists[i]), first(ipLists[i + 1]))) {
          const ip1 =
            first(ipLists[i]) === last(ipLists[i])
              ? first(ipLists[i])
              : join(ipLists[i], '-');
          const ip2 =
            first(ipLists[i + 1]) === last(ipLists[i + 1])
              ? first(ipLists[i + 1])
              : join(ipLists[i + 1], '-');
          assign(this.ipsErrorTip, {
            repeatIp: this.i18n.get('common_same_ip_tips_label', [
              ip1 === ip2
                ? ip1
                : `${ip1} ${this.i18n.get('common_and_label')} ${ip2}`
            ])
          });
          return {
            repeatIp: {
              value: control.value
            }
          };
        }
      }
      return null;
    };
  }

  getAzOptions() {
    if (!this.isHcsUser && !this.isHcsEnvir) {
      return;
    }
    if (this.isHcsUser) {
      this.hcsResourceService.GetHcsAz({}).subscribe(res => {
        this.azOptions = map(res.resources, item => {
          return assign(item, {
            value: item.resource_id,
            label: first(item.tags?.display_name),
            isLeaf: true
          });
        });
      });
    } else {
      this.opHcsServiceApiService.getAvailableZones({}).subscribe(res => {
        this.azOptions = map(res.records, item => {
          return assign(item, {
            value: item.azId,
            label: item.name,
            isLeaf: true
          });
        });
      });
    }
  }

  onOk() {
    return {
      ...this.hostFormGroup.value,
      infoId: uniqueId()
    };
  }
}
