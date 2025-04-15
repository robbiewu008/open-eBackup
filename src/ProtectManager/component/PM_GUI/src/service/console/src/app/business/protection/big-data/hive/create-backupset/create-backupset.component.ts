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
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CatalogName,
  CommonConsts,
  DataMap,
  I18NService,
  MODAL_COMMON,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  get,
  has as _has,
  isArray,
  isEmpty,
  isNumber,
  map,
  reject,
  replace,
  size,
  split,
  startsWith
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { SelectTableComponent } from './select-table/select-table.component';

@Component({
  selector: 'aui-create-backupset',
  templateUrl: './create-backupset.component.html',
  styleUrls: ['./create-backupset.component.less']
})
export class CreateBackupsetComponent implements OnInit {
  formGroup: FormGroup;
  clusterOptions = [];
  databaseOptions = [];
  metadataPathData = [];
  databaseData = [];
  clusterUuid = '';
  selectedDatabaseTables = [];
  isAutoProtectDisabled = true;
  protectDatabaseHelp = this.i18n.get(
    'protection_auto_protect_table_help_label'
  );

  @Input('data') data;
  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getClusters();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      cluster: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      database: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      autoProtect: new FormControl(true),
      metadataPath: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
    this.listenForm();

    if (this.data) {
      this.formGroup.patchValue({
        name: this.data.name,
        cluster: this.data.rootUuid
      });
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      if (res === '') {
        return;
      }
      this.getDatabase(res);
      const selectCluster = cloneDeep(find(this.clusterOptions, { uuid: res }));
      if (selectCluster) {
        assign(selectCluster, {
          label: selectCluster?.name,
          children: [],
          isLeaf: false
        });
        this.metadataPathData = [selectCluster];
        this.clusterUuid = selectCluster.uuid;
      }
    });

    this.formGroup.get('database').valueChanges.subscribe(res => {
      this.databaseData = filter(this.databaseOptions, item => {
        return item.value === res;
      });
      if (
        _has(first(this.databaseData), ['extendInfo', 'auto-protection']) &&
        first(this.databaseData)['extendInfo']['auto-protection'] === 'false'
      ) {
        this.isAutoProtectDisabled = true;
        this.formGroup.get('autoProtect').setValue(false);
        this.formGroup.get('autoProtect').disable();
      } else {
        this.isAutoProtectDisabled = false;
        this.formGroup.get('autoProtect').setValue(true);
        this.formGroup.get('autoProtect').enable();
      }

      this.selectedDatabaseTables = [];
    });
  }

  getClusters(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.Hive.value
      })
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
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE_MAX) ||
        res.totalCount === 0
      ) {
        const clusterArray = [];
        each(recordsTemp, item => {
          clusterArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.clusterOptions = clusterArray;
        if (this.data) {
          const selectCluster = cloneDeep(
            find(this.clusterOptions, {
              uuid: this.data.rootUuid
            })
          );
          assign(selectCluster, {
            label: selectCluster?.name,
            children: [],
            isLeaf: false
          });
          this.metadataPathData = [selectCluster];
          this.clusterUuid = this.data.rootUuid;

          if (this.data.extendInfo.tempPath !== '/') {
            this.getClusterResourceByPath(
              selectCluster,
              split(this.data.extendInfo.tempPath, '/')
            );
          } else {
            this.formGroup.get('metadataPath').setValue([selectCluster]);
          }
        }
        return;
      }
      this.getClusters(recordsTemp, startPage);
    });
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.invalid ||
      (!this.formGroup.value.autoProtect && !size(this.selectedDatabaseTables));
  }

  getDatabase(clusterId, recordsTemp?, startPage?) {
    if (!clusterId) {
      return;
    }

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      envId: clusterId,
      parentId: this.data?.uuid || '',
      resourceType: 'HiveDataBase'
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START + 1;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage ===
            Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE_MAX) + 1 ||
          res.totalCount === 0
        ) {
          const clusterArray = [];
          each(recordsTemp, item => {
            clusterArray.push({
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });
          this.databaseOptions = clusterArray;
          if (
            this.data &&
            clusterId === this.data.rootUuid &&
            !this.formGroup.get('database').value
          ) {
            this.formGroup.get('database').setValue(
              get(
                find(
                  this.databaseOptions,
                  item => item.name === this.data.extendInfo.databaseName
                ),
                'uuid'
              )
            );

            if (this.data.extendInfo.tableList) {
              each(split(this.data.extendInfo.tableList, ','), val => {
                this.selectedDatabaseTables.push({
                  name: val,
                  extendInfo: {
                    isLocked: 'false'
                  }
                });
              });
            }

            this.data.extendInfo.tableList
              ? this.formGroup.get('autoProtect').setValue(false)
              : this.formGroup.get('autoProtect').setValue(true);
          } else if (!this.data) {
            this.formGroup.get('database').setValue('');
          }
          return;
        }
        this.getDatabase(clusterId, recordsTemp, startPage);
      });
  }

  selectTable(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('protection_select_table_label'),
      lvContent: SelectTableComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.largeWidth - 40,
      lvComponentParams: {
        data: assign({ clusterId: this.formGroup.value.cluster }, data),
        selectedTableData: this.selectedDatabaseTables,
        modifiedTables: this.data
          ? split(this.data.extendInfo.tableList, ',')
          : []
      },
      lvOk: modal => {
        const content = modal.getContentComponent() as SelectTableComponent;
        this.selectedDatabaseTables = content.onOK();
        this.disableOkBtn();
      }
    });
  }

  expandedChange(node) {
    if (!node.expanded) {
      return;
    }
    this.getClusterResource(node);
  }

  getClusterResource(node, startPage?: number) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      envId: this.clusterUuid,
      parentId: node.extendInfo.path || '/',
      resourceType: 'HiveDirSet'
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        each(res.records, item => {
          assign(item, {
            rootPath: node.rootPath
              ? `${node.rootPath}/${item.extendInfo?.path}`
              : `/${item.extendInfo?.path}`,
            label: item.name,
            isLeaf: false
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
            startPage:
              Math.floor(size(node.children) / CommonConsts.PAGE_SIZE_MAX) + 1
          };
          node.children = [...node.children, moreClickNode];
        }
        this.metadataPathData = [...this.metadataPathData];
      });
  }

  getClusterResourceByPath(node, path, startPage?: number) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      envId: this.clusterUuid,
      parentId: node.extendInfo.path || '/',
      resourceType: 'HiveDirSet'
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        each(res.records, item => {
          assign(item, {
            rootPath: node.rootPath
              ? `${node.rootPath}/${item.extendInfo?.path}`
              : `/${item.extendInfo?.path}`,
            label: item.name,
            isLeaf: false
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
            startPage:
              Math.floor(size(node.children) / CommonConsts.PAGE_SIZE_MAX) + 1
          };
          node.children = [...node.children, moreClickNode];
        }
        this.metadataPathData = [...this.metadataPathData];

        path.shift();
        if (path.length > 1) {
          this.getClusterResourceByPath(
            find(node.children, item => item.name === first(path)),
            path
          );
        } else {
          this.formGroup
            .get('metadataPath')
            .setValue([find(node.children, item => item.name === first(path))]);
        }
      });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const tempPath: any = first(this.formGroup.value.metadataPath) || {};
      const params = {
        name: this.formGroup.value.name,
        parentUuid: this.formGroup.value.cluster,
        subType: DataMap.Resource_Type.HiveBackupSet.value,
        type: DataMap.Resource_Type.Hive.value,
        path: find(this.clusterOptions, {
          key: this.formGroup.value.cluster
        })?.name,
        extendInfo: {
          databaseName: find(this.databaseOptions, {
            value: this.formGroup.value.database
          })?.name,
          tempPath: tempPath.extendInfo.path || '/',
          tableList: this.formGroup.value.autoProtect
            ? ''
            : map(this.selectedDatabaseTables, 'name').toString()
        }
      };
      if (this.data) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.data.uuid,
            UpdateResourceRequestBody: params
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
      } else {
        this.protectedResourceApiService
          .CreateResource({ CreateResourceRequestBody: params })
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
    });
  }
}
