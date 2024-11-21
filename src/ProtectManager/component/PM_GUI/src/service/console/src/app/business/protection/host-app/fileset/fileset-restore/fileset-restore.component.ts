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
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMap,
  I18NService,
  isRBACDPAdmin,
  RestoreFileType,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type,
  VmFileReplaceStrategy
} from 'app/shared';
import {
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  assign,
  cloneDeep,
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
  last,
  map,
  reject,
  replace,
  set,
  size,
  split,
  startsWith,
  toNumber
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-fileset-restore',
  templateUrl: './fileset-restore.component.html',
  styleUrls: ['./fileset-restore.component.less']
})
export class FilesetRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = VmFileReplaceStrategy;
  hostOptions = [];
  metadataPathData = [];
  hostUuid: any;
  fileValid$ = new Subject<boolean>();
  originalHost;
  newHost;
  osType;
  dataMap = DataMap;
  RestoreType = RestoreType;
  isWindows;
  isVolume;
  isOsBackup = false; // 用于判断副本是否使用操作系统备份
  authHosts = [];
  tapeCopy = false;
  isDataProtectionAdmin = isRBACDPAdmin(this.cookieService.role);
  scriptPlaceholder = this.i18n.get('common_script_linux_placeholder_label');
  scriptTips = this.i18n.get('protection_fileset_restore_advance_params_label');
  disableOriginLocation = false;
  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  channelsErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 40])
  };
  isIncremental: boolean;
  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private cookieService: CookieService
  ) {}

  ngOnInit() {
    // 判断副本是否是磁带归档，且已开启索引
    this.tapeCopy =
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.tapeArchival.value &&
      this.rowCopy?.indexed === DataMap.CopyData_fileIndex.indexed.value;
    this.isVolume =
      this.rowCopy.resource_sub_type === DataMap.Resource_Type.volume.value;
    this.disableOriginLocation =
      this.rowCopy?.resource_status ===
        DataMap.Resource_Status.notExist.value ||
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.cascadedReplication.value ||
      this.isVolume;
    this.osType = JSON.parse(this.rowCopy.resource_properties)[
      'environment_os_type'
    ];
    this.isOsBackup =
      this.osType === DataMap.Fileset_Template_Os_Type.linux.value &&
      !this.isVolume &&
      JSON.parse(this.rowCopy.resource_properties)?.extendInfo?.is_OS_backup ===
        'true';
    this.isWindows =
      this.osType === DataMap.Fileset_Template_Os_Type.windows.value;
    this.initForm();
    this.getHosts();
    this.scriptPlaceholder = this.isWindows
      ? this.i18n.get('protection_fileset_advance_script_windows_label')
      : this.i18n.get('protection_fileset_advance_script_linux_label');
    this.scriptTips = this.isWindows
      ? this.i18n.get('protection_fileset_restore_script_windows_tips_label')
      : this.i18n.get('protection_fileset_restore_script_linux_tips_label');
    this.isIncremental =
      this.restoreType === RestoreType.CommonRestore &&
      includes(
        [
          DataMap.CopyData_Backup_Type.incremental.value,
          DataMap.CopyData_Backup_Type.permanent.value
        ],
        this.rowCopy.source_copy_type
      ) &&
      includes(
        [
          DataMap.CopyData_generatedType.backup.value,
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value,
          DataMap.CopyData_generatedType.reverseReplication.value
        ],
        this.rowCopy.generated_by
      );
  }

  initForm() {
    const resource = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};

    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      isDirectRecovery: new FormControl(this.tapeCopy),
      originLocation: new FormControl({
        value: resource?.environment_name,
        disabled: true
      }),
      host: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      metadataPath: new FormControl(
        { value: [], disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      overwriteType: new FormControl(VmFileReplaceStrategy.Overwriting, {
        validators: this.baseUtilService.VALID.required()
      }),
      incrementalRestore: new FormControl(false),
      channels: new FormControl(1, {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 40)
        ]
      }),
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
      }),
      is_OS_restore: new FormControl(false),
      reboot_system_after_restore: new FormControl(false)
    });

    this.listenForm();
    this.modal.getInstance().lvOkDisabled = false;
    if (this.disableOriginLocation) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
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

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      this.formGroup.get('host').setValue('');

      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('host').disable();
        this.formGroup.get('metadataPath').disable();
      } else {
        this.formGroup.get('host').enable();
        this.formGroup.get('metadataPath').enable();
        this.metadataPathData = [];
      }
    });

    this.formGroup.get('host').valueChanges.subscribe(res => {
      this.formGroup.get('metadataPath').setValue([]);

      each(this.hostOptions, item => {
        if (item.uuid === res) {
          this.newHost = item.environment.uuid;
        }
      });
      if (res === '') {
        return;
      }

      const selectHost = cloneDeep(find(this.hostOptions, { uuid: res }));
      assign(selectHost, {
        label: selectHost?.environment?.name,
        children: [],
        isLeaf: false
      });

      if (this.osType === DataMap.Os_Type.windows.value) {
        assign(selectHost, {
          disabled: true
        });
      } else {
        if (this.isDataProtectionAdmin) {
          const isAuthHost = !find(this.authHosts, { key: res })?.value;
          assign(selectHost, {
            disabled: isAuthHost
          });
        } else {
          assign(selectHost, {
            disabled: false
          });
        }
      }

      this.metadataPathData = [selectHost];
      this.hostUuid = selectHost?.environment?.uuid;
    });

    this.formGroup.get('is_OS_restore').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('reboot_system_after_restore').setValue(false);
        if (
          this.formGroup.get('restoreLocation').value ===
          RestoreV2LocationType.NEW
        ) {
          this.formGroup.get('metadataPath').enable();
        }
      } else {
        this.formGroup.get('metadataPath').disable();
      }
    });
  }

  getHosts(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: 20,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          subType: this.isVolume ? ['VolumePlugin'] : ['FilesetPlugin']
        })
      })
      .subscribe(res => {
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
          const hostArr = [];
          each(recordsTemp, item => {
            if (
              item.environment?.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
            ) {
              hostArr.push({
                ...item,
                key: item.uuid,
                value: item.uuid,
                label: !isEmpty(item.environment?.endpoint)
                  ? `${item.environment?.name}(${item.environment?.endpoint})`
                  : item.environment?.name,
                os_type: item.environment?.osType,
                parentUuid: item.parentUuid,
                isLeaf: true
              });
            }

            if (
              this.rowCopy.resource_environment_ip ===
              item.environment?.endpoint
            ) {
              this.originalHost = item.environment.uuid;
            }
          });
          this.hostOptions = filter(
            hostArr,
            item => item.os_type === this.osType
          );

          // 当角色为数据保护管理员时需要判断是否授权
          if (this.isDataProtectionAdmin) {
            this.authHosts = map(hostArr, item => {
              return {
                key: item.uuid,
                value:
                  item?.userId ===
                  get(item.environment.extendInfo, 'register_user_id', '')
              };
            });
          }
          return;
        }
        this.getHosts(recordsTemp, startPage);
      });
  }

  expandedChange(node) {
    if (!node.expanded) {
      return;
    }
    node.children = [];
    this.getHostResource(node);
  }

  getHostResource(node, startPage?: number) {
    const params = {
      envId: this.hostUuid,
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: 100,
      parentId: node.extendInfo?.path || this.getNullPath(),
      resourceType: DataMap.Resource_Type.fileset.value
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        const hostId = this.formGroup.getRawValue().host;
        each(res.records, item => {
          const pathArr = this.isWindows
            ? split(item.extendInfo?.path, '\\')
            : split(item.extendInfo?.path, '/');
          const pathLabel =
            this.isWindows && size(pathArr) === 2 && !last(pathArr)
              ? item.extendInfo?.path
              : this.isWindows
              ? replace(last(pathArr), '\\', '')
              : replace(last(pathArr), '/', '');
          const isRestoreFile = item.extendInfo?.type === RestoreFileType.File;
          let isAuthHost = false;
          if (this.isDataProtectionAdmin) {
            isAuthHost = !find(this.authHosts, { key: hostId })?.value;
          }
          const isNotMntPath = !startsWith(item.extendInfo?.path, '/mnt');
          const isDisabled =
            isRestoreFile ||
            (isAuthHost &&
              isNotMntPath &&
              this.osType === DataMap.Fileset_Template_Os_Type.linux.value);
          assign(item, {
            label: pathLabel,
            isLeaf: item.extendInfo?.hasChildren === 'true' ? false : true,
            disabled: isDisabled
          });
        });
        if (isArray(node.children) && !isEmpty(node.children)) {
          node.children = [
            ...reject(node.children, n => {
              return n.isMoreBtn;
            }),
            ...res.records
          ];
        } else {
          node.children = [...res.records];
        }
        if (res.totalCount > size(node.children)) {
          const moreClickNode = {
            label: `${this.i18n.get('common_more_label')}...`,
            isMoreBtn: true,
            isLeaf: true,
            disabled: true,
            startPage: Math.floor(size(node.children) / 200) + 1
          };
          node.children = [...node.children, moreClickNode];
        }
        this.metadataPathData = [...this.metadataPathData];
      });
  }

  getParams() {
    const tempPath: any = first(this.formGroup.value.metadataPath) || {};
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.originalHost
          : this.newHost,
      restoreType:
        this.restoreType === RestoreType.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      filters: [],
      agents: [],
      extendInfo: {
        restoreOption: this.formGroup.value.overwriteType,
        channels: toNumber(this.formGroup.value.channels),
        is_OS_restore: this.formGroup.get('is_OS_restore').value,
        reboot_system_after_restore: this.formGroup.get(
          'reboot_system_after_restore'
        ).value
      },
      scripts: {
        preScript: this.formGroup.value.preScript,
        postScript: this.formGroup.value.postScript,
        failPostScript: this.formGroup.value.executeScript
      }
    };
    if (this.isIncremental) {
      assign(params.extendInfo, {
        isAccumulate: this.formGroup.get('incrementalRestore').value
          ? 'true'
          : 'false'
      });
    }
    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW) {
      set(
        params,
        'targetObject',
        isEmpty(tempPath)
          ? this.getNullPath()
          : tempPath.extendInfo.path
          ? tempPath.extendInfo.path
          : this.getNullPath()
      );
    }
    if (this.tapeCopy && this.formGroup.get('isDirectRecovery')?.value) {
      assign(params, {
        restoreType: RestoreV2Type.FileRestore,
        subObjects: ['/']
      });
    }
    return params;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? {
              name: this.formGroup.value.originLocation,
              value: this.originalHost
            }
          : assign(
              {},
              find(this.hostOptions, {
                value: this.formGroup.value.host
              }),
              {
                name: find(this.hostOptions, {
                  value: this.formGroup.value.host
                })?.label
              }
            ),
      restoreLocation: this.formGroup.value.restoreLocation,
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    const tempPath: any = first(this.formGroup.value.metadataPath) || {};
    const resource = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};

    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? resource?.environment_name
      : `${
          find(this.hostOptions, {
            value: this.formGroup.value.host
          })['label']
        }: ${
          isEmpty(tempPath)
            ? this.getNullPath()
            : tempPath.extendInfo.path
            ? tempPath.extendInfo.path
            : this.getNullPath()
        }`;
  }

  getNullPath() {
    return this.isWindows ? '' : '/';
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
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

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }
}
