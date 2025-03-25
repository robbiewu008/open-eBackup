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
import {
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  InstanceType,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { TableConfig } from 'app/shared/components/pro-table';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';
import {
  assign,
  each,
  filter,
  find,
  get,
  includes,
  isNumber,
  isUndefined,
  join,
  map,
  reject,
  size,
  slice
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-create-tableset',
  templateUrl: './create-tableset.component.html',
  styleUrls: ['./create-tableset.component.less']
})
export class CreateTablesetComponent implements OnInit {
  set;
  item;
  colon = false;
  optsConfig;
  optItems = [];
  instanceOptions = [];
  databaseOptions = [];
  clusterOptions = [];
  treeData = [];
  allTableData;
  totalTable;
  modifiedTables = [];
  selectedTableData = [];
  selection = [];
  selectionSchema = [];
  dataMap = DataMap;
  queryTableName;
  pageIndexS = CommonConsts.PAGE_START;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE * 5;
  tableConfig: TableConfig;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  readonly PAGESIZE = CommonConsts.PAGE_SIZE * 10;

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  @ViewChild('pageS', { static: false }) pageS;
  @ViewChild('pageA', { static: false }) pageA;
  @ViewChild('namePopover', { static: false }) namePopover;

  @Input() data;
  tableData: any;
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
    this.getClusterOptions();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      cluster: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      instance: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      database: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      set: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
    this.listenForm();

    if (this.data) {
      this.formGroup.patchValue({
        name: this.data.name,
        cluster: this.data.environment.rootUuid,
        instance: this.data.extendInfo.instanceUuid,
        database: this.data.parentUuid,
        set: this.data.extendInfo.table.split(',')
      });
      this.formGroup.get('name').disable();
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => {
      this.disableOkBtn();
    });
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      if (res) {
        this.getInstanceOptions(res);
      }
      this.formGroup.get('database').setValue('');
      this.formGroup.get('instance').setValue('');
      this.selectedTableData = [];
      this.allTableData = [];
      this.formGroup.get('set').setValue(this.selectedTableData);
    });
    this.formGroup.get('instance').valueChanges.subscribe(res => {
      if (res) {
        this.getDatabaseOptions(res);
      }
      this.formGroup.get('database').setValue('');
      this.selectedTableData = [];
      this.allTableData = [];
      this.formGroup.get('set').setValue(this.selectedTableData);
    });
    this.formGroup.get('database').valueChanges.subscribe(res => {
      if (res) {
        this.getSchemas(res);
      }
      this.queryTableName = '';
      this.selectedTableData = [];
      this.allTableData = [];
      this.selectionSchema = [];
      this.formGroup.get('set').setValue(this.selectedTableData);
    });
  }

  getClusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: this.PAGESIZE,
      conditions: JSON.stringify({
        subType: [
          DataMap.Resource_Type.dbTwoClusterInstance.value,
          DataMap.Resource_Type.dbTwoInstance.value
        ],
        isTopInstance: InstanceType.TopInstance
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
        startPage === Math.ceil(res.totalCount / this.PAGESIZE) ||
        res.totalCount === 0
      ) {
        const clusterArray = [];
        each(recordsTemp, item => {
          clusterArray.push({
            ...item,
            key: item?.environment.uuid,
            value: item?.environment.uuid,
            label:
              item?.subType === DataMap.Resource_Type.dbTwoClusterInstance.value
                ? item?.environment.name
                : `${item?.environment.name}(${item?.environment.endpoint})`,
            isLeaf: true
          });
        });
        // 原有过滤HADR类型，新增过滤RHEL HA创建表级
        this.clusterOptions = filter(clusterArray, item => {
          return (
            item.extendInfo?.clusterType !== DataMap.dbTwoType.hadr.value &&
            item.extendInfo?.clusterType !== DataMap.dbTwoType.rhel.value
          );
        });

        return;
      }
      this.getClusterOptions(recordsTemp, startPage);
    });
  }

  getInstanceOptions(clusterId, recordsTemp?, startPage?) {
    if (!clusterId) {
      return;
    }

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: this.PAGESIZE,
      conditions: JSON.stringify({
        subType: [
          DataMap.Resource_Type.dbTwoClusterInstance.value,
          DataMap.Resource_Type.dbTwoInstance.value
        ],
        isTopInstance: InstanceType.TopInstance,
        rootUuid: clusterId
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
        startPage === Math.ceil(res.totalCount / this.PAGESIZE) ||
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

        this.instanceOptions = clusterArray;

        return;
      }
      this.getInstanceOptions(clusterId, recordsTemp, startPage);
    });
  }

  getDatabaseOptions(instanceId, recordsTemp?, startPage?) {
    if (!instanceId) {
      return;
    }

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: this.PAGESIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.dbTwoDatabase.value,
        parentUuid: instanceId
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
        startPage === Math.ceil(res.totalCount / this.PAGESIZE) ||
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
        return;
      }
      this.getDatabaseOptions(instanceId, recordsTemp, startPage);
    });
  }

  getSchemas(database, recordsTemp?, startPage?) {
    if (!database) {
      return;
    }

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: this.PAGESIZE,
      envId: this.formGroup.get('cluster').value,
      parentId: database,
      resourceType: DataMap.Resource_Type.dbTwoTableSet.value
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
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
          startPage === Math.ceil(res.totalCount / this.PAGESIZE) ||
          res.totalCount === 0
        ) {
          this.allTableData = recordsTemp;
          this.totalTable = size(recordsTemp);

          if (this.data) {
            this.modifiedTables = this.data?.extendInfo?.table.split(',');
          }

          if (this.modifiedTables?.length) {
            each(this.allTableData, item => {
              if (find(this.modifiedTables, val => val === item.name)) {
                assign(item.extendInfo, {
                  isLocked: 'false'
                });
              }
            });
            const selected = [];
            each(this.modifiedTables, val => {
              const selectedTable = find(
                this.allTableData,
                item => val === item.name
              );

              if (!isUndefined(selectedTable)) {
                selected.push(selectedTable);
              }
            });
            this.selectedTableData = selected;
            this.selectionSchema = [...this.selectedTableData];
            this.formGroup.get('set').setValue(this.selectedTableData);
          }

          return;
        }
        this.getSchemas(database, recordsTemp, startPage);
      });
  }

  removeSingle(item) {
    this.selectionSchema = reject(this.selectionSchema, value => {
      return item.name === value.name;
    });

    this.selectedTableData = reject(this.selectedTableData, value => {
      return item.name === value.name;
    });
    this.formGroup.get('set').setValue(this.selectedTableData);
    this.cdr.detectChanges();
    this.disableOkBtn();
  }

  singleChange(rowData) {
    if (rowData.checked) {
      this.selectedTableData = [...this.selectedTableData, rowData];
      this.disableOkBtn();
    } else {
      this.selectedTableData = filter(
        this.selectedTableData,
        item => item.name !== rowData.name
      );
      this.disableOkBtn();
    }

    this.formGroup.get('set').setValue(this.selectedTableData);
  }

  selectionChange(selection) {
    this.selectionSchema = filter(
      selection,
      item =>
        !get(item, 'extendInfo.isLocked') ||
        get(item, 'extendInfo.isLocked') === 'false'
    );
    this.selectedTableData = [...this.selectionSchema];
    this.formGroup.get('set').setValue(this.selectedTableData);
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

  selectPage() {
    const pageData = slice(
      this.allTableData,
      this.pageIndex * this.pageSize,
      this.pageIndex * this.pageSize + this.pageSize
    );
    each(pageData, item => {
      if (
        !get(item, 'checked') &&
        get(item, 'extendInfo.isLocked') === 'false'
      ) {
        this.selectedTableData = [...this.selectedTableData, item];
        assign(item, {
          checked: true
        });
      }
    });

    this.formGroup.get('set').setValue(this.selectedTableData);
    this.disableOkBtn();
  }

  clearSelected() {
    this.selectionSchema = [];

    this.selectedTableData = [];
    this.formGroup.get('set').setValue(this.selectedTableData);
    this.disableOkBtn();
  }

  getParams() {
    return {
      name: this.formGroup.get('name').value,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.dbTwoTableSet.value,
      extendInfo: {
        instance: get(
          find(
            this.instanceOptions,
            item => item.value === this.formGroup.value.instance
          ),
          'name'
        ),
        instanceUuid: this.formGroup.value.instance,
        table: join(
          map(this.selectedTableData, item => item.name),
          ','
        )
      },
      parentName: get(
        find(
          this.databaseOptions,
          item => item.value === this.formGroup.value.database
        ),
        'name'
      ),
      parentUuid: this.formGroup.value.database,
      rootUuid: this.formGroup.value.cluster
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.data) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.data.uuid,
            UpdateResourceRequestBody: params as any
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
            CreateResourceRequestBody: params as any
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

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }
}
