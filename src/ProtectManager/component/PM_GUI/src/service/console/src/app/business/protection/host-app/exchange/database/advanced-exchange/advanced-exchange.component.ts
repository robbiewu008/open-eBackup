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
import { MessageService } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import { assign, first, includes, isArray, isEmpty } from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-advanced-exchange',
  templateUrl: './advanced-exchange.component.html',
  styleUrls: ['./advanced-exchange.component.less']
})
export class AdvancedExchangeComponent implements OnInit {
  resourceData;
  resourceType;
  selectedNode;
  hostOptions = [];
  formGroup: FormGroup;
  dataMap = DataMap;
  showDAGBackUp = false;
  dagBackUpOpts = this.dataMapService
    .toArray('dagBackupType')
    .filter(item => (item.isLeaf = true));
  valid$ = new Subject<boolean>();
  scriptErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  scriptPlaceHolder = this.i18n.get(
    'protection_fileset_advance_script_windows_label'
  );
  scriptToolTip = this.i18n.get('common_script_agent_windows_position_label');

  constructor(
    public fb: FormBuilder,
    private i18n: I18NService,
    public message: MessageService,
    public baseUtilService: BaseUtilService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit(): void {
    // 只有DAG才有多台服务器，这时才有主动数据、被动数据库的概念
    this.showDAGBackUp =
      DataMap.Resource_Type.ExchangeGroup.value === this.resourceData.subType ||
      (DataMap.Resource_Type.ExchangeDataBase.value ===
        this.resourceData.subType &&
        this.resourceData.environment?.subType ===
          DataMap.Resource_Type.ExchangeGroup.value);
    this.initForm();
    this.formGroup.statusChanges.subscribe(() => {
      this.valid$.next(this.formGroup.value);
    });
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
  }

  initForm() {
    this.formGroup = this.fb.group({
      before_protect_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.windowsScript,
            false
          )
        ]
      }),
      after_protect_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.windowsScript,
            false
          )
        ]
      }),
      protect_failed_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.windowsScript,
            false
          )
        ]
      }),
      copyVerify: new FormControl(false),
      dagBackup: new FormControl(first(this.dagBackUpOpts).value)
    });
    const { protectedObject } = this.resourceData;
    const extParameters = protectedObject?.extParameters || {};
    const { pre_script, post_script, failed_script } = extParameters;
    this.formGroup
      .get('copyVerify')
      .setValue(
        isEmpty(protectedObject) ? false : !!extParameters?.m_isConsistent
      );
    this.formGroup
      .get('dagBackup')
      .setValue(isEmpty(protectedObject) ? '' : extParameters?.dag_backup);
    this.formGroup.statusChanges.subscribe(() => {
      this.valid$.next(this.formGroup.value);
    });
    this.formGroup.patchValue({
      before_protect_script: pre_script,
      after_protect_script: post_script,
      protect_failed_script: failed_script
    });
  }

  onOK() {
    const ext_parameters = {};
    assign(ext_parameters, {
      m_isConsistent: this.formGroup.value.copyVerify
    });
    if (this.showDAGBackUp) {
      assign(ext_parameters, {
        dag_backup: this.formGroup.value.dagBackup
      });
    } else {
      assign(ext_parameters, {
        pre_script: this.formGroup.value.before_protect_script || null,
        post_script: this.formGroup.value.after_protect_script || null,
        failed_script: this.formGroup.value.protect_failed_script || null
      });
    }
    return {
      ext_parameters
    };
  }
}
