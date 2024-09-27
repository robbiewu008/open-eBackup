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
import { BaseUtilService, I18NService } from 'app/shared/services';
import { Observable, Observer } from 'rxjs';
import { StoragesAlarmThresholdApiService, DataMap } from 'app/shared';

@Component({
  selector: 'aui-threshold-modify',
  templateUrl: './threshold-modify.component.html',
  styleUrls: ['./threshold-modify.component.less']
})
export class ThresholdModifyComponent implements OnInit {
  data;
  drawData;
  currentCluster;
  formGroup: FormGroup;
  thresholdErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [50, 90])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private storagesAlarmThresholdApiService: StoragesAlarmThresholdApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getData();
  }

  initForm() {
    this.formGroup = this.fb.group({
      limitValue: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(50, 90)
        ],
        updateOn: 'change'
      })
    });
  }

  getData() {
    this.storagesAlarmThresholdApiService
      .queryThresholdUsingGET({
        id: this.data.repositoryId,
        memberEsn: this.drawData.storageEsn
      })
      .subscribe(res => {
        this.formGroup.patchValue(res);
      });
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const params = {
        id: this.data.repositoryId,
        limitValue: +this.formGroup.value.limitValue,
        memberEsn: this.drawData?.storageEsn
      };
      this.storagesAlarmThresholdApiService
        .updateThresholdUsingPUT(params)
        .subscribe(
          res => {
            observer.next(res);
            observer.complete();
          },
          error => {
            observer.error({});
            observer.complete();
          }
        );
    });
  }
}
