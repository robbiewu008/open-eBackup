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
import { Component, Input, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService, ModalRef } from '@iux/live';
import {
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  extendParams,
  I18NService,
  OverWriteOption,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type,
  Scene
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  each,
  filter,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isNumber,
  isString,
  map,
  set,
  split,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-alwayson-restore',
  templateUrl: './alwayson-restore.component.html',
  styleUrls: ['./alwayson-restore.component.less']
})
export class SQLServerAlwaysOnComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  copyLocation;
  hostData = [];
  instData = [];
  filterParams = [];
  options = [];
  instanceOptions = [];
  dataMap = DataMap;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = OverWriteOption;
  formGroup: FormGroup;
  resourceData;
  location = this.i18n.get('common_location_label');
  isClusterInstance = false;
  originalLocation;
  disableOriginLocation = false;
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    pathError: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048])
  };
  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  isSupport = true;
  instancesList = [];
  constructor(
    public i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private modal: ModalRef,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private messageService: MessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      host: new FormControl(this.resourceData.environment_uuid, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      instance: new FormControl(this.rowCopy.resource_id, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      path: new FormControl('')
    });

    this.watch();
    if (this.disableOriginLocation) {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
    } else {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.ORIGIN);
    }
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      this.instanceOptions = [];
      this.isSupport = true;
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('host').setValue(this.resourceData.environment_uuid);
        this.formGroup.get('host').disable();
        this.formGroup.get('instance').disable();
        this.formGroup.get('path').disable();
        this.location = this.i18n.get('common_location_label');
        if (this.restoreType === RestoreType.FileRestore) {
          this.getClusters();
        }
      } else {
        this.formGroup.get('host').setValue('');
        this.formGroup.get('host').enable();
        this.formGroup.get('instance').enable();
        this.formGroup.get('path').enable();
        this.location = this.i18n.get('explore_target_host_cluster_label');
        if (this.restoreType === RestoreType.FileRestore) {
          this.getHosts();
        }
      }
    });

    this.formGroup.get('host').valueChanges.subscribe(res => {
      this.formGroup.get('instance').setValue('');
      this.getInstance();
    });

    this.formGroup.statusChanges.subscribe(res => {
      this.disableOkBtn();
    });
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};

    if (
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      return;
    }

    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowCopy.resource_id })
      .subscribe(res => {
        this.resourceData.environment_uuid = res.environment.uuid;
        this.formGroup.get('host').setValue(this.resourceData.environment_uuid);
      });
  }

  getHosts(labelParams?: any) {
    const conditions = {
      type: 'Plugin',
      subType: [`${DataMap.Resource_Type.SQLServerInstance.value}Plugin`]
    };
    extendParams(conditions, labelParams);
    const extParams = {
      conditions: JSON.stringify(conditions)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.osType === DataMap.Os_Type.windows.value &&
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          }
        });
        this.options = hostArray;
      }
    );
  }

  getClusters(recordsTemp?: any[], startPage?: number, labelParams?: any) {
    const conditions = {
      subType: DataMap.Resource_Type.SQLServerCluster.value
    };
    extendParams(conditions, labelParams);
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        this.options = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name || '',
            isLeaf: true
          };
        });
        this.updateDrillData();
        return;
      }
      this.getClusters(recordsTemp, startPage, labelParams);
    });
  }

  getInstance(recordsTemp?: any[], startPage?: number) {
    if (!this.formGroup.value.host) {
      return;
    }

    const endpoint = get(
      find(
        this.options,
        item => item.uuid === this.formGroup.get('host').value
      ),
      'endpoint'
    );
    const conditions =
      this.restoreType === RestoreType.CommonRestore
        ? {
            subType: [
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value
            ],
            environment: {
              uuid: this.formGroup.value.host
            }
          }
        : {
            subType: [
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value
            ],
            environment: {
              endpoint: [['~~'], endpoint]
            }
          };

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        if (
          res.totalCount < 2 &&
          this.formGroup.value.restoreTo === RestoreV2LocationType.NEW &&
          this.restoreType === RestoreType.CommonRestore
        ) {
          this.messageService.error(
            this.i18n.get('protection_sql_server_no_instance_label', [
              find(
                this.options,
                item => item.uuid === this.formGroup.value.host
              ).name
            ]),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'noInstanceMesageKey'
            }
          );
          this.formGroup.get('instance').setValue('');
          return;
        }

        if (
          this.formGroup.value.restoreTo === RestoreV2LocationType.NEW &&
          this.restoreType !== RestoreType.CommonRestore &&
          !find(recordsTemp, item => {
            return (
              item.path === endpoint || item.environment.endpoint === endpoint
            );
          })
        ) {
          this.messageService.error(
            this.i18n.get('protection_sql_server_fr_no_instance_label', [
              find(
                this.options,
                item => item.uuid === this.formGroup.value.host
              )?.name
            ]),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'noInstanceMesageKey'
            }
          );
          this.formGroup.get('instance').setValue('');
          return;
        }
        this.instanceOptions = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          };
        });
        if (
          this.formGroup.get('restoreTo').value === this.restoreLocationType.NEW
        ) {
          if (!isEmpty(this.instanceOptions)) {
            this.isSupportFunc(
              this.instanceOptions[0].environment.endpoint.split(',')
            );
          }
        } else {
          this.formGroup.get('path').clearValidators();
          this.formGroup.get('path').updateValueAndValidity();
        }
        this.formGroup
          .get('instance')
          .setValue(get(first(this.instanceOptions), 'key'));
        return;
      }
      this.getInstance(recordsTemp, startPage);
      this.instancesList = recordsTemp;
    });
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? {
              name:
                this.rowCopy?.resource_environment_name ||
                this.rowCopy?.resource_location,
              value: this.resourceData?.environment_uuid
            }
          : {
              name: find(
                this.options,
                item => item.uuid === this.formGroup.value.host
              )['label'],
              value: this.formGroup.value.host,
              endpoint: find(
                this.options,
                item => item.uuid === this.formGroup.value.host
              )['endpoint']
            },
      restoreLocation: this.formGroup.value.restoreTo,
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? this.rowCopy?.resource_environment_name ||
          this.rowCopy?.resource_location
      : find(this.options, item => item.uuid === this.formGroup.value.host)[
          'label'
        ];
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resourceData.environment_uuid
          : this.formGroup.value.host,
      restoreType:
        this.restoreType === RestoreType.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreTo,
      extendInfo: {}
    };

    if (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW) {
      set(params, 'targetObject', this.formGroup.value.instance);
      if (!this.isSupport) {
        set(params, 'extendInfo.newDatabasePath', this.formGroup.value.path);
      }
    }

    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      set(params, 'targetObject', this.rowCopy.resource_id);
    }

    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      set(
        params,
        'extendInfo.restoreTimestamp',
        this.rowCopy.restoreTimeStamp || ''
      );
    }

    return params;
  }

  isSupportFunc(agent) {
    const params = {
      hostUuidsAndIps: isArray(agent) ? agent : [agent],
      applicationType: 'SQLServer',
      scene: Scene.Restore,
      buttonNames: ['newDatabasePath']
    };
    this.clientManagerApiService
      .queryAgentApplicationUsingPOST({
        AgentCheckSupportParam: params,
        akOperationTips: false
      })
      .subscribe(res => {
        this.isSupport = res?.newDatabasePath;
        if (res?.newDatabasePath) {
          this.formGroup.get('path').clearValidators();
        } else {
          this.formGroup
            .get('path')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.validPath()
            ]);
        }
        this.formGroup.get('path').updateValueAndValidity();
      });
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return { required: { value: control.value } };
      }

      if (!CommonConsts.REGEX.windowsPath.test(control.value)) {
        return { pathError: { value: control.value } };
      }

      const path = split(control.value, '\\');

      if (find(path, item => !trim(item))) {
        return { pathError: { value: control.value } };
      }

      if (!/[a-zA-Z]:/.test(first(path))) {
        return { pathError: { value: control.value } };
      }

      path.shift();
      let vaild = true;
      each(path, item => {
        if (!/[\w\u4e00-\u9fa5\s]+/.test(item)) {
          vaild = false;
        }
      });

      if (!vaild) {
        return { pathError: { value: control.value } };
      }

      return null;
    };
  }
  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.restoreV2Service
        .CreateRestoreTask({
          CreateRestoreTaskRequestBody: this.getParams() as any
        })
        .subscribe({
          next: res => {
            observer.next();
            observer.complete();
          },
          error: err => {
            observer.error(err);
            observer.complete();
          }
        });
    });
  }

  ngOnInit() {
    this.disableOriginLocation =
      this.rowCopy?.resource_status === 'NOT_EXIST' ||
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.reverseReplication.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy?.generated_by
      );

    this.getResourceData();
    this.initForm();
    if (this.resourceData.environment_is_cluster === 'False') {
      this.getClusters();
    } else {
      this.getClusters();
    }
    this.disableOkBtn();
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('host').setValue(config.targetEnv);
    }
  }

  updateTable(event?) {
    this.getClusters(null, null, event);
  }
}
