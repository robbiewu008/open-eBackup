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
import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import {
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedEnvironmentApiService,
  CapacityCalculateLabel,
  CAPACITY_UNIT
} from 'app/shared';
import {
  map,
  size,
  each,
  isNumber,
  find,
  isUndefined,
  isEmpty,
  reject,
  includes,
  union,
  set,
  first
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-tables',
  templateUrl: './tables.component.html',
  styleUrls: ['./tables.component.less'],
  providers: [CapacityCalculateLabel]
})
export class TablesComponent implements OnInit {
  tableData = [];
  tableSelection = [];
  lvSelectedByCheckbox = true;
  children$ = new Subject<any>();

  @Input() source;
  @Input() isSummary;
  @Output() onSelectionChange = new EventEmitter<any>();

  constructor(
    private i18n: I18NService,
    private capacityCalculateLabel: CapacityCalculateLabel,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.updateData();
  }

  updateData() {
    if (!this.isSummary || !this.source) {
      return;
    }
    this.updateNameSpace();
  }

  updateNameSpace() {
    if (isEmpty(this.source.extendInfo.table)) {
      return;
    }
    if (this.source?.subType === DataMap.Resource_Type.HiveBackupSet.value) {
      this.tableData = [
        {
          label: this.source.extendInfo?.databaseName,
          contentToggleIcon: 'aui-icon-directory',
          children: []
        }
      ];
      return;
    }
    if (
      this.source?.subType ===
      DataMap.Resource_Type.ElasticsearchBackupSet.value
    ) {
      this.tableData = [
        {
          label: this.source.environment.name,
          contentToggleIcon: 'aui-icon-directory',
          children: []
        }
      ];
      return;
    }

    const paths = this.source.extendInfo.table.split(',');
    const nameSpaces = [];
    each(paths, path => {
      const namespaceLabel = path.startsWith('/')
        ? path.split('/')[1]
        : path.split('/')[0];
      const namespace = find(nameSpaces, {
        label: namespaceLabel
      });
      if (isUndefined(namespace)) {
        nameSpaces.push({
          label: namespaceLabel,
          contentToggleIcon: 'aui-icon-directory-namespace',
          children: []
        });
        this.tableData = nameSpaces;
      }
      this.expandChildren(
        find(this.tableData, { label: namespaceLabel }),
        path
      );
    });
    nameSpaces.filter(item => {
      if (!size(item['children'])) {
        item.expandedToggleIcon = '';
      }
    });
    this.tableData = [...this.tableData];
  }

  expandChildren(node, path) {
    if (`/${node.label}` === path) {
      return;
    }
    this.tableData.filter(item => {
      if (item.label === node.label) {
        item.expanded = true;
        item.children = item.children.concat([
          {
            label: path.split('/')[2],
            isLeaf: true,
            contentToggleIcon: 'aui-icon-file'
          }
        ]);
      }
    });
    this.tableData = [...this.tableData];
  }

  getTables(recordsTemp?, startPage?) {
    let params;

    if (this.source?.subType === DataMap.Resource_Type.HiveBackupSet.value) {
      params = {
        pageNo: startPage || CommonConsts.PAGE_START + 1,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        envId: this.source.environment.uuid,
        parentId: this.source.extendInfo?.databaseName,
        resourceType: 'HiveTable'
      };
    }
    if (
      this.source?.subType ===
      DataMap.Resource_Type.ElasticsearchBackupSet.value
    ) {
      params = {
        pageNo: startPage || CommonConsts.PAGE_START + 1,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        envId: this.source.environment.uuid,
        parentId: '',
        resourceType: 'ElasticsearchIndex'
      };
    }

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
          set(
            first(this.tableData),
            'children',
            map(recordsTemp, item => {
              return {
                label: item.name,
                isLeaf: true,
                contentToggleIcon: 'aui-icon-file'
              };
            })
          );
          this.tableData = [...this.tableData];
          return;
        }
        this.getTables(recordsTemp, startPage);
      });
  }

  getNameSpace(tableData) {
    map(tableData, item => {
      item['label'] = item.extendInfo.nameSpace;
      item['contentToggleIcon'] = 'aui-icon-directory-namespace';
      item['children'] = item['children'] || [];
      item['expanded'] = item['expanded'] || false;
      item['isLeaf'] = false;
      return item;
    });
    this.tableData = map(tableData, item => {
      const originItem = find(this.tableData, data => data.name === item.name);

      return originItem ? originItem : item;
    });
    // 处理selection
    const childrenMap = [];
    each(this.tableData, item => {
      childrenMap.push(...item.children);
    });
    this.tableSelection = reject(this.tableSelection, item => {
      return (
        !includes(map(this.tableData, 'name'), item.name) &&
        !includes(map(childrenMap, 'name'), item.name)
      );
    });
    this.selectionChange();
  }

  expandedChange(node) {
    if (
      includes(
        [
          DataMap.Resource_Type.HiveBackupSet.value,
          DataMap.Resource_Type.ElasticsearchBackupSet.value
        ],
        this.source?.subType
      )
    ) {
      this.getTables();
      return;
    }
    if (this.isSummary || !node.expanded || !!size(node.children)) {
      return;
    }
    this.getResource(node);
  }

  getResource(node, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: 100,
      envId: node.rootUuid,
      parentId: node.uuid,
      resourceType: DataMap.Resource_Type.HBaseTable.value
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
          startPage === Math.ceil(res.totalCount / 100) ||
          res.totalCount === 0
        ) {
          each(recordsTemp, (item: any) => {
            item['label'] = `${
              item.extendInfo.tableName
            }(${this.capacityCalculateLabel.transform(
              +item.extendInfo.tableSingleSize,
              '1.3-3',
              CAPACITY_UNIT.BYTE,
              true
            )})`;
            item['isLeaf'] = true;
            item['contentToggleIcon'] = 'aui-icon-file';
          });
          node.children = recordsTemp;
          if (!isUndefined(find(this.tableSelection, { label: node.label }))) {
            this.tableSelection = union(this.tableSelection, node.children);
            this.selectionChange();
          }
          this.tableData = [...this.tableData];
          this.children$.next(node);
          return;
        }
        this.getResource(node, recordsTemp, startPage);
      });
  }

  selectionChange() {
    this.onSelectionChange.emit(this.tableSelection);
  }
}
