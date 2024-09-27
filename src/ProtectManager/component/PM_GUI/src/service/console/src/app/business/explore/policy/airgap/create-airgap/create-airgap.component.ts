import { DatePipe } from '@angular/common';
import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  AntiRansomwareAirgapApiService,
  ApiService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService
} from 'app/shared';
import { assign, each, isEmpty, map, split } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-airgap',
  templateUrl: './create-airgap.component.html',
  styleUrls: ['./create-airgap.component.less'],
  providers: [DatePipe]
})
export class CreateAirgapComponent implements OnInit {
  data;
  formGroup: FormGroup;
  dataMap = DataMap;
  daysOfWeekOptions = this.dataMapService
    .toArray('Days_Of_Week')
    .filter(item => (item.isLeaf = true));

  timeOfDayOptions = this.dataMapService
    .toArray('timeOfDay')
    .filter(item => (item.isLeaf = true));
  timeOfADayOptions = this.dataMapService
    .toArray('timeOfADay')
    .filter(item => (item.isLeaf = true));
  protOptions = [];

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidNameBegin: this.i18n.get('common_valid_name_begin_label'),
    invalidNameCombination: this.i18n.get(
      'common_valid_name_combination_label'
    ),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private apiService: ApiService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    public datePipe: DatePipe,
    private antiRansomwareAirgapApiService: AntiRansomwareAirgapApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.updateData();
  }
  initForm() {
    this.formGroup = this.fb.group({
      periods: this.fb.array([this.getTimePeriodsFormGroup()]),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64),
          this.validCopyName()
        ]
      }),
      day: new FormControl({
        value: this.i18n.get('common_everyday_label'),
        disabled: true
      }),
      desc: new FormControl(''),
      triggerCycle: new FormControl(DataMap.timeType.day.value),
      frequency: new FormControl()
    });
    this.watch();
  }

  get periods() {
    return (this.formGroup.get('periods') as FormArray).controls;
  }

  validCopyName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }

      const value = control.value;
      const reg1 = CommonConsts.REGEX.nameBegin;
      if (!reg1.test(value)) {
        return { invalidNameBegin: { value: control.value } };
      }

      const reg2 = CommonConsts.REGEX.nameCombination;
      if (!reg2.test(value)) {
        return { invalidNameCombination: { value: control.value } };
      }

      return null;
    };
  }

  watch() {
    this.formGroup.get('triggerCycle').valueChanges.subscribe(res => {
      if (res === DataMap.timeType.day.value) {
        this.formGroup.get('frequency').clearValidators();
      } else {
        this.formGroup
          .get('frequency')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('frequency').updateValueAndValidity();
    });
  }
  getTimePeriodsFormGroup() {
    return this.fb.group({
      startTime: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      endTime: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
  }

  addTime() {
    (this.formGroup.get('periods') as FormArray).push(
      this.getTimePeriodsFormGroup()
    );
  }
  removeRow(i) {
    (this.formGroup.get('periods') as FormArray).removeAt(i);
  }

  updateData() {
    if (!this.data) {
      return;
    }

    let params = {
      periods: []
    };

    if (this.data[0].triggerWeekFreq) {
      assign(params, {
        frequency: this.data[0].triggerWeekFreq.split(',')
      });
    }
    assign(params, {
      name: this.data[0].name,
      desc: this.data[0].description,
      triggerCycle: this.data[0].triggerCycle
    });
    for (let i = 0; i < this.data[0].airGapPolicyWindows.length - 1; i++) {
      this.addTime();
    }
    each(this.data[0].airGapPolicyWindows, item => {
      params.periods.push({
        endTime: item.endTime,
        startTime: item.startTime
      });
    });
    this.formGroup.patchValue(params);
  }

  getParams() {
    const params = {
      name: this.formGroup.value.name,
      description: this.formGroup.value.desc,
      airGapPolicyWindowInfos: this.formGroup.value.periods
    };
    if (this.formGroup.value.triggerCycle === DataMap.timeType.day.value) {
      assign(params, {
        triggerCycle: this.dataMap.timeType.day.value,
        triggerWeekFreq: ''
      });
    } else {
      assign(params, {
        triggerWeekFreq: this.formGroup.value.frequency.toString(),
        triggerCycle: this.dataMap.timeType.week.value
      });
    }
    return params;
  }

  onOk(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      let params = this.getParams();
      if (this.data) {
        this.antiRansomwareAirgapApiService
          .UpdateAirGapPolicy({
            UpdateAirGapPolicyRequestBody: params as any,
            policyId: this.data[0].id
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
      } else {
        this.antiRansomwareAirgapApiService
          .CreateAirGapPolicy({
            CreateAirGapPolicyRequestBody: params as any
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
}
