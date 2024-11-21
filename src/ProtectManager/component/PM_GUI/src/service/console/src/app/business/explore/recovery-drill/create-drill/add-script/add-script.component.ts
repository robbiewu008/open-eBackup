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
  I18NService
} from 'app/shared';
import { includes, isEmpty } from 'lodash';

@Component({
  selector: 'aui-add-script',
  templateUrl: './add-script.component.html',
  styleUrls: ['./add-script.component.less']
})
export class AddScriptComponent implements OnInit {
  item: any;
  formGroup: FormGroup;
  scriptMaxLen = 8192;
  isWindows = false;
  scriptHelp = this.i18n.get('common_script_oracle_linux_help_label');
  scriptPlaceholder = this.i18n.get('common_script_linux_placeholder_label');

  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.scriptMaxLen
    ])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.isWindows =
      includes(
        [
          DataMap.Resource_Type.SQLServerInstance.value,
          DataMap.Resource_Type.SQLServerClusterInstance.value,
          DataMap.Resource_Type.SQLServerGroup.value,
          DataMap.Resource_Type.SQLServerDatabase.value,
          DataMap.Resource_Type.ExchangeDataBase.value,
          DataMap.Resource_Type.ActiveDirectory.value
        ],
        this.item?.subType
      ) ||
      (includes(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value
        ],
        this.item?.subType
      ) &&
        this.item?.environment?.osType === DataMap.Os_Type.windows.value);
    if (this.isWindows) {
      this.scriptHelp = this.i18n.get(
        'common_script_sqlserver_windows_help_label'
      );
      this.scriptPlaceholder = this.i18n.get(
        'common_script_windows_placeholder_label'
      );
    }
    this.initForm();
    this.updateForm();
  }

  initForm() {
    const scriptRegex = this.isWindows
      ? CommonConsts.REGEX.windowsScript
      : CommonConsts.REGEX.linuxScript;
    this.formGroup = this.fb.group({
      preScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(this.scriptMaxLen),
          this.baseUtilService.VALID.name(scriptRegex, false)
        ]
      }),
      postScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(this.scriptMaxLen),
          this.baseUtilService.VALID.name(scriptRegex, false)
        ]
      }),
      executeScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(this.scriptMaxLen),
          this.baseUtilService.VALID.name(scriptRegex, false)
        ]
      })
    });
  }

  updateForm() {
    if (isEmpty(this.item?.scriptConfig)) {
      return;
    }
    this.formGroup.patchValue({
      preScript: this.item.scriptConfig?.preScript,
      postScript: this.item.scriptConfig?.postScript,
      executeScript: this.item.scriptConfig?.executeScript
    });
  }
}
