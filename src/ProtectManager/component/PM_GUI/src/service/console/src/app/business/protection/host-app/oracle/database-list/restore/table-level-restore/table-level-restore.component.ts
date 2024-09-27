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
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  lvFlattenTreeData,
  ModalRef,
  PopoverComponent,
  SelectComponent,
  TreetableComponent,
  TreeTableNode
} from '@iux/live';
import {
  AppService,
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  CopyControllerService,
  DataMap,
  I18NService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreFileType,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  filter,
  find,
  first,
  get,
  includes,
  isEmpty,
  isNil,
  map,
  remove,
  set,
  trim,
  uniqBy
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { startWith, takeUntil } from 'rxjs/operators';
import ListCopyCatalogsParams = CopyControllerService.ListCopyCatalogsParams;

@Component({
  selector: 'aui-table-level-restore',
  templateUrl: './table-level-restore.component.html',
  styleUrls: ['./table-level-restore.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class TableLevelRestoreComponent
  implements OnInit, OnDestroy, AfterViewInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  formGroup: FormGroup;
  dataMap = DataMap;
  isWindows = false;
  isCDB = false; // 根据后台返回的数据是pdb_infos还是table_infos，判断是否是CDB场景
  originCopyIsCluster = false; // 原环境是集群数据库还是单机数据库
  /*
    CDB: Container Database
    PDB: Pluggable Database
    CDB作为容器，管理多个PDB，PDB可以动态的移入移出CDB
  */
  scriptArr = [
    {
      key: 'preProcessing',
      label: this.i18n.get('protection_restore_pre_script_label')
    },
    {
      key: 'postProcessing',
      label: this.i18n.get('protection_restore_post_script_label')
    },
    {
      key: 'postProcessing',
      label: this.i18n.get('protection_restore_fail_script_label')
    }
  ];
  restoreV2LocationType = RestoreV2LocationType;
  resourceProperties: any;
  properties: any;
  valid$ = new Subject();
  selectedHostIsCluster = false;
  originSingleNodeOpts = []; // 原集群下的实例
  singleNodeOpts = []; // 集群下的节点
  activeIndex = 'select';
  targetHostOptions = [];
  treeData: TreeTableNode[] = [];
  selectionData: TreeTableNode[] = [];
  displayData = [];
  targetUserNames = [];
  targetTableSpaces = [];
  usersTableSpaceMap = new Map();
  restoreToNewLocationOnly = false;
  originCopyIsCDB = ''; // 需要用该变量来过滤目标主机
  iconMap = {
    Database: 'aui-icon-data-storage',
    [RestoreFileType.Directory]: 'aui-icon-topo',
    [RestoreFileType.File]: 'aui-icon-table'
  };
  recMemoryLimitGbErrorTips = {
    ...this.baseUtilService.integerErrorTip
  };
  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  numberOfChannelRangeErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 254])
  };
  private treeTableDestroy$ = new Subject<void>();
  @ViewChild('lvTreeTable', { static: false })
  lvTreeTable: TreetableComponent;
  @ViewChild('lvTable', { static: false })
  lvTable: TreetableComponent;
  constructor(
    private fb: FormBuilder,
    private restoreV2Service: RestoreApiV2Service,
    private copyControllerService: CopyControllerService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private appService: AppService,
    private cdr: ChangeDetectorRef,
    public i18n: I18NService,
    private model: ModalRef
  ) {}

  ngOnInit(): void {
    this.initData();
    this.initForm();
    this.listenForm();
    this.getProxyOptions();
  }

  ngAfterViewInit() {
    this.subscribeRenderChange();
  }

  ngOnDestroy(): void {
    this.treeTableDestroy$.next();
    this.treeTableDestroy$.complete();
    this.lvTreeTable.renderDataChange$.unsubscribe();
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

  initFirstNode() {
    // 构造初始节点
    this.treeData = [
      this.createTreeNode(
        {
          path: this.rowCopy.resource_name,
          type: 'Database',
          parentPath: '',
          isFirst: true
        },
        ''
      )
    ];
  }

  initData() {
    this.model.getInstance().lvOkDisabled = true;
    this.isWindows =
      this.rowCopy.environment_os_type === DataMap.Os_Type.windows.value;
    this.resourceProperties = JSON.parse(
      get(this.rowCopy, 'resource_properties', '{}')
    );
    this.properties = JSON.parse(get(this.rowCopy, 'properties', '{}'));
    this.originCopyIsCDB = this.resourceProperties.extendInfo?.is_cdb;
    this.originSingleNodeOpts = JSON.parse(
      this.resourceProperties.extendInfo?.instances || '{}'
    );
    this.originCopyIsCluster =
      this.rowCopy.resource_sub_type ===
      DataMap.Resource_Type.oracleCluster.value;
    // 初始化首个节点
    this.initFirstNode();
    // 默认查询首个节点下的内容
    this.queryTreeData(first(this.treeData), this.treeData);

    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) ||
      this.rowCopy.is_replicated ||
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value;
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(''),
      originLocation: new FormControl({
        value: `${this.rowCopy?.resource_environment_name || ''}/${
          this.rowCopy?.resource_name
        }`,
        disabled: true
      }),
      targetHost: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      singleNode: new FormControl(''),
      isOverwrite: new FormControl(false),
      numberOfChannelOpen: new FormControl(false),
      recMemoryLimitGb: new FormControl(1, {
        validators: [this.baseUtilService.VALID.integer()]
      }),
      numberOfChannels: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 254)
        ]
      }),
      scriptOpen: new FormControl(false),
      preProcessing: new FormControl(''),
      postProcessing: new FormControl(''),
      restore_failed_script: new FormControl('')
    });
  }

  listenForm() {
    this.formGroup.get('numberOfChannelOpen').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('numberOfChannels')
          .setValidators([
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 254),
            this.baseUtilService.VALID.required()
          ]);
      } else {
        this.formGroup.get('numberOfChannels').clearValidators();
      }
      this.formGroup.get('numberOfChannels').updateValueAndValidity();
    });

    this.formGroup.get('scriptOpen').valueChanges.subscribe(res => {
      each(this.scriptArr, item => {
        this.createScriptFormControl(res, item);
        this.formGroup.get(item.key).updateValueAndValidity();
      });
    });

    const originalHostObj = {
      uuid: this.rowCopy.resource_id,
      rootUuid: this.resourceProperties?.environment_uuid
    };
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('targetHost').disable();
        this.formGroup.get('targetHost').setValue('');
        this.getUserAndNameSpace(
          originalHostObj,
          this.originCopyIsCluster ? this.originSingleNodeOpts[0] : null
        );
      } else {
        this.formGroup.get('targetHost').enable();
      }
      this.formGroup.get('targetHost').updateValueAndValidity();
      this.setValid();
    });

    this.formGroup.get('targetHost').valueChanges.subscribe(res => {
      this.resetItemOptions();
      if (!res) {
        this.selectedHostIsCluster = false;
        this.singleNodeOpts = [];
        return;
      }
      const targetHost = find(this.targetHostOptions, { value: res });
      this.selectedHostIsCluster =
        targetHost.subType === DataMap.Resource_Type.oracleCluster.value;
      if (this.selectedHostIsCluster) {
        this.formGroup
          .get('singleNode')
          .setValidators(this.baseUtilService.VALID.required());
        const instances = JSON.parse(
          get(targetHost, 'extendInfo.instances', '[]')
        );
        this.singleNodeOpts = map(instances, item => ({
          value: item.hostId,
          label: item.inst_name,
          isLeaf: true,
          ...item
        }));
      } else {
        this.singleNodeOpts = [];
        this.formGroup.get('singleNode').clearValidators();
        this.getUserAndNameSpace(targetHost);
      }
      this.formGroup.get('singleNode').updateValueAndValidity();
      this.setValid();
    });

    this.formGroup.get('singleNode').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      const selectedHost = find(this.targetHostOptions, {
        value: this.formGroup.value.targetHost
      });
      const selectedNode = find(this.singleNodeOpts, { value: res });
      this.getUserAndNameSpace(selectedHost, selectedNode);
    });

    if (this.restoreToNewLocationOnly) {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
    } else {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.ORIGIN);
    }
  }

  private createScriptFormControl(res, item) {
    if (res) {
      this.formGroup
        .get(item.key)
        .setValidators([
          this.isWindows
            ? this.baseUtilService.VALID.name(
                CommonConsts.REGEX.windowsScript,
                false
              )
            : this.baseUtilService.VALID.name(
                CommonConsts.REGEX.linuxScript,
                false
              ),
          this.baseUtilService.VALID.maxLength(8192)
        ]);
    } else {
      this.formGroup.get(item.key).clearValidators();
    }
  }

  selectChange(e, data) {
    if (!this.isCDB) {
      return;
    }
    // CDB场景 选完目标用户后才能出现选项
    if (isNil(e)) {
      // 删除选中的用户，绑定的信息都要重置
      data.tableSpaceOptions = [];
      data.target_table_space_info = [];
      return;
    }
    data.tableSpaceOptions = this.usersTableSpaceMap.get(e);
  }

  deleteItem(source, deleteAll?: boolean) {
    if (deleteAll) {
      this.lvTreeTable.clearSelection();
    }
    const removeRow = remove(
      this.lvTreeTable.lvSelection,
      item => item.data.name === source.name
    );
    this.lvTreeTable.deleteSelection(removeRow);
  }

  resetItemOptions() {
    // 清除选项下面的额外信息
    this.targetUserNames = [];
    this.targetTableSpaces = [];
    this.usersTableSpaceMap.clear();
    each(this.lvTreeTable?.renderData || [], item => {
      if (item.isLeaf) {
        this.assignItemTableInfo(item);
      }
    });
    each(this.displayData, item => this.assignItemTableInfo(item));
  }

  private assignItemTableInfo(item) {
    assign(item, {
      target_pdb_name: null,
      target_user_name: null,
      target_table_space: null,
      target_table_name: item.name,
      target_table_space_info: [],
      tableSpaceOptions: []
    });
  }

  checkNameIsEmpty(name: string) {
    return isEmpty(trim(name));
  }

  setValid() {
    this.valid$.next(this.formGroup.invalid || !this.displayData.length);
  }

  searchByName(e) {
    const nameIsEmpty = this.checkNameIsEmpty(e);
    if (nameIsEmpty) {
      return;
    }
    this.treeData = [];
    const params = {
      parentPath: `/`,
      copyId: this.rowCopy.uuid,
      name: e
    };

    this.copyControllerService.ListCopyCatalogsByName(params).subscribe(res => {
      // 处于搜索状态时，取消父子关联
      this.selectionData = []; // 搜索时清空选择数据
      this.initFirstNode(); // 返回的结果没有第一层，所以需要手动构造
      const node = this.treeData[0];
      this.addItemToNode(node, res);
      this.updateRowData(node, this.treeData);
    });
  }

  addItemToNode(parentNode, result) {
    parentNode.expanded = true;
    for (const item of result.records) {
      const node = this.createTreeNode(item, parentNode.data.parentPath);
      parentNode.children.push(node);
      if (item.children) {
        this.addItemToNode(node, item.children);
      }
    }
  }

  createTreeNode(nodeData, parentNode?) {
    const data = {
      ...nodeData,
      name: get(nodeData, 'path', ''),
      date: get(nodeData, 'modifyTime', ''),
      size: get(nodeData, 'size', ''),
      icon: this.iconMap[get(nodeData, 'type', RestoreFileType.Directory)]
    };
    if (nodeData.type === RestoreFileType.File) {
      this.assignItemTableInfo(data);
    }
    return {
      data,
      children: [],
      expanded: false,
      disabled: false,
      isLeaf: nodeData.type === RestoreFileType.File
    };
  }

  queryTreeData(node: TreeTableNode, rowData) {
    // 正常搜索展开时，开启父子关联
    const targetPath: string = node.data?.isFirst
      ? '/'
      : `${node.data.parentPath}`;
    const extParams = {
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      copyId: this.rowCopy.uuid,
      parentPath: targetPath
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      (param: ListCopyCatalogsParams) =>
        this.copyControllerService.ListCopyCatalogs(param),
      resource => {
        node.children = map(resource, (item: any) => {
          item.parentPath = `${node.data.parentPath}/${item.path}`;
          return this.createTreeNode(item);
        });
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
    this.displayData = data
      .filter((item: any) => item.isLeaf)
      .map((item: any) => item.data);
    this.setValid();
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

  initChildSelection(source) {
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

  getProxyOptions() {
    const extParams = {
      queryDependency: true,
      conditions: JSON.stringify({
        subType: this.originCopyIsCluster
          ? [
              DataMap.Resource_Type.oracle.value,
              DataMap.Resource_Type.oracleCluster.value
            ]
          : [DataMap.Resource_Type.oracle.value]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        // 过滤操作系统，不能跨操作系统恢复
        let filterRes = [];
        filterRes = filter(
          resource,
          item =>
            item.environment?.osType === this.rowCopy.environment_os_type &&
            this.originCopyIsCDB === item.extendInfo?.is_cdb
        );
        filterRes = uniqBy(filterRes, 'rootUuid');
        this.targetHostOptions = map(filterRes, item => ({
          ...item,
          key: item.rootUuid || item.parentUuid,
          value: item.rootUuid || item.parentUuid,
          label: `${item.environment?.name}(${item.environment?.endpoint})`,
          isLeaf: true,
          resourceRoleAuth: map(item.resourceRoleAuth, 'name')
        }));
      }
    );
  }

  getUserAndNameSpace(host, node?) {
    const extParams = {
      envId: host.rootUuid,
      agentId: node ? node.hostId : host.rootUuid,
      pageNo: CommonConsts.PAGE_START_EXTRA,
      resourceIds: [host.uuid],
      appType: DataMap.Resource_Type.oracle.value,
      conditions: JSON.stringify({
        queryType: 'pdbInfo',
        hostId: node ? node.hostId : host.rootUuid
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.appService.ListResourcesDetails(params),
      resource => {
        if (!isEmpty(resource)) {
          this.formatTableSpaceData(resource[0]);
        } else {
          this.resetItemOptions();
        }
      }
    );
  }

  formatOptionItem(item) {
    return {
      key: item,
      value: item,
      label: item,
      isLeaf: true
    };
  }

  formatTableSpaceData(data) {
    if (isEmpty(data) || isEmpty(data.extendInfo.table_space_infos)) {
      this.targetUserNames = [];
    }
    const tableSpaceInfo = JSON.parse(data.extendInfo.table_space_infos);
    this.isCDB = tableSpaceInfo.is_cdb;

    if (this.isCDB) {
      this.formatCDBTableSpaces(tableSpaceInfo);
    } else {
      this.targetUserNames = map(tableSpaceInfo.user_names, item =>
        this.formatOptionItem(item)
      );
      this.targetTableSpaces = map(tableSpaceInfo.table_spaces, item =>
        this.formatOptionItem(item)
      );
    }
  }

  formatCDBTableSpaces(tableSpaceInfo) {
    this.targetUserNames = map(tableSpaceInfo.users, item => {
      const pdbTableSpaceData = map(item.pdbs, pdb => {
        return {
          ...this.formatOptionItem(pdb.pdb_name),
          isLeaf: false,
          children: map(pdb.table_spaces, tableName =>
            this.formatOptionItem(tableName)
          )
        };
      });
      this.usersTableSpaceMap.set(item.user_name, pdbTableSpaceData);
      return this.formatOptionItem(item.user_name);
    });
  }

  getParam() {
    const isOrigin =
      this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN;
    const param = {
      copyId: this.rowCopy.uuid,
      agents: [],
      targetEnv: isOrigin
        ? this.resourceProperties.environment_uuid
        : this.formGroup.value.targetHost,
      restoreType: RestoreV2Type.CommonRestore,
      targetLocation: this.formGroup.value.restoreTo,
      targetObject: isOrigin
        ? this.resourceProperties.uuid
        : find(this.targetHostOptions, {
            value: this.formGroup.value.targetHost
          }).uuid,
      extendInfo: {
        restoreFrom:
          this.rowCopy?.backup_type === DataMap.CopyData_Backup_Type.log.value
            ? 'log'
            : 'data',
        bctStatus: false,
        CHANNELS: this.formGroup.value.numberOfChannels || '',
        UUID: this.resourceProperties.uuid || '',
        RESTORE_TARGET_HOST: '',
        RESTORE_PATH: '',
        instances: '[]',
        hostUuid: '',
        isOverwrite: this.formGroup.get('isOverwrite').value,
        recMemoryLimitGb: this.formGroup.get('recMemoryLimitGb').value
      },
      scripts: {
        preScript: this.formGroup.value.scriptOpen
          ? this.formGroup.value.preProcessing || ''
          : '',
        postScript: this.formGroup.value.scriptOpen
          ? this.formGroup.value.postProcessing || ''
          : '',
        failPostScript: this.formGroup.value.scriptOpen
          ? this.formGroup.value.restore_failed_script || ''
          : ''
      }
    };
    const tables = map(this.displayData, item => {
      const extObj = JSON.parse(item.extendInfo);
      // CDB场景时选中的结构是['pdb_name','table_space']
      const targetSpaceInfos = this.isCDB
        ? item.target_table_space_info
        : item.target_table_space;
      return {
        user_name: extObj.user_name, // 用户名
        table_name: item.name, // 表名
        pdb_name: extObj.pdb_name || null, // 原pdb名
        table_space: extObj.table_space || null, // 原表空间
        target_pdb_name: this.isCDB
          ? targetSpaceInfos[0] || null
          : item.target_pdb_name, // 恢复后pdb名，
        target_user_name: item.target_user_name,
        target_table_space: this.isCDB
          ? targetSpaceInfos[1] || null
          : targetSpaceInfos,
        target_table_name: item.target_table_name
      };
    });
    set(param, 'extendInfo.tables', JSON.stringify(tables));
    return param;
  }

  restore(): Observable<void> {
    const param = this.getParam();
    return new Observable<void>((observer: Observer<void>) => {
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
}
