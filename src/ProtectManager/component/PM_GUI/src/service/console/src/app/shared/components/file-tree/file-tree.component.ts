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
  Component,
  ElementRef,
  EventEmitter,
  Input,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { I18NService } from 'app/shared/services';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from '../pro-table';
import { MenuItem, TreeNode } from '@iux/live';
import {
  cloneDeep,
  each,
  first,
  includes,
  isEmpty,
  map,
  reject,
  size,
  take,
  trim
} from 'lodash';
import { CopyControllerService } from 'app/shared/api/services';
import { CAPACITY_UNIT } from 'app/shared/consts';
import { extendNodeParams } from 'app/shared';

@Component({
  selector: 'aui-file-tree',
  templateUrl: './file-tree.component.html',
  styleUrls: ['./file-tree.component.less']
})
export class FileTreeComponent implements OnInit {
  @Input() copy;
  @Input() treeData: TreeNode[];
  @Output() tableSelectionChange = new EventEmitter<any>();
  @Output() treeExpandedChange = new EventEmitter<any>();

  title = this.i18n.get('explore_object_need_restore_label');
  unitconst = CAPACITY_UNIT;

  pathView = 'all';
  total = 0;
  selectedTotal = 0;

  filePathItems: MenuItem[];

  selectionTree = [];
  tableData: TableData;
  tableConfig: TableConfig;
  selectionTableData = [];
  selectTableData: TableData;
  selectTableConfig: TableConfig;
  // 是否是路径搜索
  isPathSearchFlag = false;
  // 当前选中路径
  currentPath: string;
  // 搜索关键字
  searcKey: string;

  @ViewChild('input', { static: false }) pathInput: ElementRef<
    HTMLIFrameElement
  >;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('dataSelectTable', { static: false })
  dataSelectTable: ProTableComponent;
  @ViewChild('fileTpl', { static: true }) fileTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('optTpl', { static: true }) optTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private copyControllerService: CopyControllerService
  ) {}

  ngOnInit(): void {
    this.selectionTree = [first(this.treeData)];
    this.getFilePathItems(this.selectionTree[0]);
    this.initConfig();
    // 默认选中根节点
    setTimeout(() => {
      this.pathNodeCheck({ node: this.selectionTree[0] });
    });
  }

  getFilePathItems(node) {
    const path = node?.absolutePath;
    if (!path) {
      this.filePathItems = [];
      return;
    }
    const breadcrumbItems = [];
    const pathItems = path === '/' ? [''] : path.split('/');
    each(pathItems, (item, index) => {
      breadcrumbItems.push({
        id: `${index}_${item}`,
        label: item || this.copy?.resource_name,
        onClick: event => this.toTargetPath(event, node, item)
      });
    });
    this.currentPath = map(breadcrumbItems, 'label').join('/');
    this.filePathItems = [...breadcrumbItems];
  }

  setSelection() {
    this.selectedTotal = size(this.selectionTableData);
    this.selectTableData = {
      data: this.selectionTableData,
      total: this.selectedTotal
    };
    this.tableSelectionChange.emit(cloneDeep(this.selectionTableData));
  }

  tabChange() {
    if (this.pathView === 'all') {
      setTimeout(() => {
        this.dataTable?.setSelections(this.selectionTableData);
      });
    }
  }

  initConfig(): void {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        cellRender: this.fileTpl
      },
      {
        key: 'size',
        name: this.i18n.get('common_size_label'),
        cellRender: this.sizeTpl
      },
      {
        key: 'modifyTime',
        name: this.i18n.get('protection_last_modifyed_label')
      }
    ];
    const page: any = {
      mode: 'simple',
      pageSizeOptions: [20, 50, 100, 200]
    };
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
    this.selectTableConfig = {
      table: {
        compareWith: 'absolutePath',
        async: false,
        columns: [
          ...take(cols, 2),
          {
            key: 'modifyTime',
            name: this.i18n.get('protection_last_modifyed_label'),
            cellRender: this.optTpl
          }
        ],
        scroll: { y: '520px' },
        colDisplayControl: false,
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
    if (this.selectionTree[0].absolutePath === item.absolutePath) {
      return false;
    }
  };

  findNodeFromPath(node, path) {
    if (node.nodeName === path) {
      return node;
    }
    if (!isEmpty(node.parent)) {
      return this.findNodeFromPath(node.parent, path);
    }
  }

  // 点击面包屑路径跳转
  toTargetPath($event, node, clickPath) {
    if ($event.event?.stopPropagation) {
      $event.event.stopPropagation();
    }
    //选中的是当前激活节点，直接返回
    if (node.nodeName === clickPath) {
      return;
    }
    const findNode = this.findNodeFromPath(node, clickPath);
    if (!isEmpty(findNode)) {
      this.pathNodeCheck({ node: findNode });
      this.selectionTree = [findNode];
    }
  }

  // 点击面包屑空白处变输入框
  pathClick($event): void {
    if (this.isPathSearchFlag) {
      return;
    }
    this.isPathSearchFlag = true;
    setTimeout(() => {
      this.pathInput.nativeElement.focus();
    }, 100);
  }

  // 获取指定路径下的文件
  pathSearch(): void {
    this.isPathSearchFlag = false;
    const path = this.currentPath.replace(this.copy?.resource_name, '');
    const pathArr = path.split('/');
    const nodeName = pathArr.pop();
    this.dataTable.fetchData({
      isLeaf: false,
      absolutePath: path,
      formPath: true,
      path: pathArr.join('/') || '/',
      nodeName: nodeName
    });
  }

  // 通过名称搜索文件
  searchName($event): void {
    this.dataTable.fetchData({
      ...first(this.selectionTree),
      searrchByName: $event
    });
  }

  // 叶子节点展开
  expandedChange($event): void {
    this.treeExpandedChange.emit($event);
  }

  // 点击左树节点
  pathNodeCheck($event) {
    const node = $event.node;
    this.getFilePathItems(node);
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
    // 获取子节点
    if (!node.isLeaf) {
      this.treeExpandedChange.emit(node);
    }
  }

  // 点击左树更多
  clickMoreFile($event, item) {
    if ($event.stopPropagation) {
      $event.stopPropagation();
    }
    this.treeExpandedChange.emit(item.parent);
  }

  addRootPath(path) {
    return path === '/' ? '/' : `${path}/`;
  }

  // 通过路径一层一层找到搜索的节点
  getSelectNode(node, path) {
    if (includes(this.addRootPath(path), this.addRootPath(node.absolutePath))) {
      if (!isEmpty(node.children) && path !== node.absolutePath) {
        node.expanded = true;
        each(node.children, n => {
          this.getSelectNode(n, path);
        });
      } else {
        this.selectionTree = [node];
        this.getFilePathItems(node);
      }
      this.treeData = [...this.treeData];
    }
  }

  updateTable(res, node) {
    each(res.records, (item: any) => {
      item = extendNodeParams(node, item);
    });
    this.tableData = {
      data: res.records,
      total: res.totalCount
    };
    this.total = res.totalCount;
    // 通过输入路径来搜索
    if (node.formPath) {
      this.getSelectNode(first(this.treeData), node.absolutePath);
    }
  }

  // 获取指定路径下的文件
  getTableData(filters: Filters, node) {
    if (!node) {
      node = first(this.selectionTree);
    }
    if (!node || node.isLeaf) {
      this.tableData = {
        data: [],
        total: 0
      };
      this.total = 0;
      return;
    }
    if (trim(node.searrchByName) || trim(this.searcKey)) {
      const params = {
        copyId: this.copy.uuid,
        pageNum: filters.paginator.pageIndex,
        pageSize: filters.paginator.pageSize,
        parentPath: node.absolutePath || '/',
        name: trim(this.searcKey)
      };
      this.copyControllerService
        .ListCopyCatalogsByName(params)
        .subscribe(res => this.updateTable(res, node));
    } else {
      const params = {
        copyId: this.copy.uuid,
        pageNo: filters.paginator.pageIndex,
        pageSize: filters.paginator.pageSize,
        parentPath: node.absolutePath || '/'
      };
      this.copyControllerService
        .ListCopyCatalogs(params)
        .subscribe(res => this.updateTable(res, node));
    }
  }
}
