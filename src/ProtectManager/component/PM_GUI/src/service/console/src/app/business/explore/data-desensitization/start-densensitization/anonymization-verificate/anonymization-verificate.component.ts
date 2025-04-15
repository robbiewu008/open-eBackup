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
  AnonyControllerService,
  DatabaseControllerService,
  DataMap,
  DesensitizationSourceType
} from 'app/shared';
import { BaseUtilService, I18NService } from 'app/shared/services';
import { assign, each } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-anonymization-verificate',
  templateUrl: './anonymization-verificate.component.html',
  styleUrls: ['./anonymization-verificate.component.less']
})
export class AnonymizationVerificateComponent implements OnInit {
  rowItem;
  formGroup: FormGroup;
  nameErrorTip = { ...this.baseUtilService.requiredErrorTip };
  passwordErrorTip = { ...this.baseUtilService.requiredErrorTip };
  portErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip
  };
  startErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidMinSize: this.i18n.get('common_valid_minsize_label', [1])
  };
  limitErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
  });
  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private jobServicApiService: AnonyControllerService,
    private sqlVerificateApiService: DatabaseControllerService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      ip: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ]
      }),
      port: new FormControl(this.getPort(), {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer()
        ]
      }),
      start_row: new FormControl(1),
      limit: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 10)
        ]
      })
    });
    if (this.rowItem.isVerificationPost) {
      this.formGroup
        .get('start_row')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.minSize(0)
        ]);
      this.formGroup
        .get('limit')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 10)
        ]);
      this.formGroup.get('start_row').updateValueAndValidity();
      this.formGroup.get('limit').updateValueAndValidity();
    }
  }

  getPort() {
    if (!this.rowItem) {
      return '';
    }
    switch (this.rowItem.sub_type) {
      case DataMap.Resource_Type.oracle.value:
        return '1521';
      case DataMap.Resource_Type.SQLServer.value:
        return '1433';
      case DataMap.Resource_Type.MySQL.value:
        return '3306';
      default:
        return '';
    }
  }

  verificate(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const columns = [];
      if (this.rowItem.columns) {
        each(this.rowItem.columns, item => {
          columns.push(item.label);
        });
      }

      this.sqlVerificateApiService
        .verifyDatabaseUsingPOST({
          request: {
            column_list: columns,
            db_obj: {
              DBID: this.rowItem.uuid,
              DBName: this.rowItem.name,
              DBType: DesensitizationSourceType[this.rowItem.sub_type],
              IPAdress: this.formGroup.value.ip,
              Password: this.formGroup.value.password,
              Port: this.formGroup.value.port,
              Username: this.formGroup.value.name
            },
            table_name: this.rowItem.columns[0].parent.label,
            table_space: this.rowItem.columns[0].parent.parent.label,
            limit: +this.formGroup.value.limit,
            start_row: +this.formGroup.value.start_row
          }
        })
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
    });
  }

  identify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.jobServicApiService
        .createAnonyJobUsingPOST({
          createRequest: {
            anonymization_type: 'Offline',
            db_obj: {
              DBID: this.rowItem.uuid,
              DBName: this.rowItem.name,
              DBType: DesensitizationSourceType[this.rowItem.sub_type],
              IPAdress: this.formGroup.value.ip,
              Password: this.formGroup.value.password,
              Port: this.formGroup.value.port,
              Username: this.formGroup.value.name
            },
            job_type: 'Identification',
            policy_id: this.rowItem.policy_id,
            policy_name: this.rowItem.policy_name
          }
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

  desensitizate(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.jobServicApiService
        .createAnonyJobUsingPOST({
          createRequest: {
            anonymization_type: 'Offline',
            db_obj: {
              DBID: this.rowItem.uuid,
              DBName: this.rowItem.name,
              DBType: DesensitizationSourceType[this.rowItem.sub_type],
              IPAdress: this.formGroup.value.ip,
              Password: this.formGroup.value.password,
              Port: this.formGroup.value.port,
              Username: this.formGroup.value.name
            },
            job_type: 'Anonymization',
            policy_id: this.rowItem.desesitization_policy_id,
            policy_name: this.rowItem.desesitization_policy_name
          }
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

  ngOnInit() {
    this.initForm();
  }
}
