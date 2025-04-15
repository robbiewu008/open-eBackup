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
import { BaseUtilService, I18NService } from 'app/shared';

@Component({
  selector: 'aui-configure-dataplane-ip',
  templateUrl: './configure-dataplane-ip.component.html'
})
export class ConfigureDataplaneIpComponent implements OnInit {
  drawData;
  formGroup: FormGroup;
  requiredLabel = this.i18n.get('common_required_label');
  dataplaneIpLabel = this.i18n.get('common_dataplane_ip_label');

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public baseUtilService: BaseUtilService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      dataplaneIp: new FormControl(this.drawData.dataplaneIp, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ],
        updateOn: 'change'
      })
    });
  }

  ngOnInit() {
    this.initForm();
  }
}
