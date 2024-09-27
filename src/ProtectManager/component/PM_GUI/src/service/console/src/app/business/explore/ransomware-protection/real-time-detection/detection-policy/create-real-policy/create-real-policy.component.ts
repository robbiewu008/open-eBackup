import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  I18NService,
  IODETECTPOLICYService,
  SnapshotRetenTion
} from 'app/shared';
import { assign } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-real-policy',
  templateUrl: './create-real-policy.component.html',
  styleUrls: ['./create-real-policy.component.less']
})
export class CreateRealPolicyComponent implements OnInit {
  rowData;
  isClone;

  formGroup: FormGroup;
  snapshotRetenTion = SnapshotRetenTion;
  enableUpdate = 'update';
  disableUpdate = 'none';
  isOpen = false;

  retentionDay = 14;
  frequencyDay = 30;

  retentionErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [
      1,
      this.retentionDay
    ])
  };
  frequencyErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [
      1,
      this.frequencyDay
    ])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private ioDetectPolicyService: IODETECTPOLICYService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.updateForm();
  }

  openTips() {
    this.isOpen = !this.isOpen;
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.name()]
      }),
      retention_duration: new FormControl('2', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.retentionDay)
        ]
      }),
      alarm_analysis: new FormControl(true),
      decoy_detection: new FormControl(false),
      update_type: new FormControl(this.enableUpdate),
      frequency: new FormControl('')
    });

    this.listenForm();
  }

  listenForm() {
    this.formGroup.get('decoy_detection').valueChanges.subscribe(res => {
      if (res) {
        if (this.formGroup.value.update_type === this.enableUpdate) {
          this.formGroup
            .get('frequency')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, this.frequencyDay)
            ]);
        } else {
          this.formGroup.get('frequency').clearValidators();
        }
      } else {
        this.formGroup.get('frequency').clearValidators();
      }
      this.formGroup.get('frequency').updateValueAndValidity();
    });

    this.formGroup.get('update_type').valueChanges.subscribe(res => {
      if (res === this.enableUpdate) {
        this.formGroup
          .get('frequency')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.frequencyDay)
          ]);
      } else {
        this.formGroup.get('frequency').clearValidators();
      }
      this.formGroup.get('frequency').updateValueAndValidity();
    });
  }

  updateForm() {
    if (!this.rowData) {
      return;
    }
    const parmas = {
      name: this.rowData.name,
      retention_duration: this.rowData.retentionDuration,
      alarm_analysis: this.rowData.isIoEnhancedEnabled,
      decoy_detection: this.rowData.isHoneypotDetectEnable,
      update_type:
        this.rowData.period === 0 ? this.disableUpdate : this.enableUpdate,
      frequency: !this.rowData.period ? '' : this.rowData.period
    };
    this.formGroup.patchValue(parmas);
  }

  getParams() {
    const params: any = {
      name: this.formGroup.value.name,
      durationUnit: DataMap.Interval_Unit.day.value,
      isIoEnhancedEnabled: this.formGroup.value.alarm_analysis,
      isHoneypotDetectEnable: this.formGroup.value.decoy_detection,
      period:
        this.formGroup.value.decoy_detection &&
        this.formGroup.value.update_type === this.enableUpdate
          ? Number(this.formGroup.value.frequency)
          : 0,
      retentionDuration: Number(this.formGroup.value.retention_duration)
    };
    return params;
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const parmas = this.getParams();
      if (this.rowData && !this.isClone) {
        this.ioDetectPolicyService
          .modifyIoDetectPolicy({
            modifyIoDetectPolicyReq: assign(parmas, { id: this.rowData.id })
          })
          .subscribe(
            res => {
              observer.next(res);
              observer.complete;
            },
            err => {
              observer.error(err);
              observer.complete;
            }
          );
      } else {
        this.ioDetectPolicyService
          .createIoDetectPolicy({ createIoDetectPolicyReq: parmas })
          .subscribe(
            res => {
              observer.next(res);
              observer.complete;
            },
            err => {
              observer.error(err);
              observer.complete;
            }
          );
      }
    });
  }
}
