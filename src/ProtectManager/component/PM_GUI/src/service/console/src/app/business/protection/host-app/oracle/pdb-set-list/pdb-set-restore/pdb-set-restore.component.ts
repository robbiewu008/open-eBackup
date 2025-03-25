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
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType,
  SYSTEM_TIME
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { each, find, includes, isEmpty, map } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-pdb-set-restore',
  templateUrl: './pdb-set-restore.component.html',
  styleUrls: ['./pdb-set-restore.component.less']
})
export class PdbSetRestoreComponent implements OnInit {
  formGroup: FormGroup;
  databaseOptions = [];
  rowCopy;
  oldVersion;
  restoreToNewLocationOnly;
  restoreToNewLocationOnlyTips = this.i18n.get(
    'protection_origin_restore_disabled_label'
  );
  resourceProperties;
  disableNewLocation = false; // oracle有些场景不支持新位置恢复
  restoreType;
  filterParams = [];
  restoreV2LocationType = RestoreV2LocationType;
  dataMap = DataMap;
  scriptArr = [
    {
      key: 'preProcessing',
      label: this.i18n.get('protection_restore_pre_script_label')
    },
    {
      key: 'postProcessing',
      label: this.i18n.get('protection_restore_post_script_label')
    },
    {
      key: 'failedProcessing',
      label: this.i18n.get('protection_restore_fail_script_label')
    }
  ];
  isWindows = false;
  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  destinationPathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidPath: this.baseUtilService.invalidPathLabel,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
  };
  numberOfChannelRangeErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 254])
  };
  isCluster = false;

  timeZone = SYSTEM_TIME.timeZone;

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.formatRowCopy();
    this.initForm();
    this.getDatabase();
  }

  formatRowCopy() {
    if (this.rowCopy.resource_properties) {
      let resourceProperties;
      resourceProperties = JSON.parse(this.rowCopy.resource_properties);
      this.rowCopy.environment_os_type =
        resourceProperties.environment_os_type ||
        resourceProperties.environment?.osType;
      this.rowCopy.environment_uuid =
        resourceProperties.environment_uuid ||
        resourceProperties.environment?.uuid ||
        resourceProperties.rootUuid;
      this.resourceProperties = resourceProperties;
      this.isCluster =
        resourceProperties.resource_sub_type ===
        DataMap.Resource_Type.oracleCluster.value;
      this.oldVersion = resourceProperties.version || '';
      this.isWindows =
        this.rowCopy.environment_os_type === DataMap.Os_Type.windows.value;
    }
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) || this.rowCopy.is_replicated;
    if (
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.restoreToNewLocationOnly = true;
      this.restoreToNewLocationOnlyTips = this.i18n.get(
        'protection_unsupport_restore_to_online_database_label'
      );
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl(
        `${this.rowCopy?.resource_environment_name || ''}/${
          this.rowCopy?.resource_name
        }`
      ),
      database: new FormControl(
        {
          value: '',
          disabled: true
        },
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      destinationPath: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256),
          this.baseUtilService.VALID.path(this.rowCopy.environment_os_type)
        ]
      }),
      isOverwrite: new FormControl(false),
      bctStatus: new FormControl(false),
      power_on: new FormControl(false),
      open_pdb: new FormControl(true),
      numberOfChannelOpen: new FormControl(false),
      dbConfig: this.fb.array([]),
      numberOfChannels: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 254)
        ]
      }),
      scriptOpen: new FormControl(false),
      preProcessing: new FormControl(''),
      postProcessing: new FormControl(''),
      failedProcessing: new FormControl('')
    });
    this.listenForm();
    if (this.restoreToNewLocationOnly) {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
    }
  }

  listenForm() {
    this.restoreToValueChange();
    this.scriptSwitchValueChange();
    this.channelNumberValueChange();
  }

  restoreToValueChange() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.NEW) {
        this.formGroup.get('database').enable();
      } else {
        this.formGroup.get('database').disable();
      }
    });
  }

  scriptSwitchValueChange() {
    this.formGroup.get('scriptOpen').valueChanges.subscribe(res => {
      each(this.scriptArr, item => {
        this.createScriptFormControl(res, item);
        this.formGroup.get(item.key).updateValueAndValidity();
      });
    });
  }

  createScriptFormControl(res, item) {
    if (res) {
      this.formGroup
        .get(item.key)
        .setValidators([
          this.isWindows
            ? this.baseUtilService.VALID.name(
                CommonConsts.REGEX.windowsScript,
                false
              )
            : this.baseUtilService.VALID.name(
                CommonConsts.REGEX.linuxScript,
                false
              ),
          this.baseUtilService.VALID.maxLength(8192)
        ]);
    } else {
      this.formGroup.get(item.key).clearValidators();
    }
  }

  channelNumberValueChange() {
    this.formGroup.get('numberOfChannelOpen').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('numberOfChannels')
          .setValidators([
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 254),
            this.baseUtilService.VALID.required()
          ]);
      } else {
        this.formGroup.get('numberOfChannels').clearValidators();
      }
      this.formGroup.get('numberOfChannels').updateValueAndValidity();
    });
  }

  get dbConfig() {
    return (this.formGroup.get('dbConfig') as FormArray).controls;
  }

  getDatabase() {
    const extParams = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      queryDependency: true,
      conditions: JSON.stringify({
        subType: [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value
        ]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      param => this.protectedResourceApiService.ListResources(param),
      resource => {
        this.databaseOptions = map(resource, item => ({
          ...item,
          linkStatus: item.extendInfo.linkStatus,
          key: item.uuid,
          value: item.uuid,
          label: `${item.name}(${item.environment.endpoint})`,
          isLeaf: true
        }));
      }
    );
  }

  getParams() {
    const restoreTargetHost = {};
    const isOrigin =
      this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN;
    const dbConfigControls = (this.formGroup.get('dbConfig') as FormArray)
      .controls;
    dbConfigControls.forEach(control => {
      restoreTargetHost[control.value.key] = control.value.newParam
        ? control.value.newParam
        : control.value.originParam;
    });
    const targetDatabase = find(this.databaseOptions, {
      value: this.formGroup.value.database
    });
    const params = {
      copyId: this.rowCopy.uuid,
      agents: [],
      targetEnv: isOrigin
        ? this.rowCopy.environment_uuid
        : targetDatabase.rootUuid,
      targetLocation: this.formGroup.value.restoreTo,
      restoreType: this.restoreType,
      targetObject: isOrigin
        ? this.resourceProperties.extendInfo.dbUuid
        : this.formGroup.value.database,
      extendInfo: {
        restoreFrom:
          this.rowCopy?.backup_type === DataMap.CopyData_Backup_Type.log.value
            ? 'log'
            : 'data',
        bctStatus: this.formGroup.value.bctStatus,
        CHANNELS: this.formGroup.value.numberOfChannels || '',
        UUID: this.resourceProperties.uuid || '',
        RESTORE_TARGET_HOST: isEmpty(restoreTargetHost)
          ? ''
          : JSON.stringify(restoreTargetHost),
        isOverwrite: this.formGroup.get('isOverwrite').value,
        isOpenPdb: this.formGroup.get('open_pdb').value,
        RESTORE_PATH: this.formGroup.value.destinationPath,
        instances: '[]'
      },
      scripts: {
        preScript: this.formGroup.value.scriptOpen
          ? this.formGroup.value.preProcessing || ''
          : '',
        postScript: this.formGroup.value.scriptOpen
          ? this.formGroup.value.postProcessing || ''
          : '',
        failPostScript: this.formGroup.value.scriptOpen
          ? this.formGroup.value.failedProcessing || ''
          : ''
      }
    };
    return params;
  }

  buildRestoreCmd(params, observer: Observer<void>) {
    this.restoreV2Service
      .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
      .subscribe({
        next: value => {
          observer.next();
          observer.complete();
        },
        error: err => {
          observer.error(err);
          observer.complete();
        }
      });
  }

  restore(): Observable<void> {
    const params = this.getParams();
    return new Observable<void>((observer: Observer<void>) => {
      this.buildRestoreCmd(params, observer);
    });
  }
}
