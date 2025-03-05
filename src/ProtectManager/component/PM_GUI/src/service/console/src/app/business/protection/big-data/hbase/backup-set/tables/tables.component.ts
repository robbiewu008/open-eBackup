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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output
} from '@angular/core';
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
  first,
  isArray
} from 'lodash';
import { Subject } from 'rxjs';
import { map as _map } from 'rxjs/operators';
@Component({
  selector: 'aui-tables',
  templateUrl: './tables.component.html',
  styleUrls: ['./tables.component.less'],
  providers: [CapacityCalculateLabel],
  changeDetection: ChangeDetectionStrategy.OnPush
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
    private cdr: ChangeDetectorRef,
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
        this.cdr.detectChanges();
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
    this.cdr.detectChanges();
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
    this.cdr.detectChanges();
  }

  getTables(node, startPage?) {
    let params;

    if (this.source?.subType === DataMap.Resource_Type.HiveBackupSet.value) {
      params = {
        pageNo: startPage || CommonConsts.PAGE_START + 1,
        pageSize: CommonConsts.PAGE_SIZE_MAX,
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
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        envId: this.source.environment.uuid,
        parentId: '',
        resourceType: 'ElasticsearchIndex'
      };
    }

    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .pipe(
        _map(res => {
          res.records = res.records.map(item => {
            return {
              ...item,
              label: item.name,
              isLeaf: true,
              contentToggleIcon: 'aui-icon-file'
            };
          });
          return res;
        })
      )
      .subscribe(res => {
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START + 1;
        }
        startPage++;
        if (isArray(node.children) && !isEmpty(node.children)) {
          node.children = [
            ...reject(node.children, n => !!n.isMoreBtn),
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
        this.tableData = [...this.tableData];
        this.cdr.detectChanges();
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
    this.cdr.detectChanges();
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
    this.cdr.detectChanges();
    this.selectionChange();
  }

  expandedChange(node, startPage?) {
    if (!isEmpty(node?.children)) {
      // 如果children已经有值，反复展开折叠不应该触发查询
      return;
    }
    this.getTreeNode(node, startPage);
  }

  getTreeNode(node, startPage) {
    if (
      includes(
        [
          DataMap.Resource_Type.HiveBackupSet.value,
          DataMap.Resource_Type.ElasticsearchBackupSet.value
        ],
        this.source?.subType
      )
    ) {
      this.getTables(node, startPage);
      return;
    }
    if (this.isSummary || !node.expanded || !!size(node.children)) {
      return;
    }
    this.getResource(node, startPage);
  }

  getResource(node, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      envId: node.rootUuid,
      parentId: node.uuid,
      resourceType: DataMap.Resource_Type.HBaseTable.value
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        each(res.records, (item: any) => {
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
        if (isArray(node.children) && !isEmpty(node.children)) {
          node.children = [
            ...reject(node.children, n => {
              return !!n.isMoreBtn;
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
            startPage: Math.floor(
              size(node.children) / CommonConsts.PAGE_SIZE_MAX
            )
          };
          node.children = [...node.children, moreClickNode];
        }
        if (!isUndefined(find(this.tableSelection, { label: node.label }))) {
          this.tableSelection = union(this.tableSelection, node.children);
          this.selectionChange();
        }
        this.tableData = [...this.tableData];
        this.cdr.detectChanges();
        this.children$.next(node);
      });
  }

  selectionChange() {
    this.onSelectionChange.emit(this.tableSelection);
    this.cdr.detectChanges();
  }
}
