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
import { Component, EventEmitter, Input, Output } from '@angular/core';
import { FormControl, FormGroup } from '@angular/forms';
import { I18NService } from 'app/shared';
import { LabelApiService } from 'app/shared/api/services';
import { CommonConsts } from 'app/shared/consts';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { size } from 'lodash';

@Component({
  selector: 'aui-select-tag',
  templateUrl: './select-tag.component.html',
  styleUrls: ['./select-tag.component.less']
})
export class SelectTagComponent {
  @Input() formGroup: FormGroup;
  @Input() targetKey: string;
  @Input() isAgentTag: Boolean;
  @Output() updateTable = new EventEmitter();

  labelOptions;

  constructor(
    private i18n: I18NService,
    private labelApiService: LabelApiService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {
    this.updateForm();
    this.getLabelOptions();
    this.listenChanges();
  }

  updateForm() {
    this.formGroup.addControl('selectTag', new FormControl([]));
  }

  listenChanges() {
    this.formGroup.get('selectTag').valueChanges.subscribe(res => {
      if (size(res)) {
        if (this.isAgentTag) {
          this.updateTable.emit({
            labelCondition: { labelEnvironmentList: res }
          });
        } else {
          this.updateTable.emit({ labelCondition: { labelList: res } });
        }
      } else {
        this.updateTable.emit();
      }
      this.formGroup.get(this.targetKey).setValue('');
    });
  }

  getLabelOptions() {
    const extParams = {
      startPage: CommonConsts.PAGE_START_EXTRA,
      akLoading: true
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.labelApiService.queryLabelUsingGET(params),
      res => {
        const arr = res?.map(item => {
          return {
            id: item.uuid,
            label: item.name,
            value: item.uuid,
            isLeaf: true
          };
        });
        this.labelOptions = arr;
      }
    );
  }
}
