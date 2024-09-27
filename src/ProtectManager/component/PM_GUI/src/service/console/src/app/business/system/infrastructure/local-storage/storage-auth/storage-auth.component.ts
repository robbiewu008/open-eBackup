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
import { Component, EventEmitter, OnInit, Output } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  I18NService,
  LANGUAGE,
  LicenseApiService,
  LocalStorageApiService,
  SupportLicense
} from 'app/shared';
import { each, find, includes, isFunction, omit } from 'lodash';

@Component({
  selector: 'aui-storage-auth',
  templateUrl: './storage-auth.component.html',
  styleUrls: ['./storage-auth.component.less']
})
export class StorageAuthComponent implements OnInit {
  isView = true;
  isAuth = true;
  formGroup: FormGroup;
  authForm: FormGroup;
  disabled = false;
  serviceAble = false;
  managerAble = false;
  isEn = false;

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  pwdErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  @Output() onStatusChange = new EventEmitter<any>();

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private localStorageApiService: LocalStorageApiService,
    private licenseApiService: LicenseApiService
  ) {}

  ngOnInit() {
    this.isEn = this.i18n.language.toLocaleLowerCase() === LANGUAGE.EN;
    this.initForm();
    this.getData();
  }

  initForm() {
    this.formGroup = this.fb.group({
      status: new FormControl(),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      })
    });
    this.authForm = this.fb.group({
      status: new FormControl(),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      })
    });
  }

  getData(callback?: () => void) {
    this.localStorageApiService.getLocalStorageAuthUsingGET({}).subscribe(
      res => {
        each(res, item => {
          if (item.authType === 'managerAuth') {
            this.managerAble = item.modifiable === '1';
            this.formGroup.patchValue({ ...item, password: '' });
          } else {
            this.serviceAble = item.modifiable === '1';
            this.authForm.patchValue({ ...item, password: '' });
          }
        });

        isFunction(callback) && callback();
        this.onStatusChange.emit(res);
      },
      () => {
        isFunction(callback) && callback();
      }
    );
  }

  save() {
    let params = {
      password: '',
      userName: '',
      authType: ''
    };
    if (this.formGroup.valid) {
      params = {
        password: this.formGroup.value.password,
        userName: this.formGroup.value.userName,
        authType: 'managerAuth'
      };
    }

    if (this.authForm.valid) {
      params = {
        password: this.authForm.value.password,
        userName: this.authForm.value.userName,
        authType: 'serviceAuth'
      };
    }

    this.localStorageApiService
      .updateLocalStorageAuthUsingPUT({
        request: params
      })
      .subscribe(() => {
        this.cancel();

        // 防勒索场景要获取license最新数据
        if (
          includes(
            [DataMap.Deploy_Type.hyperdetect.value],
            this.i18n.get('deploy_type')
          )
        ) {
          this.getLicense();
        }
      });
  }

  getLicense() {
    this.licenseApiService
      .queryLicenseUsingGET({ akLoading: false, akDoException: false })
      .subscribe(res => {
        SupportLicense.isInit = true;
        SupportLicense.isFile = !!find(
          res,
          item => item.name === 'HyperDetect (anti-Ransomware)'
        );
        SupportLicense.isSan = !!find(
          res,
          item => item.name === 'HyperDetect for SAN'
        );

        SupportLicense.isBoth = SupportLicense.isFile && SupportLicense.isSan;
      });
  }

  cancel() {
    this.getData(() => {
      this.isView = true;
      this.isAuth = true;
    });
  }

  authService() {
    this.getData(() => {
      this.isAuth = !this.isAuth;
    });
  }

  modify() {
    this.getData(() => {
      this.isView = !this.isView;
    });
  }
}
