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
import { DataMap, DataMapService } from 'app/shared';
import { includes } from 'lodash';

@Component({
  selector: 'aui-set-quota',
  templateUrl: './set-quota.component.html',
  styleUrls: ['./set-quota.component.less']
})
export class SetQuotaComponent implements OnInit {
  rowData;
  formGroup: FormGroup;
  sizeUnitOptions = this.dataMapService
    .toArray('Capacity_Unit')
    .filter(item => {
      item.isLeaf = true;
      return includes(
        [DataMap.Capacity_Unit.gb.value, DataMap.Capacity_Unit.mb.value],
        item.value
      );
    });

  constructor(
    private fb: FormBuilder,
    private dataMapService: DataMapService
  ) {}

  ngOnInit(): void {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      quota: new FormControl('1'),
      size: new FormControl(''),
      unit: new FormControl(DataMap.Capacity_Unit.gb.value)
    });
  }
}
