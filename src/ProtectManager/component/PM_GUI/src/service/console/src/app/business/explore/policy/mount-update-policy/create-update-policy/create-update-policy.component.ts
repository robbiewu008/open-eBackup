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
import { DatePipe } from '@angular/common';
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef, OptionItem } from '@iux/live';
import {
  BaseUtilService,
  CopyDataSelectionPolicy,
  DataMap,
  DataMapService,
  I18NService,
  LiveMountPolicyApiService,
  RetentionPolicy,
  SchedulePolicy
} from 'app/shared';
import { assign, includes, omit } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-update-create-policy',
  templateUrl: './create-update-policy.component.html',
  styleUrls: ['./create-update-policy.component.less'],
  providers: [DatePipe]
})
export class CreateUpdatePolicyComponent implements OnInit {
  data;
  formGroup: FormGroup;
  schedulePolicy = SchedulePolicy;
  retentionPolicy = RetentionPolicy;
  copyDataSelectionPolicy = CopyDataSelectionPolicy;
  retentionUnitDisabled = false;
  scheduleStartTimeDisabled = false;
  scheduleIntervalUnitDisabled = false;
  copyDataSelectionPolicys = this.dataMapService
    .toArray('CopyData_Selection_Policy')
    .filter((v: OptionItem) => {
      return (v.isLeaf = true);
    });
  scheduleIntervalUnits = this.dataMapService
    .toArray('Interval_Unit')
    .filter(v => {
      return includes(
        [
          DataMap.Interval_Unit.hour.value,
          DataMap.Interval_Unit.day.value,
          DataMap.Interval_Unit.week.value
        ],
        v.value
      );
    })
    .filter(v => {
      return (v.isLeaf = true);
    });
  retentionUnits = this.dataMapService
    .toArray('Interval_Unit')
    .filter(v => {
      return includes(
        [
          DataMap.Interval_Unit.day.value,
          DataMap.Interval_Unit.week.value,
          DataMap.Interval_Unit.month.value,
          DataMap.Interval_Unit.year.value
        ],
        v.value
      );
    })
    .filter(v => {
      return (v.isLeaf = true);
    });
  scheduleIntervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 23])
  });
  retentionValueErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
  });
  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public modal: ModalRef,
    public datePipe: DatePipe,
    public baseUtilService: BaseUtilService,
    public dataMapService: DataMapService,
    public liveMountPolicyApiService: LiveMountPolicyApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateData();
  }

  updateData() {
    if (!this.data) {
      return;
    }

    let startTime = new Date(this.data.scheduleStartTime);
    if (startTime && !startTime.getTime()) {
      startTime = new Date(this.data.scheduleStartTime?.replace(/-/g, '/'));
    }
    assign(this.data, {
      scheduleStartTime:
        this.data.schedulePolicy === SchedulePolicy.PeriodSchedule
          ? startTime
          : null,
      copyDataPolicy: this.data.copyDataSelectionPolicy
    });

    this.formGroup.patchValue(this.data);

    if (
      this.formGroup.value.scheduleIntervalUnit &&
      this.formGroup.value.schedulePolicy === SchedulePolicy.PeriodSchedule
    ) {
      this.changeTimeUnits(
        this.formGroup.value.scheduleIntervalUnit,
        'scheduleInterval'
      );
    }

    if (
      this.formGroup.value.retentionUnit &&
      this.formGroup.value.retentionPolicy === RetentionPolicy.FixedTime
    ) {
      this.changeTimeUnits(
        this.formGroup.value.retentionUnit,
        'retentionValue'
      );
    }

    setTimeout(() => {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    }, 0);
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ],
        updateOn: 'change'
      }),
      copyDataSelectionPolicy: new FormControl(
        CopyDataSelectionPolicy.LastHour
      ),
      copyDataPolicy: new FormControl(CopyDataSelectionPolicy.LastHour),
      schedulePolicy: new FormControl(SchedulePolicy.PeriodSchedule),
      scheduleInterval: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 23)
        ],
        updateOn: 'change'
      }),
      scheduleIntervalUnit: new FormControl(DataMap.Interval_Unit.hour.value),
      scheduleStartTime: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      retentionPolicy: new FormControl(RetentionPolicy.FixedTime),
      retentionValue: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 365)
        ],
        updateOn: 'change'
      }),
      retentionUnit: new FormControl(DataMap.Interval_Unit.day.value)
    });
    this.formGroup.get('copyDataPolicy').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.formGroup.get('copyDataSelectionPolicy').setValue(res);
    });

    this.formGroup.get('schedulePolicy').valueChanges.subscribe(res => {
      this.scheduleStartTimeDisabled = res == SchedulePolicy.AfterBackupDone;
      this.scheduleIntervalUnitDisabled = res == SchedulePolicy.AfterBackupDone;
      if (res == SchedulePolicy.AfterBackupDone) {
        this.formGroup.get('scheduleStartTime').clearValidators();
        this.formGroup.get('scheduleInterval').clearValidators();
      } else {
        this.formGroup
          .get('scheduleIntervalUnit')
          .setValue(DataMap.Interval_Unit.hour.value);
        this.formGroup
          .get('scheduleStartTime')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('scheduleInterval').updateValueAndValidity();
      this.formGroup.get('scheduleStartTime').updateValueAndValidity();
    });

    this.formGroup.get('retentionPolicy').valueChanges.subscribe(res => {
      this.retentionUnitDisabled = res !== RetentionPolicy.FixedTime;
      if (res === RetentionPolicy.FixedTime) {
        this.formGroup
          .get('retentionUnit')
          .setValue(DataMap.Interval_Unit.day.value);
      } else {
        this.formGroup.get('retentionValue').clearValidators();
      }
      this.formGroup.get('retentionValue').updateValueAndValidity();
    });

    this.formGroup.get('scheduleIntervalUnit').valueChanges.subscribe(res => {
      setTimeout(() => {
        if (!this.formGroup.get('scheduleInterval').value) {
          return;
        }
        this.formGroup.get('scheduleInterval').markAsTouched();
        this.formGroup.get('scheduleInterval').updateValueAndValidity();
      }, 0);
    });

    this.formGroup.get('retentionUnit').valueChanges.subscribe(res => {
      setTimeout(() => {
        if (!this.formGroup.get('retentionValue').value) {
          return;
        }
        this.formGroup.get('retentionValue').markAsTouched();
        this.formGroup.get('retentionValue').updateValueAndValidity();
      }, 0);
    });
  }

  changeTimeUnits(value, formControlName) {
    if (value === DataMap.Interval_Unit.hour.value) {
      this.formGroup
        .get(formControlName)
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 23)
        ]);
      if ('scheduleInterval' === formControlName) {
        this.scheduleIntervalErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 23])
          }
        );
      } else if ('retentionValue' === formControlName) {
        this.retentionValueErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 23])
          }
        );
      }
    } else if (value === DataMap.Interval_Unit.day.value) {
      if ('scheduleInterval' === formControlName) {
        this.formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 30)
          ]);
        this.scheduleIntervalErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 30])
          }
        );
      } else if ('retentionValue' === formControlName) {
        this.formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 365)
          ]);
        this.retentionValueErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
          }
        );
      }
    } else if (value === DataMap.Interval_Unit.week.value) {
      if ('scheduleInterval' === formControlName) {
        this.formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 4)
          ]);
        this.scheduleIntervalErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 4])
          }
        );
      } else if ('retentionValue' === formControlName) {
        this.formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 54)
          ]);
        this.retentionValueErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 54])
          }
        );
      }
    } else if (value === DataMap.Interval_Unit.month.value) {
      if ('scheduleInterval' === formControlName) {
        this.formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 12)
          ]);
        this.scheduleIntervalErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 12])
          }
        );
      } else if ('retentionValue' === formControlName) {
        this.formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 24)
          ]);
        this.retentionValueErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 24])
          }
        );
      }
    } else if (value === DataMap.Interval_Unit.year.value) {
      this.formGroup
        .get(formControlName)
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 10)
        ]);
      if ('scheduleInterval' === formControlName) {
        this.scheduleIntervalErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
          }
        );
      } else if ('retentionValue' === formControlName) {
        this.retentionValueErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
          }
        );
      }
    }
    this.formGroup.get(formControlName).updateValueAndValidity();
  }

  onCreate(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const createRequest = this.formGroup.value;
      assign(createRequest, {
        scheduleStartTime: this.datePipe.transform(
          createRequest.scheduleStartTime,
          'yyyy-MM-dd HH:mm:ss'
        )
      });
      this.liveMountPolicyApiService
        .createPolicyUsingPOST({
          createRequest: omit(createRequest, 'copyDataPolicy')
        })
        .subscribe(
          res => {
            observer.next(res);
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }

  onModify(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const policyId = this.data.policyId;
      const updateRequest = omit(this.formGroup.value, 'copyDataPolicy');
      assign(updateRequest, {
        scheduleStartTime: this.datePipe.transform(
          updateRequest.scheduleStartTime,
          'yyyy-MM-dd HH:mm:ss'
        )
      });

      this.liveMountPolicyApiService
        .updatePolicyUsingPUT({
          policyId,
          updateRequest
        })
        .subscribe(
          res => {
            observer.next(res);
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }
}
