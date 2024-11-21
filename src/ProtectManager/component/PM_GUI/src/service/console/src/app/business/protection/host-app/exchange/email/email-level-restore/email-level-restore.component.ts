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
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnDestroy,
  OnInit,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import {
  lvFlattenTreeData,
  ModalRef,
  PopoverComponent,
  TreetableComponent,
  TreeTableNode
} from '@iux/live';
import {
  CAPACITY_UNIT,
  CommonConsts,
  CopyControllerService,
  DataMap,
  FileReplaceStrategy,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import { FileSystemResponse } from 'app/shared/api/models/file-system-response';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  every,
  find,
  first,
  get,
  isEmpty,
  isNil,
  map,
  trim
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { startWith, takeUntil } from 'rxjs/operators';
import ListCopyCatalogsParams = CopyControllerService.ListCopyCatalogsParams;

@Component({
  selector: 'aui-restore',
  templateUrl: './email-level-restore.component.html',
  styleUrls: ['./email-level-restore.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class EmailLevelRestoreComponent
  implements OnInit, OnDestroy, AfterViewInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  _isEmpty = isEmpty;
  _trim = trim;
  formGroup: FormGroup;
  restoreV2Type = RestoreV2Type;
  unionist = CAPACITY_UNIT;
  fileReplaceStrategy = FileReplaceStrategy;
  resourceProperties: any;
  properties: any;
  rangeDate: [Date, Date] = [null, null];
  searchName: string = null;
  disableDate = true;
  lvSelectionMode = true;
  hostAndDAGOptions = [];
  treeData: TreeTableNode[] = [];
  selectionData: TreeTableNode[] = [];
  iconMap = {
    Email: 'aui-icon-exchange-server',
    Folder: 'aui-icon-directory',
    Root: 'aui-icon-exchange-server'
  };
  private treeTableDestroy$ = new Subject<void>();
  @ViewChild(TreetableComponent, { static: false })
  lvTreeTable: TreetableComponent;
  @ViewChild('datePopover', { static: false }) datePopover: PopoverComponent;
  @ViewChild('namePopover', { static: false }) namePopover: PopoverComponent;
  constructor(
    private fb: FormBuilder,
    private restoreV2Service: RestoreApiV2Service,
    private copyControllerService: CopyControllerService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private appUtilsService: AppUtilsService,
    private cdr: ChangeDetectorRef,
    private model: ModalRef
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getTargetHosts();
  }

  ngAfterViewInit() {
    if (this.restoreType === RestoreV2Type.FileRestore) {
      this.subscribeRenderChange();
    }
  }

  subscribeRenderChange() {
    this.lvTreeTable.renderDataChange$
      .pipe(
        startWith(this.lvTreeTable.renderData),
        takeUntil(this.treeTableDestroy$)
      )
      .subscribe(() => {
        this.lvTreeTable.renderData = lvFlattenTreeData(
          this.lvTreeTable.renderData,
          null,
          { getNodeLevel: true }
        ).map(item => item.data);
      });
  }

  initForm() {
    this.resourceProperties = JSON.parse(
      get(this.rowCopy, 'resource_properties', '{}')
    );
    this.properties = JSON.parse(get(this.rowCopy, 'properties', '{}'));
    this.formGroup = this.fb.group({
      overwriteType: [FileReplaceStrategy.Ignore]
    });
    if (this.restoreType === RestoreV2Type.FileRestore) {
      this.model.getInstance().lvOkDisabled = true;
      this.initFileRestorePath();
      this.queryTreeData(first(this.treeData), this.treeData);
    }
  }

  initFileRestorePath() {
    const parentPath = get(
      this.resourceProperties,
      'extendInfo.PrimarySmtpAddress',
      ''
    );
    this.treeData = [
      this.createTreeNode({
        path: parentPath,
        queryPath: parentPath,
        type: 'Root',
        icon: this.iconMap['Root'],
        extendInfo: JSON.stringify({
          meta_file: 'root'
        })
      })
    ];
  }

  dateChange(data) {
    this.disableDate = this.checkDateIsNull();
    if (this.disableDate) {
      this.searchByName(this.searchName);
    }
  }

  selectDate() {
    this.datePopover.hide();
    if (this.rangeDate[0] == null && this.rangeDate[1] == null) {
      // 删除选择时间，将rangeDate赋空方便check函数判断
      this.rangeDate = [null, null];
    }
    this.searchByName(this.searchName);
  }

  cancelDate() {
    this.datePopover.hide();
  }

  checkDateIsNull() {
    return every(this.rangeDate, isNil);
  }

  checkNameIsEmpty(name: string) {
    return isEmpty(trim(name));
  }

  convertToTimestamp(dateString: Date) {
    if (isNil(dateString)) {
      // 不需要发时间
      return undefined;
    }
    return Math.floor(new Date(dateString).getTime() / 1000);
  }

  searchByName(e) {
    if (this.namePopover) {
      this.namePopover.hide();
    }
    const nameIsEmpty = this.checkNameIsEmpty(e);
    if (nameIsEmpty && this.checkDateIsNull()) {
      // 名称和时间都不输入时直接调用初始化函数
      this.selectionData = []; // 后续修改
      this.initFileRestorePath();
      this.queryTreeData(first(this.treeData), this.treeData);
      return;
    }
    const condition = {
      ignoreName: nameIsEmpty ? '1' : '0'
    };
    assign(condition, {
      startTime: this.convertToTimestamp(this.rangeDate[0]),
      endTime: this.convertToTimestamp(this.rangeDate[1])
    });
    this.treeData = [];
    const params = {
      parentPath: `/${get(
        this.resourceProperties,
        'extendInfo.PrimarySmtpAddress',
        ''
      )}`,
      copyId: this.rowCopy.uuid,
      name: this.checkNameIsEmpty(e) ? 'all' : e,
      conditions: JSON.stringify(condition)
    };

    this.copyControllerService.ListCopyCatalogsByName(params).subscribe(res => {
      // 处于搜索状态时，取消父子关联
      this.lvSelectionMode = false;
      this.selectionData = []; // 搜索时清空选择数据
      this.initFileRestorePath(); // 返回的结果没有第一层，所以需要手动构造
      const node = this.treeData[0];
      this.addItemToNode(node, res);
      this.updateRowData(node, this.treeData);
    });
  }

  addItemToNode(parentNode, result) {
    parentNode.expanded = true;
    for (const item of result.records) {
      const node = this.createTreeNode(item, parentNode.data.queryPath);
      parentNode.children.push(node);
      if (item.children) {
        this.addItemToNode(node, item.children);
      }
    }
  }

  createTreeNode(nodeData, parentPath?) {
    return {
      data: {
        ...nodeData,
        name: get(nodeData, 'path', ''),
        date: get(nodeData, 'modifyTime', ''),
        size: get(nodeData, 'size', ''),
        queryPath: isEmpty(parentPath)
          ? `${get(nodeData, 'queryPath')}`
          : `${parentPath}/${get(nodeData, 'path')}`,
        icon: this.iconMap[get(nodeData, 'type', 'Email')],
        key: JSON.parse(get(nodeData, 'extendInfo', '{}'))['meta_file']
      },
      children: [],
      expanded: false,
      disabled: false,
      isLeaf: false
    };
  }

  queryTreeData(node: TreeTableNode, rowData) {
    // 正常搜索展开时，开启父子关联
    this.lvSelectionMode = true;
    const targetPath = `/${node.data.queryPath}`;
    const extParams = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      copyId: this.rowCopy.uuid,
      parentPath: targetPath
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      (param: ListCopyCatalogsParams) =>
        this.copyControllerService.ListCopyCatalogs(param),
      (resource: FileSystemResponse[]) => {
        node.children = map(resource, item =>
          this.createTreeNode(item, get(node, 'data.queryPath', ''))
        );
        node.expanded = true;
        this.updateRowData(node, rowData);
      }
    );
  }

  trackByIndex(index) {
    return index;
  }

  expandChange(
    isExpanded: boolean,
    item: TreeTableNode,
    rowData: TreeTableNode[]
  ) {
    if (!isExpanded) {
      rowData.forEach((rowItem, index) => {
        rowData[index] = { ...rowItem };
      });
      this.treeData = [...this.treeData];
      return;
    }
    if (isEmpty(item.children)) {
      this.queryTreeData(item, rowData);
      return;
    }
    this.updateRowData(item, rowData);
  }

  selectionChange(data) {
    // 后续需要增加功能
    this.model.getInstance().lvOkDisabled = !this.selectionData.length;
  }

  getTargetPath() {
    return (
      find(this.hostAndDAGOptions, {
        uuid: this.resourceProperties?.environment_uuid
      })['label'] || this.resourceProperties.environment_name
    );
  }

  updateRowData(item, rowData) {
    // rowData的每个item需要变更地址，触发<td>的变更，拿到每个节点最新的子节点
    rowData.forEach((rowItem, index) => {
      rowData[index] = { ...rowItem };
    });
    // data地址需要变更
    this.treeData = [...this.treeData];
    this.initChildSelection(item);
    // selection地址需要变更
    this.selectionData = [...this.selectionData];
    this.cdr.detectChanges();
  }

  private initChildSelection(source) {
    const isSelected = this.selectionData.some(
      item => item.data.key === source.data.key
    );

    if (isSelected) {
      source.children.forEach(item => {
        this.selectionData.push(item);
      });
    } else {
      this.selectionData = this.selectionData.filter(
        item => item.parent.data.key === source.data.key
      );
    }
  }

  getTargetHosts() {
    const extParams = {
      queryDependency: true,
      conditions: JSON.stringify({
        subType: [
          `${DataMap.Resource_Type.ExchangeSingle.value}`,
          `${DataMap.Resource_Type.ExchangeGroup.value}`
        ]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.hostAndDAGOptions = map(resource, item => ({
          ...item,
          key: item.uuid,
          value: item.uuid,
          label: `${item.name}(${item?.path})`,
          isLeaf: true
        }));
      }
    );
  }

  getParam() {
    const param = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.resourceProperties.root_uuid || this.resourceProperties.rootUuid,
      restoreType: this.restoreType,
      targetLocation: RestoreV2LocationType.ORIGIN,
      targetObject: this.resourceProperties.uuid,
      filters: [],
      agents: [],
      extendInfo: {
        fileReplaceStrategy: this.formGroup.value.overwriteType
      }
    };
    if (this.restoreType === RestoreV2Type.FileRestore) {
      const subObjects = map(this.selectionData, 'data.key');
      assign(param, {
        subObjects
      });
    }
    return param;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const param = this.getParam();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: param as any })
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
    });
  }

  ngOnDestroy(): void {
    this.treeTableDestroy$.next();
    this.treeTableDestroy$.complete();
  }
}
