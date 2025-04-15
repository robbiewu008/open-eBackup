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
import { BaseUtilService, DataMap } from 'app/shared';
import { assign, isArray } from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-tdsql-advanced-parameter',
  templateUrl: './advanced-parameter.component.html',
  styleUrls: ['./advanced-parameter.component.less']
})
export class AdvancedParameterComponent implements OnInit {
  osType;
  resourceData;
  resourceType;
  dataMap = DataMap;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  constructor(
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    const { protectedObject } = this.resourceData;
    const extParameters = protectedObject?.extParameters || {};
    this.formGroup = this.fb.group({
      delete_archived_log: new FormControl(
        extParameters.delete_archived_log ?? false
      ),
      archive_res_auto_index: new FormControl(
        extParameters.archive_res_auto_index ?? false
      )
    });
    this.formGroup.statusChanges.subscribe(() => {
      this.valid$.next(this.formGroup.value);
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
        delete_archived_log: this.formGroup.value.delete_archived_log,
        archive_res_auto_index: this.formGroup.value.archive_res_auto_index
      }
    });
  }
}
