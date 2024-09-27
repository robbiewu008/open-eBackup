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
import { assign, get, isArray, isEmpty } from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-protection-advance',
  templateUrl: './protection-advance.component.html',
  styleUrls: ['./protection-advance.component.less']
})
export class ProtectionAdvanceComponent implements OnInit {
  resourceData;
  resourceType;
  formGroup: FormGroup;
  lines = [{ value: [10, 50] }];
  points = [
    {
      value: 10,
      label: '10%'
    },
    {
      value: 20,
      label: '20%'
    },
    {
      value: 30,
      label: '30%'
    },
    {
      value: 40,
      label: '40%'
    },
    {
      value: 50,
      label: '50%'
    }
  ];
  valid$ = new Subject<boolean>();

  lvmErrorTip = {
    invalidRang: this.i18n.get('common_valid_rang_label', [10, 50]),
    invalidInteger: this.i18n.get('common_valid_integer_label')
  };

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      lvmPercentSlider: new FormControl(10),
      lvmPercent: new FormControl(10, {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(10, 50)
        ]
      })
    });
    this.formGroup.get('lvmPercentSlider').valueChanges.subscribe(res => {
      this.formGroup.get('lvmPercent').setValue(res, { emitEvent: false });
    });
    this.formGroup.get('lvmPercent').valueChanges.subscribe(res => {
      this.formGroup
        .get('lvmPercentSlider')
        .setValue(res, { emitEvent: false });
    });
    this.formGroup.statusChanges.subscribe(() => {
      this.valid$.next(this.formGroup.valid);
    });
  }

  updateForm() {
    if (
      isEmpty(this.resourceData) ||
      isEmpty(this.resourceData[0]) ||
      isEmpty(this.resourceData[0].protectedObject)
    ) {
      return;
    }
    this.formGroup.patchValue({
      lvmPercentSlider: get(
        this.resourceData[0],
        'protectedObject.extParameters.create_lvm_percent',
        ''
      ),
      lvmPercent: get(
        this.resourceData[0],
        'protectedObject.extParameters.create_lvm_percent',
        ''
      )
    });
  }

  initData(data: any, resourceType: string) {
    this.resourceData = data;
    this.resourceType = resourceType;
  }

  onOK() {
    const resourceData = isArray(this.resourceData)
      ? this.resourceData[0]
      : this.resourceData;
    return assign(resourceData, {
      ext_parameters: {
        create_lvm_percent: Number(this.formGroup.value.lvmPercent)
      }
    });
  }
}
