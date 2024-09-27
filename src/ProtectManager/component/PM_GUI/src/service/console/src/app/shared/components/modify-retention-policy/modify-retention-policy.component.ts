import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef, OptionItem } from '@iux/live';
import { BaseUtilService, DataMap } from 'app/shared';
import { CopiesService } from 'app/shared/api/services/copies.service';
import { DataMapService, I18NService } from 'app/shared/services';
import { assign } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-modify-retention-policy',
  templateUrl: './modify-retention-policy.component.html',
  styleUrls: ['./modify-retention-policy.component.less']
})
export class ModifyRetentionPolicyComponent implements OnInit {
  data;
  formGroup: FormGroup;
  retentionDurationErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
  });
  retentionDurations = this.dataMapService
    .toArray('Interval_Unit')
    .filter((v: OptionItem) => {
      return v.value !== 'm' && v.value !== 'h' && v.value !== 'p';
    })
    .filter((v: OptionItem) => {
      return (v.isLeaf = true);
    });
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  constructor(
    public fb: FormBuilder,
    private i18n: I18NService,
    private modal: ModalRef,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private copiesApiService: CopiesService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      retention_type: new FormControl(''),
      retention_duration: new FormControl(''),
      duration_unit: new FormControl('')
    });

    this.formGroup.patchValue({
      retention_type: this.data.retention_type,
      retention_duration: this.data.retention_duration || '',
      duration_unit: this.data.duration_unit || DataMap.Interval_Unit.day.value
    });

    setTimeout(() => {
      if (this.formGroup.value.retention_type === 2) {
        this.changeTimeUnits(this.formGroup.value.duration_unit);
      }
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    }, 0);

    this.formGroup.get('retention_type').valueChanges.subscribe(res => {
      if (res === 2) {
        this.changeTimeUnits(this.formGroup.value.duration_unit);
        this.formGroup
          .get('duration_unit')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('duration_unit').updateValueAndValidity();
      } else {
        this.formGroup.get('duration_unit').clearValidators();
        this.formGroup.get('retention_duration').clearValidators();
        this.formGroup.get('retention_duration').updateValueAndValidity();
      }
    });
  }

  changeTimeUnits(value) {
    if (value === DataMap.Interval_Unit.day.value) {
      this.formGroup
        .get('retention_duration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 365)
        ]);
      const errorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
      });
      this.retentionDurationErrorTip = errorTip;
    } else if (value === DataMap.Interval_Unit.week.value) {
      this.formGroup
        .get('retention_duration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 54)
        ]);
      const errorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 54])
      });
      this.retentionDurationErrorTip = errorTip;
    } else if (value === DataMap.Interval_Unit.month.value) {
      this.formGroup
        .get('retention_duration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 24)
        ]);
      const errorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 24])
      });
      this.retentionDurationErrorTip = errorTip;
    } else if (value === DataMap.Interval_Unit.year.value) {
      this.formGroup
        .get('retention_duration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 10)
        ]);
      const errorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
      });
      this.retentionDurationErrorTip = errorTip;
    }
    this.formGroup.get('retention_duration').markAsTouched();
    this.formGroup.get('retention_duration').updateValueAndValidity();
  }

  getParams() {
    const body = {
      ...this.formGroup.value,
      resource_id: this.data.uuid
    };
    if (body.retention_type === 1) {
      delete body.duration_unit;
      delete body.retention_duration;
    }
    return body;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.isCyberEngine) {
        this.copiesApiService
          .updateCopyRetentionV1CopiesCopyIdActionUpdateRetentionCyberPost({
            copyId: this.data.uuid,
            body: this.getParams()
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
      } else {
        this.copiesApiService
          .updateCopyRetentionV1CopiesCopyIdActionUpdateRetentionPost({
            copyId: this.data.uuid,
            body: this.getParams()
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
      }
    });
  }
}
