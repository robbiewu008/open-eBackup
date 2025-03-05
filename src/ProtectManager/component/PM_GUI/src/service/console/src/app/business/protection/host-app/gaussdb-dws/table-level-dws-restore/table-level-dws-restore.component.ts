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
import { DatePipe } from '@angular/common';
import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ModalRef, TreeNode } from '@databackup/live';
import {
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  I18NService,
  RestoreFileType,
  RestoreV2LocationType
} from 'app/shared';
import {
  CopyControllerService,
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  first,
  get,
  includes,
  isEmpty,
  isUndefined,
  join,
  map,
  reject,
  set,
  size,
  slice,
  split,
  startsWith,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { ClusterRestoreComponent } from '../restore-cluster/restore-cluster.component';
import { TableRestoreComponent } from '../restore-table/restore-table.component';

@Component({
  selector: 'aui-table-level-dws-restore',
  templateUrl: './table-level-dws-restore.component.html',
  styleUrls: ['./table-level-dws-restore.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [DatePipe]
})
export class TableLevelDwsRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  @Input() restoreLevel;

  unitconst = CAPACITY_UNIT;
  RestoreFileType = RestoreFileType;
  dataMap = DataMap;
  rowCopyResPro;
  databaseOptions = [];
  database = '';
  name;

  originalFileData = [];
  treeData: TreeNode[];
  totalView = 'all';
  selectedView = 'selected';
  pathView = this.totalView;
  total = 0;
  selectedTotal = 0;
  selectionTree = [];
  schemaTableData: TableData;
  schemaTableConfig: TableConfig;
  tableData: TableData;
  tableConfig: TableConfig;
  selectionTableData = []; // 用于存放所有的已选数据
  selectedTableData: TableData;
  selectedTableConfig: TableConfig;
  tableLoading = false;

  allFileData = {}; // 用来存储之前获取过的数据，记忆所填的东西

  newLocCols: TableCols[];
  normalCols: TableCols[];

  isRestoreNew = false;
  targetParams;

  filePathItems = [];

  isSearch = false; // 用于表明当前是不是在搜索状态
  searchKey;
  searchSelectedKey;
  isSearchRestore = false; // 用于标明是不是从全局检索点进来的恢复
  isDatabase = true; // 用来表明现在左侧所选项是不是数据库，因为只有两层就这么写了
  isTable = false; // 用来标识当前副本类型是表集
  schemaName;
  currentPath; // 用于标识当前路径名，不包含父路径
  dwsNotAllowedSchemaOption = []; // 用于新位置判断重名且不可恢复schema
  formStatus = false; // 用于标识子组件form状态
  tmpResourceName = false; // 用于判断是否需要获取新位置schema
  newNameErrorTip = {
    nameRequired: this.i18n.get('common_required_label'),
    invalidName: this.i18n.get('protection_dws_new_name_error_tips_label'),
    invalidSpecialName: this.i18n.get(
      'protection_dws_new_special_name_error_tips_label'
    ),
    invalidSameName: this.i18n.get(
      'protection_dws_restore_not_same_table_name_label'
    ),
    invalidNotAllowedName: this.i18n.get(
      'protection_dws_not_allowed_schema_name_label'
    )
  };

  @ViewChild(ClusterRestoreComponent, { static: false })
  ClusterRestoreComponent: ClusterRestoreComponent;
  @ViewChild(TableRestoreComponent, { static: false })
  TableRestoreComponent: TableRestoreComponent;
  @ViewChild('tableDataTable', { static: false })
  tableDataTable: ProTableComponent;
  @ViewChild('dataSelectTable', { static: false })
  dataSelectTable: ProTableComponent;
  @ViewChild('fileTpl', { static: true }) fileTpl: TemplateRef<any>;
  @ViewChild('targetSchemaTpl', { static: true }) targetSchemaTpl: TemplateRef<
    any
  >;
  @ViewChild('targetTableTpl', { static: true }) targetTableTpl: TemplateRef<
    any
  >;
  @ViewChild('optTpl', { static: true }) optTpl: TemplateRef<any>;
  @ViewChild('targetOptTpl', { static: true }) targetOptTpl: TemplateRef<any>;
  @ViewChild('batchSchemaNamePopover', { static: false })
  batchSchemaNamePopover;
  @ViewChild('starTpl', { static: true }) starTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    public appUtilsService: AppUtilsService,
    private modal: ModalRef,
    private datePipe: DatePipe,
    private cdr: ChangeDetectorRef,
    private restoreV2Service: RestoreApiV2Service,
    private copyControllerService: CopyControllerService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.rowCopyResPro = JSON.parse(this.rowCopy.resource_properties || '{}');
    this.isTable = this.childResType === DataMap.Resource_Type.DWS_Table.value;
    this.initConfig();
    this.isSearchRestore = !!this.rowCopy.isSearchRestore;
    if (
      includes([DataMap.Resource_Type.DWS_Cluster.value], this.childResType)
    ) {
      this.getDatabaseOptions();
    } else if (
      includes(
        [
          DataMap.Resource_Type.DWS_Schema.value,
          DataMap.Resource_Type.DWS_Table.value
        ],
        this.childResType
      )
    ) {
      this.database = `/${this.rowCopyResPro.environment_name}/${this.rowCopyResPro.parent_name}`;
      this.treeData = [
        {
          label: this.rowCopyResPro.parent_name,
          nodeName: '',
          children: [],
          icon: 'aui-icon-dws-database',
          isLeaf: false,
          rootPath: this.database,
          path: this.rowCopyResPro.parent_name
        }
      ];
      this.selectionTree = [this.treeData[0]];
      this.currentPath = this.selectionTree[0].path;
      this.filePathItems = [
        {
          id: String(this.filePathItems.length + 1),
          label: this.rowCopyResPro.parent_name,
          rootPath: this.database,
          onClick: data => {
            if (this.selectionTree[0].rootPath === this.database) {
              return;
            }
            this.clickDatabase();
          }
        }
      ];
      defer(() => {
        this.configTableCols();
      });
      this.getSchema();
    }
  }

  initConfig(): void {
    this.newLocCols = [
      {
        key: 'databaseName',
        name: this.i18n.get('explore_dws_restore_database_label')
      },
      {
        key: 'targetSchema',
        name: this.i18n.get('explore_dws_restore_schema_label'),
        cellRender: this.targetSchemaTpl,
        thExtra: this.starTpl
      },
      {
        key: 'newName',
        name: this.i18n.get('explore_dws_restore_name_label'),
        cellRender: this.targetTableTpl,
        thExtra: this.starTpl
      }
    ];
    this.normalCols = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        cellRender: this.fileTpl
      },
      {
        key: 'modifyTime',
        name: this.i18n.get('protection_last_modifyed_label')
      }
    ];
    this.schemaTableConfig = {
      table: {
        async: false,
        compareWith: 'rootPath',
        columns: this.normalCols,
        scroll: { y: '530px' },
        virtualScroll: true,
        colDisplayControl: false,
        trackByFn: (_, item) => {
          return item.rootPath;
        }
      }
    };
    const page: any = {
      mode: 'simple',
      pageSizeOptions: [20, 50, 100, 200]
    };
    this.tableConfig = {
      table: {
        compareWith: 'rootPath',
        columns: this.normalCols,
        scroll: { y: '520px' },
        virtualScroll: true,
        colDisplayControl: false,
        size: 'large',
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: selection => {
          this.selectionTableData = selection;
          each(this.tableData.data, item => {
            item.inputDisabled = !find(this.selectionTableData, {
              rootPath: item.rootPath
            });
          });
          this.syncSelectedData();
          if (!this.isTable) {
            each(this.selectionTableData, item => {
              this.validTargetSchema(item);
            });
          }
          this.disabledOkbtn();
        },
        fetchData: (filter: Filters, args) => {
          if (this.isSearch) {
            this.searchName(this.searchKey, filter.paginator.pageIndex);
          } else {
            this.checkSchema(
              { node: this.selectionTree[0] },
              filter.paginator.pageIndex
            );
          }
        },
        trackByFn: (_, item) => {
          return item.rootPath;
        }
      },
      pagination: {
        mode: 'simple',
        pageSize: 200,
        showPageSizeOptions: false
      }
    };
    this.selectedTableConfig = {
      table: {
        compareWith: 'rootPath',
        async: false,
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label'),
            cellRender: this.fileTpl
          },
          {
            key: 'modifyTime',
            name: this.i18n.get('protection_last_modifyed_label'),
            cellRender: this.optTpl
          }
        ],
        size: 'large',
        scroll: { y: '500px' },
        colDisplayControl: false,
        trackByFn: (_, item) => {
          return item.rootPath;
        }
      },
      pagination: page
    };
  }

  deleteNode(node) {
    this.selectionTableData = reject(
      this.selectionTableData,
      item => item.rootPath === node.rootPath
    );
    each(this.allFileData[node.parentPath], item => {
      item.inputDisabled = !find(this.selectionTableData, {
        rootPath: item.rootPath
      });
    });
    this.syncSelectedData();
    this.disabledOkbtn();
  }

  tabChange() {
    if (this.pathView === this.totalView) {
      setTimeout(() => {
        this.tableDataTable?.setSelections(this.selectionTableData);
        // 重新初始化会导致页面跳到第一页但是数据没变，没找到办法前重新获取第一页数据
        this.tableDataTable?.fetchData();
      });
    } else {
      setTimeout(() => {
        this.searchSelectedName(this.searchSelectedKey);
      });
    }
  }

  getDatabaseOptions() {
    this.appUtilsService.getResourceByRecursion(
      {
        copyId: this.rowCopy.uuid,
        parentPath: `/${get(this.rowCopyResPro, 'name')}`
      },
      params => this.copyControllerService.ListCopyCatalogs(params),
      resource => {
        if (isEmpty(resource)) {
          return;
        }
        this.databaseOptions = map(resource, item => {
          return {
            ...item,
            label: item.path,
            key: item.path,
            value: `/${get(this.rowCopyResPro, 'name')}/${item.path}`,
            rootPath: `/${get(this.rowCopyResPro, 'name')}/${item.path}`,
            isLeaf: true
          };
        });
        this.database = first(this.databaseOptions)?.value;
        this.changeDatabase(this.database);
        this.cdr.detectChanges();
      }
    );
  }

  beforeSelected = item => {
    if (this.selectionTree[0].rootPath === item.rootPath) {
      return false;
    }
  };

  changeDatabase(opt) {
    // 集群的表级恢复每次切换数据库就相当于清空并初始化数据
    this.isDatabase = true;
    const tmpDatabase = find(this.databaseOptions, { rootPath: opt });
    this.treeData = [
      {
        ...tmpDatabase,
        nodeName: '',
        children: [],
        icon: 'aui-icon-dws-database',
        isLeaf: false
      }
    ];
    this.filePathItems = [
      {
        id: String(this.filePathItems.length + 1),
        label: tmpDatabase.label,
        rootPath: tmpDatabase.rootPath,
        onClick: data => {
          if (this.selectionTree[0].rootPath === this.database) {
            return;
          }
          this.clickDatabase();
        }
      }
    ];
    this.selectionTree = [this.treeData[0]];
    this.name = '';
    this.selectionTableData = [];
    this.tableDataTable?.setSelections(this.selectionTableData);
    this.total = 0;
    this.allFileData = {};
    this.syncSelectedData();
    this.filePathItems = [...this.filePathItems];
    this.configTableCols();
    this.getSchema();
    this.disabledOkbtn();
  }

  clickDatabase() {
    // 不管是在树里点还是在路径里点都是一个效果，把右表的数据切换成schema并且切换表项
    this.isDatabase = true;
    this.configTableCols();
    this.selectionTree = [this.treeData[0]];
    defer(() => {
      this.schemaTableData = {
        data: cloneDeep(this.treeData[0].children).map(item => {
          return assign(item, {
            disabled: true
          });
        }),
        total: size(this.treeData[0].children)
      };
      this.total = 0;
    });
    this.filePathItems.pop();
    this.filePathItems = [...this.filePathItems];
    this.cdr.detectChanges();
  }

  syncSelectedData() {
    // 同步已选表格中的选项
    this.selectedTotal = size(this.selectionTableData);
    this.selectedTableData = {
      data: this.selectionTableData,
      total: size(this.selectionTableData)
    };
  }

  getSchema() {
    // 集群开局选中数据库后自动获取下面的schema并且左右两边都加上
    this.tableLoading = true;
    this.appUtilsService.getResourceByRecursion(
      { copyId: this.rowCopy.uuid, parentPath: this.database },
      params => this.copyControllerService.ListCopyCatalogs(params),
      resource => {
        this.treeData[0].children = this.parseData(resource).map(item => {
          return assign(item, {
            disabled: false
          });
        });
        this.originalFileData = [...this.originalFileData];
        this.treeData = [...this.treeData];
        this.schemaTableData = {
          data: cloneDeep(this.treeData[0].children).map(item => {
            return assign(item, {
              disabled: true
            });
          }),
          total: size(this.treeData[0].children)
        };
        this.tableLoading = false;
        this.disabledOkbtn();
        this.cdr.detectChanges();
      }
    );
  }

  getNewLocationSchema(params) {
    if (
      (!isEmpty(this.dwsNotAllowedSchemaOption) &&
        this.tmpResourceName === params?.resource?.name) ||
      !params?.resource?.name
    ) {
      return;
    }
    this.tmpResourceName = params.resource.name;
    const extParams = {
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.DWS_Schema.value,
        environment: { name: [['~~'], params.resource.name] }
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.dwsNotAllowedSchemaOption = [];
        each(resource, item => {
          if (get(item, 'extendInfo.isAllowRestore', 'false') === 'false') {
            this.dwsNotAllowedSchemaOption.push(
              ...item.extendInfo.table.split(',')
            );
          }
        });
        if (!this.isTable) {
          each(this.selectionTableData, item => {
            this.validTargetSchema(item);
          });
          this.cdr.detectChanges();
        }
      }
    );
  }

  clickSchema(item) {
    if (item.type !== RestoreFileType.Directory) {
      return;
    }
    this.treeData[0].expanded = true;
    this.treeData = [...this.treeData];
    this.selectionTree = [item];
    this.pathNodeCheck({ node: item });
  }

  pathNodeCheck(node, pageIndex = 0) {
    this.searchKey = '';
    this.isSearch = false;
    this.isDatabase =
      !isEmpty(this.selectionTree) &&
      this.selectionTree[0].rootPath === this.database;
    this.currentPath = this.selectionTree[0].path;
    if (this.isDatabase) {
      this.clickDatabase();
    } else {
      this.checkSchema(node, pageIndex);
    }
  }

  private checkSchema(node: any, pageIndex: number) {
    const tmpItem = {
      id: String(this.filePathItems.length + 1),
      label: node.node.label,
      icon: null
    };
    this.configTableCols();
    if (size(this.filePathItems) === 2) {
      this.filePathItems[1] = tmpItem;
    } else {
      this.filePathItems.push(tmpItem);
    }
    this.filePathItems = [...this.filePathItems];
    this.tableLoading = true;
    this.copyControllerService
      .ListCopyCatalogs({
        pageNo: pageIndex,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        copyId: this.rowCopy.uuid,
        parentPath: node.node.rootPath
      })
      .subscribe((res: any) => {
        const dataArray = this.parseData(res.records);
        this.tableData = {
          data: dataArray,
          total: res.totalCount
        };
        this.tableLoading = false;
        // 把获取到的数据塞到总数据里
        this.updateNewData(dataArray);
        // 这一步是为了把所有选中项都设置进表里
        this.tableDataTable?.setSelections(this.selectionTableData);
        this.syncSelectedData();
        this.total = res.totalCount;
        this.cdr.detectChanges();
      });
  }

  private updateNewData(dataArray: any) {
    each(dataArray, (item: any) => {
      if (!isEmpty(this.allFileData[item.parentPath])) {
        if (
          !find(this.allFileData[item.parentPath], {
            rootPath: item.rootPath
          })
        ) {
          this.allFileData[item.parentPath].push(item);
        }
      } else {
        set(this.allFileData, item.parentPath, dataArray);
      }
    });
  }

  searchSelectedName(e) {
    this.dataSelectTable.filterChange({
      caseSensitive: false,
      filterMode: 'contains',
      key: 'name',
      value: trim(e)
    });
  }

  searchName(e, pageIndex = 0) {
    if (!e) {
      this.isSearch = false;
      this.configTableCols();

      if (this.isDatabase) {
        // 左边选的是数据库就把树下的节点再重新赋值过来就行了
        this.schemaTableData = {
          data: cloneDeep(this.treeData[0].children).map(item => {
            return assign(item, {
              disabled: true
            });
          }),
          total: size(this.treeData[0].children)
        };
      } else {
        // 如果左边是schema，就得转而获取第一页的
        if (this.tableDataTable.page._isFirstIndex) {
          this.tableDataTable.fetchData();
        } else {
          this.tableDataTable.page.jumpToFisrtPage();
        }
      }
    } else {
      this.isSearch = true;
      let path = '/';
      this.configTableCols();

      if (
        includes(
          [
            DataMap.Resource_Type.DWS_Schema.value,
            DataMap.Resource_Type.DWS_Table.value
          ],
          this.rowCopy.resource_sub_type
        )
      ) {
        path = `/${this.rowCopyResPro.environment_name}/${this.rowCopyResPro.parent_name}`;
      } else {
        const database = find(
          this.databaseOptions,
          val => val.value === this.database
        );
        path = `/${get(this.rowCopyResPro, 'name')}/${database.path}${
          this.isDatabase ? '' : '/' + this.selectionTree[0].path
        }`;
      }
      const params = {
        pageNum: pageIndex,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        parentPath: path,
        copyId: this.rowCopy.uuid,
        name: e
      };
      if (!this.isDatabase) {
        assign(params, {
          conditions: JSON.stringify({
            type: 'table'
          })
        });
      }
      this.tableLoading = true;
      this.copyControllerService
        .ListCopyCatalogsByName(params)
        .subscribe(res => {
          this.tableLoading = false;
          this.tableData = {
            data: this.parseData(res.records),
            total: res.totalCount
          };
          this.total = res.totalCount;
          if (this.isDatabase) {
            this.total -= size(
              this.treeData[0].children.filter(item =>
                item.name.includes(this.searchKey)
              )
            );
          }
          this.cdr.detectChanges();
          this.updateNewData(this.tableData.data);
          this.tableDataTable?.setSelections(this.selectionTableData);
        });
    }
  }

  parseData(res) {
    return res.map(item => {
      assign(item, {
        name: get(item, 'path'),
        label: get(item, 'path'),
        isLeaf: true,
        children: null,
        disabled: item.type === 'schema',
        rootPath: `${item.parentPath}/${get(item, 'path')}`,
        type:
          item.type === 'schema'
            ? RestoreFileType.Directory
            : RestoreFileType.File,
        icon: item.type === 'schema' ? 'aui-icon-dws-schema' : 'aui-icon-table'
      });
      if (item.type !== RestoreFileType.Directory) {
        // 这里取得currentPath要改
        const previousData = find(
          this.allFileData[item.parentPath],
          data => data.rootPath === `${item.parentPath}/${get(item, 'path')}`
        );
        if (previousData) {
          return previousData;
        } else {
          assign(item, {
            databaseName:
              find(this.databaseOptions, {
                rootPath: this.database
              })?.label || this.rowCopyResPro.parent_name,
            targetSchema: item.parentPath.split('/')[3],
            targetTableName: get(item, 'path'),
            inputDisabled: !find(this.selectionTableData, {
              rootPath: `${item.parentPath}/${get(item, 'path')}`
            })
          });
        }
      }
      return item;
    });
  }

  formChange($event) {
    if (this.childResType === DataMap.Resource_Type.DWS_Cluster.value) {
      this.isRestoreNew =
        !isUndefined(this.ClusterRestoreComponent) &&
        this.ClusterRestoreComponent.formGroup.get('restoreLocation').value ===
          RestoreV2LocationType.NEW;
      this.formStatus =
        !isUndefined(this.ClusterRestoreComponent) &&
        this.ClusterRestoreComponent?.formGroup.valid;
      // 恢复至新位置可以选取目标Schema,但是是表那一层
    } else if (
      includes(
        [
          DataMap.Resource_Type.DWS_Schema.value,
          DataMap.Resource_Type.DWS_Table.value
        ],
        this.childResType
      )
    ) {
      this.isRestoreNew =
        !isUndefined(this.TableRestoreComponent) &&
        this.TableRestoreComponent.formGroup.get('restoreTo').value ===
          RestoreV2LocationType.NEW;
      this.formStatus =
        !isUndefined(this.TableRestoreComponent) &&
        this.TableRestoreComponent?.formGroup.valid;
    }
    if (!this.isRestoreNew) {
      // dws集群和schema如果新位置恢复，需要去检测填写的schema是不是重名且不允许恢复
      this.dwsNotAllowedSchemaOption = [];
    }
    this.configTableCols();
    this.disabledOkbtn();
  }

  private configTableCols() {
    // 右边是表时动态切换所展示的表项
    // dws表集不涉及这个
    if (this.isTable) {
      return;
    }
    const tmpTableColLength = size(this.tableConfig.table.columns);
    if (!this.isRestoreNew || this.isDatabase) {
      this.tableConfig.table.columns = [...this.normalCols];
      this.tableConfig.table.scroll = { y: '520px' };
    }
    if (
      this.isRestoreNew &&
      (!this.isDatabase || (this.isDatabase && this.isSearch))
    ) {
      this.tableConfig.table.columns = [...this.normalCols, ...this.newLocCols];
      this.tableConfig.table.scroll = { y: '495px' };
    }
    if (this.isRestoreNew && this.formStatus) {
      this.getNewLocationSchema(this.getParams());
    }

    if (
      !isUndefined(this.tableDataTable) &&
      tmpTableColLength !== size(this.tableConfig.table.columns)
    ) {
      const { data, total } = this.tableData;
      this.tableDataTable.reinit();
      // 因为reinit会清空原本表格数据，所以要重新赋值
      defer(() => {
        this.tableData = {
          data: data,
          total: total
        };
      });
    }

    // 不深拷贝会导致原本tableconfig里面也变化，但是深拷贝又会导致特别慢，只能新写了
    const tmpNormalCols = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        cellRender: this.fileTpl
      },
      {
        key: 'modifyTime',
        name: this.i18n.get('protection_last_modifyed_label'),
        cellRender: this.optTpl
      }
    ];

    const tmpNewCols = [
      {
        key: 'databaseName',
        name: this.i18n.get('explore_dws_restore_database_label')
      },
      {
        key: 'targetSchema',
        name: this.i18n.get('explore_dws_restore_schema_label'),
        cellRender: this.targetSchemaTpl,
        thExtra: this.starTpl
      },
      {
        key: 'newName',
        name: this.i18n.get('explore_dws_restore_name_label'),
        cellRender: this.targetOptTpl,
        thExtra: this.starTpl
      }
    ];

    // 已选的表格只根据是否是新位置决定
    const tmpColLength = size(this.selectedTableConfig.table.columns);
    if (this.isRestoreNew) {
      this.selectedTableConfig.table.columns = [
        ...this.normalCols,
        ...tmpNewCols
      ];
      this.selectedTableConfig.table.columns[4].cellRender = this.targetOptTpl;
    } else {
      this.selectedTableConfig.table.columns = [...tmpNormalCols];
      this.selectedTableConfig.table.columns[1].cellRender = this.optTpl;
    }
    if (
      !isUndefined(this.dataSelectTable) &&
      tmpColLength !== size(this.selectedTableConfig.table.columns)
    ) {
      const { data, total } = this.selectedTableData;
      this.dataSelectTable.reinit();
      // 因为reinit会清空原本表格数据，所以要重新赋值
      defer(() => {
        this.selectedTableData = {
          data: data,
          total: total
        };
      });
    }
  }

  batchSchemaNamePopoverHide() {
    this.batchSchemaNamePopover.hide();
  }

  batchSetTargetSchema() {
    this.batchSchemaNamePopoverHide();
    if (!trim(this.schemaName)) {
      return;
    }

    each(this.selectionTableData, item => {
      if (this.isDatabase) {
        // 数据库说明是在搜索状态下设置，对于所有符合搜索条件的数据去生效
        if (item.name.includes(this.searchKey)) {
          item.targetSchema = this.schemaName;
          // 这里相当于批量设置了一次，也得同步到存储数据里
          find(this.allFileData[item.parentPath], {
            rootPath: item.rootPath
          }).targetSchema = item.targetSchema;
          this.validTargetSchema(item);
        }
      } else {
        // 非数据库则是在某个层级下面，根据左树作为前缀来筛选
        if (startsWith(item.rootPath, this.selectionTree[0].rootPath)) {
          item.targetSchema = this.schemaName;
          // 这里相当于批量设置了一次，也得同步到存储数据里
          find(this.allFileData[item.parentPath], {
            rootPath: item.rootPath
          }).targetSchema = item.targetSchema;
          this.validTargetSchema(item);
        }
      }
    });
    this.schemaName = '';
    this.disabledOkbtn();
  }

  validTargetSchema(item) {
    const value = item.targetSchema;
    const reg = /^[a-zA-Z\_\$\#]{1}[a-zA-Z0-9\_\$\#]{0,62}$/;

    const resProperties = JSON.parse(this.rowCopy.resource_properties);
    let tmpParentName =
      this.childResType === DataMap.Resource_Type.DWS_Schema.value
        ? resProperties.parent_name
        : find(this.databaseOptions, { value: this.database }).label;

    if (!trim(value)) {
      item.schemaInvalid = true;
      item.schemaErrorTips = this.newNameErrorTip.nameRequired;
    } else if (
      this.dwsNotAllowedSchemaOption.some(
        val => val === `${tmpParentName}/${item.targetSchema}`
      )
    ) {
      item.schemaInvalid = true;
      item.schemaErrorTips = this.newNameErrorTip.invalidNotAllowedName;
    } else if (startsWith(trim(value), 'PG_')) {
      item.schemaInvalid = true;
      item.schemaErrorTips = this.newNameErrorTip.invalidSpecialName;
    } else if (!reg.test(value)) {
      item.schemaInvalid = true;
      item.schemaErrorTips = this.newNameErrorTip.invalidName;
    } else {
      item.schemaInvalid = false;
    }
    this.disabledOkbtn();

    // 输入之后也要同步到该数据的存储版本里去
    find(this.allFileData[item.parentPath], {
      rootPath: item.rootPath
    }).targetSchema = item.targetSchema;
  }

  validTargetTable(item) {
    const value = item.targetTableName;

    const reg = /^[a-zA-Z\_\$\#]{1}[a-zA-Z0-9\_\$\#]{0,62}$/;
    const tmpSameSchemaArray = this.selectionTableData.filter(
      val =>
        val?.targetSchema === item.targetSchema &&
        val.rootPath !== item.rootPath
    );

    if (!trim(value)) {
      item.tableInvalid = true;
      item.tableErrorTips = this.newNameErrorTip.nameRequired;
    } else if (!reg.test(value)) {
      item.tableInvalid = true;
      item.tableErrorTips = this.newNameErrorTip.invalidName;
    } else if (
      find(tmpSameSchemaArray, { targetTableName: item.targetTableName })
    ) {
      item.tableInvalid = true;
      item.tableErrorTips = this.newNameErrorTip.invalidSameName;
    } else {
      item.tableInvalid = false;
    }

    this.disabledOkbtn();

    find(this.allFileData[item.parentPath], {
      rootPath: item.rootPath
    }).targetTableName = item.targetTableName;
  }

  disabledOkbtn() {
    const formStatus =
      this.childResType === DataMap.Resource_Type.DWS_Cluster.value
        ? !isUndefined(this.ClusterRestoreComponent) &&
          this.ClusterRestoreComponent.formGroup.valid
        : !isUndefined(this.TableRestoreComponent) &&
          this.TableRestoreComponent.formGroup.valid;
    this.modal.getInstance().lvOkDisabled =
      !formStatus ||
      isEmpty(this.selectionTableData) ||
      (this.isRestoreNew &&
        !this.isTable &&
        !!find(
          this.selectionTableData,
          item => item.tableInvalid || item.schemaInvalid
        ));
  }

  getPaths(paths) {
    return map(
      filter(paths, item => {
        return !isEmpty(item.rootPath);
      }),
      item => {
        return {
          rootPath: item.rootPath,
          targetSchema: item.targetSchema || '',
          targetTableName: item.targetTableName || ''
        };
      }
    );
  }

  getParams() {
    switch (this.childResType) {
      case DataMap.Resource_Type.DWS_Cluster.value:
        return this.ClusterRestoreComponent.getTargetParams();
      default:
        return this.TableRestoreComponent.getTargetParams();
    }
  }

  getType(path) {
    if (size(path) === 0) {
      return DataMap.Resource_Type.DWS_Cluster.value;
    }
    if (size(path) === 1) {
      return DataMap.Resource_Type.DWS_Database.value;
    }
    if (size(path) === 2) {
      return DataMap.Resource_Type.DWS_Schema.value;
    }
    return DataMap.Resource_Type.DWS_Table.value;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const selection = this.selectionTableData;
      const allParams = this.getParams();
      const params = allParams?.requestParams || {};
      assign(params, {
        subObjects: this.rowCopy.isSearchRestore
          ? [this.rowCopy.searchRestorePath]
          : map(this.getPaths(cloneDeep(selection)), val => {
              const path = slice(split(val.rootPath, '/'), 2);
              return JSON.stringify({
                name: this.getName(allParams, params, path, val),
                type: 'Database',
                subType: this.getType(path),
                extendInfo: {
                  oldName: join(path, '/')
                }
              });
            })
      });
      if (this.isTable && this.isRestoreNew) {
        delete params.extendInfo.database;
      }
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
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
    });
  }

  private getName(allParams: any, params: any, path: string[], item) {
    if (this.isTable && this.isRestoreNew) {
      return `${split(
        `${allParams.resource.name}/${params?.extendInfo.database}`,
        '/'
      )[1] || path[0]}/${path[1]}/${path[2]}`;
    }
    if (!this.isTable && this.isRestoreNew) {
      return `${path[0]}/${item.targetSchema}/${item.targetTableName}`;
    }
    return join(path, '/');
  }

  getTargetPath() {
    return {
      tips: this.i18n.get('common_table_label'),
      targetPath:
        this.childResType === DataMap.Resource_Type.DWS_Cluster.value
          ? this.ClusterRestoreComponent.getTargetParams().resource.name
          : this.TableRestoreComponent.getTargetParams().resource.name
    };
  }
}
