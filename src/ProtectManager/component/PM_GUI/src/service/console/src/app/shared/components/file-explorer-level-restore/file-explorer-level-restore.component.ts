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
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';
import { ObjectRestoreComponent } from 'app/business/protection/storage/object/object-service/object-restore/object-restore.component';
import {
  CopyControllerService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  RestoreFileType,
  SYSTEM_TIME
} from 'app/shared/consts';
import { I18NService } from 'app/shared/services';
import {
  assign,
  cloneDeep,
  difference,
  each,
  filter,
  find,
  findIndex,
  isEmpty,
  isUndefined,
  map,
  set,
  size,
  startsWith,
  toNumber,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-file-explorer-level-restore',
  templateUrl: './file-explorer-level-restore.component.html',
  styleUrls: ['./file-explorer-level-restore.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [DatePipe]
})
export class FileExplorerLevelRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  @Input() restoreLevel;
  tableData = [];
  items = [];
  hideItems = [];
  maxBreadNum = 5; // 想要限制的面包屑最大长度，超过该长度展示下拉

  dataMap = DataMap;
  restoreFileType = RestoreFileType;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  unitconst = CAPACITY_UNIT;
  allFileData = {};
  allNum = {};
  currentFileData = [];
  selectionData = [];
  oldSelectionData = [];
  total = 0;
  currentPath = '/';
  inputTarget = '';
  fileLevelRestoreTips = '';
  name;
  isSearch = false; // 用于标明当前是否在搜索状态中
  isSearchRestore = false; // 用于标明是不是从全局检索点进来的恢复
  incrementFileEmptyTips = false; // 增量副本备份前后数据没有变化时，界面会展示无数据
  timeZone = SYSTEM_TIME.timeZone;

  @ViewChild(ObjectRestoreComponent, { static: false })
  objectRestoreComponent: ObjectRestoreComponent;

  constructor(
    public i18n: I18NService,
    private modal: ModalRef,
    private datePipe: DatePipe,
    private cdr: ChangeDetectorRef,
    private restoreV2Service: RestoreApiV2Service,
    private copyControllerService: CopyControllerService
  ) {}

  ngOnInit() {
    this.isSearchRestore = !!this.rowCopy.isSearchRestore;
    if (!this.isSearchRestore) {
      this.getOriginalPath();
    }
  }

  getOriginalPath() {
    this.currentFileData = [
      {
        name: this.rowCopy.resource_name,
        parent_uuid: null,
        path: this.rowCopy.resource_name,
        rootPath: `/`,
        icon: 'aui-icon-directory',
        realCount: 0,
        restSelected: false
      }
    ];
    this.items.push({
      id: String(this.items.length + 1),
      label: this.rowCopy.resource_name,
      parentPath: '/',
      rootPath: `/`,
      name: this.rowCopy.resource_name,
      onClick: data => {
        this.getCertainPath(data?.data || data?.item);
      }
    });
    this.cdr.detectChanges();
    this.getChildren(this.currentFileData[0], true);
    this.incrementFileEmptyTips =
      this.childResType === DataMap.Resource_Type.ObjectSet.value &&
      [DataMap.CopyData_Backup_Type.incremental.value].includes(
        this.rowCopy.source_copy_type
      );
  }

  getCertainPath(item) {
    // 从面包屑点击切换指定层级
    const num = findIndex(this.items, val => val.rootPath === item.rootPath);
    if (num === -1) {
      // 找不到在当前层级中的位置说明是从下拉框里点进来的
      this.parseBreadItems(item);
    } else {
      this.items.splice(num + 1, this.items.length - num);
    }

    this.getChildren(item, true);
  }

  parseBreadItems(item) {
    // 用于解决点击下拉框跳转时的数据
    // 这时候把下拉框里在点击位置之前的全塞回到items里面
    const hideNum = this.hideItems.findIndex(
      val => val.rootPath === item.rootPath
    );
    this.hideItems.splice(hideNum + 1);
    this.items.splice(
      this.maxBreadNum - 2,
      this.items.length - this.maxBreadNum + 2,
      ...this.hideItems
    );
    // 这时候如果大于限制的最大长度，重新分配hideitems
    if (this.items.length > this.maxBreadNum) {
      const hideItem = this.items
        .splice(this.maxBreadNum - 2, this.items.length - this.maxBreadNum + 1)
        .filter(item => item.label !== '...');
      each(hideItem, val => {
        if (!isUndefined(val?.items)) {
          delete val.items;
        }
      });
      this.hideItems = [...hideItem];
      this.items.splice(this.maxBreadNum - 2, 0, {
        id: this.maxBreadNum - 1,
        label: '...',
        icon: null
      });
      set(this.items[this.maxBreadNum - 2], 'items', this.hideItems);
    } else {
      // 无需hide则清空
      this.hideItems = [];
    }
  }

  getChildren(item, isClickPath?) {
    if (item?.type === RestoreFileType.File) {
      return;
    }
    if (this.currentPath !== item.rootPath) {
      this.pageIndex = 0;
    }
    this.copyControllerService
      .ListCopyCatalogs({
        memberEsn: this.rowCopy?.device_esn || '',
        pageNo: this.pageIndex,
        pageSize: this.pageSize * 10,
        copyId: this.rowCopy.uuid,
        parentPath: item.rootPath
      })
      .subscribe(
        (res: any) => {
          if (this.currentPath === item.rootPath) {
            // 翻页不增加面包屑路径
            isClickPath = true;
          }
          this.currentPath = item.rootPath;
          this.parseData(res, item);

          if (!isClickPath) {
            this.pathChange(item);
          }
          this.disableBtn();
          this.cdr.detectChanges();
        },
        error => {
          // 报错时进入下一层，只是没有数据
          if (!isClickPath) {
            this.pathChange(item);
          }
          this.currentPath = item.rootPath;
          if (isEmpty(this.allFileData[this.currentPath])) {
            set(this.allFileData, this.currentPath, []);
          }
          this.currentFileData = [];
          this.total = 0;
          this.cdr.detectChanges();
        }
      );
  }

  pathChange(item) {
    // 更新面包屑路径
    this.items.push(
      assign(item, {
        id: String(this.items.length + 1),
        label: item.name,
        onClick: data => {
          this.getCertainPath(data?.data || data?.item);
        },
        icon: null
      })
    );
    if (this.items.length > this.maxBreadNum) {
      // 就把第三个和第四个塞到点点点里面
      const hideItem = this.items
        .splice(this.maxBreadNum - 2, this.items.length - this.maxBreadNum + 1)
        .filter(item => item.label !== '...');
      each(hideItem, val => {
        if (!isUndefined(val?.items)) {
          delete val.items;
        }
      });
      this.hideItems = [...this.hideItems, ...hideItem];
      this.items.splice(this.maxBreadNum - 2, 0, {
        id: this.maxBreadNum - 1,
        label: '...',
        icon: null
      });
      set(this.items[this.maxBreadNum - 2], 'items', this.hideItems);
    } else {
      this.hideItems = [];
    }

    this.items = [...this.items];
  }

  searchObjectName(e) {
    // 找到上一层的父节点数据
    let parentData = this.getParentData();
    if (!trim(e)) {
      this.pageIndex = 0;
      this.isSearch = false;
      if (!parentData) {
        // 如果是根节点则相当于重新获取最初的数据
        this.getChildren(
          {
            name: this.rowCopy.resource_name,
            parent_uuid: null,
            path: this.rowCopy.resource_name,
            rootPath: `/`,
            icon: 'aui-icon-directory',
            realCount: 0,
            restSelected: false
          },
          true
        );
      } else {
        this.getChildren(parentData);
      }
      return;
    }
    this.isSearch = true;
    // 搜索当前文件夹下的同名资源
    this.copyControllerService
      .ListCopyCatalogsByName({
        pageNum: this.pageIndex,
        pageSize: this.pageSize * 10,
        parentPath: this.currentPath,
        copyId: this.rowCopy.uuid,
        name: trim(e),
        conditions: JSON.stringify({
          resType: trim(e),
          ignoreName: '1'
        })
      })
      .subscribe((res: any) => {
        // 如果获取到了新的数据，还是得跟上面一样的处理，所以把上面那个数据处理写成函数
        if (res.records.length !== 0) {
          this.parseData(res, parentData);
        } else {
          this.currentFileData = [];
          this.total = 0;
        }
        this.disableBtn();
        this.cdr.detectChanges();
      });
  }

  parseData(res, item) {
    // 用来对获取到的数据进行处理
    // 由于是精确搜索不用管分页，但是要标明搜索出来的东西的选择状态
    this.total = res.totalCount;
    each(res.records, val => {
      assign(val, {
        name: val.resType,
        parentPath: item.rootPath,
        rootPath:
          item.rootPath === '/'
            ? `/${val.path}`
            : `${item.rootPath}/${val.path}`,
        icon:
          val.type === RestoreFileType.Directory
            ? 'aui-icon-directory'
            : 'aui-icon-file',
        modifyTime:
          this.datePipe.transform(
            toNumber(val.modifyTime) * 1000,
            'yyyy-MM-dd HH:mm:ss',
            this.timeZone
          ) || val.modifyTime
      });
      // 如果该数据之前获取过，把之前的状态同步给它
      const previousData = find(
        this.allFileData[this.currentPath],
        data => data.rootPath === val.rootPath
      );
      if (previousData) {
        assign(val, previousData, {
          icon:
            val.type === RestoreFileType.Directory
              ? 'aui-icon-directory'
              : 'aui-icon-file'
        });
      }
      // 如果父级被选中，则自动选中子级文件
      if (
        find(this.selectionData, select => select.rootPath === val.parentPath)
      ) {
        val.selected = true;
        val.isHalfSelected = false;
        val.restSelected = true;
        if (
          !find(this.selectionData, select => select.rootPath === val.rootPath)
        ) {
          // 之前没push进去才push
          this.selectionData.push(val);
        }
      }
    });

    if (res.totalCount <= this.pageSize * 10) {
      item.realCount = res.totalCount;
    } else if (
      !find(
        this.allFileData[this.currentPath],
        existedFile => existedFile.rootPath === res.records[0].rootPath
      )
    ) {
      // 如果之前没有当前页数据并且一页超过200，则增加当前总条数
      item.realCount =
        (!!item.realCount ? item.realCount : 0) + res.records.length;
    }

    // 把item的变化同步到存储的数据中去
    if (!isUndefined(item.parentPath)) {
      let parentData = this.getParentData();
      if (!isUndefined(parentData)) {
        parentData.realCount = item?.realCount
          ? item.realCount
          : parentData.realCount;
        parentData.restSelected = item?.restSelected;
      }
    }

    // 如果是分页的话，检测该数据有没有，否则不覆盖
    if (!isEmpty(this.allFileData[this.currentPath])) {
      each(res.records, file => {
        if (
          !find(this.allFileData[this.currentPath], { rootPath: file.rootPath })
        ) {
          this.allFileData[this.currentPath].push(file);
        }
      });
    } else {
      set(this.allFileData, this.currentPath, res.records);
    }

    this.currentFileData = res.records;
    if (!this.isSearch) {
      this.allNum[this.currentPath] = res.totalCount;
      this.dataUpdate();
    }

    this.oldSelectionData = [...this.selectionData];
    this.cdr.detectChanges();
  }

  trackByRootPath(index: number, list: any) {
    return list.rootPath;
  }

  checkChange(e, item) {
    item.isHalfSelected = false;
    if (e) {
      this.selectionData.push(item);
      item.restSelected = true;
    } else {
      this.selectionData = this.selectionData.filter(
        val => val.rootPath !== item.rootPath
      );
    }
    this.selectionChange(null, 'clickSelect');
  }

  dataUpdate() {
    // 将当前数据同步到存储的数据中
    this.allFileData[this.currentPath] = this.allFileData[this.currentPath].map(
      item => {
        const latestData = find(this.currentFileData, {
          rootPath: item.rootPath
        });
        return latestData ? { ...item, ...latestData } : item;
      }
    );
  }

  selectionChange(e?, type?) {
    let diffArray = [];
    if (type === 'headSelect') {
      // 全选触发遍历为数据添加标识
      if (this.oldSelectionData.length < this.selectionData.length) {
        diffArray = difference(this.selectionData, this.oldSelectionData);
        each(this.currentFileData, item => {
          if (find(diffArray, select => select.rootPath === item.rootPath)) {
            item.selected = true;
            item.isHalfSelected = false;
            item.restSelected = true;
          }
        });
      } else {
        diffArray = difference(this.oldSelectionData, this.selectionData);
        each(this.currentFileData, item => {
          if (find(diffArray, select => select.rootPath === item.rootPath)) {
            item.selected = false;
            item.restSelected = false;
          }
        });
      }
    }
    // 将变化后的当前数据同步到存储的数据中去
    if (!this.isSearch) {
      this.dataUpdate();
    } else {
      // 搜索状态下数据次序会打乱，不能用上面那个方法
      this.dataUpdate();
    }

    // 把子级放前面有利于后续处理
    this.selectionData.sort((a, b) => {
      return size(a.rootPath) < size(b.rootPath) ? 1 : -1;
    });
    if (this.oldSelectionData.length === this.selectionData.length) {
      return;
    }
    if (this.oldSelectionData.length > this.selectionData.length) {
      diffArray = difference(this.oldSelectionData, this.selectionData);
      // 处理取消选中时父级到子级的向下传递
      this.setChildSelected(diffArray);
    }
    // 处理子级变化到父级的向上传递
    this.setParentSelected();
    this.selectionData = [...this.selectionData];
    this.oldSelectionData = [...this.selectionData];
    this.disableBtn();
  }

  setParentSelected() {
    // 当当前层级的子节点被选中或取消选中时，可能会造成父节点的被选中或者被半选或者被取消选中，我们写到同一个逻辑里，去判断
    // 每次子节点的变化都会造成父节点的状态变化，我们针对所有前缀相同的父节点逐级变化
    if (this.currentPath === '/') {
      // 根目录就不用上传了
      return;
    }

    this.parentConfig(null, null, { parentPath: this.currentPath }, null);
  }

  parentConfig(lastIndex, result, parentData, childSelect) {
    while (result !== '/') {
      // 找到上一层的父节点数据
      lastIndex = parentData.parentPath.lastIndexOf('/');
      result = parentData.parentPath.substring(0, lastIndex);
      if (!result) {
        result = '/';
      }
      parentData = find(
        this.allFileData[result],
        val => val.rootPath === parentData.parentPath
      );
      // 找到上一层父节点下被选中的所有子节点
      childSelect = this.selectionData.filter(
        item => item.parentPath === parentData.rootPath
      );
      // 找到上一层父节点下被半选的子节点数量
      let childHalfSelect = this.allFileData[parentData.rootPath].filter(
        item => !!item.isHalfSelected
      );
      if (childSelect.length === 0 && !childHalfSelect.length) {
        // 如果上一层的节点没有半选或被选中的子节点则父节点为未被选中
        parentData.selected = false;
        parentData.isHalfSelected = false;
        parentData.restSelected = false;
        result = parentData.parentPath;
        this.selectionData = this.selectionData.filter(
          item => item.rootPath !== parentData.rootPath
        );
      } else if (
        childSelect.length === this.allNum[parentData.rootPath] ||
        (!!parentData.restSelected &&
          childSelect.length === parentData.realCount)
      ) {
        // 如果上一层的节点下的子节点被全部选中，或者是子节点有分页，其他还没获取的页默认全选的，则比较当前已获取节点数和已选中子节点数
        parentData.selected = true;
        parentData.isHalfSelected = false;
        parentData.restSelected = true;
        result = parentData.parentPath;
        if (
          !find(
            this.selectionData,
            item => item.rootPath === parentData.rootPath
          )
        ) {
          // 不重复添加所选项
          this.selectionData.push(parentData);
        }
      } else {
        if (!!parentData.isHalfSelected) {
          // 半选变半选的话就不用传递了
          result = '/';
        } else {
          this.selectionData = this.selectionData.filter(
            item => item.rootPath !== parentData.rootPath
          );
          parentData.selected = false;
          parentData.isHalfSelected = true;
          result = parentData.parentPath;
        }
      }
    }
  }

  setChildSelected(diffArray) {
    // 父级的选中或不选也会导致子节点的变化，当然，得已经获取了子节点才会考虑这种情况
    // 如果是取消选中，则应该取消所有子节点的选中，即前缀包含当前节点路径的子节点
    each(diffArray, item => {
      this.selectionData = this.selectionData.filter(
        val => !startsWith(val.parentPath, item.rootPath)
      );
      // 所有子节点里半选的选择的都干掉
      for (let key in this.allFileData) {
        if (startsWith(key, item.rootPath)) {
          this.allFileData[key] = map(this.allFileData[key], item => {
            return assign(item, {
              isHalfSelected: false,
              selected: false
            });
          });
        }
      }
    });
  }

  pageChange(page) {
    this.pageIndex = page.pageIndex;
    if (this.isSearch) {
      this.searchObjectName(this.name);
    } else {
      let parentData = this.getParentData();
      this.getChildren(parentData);
    }
  }

  getParentData() {
    // 提出来的用于获取存储数据中父节点所代表数据的函数
    const lastIndex = this.currentPath.lastIndexOf('/');
    let result = this.currentPath.substring(0, lastIndex);
    if (!result) {
      result = '/';
    }
    return find(
      this.allFileData[result],
      val => val.rootPath === this.currentPath
    );
  }

  disableBtn() {
    this.modal.getInstance().lvOkDisabled =
      (!this.selectionData.length && !this.isSearchRestore) ||
      this.objectRestoreComponent?.formGroup.invalid;
  }

  getfileLevelRestoreTips() {
    let restoreType = this.i18n.get('common_files_label');

    switch (this.childResType) {
      case DataMap.Resource_Type.ObjectSet.value:
        restoreType = this.i18n.get('protection_object_bucket_label');
        break;
      default:
        restoreType = this.i18n.get('common_files_label');
    }

    this.fileLevelRestoreTips = this.i18n.get(
      'protection_file_level_restore_tip_label',
      [restoreType]
    );
    return restoreType;
  }

  getTargetPath() {
    let params = this.objectRestoreComponent?.getTargetParams();
    this.inputTarget = params.name;
    return {
      tips: this.getfileLevelRestoreTips(),
      targetPath: this.inputTarget
    };
  }

  getPath(data) {
    // 如果父级被全选则只下发父级path名
    data.sort((a, b) => {
      return size(a.rootPath) < size(b.rootPath) ? 1 : -1;
    });
    each(data, val => {
      if (find(data, item => item.rootPath === val.parentPath)) {
        data = filter(data, select => select.rootPath !== val.rootPath);
      }
    });
    return data.map(item =>
      item.type === RestoreFileType.Directory
        ? item.rootPath + '/'
        : item.rootPath
    );
  }

  getParams() {
    let params =
      this.objectRestoreComponent?.getTargetParams()?.requestParams || {};
    assign(params, {
      subObjects: this.isSearchRestore
        ? [this.rowCopy.searchRestorePath]
        : this.getPath(cloneDeep(this.selectionData))
    });
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      let memberEsn = '';
      const params: any = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({
          CreateRestoreTaskRequestBody: params,
          memberEsn: memberEsn
        })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: error => {
            observer.error(error);
            observer.complete();
          }
        });
    });
  }
}
