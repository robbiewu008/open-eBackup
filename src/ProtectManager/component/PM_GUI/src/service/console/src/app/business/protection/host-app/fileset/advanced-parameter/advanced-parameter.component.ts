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
import { Component, OnDestroy, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  GlobalService,
  I18NService,
  ProtectResourceAction
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  find,
  has,
  includes,
  isArray,
  set,
  toNumber,
  trim
} from 'lodash';
import { Subject, Subscription } from 'rxjs';

@Component({
  selector: 'aui-advanced-parameter',
  templateUrl: './advanced-parameter.component.html',
  styleUrls: ['./advanced-parameter.component.less']
})
export class AdvancedParameterComponent implements OnInit, OnDestroy {
  resourceData;
  resourceType;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  channelsErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 40])
  };
  percentErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 100])
  };
  osType;
  dataMap = DataMap;
  enableSmallFile = false;
  enableScript = false;
  enableNfsBackup = false;
  scriptPlaceholder = '';
  scriptTips = '';
  isModified = false;
  disableSmallFile = false;
  isDetail = false;
  protectData;
  isExpanded = false;
  batchModify = false;
  extParams;
  hasRansomware = false; // 用于判断是否有已创建的防勒索策略
  isOsBackup = false; // 用于判断是否打开了操作系统备份
  ransomwareStatus$: Subscription = new Subscription();

  constructor(
    public fb: FormBuilder,
    private i18n: I18NService,
    private globalService: GlobalService,
    private baseUtilService: BaseUtilService,
    private messageService: MessageService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.getOsType();
    this.getRansomwareStatus();
    this.initForm();
  }

  ngOnDestroy() {
    this.ransomwareStatus$.unsubscribe();
  }

  initDetailData(data) {
    if (data.protectionStatus === DataMap.Protection_Status.protected.value) {
      this.isDetail = true;
      this.isExpanded = true;
      this.protectData = data.protectedObject.extParameters;
      this.enableSmallFile = this.protectData.small_file_aggregation;
      if (
        this.protectData.pre_script ||
        this.protectData.post_script ||
        this.protectData.failed_script
      ) {
        this.enableScript = true;
      }
    }
  }

  getOsType() {
    if (isArray(this.resourceData)) {
      this.osType =
        this.resourceData[0].environment_os_type ||
        this.resourceData[0]?.environment?.osType;
      this.isOsBackup = !!find(this.resourceData, item => item.osBackup);
    } else if (this.resourceData) {
      this.osType =
        this.resourceData.environment_os_type ||
        this.resourceData?.environment?.osType;
      this.isOsBackup = this.resourceData.osBackup;
    }
  }

  getRansomwareStatus() {
    // 开启了防勒索策略的资源是不能开小文件聚合的
    this.ransomwareStatus$ = this.globalService
      .getState('syncRansomwareStatus')
      .subscribe(res => {
        this.hasRansomware = res;
      });
  }

  initForm() {
    this.scriptPlaceholder =
      this.osType === DataMap.Os_Type.windows.value
        ? this.i18n.get('protection_fileset_advance_script_windows_label')
        : this.i18n.get('protection_fileset_advance_script_linux_label');
    this.scriptTips =
      this.osType === DataMap.Os_Type.windows.value
        ? this.i18n.get('protection_fileset_advance_script_windows_tips_label')
        : this.i18n.get('protection_fileset_advance_script_linux_tips_label');
    this.formGroup = this.fb.group({
      channels: new FormControl(1, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 40)
        ]
      }),
      sameBackup: new FormControl(true),
      snapshot_size_percent: new FormControl(5, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 100)
        ]
      }),
      crossFileBackup: new FormControl(true),
      nfsBackup: new FormControl(true),
      continueBackup: new FormControl(true),
      sparseFileDetect: new FormControl(false),
      smallFile: new FormControl(false),
      fileSize: new FormControl(4096),
      maxFileSize: new FormControl(1024),
      script: new FormControl(false),
      preScript: new FormControl('', {
        validators: [
          this.validPath(),
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
        ]
      }),
      postScript: new FormControl('', {
        validators: [
          this.validPath(),
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
        ]
      }),
      executeScript: new FormControl('', {
        validators: [
          this.validPath(),
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
        ]
      })
    });
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(
        this.formGroup.valid &&
          this.formGroup.get('fileSize').value >=
            this.formGroup.get('maxFileSize').value
      );
    });

    this.formGroup.get('sameBackup').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('snapshot_size_percent')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 100)
          ]);
      } else {
        this.formGroup.get('snapshot_size_percent').clearValidators();
      }
      this.formGroup.get('snapshot_size_percent').updateValueAndValidity();
    });

    this.formGroup.get('smallFile').valueChanges.subscribe(res => {
      if (res) {
        this.enableSmallFile = true;
      } else {
        this.enableSmallFile = false;
      }
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

    this.formGroup.get('fileSize').valueChanges.subscribe(res => {
      if (
        this.formGroup.get('fileSize').value <
        this.formGroup.get('maxFileSize').value
      ) {
        this.messageService.error(
          this.i18n.get('protection_small_file_size_tips_label')
        );
      }
    });

    this.formGroup.get('maxFileSize').valueChanges.subscribe(res => {
      if (
        this.formGroup.get('fileSize').value <
        this.formGroup.get('maxFileSize').value
      ) {
        this.messageService.error(
          this.i18n.get('protection_small_file_size_tips_label')
        );
      }
    });

    this.formGroup.get('crossFileBackup').valueChanges.subscribe(res => {
      if (res) {
        this.enableNfsBackup = true;
        this.formGroup.get('nfsBackup').enable();
      } else {
        this.enableNfsBackup = false;
        this.formGroup.get('nfsBackup').disable();
      }
    });

    if (has(this.resourceData, 'protectedObject.extParameters')) {
      this.isModified = true;
      const extParameters: any =
        this.resourceData.protectedObject.extParameters || {};
      const isWindows =
        this.osType === DataMap.Fileset_Template_Os_Type.windows.value;

      const script =
        extParameters.pre_script ||
        extParameters.post_script ||
        extParameters.failed_script;

      this.extParams = extParameters;
      this.formGroup.patchValue({
        channels: extParameters.channels || 1,
        sameBackup: extParameters.consistent_backup,
        snapshot_size_percent: extParameters?.snapshot_size_percent || 5,
        crossFileBackup: extParameters.cross_file_system,
        nfsBackup: isWindows
          ? extParameters.backup_smb
          : extParameters.backup_nfs,
        sparseFileDetect: extParameters.sparse_file_detection,
        continueBackup: extParameters.backup_continue_with_files_backup_failed,
        smallFile: extParameters.small_file_aggregation,
        script: script,
        preScript: extParameters.pre_script,
        postScript: extParameters.post_script,
        executeScript: extParameters.failed_script
      });
    }
    if (this.batchModify) {
      this.isModified = true;
    }
    if (this.isOsBackup) {
      this.formGroup.get('crossFileBackup').setValue(true);
    }
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (this.osType === DataMap.Os_Type.windows.value) {
        return;
      }

      const reg = /[|;&$<>`\\!]+/;

      if (reg.test(control.value) || includes(control.value, '..')) {
        return { invalidName: { value: control.value } };
      }

      return null;
    };
  }

  initData(data: any, resourceType: string, action: ProtectResourceAction) {
    this.resourceData = data;
    this.resourceType = resourceType;
    this.batchModify =
      action === ProtectResourceAction.Modify &&
      isArray(data) &&
      data.length > 1;
  }

  getParams() {
    const params = {
      channels: toNumber(this.formGroup.get('channels').value),
      consistent_backup: this.formGroup.get('sameBackup').value,
      cross_file_system: this.formGroup.get('crossFileBackup').value,
      backup_nfs: false,
      backup_smb: false,
      sparse_file_detection: this.formGroup.get('sparseFileDetect').value,
      backup_continue_with_files_backup_failed: this.formGroup.get(
        'continueBackup'
      ).value,
      small_file_aggregation: this.formGroup.get('smallFile').value
    };

    if (this.formGroup.get('sameBackup').value) {
      assign(params, {
        snapshot_size_percent: toNumber(
          this.formGroup.get('snapshot_size_percent').value
        )
      });
    }

    if (this.osType === DataMap.Fileset_Template_Os_Type.windows.value) {
      set(params, 'backup_smb', this.formGroup.get('nfsBackup').value);
    } else {
      set(params, 'backup_nfs', this.formGroup.get('nfsBackup').value);
    }

    if (this.formGroup.get('smallFile').value) {
      set(
        params,
        'aggregation_file_size',
        this.formGroup.get('fileSize').value
      );
      set(
        params,
        'aggregation_file_max_size',
        this.formGroup.get('maxFileSize').value
      );
    }

    if (this.formGroup.get('script').value) {
      if (trim(this.formGroup.get('preScript').value)) {
        set(params, 'pre_script', trim(this.formGroup.get('preScript').value));
      }

      if (trim(this.formGroup.get('postScript').value)) {
        set(
          params,
          'post_script',
          trim(this.formGroup.get('postScript').value)
        );
      }

      if (trim(this.formGroup.get('executeScript').value)) {
        set(
          params,
          'failed_script',
          trim(this.formGroup.get('executeScript').value)
        );
      }
    }

    // 手动设置索引
    each(
      [
        'backup_res_auto_index',
        'archive_res_auto_index',
        'tape_archive_auto_index',
        'enable_security_archive'
      ],
      key => {
        if (this.formGroup.get(key)) {
          assign(params, {
            [key]: this.formGroup.get(key).value
          });
        }
      }
    );

    return params;
  }

  onOK() {
    const resourceData = isArray(this.resourceData)
      ? this.resourceData[0]
      : this.resourceData;
    return assign(resourceData, {
      ext_parameters: {
        ...this.getParams()
      }
    });
  }

  updateForm(osType) {
    this.osType = osType;
    if (osType === DataMap.Os_Type.windows.value) {
      this.formGroup
        .get('preScript')
        .setValidators([
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.windowsScript,
            false
          )
        ]);

      this.formGroup
        .get('postScript')
        .setValidators([
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.windowsScript,
            false
          )
        ]);

      this.formGroup
        .get('executeScript')
        .setValidators([
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.windowsScript,
            false
          )
        ]);

      this.scriptTips = this.i18n.get(
        'protection_fileset_advance_script_windows_tips_label'
      );
      this.scriptPlaceholder = this.i18n.get(
        'protection_fileset_advance_script_windows_label'
      );
    } else {
      this.formGroup
        .get('preScript')
        .setValidators([
          this.validPath(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]);

      this.formGroup
        .get('postScript')
        .setValidators([
          this.validPath(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]);

      this.formGroup
        .get('executeScript')
        .setValidators([
          this.validPath(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]);

      this.scriptTips = this.i18n.get(
        'protection_fileset_advance_script_linux_tips_label'
      );
      this.scriptPlaceholder = this.i18n.get(
        'protection_fileset_advance_script_linux_label'
      );
    }
  }
}
