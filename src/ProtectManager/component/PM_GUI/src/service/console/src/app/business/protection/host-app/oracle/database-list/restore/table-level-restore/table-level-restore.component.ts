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
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidationErrors
} from '@angular/forms';
import {
  lvFlattenTreeData,
  ModalRef,
  TreetableComponent,
  TreeTableNode
} from '@iux/live';
import {
  AppService,
  BaseUtilService,
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
  compact,
  differenceBy,
  each,
  filter,
  find,
  first,
  get,
  includes,
  isEmpty,
  isNil,
  last,
  map,
  remove,
  set,
  size,
  some,
  trim
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { startWith, takeUntil, map as _map } from 'rxjs/operators';

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
  dbLinkStatusErrorTip;
  pdbAbnormal = false; // PDB状态是否异常
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
      key: 'restore_failed_script',
      label: this.i18n.get('protection_restore_fail_script_label')
    }
  ];
  restoreV2LocationType = RestoreV2LocationType;
  resourceProperties: any;
  properties: any;
  valid$ = new Subject();
  selectedHostIsCluster = false;
  isSearch = false;
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
    ...this.baseUtilService.rangeErrorTip,
    invalidMinSize: this.i18n.get('common_valid_minsize_label', [1])
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
    public i18n: I18NService,
    private model: ModalRef,
    private fb: FormBuilder,
    private appService: AppService,
    private cdr: ChangeDetectorRef,
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private restoreV2Service: RestoreApiV2Service,
    private copyControllerService: CopyControllerService,
    private protectedResourceApiService: ProtectedResourceApiService
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
    this.isCDB = this.originCopyIsCDB === 'true';
    this.originSingleNodeOpts = JSON.parse(
      this.resourceProperties.extendInfo?.instances || '{}'
    );
    this.originCopyIsCluster =
      this.rowCopy.resource_sub_type ===
      DataMap.Resource_Type.oracleCluster.value;
    // 初始化首个节点
    this.initFirstNode();
    // 默认查询首个节点下的内容
    this.queryTreeData(first(this.treeData));

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
      originLocation: new FormControl(
        `${this.rowCopy?.resource_environment_name || ''}/${
          this.rowCopy?.resource_name
        }`,
        {
          asyncValidators: [this.asyncCheckLinkStatus()]
        }
      ),
      targetHost: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      singleNode: new FormControl(''),
      isOverwrite: new FormControl(false),
      numberOfChannelOpen: new FormControl(false),
      recMemoryLimitGb: new FormControl(1, {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.minSize(0),
          this.baseUtilService.VALID.required()
        ]
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

    this.formGroup.statusChanges.subscribe(() => {
      this.setValid();
    });

    const originalHostObj = {
      uuid: this.rowCopy.resource_id,
      rootUuid: this.resourceProperties?.environment_uuid
    };
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      this.pdbAbnormal = false;
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup
          .get('originLocation')
          .setAsyncValidators([this.asyncCheckLinkStatus()]);
        this.formGroup.get('targetHost').clearAsyncValidators();
        this.formGroup.get('targetHost').disable();
        this.formGroup.get('targetHost').setValue('');
        this.getUserAndNameSpace(
          originalHostObj,
          this.originCopyIsCluster ? this.originSingleNodeOpts[0] : null
        );
      } else {
        this.formGroup
          .get('targetHost')
          .setAsyncValidators([this.asyncCheckLinkStatus()]);
        this.formGroup.get('originLocation').clearAsyncValidators();
        this.formGroup.get('targetHost').enable();
      }
      this.formGroup.get('targetHost').updateValueAndValidity();
      this.setValid();
    });

    this.formGroup.get('targetHost').valueChanges.subscribe(res => {
      this.pdbAbnormal = false;
      this.formGroup.get('singleNode').setValue('');
      this.resetItemOptions();
      if (!res) {
        this.selectedHostIsCluster = false;
        this.singleNodeOpts = [];
        return;
      }
      const targetHost = find(this.targetHostOptions, { key: res });
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
      this.formGroup.get('singleNode').updateValueAndValidity();
      this.formGroup.get('originLocation').updateValueAndValidity();
      this.setValid();
    });

    this.formGroup.get('singleNode').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      const selectedHost = find(this.targetHostOptions, {
        key: this.formGroup.value.targetHost
      });
      const selectedNode = find(this.singleNodeOpts, { value: res });
      this.getUserAndNameSpace(selectedHost, selectedNode);
    });

    if (this.restoreToNewLocationOnly) {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
    }
  }

  asyncCheckLinkStatus() {
    return (control: AbstractControl): Observable<ValidationErrors | null> => {
      return new Observable(observer => {
        this.checkDBLinkStatus(control, observer);
      });
    };
  }

  private checkDBLinkStatus(
    control: AbstractControl,
    observer: Observer<ValidationErrors | null>
  ) {
    this.getProtectedResourceByUuid(
      includes(control.value, this.rowCopy.resource_name)
        ? this.rowCopy.resource_id
        : control.value
    ).subscribe(res => {
      const dbOnline =
        res?.extendInfo?.linkStatus ===
        DataMap.resource_LinkStatus_Special.normal.value;
      const errorKey = 'protection_target_host_restore_offline_db_label';
      this.dbLinkStatusErrorTip = assign({}, this.dbLinkStatusErrorTip, {
        invalidSameDb: this.i18n.get(errorKey, [res?.name]),
        ...this.baseUtilService.requiredErrorTip
      });
      if (dbOnline) {
        observer.next(null);
      } else {
        observer.next({
          invalidSameDb: control.value
        });
      }
      observer.complete();
    });
  }

  private getProtectedResourceByUuid(uuid) {
    return this.protectedResourceApiService
      .ListResources({
        pageNo: 0,
        pageSize: 1,
        akLoading: false,
        conditions: JSON.stringify({
          uuid
        })
      })
      .pipe(_map(res => res.records[0]));
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
      data.rowGroup.get('target_table_space_info').setValue(null);
      return;
    }
    data.tableSpaceOptions = this.usersTableSpaceMap.get(e.value);
    this.cdr.detectChanges();
  }

  deleteItem(source, deleteAll?: boolean) {
    if (deleteAll) {
      this.lvTreeTable.clearSelection();
      this.cdr.detectChanges();
      return;
    }
    const removeRow = remove(
      this.selectionData,
      item =>
        (item?.data?.uniqueId || item?.data?.data?.uniqueId) === source.uniqueId
    );
    this.lvTreeTable.deleteSelection(removeRow);
    this.cdr.detectChanges();
  }

  resetItemOptions() {
    // 清除选项下面的额外信息
    this.targetUserNames = [];
    this.targetTableSpaces = [];
    this.usersTableSpaceMap.clear();
    each(this.lvTreeTable?.renderData || [], item => {
      if (item.isLeaf && item.data?.rowGroup) {
        this.clearForm(item.data.rowGroup);
      }
    });
  }

  private clearForm(formGroup: FormGroup): void {
    Object.keys(formGroup.controls).forEach(key => {
      formGroup.get(key).setValue(null);
      formGroup.markAllAsTouched();
    });
  }

  private assignItemTableInfo(item) {
    let extendObj = {};
    try {
      extendObj = JSON.parse(item.extendInfo);
    } catch {
      console.error('Invalid JSON!');
    }
    if (!item?.rowGroup && !this.isSearch) {
      this.createFormControl(item);
    }
    const data = find(this.displayData, { uniqueId: item.uniqueId });
    data ? (item.rowGroup = data.rowGroup) : this.createFormControl(item);
    if (!data) {
      this.watchForm(item);
      this.manualPatchValue(extendObj, item);
      this.cdr.detectChanges();
    }
  }

  private createFormControl(item) {
    const formControlArr = [
      'target_user_name',
      'target_table_space',
      'target_table_name',
      'target_pdb_name',
      'target_table_space_info'
    ];
    const rowGroup = this.fb.group({});
    formControlArr.forEach(ctrlName => {
      rowGroup.addControl(
        ctrlName,
        new FormControl(
          {
            value: null,
            disabled: true
          },
          {
            validators: [this.baseUtilService.VALID.required()]
          }
        )
      );
    });
    item.rowGroup = rowGroup;
  }

  private watchForm(item) {
    // 为了两边的formControl值同步变化，需要手动setValue
    const setValue = (control, value) =>
      control.setValue(value, { emitEvent: false });

    item.rowGroup.get('target_table_name').setValue(item.name);

    // CDB场景时，表空间和pdb--[table_space,pdb_name]
    if (!this.isCDB) {
      item.rowGroup.get('target_table_space_info').disable();
    }

    item.rowGroup.get('target_user_name').valueChanges.subscribe(res => {
      setValue(item.rowGroup.get('target_user_name'), res);
      const targetUser = find(this.targetUserNames, { value: res });
      this.selectChange(targetUser, item);
    });

    const formControls = ['target_table_space', 'target_table_name'];

    formControls.forEach(controlName => {
      item.rowGroup
        .get(controlName)
        .valueChanges.subscribe(res =>
          setValue(item.rowGroup.get(controlName), res)
        );
    });

    item.rowGroup.get('target_table_space_info').valueChanges.subscribe(res => {
      setValue(item.rowGroup.get('target_table_space_info'), res);
      if (this.isCDB) {
        setValue(item.rowGroup.get('target_table_space'), res ? res[0] : null);
        setValue(item.rowGroup.get('target_pdb_name'), res ? res[1] : null);
      }
    });

    item.rowGroup.statusChanges.subscribe(res => this.setValid());
  }

  private manualPatchValue(extendObj, item) {
    const { user_name, pdb_name, table_space } = extendObj ?? {};
    let targetSpaceInfo = [];
    const userNameIndex = this.targetUserNames.findIndex(
      item => item.value === user_name
    );
    if (userNameIndex !== -1) {
      // 如果targetUser中有和原信息相同的，则默认赋值
      item.rowGroup.get('target_user_name').setValue(user_name);
    }

    if (this.isCDB) {
      /* CDB场景，是级联选择，先选pdb，再选tableSpace。
       * usersTableSpaceMap里存的是
       * {key:username,value:[{key:pdb,value:pdb,children:[tableSpaces,tableSpaces]}]}*/
      const pdbAndTableSpaceArr = this.usersTableSpaceMap.get(user_name);
      if (pdbAndTableSpaceArr) {
        const targetPdb = pdbAndTableSpaceArr.find(
          item => item.value === pdb_name
        );
        if (targetPdb) {
          const targetSpace = targetPdb.children?.find(
            item => item.value === table_space
          );
          targetSpaceInfo = [targetPdb.value, targetSpace?.value];
        }
      }
      item.rowGroup
        .get('target_table_space_info')
        .setValue(compact(targetSpaceInfo));
    } else {
      // 如果targetTableSpaces中有和原信息相同的，则默认赋值
      const tableSpaceIndex = this.targetTableSpaces.findIndex(
        item => item.value === table_space
      );
      if (tableSpaceIndex !== -1) {
        item.rowGroup.get('target_table_space').setValue(table_space);
      }
    }
  }

  updateGroupControl(data, setFlag = true) {
    const controlArr = [
      'target_user_name',
      'target_table_space',
      'target_table_name'
    ];
    const { rowGroup } = data;
    controlArr.forEach(ctrlName => {
      if (setFlag) {
        rowGroup.get(ctrlName).enable();
        rowGroup.get(ctrlName).markAsTouched();
      } else {
        rowGroup.get(ctrlName).disable();
        rowGroup.get(ctrlName).markAsUntouched();
      }
    });
    if (this.isCDB && setFlag) {
      rowGroup.get('target_table_space_info').enable();
      rowGroup.get('target_pdb_name').enable();
      rowGroup.get('target_table_space_info').markAsTouched();
    } else {
      rowGroup.get('target_table_space_info').disable();
      rowGroup.get('target_pdb_name').disable();
      rowGroup.get('target_table_space_info').markAsUntouched();
    }
  }

  checkNameIsEmpty(name: string) {
    return isEmpty(trim(name));
  }

  setValid() {
    this.valid$.next(
      this.formGroup.invalid ||
        this.pdbAbnormal ||
        !this.displayData.length ||
        some(this.displayData, item => item.rowGroup.invalid)
    );
  }

  searchByName(e) {
    const nameIsEmpty = this.checkNameIsEmpty(e);
    if (nameIsEmpty) {
      this.isSearch = false;
      this.initFirstNode();
      return;
    }
    this.isSearch = true;
    this.initFirstNode();
    this.treeData[0].disabled = true;
    const params = {
      parentPath: `/`,
      copyId: this.rowCopy.uuid,
      name: e
    };

    this.copyControllerService.ListCopyCatalogsByName(params).subscribe(res => {
      // 处于搜索状态时，取消父子关联
      const node = this.treeData[0];
      this.addItemToNode(node, res);
      this.cdr.detectChanges();
      this.updateRowData(node);
    });
  }

  addItemToNode(parentNode, result) {
    parentNode.expanded = true;
    for (const item of result.records || result.items || []) {
      item.parentPath = `${parentNode.data.parentPath}/${item.path}`;
      const node = this.createTreeNode(item, parentNode);
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
      icon: this.iconMap[get(nodeData, 'type', RestoreFileType.Directory)],
      uniqueId: nodeData.parentPath,
      isLeaf: nodeData.type === RestoreFileType.File
    };
    if (nodeData.type === RestoreFileType.File) {
      this.assignItemTableInfo(data);
    }
    return {
      data,
      children: [],
      expanded: false,
      disabled: this.isSearch && nodeData.type !== RestoreFileType.File,
      isLeaf: nodeData.type === RestoreFileType.File
    };
  }

  queryTreeData(node: TreeTableNode, specifyPage?: number) {
    // 正常搜索展开时，开启父子关联
    const targetPath: string = node.data?.isFirst
      ? '/'
      : `${node.data.parentPath}`;
    const extParams = {
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      pageNo: specifyPage || CommonConsts.PAGE_START,
      copyId: this.rowCopy.uuid,
      parentPath: targetPath
    };
    this.copyControllerService.ListCopyCatalogs(extParams).subscribe(res => {
      const resource = res.records;
      const childrenNodes: TreeTableNode[] = map(resource, (item: any) => {
        item.parentPath = `${node.data.parentPath}/${item.path}`;
        return this.createTreeNode(item);
      });
      if (last(node.children)?.data?.isMoreBtn) {
        node.children.pop();
      }
      node.children.push(...childrenNodes);
      this.cdr.detectChanges();
      if (res.totalCount > size(node.children)) {
        this.createMoreBtnNode(node, resource);
      }

      node.expanded = true;
      this.updateRowData(node);
    });
  }

  private createMoreBtnNode(node: TreeTableNode, resource) {
    const moreBtnNode: TreeTableNode = {
      data: {
        parentNode: node,
        name: `${this.i18n.get('common_more_label')}...`,
        uniqueId: node.data.uniqueId + '/more',
        pageNo: Math.ceil(size(node.children) / CommonConsts.PAGE_SIZE_MAX),
        isMoreBtn: true,
        disabled: true
      },
      expanded: false,
      disabled: true,
      isLeaf: resource[0]['type'] === RestoreFileType.File
    };
    node.children = [...node.children, moreBtnNode];
  }

  trackByIndex(_, node) {
    return node.data.uniqueId;
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
      this.queryTreeData(item);
      return;
    }
    rowData.forEach((rowItem, index) => {
      rowData[index] = { ...rowItem };
    });
    this.treeData = [...this.treeData];
    this.updateRowData(item);
  }

  selectionChange(data, node?) {
    const selection = map(data, 'data').filter(
      item => item.isLeaf && !item.isMoreBtn
    );
    const removedArr = differenceBy(this.displayData, selection, 'uniqueId');
    selection.forEach(item => this.updateGroupControl(item, true));
    removedArr.forEach(item => this.updateGroupControl(item, false));
    this.displayData = selection;
    this.cdr.detectChanges();
    this.setValid();
  }

  updateRowData(item) {
    // rowData的每个item需要变更地址，触发<td>的变更，拿到每个节点最新的子节点
    this.lvTreeTable.renderData = map(this.lvTreeTable.renderData, rowItem => ({
      ...rowItem
    }));
    // data地址需要变更
    this.treeData = [...this.treeData];
    this.initChildSelection(item);
    // selection地址需要变更
    this.selectionData = [...this.selectionData];
    this.cdr.detectChanges();
  }

  initChildSelection(source) {
    const result = lvFlattenTreeData(this.lvTreeTable.renderData);
    const isSelected = this.selectionData.some(
      item => item.data.uniqueId === source.data.uniqueId && !item.disabled
    );
    let selection = [...this.selectionData];
    if (isSelected) {
      selection.push(...source.children.filter(item => !item.disabled));
    }
    selection = map(selection, node => {
      const target = find(
        result,
        item => item.data.data.uniqueId === node.data.uniqueId
      );
      return target ? target.data : node;
    });
    this.lvTreeTable.bulkSelection(selection);
    this.cdr.detectChanges();
  }

  getProxyOptions() {
    const extParams = {
      queryDependency: true,
      conditions: JSON.stringify({
        subType: [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value
        ]
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
        this.targetHostOptions = map(filterRes, item => ({
          ...item,
          key: item.uuid,
          value: item.rootUuid,
          label: `${item.name}(${item.environment?.endpoint})`,
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
      pageSize: CommonConsts.PAGE_START_EXTRA,
      resourceIds: [host.uuid],
      appType: DataMap.Resource_Type.oracle.value,
      conditions: JSON.stringify({
        queryType: 'pdbInfo',
        hostId: node ? node.hostId : host.rootUuid
      })
    };
    this.appService.ListResourcesDetails(extParams).subscribe(
      res => {
        if (!isEmpty(res.records)) {
          this.formatTableSpaceData(res.records[0]);
          this.lvTreeTable.renderData
            .filter(item => item.isLeaf && !item.data?.isMoreBtn)
            .forEach(item => this.assignItemTableInfo(item.data));
        } else {
          this.pdbAbnormal = false;
        }
      },
      error => {
        this.pdbAbnormal = false;
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
    this.pdbAbnormal = tableSpaceInfo?.pdb_status === false;
    if (tableSpaceInfo.is_cdb) {
      this.formatCDBTableSpaces(tableSpaceInfo);
    } else {
      this.targetUserNames = map(tableSpaceInfo.user_names, item =>
        this.formatOptionItem(item)
      );
      this.targetTableSpaces = map(tableSpaceInfo.table_spaces, item =>
        this.formatOptionItem(item)
      );
    }
    this.setValid();
    this.cdr.detectChanges();
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
        : find(this.targetHostOptions, {
            key: this.formGroup.value.targetHost
          }).value,
      restoreType: RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreTo,
      targetObject: isOrigin
        ? this.resourceProperties.uuid
        : this.formGroup.value.targetHost,
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
      const rowGroupValue = item.rowGroup.getRawValue();
      // CDB场景时选中的结构是['pdb_name','table_space']
      const targetSpaceInfos = this.isCDB
        ? rowGroupValue.target_table_space_info
        : rowGroupValue.target_table_space;
      return {
        user_name: extObj.user_name, // 用户名
        table_name: item.name, // 表名
        pdb_name: extObj.pdb_name || null, // 原pdb名
        table_space: extObj.table_space || null, // 原表空间
        target_pdb_name: this.isCDB
          ? targetSpaceInfos[0]
          : rowGroupValue.target_pdb_name, // 恢复后pdb名，
        target_user_name: rowGroupValue.target_user_name,
        target_table_space: this.isCDB ? targetSpaceInfos[1] : targetSpaceInfos,
        target_table_name: rowGroupValue.target_table_name,
        is_has_foreign_key: extObj?.is_has_foreign_key || ''
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
