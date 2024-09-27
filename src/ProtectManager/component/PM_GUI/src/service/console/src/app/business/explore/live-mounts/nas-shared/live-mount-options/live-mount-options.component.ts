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
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  DataMap,
  DataMapService,
  I18NService,
  PermissonLimitation,
  PortPermisson,
  RootPermisson
} from 'app/shared';
import { assign, each, isEmpty, map, pick, trim } from 'lodash';

@Component({
  selector: 'aui-live-mount-nas-shared-options',
  templateUrl: './live-mount-options.component.html',
  styleUrls: ['./live-mount-options.component.less']
})
export class LiveMountOptionsComponent implements OnInit {
  @Input() componentData;
  @Input() activeIndex;
  @Output() selectMountOptionChange = new EventEmitter<any>();

  formGroup: FormGroup;
  dataMap = DataMap;
  targetHostOptions = [];
  shareProtocolOptions = this.dataMapService
    .toArray('Shared_Mode')
    .filter(v => (v.isLeaf = true));
  authorityLevelOptions = [];
  userTypeOptions = [];
  userOptions = [];
  disableFileSystemName = false;
  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initData();
  }

  initForm() {
    this.formGroup = this.fb.group({});
    this.formGroup.statusChanges.subscribe(res => {
      this.selectMountOptionChange.emit(
        res === 'VALID' &&
          (this.formGroup.value.nfsEnable || this.formGroup.value.cifsEnable)
      );
    });
  }

  initData() {
    try {
      const properties = JSON.parse(this.componentData.selectionCopy.properties)
        .fileSystemShareInfo;
      if (
        this.componentData.selectionCopy.generated_by ===
          DataMap.CopyData_generatedType.liveMount.value &&
        properties &&
        properties[0] &&
        properties[0].fileSystemName
      ) {
        setTimeout(() => {
          this.disableFileSystemName = true;
          this.formGroup
            .get('name')
            .setValue(properties[0].fileSystemName.replace('mount_', ''));
        });
      } else {
        this.disableFileSystemName = false;
      }
    } catch (error) {
      this.disableFileSystemName = false;
    }
    this.selectMountOptionChange.emit(
      this.formGroup.valid &&
        (this.formGroup.value.nfsEnable || this.formGroup.value.cifsEnable)
    );
  }

  getComponentData() {
    const requestParams = {
      target_resource_uuid_list: [`mount_${this.formGroup.value.name}`],
      target_location: 'original',
      name: `mount_${this.formGroup.value.name}`
    };
    const file_system_share_info_list = [];
    if (this.formGroup.value.nfsEnable) {
      file_system_share_info_list.push({
        fileSystemName: `mount_${this.formGroup.value.name}`,
        type: +DataMap.Shared_Mode.nfs.value,
        accessPermission: this.formGroup.get('unixType').value,
        advanceParams: {
          clientType: DataMap.Nfs_Share_Client_Type.host.value,
          clientName: this.formGroup.value.client,
          squash: PermissonLimitation.Retained,
          rootSquash: this.formGroup.get('rootType').value,
          portSecure: PortPermisson.Arbitrary
        }
      });
    }
    if (this.formGroup.value.cifsEnable) {
      file_system_share_info_list.push({
        fileSystemName: `mount_${this.formGroup.value.name}`,
        type: +DataMap.Shared_Mode.cifs.value,
        accessPermission: this.formGroup.get('permissionType').value,
        advanceParams: {
          shareName: this.formGroup.value.cifsShareName,
          domainType: 2,
          usernames:
            this.formGroup.value.userType ===
            DataMap.Cifs_Domain_Client_Type.everyone.value
              ? ['@EveryOne']
              : this.formGroup.value.userType ===
                DataMap.Cifs_Domain_Client_Type.windowsGroup.value
              ? map(this.formGroup.value.userName, item => {
                  return '@' + item;
                })
              : this.formGroup.value.userName
        }
      });
    }
    const parameters = {} as any;
    const performance = {};
    const performanceParams = pick(this.formGroup.value, [
      'min_bandwidth',
      'max_bandwidth',
      'burst_bandwidth',
      'min_iops',
      'max_iops',
      'burst_iops',
      'burst_time',
      'latency'
    ]);
    each(performanceParams, (v, k) => {
      if (isEmpty(trim(String(v)))) {
        return;
      }
      if (!this.formGroup.value.latencyStatus && k === 'latency') {
        return;
      }
      performance[k] = v;
    });
    assign(parameters, { performance });
    assign(requestParams, { file_system_share_info_list });
    assign(requestParams, { parameters });
    return assign(this.componentData, {
      requestParams: assign(
        {},
        this.componentData.requestParams,
        requestParams
      ),
      selectionMount: {
        ...this.formGroup.value
      }
    });
  }
}
