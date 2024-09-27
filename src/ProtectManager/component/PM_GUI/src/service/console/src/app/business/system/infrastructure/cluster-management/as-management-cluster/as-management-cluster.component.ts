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
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  ApiMultiClustersService,
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  I18NService
} from 'app/shared';
import { assign, isUndefined } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-as-management-cluster',
  templateUrl: './as-management-cluster.component.html',
  styleUrls: ['./as-management-cluster.component.less']
})
export class AsManagementClusterComponent implements OnInit {
  data;
  formGroup: FormGroup;

  clusterNameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidName: this.i18n.get('common_cluster_name_invalid_label')
  });

  passwordErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [8, 64])
  });

  usernameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [5, 64])
  });

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private clusterApiService: ClustersApiService,
    private baseUtilService: BaseUtilService,
    private multiClustersServiceApi: ApiMultiClustersService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  validLength(min, max): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }
      const value = control.value;
      if (!new RegExp(`^.{${min},${max}}$`).test(value)) {
        return { invalidNameLength: { value: control.value } };
      }
      return null;
    };
  }

  initForm() {
    this.formGroup = this.fb.group({
      clusterName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.clusterName)
        ]
      }),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validLength(5, 64)
        ],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validLength(8, 64)
        ],
        updateOn: 'change'
      })
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      this.multiClustersServiceApi
        .grantToManager({
          request: {
            localClusterName: this.formGroup.value.clusterName,
            username: this.formGroup.value.username,
            password: this.formGroup.value.password,
            syncToRemote: true
          },
          clusterId: this.data.clusterId
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
}
