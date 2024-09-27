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
import { MessageService, ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  MODAL_COMMON,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { TableConfig } from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  cloneDeep,
  defer,
  each,
  eq,
  find,
  get,
  isNil,
  isNumber,
  map,
  reject,
  size,
  split
} from 'lodash';
import { Subject } from 'rxjs';
import { SelectTableComponent } from '../select-table/select-table.component';

@Component({
  selector: 'aui-create',
  templateUrl: './create.component.html',
  styles: ['.table-names-item{ padding-right:20px; }']
})
export class CreateComponent implements OnInit {
  item;
  colon = false;
  optsConfig;
  optItems = [];
  databaseOptions = [];
  clusterOptions = [];
  databaseName: string;
  modifiedTables = [];
  selectedTableData = [];
  selection = [];
  diffSelection = [];
  dataMap = DataMap;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE * 5;
  tableConfig: TableConfig;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();

  nameErrorTip = {
    ...this.baseUtil.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  @ViewChild('pageS', { static: false }) pageS;
  @ViewChild('pageA', { static: false }) pageA;

  @Input() type;
  @Input() data;
  tableData: any;
  constructor(
    public baseUtil: BaseUtilService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getclusterOptions();
    if (!isNil(this.item)) {
      defer(() => {
        this.initData();
      });
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtil.VALID.name(),
          this.baseUtil.VALID.maxLength(64)
        ]
      }),
      cluster: new FormControl([], {
        validators: [this.baseUtil.VALID.required()]
      }),
      database: new FormControl([], {
        validators: [this.baseUtil.VALID.required()]
      })
    });

    this.listenForm();
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(() => {
      this.disableOkBtn();
    });
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      this.getDatabaseOptions(res);
    });
    this.formGroup.get('database').valueChanges.subscribe(res => {
      this.databaseName = get(
        find(this.databaseOptions, item => eq(res, item.uuid)),
        'name'
      );
      if (
        !isNil(this.item) &&
        !eq(res, this.item?.parentUuid) &&
        eq(size(this.diffSelection), 0)
      ) {
        this.diffSelection = this.selection.map(({ uuid }) => ({ uuid }));
      }
      this.selection = [];
    });
  }
  initData() {
    this.formGroup.setValue({
      name: this.item.name,
      cluster: this.item.rootUuid,
      database: this.item.parentUuid
    });

    this.selection = [];
  }

  selectTableset() {
    const params = {
      pageNo: this.pageIndex + 1,
      pageSize: this.pageSize,
      envId: this.formGroup.get('cluster').value,
      parentId: this.formGroup.get('database').value || 'default',
      resourceType: DataMap.Resource_Type.ClickHouse.value
    };
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'click-house-select-table',
      lvWidth: MODAL_COMMON.normalWidth + 300,
      lvHeader: this.i18n.get('protection_select_table_label'),
      lvContent: SelectTableComponent,
      lvOkDisabled: true,
      lvComponentParams: {
        selectedTableData: this.selection,
        data: {
          clusterId: this.formGroup.get('cluster').value,
          parentId: this.formGroup.get('database').value || 'default',
          name: this.databaseName || 'default'
        },
        item: this.item
      },
      lvOk: modal => {
        const content = modal.getContentComponent() as SelectTableComponent;

        this.selection = content.selectData;

        this.diffSelection = [...this.diffSelection, ...content?.diffSelection];

        this.disableOkBtn();
      }
    });
  }

  getclusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        type: ResourceType.CLUSTER,
        subType: DataMap.Resource_Type.ClickHouse.value
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
        return;
      }
      this.getclusterOptions(recordsTemp, startPage);
    });
  }

  getDatabaseOptions(clusterId, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        parentUuid: clusterId,
        type: ResourceType.DATABASE,
        subType: DataMap.Resource_Type.ClickHouse.value
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
        if (!isNil(this.item)) {
          this.databaseName = get(
            find(this.databaseOptions, _item =>
              eq(this.item.parentUuid, _item.uuid)
            ),
            'name'
          );
          this.getTablesDetail();
        }
        return;
      }
      this.getDatabaseOptions(clusterId, recordsTemp, startPage);
    });
  }

  getTablesDetail() {
    const params = {
      resourceId: this.item.uuid
    };
    this.protectedResourceApiService.ShowResource(params).subscribe(res => {
      this.selection = get(res, ['dependencies', 'children']);
      this.disableOkBtn();
    });
  }

  removeSingle(item) {
    this.selectedTableData = reject(this.selectedTableData, value => {
      return item.name === value.name;
    });
    this.formGroup.get('set').setValue(this.selectedTableData);
  }

  getParams() {
    return {
      name: this.formGroup.value.name,
      type: ResourceType.TABLE_SET,
      subType: DataMap.Resource_Type.ClickHouse.value,
      parentUuid: this.formGroup.value.database,
      parentName: this.databaseName,
      dependencies: {
        children: map(this.selection, item => {
          const _item = cloneDeep(item);
          if (isNil(this.item)) {
            delete _item['parentName'];
            delete _item['parentUuid'];
            delete _item['uuid'];
          }
          if (!isNil(this.item)) delete _item['createdTime'];
          delete _item['disabled'];
          return {
            ..._item
          };
        }),
        '#children': isNil(this.item) ? void 0 : this.diffSelection
      }
    };
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.invalid || eq(size(this.selection), 0);
  }
}
