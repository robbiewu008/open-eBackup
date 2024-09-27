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
import {
  BaseUtilService,
  I18NService,
  ProductStoragesApiService
} from 'app/shared';
import { assign, isEmpty } from 'lodash';
import { Observable, Observer } from 'rxjs';
import { ModalRef } from '@iux/live';

@Component({
  selector: 'aui-add-storage',
  templateUrl: './add-storage.component.html',
  styleUrls: ['./add-storage.component.less']
})
export class AddStorageComponent implements OnInit {
  item;
  formGroup: FormGroup;
  isFocusPassword = false;
  portErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  });
  usernameErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.lengthErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
    }
  );
  passwordErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.lengthErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
    }
  );

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private modal: ModalRef,
    public baseUtilService: BaseUtilService,
    private productStoragesApiService: ProductStoragesApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      ip: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ],
        updateOn: 'change'
      }),
      port: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ],
        updateOn: 'change'
      }),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      password: new FormControl(!isEmpty(this.item) ? '********' : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      })
    });
  }

  updateForm() {
    if (!this.item) {
      return;
    }
    this.formGroup.patchValue(this.item);
    setTimeout(() => {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    }, 0);
  }

  passwordFocus() {
    if (!isEmpty(this.item) && !this.isFocusPassword) {
      this.formGroup.get('password').setValue('');
      this.isFocusPassword = true;
    }
  }

  passwordBlur() {
    if (!isEmpty(this.item) && this.formGroup.value.password === '') {
      this.formGroup.get('password').setValue('********');
      this.isFocusPassword = false;
    }
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      this.productStoragesApiService
        .createStorageUsingPOST({ storageDeviceInfo: this.formGroup.value })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }

  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const storageDeviceInfo = this.formGroup.value;
      if (!this.isFocusPassword) {
        delete storageDeviceInfo.password;
      }

      this.productStoragesApiService
        .updateProductStorageUsingPUT({
          storageId: this.item.id,
          storageDeviceInfo: storageDeviceInfo
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      if (isEmpty(this.item)) {
        this.create().subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
      } else {
        this.modify().subscribe(
          res => {
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
