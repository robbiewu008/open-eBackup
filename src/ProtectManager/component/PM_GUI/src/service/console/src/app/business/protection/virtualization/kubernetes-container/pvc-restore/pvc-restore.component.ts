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
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  FileReplaceStrategy,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType
} from 'app/shared';
import { TableConfig, TableData } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  defer,
  differenceBy,
  each,
  every,
  filter,
  first,
  get,
  includes,
  indexOf,
  isArray,
  isEmpty,
  map,
  reject,
  size
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-pvc-restore',
  templateUrl: './pvc-restore.component.html',
  styleUrls: ['./pvc-restore.component.less']
})
export class PvcRestoreComponent implements OnInit {
  rowCopy;
  childResType;
  restoreType;

  allTableData: TableData;
  allTableConfig: TableConfig;
  selectionPvc = [];
  recoveyTableData: TableData;
  recoveyTableConfig: TableConfig;

  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = FileReplaceStrategy;
  restoreToNewLocationOnly = false;
  resource;

  namespaceTreeData = [];
  targetPvcOptions = [];
  cacheSelectedPvc = [];

  pvc$ = new Subject<boolean>();

  @ViewChild('targetPvcTpl', { static: true })
  targetPvcTpl: TemplateRef<any>;
  @ViewChild('targetPvcHelpTpl', { static: true })
  targetPvcHelpTpl: TemplateRef<any>;
  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.getResource();
    this.initTableConfig();
    this.initForm();
  }

  getResource() {
    this.resource = JSON.parse(this.rowCopy?.resource_properties || '{}');
    this.restoreToNewLocationOnly = includes(
      [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ],
      this.rowCopy.generated_by
    );
    const properties = JSON.parse(this.rowCopy?.properties || '{}');
    const pvcs = !isEmpty(properties.extendInfo?.backup_pvcs)
      ? properties.extendInfo?.backup_pvcs
      : properties.extendInfo?.extendInfo?.backup_pvcs;
    // 升级适配 低版本没有size会显示异常
    if (!isEmpty(pvcs)) {
      this.allTableData = {
        data: isArray(pvcs)
          ? map(pvcs, item => {
              return { name: item };
            })
          : map(pvcs, (value, key) => {
              return { name: key, size: value };
            }),
        total: size(pvcs)
      };
    }
  }

  initTableConfig() {
    const pagination: any = {
      pageSize: CommonConsts.PAGE_SIZE_SMALL,
      winTablePagination: true,
      mode: 'simple',
      showPageSizeOptions: false,
      pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS
    };
    this.allTableConfig = {
      table: {
        async: false,
        compareWith: 'name',
        columns: [
          { key: 'name', name: this.i18n.get('common_name_label') },
          { key: 'size', name: this.i18n.get('common_size_label') }
        ],
        colDisplayControl: false,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: selection => {
          const canceledDisk = differenceBy(
            this.recoveyTableData?.data,
            selection,
            'name'
          );
          this.cacheSelectedPvc = reject(this.cacheSelectedPvc, item =>
            includes(map(canceledDisk, 'targetPvc'), item)
          );
          this.recoveyTableData = {
            data: selection,
            total: size(selection)
          };
          if (!isEmpty(this.targetPvcOptions)) {
            each(this.recoveyTableData.data, item => {
              assign(item, {
                targetPvcOptions: filter(
                  this.targetPvcOptions,
                  value => value.name === item.targetPvc || this.fiterPvc(value)
                )
              });
            });
          }
          this.setVaild();
        }
      },
      pagination: pagination
    };
    this.recoveyTableConfig = {
      table: {
        async: false,
        compareWith: 'name',
        columns: [
          {
            key: 'name',
            name: this.i18n.get('protection_recovery_pvc_name_label')
          },
          { key: 'size', name: this.i18n.get('common_size_label') },
          {
            key: 'target',
            name: this.i18n.get('protection_restore_target_label'),
            cellRender: this.targetPvcTpl,
            thExtra: this.targetPvcHelpTpl
          }
        ],
        colDisplayControl: false
      },
      pagination: pagination
    };
  }

  setVaild() {
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      this.pvc$.next(
        !isEmpty(this.recoveyTableData?.data) && this.formGroup.valid
      );
    } else {
      this.pvc$.next(
        !isEmpty(this.recoveyTableData?.data) &&
          this.formGroup.valid &&
          every(this.recoveyTableData?.data, item => {
            return !isEmpty(item.targetPvc);
          })
      );
    }
  }

  availableSize(recovey, target): boolean {
    const unit = ['Ki', 'Mi', 'Gi', 'Ti', 'Pi', 'Ei'];
    if (isEmpty(recovey.size) || isEmpty(target.size)) {
      return true;
    }
    const recoveryUnit = indexOf(unit, recovey.size?.slice(-2));
    const targetUnit = indexOf(unit, target.size?.slice(-2));
    if (recoveryUnit === -1 || targetUnit === -1) {
      return true;
    }
    return (
      parseFloat(target.size) * 1024 ** targetUnit >=
      parseFloat(recovey.size) * 1024 ** recoveryUnit
    );
  }

  getTargetPvc(namespace) {
    if (
      isEmpty(namespace) ||
      this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
    ) {
      return;
    }
    const extParams = {
      resourceType: '',
      envId: namespace.environment?.uuid,
      conditions: JSON.stringify({
        kind: 'PersistentVolumeClaim',
        namespace: namespace.name
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params =>
        this.protectedEnvironmentApiService.ListEnvironmentResource(params),
      resource => {
        this.targetPvcOptions = map(resource, item => {
          return assign(item, {
            label: item.name,
            value: item.name,
            size: item.extendInfo?.capacity,
            isLeaf: true
          });
        });

        this.cacheSelectedPvc = [];

        if (!isEmpty(this.recoveyTableData?.data)) {
          each(this.recoveyTableData?.data, item => {
            item.targetPvc = '';
            assign(item, {
              targetPvcOptions: filter(
                cloneDeep(this.targetPvcOptions),
                target => this.availableSize(item, target)
              )
            });
          });
        }

        this.setVaild();
      }
    );
  }

  pvcChange(_, pvc) {
    this.cacheSelectedPvc = reject(
      map(this.recoveyTableData?.data, 'targetPvc'),
      item => isEmpty(item)
    );
    each(this.recoveyTableData?.data, item => {
      if (item.name === pvc.name) {
        return;
      }
      item.targetPvcOptions = filter(
        this.targetPvcOptions,
        value => value.name === item.targetPvc || this.fiterPvc(value)
      );
      item.targetPvcOptions = filter(item.targetPvcOptions, target =>
        this.availableSize(item, target)
      );
    });
    this.setVaild();
  }

  fiterPvc(value) {
    return !includes(this.cacheSelectedPvc, value.name);
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      targetNamespace: new FormControl([]),
      overwriteType: new FormControl(FileReplaceStrategy.Replace)
    });

    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('targetNamespace').clearValidators();
      } else {
        this.getCluster();
        this.formGroup
          .get('targetNamespace')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('targetNamespace').updateValueAndValidity();
      this.setVaild();
    });

    this.formGroup
      .get('targetNamespace')
      .valueChanges.subscribe(res =>
        defer(() => this.getTargetPvc(first(res)))
      );

    if (this.restoreToNewLocationOnly) {
      defer(() =>
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW)
      );
    }
  }

  getOptions(subType, params?, node?) {
    const extParams = {
      conditions: JSON.stringify({
        subType: [subType],
        ...params
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        if (subType === DataMap.Resource_Type.kubernetesClusterCommon.value) {
          // 新位置恢复限制：相同大版本间的同类型集群
          const originCluster: any = filter(
            resource,
            item => item.uuid === this.resource.environment_uuid
          );
          const clusterType = get(
            originCluster[0]?.extendInfo,
            'clusterType',
            ''
          );
          const version = get(originCluster[0], 'version', '')
            .split('.')
            .slice(0, 2)
            .join('.');
          const arr = filter(
            resource,
            item =>
              item.extendInfo?.clusterType === clusterType &&
              item.version
                .split('.')
                .slice(0, 2)
                .join('.') === version
          );

          this.namespaceTreeData = map(arr, item => {
            return assign(item, {
              label: item.name,
              disabled: true,
              contentToggleIcon: 'aui-icon-dataCenter',
              children: [],
              isLeaf: false,
              expanded: false
            });
          });
        } else {
          each(resource, item => {
            node.children.push(
              assign(item, {
                label: item.name,
                contentToggleIcon: 'aui-icon-directory-namespace',
                isLeaf: true,
                expanded: false
              })
            );
          });
          this.namespaceTreeData = [...this.namespaceTreeData];
        }
      }
    );
  }

  getCluster() {
    if (!isEmpty(this.namespaceTreeData)) {
      return;
    }
    this.getOptions(DataMap.Resource_Type.kubernetesClusterCommon.value);
  }

  expandedChange(node) {
    if (!node.expanded || node.children?.length) {
      return;
    }
    node.children = [];
    this.getOptions(
      DataMap.Resource_Type.kubernetesNamespaceCommon.value,
      { parentUuid: node.uuid },
      node
    );
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? this.resource.path
      : this.formGroup.value.targetNamespace[0]?.path;
  }

  getParams() {
    const params = {
      copyId: this.rowCopy?.uuid,
      restoreType: this.restoreType,
      targetEnv:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resource?.environment_uuid
          : this.formGroup.value.targetNamespace[0].environment?.uuid,
      targetLocation: this.formGroup.value.restoreTo,
      targetObject:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resource.name
          : this.formGroup.value.targetNamespace[0].uuid,
      extendInfo: {
        restoreOption: this.formGroup.value.overwriteType
      },
      subObjects: map(this.recoveyTableData?.data, item => {
        return JSON.stringify({
          name: item.name,
          extendInfo: {
            pvc:
              this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
                ? item.name
                : item.targetPvc
          }
        });
      })
    };
    return params;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
        .subscribe(
          () => {
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
}
