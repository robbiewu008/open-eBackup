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
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  HoneypotService,
  I18NService
} from 'app/shared';
import { assign, each, includes, isNumber } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-honeypot-setting',
  templateUrl: './honeypot-setting.component.html',
  styleUrls: ['./honeypot-setting.component.less']
})
export class HoneypotSettingComponent implements OnInit {
  data;
  formGroup: FormGroup;
  value1 = 'update';
  value2 = 'none';
  settingTip = this.i18n.get('explore_honeypot_setting_info_label');
  fileName;
  vstoreName;
  isModify = false;

  numErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 30]),
    invalidInput: this.i18n.get('common_valid_integer_label')
  };

  constructor(
    public baseUtilService: BaseUtilService,
    public i18n: I18NService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private honeypotService: HoneypotService
  ) {}

  ngOnInit(): void {
    this.isModify = includes(
      [
        DataMap.fileHoneypotStatus.enable.value,
        DataMap.fileHoneypotStatus.deploy.value
      ],
      this.data[0].status
    );
    this.initForm();
    this.initName();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(''),
      dayNum: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.rangeValue(1, 30),
          this.baseUtilService.VALID.integer()
        ],
        updateOn: 'change'
      }),
      update: new FormControl(this.value1)
    });

    this.formGroup.get('update').valueChanges.subscribe(res => {
      if (res === this.value1) {
        this.formGroup
          .get('dayNum')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.rangeValue(1, 30),
            this.baseUtilService.VALID.integer()
          ]);
        this.formGroup.get('dayNum').updateValueAndValidity();
      } else {
        this.formGroup.get('dayNum').clearValidators();
        this.formGroup.get('dayNum').updateValueAndValidity();
      }
    });

    if (this.isModify) {
      this.formGroup.patchValue({
        name: this.data[0].name,
        update: isNumber(this.data[0].period) ? 'update' : 'none',
        dayNum: isNumber(this.data[0].period) ? this.data[0].period : ''
      });
      this.modal.getInstance().lvOkDisabled = false;
    }
  }

  initName() {
    each(this.data, item => {
      if (this.fileName) {
        this.fileName += `,${item.name}`;
        this.vstoreName += `,${item.extendInfo.tenantName}`;
      } else {
        this.fileName = item.name;
        this.vstoreName = item.extendInfo.tenantName;
      }
    });
  }

  getParams() {
    const honeypotRequests = {
      honeypotRequests: []
    };
    each(this.data, item => {
      honeypotRequests.honeypotRequests.push({
        vstoreName: item.extendInfo.tenantName,
        fsName: item.name,
        period:
          this.formGroup.value.update === 'update'
            ? Number(this.formGroup.value?.dayNum)
            : 0
      });
    });
    const params: any = {
      honeypotRequests: honeypotRequests
    };
    return params;
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const params = this.getParams();
      if (this.isModify) {
        this.honeypotService
          .ModifyHoneypot(
            assign({}, params, {
              akOperationTips: false
            })
          )
          .subscribe(
            (res: any) => {
              observer.next(res);
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      } else {
        this.honeypotService
          .EnableHoneypot(
            assign({}, params, {
              akOperationTips: false
            })
          )
          .subscribe(
            res => {
              observer.next(res);
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      }
    });
  }
}
