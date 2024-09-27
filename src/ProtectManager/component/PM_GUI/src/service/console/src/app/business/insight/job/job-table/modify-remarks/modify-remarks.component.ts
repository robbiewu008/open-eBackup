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
import { BaseUtilService, JobAPIService } from 'app/shared';
import { JobBo } from 'app/shared/api/models';
import { get } from 'lodash';

@Component({
  selector: 'aui-modify-remarks',
  templateUrl: './modify-remarks.component.html',
  styleUrls: ['./modify-remarks.component.css']
})
export class ModifyRemarksComponent implements OnInit {
  public readonly MAX_LENGTH = 64;
  formGroup: FormGroup;
  row: JobBo;

  constructor(
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService,
    private jobApiService?: JobAPIService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    const tag = get(JSON.parse(this.row.extendStr), 'tag', void 0);
    this.formGroup = this.fb.group({
      remarks: new FormControl(tag ?? '', {
        validators: [this.baseUtilService.VALID.maxLength(this.MAX_LENGTH)],
        updateOn: 'change'
      })
    });
  }

  onOK() {
    return this.jobApiService.setJobTagUsingPUT({
      jobId: this.row.jobId,
      tag: this.formGroup.value.remarks
    });
  }
}
