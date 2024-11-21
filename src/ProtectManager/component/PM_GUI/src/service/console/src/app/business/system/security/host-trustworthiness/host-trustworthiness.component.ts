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
import { MessageboxService } from '@iux/live';
import { I18NService, SwitchService, WarningMessageService } from 'app/shared';
import { SystemSwitch } from 'app/shared/api/models';
import { find, isEmpty, size } from 'lodash';
import { forkJoin } from 'rxjs';

@Component({
  selector: 'aui-host-trustworthiness',
  templateUrl: './host-trustworthiness.component.html',
  styleUrls: ['./host-trustworthiness.component.css']
})
export class HostTrustworthinessComponent implements OnInit {
  formGroup: FormGroup;
  viewSettingFlag: boolean = true;
  replicationLinkEncryption: boolean;
  backupLinkEncryption: boolean;

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private switchService: SwitchService,
    private messageBox: MessageboxService,
    private warningMessageService: WarningMessageService
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
      replicationLinkEncryption: new FormControl(false)
    });
  }

  getEncryptionPolicy(viewSettingFlag?: boolean) {
    this.switchService.ListSystemSwitchApi({}).subscribe((res: any) => {
      this.replicationLinkEncryption = !isEmpty(
        find(res.switches, item => {
          return item.name === 'HOST_TRUST' && item.status === 1;
        })
      );
      this.formGroup
        .get('replicationLinkEncryption')
        .setValue(this.replicationLinkEncryption);
      if (viewSettingFlag) {
        this.viewSettingFlag = true;
      }
    });
  }

  saveEncryptionPolicy() {
    const callApi = () => {
      forkJoin([
        this.switchService.UpdateSystemSwitchApi({
          akOperationTips: false,
          UpdateSystemSwitchApiRequestBody: {
            name: 'HOST_TRUST',
            status: this.formGroup.value.replicationLinkEncryption ? 1 : 0
          }
        })
      ]).subscribe(res => this.getEncryptionPolicy(true));
    };
    // 开启
    if (this.formGroup.value.replicationLinkEncryption) {
      this.messageBox.confirm({
        lvHeader: this.i18n.get('common_alarms_info_label'),
        lvContent: this.i18n.get('common_host_trust_open_tips_label'),
        lvOk: () => {
          callApi();
        }
      });
    }
    // 关闭
    else {
      this.warningMessageService.create({
        content: this.i18n.get('common_host_trust_close_tips_label'),
        onOK: () => {
          callApi();
        }
      });
    }
  }

  cancelEncryptionPolicy() {
    this.viewSettingFlag = true;
    this.getEncryptionPolicy();
  }

  modifyEncryptionPolicy() {
    this.viewSettingFlag = false;
  }
}
