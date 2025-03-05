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
  FormGroup
} from '@angular/forms';
import { ModalRef, OptionItem } from '@iux/live';
import { BaseUtilService, DataMap } from 'app/shared';
import { CopiesService } from 'app/shared/api/services/copies.service';
import { DataMapService, I18NService } from 'app/shared/services';
import { assign, trim, findIndex, toNumber, includes } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-worm-set',
  templateUrl: './worm-set.html',
  styleUrls: ['./worm-set.less']
})
export class WormSetComponent implements OnInit {
  data;
  formGroup: FormGroup;
  retentionDurationErrorTip = assign(
    {},
    this.baseUtilService.rangeErrorTip,
    {
      invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
    },
    {
      invalidTime: this.i18n.get('common_worm_retention_label')
    }
  );
  unitTranslateMap = {
    [DataMap.Interval_Unit.day.value]: 1,
    [DataMap.Interval_Unit.week.value]: 7,
    [DataMap.Interval_Unit.month.value]: 30,
    [DataMap.Interval_Unit.year.value]: 365,
    [DataMap.Interval_Unit.persistent.value]: Infinity
  };
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
  dataMap = DataMap;
  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    private modal: ModalRef,
    public dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private copiesApiService: CopiesService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      worm_validity_type: new FormControl(1),
      retention_duration: new FormControl(''),
      duration_unit: new FormControl(''),
      convert_worm_switch: new FormControl(false, [
        this.baseUtilService.VALID.required()
      ])
    });
    this.formGroup.patchValue({
      worm_validity_type: this.data.worm_validity_type || 1,
      retention_duration: this.data.worm_retention_duration || '',
      duration_unit:
        this.data.worm_duration_unit || DataMap.Interval_Unit.day.value,
      convert_worm_switch:
        this.data.worm_status === 2 || this.data.worm_status === 3
    });

    setTimeout(() => {
      if (this.formGroup.value.worm_validity_type === 2) {
        this.changeTimeUnits(this.formGroup.value.duration_unit);
      }
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    }, 0);

    this.formGroup.get('worm_validity_type').valueChanges.subscribe(res => {
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
    this.formGroup.get('convert_worm_switch').valueChanges.subscribe(res => {
      // 开启时，如果是SQLServer或HBASE的日志副本，拨到自定义
      if (res) {
        this.formGroup.get('worm_validity_type').setValue(1);
        this.formGroup.get('duration_unit').clearValidators();
        this.formGroup.get('retention_duration').clearValidators();
        this.formGroup.get('retention_duration').updateValueAndValidity();
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
          this.baseUtilService.VALID.rangeValue(1, 365),
          this.validproTime()
        ]);
      const errorTip = assign(
        {},
        this.baseUtilService.rangeErrorTip,
        {
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
        },
        { invalidTime: this.i18n.get('common_worm_retention_label') }
      );
      this.retentionDurationErrorTip = errorTip;
    } else if (value === DataMap.Interval_Unit.week.value) {
      this.formGroup
        .get('retention_duration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 54),
          this.validproTime()
        ]);
      const errorTip = assign(
        {},
        this.baseUtilService.rangeErrorTip,
        {
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 54])
        },
        { invalidTime: this.i18n.get('common_worm_retention_label') }
      );
      this.retentionDurationErrorTip = errorTip;
    } else if (value === DataMap.Interval_Unit.month.value) {
      this.formGroup
        .get('retention_duration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 24),
          this.validproTime()
        ]);
      const errorTip = assign(
        {},
        this.baseUtilService.rangeErrorTip,
        {
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 24])
        },
        { invalidTime: this.i18n.get('common_worm_retention_label') }
      );
      this.retentionDurationErrorTip = errorTip;
    } else if (value === DataMap.Interval_Unit.year.value) {
      this.formGroup
        .get('retention_duration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 20),
          this.validproTime()
        ]);
      const errorTip = assign(
        {},
        this.baseUtilService.rangeErrorTip,
        {
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 20])
        },
        {
          invalidTime: this.i18n.get('common_worm_retention_label')
        }
      );
      this.retentionDurationErrorTip = errorTip;
    }
    this.formGroup.get('retention_duration').markAsTouched();
    this.formGroup.get('retention_duration').updateValueAndValidity();
  }
  // worm时间小于等于副本保留时间
  validproTime() {
    let rowWormDate =
      this.unitTranslateMap[this.data.worm_duration_unit] *
      this.data.worm_retention_duration;
    if (this.data.worm_validity_type === 1 && this.data.retention_type === 1) {
      rowWormDate = Infinity;
    }
    return (control: AbstractControl): { [key: string]: { value: any } } => {
      const wormSetUnit = this.formGroup.get('duration_unit').value;
      const copyDurationUnit = this.data.duration_unit;
      const wormDate = this.unitTranslateMap[wormSetUnit] * control.value;
      const copyDate =
        this.unitTranslateMap[copyDurationUnit] *
        toNumber(this.data.retention_duration);
      if (wormDate > copyDate) {
        return { invalidTime: { value: control.value } };
      }
      if (wormDate < rowWormDate) {
        return {
          [this.i18n.get('common_worm_shorten_label')]: {
            value: control.value
          }
        };
      }
      return null;
    };
  }
  getParams() {
    const body = {
      ...this.formGroup.value,
      resource_id: this.data.uuid
    };
    if (body.worm_validity_type === 1) {
      delete body.duration_unit;
      delete body.retention_duration;
    }
    return body;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.copiesApiService
        .updateWormSettingV1CopiesCopyIdActionUpdateWormSettingPost({
          copyId: this.data.uuid,
          wormSetting: this.getParams()
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
