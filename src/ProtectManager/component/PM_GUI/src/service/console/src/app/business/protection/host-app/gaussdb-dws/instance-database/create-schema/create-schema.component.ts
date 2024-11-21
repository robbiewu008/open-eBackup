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
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService, ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  compareVersion,
  DataMap,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { TableConfig } from 'app/shared/components/pro-table';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  get,
  includes,
  isEmpty,
  isNumber,
  isUndefined,
  join,
  map,
  reject,
  set,
  size,
  slice,
  startsWith,
  uniq,
  uniqBy
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-create-schema',
  templateUrl: './create-schema.component.html',
  styleUrls: ['./create-schema.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class CreateSchemaComponent implements OnInit {
  set;
  item;
  colon = false;
  optsConfig;
  optItems = [];
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
  isSearch = false; // 是否处于搜索状态
  consTreeData = []; // 用于树表搜索时的展示,不然过滤后找不回原树了

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  @ViewChild('pageS', { static: false }) pageS;
  @ViewChild('pageA', { static: false }) pageA;
  @ViewChild('namePopover', { static: false }) namePopover;

  @Input() type;
  @Input() data;
  tableData: any;
  constructor(
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private message: MessageService,
    private cdr: ChangeDetectorRef,
    private virtualScroll: VirtualScrollService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getclusterOptions();
    this.modifyTable();
    this.set =
      this.type === DataMap.Resource_Type.DWS_Table.value
        ? this.i18n.get('common_table_label')
        : this.i18n.get('Schemas');
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
      database: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      set: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
    this.listenForm();

    if (this.data) {
      if (this.type === DataMap.Resource_Type.DWS_Table.value) {
        this.formGroup.patchValue({
          name: this.data.name,
          cluster: this.data.environment.uuid,
          database: this.data.parentName,
          set: this.data.extendInfo.table.split(',')
        });
      } else {
        this.formGroup.patchValue({
          name: this.data.name,
          cluster: this.data.environment.uuid,
          database: this.data.parentName,
          set: this.data.extendInfo.table.split(',')
        });
      }
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => {
      this.disableOkBtn();
    });
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      this.formGroup.get('database').setValue('');
      this.getDatabaseOptions(res);
      this.treeData = [];
      this.consTreeData = [];
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
      this.selectionSchema = [];
      this.selection = [];
      this.formGroup.get('set').setValue(this.selectedTableData);
    });
  }

  modifyTable() {
    if (!this.data || this.type !== DataMap.Resource_Type.DWS_Table.value) {
      return;
    }

    // 这里如果上次选中的是schema就该level为0
    this.selectedTableData = map(
      this.data?.extendInfo?.table.split(','),
      item => {
        return {
          name: item,
          level: item.split('/').length === 2 ? 0 : 1
        };
      }
    );

    this.formGroup.get('set').setValue(this.selectedTableData);
  }

  getclusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.DWS_Cluster.value
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

        if (this.type === DataMap.Resource_Type.DWS_Schema.value) {
          this.clusterOptions = filter(
            clusterArray,
            item => compareVersion(item.version, '8.1.2') !== -1
          );
        } else {
          this.clusterOptions = clusterArray;
        }
        this.cdr.detectChanges();
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
        subType: DataMap.Resource_Type.DWS_Database.value,
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
        this.cdr.detectChanges();
        return;
      }
      this.getDatabaseOptions(clusterId, recordsTemp, startPage);
    });
  }

  getSchemas(database, recordsTemp?, startPage?, isSearch = false) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 100,
      envId: this.formGroup.get('cluster').value,
      parentId: database,
      resourceType: DataMap.Resource_Type.DWS_Schema.value
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
          startPage ===
            Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 100)) ||
          res.totalCount === 0
        ) {
          if (this.type === DataMap.Resource_Type.DWS_Schema.value) {
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
            }
          } else {
            this.totalTable = res.totalCount;
            const nodes = map(recordsTemp, item => {
              // 修改时的schema半选逻辑, 由于搜索过后回显就不能按照原数据回显了，则不能走这里
              if (
                this.data &&
                find(this.data.extendInfo?.table?.split(','), path => {
                  return includes(path, `${item.name}/`);
                }) &&
                !isSearch
              ) {
                item.indeterminate = true;
              }
              return {
                ...item,
                label: item.name,
                children: []
              };
            });
            // 修改时回显选择项的逻辑
            each(nodes, item => {
              if (
                this.data &&
                find(
                  this.data.extendInfo?.table?.split(','),
                  path => path === item.name
                ) &&
                !isSearch
              ) {
                this.selection.push(item);
              }
            });
            // 如果搜索选择后回复，则需要回显选择, 半选交给树组件父子关联去做,而且不能重复添加
            if (isSearch) {
              // 由于搜索可能会导致selection没有包含所有的已选项，所以需要同步一次
              this.selection = uniqBy(
                [...this.selectedTableData, ...this.selection],
                'name'
              );
              each(nodes, item => {
                if (
                  !find(this.selection, { name: item.name }) &&
                  find(this.selection, val =>
                    startsWith(val.name, item.name + '/')
                  )
                ) {
                  item.indeterminate = true;
                }
              });
            }
            this.treeData = nodes;
            this.consTreeData = nodes;
            this.selection = [...this.selection];
          }
          this.cdr.detectChanges();
          return;
        }
        this.getSchemas(database, recordsTemp, startPage);
      });
  }

  expandedChange(node) {
    node.disabled = false;

    if (size(node.children)) {
      return;
    }

    this.getNode(node);
  }

  getNode(node, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      envId: this.formGroup.get('cluster').value,
      parentId: node.name,
      resourceType: DataMap.Resource_Type.DWS_Table.value
    };

    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        const recordsTemp = cloneDeep(res.records);
        each(recordsTemp, item => {
          assign(item, {
            rootPath: node.rootPath
              ? `${node.rootPath}/${item.extendInfo?.path}`
              : `/${item.extendInfo?.path}`,
            label: item.name,
            disabled: false,
            isLeaf: true
          });

          if (
            find(
              this.data?.extendInfo?.table.split(','),
              val => val === item.name
            )
          ) {
            set(item, 'disabled', false);
          }
          if (includes(map(this.selection, 'name'), node.name)) {
            this.selection = [...this.selection, item];
          }
        });

        node.children = !!startPage
          ? [
              ...reject(node.children, n => {
                return n.isMoreBtn;
              }),
              ...recordsTemp
            ]
          : [...recordsTemp];

        if (res.totalCount > size(node.children)) {
          const moreClickNode = {
            label: `${this.i18n.get('common_more_label')}...`,
            isMoreBtn: true,
            isLeaf: true,
            disabled: true,
            startPage: Math.floor(
              size(node.children) / (CommonConsts.PAGE_SIZE * 10)
            )
          };
          node.children = [...node.children, moreClickNode];
        }

        const matchSelection = filter(recordsTemp, item => {
          return !!find(
            this.selectedTableData,
            table => table.name === item.name
          );
        });

        // 如果搜索后回复正常，发现父级下所有子级都被选中且有子级，那么选中父级
        if (
          recordsTemp.length ===
            filter(this.selectedTableData, val =>
              startsWith(val.name, node.name + '/')
            ).length &&
          !!recordsTemp.length
        ) {
          this.selection.push(node);
          this.singleChange({ node: node });
        }

        this.selection = uniqBy([...matchSelection, ...this.selection], 'name');
        this.treeData = [...this.treeData];
        this.consTreeData = [...this.consTreeData];
        this.cdr.detectChanges();
        return;
      });
  }

  removeSingle(item) {
    if (this.type === DataMap.Resource_Type.DWS_Table.value) {
      // 这里应该把父级下的子级也干掉吧
      this.selection = reject(this.selection, value => {
        return (
          item.name === value.name || startsWith(value.name, item.name + '/')
        );
      });
      each(this.treeData, node => {
        if (includes(item.name, `${node.name}/`)) {
          delete node.indeterminate;
        }
      });
      this.treeData = [...this.treeData];
    } else {
      this.selectionSchema = reject(this.selectionSchema, value => {
        return item.name === value.name;
      });
    }
    this.selectedTableData = reject(this.selectedTableData, value => {
      return item.name === value.name;
    });
    this.formGroup.get('set').setValue(this.selectedTableData);
    this.cdr.detectChanges();
    this.disableOkBtn();
  }

  getSelectedTableData(paths) {
    const schemaPath = filter(paths, item => item.level === 0);
    // 如果父级被选中，则子级也得被选中
    each(this.treeData, node => {
      if (find(schemaPath, { name: node.name }) && !isEmpty(node.children)) {
        this.selection = uniq([...this.selection, ...node.children]);
      }
    });

    // 由于有搜索功能的存在，我们需要用父级把两边的选中的表都去除
    const existTable = [];
    if (size(schemaPath)) {
      each(schemaPath, item => {
        // 最搞笑的是，原来这个东西的判断还有问题，如果children里没有还判断不了
        paths = reject(paths, p => startsWith(p.name, item.name + '/'));
        this.selectedTableData = reject(this.selectedTableData, p =>
          startsWith(p.name, item.name + '/')
        );
      });
    }

    each(this.treeData, node => {
      if (
        this.data &&
        node.level === 0 &&
        isEmpty(node.children) &&
        !includes(map(paths, 'name'), node.name + '/')
      ) {
        existTable.push(
          ...filter(this.selectedTableData, selected => {
            return includes(selected.name, `${node.name}/`);
          })
        );
      }
    });
    this.selectedTableData = uniqBy(
      [...schemaPath, ...paths, ...existTable, ...this.selectedTableData],
      'name'
    );
    this.treeData = [...this.treeData];
  }

  searchSingleChange(rowData) {
    // 子取消则父取消，父取消则子也得取消
    // 并且得考虑子级的名字互相覆盖的问题，所以如果取消子级，不能把其他的子级给取消掉
    // 如果选中父级，会在后面的处理中把子级重新选中，所以不影响
    // 而能选中子级，则父级本身也没有被选中，不影响
    this.selection = filter(
      this.selection,
      val =>
        !(
          startsWith(rowData?.node?.name, val.name + '/') &&
          val.level === 0 &&
          rowData?.node?.level === 1
        ) &&
        !(
          startsWith(val.name, rowData?.node?.name + '/') &&
          val.level === 1 &&
          rowData?.node?.level === 0
        )
    );
    // 取消子的时候，得考虑父亲还有没有选中的子，如果父级没被选中且有自己的子则父级半选
    this.cancelParent(rowData);
  }

  cancelParent(rowData: any) {
    if (rowData?.node?.level === 1) {
      let parentNode = find(this.treeData, node =>
        startsWith(rowData?.node?.name, node.name + '/')
      );
      if (
        this.selectedTableData.some(item =>
          item.name?.startsWith(`${parentNode.name}/`)
        ) &&
        !this.selection.some(item => item.name === rowData?.node?.name)
      ) {
        parentNode.indeterminate = true;
      } else {
        delete parentNode.indeterminate;
      }
    }
  }

  singleChange(rowData) {
    if (this.type === DataMap.Resource_Type.DWS_Table.value) {
      delete rowData.node?.indeterminate;
      // 这里做个处理，把取消掉的现在右表里取消掉，父亲也解决掉
      this.selectedTableData = filter(
        this.selectedTableData,
        val =>
          val.name !== rowData?.node?.name &&
          !(startsWith(rowData?.node?.name, val.name + '/') && val.level === 0)
      );
      if (this.isSearch) {
        // 搜索因为取消了父子关联，需要手动处理
        this.searchSingleChange(rowData);
      } else {
        // 还得有个处理是如果是修改或者搜索回来，我们取消子级，父级的半选没有被取消掉
        this.cancelParent(rowData);
      }
      this.getSelectedTableData(cloneDeep(this.selection));
    } else {
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
    }
    this.formGroup.get('set').setValue(this.selectedTableData);
  }

  selectionChange(selection) {
    this.selectionSchema = filter(
      selection,
      item => get(item, 'extendInfo.isLocked') === 'false'
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
    if (this.type === DataMap.Resource_Type.DWS_Table.value) {
      this.selection = [];
    } else {
      this.selectionSchema = [];
    }

    this.selectedTableData = [];
    this.formGroup.get('set').setValue(this.selectedTableData);
    this.disableOkBtn();
  }

  searchByName() {
    if (!this.treeData?.length && !this.isSearch) {
      return;
    }

    if (this.namePopover) {
      this.namePopover.hide();
    }

    this.isSearch = !!this.queryTableName;
    if (!this.queryTableName) {
      this.getSchemas(this.formGroup.get('database').value, null, null, true);
    } else {
      each(this.treeData, item => {
        item.children = [];
      });
      this.searchTable();
    }
  }

  searchTable(startPage?, recordsTemp?) {
    this.protectedEnvironmentApiService
      .ListEnvironmentResource({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        envId: this.formGroup.get('cluster').value,
        parentId: this.formGroup.get('database').value,
        resourceType: DataMap.Resource_Type.DWS_Table.value,
        conditions: JSON.stringify({ tableName: this.queryTableName })
      })
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
          startPage ===
            Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
          res.totalCount === 0
        ) {
          this.parseSearch(recordsTemp);
          return;
        }
        this.searchTable(startPage, recordsTemp);
      });
  }

  parseSearch(recordsTemp: any) {
    // 处理搜索后的数据
    each(recordsTemp, item => {
      assign(item, {
        label: item.name,
        disabled: false,
        isLeaf: true
      });
      let parentSchema = find(this.consTreeData, val =>
        startsWith(item.name, val.name + '/')
      );
      if (parentSchema) {
        parentSchema.children.push(item);
      }
    });
    // schema下面没有符合的表就不展示schema了
    this.treeData = filter(this.consTreeData, item => !!size(item.children));
    each(this.treeData, item => {
      item.expanded = !!size(item.children);
    });

    const matchSelection = filter(recordsTemp, item => {
      return !!find(this.selectedTableData, table => table.name === item.name);
    });
    this.selection = uniqBy([...matchSelection, ...this.selection], 'name');
    this.treeData = [...this.treeData];
    this.cdr.detectChanges();
    // 这里触发一次父传子
    this.getSelectedTableData(cloneDeep(this.selection));
  }

  getParams() {
    return this.type === DataMap.Resource_Type.DWS_Schema.value
      ? {
          name: this.formGroup.value.name,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.DWS_Schema.value,
          extendInfo: {
            table: join(
              map(this.selectedTableData, item => item.name),
              ','
            )
          },
          parentName: this.formGroup.value.database,
          parentUuid: get(
            find(
              this.databaseOptions,
              item => item.name === this.formGroup.value.database
            ),
            'uuid'
          ),
          rootUuid: this.formGroup.value.cluster
        }
      : {
          name: this.formGroup.value.name,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.DWS_Table.value,
          extendInfo: {
            table: join(
              map(this.formGroup.value.set, item => item.name),
              ','
            )
          },
          parentName: this.formGroup.value.database,
          parentUuid: get(
            find(
              this.databaseOptions,
              item => item.name === this.formGroup.value.database
            ),
            'uuid'
          ),
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
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      } else {
        this.protectedResourceApiService
          .CreateResource({
            CreateResourceRequestBody: params as any
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      }
    });
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }
}
