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
  Pipe,
  PipeTransform,
  ViewChild
} from '@angular/core';
import { MessageService, ModalRef } from '@iux/live';
import {
  CommonConsts,
  I18NService,
  ProtectedEnvironmentApiService
} from 'app/shared';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  get,
  isNumber,
  reject,
  set,
  size,
  slice
} from 'lodash';

@Pipe({ name: 'selectable' })
export class SelectionPipe implements PipeTransform {
  transform(value: [], exponent: string = 'disabled') {
    return filter(value, item => !item[exponent]);
  }
}

@Component({
  selector: 'aui-select-table',
  templateUrl: './select-table.component.html',
  styleUrls: ['./select-table.component.less']
})
export class SelectTableComponent implements OnInit {
  @Input() data;
  @Input() selectedTableData;
  @Input() modifiedTables;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE * 5;
  totalTable = 0;
  allTableData = [];
  selectionData = [];
  @ViewChild('pageA', { static: false }) pageA;
  @ViewChild('pageS', { static: false }) pageS;

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private message: MessageService,
    private cdr: ChangeDetectorRef,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.getTables();
    this.disableOkBtn();
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled =
      !size(this.selectedTableData) || size(this.selectedTableData) > 256;

    if (size(this.selectedTableData) > 256) {
      this.message.error(this.i18n.get('protection_hive_table_max_tip_label'), {
        lvShowCloseButton: true,
        lvMessageKey: 'hiveMaxTableKey'
      });
    }
  }

  getTables(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      envId: this.data?.clusterId,
      parentId: this.data.name,
      resourceType: 'HiveTable'
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
            Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) + 1 ||
          res.totalCount === 0
        ) {
          this.allTableData = recordsTemp;
          this.totalTable = recordsTemp.length;

          if (this.modifiedTables.length) {
            each(this.allTableData, item => {
              if (find(this.modifiedTables, val => val === item.name)) {
                assign(item.extendInfo, {
                  isLocked: 'false'
                });
                assign(item, {
                  disabled: false
                });
              } else {
                assign(item, {
                  disabled: item.extendInfo?.isLocked === 'true'
                });
              }
            });
          } else {
            each(this.allTableData, item => {
              assign(item, {
                disabled: item.extendInfo?.isLocked === 'true'
              });
            });
          }
          each(this.allTableData, item => {
            if (find(this.selectedTableData, { name: item.name })) {
              this.selectionData = [...this.selectionData, item];
            }
          });
          return;
        }
        this.getTables(recordsTemp, startPage);
      });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
  }

  selectionChange() {
    this.selectedTableData = cloneDeep(this.selectionData);
    this.disableOkBtn();
  }

  clearSelected() {
    this.selectionData = [];
    this.selectedTableData = [];
    this.disableOkBtn();
  }

  removeSingle(item) {
    this.selectedTableData = reject(this.selectedTableData, value => {
      return item.name === value.name;
    });

    this.selectionData = reject(this.selectionData, value => {
      return item.name === value.name;
    });

    this.selectionData = [...this.selectionData];

    this.cdr.detectChanges();
    this.disableOkBtn();
  }

  onOK() {
    return cloneDeep(this.selectedTableData);
  }
}
