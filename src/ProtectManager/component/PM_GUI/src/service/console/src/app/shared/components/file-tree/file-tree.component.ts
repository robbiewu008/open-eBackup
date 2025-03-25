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
  OnDestroy,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { GlobalService, I18NService } from 'app/shared/services';
import { Subject, timer } from 'rxjs';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from '../pro-table';
import { MenuItem, TreeNode } from '@iux/live';
import {
  assign,
  cloneDeep,
  defer,
  each,
  first,
  get,
  includes,
  isEmpty,
  map,
  reject,
  set,
  size,
  take,
  trim
} from 'lodash';
import { CopiesService, CopyControllerService } from 'app/shared/api/services';
import {
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  RestoreFileType
} from 'app/shared/consts';
import { extendNodeParams } from 'app/shared';
import { finalize, switchMap, takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-file-tree',
  templateUrl: './file-tree.component.html',
  styleUrls: ['./file-tree.component.less']
})
export class FileTreeComponent implements OnInit, OnDestroy {
  @Input() copy;
  @Input() treeData: TreeNode[];
  @Output() tableSelectionChange = new EventEmitter<any>();
  @Output() treeExpandedChange = new EventEmitter<any>();

  title = this.i18n.get('explore_object_need_restore_label');
  unitconst = CAPACITY_UNIT;
  restoreFileType = RestoreFileType;
  protected readonly DataMap = DataMap;
  timeSub$;
  destroy$ = new Subject<boolean>();
  _includes = includes;

  totalView = 'all';
  selectedView = 'selected';
  pathView = this.totalView;
  total = 0;
  selectedTotal = 0;
  queryTaskTimeout = CommonConsts.TIME_INTERVAL / 2;
  filePathItems: MenuItem[] = [];

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
  // 搜索是否超过1000条
  searchMax = false;

  hideItems = [];
  maxBreadNum = 5;

  // 索引失败和未索引提示
  showIndexTip = false;

  // 路径选择方式
  modeMap = {
    fromTree: '1',
    fromTag: '2'
  };
  pathMode = this.modeMap.fromTree;
  tableLoading = false;
  // 修改表格实现，只能一页一页查，不能跳过页码查询，通过滚动鼠标滑轮加载下一页
  // 初始页
  startPage = CommonConsts.PAGE_START;
  // 是否已查询完毕的标识
  hasQueryAll = false;
  // 查询下一页需要上一页最后一条数据的sort值
  searchAfter = [];
  // 接口锁定，锁定时不再触发同一接口
  lockFileRequest = false;

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
    private globalService: GlobalService,
    private copiesApiService: CopiesService,
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
    this.showIndexTip = includes(
      [
        DataMap.CopyData_fileIndex.unIndexed.value,
        DataMap.CopyData_fileIndex.deletedFailed.value
      ],
      this.copy?.indexed
    );
  }

  pathModeChange() {
    if (this.pathMode === this.modeMap.fromTree) {
      this.tableSelectionChange.emit(
        cloneDeep(this.selectTableData?.data) || []
      );
      defer(() => this.dataTable?.setSelections(this.selectionTableData));
    } else {
      this.tableSelectionChange.emit([]);
    }
  }

  pathChange(path) {
    this.tableSelectionChange.emit([...path]);
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
    this.filePathItems = [...breadcrumbItems];
    if (this.filePathItems?.length > this.maxBreadNum) {
      const hideItem = this.filePathItems.splice(
        this.maxBreadNum - 2,
        this.filePathItems.length - this.maxBreadNum + 1
      );
      this.hideItems = [...hideItem];
      this.filePathItems.splice(this.maxBreadNum - 2, 0, {
        id: 'more_bread',
        label: '...',
        items: this.hideItems
      });
    } else {
      this.hideItems = [];
    }
    this.filePathItems = [...this.filePathItems];
    this.currentPath = map(breadcrumbItems, 'label').join('/');
  }

  // 如果父节点选了，子节点选中自动忽略
  mergeSelection() {
    let mergePaths = [];
    let parentPaths = map(this.selectionTableData, 'absolutePath');
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
    this.tableSelectionChange.emit(cloneDeep(mergePaths));
  }

  tabChange() {
    if (this.pathView === this.totalView) {
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
        async: false,
        compareWith: 'absolutePath',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scroll: { y: '530px' },
        virtualScroll: true,
        colDisplayControl: false,
        selectionChange: selection => {
          this.selectionTableData = selection;
          this.setSelection();
        },
        scrollEnd: () => this.scrollEnd(),
        trackByFn: (_, item) => {
          return item.absolutePath;
        }
      },
      pagination: null
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
    // 重置页码和状态
    this.resetQueryStatus();
    this.isPathSearchFlag = false;
    const path = this.currentPath.replace(this.copy?.resource_name, '');
    const pathArr = path.split('/');
    const nodeName = pathArr.pop();
    this.getTableData({
      isLeaf: false,
      absolutePath: path,
      formPath: true,
      path: pathArr.join('/') || '/',
      nodeName: nodeName
    });
  }

  // 通过名称搜索文件
  searchName($event): void {
    // 重置页码和状态
    this.resetQueryStatus();
    if (this.pathView === this.totalView) {
      this.getTableData({
        ...first(this.selectionTree),
        searrchByName: $event
      });
    } else {
      this.dataSelectTable.filterChange({
        caseSensitive: false,
        filterMode: 'contains',
        key: 'name',
        value: trim($event)
      });
    }
  }

  // 叶子节点展开
  expandedChange($event): void {
    this.treeExpandedChange.emit($event);
  }

  // 点击左树节点
  pathNodeCheck($event) {
    if (
      [
        DataMap.Browse_LiveMount_Status.unmount.value,
        DataMap.Browse_LiveMount_Status.mounting.value,
        DataMap.Browse_LiveMount_Status.mountFail.value
      ].includes(this.copy.browse_mounted) &&
      this.copy.indexed !== DataMap.CopyData_fileIndex.indexed.value
    ) {
      return;
    }
    const node = $event.node;
    this.getFilePathItems(node);
    // 重置页码和状态
    this.resetQueryStatus();
    // 获取右边表格数据，当前路径下子路径
    if (this.pathView === this.totalView) {
      this.getTableData(node);
    } else {
      this.pathView = this.totalView;
      setTimeout(() => {
        this.getTableData(node);
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

  updateTable(res, node, byScroll = false) {
    each(res.records, (item: any) => {
      item = extendNodeParams(node, item);
    });

    // 如果是通过滚动加载，需要把数据合并，否则不用
    if (byScroll) {
      this.tableData = {
        data: [...this.tableData.data, ...res.records],
        total: res.totalCount,
        keepScroll: true
      };
    } else {
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
    }
    this.total = res.totalCount;
    // 通过输入路径来搜索
    if (node.formPath) {
      this.getSelectNode(first(this.treeData), node.absolutePath);
    }
  }

  // 获取指定路径下的文件
  getTableData(node, byScroll = false) {
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
        pageNum: this.startPage,
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        parentPath: node.absolutePath || '/',
        name: trim(this.searcKey),
        akLoading: false
      };
      // 已索引的副本翻页时需要传上一条的sort
      if (byScroll && !isEmpty(this.searchAfter)) {
        assign(params, {
          searchAfter: this.searchAfter
        });
      }
      this.tableLoading = true;
      this.copyControllerService
        .ListCopyCatalogsByName(params)
        .pipe(
          finalize(() => {
            this.tableLoading = false;
            this.lockFileRequest = false;
          })
        )
        .subscribe(res => {
          this.searchMax = res.totalCount >= 1000;
          // 更新查询滚动查询状态
          this.updateSort(res.records, res.totalCount);
          this.updateTable(res, node, byScroll);
        });
    } else {
      this.searchMax = false;
      const params = {
        copyId: this.copy.uuid,
        pageNo: this.startPage,
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        parentPath: node.absolutePath || '/',
        akLoading: false
      };
      // 已索引的副本翻页时需要传上一条的sort
      if (byScroll && !isEmpty(this.searchAfter)) {
        assign(params, {
          searchAfter: this.searchAfter
        });
      }
      this.tableLoading = true;
      this.copyControllerService
        .ListCopyCatalogs(params)
        .pipe(
          finalize(() => {
            this.tableLoading = false;
            this.lockFileRequest = false;
          })
        )
        .subscribe(res => {
          // 更新查询滚动查询状态
          this.updateSort(res.records, res.totalCount);
          this.updateTable(res, node, byScroll);
        });
    }
  }

  // 更新标记和查询状态
  updateSort(records, totalCount) {
    // 记录最后一条数据的sort
    this.searchAfter = records[records.length - 1]?.sort;
    // 记录是否已经查询完，滚动是否还需要查询
    this.hasQueryAll =
      this.startPage === Math.floor(totalCount / CommonConsts.PAGE_SIZE_MAX);
  }

  // 重置标记和查询状态
  resetQueryStatus() {
    this.searchAfter = [];
    this.startPage = CommonConsts.PAGE_START;
    this.hasQueryAll = false;
    this.lockFileRequest = false;
  }

  // 滚动到底部，触发查询下一页
  scrollEnd() {
    if (!this.hasQueryAll && !this.lockFileRequest) {
      this.startPage++;
      this.lockFileRequest = true;
      this.getTableData(first(this.selectionTree), true);
    }
  }

  fileLevelExploreMount() {
    this.copy.browse_mounted = DataMap.Browse_LiveMount_Status.mounting.value; // 点击创建时，手动设置为挂载中
    this.copyControllerService
      .OpenCopyGuestSystem({
        copyId: this.copy.uuid,
        akLoading: false
      })
      .subscribe();
    this.globalService.emitStore({
      action: 'fileLevelExploreMount',
      state: 'mount'
    });
    setTimeout(() => this.pollQueryCopy(), this.queryTaskTimeout);
  }

  pollQueryCopy() {
    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        takeUntil(this.destroy$),
        switchMap(index => {
          const params = {
            pageSize: 1,
            pageNo: 0,
            conditions: JSON.stringify({
              uuid: this.copy.uuid
            })
          };
          return this.copiesApiService.queryResourcesV1CopiesGet({
            ...params,
            akLoading: false
          });
        })
      )
      .subscribe(res => {
        const copy = first(res.items);
        if (
          get(copy, 'browse_mounted') !==
          DataMap.Browse_LiveMount_Status.mounting.value
        ) {
          // 非挂载中，说明挂载操作已经结束。
          this.destroy$.next(true);
          this.destroy$.complete();
          this.globalService.emitStore({
            // 发布消息更新副本列表
            action: 'fileLevelExploreMount',
            state: 'mount'
          });
          setTimeout(() => {
            this.pathNodeCheck({ node: this.selectionTree[0] });
          });
        }
        set(
          this.copy,
          'browse_mounted',
          get(
            copy,
            'browse_mounted',
            DataMap.Browse_LiveMount_Status.unmount.value
          )
        );
      });
  }

  ngOnDestroy() {
    setTimeout(() => {
      if (this.timeSub$) {
        this.timeSub$.unsubscribe();
      }
    }, this.queryTaskTimeout);
    this.destroy$.next(true);
    this.destroy$.complete();
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }
  }
}
