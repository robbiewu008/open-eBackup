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
import { Component, Input, OnInit } from '@angular/core';
import { FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  I18NService,
  ProtectResourceAction,
  QosService,
  RouterUrl
} from 'app/shared';
import { assign, find, first, map, size } from 'lodash';

@Component({
  selector: 'aui-hyper-v-parameter',
  templateUrl: './hyper-v-parameter.component.html',
  styleUrls: ['./hyper-v-parameter.component.less']
})
export class HyperVParameterComponent implements OnInit {
  find = find;
  qosNames = [];
  protectResourceAction = ProtectResourceAction;
  @Input() isSlaDetail: boolean;
  @Input() action: any;
  @Input() data: any;
  @Input() formGroup: FormGroup;
  hostConcurrentBackupErrorTip = assign(
    {},
    this.baseUtilService.rangeErrorTip,
    {
      invalidRang: this.i18n.get('common_valid_rang_label', [2, 10])
    }
  );
  retryTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 5])
  });
  waitTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 30])
  });

  ratePolicyRouterUrl = RouterUrl.ProtectionLimitRatePolicy;

  constructor(
    private baseUtilService: BaseUtilService,
    public i18n: I18NService,
    private qosServiceApi: QosService
  ) {}

  ngOnInit() {
    this.getQosNames();
    this.updateForm();
    this.updateData();
  }

  updateData() {
    if (!this.data || !size(this.data)) {
      return;
    }

    const data = first(map(this.data, 'ext_parameters'));
    if (data) {
      this.formGroup.patchValue(data);
    }
  }

  updateForm() {
    this.formGroup.addControl('use_client_thread', new FormControl(false));
    this.formGroup.addControl(
      'use_client_num',
      new FormControl(2, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(2, 10)
        ],
        updateOn: 'change'
      })
    );
    this.formGroup.addControl('encryption', new FormControl(false));
    this.formGroup.addControl('qos_id', new FormControl(''));
    this.formGroup.addControl('auto_retry', new FormControl(true));
    this.formGroup.addControl(
      'auto_retry_times',
      new FormControl(3, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 5)
        ],
        updateOn: 'change'
      })
    );
    this.formGroup.addControl(
      'auto_retry_wait_minutes',
      new FormControl(5, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 30)
        ],
        updateOn: 'change'
      })
    );

    this.formGroup.get('use_client_thread').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.removeControl('use_client_num');
      } else {
        this.formGroup.addControl(
          'use_client_num',
          new FormControl(2, {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(2, 10)
            ],
            updateOn: 'change'
          })
        );
      }
    });

    this.formGroup.get('auto_retry').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.removeControl('auto_retry_times');
        this.formGroup.removeControl('auto_retry_wait_minutes');
      } else {
        this.formGroup.addControl(
          'auto_retry_times',
          new FormControl(3, {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 5)
            ],
            updateOn: 'change'
          })
        );
        this.formGroup.addControl(
          'auto_retry_wait_minutes',
          new FormControl(5, {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 30)
            ],
            updateOn: 'change'
          })
        );
      }
    });
  }

  getQosNames() {
    this.qosServiceApi
      .queryResourcesV1QosGet({
        pageNo: 0,
        pageSize: 100
      })
      .subscribe(res => {
        this.qosNames = map(res.items, (item: any) => {
          item['isLeaf'] = true;
          item['label'] = item.name;
          return item;
        });
      });
  }
}
