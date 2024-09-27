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
import { ModalRef } from '@iux/live';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { Component, OnInit } from '@angular/core';
import { Observable, Observer } from 'rxjs';
import { I18NService, BaseUtilService } from 'app/shared';
import { find, get, isUndefined, set } from 'lodash';

@Component({
  selector: 'aui-config-node',
  templateUrl: './config-node.component.html',
  styleUrls: ['./config-node.component.less']
})
export class ConfigNodeComponent implements OnInit {
  rowData;
  name;
  parentUuid;
  children = [];
  isTest = false;
  okLoading = false;
  testLoading = false;
  options = [];
  hostOptions = [];
  formGroup: FormGroup;

  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  constructor(
    private fb: FormBuilder,
    public modal: ModalRef,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initData();
  }

  initForm() {
    this.formGroup = this.fb.group({
      parentUuid: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      osUser: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      })
    });
  }

  initData() {
    this.hostOptions = this.options;
    if (this.rowData?.parentUuid) {
      const originalHost = find(this.options, {
        uuid: this.rowData.parentUuid
      });

      if (isUndefined(originalHost)) {
        return;
      }
      this.formGroup.get('osUser').setValue(this.rowData.osUser);
      this.formGroup.get('parentUuid').setValue(get(originalHost, 'uuid'));
    }
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const host = find(this.hostOptions, {
        uuid: this.formGroup.value.parentUuid
      });
      set(this.rowData, 'parentUuid', this.formGroup.value.parentUuid);
      set(
        this.rowData,
        'parentName',
        `${get(host, 'name')}(${get(host, 'endpoint')})`
      );
      set(this.rowData, 'osUser', this.formGroup.value.osUser);
      observer.next();
      observer.complete();
    });
  }
}
