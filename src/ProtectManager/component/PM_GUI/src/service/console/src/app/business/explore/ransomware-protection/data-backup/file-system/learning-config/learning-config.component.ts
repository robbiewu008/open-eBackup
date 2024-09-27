import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { BaseUtilService, DataMap, I18NService } from 'app/shared';
import { assign, every, isArray, isEmpty, map } from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-learning-config',
  templateUrl: './learning-config.component.html',
  styleUrls: ['./learning-config.component.less']
})
export class LearningConfigComponent implements OnInit {
  fileSystem;
  resourceData;
  resourceType;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  formGroup: FormGroup;
  dataMap = DataMap;
  isAllOceanProtect = true;
  isEn = this.i18n.isEn;

  valid$ = new Subject<boolean>();

  timesErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [15, 60])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateForm();
  }

  setRangeValid(key: string) {
    this.formGroup
      .get(key)
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.integer(),
        this.baseUtilService.VALID.rangeValue(15, 60)
      ]);
  }

  initForm() {
    this.formGroup = this.fb.group({
      isOpen: new FormControl(this.isAllOceanProtect),
      type: new FormControl(DataMap.selfLearningType.day.value),
      day: new FormControl(this.isAllOceanProtect ? '30' : '', {
        validators: this.isAllOceanProtect
          ? [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(15, 60)
            ]
          : []
      }),
      times: new FormControl('')
    });

    this.formGroup.get('isOpen').valueChanges.subscribe(res => {
      if (res) {
        if (this.formGroup.value.type === DataMap.selfLearningType.day.value) {
          this.setRangeValid('day');
        } else {
          this.setRangeValid('times');
        }
      } else {
        this.formGroup.get('day').clearValidators();
        this.formGroup.get('times').clearValidators();
      }
      this.formGroup.get('day').updateValueAndValidity();
      this.formGroup.get('times').updateValueAndValidity();
    });

    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.selfLearningType.day.value) {
        this.setRangeValid('day');
        this.formGroup.get('times').clearValidators();
      } else {
        this.setRangeValid('times');
        this.formGroup.get('day').clearValidators();
      }
      this.formGroup.get('day').updateValueAndValidity();
      this.formGroup.get('times').updateValueAndValidity();
    });

    this.formGroup.statusChanges.subscribe(() =>
      this.valid$.next(this.formGroup.valid)
    );
  }

  updateForm() {
    if (isEmpty(this.resourceData.protectedObject)) {
      return;
    }
    const isOpen = this.resourceData.protectedObject?.extParameters?.is_open;
    const type = this.resourceData.protectedObject?.extParameters?.type;
    const duration = this.resourceData.protectedObject?.extParameters?.duration;
    if (isOpen && this.isAllOceanProtect) {
      this.formGroup.patchValue({
        isOpen: isOpen,
        type: type,
        day: type === DataMap.selfLearningType.day.value ? duration : '',
        times: type === DataMap.selfLearningType.times.value ? duration : ''
      });
    } else {
      this.formGroup.patchValue({
        isOpen: false
      });
    }
  }

  initData(data: any, resourceType: string) {
    this.fileSystem = isArray(data) ? data : [data];
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
    this.isAllOceanProtect = every(
      this.fileSystem,
      item =>
        item.environment?.subType ===
        DataMap.cyberDeviceStorageType.OceanProtect.value
    );
    this.isCyberEngine =
      this.isCyberEngine && this.resourceData.isRealDetection !== true;
  }

  onOK() {
    const ext_parameters = {};
    if (this.isCyberEngine && this.isAllOceanProtect) {
      assign(ext_parameters, {
        is_open: this.formGroup.value.isOpen,
        type: this.formGroup.value.type,
        duration: !this.formGroup.value.isOpen
          ? 15
          : this.formGroup.value.type === DataMap.selfLearningType.day.value
          ? Number(this.formGroup.value.day)
          : Number(this.formGroup.value.times)
      });
    }
    return {
      ext_parameters
    };
  }
}
