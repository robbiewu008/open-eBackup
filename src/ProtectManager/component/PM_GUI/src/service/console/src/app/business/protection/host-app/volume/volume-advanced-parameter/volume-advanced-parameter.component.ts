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
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService
} from 'app/shared';
import {
  assign,
  each,
  includes,
  isArray,
  isString,
  set,
  toNumber,
  trim
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-volume-advanced-parameter',
  templateUrl: './volume-advanced-parameter.component.html',
  styleUrls: ['./volume-advanced-parameter.component.less']
})
export class VolumeAdvancedParameterComponent implements OnInit {
  resourceData;
  resourceType;
  dataMap = DataMap;
  enableScript = false;
  scriptPlaceholder = '';
  scriptTips = '';
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  percentErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 100])
  };

  extParams;
  isWindows = false;

  constructor(
    public fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private i18n: I18NService
  ) {}

  ngOnInit(): void {
    this.isWindows =
      this.resourceData?.environment?.osType === DataMap.Os_Type.windows.value;
    this.initForm();
    this.updateData();
  }

  initForm() {
    this.scriptPlaceholder = this.i18n.get(
      'protection_fileset_advance_script_linux_label'
    );
    this.scriptTips = this.i18n.get(
      'protection_fileset_advance_script_linux_tips_label'
    );
    this.formGroup = this.fb.group({
      snapshot_size_percent: new FormControl(5, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 100)
        ]
      }),
      osBackup: new FormControl(
        this.resourceData.extendInfo?.system_backup_flag === 'true'
      ),
      script: new FormControl(false),
      preScript: new FormControl('', {
        validators: [
          this.validPath(),
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]
      }),
      postScript: new FormControl('', {
        validators: [
          this.validPath(),
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]
      }),
      executeScript: new FormControl('', {
        validators: [
          this.validPath(),
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]
      })
    });
    if (this.resourceData.extendInfo?.system_backup_flag === 'false') {
      // 创建不选系统备份则不能开
      this.formGroup.get('osBackup').disable();
    }
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(this.formGroup.valid);
    });

    this.formGroup.get('script').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup.get('preScript').enable();
        this.formGroup.get('postScript').enable();
        this.formGroup.get('executeScript').enable();
        this.enableScript = true;
      } else {
        this.formGroup.get('preScript').disable();
        this.formGroup.get('postScript').disable();
        this.formGroup.get('executeScript').disable();
        this.enableScript = false;
      }
    });
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return;
      }
      const reg = /[|;&$<>`\\!]+/;

      if (reg.test(control.value) || includes(control.value, '..')) {
        return { invalidName: { value: control.value } };
      }

      return null;
    };
  }

  updateData() {
    if (!this.resourceData.protectedObject?.extParameters) {
      return;
    }
    const extParameters = isString(
      this.resourceData.protectedObject?.extParameters
    )
      ? JSON.parse(this.resourceData.protectedObject?.extParameters)
      : this.resourceData.protectedObject?.extParameters;

    assign(extParameters, {
      snapshot_size_percent: extParameters?.snapshot_size_percent || 5,
      osBackup: extParameters.system_backup_flag,
      script:
        extParameters?.pre_script ||
        extParameters?.post_script ||
        extParameters?.failed_script,
      preScript: extParameters?.pre_script || '',
      postScript: extParameters?.post_script || '',
      executeScript: extParameters?.failed_script || ''
    });
    this.formGroup.patchValue(extParameters);
    // 索引设置
    this.extParams = extParameters;
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
  }

  onOK() {
    const ext_parameters = {};
    assign(ext_parameters, {
      system_backup_flag: this.formGroup.value?.osBackup,
      snapshot_size_percent: toNumber(
        this.formGroup.get('snapshot_size_percent').value
      )
    });

    if (this.formGroup.get('script').value) {
      if (trim(this.formGroup.get('preScript').value)) {
        set(
          ext_parameters,
          'pre_script',
          trim(this.formGroup.get('preScript').value)
        );
      }

      if (trim(this.formGroup.get('postScript').value)) {
        set(
          ext_parameters,
          'post_script',
          trim(this.formGroup.get('postScript').value)
        );
      }

      if (trim(this.formGroup.get('executeScript').value)) {
        set(
          ext_parameters,
          'failed_script',
          trim(this.formGroup.get('executeScript').value)
        );
      }
    }

    each(['backup_res_auto_index', 'archive_res_auto_index'], key => {
      if (this.formGroup.get(key)) {
        assign(ext_parameters, {
          [key]: this.formGroup.get(key).value
        });
      }
    });

    return {
      ext_parameters
    };
  }
}
