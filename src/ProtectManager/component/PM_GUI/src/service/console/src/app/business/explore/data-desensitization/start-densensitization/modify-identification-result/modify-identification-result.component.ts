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
import {
  AnonyControllerService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  DesensitizationSourceType,
  I18NService,
  MaskRuleControllerService,
  IdentRuleControllerService
} from 'app/shared';
import { each, find, isEmpty, isNumber, toString as _toString } from 'lodash';
import { Observable, Observer } from 'rxjs';
import { pairwise } from 'rxjs/operators';

@Component({
  selector: 'aui-modify-identification-result',
  templateUrl: './modify-identification-result.component.html',
  styleUrls: ['./modify-identification-result.component.less']
})
export class ModifyIdentificationResultComponent implements OnInit {
  rowItem;
  columnName;
  formGroup: FormGroup;
  ruleOptions = [];
  showSensitiveDataIndex = false;
  startErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    startErrorTip: this.i18n.get('explore_mask_start_error_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 1024])
  };
  endErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    endErrorTip: this.i18n.get('explore_mask_end_error_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 1024])
  };
  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    private identRuleControllerService: IdentRuleControllerService,
    private policyManagerApiService: MaskRuleControllerService,
    private statisticsApiService: AnonyControllerService,
    private baseUtilService: BaseUtilService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      sensitive: new FormControl(!isEmpty(this.rowItem.pii)),
      type: new FormControl(this.rowItem.pii ? this.rowItem.pii.pattern : ''),
      startIndex: new FormControl(
        this.rowItem.pii ? +this.rowItem.pii.start + 1 : ''
      ),
      stopIndex: new FormControl(
        this.rowItem.pii ? +this.rowItem.pii.end + 1 : ''
      ),
      selectedRule: new FormControl(
        this.rowItem.pii && this.rowItem.mask_rule_id
          ? this.rowItem.mask_rule_id
          : ''
      )
    });
    if (!isEmpty(this.rowItem.pii)) {
      this.addValidators();
    }
    this.formGroup.get('sensitive').valueChanges.subscribe(res => {
      if (res) {
        this.addValidators();
      } else {
        this.formGroup.get('type').clearValidators();
        this.formGroup.get('startIndex').clearValidators();
        this.formGroup.get('stopIndex').clearValidators();
        this.formGroup.get('selectedRule').clearValidators();
        this.formGroup.get('type').updateValueAndValidity();
        this.formGroup.get('startIndex').updateValueAndValidity();
        this.formGroup.get('stopIndex').updateValueAndValidity();
        this.formGroup.get('selectedRule').updateValueAndValidity();
      }
    });
    this.formGroup.get('selectedRule').valueChanges.subscribe(res => {
      this.showSensitiveDataIndex =
        find(this.ruleOptions, { key: res }) &&
        find(this.ruleOptions, { key: res }).type ===
          DataMap.Desensitization_Rule_Type.partialMask.value &&
        this.formGroup.value.sensitive;
      if (this.showSensitiveDataIndex) {
        this.addIndexValidators();
      } else {
        this.formGroup.get('startIndex').setValue('');
        this.formGroup.get('stopIndex').setValue('');
        this.formGroup.get('startIndex').clearValidators();
        this.formGroup.get('stopIndex').clearValidators();
        this.formGroup.get('startIndex').updateValueAndValidity();
        this.formGroup.get('stopIndex').updateValueAndValidity();
      }
    });
    this.formGroup
      .get('startIndex')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (_toString(res[0]) === _toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (!this.formGroup.get('stopIndex').value) {
            return;
          }
          this.formGroup.get('stopIndex').markAsTouched();
          this.formGroup.get('stopIndex').updateValueAndValidity();
        }, 0);
      });
    this.formGroup
      .get('stopIndex')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (_toString(res[0]) === _toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (!this.formGroup.get('startIndex').value) {
            return;
          }
          this.formGroup.get('startIndex').markAsTouched();
          this.formGroup.get('startIndex').updateValueAndValidity();
        }, 0);
      });
  }

  addValidators() {
    this.formGroup
      .get('type')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('selectedRule')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup.get('type').updateValueAndValidity();
    this.formGroup.get('selectedRule').updateValueAndValidity();
  }

  addIndexValidators() {
    this.formGroup
      .get('startIndex')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.integer(),
        this.baseUtilService.VALID.rangeValue(1, 1024),
        this.validMaskStart()
      ]);
    this.formGroup
      .get('stopIndex')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.integer(),
        this.baseUtilService.VALID.rangeValue(1, 1024),
        this.validMaskEnd()
      ]);
    this.formGroup.get('startIndex').updateValueAndValidity();
    this.formGroup.get('stopIndex').updateValueAndValidity();
  }

  validMaskStart(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        !isEmpty(this.formGroup.value.stopIndex) &&
        this.formGroup.value.stopIndex < +control.value
      ) {
        return { startErrorTip: { value: control.value } };
      }
      return null;
    };
  }

  validMaskEnd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        !isEmpty(this.formGroup.value.startIndex) &&
        +this.formGroup.value.startIndex > +control.value
      ) {
        return { endErrorTip: { value: control.value } };
      }
      return null;
    };
  }

  getRules(recordsTemp?, startPage?) {
    this.policyManagerApiService
      .getPageMaskRulesUsingGET({
        pageNo: startPage || 0,
        pageSize: 100
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.items];
        if (startPage === Math.ceil(res.total / 100) || res.total === 0) {
          const arr = [];
          each(recordsTemp, item => {
            arr.push({
              key: item.id,
              value: item.id,
              label: item.name,
              type: item.type,
              isLeaf: true
            });
          });
          this.ruleOptions = arr;
          if (this.rowItem.pii && !this.rowItem.mask_rule_id) {
            this.getMaskRule();
          }
          if (this.rowItem.pii && this.rowItem.mask_rule_id) {
            const selectedMask = find(this.ruleOptions, {
              key: this.rowItem.mask_rule_id
            });
            this.showSensitiveDataIndex =
              selectedMask &&
              selectedMask.type ===
                DataMap.Desensitization_Rule_Type.partialMask.value;
            if (this.showSensitiveDataIndex) {
              this.addIndexValidators();
            }
          }
          return;
        }
        this.getRules(recordsTemp, startPage);
      });
  }

  getMaskRule() {
    if (this.rowItem.pii && this.rowItem.pii.pattern) {
      this.identRuleControllerService
        .getPageIdentRulesUsingGET({
          pageNo: CommonConsts.PAGE_START,
          pageSize: CommonConsts.PAGE_SIZE * 5,
          name: this.rowItem.pii.pattern,
          akDoException: false,
          akLoading: false
        })
        .subscribe(res => {
          const piiRule = find(res.items, { name: this.rowItem.pii.pattern });
          const maskRule = find(this.ruleOptions, { label: piiRule.mask_name });
          if (maskRule) {
            this.formGroup.get('selectedRule').setValue(maskRule.key);
            this.showSensitiveDataIndex =
              maskRule.type ===
              DataMap.Desensitization_Rule_Type.partialMask.value;
            if (this.showSensitiveDataIndex) {
              this.addIndexValidators();
            }
          }
        });
    }
  }

  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const piiParams = {
        confidence: 100,
        pattern: this.formGroup.value.type,
        start: this.showSensitiveDataIndex
          ? +this.formGroup.value.startIndex - 1
          : 0,
        end: this.showSensitiveDataIndex
          ? +this.formGroup.value.stopIndex - 1
          : 0
      };
      this.statisticsApiService
        .modifyIdentResultUsingPUT({
          resultRequest: {
            column_name: this.rowItem.label,
            db_id: this.rowItem.uuid,
            db_name: this.rowItem.name,
            db_type: DesensitizationSourceType[this.rowItem.sub_type],
            mask_rule_id: this.formGroup.value.sensitive
              ? this.formGroup.value.selectedRule
              : '',
            mask_rule_name: this.formGroup.value.sensitive
              ? find(this.ruleOptions, {
                  key: this.formGroup.value.selectedRule
                }).label
              : '',
            pii: this.formGroup.value.sensitive
              ? JSON.stringify([piiParams])
              : JSON.stringify([]),
            table_name: this.rowItem.parent.label,
            table_namespace: this.rowItem.parent.parent.label
          }
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
    });
  }

  ngOnInit() {
    this.initForm();
    this.getRules();
  }
}
