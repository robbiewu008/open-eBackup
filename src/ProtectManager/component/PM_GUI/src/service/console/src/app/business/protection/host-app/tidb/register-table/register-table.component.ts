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
import { ChangeDetectorRef, Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import { each, filter, find, get, isNumber, map, reject, size } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-table',
  templateUrl: './register-table.component.html',
  styleUrls: ['./register-table.component.less']
})
export class RegisterTableComponent implements OnInit {
  formGroup: FormGroup;
  databaseOptions = [];
  clusterOptions = [];
  tableData = [];
  selectedTableData = [];
  modifiedTables = [];
  totalTable;
  selectionTable = [];
  pageIndexS = CommonConsts.PAGE_START;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  dataMap = DataMap;
  rowData;
  colon = false;
  warningLabel = this.i18n.get('protection_tidb_table_warning_label', [0]);
  deleteNum = 0;

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  @ViewChild('pageS', { static: false }) pageS;
  @ViewChild('pageA', { static: false }) pageA;

  constructor(
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    if (!this.rowData) {
      this.getData();
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      tableName: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      database: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      cluster: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
    if (this.rowData) {
      this.formGroup.get('name').setValue(this.rowData.name);
      this.formGroup
        .get('tableName')
        .setValue(this.rowData.extendInfo.tableName);
      this.formGroup.get('database').setValue(this.rowData.parentUuid);
      this.formGroup.get('cluster').setValue(this.rowData.environment.uuid);
      this.getData();
    }
    this.watchForm();
  }

  watchForm() {
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      this.getDatabase(res);
      this.formGroup.get('database').setValue('');
      this.selectedTableData = [];
      this.tableData = [];
      this.formGroup.get('tableName').setValue(this.selectedTableData);
    });
    this.formGroup.get('database').valueChanges.subscribe(res => {
      this.getTable(res);
      this.selectedTableData = [];
      this.tableData = [];
      this.selectionTable = [];
      this.formGroup.get('tableName').setValue(this.selectedTableData);
    });
  }

  getData(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || this.pageIndex,
      pageSize: this.pageSize,
      akloading: false,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.tidbCluster.value
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
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        const hostArray = [];
        each(res.records, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}`,
            isLeaf: true
          });
        });
        this.clusterOptions = hostArray;
        if (this.rowData) {
          this.getDatabase(this.formGroup.value.cluster);
        }
        return;
      }
      this.getData(recordsTemp, startPage);
    });
  }

  getDatabase(res) {
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize,
      akloading: false,
      conditions: JSON.stringify({
        parentUuid: res,
        type: ResourceType.DATABASE,
        subType: DataMap.Resource_Type.tidbDatabase.value
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      const hostArray = [];
      each(res.records, item => {
        hostArray.push({
          ...item,
          key: item.uuid,
          value: item.uuid,
          label: `${item.name}`,
          isLeaf: true
        });
      });
      this.databaseOptions = hostArray;
      if (this.rowData) {
        this.getTable(this.formGroup.value.database);
      }
    });
  }

  getTable(res, startPage?) {
    if (!res) {
      return;
    }

    let tmp: any = filter(this.clusterOptions, item => {
      return item.uuid === this.formGroup.value.cluster;
    });
    let temp: any = filter(this.databaseOptions, item => {
      return item.uuid === res;
    });
    let test = {
      action_type: 'list_table',
      databaseName: temp[0].extendInfo.databaseName,
      clusterName: tmp[0].extendInfo.clusterName,
      tiupPath: tmp[0].extendInfo.tiupPath,
      isCluster: false,
      agentIds: [tmp[0].extendInfo.tiupUuid]
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource({
        envId: tmp[0].extendInfo.tiupUuid,
        pageSize: this.pageSize,
        pageNo: startPage || this.pageIndex,
        resourceSubType: this.dataMap.Resource_Type.tidbCluster.value,
        resourceType: this.dataMap.Resource_Type.tidbCluster.value,
        agentId: this.formGroup.value.cluster,
        conditions: JSON.stringify(test),
        parentId: temp[0].uuid
      })
      .subscribe(res => {
        const tableArray = [];
        each(res.records, item => {
          tableArray.push({
            name: item.name,
            extendInfo: { isLocked: item?.extendInfo?.isLocked }
          });
        });
        this.tableData = tableArray;
        this.totalTable = size(this.tableData);
        if (this.rowData) {
          this.modifiedTables = JSON.parse(this.rowData?.extendInfo?.tableName);
          each(this.modifiedTables, item => {
            this.selectedTableData.push({
              name: item
            });
          });
          this.formGroup.get('tableName').setValue(this.selectedTableData);
          each(this.tableData, item => {
            if (find(this.selectedTableData, val => val.name === item.name)) {
              delete item.extendInfo;
              this.selectionTable.push(item);
            }
          });
          each(this.selectedTableData, item => {
            if (!find(this.selectionTable, val => val.name === item.name)) {
              item.hasDelete = true;
            }
          });
          this.disableOkBtn();
          this.deleteNum =
            size(this.selectedTableData) - size(this.selectionTable);
          this.warningLabel = this.i18n.get(
            'protection_tidb_table_warning_label',
            [this.deleteNum]
          );
        }
        this.cdr.detectChanges();
      });
  }

  removeDelete() {
    this.selectionTable = [];
    this.selectedTableData = [];

    each(this.tableData, item => {
      if (find(this.modifiedTables, val => val === item.name)) {
        delete item.extendInfo;
        this.selectionTable.push(item);
      }
    });
    this.selectedTableData = [...this.selectionTable];
    this.deleteNum = size(this.selectedTableData) - size(this.selectionTable);
    this.warningLabel = this.i18n.get('protection_tidb_table_warning_label', [
      this.deleteNum
    ]);
    this.formGroup.get('tableName').setValue(this.selectedTableData);
  }

  removeSingle(item) {
    this.selectionTable = reject(this.selectionTable, value => {
      return item.name === value.name;
    });

    this.selectedTableData = reject(this.selectedTableData, value => {
      return item.name === value.name;
    });
    this.deleteNum = size(this.selectedTableData) - size(this.selectionTable);
    this.warningLabel = this.i18n.get('protection_tidb_table_warning_label', [
      this.deleteNum
    ]);
    this.formGroup.get('tableName').setValue(this.selectedTableData);
    this.cdr.detectChanges();
    this.disableOkBtn();
  }

  selectionChange(selection) {
    this.selectionTable = filter(
      selection,
      item =>
        !get(item, 'extendInfo.isLocked') ||
        get(item, 'extendInfo.isLocked') === 'false'
    );
    this.selectedTableData = [...this.selectionTable];
    this.formGroup.get('tableName').setValue(this.selectedTableData);
    this.disableOkBtn();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
  }

  pageChangeS(page) {
    this.pageSize = page.pageSize;
    this.pageIndexS = page.pageIndex;
  }

  clearSelected() {
    this.selectionTable = [];

    this.selectedTableData = [];
    this.formGroup.get('tableName').setValue(this.selectedTableData);
    this.deleteNum = size(this.selectedTableData) - size(this.selectionTable);
    this.disableOkBtn();
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.invalid ||
      this.formGroup.value?.tableName.length > 256 ||
      size(this.selectionTable) !== size(this.selectedTableData);
  }

  getParams() {
    let tmp: any = filter(this.clusterOptions, item => {
      return item.uuid === this.formGroup.value.cluster;
    });
    let temp: any = filter(this.databaseOptions, item => {
      return item.uuid === this.formGroup.value.database;
    });
    let tableNames = map(this.formGroup.value.tableName, item => {
      return item.name;
    });

    const params: any = {
      name: this.formGroup.value.name,
      type: 'Database',
      subType: this.dataMap.Resource_Type.tidbTable.value,
      extendInfo: {
        tiupUuid: tmp[0].extendInfo.tiupUuid,
        clusterName: tmp[0].extendInfo.clusterName,
        databaseName: temp[0].extendInfo.databaseName,
        tableName: JSON.stringify(tableNames),
        save_type: this.rowData ? 1 : 0,
        linkStatus: '1'
      },
      parentUuid: this.formGroup.value.database,
      parentName: temp[0].name
    };
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.rowData) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowData.uuid,
            UpdateResourceRequestBody: params
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
      } else {
        this.protectedResourceApiService
          .CreateResource({
            CreateResourceRequestBody: params
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
    });
  }
}
