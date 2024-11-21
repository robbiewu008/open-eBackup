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
import {
  BaseUtilService,
  ComponentRestApiService,
  I18NService
} from 'app/shared';
import { assign } from 'lodash';

@Component({
  selector: 'aui-modify',
  templateUrl: './modify.component.html'
})
export class ModifyComponent implements OnInit {
  currentComponent;
  formGroup: FormGroup;
  modifyPrivateKey = true;
  componentLabel = this.i18n.get('system_component_name_label');
  daysOfEarlyLabel = this.i18n.get('system_certificate_early_day_label');
  modifyPrivateKeyLabel = this.i18n.get('system_modify_private_key_label');
  newPasswordLabel = this.i18n.get('common_newpwd_label');
  confirmPassWordLabel = this.i18n.get('common_confirmpwd_label');
  requiredLabel = this.i18n.get('common_required_label');

  daysErrorTip = assign(
    {},
    this.baseUtilService.rangeErrorTip,
    this.baseUtilService.integerErrorTip,
    {
      invalidRang: this.i18n.get('common_valid_rang_label', [7, 180])
    }
  );

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public certApiService: ComponentRestApiService,
    public baseUtilService: BaseUtilService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      expirationWarningDays: new FormControl(
        this.currentComponent.expirationWarningDays,
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(7, 180)
          ],
          updateOn: 'change'
        }
      )
    });
  }

  modify(cb?: () => void) {
    this.certApiService
      .modifyCertificateConfigUsingPUT({
        componentId: this.currentComponent.componentId,
        config: this.formGroup.value
      })
      .subscribe(res => cb());
  }

  ngOnInit() {
    this.initForm();
  }
}
