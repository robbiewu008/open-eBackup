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
  CommonConsts,
  I18NService,
  ProtectResourceCategory,
  ProxyHostSelectMode
} from 'app/shared';
import {
  assign,
  clone,
  forOwn,
  isArray,
  isEmpty,
  isString,
  trim
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-advanced-parameter',
  templateUrl: './advanced-parameter.component.html',
  styleUrls: ['./advanced-parameter.component.less']
})
export class AdvancedParameterComponent implements OnInit {
  resourceData;
  resourceType;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  scriptErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };

  constructor(
    public fb: FormBuilder,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateData();
  }

  updateData() {
    if (!this.resourceData.protectedObject?.extParameters) {
      return;
    }
    const extParameters = isString(
      this.resourceData.protectedObject?.extParameters
    )
      ? JSON.parse(this.resourceData.protectedObject?.extParameters)
      : this.resourceData.protectedObject?.extParameters;
    assign(extParameters, {
      before_protect_script: extParameters.pre_script
        ? extParameters.pre_script
        : '',
      after_protect_script: extParameters.post_script
        ? extParameters.post_script
        : '',
      protect_failed_script: extParameters.failed_script
        ? extParameters.failed_script
        : ''
    });
    this.formGroup.patchValue(extParameters);
    setTimeout(() => {
      this.valid$.next(this.formGroup.valid);
    }, 500);
  }

  initForm() {
    this.formGroup = this.fb.group({
      before_protect_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.linuxNoPathScript,
            false
          )
        ]
      }),
      after_protect_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.linuxNoPathScript,
            false
          )
        ]
      }),
      protect_failed_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.linuxNoPathScript,
            false
          )
        ]
      })
    });

    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(this.formGroup.valid);
    });
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
  }

  onOK() {
    const formData = clone(this.formGroup.value);
    const ext_parameters = forOwn(formData, (v, k) => {
      if (trim(v) === '') {
        delete formData[k];
      }
    });
    delete ext_parameters['before_protect_script'];
    delete ext_parameters['after_protect_script'];
    delete ext_parameters['protect_failed_script'];
    if (!isEmpty(this.formGroup.value.before_protect_script)) {
      assign(ext_parameters, {
        pre_script: this.formGroup.value.before_protect_script
      });
    }
    if (!isEmpty(this.formGroup.value.after_protect_script)) {
      assign(ext_parameters, {
        post_script: this.formGroup.value.after_protect_script
      });
    }
    if (!isEmpty(this.formGroup.value.protect_failed_script)) {
      assign(ext_parameters, {
        failed_script: this.formGroup.value.protect_failed_script
      });
    }

    return {
      ext_parameters
    };
  }
}
