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
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import {
  BaseUtilService,
  I18NService,
  QosService,
  CommonConsts,
  WarningMessageService,
  deepEqualObject
} from 'app/shared';
import { Observable, Observer } from 'rxjs';
import { assign, pick, keys } from 'lodash';

@Component({
  selector: 'aui-create-limit-rate-policy',
  templateUrl: './create-limit-rate-policy.component.html',
  styleUrls: ['./create-limit-rate-policy.component.less']
})
export class CreateLimitRatePolicyComponent implements OnInit {
  rowItem;
  formGroup: FormGroup;
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_name_label')
  };
  bandwidthErrorTip = assign(
    {},
    {
      ...this.baseUtilService.requiredErrorTip,
      ...this.baseUtilService.integerErrorTip,
      ...this.baseUtilService.rangeErrorTip
    },
    {
      invalidRang: this.i18n.get('common_valid_rang_label', [10, 5120])
    }
  );

  constructor(
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService,
    private i18n: I18NService,
    private qosServiceApi: QosService,
    private warningMessageService: WarningMessageService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(
        this.rowItem && this.rowItem.name ? this.rowItem.name : '',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.name)
          ]
        }
      ),
      speed_limit: new FormControl(
        this.rowItem && this.rowItem.speed_limit
          ? this.rowItem.speed_limit
          : '1024',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(10, 5120)
          ]
        }
      ),
      description: new FormControl(
        this.rowItem && this.rowItem.description ? this.rowItem.description : ''
      )
    });
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.qosServiceApi
        .createQosV1QosPost({ body: { ...this.formGroup.value } })
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
      if (
        !deepEqualObject(
          pick(this.rowItem, keys(this.formGroup.value)),
          this.formGroup.value
        )
      ) {
        this.warningMessageService.create({
          content: this.i18n.get('protection_modify_qos_warn_label', [
            this.rowItem.name
          ]),
          onOK: () => {
            this.qosServiceApi
              .updateQosV1QosQosIdPut({
                qosId: this.rowItem.uuid,
                body: { ...this.formGroup.value }
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
          },
          onCancel: () => {
            observer.error(null);
            observer.complete();
          },
          lvAfterClose: result => {
            if (result && result.trigger === 'close') {
              observer.error(null);
              observer.complete();
            }
          }
        });
      } else {
        this.qosServiceApi
          .updateQosV1QosQosIdPut({
            qosId: this.rowItem.uuid,
            body: { ...this.formGroup.value }
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
      }
    });
  }

  ngOnInit() {
    this.initForm();
  }
}
