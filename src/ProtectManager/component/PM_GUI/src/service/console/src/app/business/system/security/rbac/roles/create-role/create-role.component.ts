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
import { FormGroup } from '@angular/forms';
import { I18NService, RoleApiService, UsersApiService } from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';

@Component({
  selector: 'aui-create-role',
  templateUrl: './create-role.component.html',
  styleUrls: ['./create-role.component.less']
})
export class CreateRoleComponent implements OnInit {
  @Input() openPage;
  @Input() data;
  rowData;
  isModify = false;
  isClone = false;
  optsConfig;
  formGroup = new FormGroup({});
  disableButton;

  constructor(
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    public usersApiService: UsersApiService,
    public roleApiService: RoleApiService,
    public virtualScroll: VirtualScrollService
  ) {}

  ngOnInit() {
    this.rowData = this.data.rowData;
    this.isModify = this.data.isModify;
    this.isClone = this.data.isClone;
    this.disableButton = !this.data.rowData;
    this.formGroup.statusChanges.subscribe(res => {
      this.disableButton = res !== 'VALID';
    });
    this.initConfig();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'cancel',
        label: this.i18n.get('common_cancel_label'),
        onClick: () => {
          this.back();
        }
      },
      {
        id: 'ok',
        type: 'primary',
        label: this.i18n.get('common_ok_label'),
        onClick: () => {
          this.onOK();
        }
      }
    ];
    this.optsConfig = [...opts];
  }

  back() {
    this.openPage.emit();
  }

  onOK() {
    const request = this.formGroup.getRawValue();
    if (this.isModify) {
      this.roleApiService
        .updateRole({
          id: this.rowData.roleId,
          request: request
        })
        .subscribe(() => this.openPage.emit());
    } else {
      this.roleApiService
        .createRole({
          request: request
        })
        .subscribe(() => this.openPage.emit());
    }
  }
}
