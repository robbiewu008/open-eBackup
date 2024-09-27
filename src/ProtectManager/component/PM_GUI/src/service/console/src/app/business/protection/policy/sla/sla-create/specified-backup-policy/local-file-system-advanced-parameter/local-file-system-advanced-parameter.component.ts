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
import { Component, OnInit, Input } from '@angular/core';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import {
  BaseUtilService,
  DataMapService,
  ProtectResourceAction,
  I18NService,
  QosService,
  DataMap
} from 'app/shared';
import { ModalRef } from '@iux/live';
import { assign, map, find, first, size, isUndefined } from 'lodash';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-local-file-system-advanced-parameter',
  templateUrl: './local-file-system-advanced-parameter.component.html',
  styleUrls: ['./local-file-system-advanced-parameter.component.less']
})
export class LocalFileSystemAdvancedParameterComponent implements OnInit {
  find = find;
  qosNames = [];
  protectResourceAction = ProtectResourceAction;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  @Input() isSlaDetail: boolean;
  @Input() action: any;
  @Input() data: any;
  @Input() formGroup: FormGroup;
  retryTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 5])
  });
  waitTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 30])
  });
  fullCopyPeriodErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 100])
  });

  constructor(
    public baseUtilService: BaseUtilService,
    public i18n: I18NService,
    public fb: FormBuilder,
    public modal: ModalRef,
    private qosServiceApi: QosService,
    public dataMapService: DataMapService,
    public appUtilsService?: AppUtilsService
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
    if (this.isHyperdetect) {
      this.hyperdetectUpdataForm();
    } else {
      this.normalUpdataForm();
    }
  }

  hyperdetectUpdataForm() {
    this.formGroup.addControl('is_security_snap', new FormControl(false));
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

  normalUpdataForm() {
    this.formGroup.addControl('qos_id', new FormControl(''));
    this.formGroup.addControl('auto_index', new FormControl(false));
    this.formGroup.addControl('open_aggregation', new FormControl(true));
    this.formGroup.addControl('network_acceleration', new FormControl(false));
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

    this.formGroup.addControl(
      'is_synthetic_full_copy_period',
      new FormControl(true)
    );
    this.formGroup.addControl(
      'synthetic_full_copy_period',
      new FormControl(10, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 100)
        ]
      })
    );

    this.formGroup.get('network_acceleration').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup.get('qos_id').setValue('');
      }
    });

    this.formGroup.get('qos_id').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup.get('network_acceleration').setValue(false);
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

    this.formGroup
      .get('is_synthetic_full_copy_period')
      .valueChanges.subscribe(res => {
        if (!res) {
          this.formGroup.removeControl('synthetic_full_copy_period');
        } else {
          this.formGroup.addControl(
            'synthetic_full_copy_period',
            new FormControl(1, {
              validators: [
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(1, 100)
              ]
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
