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
  FormGroup,
  FormBuilder,
  FormControl,
  Validators
} from '@angular/forms';
import { I18NService } from 'app/shared';

@Component({
  selector: 'aui-select-qos-policy',
  templateUrl: './select-qos-policy.component.html'
})
export class SelectQosPolicyComponent implements OnInit {
  formGroup: FormGroup;
  qosPolicyOptions;
  requiredLabel = this.i18n.get('common_required_label');
  qosPolicyLabel = this.i18n.get('common_limit_rate_policy_label');
  requiredErrorTip = {
    required: this.requiredLabel
  };
  constructor(public i18n: I18NService, public fb: FormBuilder) {}

  initForm() {
    this.formGroup = this.fb.group({
      qosPolicyType: new FormControl('', Validators.required)
    });
    this.qosPolicyOptions = [
      {
        key: 'qos1',
        label: 'Qos Policy 01',
        isLeaf: true
      }
    ];
  }

  ngOnInit() {
    this.initForm();
  }
}
