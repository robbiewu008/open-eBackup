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
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectResourceCategory
} from 'app/shared';
import {
  assign,
  get,
  has,
  includes,
  isArray,
  isEmpty,
  set,
  trim
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-advanced-parameter',
  templateUrl: './advanced-parameter.component.html',
  styleUrls: ['./advanced-parameter.component.less']
})
export class AdvancedParameterComponent implements OnInit {
  resourceData;
  resourceType;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  dataMap = DataMap;
  scriptErrorTip = {
    ...this.baseUtilService.scriptNameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  scriptPlaceholder;
  scriptTooltip;
  osType;

  constructor(
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private i18n: I18NService
  ) {}

  ngOnInit() {
    this.getOsType();
    this.initForm();
  }

  getOsType() {
    if (isArray(this.resourceData)) {
      this.osType = this.resourceData[0].environment_os_type;
    } else {
      this.osType = this.resourceData.environment_os_type;
    }

    if (this.resourceType === ProtectResourceCategory.SQLServer) {
      this.osType = DataMap.Os_Type.windows.value;
    }

    this.scriptPlaceholder = this.i18n.get(
      this.osType === DataMap.Os_Type.windows.value
        ? 'common_script_windows_placeholder_label'
        : 'common_script_linux_placeholder_label'
    );
    this.scriptTooltip = this.i18n.get(
      this.osType === DataMap.Os_Type.windows.value
        ? 'common_script_windows_help_label'
        : 'common_script_oracle_linux_help_label'
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      pre_script: new FormControl(
        !isEmpty(this.resourceData.ext_parameters)
          ? this.resourceData.ext_parameters.pre_script
          : '',
        {
          validators: [
            this.baseUtilService.VALID.maxLength(8192),
            this.osType === DataMap.Os_Type.windows.value
              ? this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.windowsScript,
                  false
                )
              : this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.linuxScript,
                  false
                )
          ],
          updateOn: 'change'
        }
      ),
      post_script: new FormControl(
        !isEmpty(this.resourceData.ext_parameters)
          ? this.resourceData.ext_parameters.post_script
          : '',
        {
          validators: [
            this.baseUtilService.VALID.maxLength(8192),
            this.osType === DataMap.Os_Type.windows.value
              ? this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.windowsScript,
                  false
                )
              : this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.linuxScript,
                  false
                )
          ],
          updateOn: 'change'
        }
      ),
      failed_script: new FormControl(
        !isEmpty(this.resourceData.ext_parameters)
          ? this.resourceData.ext_parameters.failed_script
          : '',
        {
          validators: [
            this.baseUtilService.VALID.maxLength(8192),
            this.osType === DataMap.Os_Type.windows.value
              ? this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.windowsScript,
                  false
                )
              : this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.linuxScript,
                  false
                )
          ],
          updateOn: 'change'
        }
      )
    });
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(this.formGroup.valid);
    });

    if (!isArray(this.resourceData)) {
      this.formGroup.patchValue({
        pre_script: get(
          this.resourceData,
          'protectedObject.extParameters.pre_script',
          ''
        ),
        post_script: get(
          this.resourceData,
          'protectedObject.extParameters.post_script',
          ''
        ),
        failed_script: get(
          this.resourceData,
          'protectedObject.extParameters.failed_script',
          ''
        )
      });
    }
  }

  initData(data: any, resourceType: string) {
    this.resourceData = data;
    this.resourceType = resourceType;
  }

  onOK() {
    const resourceData = assign(
      isArray(this.resourceData) ? this.resourceData[0] : this.resourceData,
      {
        ext_parameters: {}
      }
    );

    if (!isEmpty(trim(this.formGroup.value.pre_script))) {
      set(
        resourceData,
        'ext_parameters.pre_script',
        trim(this.formGroup.value.pre_script)
      );
    }

    if (!isEmpty(trim(this.formGroup.value.post_script))) {
      set(
        resourceData,
        'ext_parameters.post_script',
        trim(this.formGroup.value.post_script)
      );
    }

    if (!isEmpty(trim(this.formGroup.value.failed_script))) {
      set(
        resourceData,
        'ext_parameters.failed_script',
        trim(this.formGroup.value.failed_script)
      );
    }

    return resourceData;
  }
}
