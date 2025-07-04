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
import { assign } from 'lodash';
import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  HostService,
  I18NService,
  DataMap,
  ProtectedResourceApiService
} from 'app/shared';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-modify-host',
  templateUrl: './modify-host.component.html',
  styleUrls: ['./modify-host.component.less']
})
export class ModifyHostComponent implements OnInit {
  data;
  dataMap = DataMap;
  formGroup: FormGroup;
  nameErrorTip = assign(this.baseUtilService.requiredErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });
  autoSyncTip = this.i18n.get('common_auto_sync_host_name_tips_label');

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    const isAutoSynchronizeHostName =
      this.data.extendInfo?.is_auto_synchronize_host_name === 'true';
    this.formGroup = this.fb.group({
      name: new FormControl(
        {
          value: this.data.name,
          disabled: isAutoSynchronizeHostName
        },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ],
          updateOn: 'change'
        }
      ),
      isAutoSynchronizeHostName: new FormControl(isAutoSynchronizeHostName)
    });
    this.formGroup
      .get('isAutoSynchronizeHostName')
      .valueChanges.subscribe(res => {
        this.formGroup.get('name')[res ? 'disable' : 'enable']();
      });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = {
        name: this.formGroup.getRawValue().name,
        extendInfo: {}
      };
      assign(params, {
        extendInfo: {
          is_auto_synchronize_host_name: String(
            this.formGroup.value.isAutoSynchronizeHostName
          )
        }
      });
      this.protectedResourceApiService
        .UpdateResource({
          resourceId: this.data.uuid,
          UpdateResourceRequestBody: params
        })
        .subscribe({
          next: res => {
            observer.next();
            observer.complete();
          },
          error: error => {
            observer.error(error);
            observer.complete();
          }
        });
    });
  }
}
