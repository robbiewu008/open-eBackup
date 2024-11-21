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
  FormGroup,
  FormBuilder,
  FormControl,
  ValidatorFn,
  AbstractControl
} from '@angular/forms';
import { Component, OnInit, ViewChild, TemplateRef } from '@angular/core';
import {
  I18NService,
  BaseUtilService,
  DataMap,
  DataMapService,
  ProtectedResourceApiService
} from 'app/shared';
import { Observable, Observer } from 'rxjs';
import { each, includes, uniqueId } from 'lodash';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-add-instance',
  templateUrl: './add-instance.component.html',
  styleUrls: ['./add-instance.component.less']
})
export class AddInstanceComponent implements OnInit {
  dataMap = DataMap;
  hostOptions = [];
  formGroup: FormGroup;
  authOptions = this.dataMapService
    .toArray('Database_Auth_Method')
    .filter(item => {
      return (item.isLeaf = true);
    });

  pwdComplexTipLabel = this.i18n.get(
    'common_dameng_register_possword_tips_label'
  );

  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  usernameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  passwordErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [48]),
    unsupportValueError: this.i18n.get('common_dameng_password_error_label')
  };

  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;
  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.getProxyOptions();
    this.initForm();
  }

  addData(array: any[], item) {
    array.push({
      ...item,
      key: item.uuid,
      value: item.uuid,
      label: item.name + `(${item.endpoint})`,
      isLeaf: true
    });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.Dameng_cluster.value}Plugin`],
        environment: {
          linkStatus: [['=='], 1]
        }
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true
          });
        });
        this.hostOptions = hostArray;
      }
    );
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      host: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      port: new FormControl(5236, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ],
        updateOn: 'change'
      }),
      auth_method: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      database_username: new FormControl(''),
      database_password: new FormControl('')
    });
    this.watch();
  }

  watch() {
    this.formGroup.get('auth_method').valueChanges.subscribe(res => {
      if (res === this.dataMap.Database_Auth_Method.db.value) {
        this.formGroup
          .get('database_username')
          .setValidators([
            this.baseUtilService.VALID.name(),
            this.baseUtilService.VALID.required()
          ]);
        this.formGroup
          .get('database_password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.vaildPasswords(),
            this.baseUtilService.VALID.maxLength(48)
          ]);
      } else {
        this.formGroup.get('database_username').clearValidators();
        this.formGroup.get('database_password').clearValidators();
      }
      this.formGroup.get('database_username').updateValueAndValidity();
      this.formGroup.get('database_password').updateValueAndValidity();
    });
  }

  vaildPasswords(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const password = control.value;
      if (includes(password, ' ')) {
        return { unsupportValueError: { value: control.value } };
      }
      return null;
    };
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }
      observer.next({ ...this.formGroup.value, id: uniqueId() });
      observer.complete();
    });
  }
}
