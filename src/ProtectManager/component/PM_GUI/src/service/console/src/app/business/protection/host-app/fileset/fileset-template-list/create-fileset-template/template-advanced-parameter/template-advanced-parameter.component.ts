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
  I18NService,
  DataMap,
  CommonConsts
} from 'app/shared';
import { Subject } from 'rxjs';
import { omit, isEmpty } from 'lodash';

@Component({
  selector: 'aui-template-advanced-parameter',
  templateUrl: './template-advanced-parameter.component.html',
  styleUrls: ['./template-advanced-parameter.component.less']
})
export class TemplateAdvancedParameterComponent implements OnInit {
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };

  constructor(
    public fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      before_protect_script: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(8192)]
      }),
      after_protect_script: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(8192)]
      }),
      protect_failed_script: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(8192)]
      })
    });
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(this.formGroup.valid);
    });
  }

  updateData(data) {
    if (!data) {
      return;
    }
    this.updateValidate(data.environment_os_type);
    this.formGroup.patchValue(data.ext_parameters);
  }

  updateValidate(environment_os_type) {
    if (!environment_os_type) {
      return;
    }

    this.formGroup
      .get('before_protect_script')
      .setValidators([
        this.baseUtilService.VALID.maxLength(8192),
        this.baseUtilService.VALID.name(
          environment_os_type === DataMap.Os_Type.windows.value
            ? CommonConsts.REGEX.windowsScript
            : CommonConsts.REGEX.linuxScript,
          false
        )
      ]);
    this.formGroup
      .get('after_protect_script')
      .setValidators([
        this.baseUtilService.VALID.maxLength(8192),
        this.baseUtilService.VALID.name(
          environment_os_type === DataMap.Os_Type.windows.value
            ? CommonConsts.REGEX.windowsScript
            : CommonConsts.REGEX.linuxScript,
          false
        )
      ]);
    this.formGroup
      .get('protect_failed_script')
      .setValidators([
        this.baseUtilService.VALID.maxLength(8192),
        this.baseUtilService.VALID.name(
          environment_os_type === DataMap.Os_Type.windows.value
            ? CommonConsts.REGEX.windowsScript
            : CommonConsts.REGEX.linuxScript,
          false
        )
      ]);
    this.formGroup.get('before_protect_script').updateValueAndValidity();
    this.formGroup.get('after_protect_script').updateValueAndValidity();
    this.formGroup.get('protect_failed_script').updateValueAndValidity();
  }
}
