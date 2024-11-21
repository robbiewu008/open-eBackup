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
  I18NService,
  StoragesApiService,
  DataMapService,
  DataMap
} from 'app/shared';
import { isEmpty, includes } from 'lodash';

@Component({
  selector: 'aui-archive-storage-detail',
  templateUrl: './archive-storage-detail.component.html',
  styleUrls: ['./archive-storage-detail.component.less']
})
export class ArchiveStorageDetailComponent implements OnInit {
  rowItem;
  storageArgs = [];
  constructor(
    private i18n: I18NService,
    private storageApiService: StoragesApiService,
    private dataMapservice: DataMapService
  ) {}

  getStorage() {
    this.storageApiService
      .queryStorageByIdUsingGET({
        storageId: this.rowItem.repositoryId
      })
      .subscribe(res => {
        this.storageArgs.push(
          {
            label: this.i18n.get('common_name_label'),
            value: res.storageName
          },
          {
            label: this.i18n.get('common_type_label'),
            value: this.dataMapservice.getLabel(
              'Archive_Storage_Type',
              res.type
            )
          },
          {
            label: this.i18n.get('common_cloud_platform_type_label'),
            value: this.dataMapservice.getLabel(
              'Storage_Cloud_Platform',
              res.cloudType
            )
          },
          {
            label: this.i18n.get('system_archive_storage_link_mode_label'),
            value: this.dataMapservice.getLabel(
              'azureLinkMode',
              res['connectType']
            ),
            hide: res.cloudType !== DataMap.Storage_Cloud_Platform.azure.value
          },
          {
            label: this.i18n.get('common_endpoint_label'),
            value: res.endpoint,
            hide:
              res.cloudType === DataMap.Storage_Cloud_Platform.azure.value &&
              res['connectType'] === DataMap.azureLinkMode.connection.value
          },
          {
            label: this.i18n.get('common_protocol_label'),
            value: res.useHttps ? 'HTTPS' : 'HTTP',
            hide:
              res.cloudType === DataMap.Storage_Cloud_Platform.azure.value &&
              res['connectType'] === DataMap.azureLinkMode.connection.value
          },
          {
            label: this.i18n.get('system_certificate_label'),
            value: res.certName,
            hide: isEmpty(res.certName) && !res.useHttps
          },
          {
            label: this.i18n.get('common_data_bucket_label'),
            value: res.bucketName,
            hide: res.cloudType === DataMap.Storage_Cloud_Platform.azure.value
          },
          {
            label: 'AK',
            value: res.ak,
            hide:
              res.cloudType === DataMap.Storage_Cloud_Platform.azure.value &&
              res['connectType'] === DataMap.azureLinkMode.connection.value
          },
          {
            label:
              res.cloudType === DataMap.Storage_Cloud_Platform.azure.value
                ? res['connectType'] === DataMap.azureLinkMode.connection.value
                  ? this.i18n.get('system_azure_connection_string_label')
                  : 'SK'
                : 'SK',
            value: '********'
          },
          {
            label: this.i18n.get('system_azure_blob_port_label'),
            value: res['port'],
            hide:
              res.cloudType !== DataMap.Storage_Cloud_Platform.azure.value ||
              res['connectType'] === DataMap.azureLinkMode.connection.value
          },
          {
            label: this.i18n.get('system_azure_blob_container_label'),
            value: res.bucketName,
            hide: res.cloudType !== DataMap.Storage_Cloud_Platform.azure.value
          },
          {
            label: this.i18n.get('common_index_bucket_label'),
            value: res.indexBucketName,
            hide: res.cloudType === DataMap.Storage_Cloud_Platform.azure.value
          },
          {
            label: this.i18n.get('common_agent_service_label'),
            hide: includes(
              [DataMap.Storage_Cloud_Platform.azure.value],
              res.cloudType
            ),
            value: res.proxyEnable
              ? this.i18n.get('common_enable_label')
              : this.i18n.get('common_disable_label')
          },
          {
            label: 'URL',
            hide: !res.proxyEnable,
            value: res.proxyHostName
          },
          {
            label: this.i18n.get('common_username_label'),
            hide: !res.proxyEnable,
            value: res.proxyUserName
          },
          {
            label: this.i18n.get('common_password_label'),
            hide: !res.proxyEnable,
            value: '********'
          },
          {
            label: this.i18n.get('common_capacity_alarm_threshold_label'),
            hide: includes(
              [DataMap.Storage_Cloud_Platform.azure.value],
              res.cloudType
            ),
            value: res.alarmEnable
              ? this.i18n.get('common_enable_label')
              : this.i18n.get('common_disable_label')
          },
          {
            label: this.i18n.get('common_alarm_threshold_label'),
            hide: !res.alarmEnable,
            value: `${res.alarmThreshold} ${
              +res.alarmLimitValueUnit === 1 ? 'TB' : 'GB'
            }`
          }
        );
      });
  }

  ngOnInit() {
    this.getStorage();
  }
}
