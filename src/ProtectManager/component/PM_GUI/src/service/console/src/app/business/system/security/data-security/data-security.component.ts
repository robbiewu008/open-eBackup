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
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { DataMap, I18NService, SwitchService } from 'app/shared';
import { SystemSwitch } from 'app/shared/api/models';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { find, isEmpty, size } from 'lodash';
import { forkJoin } from 'rxjs';

@Component({
  selector: 'aui-data-security',
  templateUrl: './data-security.component.html',
  styleUrls: ['./data-security.component.less']
})
export class DataSecurityComponent implements OnInit {
  formGroup: FormGroup;
  verifyFormGroup: FormGroup;
  viewSettingFlag: boolean = true;
  viewVerifySettingFlag: boolean = true;
  replicationLinkEncryption: boolean;
  backupLinkEncryption: boolean;
  protectObjectFileSystemvVerify: boolean;
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  isDecouple = this.appUtilsService.isDecouple;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private switchService: SwitchService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getEncryptionPolicy();
  }

  onChange() {
    this.getEncryptionPolicy();
  }

  initForm() {
    this.formGroup = this.fb.group({
      replicationLinkEncryption: new FormControl(false),
      backupLinkEncryption: new FormControl(false)
    });

    this.verifyFormGroup = this.fb.group({
      protectObjectFileSystemvVerify: new FormControl(false)
    });
  }

  getEncryptionPolicy(
    viewSettingFlag?: boolean,
    viewVerifySettingFlag?: boolean
  ) {
    this.switchService.ListSystemSwitchApi({}).subscribe((res: any) => {
      this.replicationLinkEncryption = !isEmpty(
        find(res.switches, item => {
          return (
            item.name === 'REPLICATION_LINK_ENCRYPTION' && item.status === 1
          );
        })
      );
      this.formGroup
        .get('replicationLinkEncryption')
        .setValue(this.replicationLinkEncryption);
      this.backupLinkEncryption = !isEmpty(
        find(res.switches, item => {
          return item.name === 'BACKUP_LINK_ENCRYPTION' && item.status === 1;
        })
      );
      this.formGroup
        .get('backupLinkEncryption')
        .setValue(this.backupLinkEncryption);
      this.protectObjectFileSystemvVerify = !isEmpty(
        find(res.switches, item => {
          return (
            item.name === 'PROTECT_OBJECT_FILE_SYSTEM_VERIFY' &&
            item.status === 1
          );
        })
      );
      this.verifyFormGroup
        .get('protectObjectFileSystemvVerify')
        .setValue(this.protectObjectFileSystemvVerify);
      if (viewSettingFlag) {
        this.viewSettingFlag = true;
      }
      if (viewVerifySettingFlag) {
        this.viewVerifySettingFlag = true;
      }
    });
  }

  saveEncryptionPolicy() {
    const updateSwitch = [];

    if (
      this.replicationLinkEncryption !==
      this.formGroup.value.replicationLinkEncryption
    ) {
      updateSwitch.push(
        this.switchService.UpdateSystemSwitchApi({
          akOperationTips: false,
          UpdateSystemSwitchApiRequestBody: {
            name: 'REPLICATION_LINK_ENCRYPTION',
            status: this.formGroup.value.replicationLinkEncryption ? 1 : 0
          }
        })
      );
    }
    if (
      this.backupLinkEncryption !== this.formGroup.value.backupLinkEncryption
    ) {
      updateSwitch.push(
        this.switchService.UpdateSystemSwitchApi({
          UpdateSystemSwitchApiRequestBody: {
            name: 'BACKUP_LINK_ENCRYPTION',
            status: this.formGroup.value.backupLinkEncryption ? 1 : 0
          }
        })
      );
    }
    if (
      this.protectObjectFileSystemvVerify !==
      this.verifyFormGroup.value.protectObjectFileSystemvVerify
    ) {
      updateSwitch.push(
        this.switchService.UpdateSystemSwitchApi({
          UpdateSystemSwitchApiRequestBody: {
            name: 'PROTECT_OBJECT_FILE_SYSTEM_VERIFY',
            status: this.verifyFormGroup.value.protectObjectFileSystemvVerify
              ? 1
              : 0
          } as any
        })
      );
    }

    if (!!size(updateSwitch)) {
      forkJoin(updateSwitch).subscribe(() =>
        this.getEncryptionPolicy(true, true)
      );
    } else {
      this.getEncryptionPolicy(true, true);
    }
  }

  cancelEncryptionPolicy() {
    this.viewSettingFlag = true;
    this.getEncryptionPolicy();
  }

  cancelVerifyPolicy() {
    this.viewVerifySettingFlag = true;
    this.getEncryptionPolicy();
  }

  modifyEncryptionPolicy() {
    this.viewSettingFlag = false;
  }

  modifyVerifyPolicy() {
    this.viewVerifySettingFlag = false;
  }
}
