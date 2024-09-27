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
import { BaseUtilService, DataMap, I18NService } from 'app/shared';
import {
  FormGroup,
  FormBuilder,
  FormControl,
  Validators,
  ValidatorFn,
  AbstractControl
} from '@angular/forms';

@Component({
  selector: 'aui-export-request-file',
  templateUrl: './export-request-file.component.html',
  styles: [
    `
      .advanced-params {
        margin-top: 40px;
      }
    `
  ]
})
export class ExportRequestFileComponent implements OnInit {
  formGroup: FormGroup;
  keyAlgorithmLabel = this.i18n.get('system_key_algorithm_label');
  requiredLabel = this.i18n.get('common_required_label');

  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  generalMaxLength = 128;

  organizationMaxLength = 64;

  algorithmOptions = [
    {
      key: 'rsa-2048',
      label: 'RSA 2048',
      isLeaf: true
    },
    {
      key: 'rsa-4096',
      label: 'RSA 4096',
      isLeaf: true
    }
  ];

  requiredErrorTip = {
    required: this.requiredLabel
  };

  countryErrorTip = {
    invalidCountryLen: this.i18n.get('system_country_valid_label'),
    invalidName: this.i18n.get('system_country_word_valid_label')
  };

  generalErrorTip = {
    invalidName: this.i18n.get('system_general_valid_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.generalMaxLength
    ])
  };

  organizationErrorTip = {
    invalidName: this.i18n.get('system_general_valid_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.organizationMaxLength
    ])
  };

  commonNameErrorTip = {
    invalidName: this.i18n.get('system_common_name_valid_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.organizationMaxLength
    ])
  };

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    private baseUtilService: BaseUtilService
  ) {}

  validCountryLen(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      if (control.value.length !== 2) {
        return { invalidCountryLen: { value: control.value } };
      }
      return null;
    };
  }

  initForm() {
    this.formGroup = this.fb.group({
      keyAlgorithm: new FormControl('rsa-2048', Validators.required)
    });
    if (this.isCyberEngine) {
      const countryReg = /^[a-zA-Z]{2}$/;
      const generalReg = /^[a-zA-Z\d\,\.\s\_\-\*]*$/;
      const commonNameReg = /^[a-zA-Z\d\,\.\s\_\-\*\@]*$/;
      this.formGroup.addControl(
        'country',
        new FormControl('', {
          validators: [
            this.validCountryLen(),
            this.baseUtilService.VALID.name(countryReg, false)
          ]
        })
      );
      this.formGroup.addControl(
        'state',
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.maxLength(this.generalMaxLength),
            this.baseUtilService.VALID.name(generalReg, false)
          ]
        })
      );
      this.formGroup.addControl(
        'city',
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.maxLength(this.generalMaxLength),
            this.baseUtilService.VALID.name(generalReg, false)
          ]
        })
      );
      this.formGroup.addControl(
        'organization',
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.maxLength(this.organizationMaxLength),
            this.baseUtilService.VALID.name(generalReg, false)
          ]
        })
      );
      this.formGroup.addControl(
        'organizationUnit',
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.maxLength(this.organizationMaxLength),
            this.baseUtilService.VALID.name(generalReg, false)
          ]
        })
      );
      this.formGroup.addControl(
        'commonName',
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.maxLength(this.organizationMaxLength),
            this.baseUtilService.VALID.name(commonNameReg, false)
          ]
        })
      );
    }
  }

  ngOnInit() {
    this.initForm();
  }
}
