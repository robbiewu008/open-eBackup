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
import { FormBuilder, FormGroup, FormControl } from '@angular/forms';
import { I18NService } from '@iux/live';
import {
  AntiRansomwareNetworkApiService,
  DetectReportAPIService,
  BaseUtilService,
  ProtectedResourceApiService,
  CommonConsts,
  DataMap
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, each, filter, trim } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-bind-relation',
  templateUrl: './bind-relation.component.html',
  styleUrls: ['./bind-relation.component.less']
})
export class BindRelationComponent implements OnInit {
  data;
  formGroup: FormGroup;
  deviceOptions = [];
  tenantOptions = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;

  hasTenant = false;

  maskErrorTip = {
    ...this.baseUtilService.ipErrorTip,
    ...this.baseUtilService.requiredErrorTip
  };

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    private appUtilsService: AppUtilsService,
    private antiRansomwareNetworkApiService: AntiRansomwareNetworkApiService,
    private detectReportApiService: DetectReportAPIService,
    private baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getDevice();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl({
        value: this.data.iface ? this.data.iface : '',
        disabled: true
      }),
      device: new FormControl('', {
        validators: this.baseUtilService.VALID.required()
      }),
      tenant: new FormControl('', {
        validators: this.baseUtilService.VALID.required()
      }),
      mask: new FormControl('255.255.255.255', {
        validators: [
          this.baseUtilService.VALID.ip(),
          this.baseUtilService.VALID.required()
        ]
      }),
      gateway: new FormControl('', {
        validators: this.baseUtilService.VALID.ip()
      })
    });

    this.watchForm();
  }

  watchForm() {
    this.formGroup.get('device').valueChanges.subscribe(res => {
      this.getTenant(res);
    });

    this.formGroup.get('tenant').valueChanges.subscribe(res => {
      this.hasTenant = true;
    });
  }

  getDevice() {
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize,
      conditions: JSON.stringify({
        type: 'StorageEquipment',
        subType: [['!='], DataMap.Device_Storage_Type.Other.value]
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      res.records = filter(res.records, item => {
        return (
          item.extendInfo?.detectType ===
          DataMap.storageDeviceDetectType.cyberEngine.value
        );
      });
      const deviceArray = [];
      each(res.records, item => {
        let label = '';
        if (trim(item['endpoint']) === '0') {
          label = `${item.name}`;
        } else {
          label = `${item.name}(${item['endpoint']})`;
        }
        deviceArray.push({
          ...item,
          key: item.uuid,
          value: item.uuid,
          label: label,
          isLeaf: true
        });
      });
      this.deviceOptions = deviceArray;
    });
  }

  getTenant(data) {
    this.appUtilsService.getResourceByRecursion(
      { deviceId: data },
      params => this.detectReportApiService.ListQueryResources(params),
      resource => {
        const tenantArray = [];
        each(resource, item => {
          tenantArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}`,
            isLeaf: true
          });
        });
        this.tenantOptions = tenantArray;
      }
    );
  }

  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        iFace: this.data.iface,
        esn: this.formGroup.value.device,
        vStoreId: this.formGroup.value.tenant,
        netmask: this.formGroup.value.mask
      };
      if (this.formGroup.value?.gateway) {
        assign(params, {
          gateway: this.formGroup.value.gateway
        });
      }

      this.antiRansomwareNetworkApiService
        .CreateNetworkRelation({
          CreateIFaceRelationReq: params
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
    });
  }
}
