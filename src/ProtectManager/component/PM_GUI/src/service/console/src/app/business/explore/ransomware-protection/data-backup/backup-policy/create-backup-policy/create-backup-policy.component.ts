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
import { Component, OnInit, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService } from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  I18NService,
  ProtectResourceAction,
  RetentionType,
  ScheduleTrigger,
  SlaApiService,
  TRIGGER_TYPE
} from 'app/shared';
import {
  assign,
  cloneDeep,
  each,
  first,
  includes,
  isEmpty,
  isUndefined,
  map,
  size,
  sortBy,
  split,
  toString,
  uniq
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { DetectUpperBoundComponent } from '../detect-upper-bound/detect-upper-bound.component';

@Component({
  selector: 'aui-create-backup-policy',
  templateUrl: './create-backup-policy.component.html',
  styleUrls: ['./create-backup-policy.component.less'],
  providers: [DatePipe]
})
export class CreateBackupPolicyComponent implements OnInit {
  rowData: any;
  isClone;
  isFromSelectSla;
  formGroup: FormGroup;
  action;
  scheduleTrigger = ScheduleTrigger;
  triggerType = TRIGGER_TYPE;
  protectResourceAction = ProtectResourceAction;
  daysOfMonthType = DataMap.Days_Of_Month_Type;
  dataMap = DataMap;
  monthDaysItems = [];
  activeIndex;

  defaultUpperBound = 6;
  defaultPoint = 2;

  detectionOptions = this.dataMapService
    .toArray('detectionMethod')
    .filter(item => {
      return (item.isLeaf = true);
    });
  intervalUnitOptions = this.dataMapService
    .toArray('Interval_Unit')
    .filter(item => {
      return [
        DataMap.Interval_Unit.minute.value,
        DataMap.Interval_Unit.hour.value,
        DataMap.Interval_Unit.day.value
      ].includes(item.value);
    })
    .filter(item => {
      return (item.isLeaf = true);
    });
  durationUnitOptions = this.dataMapService
    .toArray('Interval_Unit')
    .filter(item => {
      return [
        DataMap.Interval_Unit.day.value,
        DataMap.Interval_Unit.week.value,
        DataMap.Interval_Unit.month.value,
        DataMap.Interval_Unit.year.value,
        DataMap.Interval_Unit.persistent.value
      ].includes(item.value);
    })
    .filter(item => {
      return (item.isLeaf = true);
    });
  daysOfMonthOptions = this.dataMapService
    .toArray('Days_Of_Month_Type')
    .filter(item => (item.isLeaf = true));
  daysOfWeekOptions = this.dataMapService
    .toArray('Days_Of_Week')
    .filter(item => (item.isLeaf = true));

  intervalErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 23])
  };
  retentionErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
  };
  specifiedRetentionErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
  };
  windowStartTimeErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  windowEndTimeErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  daysOfYearErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  daysOfMonthErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidDupInput: this.i18n.get('common_duplicate_input_label')
  };
  daysOfWeekErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_required_label')
  };

  retryTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 5])
  });
  waitTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 30])
  });

  @ViewChild(DetectUpperBoundComponent, { static: false })
  DetectUpperBoundComponent: DetectUpperBoundComponent;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    public datePipe: DatePipe,
    public slaApiService: SlaApiService,
    private dataMapService: DataMapService,
    private messageService: MessageService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit(): void {
    this.updateMonthDays();
    this.initForm();
    this.updataForm();
  }

  initForm() {
    const backupTeam = cloneDeep(this.backupTeams);
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.name()]
      }),
      backupTeams: this.fb.array([backupTeam]),
      alarm_after_failure: new FormControl(true),
      auto_retry: new FormControl(true),
      auto_retry_times: new FormControl(3, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 5)
        ]
      }),
      auto_retry_wait_minutes: new FormControl(5, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 30)
        ]
      })
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
            ]
          })
        );
        this.formGroup.addControl(
          'auto_retry_wait_minutes',
          new FormControl(5, {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 30)
            ]
          })
        );
      }
    });
    this.listenFormGroup(backupTeam);
  }

  updataForm() {
    if (!this.rowData) {
      return;
    }
    each(this.rowData.policy_list, item => {
      assign(item, {
        name: this.i18n.get('common_policy_params_label', [
          item.name?.replace(/[^0-9]/g, '')
        ])
      });
    });
    this.rowData.policy_list = sortBy(this.rowData.policy_list, 'name');
    const firstPolicyList: any = first(this.rowData.policy_list) || {};
    this.formGroup.get('name').patchValue(this.rowData.name);
    this.formGroup
      .get('alarm_after_failure')
      .patchValue(firstPolicyList.ext_parameters?.alarm_after_failure);
    this.formGroup
      .get('auto_retry')
      .patchValue(firstPolicyList.ext_parameters?.auto_retry);
    if (this.formGroup.get('auto_retry').value) {
      this.formGroup
        .get('auto_retry_times')
        .patchValue(firstPolicyList.ext_parameters?.auto_retry_times || 3);
      this.formGroup
        .get('auto_retry_wait_minutes')
        .patchValue(
          firstPolicyList.ext_parameters?.auto_retry_wait_minutes || 5
        );
    }
    this.getBackupTeams().clear();
    each(this.rowData.policy_list, (backup: any) => {
      const params = {
        name: backup.name,
        need_detect: backup.ext_parameters?.need_detect,
        is_backup_detect_enable: backup.ext_parameters?.is_backup_detect_enable,
        upper_bound:
          backup.ext_parameters?.upper_bound === 5
            ? 3
            : backup.ext_parameters?.upper_bound === 7
            ? 1
            : 2,
        is_security_snap: backup.ext_parameters?.is_security_snap,
        start_time:
          backup.schedule.trigger === ScheduleTrigger.PERIOD_EXECUTE &&
          backup.schedule.start_time
            ? new Date(backup.schedule.start_time)
            : '',
        interval: backup.schedule.interval,
        interval_unit:
          backup.schedule.interval_unit || DataMap.Interval_Unit.hour.value,
        retention_duration: backup.retention.retention_duration || '',
        duration_unit:
          backup.retention.retention_type === RetentionType.PERMANENTLY_RETAINED
            ? DataMap.Interval_Unit.persistent.value
            : backup.retention.duration_unit || DataMap.Interval_Unit.day.value,
        window_start: backup.schedule.window_start
          ? new Date(
              `${new Date().getFullYear()}` +
                `/` +
                `${new Date().getMonth() + 1}` +
                `/` +
                `${new Date().getDate()}` +
                ` ` +
                `${backup.schedule.window_start}`
            )
          : '',
        window_end: backup.schedule.window_end
          ? new Date(
              `${new Date().getFullYear()}` +
                `/` +
                `${new Date().getMonth() + 1}` +
                `/` +
                `${new Date().getDate()}` +
                ` ` +
                `${backup.schedule.window_end}`
            )
          : '',
        trigger_action:
          backup.schedule.trigger_action ||
          (backup.schedule?.interval_unit === DataMap.Interval_Unit.day.value
            ? this.triggerType.day
            : this.triggerType.hour),
        days_of_year:
          backup.schedule.trigger_action === this.triggerType.year &&
          backup.schedule.days_of_year
            ? new Date(backup.schedule.days_of_year)
            : '',
        days_of_month_type:
          backup.schedule.days_of_month ===
            DataMap.Days_Of_Month_Type.lastDay.value &&
          backup.schedule.trigger_action === this.triggerType.month
            ? DataMap.Days_Of_Month_Type.lastDay.value
            : DataMap.Days_Of_Month_Type.specifiedDate.value,
        days_of_month:
          backup.schedule.days_of_month ===
            DataMap.Days_Of_Month_Type.lastDay.value &&
          backup.schedule.trigger_action === this.triggerType.month
            ? ''
            : backup.schedule.days_of_month,
        days_of_months:
          backup.schedule.days_of_month !==
            DataMap.Days_Of_Month_Type.lastDay.value &&
          backup.schedule.trigger_action === this.triggerType.month
            ? map(backup.schedule.days_of_month.split(','), v => +v)
            : [],
        days_of_week:
          backup.schedule.trigger_action === this.triggerType.week &&
          backup.schedule.days_of_week
            ? backup.schedule.days_of_week || []
            : []
      };
      const backupTeam = cloneDeep(this.backupTeams);
      this.listenFormGroup(backupTeam);
      backupTeam.patchValue(params);
      if (
        backup.retention.retention_type === RetentionType.PERMANENTLY_RETAINED
      ) {
        backupTeam.get('retention_duration').disable();
      }
      this.listenFormGroup(backupTeam);
      this.getBackupTeams().push(backupTeam);
    });
  }

  get backupTeams(): FormGroup {
    const backupTeam: FormGroup = this.fb.group({
      name: new FormControl(''),
      need_detect: new FormControl(DataMap.detectionMethod.auto.value),
      is_backup_detect_enable: new FormControl(false),
      upper_bound: new FormControl(this.defaultPoint),
      is_security_snap: new FormControl(false),
      trigger_action: new FormControl(TRIGGER_TYPE.hour),
      start_time: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      interval: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 23)
        ]
      }),
      retention_duration: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 365)
        ]
      }),
      duration_unit: new FormControl(DataMap.Interval_Unit.day.value),
      days_of_year: new FormControl(''),
      days_of_month_type: new FormControl(
        DataMap.Days_Of_Month_Type.specifiedDate.value
      ),
      days_of_months: new FormControl([]),
      days_of_month: new FormControl(''),
      days_of_week: new FormControl([]),
      window_start: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      window_end: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
    return backupTeam;
  }

  getBackupTeams() {
    return this.formGroup.get('backupTeams') as FormArray;
  }

  addBackupTeam() {
    const backupTeam = cloneDeep(this.backupTeams);
    this.getBackupTeams().push(backupTeam);
    this.listenFormGroup(backupTeam);
  }

  removeBackupTeam(tab) {
    const controls = this.getBackupTeams().controls;
    if (controls.length === 1) {
      this.messageService.error(
        this.i18n.get('common_at_least_retain_label', [
          this.i18n.get('common_policy_label')
        ]),
        {
          lvMessageKey: 'msg_limit_key',
          lvShowCloseButton: true
        }
      );
      return;
    }
    this.getBackupTeams().removeAt(tab.lvId);
    if (this.activeIndex >= this.getBackupTeams().length) {
      this.activeIndex = 0;
    }
  }

  listenFormGroup(backupTeam: FormGroup) {
    backupTeam.get('trigger_action').valueChanges.subscribe(res => {
      if (res === TRIGGER_TYPE.year) {
        backupTeam
          .get('days_of_year')
          .setValidators([this.baseUtilService.VALID.required()]);
        backupTeam.get('days_of_month').clearValidators();
        backupTeam.get('days_of_months').clearValidators();
        backupTeam.get('days_of_week').clearValidators();
        backupTeam.get('start_time').clearValidators();
        backupTeam.get('interval').clearValidators();
      } else if (res === TRIGGER_TYPE.month) {
        if (
          backupTeam.get('days_of_month_type').value ===
          this.daysOfMonthType.specifiedDate.value
        ) {
          backupTeam
            .get('days_of_month')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          backupTeam.get('days_of_month').clearValidators();
          backupTeam.get('days_of_months').clearValidators();
        }
        backupTeam.get('days_of_year').clearValidators();
        backupTeam.get('days_of_week').clearValidators();
        backupTeam.get('start_time').clearValidators();
        backupTeam.get('interval').clearValidators();
      } else if (res === TRIGGER_TYPE.week) {
        backupTeam
          .get('days_of_week')
          .setValidators([this.baseUtilService.VALID.required()]);
        backupTeam.get('days_of_year').clearValidators();
        backupTeam.get('days_of_month').clearValidators();
        backupTeam.get('days_of_months').clearValidators();
        backupTeam.get('start_time').clearValidators();
        backupTeam.get('interval').clearValidators();
      } else if (res === TRIGGER_TYPE.day) {
        backupTeam
          .get('start_time')
          .setValidators([this.baseUtilService.VALID.required()]);
        backupTeam
          .get('interval')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 365)
          ]);
        this.intervalErrorTip = {
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
        };
        backupTeam.get('days_of_year').clearValidators();
        backupTeam.get('days_of_month').clearValidators();
        backupTeam.get('days_of_months').clearValidators();
        backupTeam.get('days_of_week').clearValidators();
      } else {
        backupTeam
          .get('start_time')
          .setValidators([this.baseUtilService.VALID.required()]);
        backupTeam
          .get('interval')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 23)
          ]);
        this.intervalErrorTip = {
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 23])
        };
        backupTeam.get('days_of_year').clearValidators();
        backupTeam.get('days_of_month').clearValidators();
        backupTeam.get('days_of_months').clearValidators();
        backupTeam.get('days_of_week').clearValidators();
      }
      backupTeam.get('days_of_year').updateValueAndValidity();
      backupTeam.get('days_of_month').updateValueAndValidity();
      backupTeam.get('days_of_months').updateValueAndValidity();
      backupTeam.get('days_of_week').updateValueAndValidity();
      backupTeam.get('start_time').updateValueAndValidity();
      backupTeam.get('interval').updateValueAndValidity();
    });
    backupTeam.get('days_of_month_type').valueChanges.subscribe(res => {
      this.dealDaysOfMonthType(backupTeam, res);
    });
    this.checkDaysOfMonth(backupTeam);
    // 快照保留
    backupTeam.get('duration_unit').valueChanges.subscribe(res => {
      if (res === DataMap.Interval_Unit.day.value) {
        backupTeam.get('retention_duration').enable();
        backupTeam
          .get('retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 365)
          ]);
        this.retentionErrorTip = {
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
        };
      } else if (res === DataMap.Interval_Unit.week.value) {
        backupTeam.get('retention_duration').enable();
        backupTeam
          .get('retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 54)
          ]);
        this.retentionErrorTip = {
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 54])
        };
      } else if (res === DataMap.Interval_Unit.month.value) {
        backupTeam.get('retention_duration').enable();
        backupTeam
          .get('retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 24)
          ]);
        this.retentionErrorTip = {
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 24])
        };
      } else if (res === DataMap.Interval_Unit.year.value) {
        backupTeam.get('retention_duration').enable();
        backupTeam
          .get('retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 10)
          ]);
        this.retentionErrorTip = {
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
        };
      } else {
        backupTeam.get('retention_duration').clearValidators();
        backupTeam.get('retention_duration').disable();
      }
      backupTeam.get('retention_duration').updateValueAndValidity();
    });
  }

  dealDaysOfMonthType(backupTeam: FormGroup, daysOfMonthType) {
    if (
      daysOfMonthType === this.daysOfMonthType.specifiedDate.value &&
      backupTeam.get('trigger_action').value === TRIGGER_TYPE.month
    ) {
      this.checkDaysOfMonth(backupTeam);
      backupTeam
        .get('days_of_month')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.validDaysOfMonth()
        ]);
      backupTeam
        .get('days_of_months')
        .setValidators([this.baseUtilService.VALID.required()]);
    } else {
      backupTeam.get('days_of_month').clearValidators();
      backupTeam.get('days_of_months').clearValidators();
    }
    backupTeam.get('days_of_month').updateValueAndValidity();
    backupTeam.get('days_of_months').updateValueAndValidity();
  }

  checkDaysOfMonth(backupTeam: FormGroup) {
    backupTeam.get('days_of_months').valueChanges.subscribe(res => {
      backupTeam
        .get('days_of_month')
        .setValue(toString((res || []).sort((a, b) => a - b)));
    });
  }

  validDaysOfMonth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }

      if (
        size(uniq(split(control.value, ','))) !==
        size(split(control.value, ','))
      ) {
        return { invalidDupInput: { value: control.value } };
      }

      if (
        !isUndefined(
          split(control.value, ',').find(item => !/^[1-9]\d*$/.test(item))
        ) ||
        !isUndefined(
          split(control.value, ',').find(
            item => !(+item > 0 && +item < 32) || isNaN(+item)
          )
        )
      ) {
        return { invalidInteger: { value: control.value } };
      }

      return null;
    };
  }

  updateMonthDays() {
    const items1 = [];
    const items2 = [];
    const items3 = [];
    const items4 = [];
    const items5 = [];
    for (let index = 1; index < 32; index++) {
      if (index <= 7) {
        items1.push(index);
      } else if (index > 7 && index <= 14) {
        items2.push(index);
      } else if (index > 14 && index <= 21) {
        items3.push(index);
      } else if (index > 21 && index <= 28) {
        items4.push(index);
      } else if (index > 28 && index <= 31) {
        items5.push(index);
      }
    }
    this.monthDaysItems = [
      { key: items1 },
      { key: items2 },
      { key: items3 },
      { key: items4 },
      { key: items5 }
    ];
  }

  getParams() {
    const params = {
      name: this.formGroup.value.name,
      application: DataMap.Resource_Type.LocalFileSystem.value,
      type: 1
    };
    const policy_list = [];
    each(this.formGroup.value.backupTeams, (backup, index) => {
      const ext_parameters = {
        alarm_after_failure: this.formGroup.value.alarm_after_failure,
        auto_retry: this.formGroup.value.auto_retry
      };
      if (this.formGroup.value.auto_retry) {
        assign(ext_parameters, {
          auto_retry_times: +this.formGroup.value.auto_retry_times,
          auto_retry_wait_minutes: +this.formGroup.value.auto_retry_wait_minutes
        });
      }
      assign(ext_parameters, {
        need_detect: backup.need_detect,
        is_security_snap:
          backup.need_detect === DataMap.detectionMethod.auto.value
            ? backup.is_security_snap
            : false,
        is_backup_detect_enable:
          backup.need_detect === DataMap.detectionMethod.auto.value
            ? backup.is_backup_detect_enable
            : false,
        upper_bound:
          backup.need_detect === DataMap.detectionMethod.auto.value &&
          backup.is_backup_detect_enable
            ? this.DetectUpperBoundComponent?.getUpperBound(
                backup.upper_bound
              ) || this.defaultUpperBound
            : this.defaultUpperBound
      });
      const schedule: any = {
        window_start: this.datePipe.transform(backup?.window_start, 'HH:mm:ss'),
        window_end: this.datePipe.transform(backup?.window_end, 'HH:mm:ss'),
        trigger: includes(
          [this.triggerType.day, this.triggerType.hour],
          backup?.trigger_action
        )
          ? ScheduleTrigger.PERIOD_EXECUTE
          : ScheduleTrigger.SPECIFIED_TIME
      };
      schedule.trigger_action = backup?.trigger_action;
      if (backup?.trigger_action === this.triggerType.year) {
        schedule.days_of_year = this.datePipe.transform(
          backup?.days_of_year,
          'yyyy-MM-dd'
        );
      } else if (backup?.trigger_action === this.triggerType.month) {
        if (
          backup?.days_of_month_type ===
          DataMap.Days_Of_Month_Type.specifiedDate.value
        ) {
          schedule.days_of_month = backup?.days_of_month;
        } else {
          schedule.days_of_month = backup?.days_of_month_type;
        }
      } else if (backup?.trigger_action === this.triggerType.week) {
        schedule.days_of_week = backup?.days_of_week;
      } else {
        schedule.start_time = this.datePipe.transform(
          backup.start_time,
          'yyyy-MM-dd HH:mm:ss'
        );
        schedule.interval = +backup?.interval;
        schedule.interval_unit =
          backup?.trigger_action === this.triggerType.day
            ? DataMap.Interval_Unit.day.value
            : DataMap.Interval_Unit.hour.value;
        delete schedule.trigger_action;
      }
      const retention = {
        duration_unit: backup?.duration_unit,
        retention_duration: backup?.retention_duration,
        retention_type:
          backup?.duration_unit === DataMap.Interval_Unit.persistent.value
            ? RetentionType.PERMANENTLY_RETAINED
            : RetentionType.TEMPORARY_RESERVATION
      };
      if (backup?.duration_unit === DataMap.Interval_Unit.persistent.value) {
        delete retention.duration_unit;
        delete retention.retention_duration;
      }
      const policy = {
        name:
          backup.name ||
          this.i18n.get('common_policy_params_label', [`0${index + 1}`]),
        type: 'backup',
        action: 'snapshot',
        uuid: '',
        schedule,
        retention,
        ext_parameters
      };
      policy_list.push(policy);
    });
    assign(params, {
      policy_list
    });
    return params;
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const parmas: any = this.getParams();
      if (this.rowData && !this.isClone) {
        assign(parmas, {
          uuid: this.rowData.uuid
        });
        this.slaApiService.modifySLACyberUsingPUT({ slaDto: parmas }).subscribe(
          res => {
            observer.next(res);
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
      } else {
        this.slaApiService
          .createSLACyberUsingPOST({
            slaDto: parmas,
            akOperationTipsContent:
              !this.isClone && !this.isFromSelectSla
                ? this.i18n.get('protection_detection_policy_instruction_label')
                : ''
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
      }
    });
  }
}
