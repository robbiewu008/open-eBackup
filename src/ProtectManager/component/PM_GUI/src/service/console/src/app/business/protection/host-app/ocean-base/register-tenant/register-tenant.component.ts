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
  map,
  reject
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-tenant',
  templateUrl: './register-tenant.component.html',
  styleUrls: ['./register-tenant.component.less']
})
export class RegisterTenantComponent implements OnInit {
  clusterOptions = [];
  allTableData;
  totalTable;
  selectedTableData = [];
  selection = []; //双向绑定
  selectionTenant;
  modifiedTables = [];
  dataMap = DataMap;
  pageIndexS = CommonConsts.PAGE_START;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE * 5;
  tableConfig: TableConfig;
  formGroup: FormGroup;
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
      tenantNames: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
    this.listenForm();

    if (this.data) {
      this.formGroup.patchValue({
        name: this.data.name,
        cluster: this.data.parentUuid || this.data.parent_uuid
      });
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => {
      this.disableOkBtn();
    });
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      if (res) {
        this.getTenantOptions(res);
      }
      this.selectedTableData = [];
      this.allTableData = [];
      this.formGroup.get('tenantNames').setValue(this.selectedTableData);
    });
  }

  getClusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.OceanBaseCluster.value
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
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true,
            parentUuid: item.uuid
          });
        });
        this.clusterOptions = clusterArray;
        return;
      }
      this.getClusterOptions(recordsTemp, startPage);
    });
  }

  getTenantOptions(clusterId, recordsTemp?, startPage?) {
    if (!clusterId) {
      return;
    }
    const params: any = {
      envId: clusterId,
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: 2000,
      resourceType: DataMap.Resource_Type.OceanBaseTenant.value
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
          startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
          res.totalCount === 0
        ) {
          this.allTableData = recordsTemp;
          each(this.allTableData, item => {
            if (!this.data)
              assign(item.extendInfo, {
                isLocked: JSON.parse(item.extendInfo.isUsed)
              });
          });
          this.totalTable = res.totalCount;
          if (this.data) {
            const clusterInfo = JSON.parse(
              get(this.data, 'extendInfo.clusterInfo')
            );
            this.modifiedTables = get(clusterInfo, 'tenantInfos');
          }
          if (this.modifiedTables?.length) {
            const selected = [];
            each(this.allTableData, item => {
              if (!find(this.modifiedTables, { name: item.name })) {
                assign(item.extendInfo, {
                  isLocked: JSON.parse(item.extendInfo.isUsed)
                });
              }
            });
            each(this.modifiedTables, val => {
              const selectedTable = find(
                this.allTableData,
                item => item.name === val.name
              );
              if (!isUndefined(selectedTable)) {
                selected.push(selectedTable);
              }
            });
            this.selectedTableData = selected;
            this.selectionTenant = [...this.selectedTableData];
            this.formGroup.get('tenantNames').setValue(this.selectedTableData);
          }
          return;
        }
        this.getTenantOptions(clusterId, recordsTemp, startPage);
      });
  }

  removeSingle(item) {
    this.selectionTenant = reject(this.selectionTenant, value => {
      return item.name === value.name;
    });

    this.selectedTableData = reject(this.selectedTableData, value => {
      return item.name === value.name;
    });
    this.formGroup.get('tenantNames').setValue(this.selectedTableData);
    this.cdr.detectChanges();
    this.disableOkBtn();
  }

  selectionChange(selection) {
    this.selectionTenant = filter(
      selection,
      item =>
        !get(item, 'extendInfo.isLocked') ||
        get(item, 'extendInfo.isLocked') === 'false'
    );
    this.selectedTableData = [...this.selectionTenant];
    this.formGroup.get('tenantNames').setValue(this.selectedTableData);
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
    this.selectionTenant = [];

    this.selectedTableData = [];
    this.formGroup.get('tenantNames').setValue(this.selectedTableData);
    this.disableOkBtn();
  }

  getParams() {
    const namesArr = this.formGroup.get('tenantNames').value;
    const namesTmp = map(namesArr, item => {
      return {
        name: item.name
      };
    });
    return {
      parentUuid: this.formGroup.value.cluster,
      name: this.formGroup.get('name').value,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.OceanBaseTenant.value,
      extendInfo: {
        clusterInfo: JSON.stringify({ tenantInfos: namesTmp })
      }
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
