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
  ElementRef,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { I18NService } from 'app/shared/services';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { ModalRef, TreeNode } from '@iux/live';
import {
  assign,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isUndefined,
  map,
  reject,
  size,
  trim,
  uniqueId
} from 'lodash';
import {
  CopyControllerService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  CommonConsts,
  RestoreFileType,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared/consts';
import { isJson } from 'app/shared';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-object-level-restore',
  templateUrl: './object-level-restore.component.html',
  styleUrls: ['./object-level-restore.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ObjectLevelRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;

  title = this.i18n.get('explore_object_need_restore_label');
  formGroup: FormGroup;
  pathView = 'all';
  total = 0;
  selectedTotal = 0;
  restoreLocationType = RestoreV2LocationType;
  resourceData;
  properties;
  treeData = [];
  treeSelection = [];
  tableData: TableData;
  tableConfig: TableConfig;
  selectionTableData = [];
  selectTableData: TableData;
  selectTableConfig: TableConfig;
  isSearch = false; // 是否触发了手动搜索
  // 搜索关键字
  searchKey: string;

  @ViewChild('input', { static: false }) pathInput: ElementRef<
    HTMLIFrameElement
  >;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('dataSelectTable', { static: false })
  dataSelectTable: ProTableComponent;
  @ViewChild('fileTpl', { static: true }) fileTpl: TemplateRef<any>;
  @ViewChild('optTpl', { static: true }) optTpl: TemplateRef<any>;
  @ViewChild('directoryNameTpl', { static: true })
  directoryNameTpl: TemplateRef<any>;
  constructor(
    private fb: FormBuilder,
    private cdr: ChangeDetectorRef,
    private i18n: I18NService,
    private modal: ModalRef,
    private restoreV2Service: RestoreApiV2Service,
    private copyControllerService: CopyControllerService
  ) {}

  ngOnInit(): void {
    this.formatCopyData();
    this.initForm();
    this.initTree();
  }

  formatCopyData() {
    this.resourceData = isJson(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.properties = isJson(this.rowCopy.properties)
      ? JSON.parse(this.rowCopy.properties)
      : {};
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      location: new FormControl({
        value: this.resourceData?.name,
        disabled: true
      })
    });
    this.modal.getInstance().lvOkDisabled = true;
  }

  initFirstTreeNode() {
    this.treeData = [
      this.createTreeNode({
        path: `${this.properties.parent_path}`,
        type: 'd',
        extendInfo: JSON.stringify({
          name: `${this.properties.parent_path}`,
          description: '',
          type: ''
        })
      })
    ];
    this.treeSelection = [first(this.treeData)];
  }

  initTree() {
    this.initFirstTreeNode();
    this.initConfig();
    // 默认选中根节点
    setTimeout(() => {
      this.pathNodeCheck({ node: this.treeSelection[0] });
    });
    this.cdr.markForCheck();
  }

  // 将type为'd'的数据改造成treeNode
  createTreeNode(curNode, parentNode?): TreeNode {
    const extendObj = JSON.parse(get(curNode, 'extendInfo', '{}'));
    const queryPath = isEmpty(parentNode)
      ? `/${get(curNode, 'path')}`
      : `${get(parentNode, 'queryPath')}/${get(curNode, 'path')}`;
    return {
      key: get(extendObj, 'name'),
      label: get(curNode, 'path'),
      queryPath,
      absolutePath: queryPath,
      ...curNode,
      children: [], // 用于左侧展示的节点，需要将curNode的children子节点中type='f'的过滤
      expanded: false,
      disabled: false,
      isLeaf: get(curNode, 'type', 'd') === RestoreFileType.File,
      contentToggleIcon: ''
    };
  }

  // 将type为'd'和'f'中的extendInfo拓展信息取出
  createTableNode(curNode) {
    const extendObj = JSON.parse(get(curNode, 'extendInfo', '{}'));
    return {
      ...curNode,
      key: get(extendObj, 'name'),
      label: get(curNode, 'path'),
      icon: '', // 需要根据extendInfo中不同的类型给出不同的icon
      file_type: get(extendObj, 'type'),
      desc: get(extendObj, 'description'),
      absolutePath: get(extendObj, 'name'),
      isLeaf: get(curNode, 'type', 'd') === RestoreFileType.File
    };
  }

  // 如果父节点选了，子节点选中自动忽略
  mergeSelection() {
    let mergePaths = [];
    const parentPaths = map(this.selectionTableData, 'absolutePath');
    mergePaths = reject(this.selectionTableData, item => {
      return includes(parentPaths, item.path);
    });
    return mergePaths;
  }

  setSelection() {
    const mergePaths = this.mergeSelection();
    this.selectedTotal = size(mergePaths);
    this.selectTableData = {
      data: mergePaths,
      total: this.selectedTotal
    };
    this.modal.getInstance().lvOkDisabled = !this.selectedTotal;
  }

  tabChange() {
    if (this.pathView === 'all') {
      setTimeout(() => {
        this.dataTable?.setSelections(this.selectionTableData);
        this.cdr.markForCheck();
      });
    }
  }

  initConfig(): void {
    const cols: TableCols[] = [
      {
        key: 'label',
        name: this.i18n.get('common_name_label'),
        cellRender: this.directoryNameTpl
      },
      {
        key: 'absolutePath',
        name: this.i18n.get('common_path_label')
      },
      {
        key: 'file_type',
        name: this.i18n.get('common_type_label'),
        width: '150px'
      },
      {
        key: 'desc',
        name: this.i18n.get('common_desc_label')
      }
    ];
    const page: any = {
      mode: 'simple',
      pageSize: CommonConsts.MAX_PAGE_SIZE,
      showPageSizeOptions: false
    };
    this.initSelectTableConfig(cols, page);
    this.initSelectedTableConfig(cols, page);
  }

  treeTrackByPath = (_, item) => {
    return item?.absolutePath;
  };

  private initSelectedTableConfig(cols: TableCols[], page: any) {
    this.selectTableConfig = {
      table: {
        compareWith: 'absolutePath',
        async: false,
        columns: [
          {
            key: 'label',
            name: this.i18n.get('common_name_label'),
            cellRender: this.directoryNameTpl,
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          ...cols.slice(1, 2),
          {
            key: 'opt',
            width: '80px',
            name: this.i18n.get('common_operation_label'),
            cellRender: this.optTpl
          }
        ],
        scroll: { y: '520px' },
        virtualScroll: true,
        colDisplayControl: false,
        trackByFn: (_, item) => {
          return item.absolutePath;
        }
      },
      pagination: page
    };
  }

  initSelectTableConfig(cols: TableCols[], page: any) {
    this.tableConfig = {
      table: {
        compareWith: 'absolutePath',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scroll: { y: '520px' },
        virtualScroll: true,
        colDisplayControl: false,
        fetchData: (filters: Filters, args) => {
          this.getTableData(filters, args);
        },
        selectionChange: selection => {
          this.selectionTableData = selection;
          this.setSelection();
        },
        trackByFn: (_, item) => {
          return item.absolutePath;
        }
      },
      pagination: page
    };
  }

  deleteNode(node) {
    this.selectionTableData = reject(
      this.selectionTableData,
      item => item.absolutePath === node.absolutePath
    );
    this.setSelection();
  }

  beforeSelected = item => {
    if (this.treeSelection[0].absolutePath === item.absolutePath) {
      return false;
    }
  };

  // 通过名称搜索文件
  searchName($event): void {
    if (this.pathView === 'selected') {
      this.pathView = 'all';
    }
    setTimeout(() => {
      this.isSearch = true;
      this.dataTable?.fetchData({
        ...first(this.treeSelection)
      });
    });
  }

  // 节点展开
  expandedChange(node): void {
    const moreBtn = find(node.children, { isMoreBtn: true });
    if (!!size(node.children) || node?.isLeaf) {
      return;
    }
    this.getCopySourceNode(node);
  }

  // 点击左树节点
  pathNodeCheck($event) {
    const node = $event.node;
    // 获取右边表格数据，当前路径下子路径
    if (this.pathView === 'all') {
      this.dataTable.fetchData(node);
    } else {
      this.pathView = 'all';
      setTimeout(() => {
        this.dataTable?.fetchData(node);
        this.dataTable?.setSelections(this.selectionTableData);
      });
    }
  }

  // 点击左树更多
  clickMoreFile($event, node) {
    if ($event.stopPropagation) {
      $event.stopPropagation();
    }
    this.getCopySourceNode(node?.parent, node?.startPage);
  }

  getCopySourceNode(node, startPage?) {
    const params = {
      copyId: this.rowCopy.uuid,
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.MAX_PAGE_SIZE,
      parentPath: node?.absolutePath || '/',
      akOperationTips: false,
      akLoading: true
    };
    if (!isEmpty(this.rowCopy.device_esn)) {
      assign(params, { memberEsn: this.rowCopy.device_esn });
    }
    this.copyControllerService.ListCopyCatalogs(params).subscribe(res => {
      const treeNodes: TreeNode[] = map(res.records, item =>
        this.createTreeNode(item, node)
      ); // 需要插入到左边的树节点
      this.appendTreeNode(treeNodes, res.totalCount, node);
      this.treeData = [...this.treeData];
      this.cdr.markForCheck();
    });
  }

  appendTreeNode(nodes: TreeNode[], total, node?) {
    if (isUndefined(node)) {
      this.treeData = nodes;
      this.cdr.markForCheck();
      return;
    }
    if (isArray(node.children) && !isEmpty(node.children)) {
      node.children = [
        ...reject(node.children, n => {
          return n.isMoreBtn;
        }),
        ...nodes
      ];
    } else {
      node.children = [...nodes];
    }
    if (total > size(node.children)) {
      const moreClickNode = {
        label: `${this.i18n.get('common_more_label')}...`,
        uuid: uniqueId(),
        isMoreBtn: true,
        isLeaf: true,
        disabled: true,
        startPage: Math.floor(size(node.children) / CommonConsts.PAGE_SIZE_MAX)
      };
      node.children = [...node.children, moreClickNode];
    }
    this.treeData = [...this.treeData];
    this.treeSelection = [...this.treeSelection];
    this.cdr.markForCheck();
  }

  updateTable(res) {
    this.tableData = {
      data: res.records.map(item => this.createTableNode(item)),
      total: res.totalCount
    };
    this.total = res.totalCount;
    this.cdr.markForCheck();
  }

  // 获取指定路径下的文件
  getTableData(filters: Filters, node) {
    if (!node) {
      node = first(this.treeSelection);
    }
    if (!node) {
      this.tableData = {
        data: [],
        total: 0
      };
      this.total = 0;
      return;
    }
    if (this.isSearch) {
      this.isSearch = false;
      this.initFirstTreeNode();
      this.total = 0;
      this.tableData = {
        data: [],
        total: 0
      };
      if (trim(this.searchKey)) {
        this.getTableDataBySearchName(first(this.treeData));
      } else {
        this.getTableDataByNode(filters, node);
      }
    } else {
      this.getTableDataByNode(filters, node);
    }
  }

  private getTableDataBySearchName(node) {
    const params = {
      copyId: this.rowCopy.uuid,
      pageNum: CommonConsts.PAGE_START,
      pageSize: CommonConsts.MAX_PAGE_SIZE,
      parentPath: `/${this.properties.parent_path}`,
      name: trim(this.searchKey),
      akLoading: true
    };
    this.copyControllerService.ListCopyCatalogsByName(params).subscribe(res => {
      const tableNode = [];
      this.formatSearchResultByRecursion(first(this.treeData), res, tableNode);
      this.treeData = [...this.treeData];
      this.updateTable({
        records: tableNode,
        totalCount: tableNode.length
      });
      this.cdr.markForCheck();
    });
  }

  private getTableDataByNode(filters: Filters, node) {
    const params = {
      copyId: this.rowCopy.uuid,
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      parentPath: node.absolutePath || '/',
      akLoading: true
    };
    this.copyControllerService.ListCopyCatalogs(params).subscribe(res => {
      this.updateTable(res);
    });
  }

  formatSearchResultByRecursion(parentNode, result, tableNode) {
    parentNode.expanded = true;
    for (const item of result?.records || result?.items) {
      const node = this.createTreeNode(item, parentNode);
      node.expanded = !isEmpty(item?.children);
      parentNode.children.push({ ...node });
      if (item.path === this.searchKey) {
        tableNode.push({ ...item });
      }
      if (!isEmpty(item.children)) {
        this.formatSearchResultByRecursion(node, item.children, tableNode);
      }
    }
  }

  getParams() {
    const objectInfo = map(this.selectTableData.data, 'extendInfo');
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.resourceData?.environment_uuid || this.resourceData?.root_uuid,
      restoreType: RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject: this.resourceData?.uuid,
      extendInfo: {
        objectInfo: JSON.stringify(objectInfo)
      }
    };
    return params;
  }

  restore(): Observable<void> {
    if (isEmpty(this.selectTableData)) {
      return;
    }
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
        .subscribe({
          next: value => {
            observer.next();
            observer.complete();
          },
          error: err => {
            observer.error(err);
            observer.complete();
          }
        });
    });
  }
}
