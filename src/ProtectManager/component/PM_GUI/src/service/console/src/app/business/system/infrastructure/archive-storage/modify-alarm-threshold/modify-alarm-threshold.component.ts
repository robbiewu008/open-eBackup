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
import { ModalRef, OptionItem } from '@iux/live';
import {
  BaseUtilService,
  DataMapService,
  I18NService,
  StoragesAlarmThresholdApiService
} from 'app/shared';
import { assign, isUndefined, toString } from 'lodash';
import { Observable, Observer } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-modify-alarm-threshold',
  templateUrl: './modify-alarm-threshold.component.html',
  styleUrls: ['./modify-alarm-threshold.component.less']
})
export class ModifyAlarmThresholdComponent implements OnInit {
  data;
  formData = {} as any;

  formGroup: FormGroup;
  alarmLevelItems: OptionItem[];
  recoverValueUnits: OptionItem[];
  limitValueUnits: OptionItem[];
  recoverValueErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    error: this.i18n.get('system_clear_threshold_error_label'),
    minSize: this.i18n.get('common_valid_minsize_label', [1])
  });
  alarmThresholdErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    error: this.i18n.get('system_limit_threshold_error_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [50, 90]),
    invalidMinSize: this.i18n.get('common_valid_minsize_label', [1])
  });
  triggerThresholdErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
  });
  timesLabel = this.i18n.get('common_times_label');
  percentageLabel = this.i18n.get('common_percentage_label');
  thresholdTypeLabel = this.i18n.get('system_threshold_type_label');
  absoluteValueLabel = this.i18n.get('common_absolute_value_label');
  alarmThresholdLabel = this.i18n.get('common_alarm_threshold_label');
  clearThresholdLabel = this.i18n.get('system_clear_threshold_label');
  triggerThresholdLabel = this.i18n.get('system_trigger_threshold_label');
  alarmLevelLabel = this.i18n.get('system_alarm_severity_label');

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public modal: ModalRef,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    public thresholdApiService: StoragesAlarmThresholdApiService
  ) {}

  ngOnInit() {
    this.initItems();
    this.initForm();
    this.getData();
  }

  getData() {
    this.thresholdApiService
      .queryThresholdUsingGET({
        id: this.data.repositoryId
      })
      .pipe(
        map((res: any) => {
          res.type = toString(res.type);
          res.alarmLevel = toString(res.alarmLevel);
          res.recoverValueUnit = toString(res.recoverValueUnit) || '1';
          res.limitValueUnit = toString(res.limitValueUnit) || '1';
          return res;
        })
      )
      .subscribe(res => {
        this.formData = res;
        this.formGroup.patchValue(res);
        if (+res.type === 2) {
          this.formGroup
            .get('limitValue')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.validLimitValue()
            ]);
        } else {
          this.formGroup
            .get('limitValue')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(50, 90),
              this.validLimitValue()
            ]);
        }
        this.formGroup
          .get('recoverValue')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.validRecoverValue()
          ]);
        this.formGroup.get('recoverValue').updateValueAndValidity();
        this.formGroup.get('limitValue').updateValueAndValidity();
        this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
      });
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl('1'),
      limitValue: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(50, 90)
        ],
        updateOn: 'change'
      }),
      recoverValue: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer()
        ],
        updateOn: 'change'
      }),
      matchTime: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 10)
        ],
        updateOn: 'change'
      }),
      alarmLevel: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      recoverValueUnit: new FormControl('1', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      limitValueUnit: new FormControl('1', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    });

    this.formGroup.statusChanges.subscribe(res => {
      this.modal.getInstance().lvOkDisabled = res !== 'VALID';
    });

    this.formGroup.get('limitValue').valueChanges.subscribe(res => {
      setTimeout(() => {
        if (!this.formGroup.get('recoverValue').value) {
          return;
        }
        this.formGroup.get('recoverValue').markAsTouched();
        this.formGroup.get('recoverValue').updateValueAndValidity();
      }, 0);
    });

    this.formGroup.get('recoverValue').valueChanges.subscribe(res => {
      setTimeout(() => {
        if (!this.formGroup.get('limitValue').value) {
          return;
        }
        this.formGroup.get('limitValue').markAsTouched();
        this.formGroup.get('limitValue').updateValueAndValidity();
      }, 0);
    });
  }

  validLimitValue(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || !this.formGroup.value.recoverValue) {
        return null;
      }

      let limitValue = +control.value;
      let recoverValue = +this.formGroup.value.recoverValue;

      const type = +this.formGroup.value.type;
      if (type === 1) {
        if (recoverValue >= limitValue) {
          return { error: { value: control.value } };
        }

        return null;
      }

      const limitValueUnit = this.formGroup.value.limitValueUnit;
      const recoverValueUnit = this.formGroup.value.recoverValueUnit;
      if (limitValueUnit === '1') {
        limitValue = limitValue * 1024;
      }

      if (recoverValueUnit === '1') {
        recoverValue = recoverValue * 1024;
      }

      if (recoverValue >= limitValue) {
        return { error: { value: control.value } };
      }

      return null;
    };
  }

  validRecoverValue(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || !this.formGroup.value.limitValue) {
        return null;
      }

      let recoverValue = +control.value;
      let limitValue = +this.formGroup.value.limitValue;

      if (recoverValue < 1) {
        return { minSize: { value: control.value } };
      }

      const type = +this.formGroup.value.type;
      if (type === 1) {
        if (recoverValue >= limitValue) {
          return { error: { value: control.value } };
        }

        return null;
      }

      const limitValueUnit = this.formGroup.value.limitValueUnit;
      const recoverValueUnit = this.formGroup.value.recoverValueUnit;
      if (limitValueUnit === '1') {
        limitValue = limitValue * 1024;
      }

      if (recoverValueUnit === '1') {
        recoverValue = recoverValue * 1024;
      }

      if (recoverValue >= limitValue) {
        return { error: { value: control.value } };
      }

      return null;
    };
  }

  clearLimitValue(type, isClearValue = false) {
    if (isClearValue) {
      this.formGroup.patchValue({
        limitValue: '',
        recoverValue: ''
      });
    }
    if (type === '2') {
      this.formGroup
        .get('limitValue')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.validLimitValue()
        ]);
    } else {
      this.formGroup
        .get('limitValue')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(50, 90),
          this.validLimitValue()
        ]);
    }
    this.formGroup
      .get('recoverValue')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.integer(),
        this.validRecoverValue()
      ]);
    this.formGroup.get('recoverValue').updateValueAndValidity();
    this.formGroup.get('limitValue').updateValueAndValidity();
  }

  initItems() {
    this.alarmLevelItems = this.dataMapService
      .toArray('Alarm_Severity')
      .filter((v: OptionItem) => {
        v.isLeaf = true;
        return v.key === '3' || v.key === '4';
      });
    this.alarmLevelItems.reverse();
    this.recoverValueUnits = this.limitValueUnits = [
      {
        key: '1',
        label: 'TB',
        isLeaf: true
      },
      {
        key: '2',
        label: 'GB',
        isLeaf: true
      }
    ];
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      let limitValue = this.formGroup.get('limitValue').value;
      let recoverValue = this.formGroup.get('recoverValue').value;
      const type = this.formGroup.get('type').value;
      if (type === '2') {
        const alarmUnit = this.formGroup.get('recoverValueUnit').value;
        const clearUnit = this.formGroup.get('limitValueUnit').value;
        if (alarmUnit === '1') {
          limitValue = +limitValue * 1024;
        }

        if (clearUnit === '1') {
          recoverValue = +recoverValue * 1024;
        }
      }

      this.thresholdApiService
        .updateThresholdUsingPUT1({
          id: this.data.repositoryId,
          repositoryThresholdRequest: this.formGroup.value
        })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: error => {
            observer.error(error);
            observer.complete();
          }
        });
    });
  }
}
