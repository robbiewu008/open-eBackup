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
import { BaseUtilService, CommonConsts, I18NService } from 'app/shared';

@Component({
  selector: 'aui-batch-config-path',
  templateUrl: './batch-config-path.component.html',
  styleUrls: ['./batch-config-path.component.less']
})
export class BatchConfigPathComponent implements OnInit {
  configPathEnum;
  type;
  formGroup: FormGroup;
  MAX_PATH_LENGTH = 255;

  pathErrorTip = {
    invalidName: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.MAX_PATH_LENGTH
    ])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      path: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(this.MAX_PATH_LENGTH),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath, false)
        ]
      })
    });
  }
}
