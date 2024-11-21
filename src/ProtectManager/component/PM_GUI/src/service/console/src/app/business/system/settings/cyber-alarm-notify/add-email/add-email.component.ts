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
import { AlarmNotifyRuleApiService } from 'app/shared/api/services/alarm-notify-rule-api.service';
import { isEmpty } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-email',
  templateUrl: './add-email.component.html',
  styleUrls: ['./add-email.component.less']
})
export class AddEmailComponent implements OnInit {
  rowData;
  formGroup: FormGroup;
  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('system_error_email_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [254])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private alarmNotifyRuleApiService: AlarmNotifyRuleApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(this.rowData?.emailAddress || '', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.email),
          this.baseUtilService.VALID.maxLength(254)
        ]
      }),
      description: new FormControl(this.rowData?.desc || '')
    });
  }

  onOk(): Observable<void> {
    return new Observable((observer: Observer<void>) => {
      if (!isEmpty(this.rowData)) {
        this.alarmNotifyRuleApiService
          .modifyEmailUsingPUT({
            modifyEmailRequest: {
              oldEmailAddress: this.rowData.emailAddress,
              emailAddress: this.formGroup.value.name,
              desc: this.formGroup.value.description
            }
          })
          .subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      } else {
        this.alarmNotifyRuleApiService
          .addEmailsUsingPOST({
            emailRequest: {
              emailAddress: this.formGroup.value.name,
              desc: this.formGroup.value.description
            }
          })
          .subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      }
    });
  }
}
