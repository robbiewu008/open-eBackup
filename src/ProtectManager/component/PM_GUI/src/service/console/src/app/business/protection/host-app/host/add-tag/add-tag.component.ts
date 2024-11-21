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
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { BaseUtilService } from 'app/shared';
import { first, get, isArray, size } from 'lodash';

@Component({
  selector: 'aui-add-tag',
  templateUrl: './add-tag.component.html',
  styleUrls: ['./add-tag.component.less']
})
export class AddTagComponent implements OnInit {
  public readonly MAX_LENGTH = 64;
  formGroup: FormGroup;
  item;

  constructor(
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      remarks: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(this.MAX_LENGTH)],
        updateOn: 'change'
      })
    });

    if (!isArray(this.item)) {
      this.formGroup
        .get('remarks')
        .setValue(get(this.item, 'extendInfo.tag', ''));
    } else if (size(this.item) === 1) {
      this.formGroup
        .get('remarks')
        .setValue(get(first(this.item), 'extendInfo.tag', ''));
    }
  }
}
