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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageboxService, MessageService } from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  I18NService,
  MODAL_COMMON,
  MultiCluster
} from 'app/shared';
import { assign, each, isArray, isString, set } from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-object-advanced-parameter',
  templateUrl: './object-advanced-parameter.component.html',
  styleUrls: ['./object-advanced-parameter.component.less']
})
export class ObjectAdvancedParameterComponent implements OnInit {
  resourceData;
  resourceType;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  isModified = false;
  dataMap = DataMap;
  enableSmallFile = false;
  disableSmallFile = false;
  isAutoIndex = false;
  isMulti = MultiCluster.isMulti;
  isHCS = false;
  extParams;

  depthTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 20]),
    invalidInput: this.i18n.get('common_valid_integer_label')
  };
  retryTimesErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 5])
  };

  @ViewChild('warningContentTpl', { static: true })
  warningContentTpl: TemplateRef<any>;

  constructor(
    public fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private i18n: I18NService,
    private messageBox: MessageboxService,
    private messageService: MessageService
  ) {}

  ngOnInit(): void {
    this.isHCS =
      Number(this.resourceData.extendInfo.storageType) ===
      DataMap.objectStorageType.hcs.value;
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      multiNodeBackupSwitch: new FormControl(false),
      continueOnFailedSwitch: new FormControl(true),
      isBackupAcl: new FormControl(false),
      useBucketLog: new FormControl(false),
      checkPoint: new FormControl(false),
      retryNum: new FormControl(3, {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 5)
        ]
      }),
      smallFile: new FormControl(false),
      maxSizeAfterAggregate: new FormControl(
        DataMap.Small_File_Size.xlarge.value
      ),
      maxSizeToAggregate: new FormControl(DataMap.Small_File_Size.small.value)
    });
    this.listenForm();
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(
        this.formGroup.valid &&
          this.formGroup.get('maxSizeAfterAggregate').value >=
            this.formGroup.get('maxSizeToAggregate').value
      );
    });

    this.formGroup.get('smallFile').valueChanges.subscribe(res => {
      this.enableSmallFile = res;
    });

    this.formGroup.get('maxSizeAfterAggregate').valueChanges.subscribe(res => {
      if (
        this.formGroup.get('maxSizeAfterAggregate').value <
        this.formGroup.get('maxSizeToAggregate').value
      ) {
        this.messageService.error(
          this.i18n.get('protection_small_file_size_tips_label')
        );
      }
    });

    this.formGroup.get('maxSizeToAggregate').valueChanges.subscribe(res => {
      if (
        this.formGroup.get('maxSizeAfterAggregate').value <
        this.formGroup.get('maxSizeToAggregate').value
      ) {
        this.messageService.error(
          this.i18n.get('protection_small_file_size_tips_label')
        );
      }
    });

    this.formGroup.get('checkPoint').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('retryNum')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 5)
          ]);
      } else {
        this.formGroup.get('retryNum').clearValidators();
      }
      this.formGroup.get('retryNum').updateValueAndValidity();
    });

    this.formGroup.get('useBucketLog').valueChanges.subscribe(res => {
      if (res && !this.isModified) {
        this.messageBox.info({
          lvOkText: this.i18n.get('common_close_label'),
          lvWidth: MODAL_COMMON.smallWidth,
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvContent: this.warningContentTpl
        });
      }
    });

    this.updateData();
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
  }

  updateData() {
    if (!this.resourceData?.protectedObject?.extParameters) {
      return;
    }

    this.isModified = true;
    const extParameters = isString(
      this.resourceData.protectedObject?.extParameters
    )
      ? JSON.parse(this.resourceData.protectedObject?.extParameters)
      : this.resourceData.protectedObject?.extParameters;
    this.formGroup.patchValue({
      multiNodeBackupSwitch: extParameters?.multiNodeBackupSwitch,
      isBackupAcl: extParameters?.isBackupAcl,
      useBucketLog: extParameters?.useBucketLog,
      continueOnFailedSwitch: extParameters?.continueOnFailedSwitch,
      smallFile: extParameters?.aggregateSwitch,
      checkPoint: extParameters?.checkPoint,
      retryNum: extParameters?.retryNum || 3
    });
    // 索引
    this.extParams = extParameters;
  }

  onOK() {
    const ext_parameters = {
      multiNodeBackupSwitch: this.formGroup.value.multiNodeBackupSwitch,
      isBackupAcl: this.formGroup.value.isBackupAcl,
      useBucketLog: this.formGroup.value.useBucketLog,
      aggregateSwitch: this.formGroup.value.smallFile,
      checkPoint: this.formGroup.get('checkPoint').value
    };

    if (this.formGroup.get('multiNodeBackupSwitch').value) {
      if (this.formGroup.value?.continueOnFailedSwitch) {
        set(
          ext_parameters,
          'continueOnFailedSwitch',
          this.formGroup.value.continueOnFailedSwitch
        );
      }
    }

    if (this.formGroup.get('smallFile').value) {
      set(
        ext_parameters,
        'maxSizeAfterAggregate',
        this.formGroup.get('maxSizeAfterAggregate').value
      );
      set(
        ext_parameters,
        'maxSizeToAggregate',
        this.formGroup.get('maxSizeToAggregate').value
      );
    }

    if (this.formGroup.get('checkPoint').value) {
      set(
        ext_parameters,
        'retryNum',
        Number(this.formGroup.get('retryNum').value)
      );
    }

    each(
      [
        'backup_res_auto_index',
        'archive_res_auto_index',
        'tape_archive_auto_index'
      ],
      key => {
        if (this.formGroup.get(key)) {
          assign(ext_parameters, {
            [key]: this.formGroup.get(key).value
          });
        }
      }
    );

    return { ext_parameters };
  }
}
