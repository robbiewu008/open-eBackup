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
import {
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import { DatatableComponent, OptionItem } from '@iux/live';
import {
  BaseUtilService,
  CAPACITY_UNIT,
  ColorConsts,
  DataMapService,
  I18NService
} from 'app/shared';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';

@Component({
  selector: 'aui-set-storage-policy',
  templateUrl: './set-storage-policy.component.html',
  styleUrls: ['./set-storage-policy.component.less']
})
export class SetStoragePolicyComponent implements OnInit {
  @ViewChild(DatatableComponent, { static: true }) lvTable: DatatableComponent;
  @Input() tableData;
  @Input() selectedData;
  searchName;
  status = true;
  data = [];
  timeUnitData: OptionItem[];
  timeForm: FormGroup;
  unitconst = CAPACITY_UNIT;
  progressBarColor = [[0, ColorConsts.NORMAL]];
  periodErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidNum: this.i18n.get('system_num_error_tips_label', [5, 120])
  };

  constructor(
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public dataMapService: DataMapService,
    public fb: FormBuilder
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.timeUnitData = this.dataMapService.toArray('timeUnit');
    this.data = this.dataMapService.toArray('newBackupPolicy');
  }

  initForm() {
    this.timeForm = this.fb.group({
      storageStrategyType: new FormControl(1, {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
  }

  protected readonly Math = Math;
}
