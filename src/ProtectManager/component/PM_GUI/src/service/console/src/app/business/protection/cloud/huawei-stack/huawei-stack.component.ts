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
  ChangeDetectorRef,
  Component,
  Input,
  OnDestroy,
  OnInit,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import {
  ApplicationType,
  CommonConsts,
  CookieService,
  DataMap,
  GlobalService,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  PROTECTION_NAVIGATE_STATUS,
  Page_Size_Options,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  RoleOperationMap,
  Table_Size,
  WarningMessageService,
  extendSlaInfo,
  getLabelList,
  getPermissionMenuItem,
  hasResourcePermission
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  defer,
  each,
  find,
  get,
  includes,
  isEmpty,
  isFunction,
  isNumber,
  mapValues,
  uniqueId
} from 'lodash';
import { Observable, Subject, Subscription, forkJoin, of, timer } from 'rxjs';
import { mergeMap, switchMap, takeUntil } from 'rxjs/operators';
import { RegisterHuaWeiStackComponent } from './register-huawei-stack/register-huawei-stack.component';
import { EnvironmentInfoComponent } from './stack-list/environment-info/environment-info.component';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';

@Component({
  selector: 'aui-huawei-stack',
  templateUrl: './huawei-stack.component.html',
  styleUrls: ['./huawei-stack.component.less']
})
export class HuaweiStackComponent implements OnInit, OnDestroy {
  @Input() resType = ApplicationType.HCSCloudHost;
  resLabel = this.i18n.get('common_cloud_label');
  cloudServerLabel = this.i18n.get('common_cloud_server_label');
  dataMap = DataMap;
  ResourceType = ResourceType;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  treeSelection = [];
  treeData = [];
  expandedNodeList = [];
  moreMenus = [];
  cloudGroupTotal = 0;
  isAuthCase = false;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  destroy$ = new Subject();
  // 后台修改后，hcs左侧树查询，展开时传子资源的subType
  queryTreeMap = {
    [DataMap.Resource_Type.HCSContainer.value]:
      DataMap.Resource_Type.HCSTenant.value,
    [DataMap.Resource_Type.HCSTenant.value]:
      DataMap.Resource_Type.HCSRegion.value,
    [DataMap.Resource_Type.HCSRegion.value]:
      DataMap.Resource_Type.HCSProject.value,
    [DataMap.Resource_Type.HCSProject.value]:
      DataMap.Resource_Type.HCSCloudHost.value
  };
  // 右侧菜单栏
  tabs: any = [
    {
      id: ResourceType.TENANT,
      label: this.i18n.get('common_tenant_else_label'),
      total: 0,
      sub: null,
      type: ResourceType.TENANT,
      sub_type: 'HCSTenant'
    },
    {
      id: ResourceType.PROJECT,
      label: this.i18n.get('common_project_resource_label'),
      total: 0,
      sub: null,
      type: ResourceType.PROJECT,
      sub_type: 'HCSProject'
    },
    {
      id: ResourceType.CLOUD_HOST,
      label: this.i18n.get('common_cloud_server_label'),
      total: 0,
      sub: null,
      type: ResourceType.CLOUD_HOST,
      sub_type: 'HCSCloudHost'
    }
  ];
  treeNodeClick = false;
  activeIndex = this.tabs[0].id;

  roleOperationMap = RoleOperationMap;

  @ViewChild('stkList', { static: false }) stkListComponent;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private cookieService: CookieService,
    public globalService: GlobalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private messageService: MessageService,
    private cdr: ChangeDetectorRef,
    public virtualScroll: VirtualScrollService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.getResInfo();
    this.getMoreMenus();
    this.getTree();
    window.addEventListener('resize', () => {
      this.virtualScroll.getScrollParam(
        240,
        Page_Size_Options.Three,
        Table_Size.Default,
        'hcs-tree'
      );
      this.cdr.detectChanges();
    });
  }

  onChange() {
    this.treeSelection = [];
    this.treeData = [];
    this.expandedNodeList = [];
    this.moreMenus = [];
    this.ngOnInit();
  }

  getOptsItems(data) {
    const menus = [
      {
        id: 'rescan',
        disabled: !this.treeSelection.length || !hasResourcePermission(data),
        permission: OperateItems.RescanVirtualizationPlatform,
        label: this.i18n.get('common_rescan_label'),
        onClick: () => this.rescanEnv()
      },
      {
        id: 'connectivityTest',
        divide: true,
        disable: this.isAuthCase || !hasResourcePermission(data),
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('protection_connectivity_test_label'),
        onClick: () => {
          this.connectTest();
        }
      },
      {
        id: 'modify',
        label: this.i18n.get('common_modify_register_label'),
        disabled:
          !this.treeSelection.length ||
          this.isAuthCase ||
          !hasResourcePermission(data),
        permission: OperateItems.ModifyHCSTenant,
        onClick: () => {
          this.protectedResourceApiService
            .ShowResource({
              resourceId: this.treeSelection[0].uuid
            })
            .subscribe((res: any) => {
              this.register(res);
            });
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('protection_unregister_label'),
        disabled:
          !this.treeSelection.length ||
          this.isAuthCase ||
          !hasResourcePermission(data),
        permission: OperateItems.UnRegisterHCSTenant,
        onClick: () => {
          this.warningMessageService.create({
            content: this.i18n.get('protection_hcs_resource_unregister_label', [
              this.treeSelection[0].name,
              this.i18n.get('protection_canncel_huawei_stack_project_label')
            ]),
            onOK: () => {
              this.deleteRegister();
            }
          });
        }
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  }

  getMoreMenus() {
    const menus = [
      {
        id: 'environmentInfo',
        divide: true,
        disabled: !this.treeSelection.length || this.isAuthCase,
        permission: OperateItems.HCSEnvironmentInfo,
        label: this.i18n.get('common_environment_info_label'),
        onClick: () => {
          this.getEnvironment();
        }
      },
      {
        id: 'rescan',
        disabled: !this.treeSelection.length,
        permission: OperateItems.RescanVirtualizationPlatform,
        label: this.i18n.get('common_rescan_label'),
        onClick: () => this.rescanEnv()
      },
      {
        id: 'connectivityTest',
        disabled: !this.treeSelection.length || this.isAuthCase,
        divide: true,
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('protection_connectivity_test_label'),
        onClick: () => {
          this.connectTest();
        }
      },
      {
        id: 'modify',
        label: this.i18n.get('common_modify_register_label'),
        disabled: !this.treeSelection.length || this.isAuthCase,
        permission: OperateItems.ModifyHCSTenant,
        onClick: () => {
          this.protectedResourceApiService
            .ShowResource({
              resourceId: this.treeSelection[0].uuid
            })
            .subscribe((res: any) => {
              this.register(res);
            });
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('protection_unregister_label'),
        disabled: !this.treeSelection.length || this.isAuthCase,
        permission: OperateItems.UnRegisterHCSTenant,
        onClick: () => {
          this.warningMessageService.create({
            content: this.i18n.get('protection_hcs_resource_unregister_label', [
              this.treeSelection[0].name,
              this.i18n.get('protection_canncel_huawei_stack_project_label')
            ]),
            onOK: () => {
              this.deleteRegister();
            }
          });
        }
      }
    ];
    this.moreMenus = getPermissionMenuItem(menus, this.cookieService.role);
  }

  getResInfo() {
    this.resLabel = {
      [ApplicationType.HCSCloudHost]: this.i18n.get('common_cloud_label')
    }[this.resType];

    // 总览跳转过来的过滤状态
    let protectionStatus = '';
    if (
      !isEmpty(PROTECTION_NAVIGATE_STATUS.protectionStatus) ||
      isNumber(PROTECTION_NAVIGATE_STATUS.protectionStatus)
    ) {
      protectionStatus = PROTECTION_NAVIGATE_STATUS.protectionStatus;
      PROTECTION_NAVIGATE_STATUS.protectionStatus = '';
    }

    // 资源类型放在tab中不再单独增加接口，HyperV无集群信息
    this.tabs.forEach(tab => {
      if (!isEmpty(protectionStatus) || isNumber(protectionStatus)) {
        tab.protectionStatus = protectionStatus;
      }
      tab.resType = this.resType;
    });
  }

  getTree() {
    this.pageSize = CommonConsts.PAGE_SIZE;
    this.pageIndex = CommonConsts.PAGE_START;
    this.getTreeData();
  }

  getTreeData(event?, startPage?, reqCb?) {
    this.isAuthCase = false;
    if (startPage === undefined) {
      this.treeData = []; // 清空原数据
      startPage = CommonConsts.PAGE_START;
    }
    const params = {
      pageNo: startPage,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        subType: ResourceType.HCS_CONTAINER,
        type: ResourceType.HCS
      })
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      isFunction(reqCb) && reqCb(res);
      if (isEmpty(res.records)) {
        this.isAuthCase = true;
        this.getProjectTreeNode(event);
        return;
      }
      each(res.records, (item: any) => {
        const node = {
          label: item.name,
          name: item.name,
          authorized_user: item.authorizedUser,
          contentToggleIcon: this.getResourceIcon(item),
          type: item.type as string,
          subType: item.subType,
          protectionStatus: item['protectionStatus'],
          rootNodeSubType: item.type,
          uuid: item.uuid,
          rootUuid: item.rootUuid,
          path: item.path,
          endpoint: item['endpoint'],
          port: item['port'],
          auth: item.auth,
          userId: item.userId,
          userName: item['username'],
          link_status: item['linkStatus'],
          children: [],
          isLeaf: false,
          expanded: this.getExpandedIndex(item.uuid) !== -1,
          resourceRoleAuth: item.resourceRoleAuth
        };
        if (node.expanded) {
          this.getExpandedChangeData(CommonConsts.PAGE_START, node);
        }

        if (!find(this.treeData, { uuid: node.uuid })) {
          this.treeData.push(node);
        }
      });
      startPage++;
      if (res.totalCount - startPage * CommonConsts.PAGE_SIZE * 10 > 0) {
        this.getTreeData(null, startPage);
        return;
      }
      this.treeData = [...this.treeData];
      if (this.treeData.length) {
        if (!this.treeSelection.length) {
          this.treeSelection = [this.treeData[0]];
        } else {
          this.treeData.forEach(node => {
            if (node.uuid === this.treeSelection[0].uuid) {
              this.treeSelection = [node];
            }
          });
        }
        this.initMoreMenus();
        this.getTableData(event);
      } else {
        this.tabs.forEach(tab => {
          tab.tableData = [];
          tab.total = 0;
          tab.resourceTotal = 0;
        });
      }
      this.virtualScroll.getScrollParam(
        240,
        Page_Size_Options.Three,
        Table_Size.Default,
        'hcs-tree'
      );
      this.cdr.detectChanges();
    });
  }

  getShowData(type): Observable<any> {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({ type, subType: ResourceType.HCSProject })
    };
    let curData = [];
    return this.protectedResourceApiService.ListResources(params).pipe(
      mergeMap((response: any) => {
        curData = [of(response)];

        const totalCount = response.totalCount;
        const pageCount = Math.ceil(totalCount / (CommonConsts.PAGE_SIZE * 10));
        for (let i = 2; i <= pageCount; i++) {
          curData.push(
            this.protectedResourceApiService.ListResources({
              pageNo: i,
              pageSize: CommonConsts.PAGE_SIZE * 10,
              conditions: JSON.stringify({ type })
            })
          );
        }
        return forkJoin(curData);
      })
    );
  }

  getProjectTreeNode(event?) {
    this.getShowData(ResourceType.PROJECT).subscribe(res => {
      const totalData = [];
      for (const item of res) {
        totalData.push(...item.records);
      }
      if (isEmpty(totalData)) {
        return;
      }

      this.assembleAuthTree(totalData, event);
    });
  }

  assembleAuthTree(data, event) {
    const map = new Map();
    map.set('root', []);
    const rootPathObj = {};
    for (const item of data) {
      const pathArr = item.path.split('/');
      // tslint:disable-next-line: prefer-for-of
      let lastPath = '';
      for (let i = 0; i < pathArr.length; i++) {
        const tmpPath = i > 0 ? lastPath + '/' + pathArr[i] : pathArr[i];
        if (i === 0) {
          if (!map.get('root').includes(tmpPath)) {
            map.get('root').push(tmpPath);
            if (isEmpty(rootPathObj[tmpPath])) {
              rootPathObj[tmpPath] = [];
              rootPathObj[tmpPath].push(item.rootUuid);
              rootPathObj[tmpPath].push(item.extendInfo?.envName || item.name);
              if (item.subType === 'HcsEnvOp') {
                rootPathObj[tmpPath].push(item.subType);
              }
            }
          }
        } else {
          if (map.has(lastPath)) {
            if (!map.get(lastPath).includes(tmpPath)) {
              map.get(lastPath).push(tmpPath);
            }
          } else {
            map.set(lastPath, []);
            map.get(lastPath).push(tmpPath);
          }
        }
        lastPath = tmpPath;
      }
    }

    const treeObj = [];
    const root = map.get('root');
    for (const item of root) {
      const rootItem = {
        label: rootPathObj[item][1],
        name: rootPathObj[item][1],
        path: item,
        isLeaf: rootPathObj[item][2] === 'HcsEnvOp',
        children: [],
        contentToggleIcon:
          rootPathObj[item][2] === 'HcsEnvOp'
            ? this.getAuthTreeIcon(null)
            : this.getAuthTreeIcon(0),
        depth: 0,
        expanded: rootPathObj[item][2] !== 'HcsEnvOp',
        uuid: rootPathObj[item][0] || uniqueId(),
        type: DataMap.Resource_Type.HCS.value
      };
      treeObj.push(rootItem);
      this.generateTree(map, rootItem, data);
    }
    this.treeData = treeObj;

    if (this.treeData.length) {
      if (!this.treeSelection.length) {
        this.treeSelection = [this.treeData[0]];
      } else {
        this.treeData.forEach(node => {
          if (node.uuid === this.treeSelection[0].uuid) {
            this.treeSelection = [node];
          }
        });
      }
      this.initMoreMenus();
      this.getTableData(event);
    } else {
      this.tabs.forEach(tab => {
        tab.tableData = [];
        tab.total = 0;
        tab.resourceTotal = 0;
      });
    }
    this.cdr.detectChanges();
  }

  generateTree(map, rootItem, data) {
    const childPath = map.get(rootItem.path);
    if (isEmpty(childPath)) {
      return;
    }
    for (const child of childPath) {
      const pathArr = child.split('/');
      const newItem = pathArr[pathArr.length - 1];
      const childItem = {
        label: newItem,
        name: newItem,
        path: child,
        contentToggleIcon: this.getAuthTreeIcon(rootItem.depth + 1),
        isLeaf: rootItem.depth === 2,
        children: [],
        depth: rootItem.depth + 1,
        expanded: true,
        uuid: find(data, { path: child })?.uuid || uniqueId(),
        type: this.getAuthTreeType(rootItem.depth + 1)
      };
      rootItem.children.push(childItem);
      this.generateTree(map, childItem, data);
    }
  }

  getAuthTreeIcon(depth) {
    switch (depth) {
      case 0:
        return 'aui-icon-hcs-platform';
      case 1:
        return 'aui-icon-hcs-tenant';
      case 2:
        return 'aui-icon-hcs-region';
      default:
        return 'aui-icon-hcs-project';
    }
  }

  getAuthTreeType(depth) {
    switch (depth) {
      case 1:
        return DataMap.Resource_Type.Tenant.value;
      case 2:
        return DataMap.Resource_Type.Region.value;
      default:
        return DataMap.Resource_Type.Project.value;
    }
  }

  initMoreMenus() {
    this.moreMenus.forEach(item => {
      if (item.id === 'resourceAuth') {
        item.disabled = this.treeSelection.find(node => {
          return (
            !!node.userId ||
            !!node.slaId ||
            !includes(
              [
                DataMap.Resource_Type.HCS.value,
                DataMap.Resource_Type.Project.value,
                DataMap.Resource_Type.Tenant.value
              ],
              node.rootNodeType
            ) ||
            this.isAuthCase
          );
        });
      } else if (item.id === 'resourceReclaiming') {
        item.disabled = this.treeSelection.find(node => {
          return (
            !node.userId ||
            !includes(
              [
                DataMap.Resource_Type.HCS.value,
                DataMap.Resource_Type.Project.value,
                DataMap.Resource_Type.Tenant.value
              ],
              node.rootNodeType
            ) ||
            this.isAuthCase
          );
        });
      } else {
        item.disabled = this.isAuthCase;
      }
    });
  }

  expandedChange(event) {
    if (event.expanded) {
      this.expandedNodeList.push({
        uuid: event.uuid,
        rootUuid: event.rootUuid
      });
    } else {
      this.expandedNodeList.splice(this.getExpandedIndex(event.uuid), 1);
    }
    if (!event.expanded || event.children.length) {
      return;
    }
    event.children = [];
    this.getExpandedChangeData(CommonConsts.PAGE_START, event);
  }

  getRootNode(node) {
    if (!!node.parent) {
      return this.getRootNode(node.parent);
    } else {
      return node;
    }
  }

  getExpandedChangeData(startPage, event) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        subType: get(
          this.queryTreeMap,
          event.subType,
          DataMap.Resource_Type.HCSContainer.value
        ),
        parentUuid: event.uuid,
        visible: event.subType === ResourceType.HCS_CONTAINER ? '1' : null
      })
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      res.records.forEach((item: any) => {
        if (item.type !== ResourceType.HCS) {
          const rootNode = this.getRootNode(event),
            node = {
              label: item.name,
              contentToggleIcon: this.getResourceIcon(item),
              type: item.type as string,
              subType: item?.subType,
              rootNodeSubType: event.rootNodeType,
              rootUuid: item.rootUuid,
              uuid: item.uuid,
              path: item.path,
              children: [],
              rootNodeLinkStatus: includes(
                [
                  DataMap.Resource_Type.HCS.value,
                  DataMap.Resource_Type.Project.value
                ],
                rootNode.rootNodeType
              )
                ? rootNode.link_status
                : DataMap.resource_LinkStatus.normal.value,
              isLeaf: item.type === ResourceType.PROJECT,
              expanded: this.getExpandedIndex(item.uuid) !== -1,
              resourceRoleAuth: item.resourceRoleAuth
            };
          if (node.expanded) {
            this.getExpandedChangeData(CommonConsts.PAGE_START, node);
          }
          event.children.push(node);
        }
      });
      startPage++;
      if (res.totalCount - startPage * CommonConsts.PAGE_SIZE * 10 > 0) {
        this.getExpandedChangeData(startPage, event);
        return;
      }
      this.treeData = [...this.treeData];
      this.cdr.detectChanges();
    });
  }

  getExpandedIndex(id) {
    return this.expandedNodeList.findIndex(node => node.uuid === id);
  }

  getResourceIcon(node) {
    const nodeResource = find(
      DataMap.Resource_Type,
      item => item.value === node.type
    );
    return nodeResource['icon'] + '';
  }

  nodeCheck(e) {
    let tabIdList = [ResourceType.CLOUD_HOST];
    if (e.node.type === ResourceType.HCS) {
      tabIdList = [
        ResourceType.TENANT,
        ResourceType.PROJECT,
        ResourceType.CLOUD_HOST
      ];
    } else if (
      [ResourceType.TENANT, ResourceType.Region].includes(e.node.type)
    ) {
      tabIdList = [ResourceType.PROJECT, ResourceType.CLOUD_HOST];
    }
    this.tabs.forEach(tab => {
      tab.hidden = !tabIdList.includes(tab.id);
      if (tab.id === this.activeIndex && tab.hidden) {
        // 切换tab而且当前tab隐藏的时候，就切到租户
        this.activeIndex = ResourceType.CLOUD_HOST;
      }
    });
    this.treeNodeClick = true;
    this.initMoreMenus();
    this.getTableData(null);
  }

  beforeSelected = item => {
    if (this.treeSelection[0].uuid === item.uuid) {
      return false;
    }
  };

  // 展示环境信息
  getEnvironment() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'show-hcs-information',
        lvWidth: MODAL_COMMON.normalWidth + 200,
        lvHeader: this.treeSelection[0].name,
        lvContent: EnvironmentInfoComponent,
        lvComponentParams: {
          treeSelection: assign({}, this.treeSelection[0], {
            optItems: this.getOptsItems(this.treeSelection[0]),
            optItemsFn: v => {
              return this.getOptsItems(v);
            }
          })
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  connectTest() {
    this.protectedResourceApiService
      .CheckProtectedResource({ resourceId: this.treeSelection[0].uuid })
      .subscribe(res => {
        this.messageService.success(this.i18n.get('job_status_success_label'), {
          lvMessageKey: 'successKey',
          lvShowCloseButton: true
        });
      });
  }

  deleteRegister() {
    this.protectedEnvironmentApiService
      .DeleteProtectedEnvironment({
        envId: this.treeSelection[0].uuid
      })
      .subscribe(() => {
        this.expandedNodeList.splice(
          this.expandedNodeList.findIndex(
            node => node.rootUuid === this.treeSelection[0].uuid
          ),
          1
        );
        this.treeSelection = [];
        this.getTreeData();
        this.cdr.detectChanges();
      });
  }

  rescanEnv() {
    this.protectedResourceApiService
      .ScanProtectedResources({
        resId: this.treeSelection[0].uuid
      })
      .subscribe(() => this.getTreeData());
  }

  register(item?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'register-huawei-cloud-stack',
        lvWidth: MODAL_COMMON.normalWidth + 200,
        lvHeader: this.i18n.get(
          `${isEmpty(item) ? 'common_register_label' : 'common_modify_label'}`
        ),
        lvContent: RegisterHuaWeiStackComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          treeSelection: this.treeSelection,
          item
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterHuaWeiStackComponent;
            const ip = content.formGroup.value.ip;
            (isEmpty(item) ? content.create() : content.modify()).subscribe(
              res => {
                const getTreeDataCb = () =>
                  isEmpty(item) &&
                  defer(() => {
                    this.stkListComponent?.addTelnet(...[, ip]);
                  }); // 体验优化 新注册HCS 直接弹出添加租户界面
                resolve(true);
                this.getTreeData(...[, , getTreeDataCb]);
              },
              error => resolve(false)
            );
          });
        }
      })
    );
  }

  getTableData(event?) {
    if (!this.treeSelection.length) {
      return;
    }
    this.tabs.forEach(item => {
      if (item.sub as Subscription) {
        item.sub.unsubscribe();
      }
      if (item.hidden) {
        return;
      }
      const conditions: any = {
        subType: item.sub_type,
        path: [['=~'], this.treeSelection[0].path + '/'],
        type: item.type
      };

      if (item.type === ResourceType.TENANT) {
        conditions['visible'] = '1';
      }

      if (event && event.tabType === item.id) {
        assign(conditions, event.source);
        if (event.paginator) {
          this.pageIndex = event.paginator.pageIndex;
          this.pageSize = event.paginator.pageSize;
        }
      }

      item.sub = timer(0, CommonConsts.TIME_INTERVAL_RESOURCE)
        .pipe(
          switchMap(index => {
            return this.protectedResourceApiService.ListResources({
              pageNo: this.pageIndex,
              pageSize: this.pageSize,
              conditions: JSON.stringify(conditions),
              akLoading: !index
            });
          }),
          takeUntil(this.destroy$)
        )
        .subscribe(res => {
          each(res.records, item => {
            // 获取标签数据
            const { showList, hoverList } = getLabelList(item);
            assign(item, {
              sub_type: item.subType,
              cloudHostCount: +item.extendInfo?.cloudHostCount || 0,
              projectCount: +item.extendInfo?.projectCount || 0,
              status: JSON.parse(item.extendInfo?.host || '{}')?.status,
              showLabelList: showList,
              hoverLabelList: hoverList
            });
            extendSlaInfo(item);
          });
          item.tableData = res.records;
          item.total = res.totalCount;
          if (
            includes(
              mapValues(this.drawModalService.modals, 'key'),
              'detail-modal'
            ) &&
            this.stkListComponent.currentDetailItemUuid &&
            find(res.records, {
              uuid: this.stkListComponent.currentDetailItemUuid
            })
          ) {
            this.globalService.emitStore({
              action: 'autoReshResource',
              state: find(res.records, {
                uuid: this.stkListComponent.currentDetailItemUuid
              })
            });
          }
          if (isEmpty(event?.source)) {
            item.resourceTotal = res.totalCount;
          }
          this.cdr.detectChanges();
        });
    });
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getTreeData(event);
  }
  updateGroupTotal(event) {
    this.cloudGroupTotal = event?.total;
  }

  tabIndexChange(e) {
    if (this.treeNodeClick) {
      this.treeNodeClick = false;
      return;
    }
    this.pageSize = CommonConsts.PAGE_SIZE;
    this.pageIndex = CommonConsts.PAGE_START;
    this.getTableData();
  }

  afterTabChange = (origin, active) => {
    each(this.tabs, item => {
      if (origin && item.id === origin.lvId) {
        item.protectionStatus = '';
      }
    });
  };
}
