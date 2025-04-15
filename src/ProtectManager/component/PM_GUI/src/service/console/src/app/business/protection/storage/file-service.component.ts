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
  ApplicationType,
  CommonConsts,
  DataMap,
  GlobalService,
  I18NService,
  ProtectedResourceApiService,
  ResourceSetType,
  RouterUrl
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-file-service',
  templateUrl: './file-service.component.html',
  styleUrls: ['./file-service.component.less']
})
export class FileServiceComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().fileService];
  typeTitle = this.i18n.get('common_file_systems_label');

  constructor(
    private i18n: I18NService,
    private globalService: GlobalService,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initDevice();
  }

  initDevice() {
    this.subApp.unshift({
      id: 'storage-device',
      slaId: ApplicationType.NASFileSystem,
      key: '',
      hide: false,
      label: this.i18n.get('protection_storage_devices_label'),
      prefix: 'S',
      color: '#000000',
      protected_count: 0,
      count: 0,
      protectionUrl: RouterUrl.ProtectionStorageDeviceInfo,
      copyUrl: RouterUrl.ProtectionStorageDeviceInfo,
      resType: DataMap.Resource_Type.NASFileSystem.value,
      resourceSetType: ResourceSetType.StorageEquipment,
      jobTargetType: [
        DataMap.Job_Target_Type.DoradoV7.value,
        DataMap.Job_Target_Type.OceanStorDoradoV7.value,
        DataMap.Job_Target_Type.OceanStorDorado_6_1_3.value,
        DataMap.Job_Target_Type.OceanStor_6_1_3.value,
        DataMap.Job_Target_Type.OceanStor_v5.value,
        DataMap.Job_Target_Type.OceanStorPacific.value,
        DataMap.Job_Target_Type.OceanStorDorado.value,
        DataMap.Job_Target_Type.OceanProtect.value
      ]
    });
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          type: 'StorageEquipment',
          subType: [['!='], DataMap.Device_Storage_Type.Other.value]
        })
      })
      .subscribe(res => {
        this.globalService.emitStore({
          action: 'emitStorage',
          state: {
            StorageEquipment: res.totalCount
          }
        });
      });
  }
}
