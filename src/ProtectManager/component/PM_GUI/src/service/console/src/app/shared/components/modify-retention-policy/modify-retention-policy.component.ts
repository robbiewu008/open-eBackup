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
import { assign, toNumber } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-modify-retention-policy',
  templateUrl: './modify-retention-policy.component.html',
  styleUrls: ['./modify-retention-policy.component.less']
})
export class ModifyRetentionPolicyComponent implements OnInit {
  data;
  formGroup: FormGroup;
  retentionDurationErrorTip = assign(
    {},
    this.baseUtilService.rangeErrorTip,
    {
      invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
    },
    { invalidTime: this.i18n.get('common_retention_worm_label') }
  );
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
  helpTipsLabel = this.i18n.get('common_retention_worm_label');
  retainedTipLabel = '';
  // 保留70年
  repOrArchiveUnitMap = {
    d: 25550,
    w: 3650,
    MO: 840,
    y: 70
  };
  backupUnitMap = {
    d: 365,
    w: 54,
    MO: 24,
    y: 10
  };
  isArchiveOrRep = false;

  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    private modal: ModalRef,
    public dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private copiesApiService: CopiesService
  ) {}

  ngOnInit() {
    this.isArchiveOrRep = [
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.reverseReplication.value,
      DataMap.CopyData_generatedType.cascadedReplication.value
    ].includes(this.data.generated_by);
    this.initForm();
    if (this.data.worm_retention_duration) {
      this.helpTipsLabel =
        this.helpTipsLabel + '(' + `${this.data.worm_retention_duration}`;
      if (this.i18n.isEn) {
        this.helpTipsLabel = this.helpTipsLabel + '(' + ' ';
      }
      if (this.data.worm_duration_unit) {
        this.helpTipsLabel =
          this.helpTipsLabel +
          this.dataMapService.getLabel(
            'Interval_Unit',
            this.data.worm_duration_unit
          ) +
          ')';
      }
    }
    if (this.data?.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      this.retainedTipLabel = this.i18n.get(
        'protection_log_retention_policy_tip_label'
      );
    }
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
    const tmpMap = this.isArchiveOrRep
      ? this.repOrArchiveUnitMap
      : this.backupUnitMap;
    const genValidators = [
      this.baseUtilService.VALID.required(),
      this.baseUtilService.VALID.integer(),
      this.baseUtilService.VALID.rangeValue(1, tmpMap[value]),
      this.validproTime()
    ];
    const genErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
      invalidRang: this.i18n.get('common_valid_rang_label', [1, tmpMap[value]]),
      invalidTime: this.i18n.get('common_retention_worm_label')
    });
    this.formGroup.get('retention_duration').setValidators(genValidators);
    this.retentionDurationErrorTip = genErrorTip;
    this.formGroup.get('retention_duration').markAsTouched();
    this.formGroup.get('retention_duration').updateValueAndValidity();
  }

  // 修改保留策略时校验是否大于worm日期
  validproTime() {
    return (control: AbstractControl): { invalidTime: { value: any } } => {
      const copyDurationUnit = this.formGroup.get('duration_unit').value;
      const wormSetUnit = this.data.worm_duration_unit;
      const copyDate =
        DataMap.unitTranslateMap[copyDurationUnit] * control.value;
      const wormDate =
        DataMap.unitTranslateMap[wormSetUnit] *
        toNumber(this.data.worm_retention_duration);
      if (copyDate < wormDate) {
        return { invalidTime: { value: control.value } };
      }
      return null;
    };
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
