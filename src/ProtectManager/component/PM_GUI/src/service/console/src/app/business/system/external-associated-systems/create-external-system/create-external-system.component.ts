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
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidationErrors,
  ValidatorFn
} from '@angular/forms';
import { MessageService, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  I18NService,
  CommonConsts,
  ExternalSystemService
} from 'app/shared';
import { assign, filter, isEmpty, isUndefined, size, trim } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-external-system',
  templateUrl: './create-external-system.component.html',
  styleUrls: ['./create-external-system.component.less']
})
export class CreateExternalSystemComponent implements OnInit {
  data;
  formGroup: FormGroup;
  portErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32]),
    invalidName: this.i18n.get('common_valid_name_with_allowed_dots_label')
  };
  passwordErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [8, 16])
  });
  usernameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [3, 32]),
    invalidName: this.i18n.get('common_valid_name_with_allowed_dots_label')
  });
  systemTypeOptions = [
    {
      value: 'ebackup',
      label: this.i18n.get('system_ebackup_label'),
      isLeaf: true
    },
    {
      value: 'dpa',
      label: this.i18n.get('system_dpa_label'),
      isLeaf: true
    }
  ];

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private messageService: MessageService,
    public externalSystemService: ExternalSystemService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.updateData();
  }

  updateData() {
    if (!this.data) {
      return;
    }
    setTimeout(() => {
      this.formGroup.patchValue({ ...this.data, password: '' });
    }, 0);
  }

  validLength(min, max): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || isEmpty(control.value)) {
        return null;
      }
      const value = control.value;
      if (!new RegExp(`^.{${min},${max}}$`).test(value)) {
        return { invalidNameLength: { value: control.value } };
      }
      return null;
    };
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(this.systemTypeOptions[0]?.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.nameWithAllowedDots
          )
        ]
      }),
      endpoint: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ]
      }),
      port: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      }),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.nameWithAllowedDots
          ),
          this.validLength(3, 32)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validLength(8, 16)
        ]
      }),
      ak: new FormControl(''),
      sk: new FormControl('')
    });
    this.listenForm();
  }

  listenForm() {
    this.formGroup.get('type').valueChanges.subscribe(res => {
      const allChangeFormItemName = ['username', 'password', 'ak', 'sk'];
      allChangeFormItemName.forEach(item =>
        this.formGroup.get(item).clearValidators()
      );
      if (res === 'ebackup') {
        this.formGroup
          .get('username')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.nameWithAllowedDots
            ),
            this.validLength(3, 32)
          ]);
        this.formGroup
          .get('password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validLength(8, 16)
          ]);
      } else if (res === 'dpa') {
        this.formGroup
          .get('ak')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('sk')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      allChangeFormItemName.forEach(item =>
        this.formGroup.get(item).updateValueAndValidity()
      );
    });
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const service = this.data
        ? this.externalSystemService.UpdateExternalSystem({
            UpdateExternalSystemRequestBody: {
              ...this.formGroup.value,
              uuid: this.data.uuid,
              scope: 0
            }
          })
        : this.externalSystemService.AddExternalSystem({
            AddExternalSystemRequestBody: {
              ...this.formGroup.value,
              scope: 0
            }
          });
      service.subscribe(
        res => {
          observer.next(res);
          observer.complete();
        },
        error => {
          observer.error(error);
          observer.complete();
        }
      );
    });
  }
}
