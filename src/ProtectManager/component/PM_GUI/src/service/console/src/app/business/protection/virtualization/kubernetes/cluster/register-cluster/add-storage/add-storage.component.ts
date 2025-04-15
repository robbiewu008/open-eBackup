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
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { BaseUtilService, I18NService } from 'app/shared';
import { Observable, Observer } from 'rxjs';
import {
  assign,
  defer,
  each,
  includes,
  isArray,
  isEmpty,
  split,
  trim
} from 'lodash';
import { ModalRef } from '@iux/live';

@Component({
  selector: 'aui-add-storage',
  templateUrl: './add-storage.component.html',
  styleUrls: ['./add-storage.component.less']
})
export class AddStorageComponent implements OnInit {
  rowItem;
  formGroup: FormGroup;
  data = {};
  ipRepeat = false;
  repeatTips;

  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private modal: ModalRef
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateData();
  }

  initForm() {
    this.formGroup = this.fb.group({
      ip: this.fb.array([
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip()
          ]
        })
      ]),
      port: new FormControl(this.rowItem?.port || '8088', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      }),
      username: new FormControl(this.rowItem?.username || '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });

    this.formGroup.statusChanges.subscribe(() =>
      defer(() => this.disableOkBtn())
    );
    this.formGroup
      .get('ip')
      .statusChanges.subscribe(() => defer(() => this.validRepeatIp()));
  }

  updateData() {
    if (isEmpty(this.rowItem)) {
      return;
    }
    (this.formGroup.get('ip') as FormArray).clear();
    each(this.rowItem.ipList.split(','), () => this.addIp());
    this.formGroup.get('ip').setValue(this.rowItem.ipList.split(','));
  }
  validRepeatIp() {
    const ip = this.formGroup.get('ip').value;
    const allIps = [];
    const repeatIps = [];
    each(ip, ipItem => {
      if (!trim(ipItem)) {
        return;
      }
      if (!includes(allIps, ipItem)) {
        allIps.push(ipItem);
      } else {
        repeatIps.push(ipItem);
      }
    });
    if (repeatIps.length) {
      this.ipRepeat = true;
      this.repeatTips = this.i18n.get('common_same_ip_tips_label', [
        repeatIps.join(this.i18n.isEn ? ',' : '，')
      ]);
    } else {
      this.ipRepeat = false;
    }
  }

  get ipArr() {
    return (this.formGroup.get('ip') as FormArray).controls;
  }

  addIp() {
    (this.formGroup.get('ip') as FormArray).push(
      new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ]
      })
    );
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.status === 'INVALID' || this.ipRepeat;
  }

  deleteIp(i) {
    (this.formGroup.get('ip') as FormArray).removeAt(i);
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = { ...this.formGroup.value };
      params.ip = this.formGroup.value.ip[0];
      params.ipList = this.formGroup.value.ip.join(',');
      observer.next(params);

      observer.complete();
    });
  }
}
