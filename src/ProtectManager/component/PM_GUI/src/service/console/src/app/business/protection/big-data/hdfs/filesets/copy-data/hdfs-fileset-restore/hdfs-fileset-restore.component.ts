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
import { ModalRef } from '@iux/live';
import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  HdfsFilesetReplaceOptions,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType,
  ProtectedEnvironmentApiService,
  I18NService,
  MODAL_COMMON
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  each,
  isNumber,
  omit,
  find,
  size,
  isEmpty,
  includes,
  isArray,
  reject,
  filter,
  unionBy,
  map,
  endsWith,
  toString,
  assign,
  cloneDeep
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-hdfs-fileset-restore',
  templateUrl: './hdfs-fileset-restore.component.html',
  styleUrls: ['./hdfs-fileset-restore.component.less']
})
export class HdfsFilesetRestoreComponent implements OnInit {
  resourceProp;
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  @Input() targetParams;
  @Output() restoreParamsChange = new EventEmitter();

  copySelectedPaths;
  restoreToNewLocationOnly;
  fileValid$ = new Subject<boolean>();
  hostOptions = [];
  filesData = [];
  fileSelection = [];
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  filesetReplaceOptions = HdfsFilesetReplaceOptions;

  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  filesetNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  capacityThresholdToolTip = this.i18n.get(
    'protection_bigdata_capacity_threshold_tip_label'
  );
  capacityThresholdErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 100])
  };
  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    public dataMapService: DataMapService,
    private restoreV2Service: RestoreApiV2Service,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getClusters();
    this.updateForm();
    this.updateData();
  }

  updateForm() {
    setTimeout(() => {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    }, 0);
  }

  initForm() {
    this.resourceProp = JSON.parse(this.rowCopy.resource_properties);
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(this.restoreLocationType.ORIGIN),
      location: new FormControl(this.resourceProp?.environment_uuid),
      new_fileset_name: new FormControl(''),
      originalType: new FormControl(this.filesetReplaceOptions.Skip),
      available_capacity_threshold: new FormControl(20, {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 100)
        ]
      })
    });

    this.listenFormGroup();
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) || this.rowCopy.is_replicated;
  }

  updateData() {
    if (this.restoreToNewLocationOnly) {
      setTimeout(() => {
        this.formGroup.get('restoreTo').setValue(this.restoreLocationType.NEW);
      }, 100);
    }
  }

  listenFormGroup() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      this.getClusters(res);
      if (res === this.restoreLocationType.ORIGIN) {
        this.formGroup.get('location').clearValidators();
        this.formGroup.get('new_fileset_name').clearValidators();
        this.fileValid$.next(true);
      } else if (res === this.restoreLocationType.NEW) {
        this.formGroup
          .get('location')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('new_fileset_name').clearValidators();
        this.fileValid$.next(
          size(this.fileSelection) > 0 &&
            size(this.getPath(this.fileSelection)) <= 64
        );
      } else if (res === this.restoreLocationType.NATIVE) {
        this.formGroup
          .get('location')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('new_fileset_name')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('path').clearValidators();
        this.fileValid$.next(true);
      }
      this.formGroup.get('location').updateValueAndValidity();
      this.formGroup.get('new_fileset_name').updateValueAndValidity();
    });

    this.formGroup.statusChanges.subscribe(res => {
      this.restoreParamsChange.emit(res === 'VALID');
    });

    this.patchValue();
  }

  getClusters(
    restoreTo = this.restoreLocationType.ORIGIN,
    recordsTemp?,
    startPage?
  ) {
    this.formGroup.get('location').setValue('');
    if (restoreTo === this.restoreLocationType.ORIGIN) {
      this.hostOptions = [
        {
          key: this.resourceProp?.environment_uuid,
          label: this.resourceProp?.environment_name,
          isLeaf: true
        }
      ];
      this.formGroup
        .get('location')
        .setValue(this.resourceProp?.environment_uuid);
      return;
    }

    this.protectedResourceApiService
      .ListResources({
        pageSize: 20,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          subType: DataMap.Resource_Type.HDFS.value
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
              item.linkStatus ===
              this.dataMapService.getConfig('resource_LinkStatus_Special')
                .normal.value
            ) {
              hostArr.push({
                ...item,
                key: item.uuid,
                label: item.name,
                isLeaf: true
              });
            }
          });
          this.hostOptions = hostArr;
          return;
        }
        this.getClusters(restoreTo, recordsTemp, startPage);
      });
  }

  hostChange(v) {
    if (!v) {
      return;
    }
    this.fileSelection = [];
    this.fileValid$.next(false);
    this.filesData = [
      {
        label: find(this.hostOptions, {
          key: this.formGroup.value.location
        }).label,
        uuid: this.formGroup.value.location,
        contentToggleIcon: 'aui-icon-cluster',
        children: [],
        extendInfo: {
          path: '/'
        },
        isHost: true
      }
    ];
  }

  expandedChange(node) {
    if (!node.expanded || !!size(node.children)) {
      return;
    }

    this.getResource(node);
  }

  getResource(node, startPage?) {
    this.protectedEnvironmentApiService
      .ListEnvironmentResource({
        envId: this.formGroup.value.location,
        pageNo: startPage || CommonConsts.PAGE_START + 1,
        pageSize: 100,
        parentId: !node.extendInfo?.path ? '/' : node.extendInfo?.path || '',
        resourceType: DataMap.Resource_Type.HDFS.value
      })
      .subscribe(res => {
        map(res.records, (item: any) => {
          item.label =
            item.extendInfo?.fileName === '/'
              ? item.extendInfo?.fileName.replace('/', '')
              : item.extendInfo?.fileName.replace(
                  item.extendInfo?.fileName + '/',
                  ''
                );
          item.isLeaf = item.extendInfo?.isDirectory !== 'true';
          item.disabled = item.extendInfo?.isDirectory !== 'true';
          item.contentToggleIcon =
            item.extendInfo?.isDirectory === 'true'
              ? 'aui-icon-directory'
              : 'aui-icon-file';
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
            startPage: Math.floor(size(node.children) / 100) + 1
          };
          node.children = [...node.children, moreClickNode];
        }
        this.filesData = [...this.filesData];
        if (find(this.fileSelection, node)) {
          this.fileSelection = [...this.fileSelection, ...res.records];
        }
        if (node.isHost && this.handlerNodeCheck(node)) {
          this.fileSelection = [...this.fileSelection, node];
        }
        this.fileSelection = [...this.fileSelection];
      });
  }

  handlerNodeCheck(node) {
    let childrenCheckedCount = 0;
    each(node.children, item => {
      if (
        find(this.fileSelection, selection => {
          return selection.extendInfo.path === item.extendInfo.path;
        })
      ) {
        childrenCheckedCount++;
      }
    });
    return childrenCheckedCount === size(node.children);
  }

  selectionChange() {
    this.fileValid$.next(
      size(this.fileSelection) > 0 &&
        size(this.getPath(this.fileSelection)) <= 64
    );
  }

  getPath(paths) {
    let filterPaths = [];
    let childPaths = [];
    paths = filter(paths, item => {
      return !isEmpty(item.extendInfo?.path);
    });
    each(paths, item => {
      if (!!size(item.children)) {
        childPaths = unionBy(childPaths, item.children, 'extendInfo.path');
      }
    });
    filterPaths = reject(paths, item => {
      return !isEmpty(
        find(childPaths, node => {
          return node.extendInfo?.path === item.extendInfo?.path;
        })
      );
    });
    return map(filterPaths, 'extendInfo.path');
  }

  nodeCheck(event) {
    this.copySelectedPaths = reject(this.copySelectedPaths, v => {
      return includes(
        v,
        endsWith(event.node.path, '/') ? event.node.path : event.node.path + '/'
      );
    });
  }

  patchValue() {
    if (this.targetParams) {
      this.formGroup.patchValue(
        omit(this.targetParams, ['resource', 'requestParams'])
      );
      if (this.targetParams.restoreTo === this.restoreLocationType.NEW) {
        setTimeout(() => {
          this.hostChange(this.targetParams.location);
        });
      }
    }
  }

  getTargetParams() {
    const targetObj =
      find(this.hostOptions, {
        key: this.formGroup.value.location
      }) || {};
    const params = {
      copyId: this.rowCopy.uuid,
      restoreType: this.restoreType,
      targetLocation: this.formGroup.value.restoreTo,
      targetEnv: this.formGroup.value.location,
      extendInfo: {
        restoreOption: this.formGroup.value.originalType
      }
    };
    if (this.formGroup.value.restoreTo === this.restoreLocationType.NEW) {
      assign(params, {
        targetObject: toString(this.getPath(this.fileSelection))
      });
    }
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreTo === this.restoreLocationType.NEW
          ? assign(targetObj, {
              name: `${targetObj.name}${toString(
                this.getPath(this.fileSelection)
              )}`
            })
          : assign({}, this.resourceProp, {
              name: this.resourceProp.environment_name
            }),
      requestParams: params
    };
  }

  private showTips(params, observer: Observer<void>) {
    let tips = this.i18n.get(
      'protection_hbase_restore_no_backup_task_tips_label'
    );
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'hdfs-restore-tips-info',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-danger-48',
        lvHeader: this.i18n.get(
          'protection_hbase_restore_no_backup_task_header_label'
        ),
        lvContent: tips,
        lvWidth: 500,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: false,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvOk: () => this.preRestoreTask(params, observer),
        lvCancel: () => {
          observer.error(null);
          observer.complete();
        },
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            observer.error(null);
            observer.complete();
          }
        }
      }
    });
  }

  getTargetPath() {
    const targetObj =
      find(this.hostOptions, {
        key: this.formGroup.value.location
      }) || {};

    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? this.resourceProp.environment_name
      : `${targetObj.name}${toString(this.getPath(this.fileSelection))}`;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const params = {
        copyId: this.rowCopy.uuid,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        targetEnv: this.formGroup.value.location,
        extendInfo: {
          restoreOption: this.formGroup.value.originalType,
          available_capacity_threshold: Number(
            this.formGroup.value.available_capacity_threshold
          )
        }
      } as any;
      if (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW) {
        this.showTips(params, observer);
      } else {
        this.preRestoreTask(params, observer);
      }
    });
  }

  private preRestoreTask(params, observer: Observer<void>) {
    this.restoreV2Service
      .CreateRestoreTask({
        CreateRestoreTaskRequestBody:
          params.targetLocation === this.restoreLocationType.ORIGIN
            ? params
            : {
                ...params,
                targetObject: toString(this.getPath(this.fileSelection))
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
  }
}
