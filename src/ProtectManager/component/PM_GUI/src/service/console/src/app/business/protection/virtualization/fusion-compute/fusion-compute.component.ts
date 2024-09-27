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
  CommonConsts,
  CookieService,
  DataMap,
  GlobalService,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  PROTECTION_NAVIGATE_STATUS,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  RoleOperationAuth,
  RoleOperationMap,
  WarningMessageService,
  extendSlaInfo,
  getLabelList,
  getPermissionMenuItem,
  hasResourcePermission
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  each,
  find,
  includes,
  isEmpty,
  isNumber,
  last,
  mapValues
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { EnvironmentInfoComponent } from './fusion-list/environment-info/environment-info.component';
import { RegisterFusionComputeComponent } from './register-fusion-compute/register-fusion-compute.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-fusion-compute',
  templateUrl: './fusion-compute.component.html',
  styleUrls: ['./fusion-compute.component.less']
})
export class FusionComputeComponent implements OnInit, OnDestroy {
  @Input() resType = ResourceType.FUSION_COMPUTE;
  resLabel = '';

  ResourceType = ResourceType;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  treeSelection = [];
  treeData = [];
  expandedNodeList = [];
  moreMenus = [];
  dataMap = DataMap;
  cloudGroupTotal = 0;

  destroy$ = new Subject();

  // 右侧菜单栏
  tabs: any = [
    {
      id: ResourceType.CLUSTER,
      label: this.i18n.get('common_clusters_label'),
      total: 0,
      sub: null,
      type: ResourceType.CLUSTER
    },
    {
      id: ResourceType.HOST,
      label: this.i18n.get('common_host_label'),
      total: 0,
      sub: null,
      type: ResourceType.HOST
    },
    {
      id: ResourceType.VM,
      label: this.i18n.get('common_virtual_machine_label'),
      total: 0,
      sub: null,
      type: ResourceType.VM
    }
  ];
  treeNodeClick = false;
  activeIndex = this.tabs[0].id;

  roleOperationMap = RoleOperationMap;
  roleOperationAuth = RoleOperationAuth;

  registerTipShow = false;

  @ViewChild('fcList', { static: false }) fcListComponent;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private cookieService: CookieService,
    public globalService: GlobalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private messageService: MessageService,
    private cdr: ChangeDetectorRef
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.getResInfo();
    this.getMoreMenus();
    this.getTree();
    this.showGuideTab();
    this.getUserGuideState();
    this.showRegisterTip();
  }

  getUserGuideState() {
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        this.showRegisterTip();
        this.showGuideTab();
      });
  }

  showRegisterTip() {
    if (
      USER_GUIDE_CACHE_DATA.active &&
      USER_GUIDE_CACHE_DATA.showTips &&
      includes(this.roleOperationAuth, this.roleOperationMap.manageResource)
    ) {
      setTimeout(() => {
        this.registerTipShow = true;
        USER_GUIDE_CACHE_DATA.showTips = false;
        this.cdr.detectChanges();
      });
    }
  }

  showGuideTab() {
    if (USER_GUIDE_CACHE_DATA.active && USER_GUIDE_CACHE_DATA.activeTab) {
      setTimeout(() => {
        this.activeIndex = <any>USER_GUIDE_CACHE_DATA.activeTab;
      });
    }
  }

  lvPopoverBeforeClose = () => {
    this.registerTipShow = false;
    this.cdr.detectChanges();
  };

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
        disabled: !hasResourcePermission(data),
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
        disabled: !this.treeSelection.length || !hasResourcePermission(data),
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
        disabled: !this.treeSelection.length || !hasResourcePermission(data),
        permission: OperateItems.UnRegisterHCSTenant,
        onClick: () => {
          this.warningMessageService.create({
            content: this.i18n.get('protection_fc_resource_unregister_label', [
              this.treeSelection[0].name,
              this.i18n.get('protection_cluster_host_vm_label')
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
        disabled: !this.treeSelection.length,
        permission: OperateItems.HCSEnvironmentInfo,
        label: this.i18n.get('common_environment_info_label'),
        onClick: () => {
          this.getEnvironment();
        }
      },
      {
        id: 'rescan',
        divide: true,
        disabled: !this.treeSelection.length,
        permission: OperateItems.RescanVirtualizationPlatform,
        label: this.i18n.get('common_rescan_label'),
        onClick: () => this.rescanEnv()
      },
      {
        id: 'modify_register',
        label: this.i18n.get('common_modify_register_label'),
        disabled: !this.treeSelection.length,
        permission: OperateItems.ModifyVirtualizationRegister,
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
        disabled: !this.treeSelection.length,
        permission: OperateItems.DeregisterVirtualizationPlatform,
        onClick: () => {
          this.warningMessageService.create({
            content: this.i18n.get('protection_fc_resource_unregister_label', [
              this.treeSelection[0].name,
              this.i18n.get('protection_cluster_host_vm_label')
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
      [ResourceType.VM]: this.i18n.get('common_vmware_label'),
      [ResourceType.PLATFORM]: 'Platform',
      [ResourceType.FUSION_COMPUTE]: this.i18n.get(
        'common_fusion_compute_label'
      )
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

  getTreeData(event?, startPage?) {
    if (startPage === undefined) {
      this.treeData = []; // 清空原数据
      startPage = CommonConsts.PAGE_START;
    }
    const params = {
      pageNo: startPage,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        subType: this.resType || ResourceType.FUSION_COMPUTE,
        type: ResourceType.PLATFORM
      })
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, (item: any) => {
        if (item.type === 'VM') {
          return;
        }
        const node = {
          label: item.name,
          name: item.name,
          authorized_user: item.authorizedUser,
          contentToggleIcon: this.getResourceIcon(item),
          authorize_user_label: !isEmpty(item.authorizedUser)
            ? 'aui-fc-auth-user'
            : 'aui-fc-no-auth-user',
          type: item.type,
          protectionStatus: item['protectionStatus'],
          rootNodeType: item.type,
          uuid: item.uuid,
          rootUuid: item.rootUuid,
          path: item.path,
          endpoint: item['endpoint'],
          port: item['port'],
          auth: item.auth,
          userId: item.userId,
          userName: item['username'],
          linkStatus: item['linkStatus'],
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
        if (
          DataMap.Resource_Type.fusionComputeCNA.value ===
          this.treeSelection[0].rootNodeType
        ) {
          this.tabs[2].hidden = true;
          this.tabs[2].total = 0;
          this.tabs[2].resourceTotal = 0;
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
    });
  }

  initMoreMenus() {
    this.moreMenus.forEach(item => {
      if (item.id !== 'environmentInfo') {
        item.disabled = this.treeSelection.find(node => {
          return !hasResourcePermission(node);
        });
      } else {
        item.disabled = false;
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
        parentUuid: event.uuid
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      res.records.forEach((item: any) => {
        if (item.type !== ResourceType.PLATFORM) {
          if (item.type === 'VM') {
            return;
          }
          const rootNode = this.getRootNode(event),
            node = {
              label: item.name,
              contentToggleIcon: this.getResourceIcon(item),
              type: item.type,
              rootNodeType: event.rootNodeType,
              rootUuid: item.rootUuid,
              uuid: item.uuid,
              path: item.path,
              children: [],
              rootNodeLinkStatus: includes(
                [
                  DataMap.Resource_Type.fusionComputePlatform.value,
                  DataMap.Resource_Type.fusionComputeCNA.value
                ],
                rootNode.rootNodeType
              )
                ? rootNode.link_status
                : DataMap.resource_LinkStatus.normal.value,
              isLeaf: item.type === ResourceType.VM,
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
    if (
      node.type === ResourceType.PLATFORM &&
      node.linkStatus !== DataMap.resource_LinkStatus_Special.normal.value
    ) {
      return 'aui-icon-vCenter-offine';
    }
    return nodeResource['icon'] + '';
  }

  nodeCheck(e) {
    let tabIdList = [ResourceType.VM];
    if (e.node.type === ResourceType.PLATFORM) {
      tabIdList = [ResourceType.VM, ResourceType.HOST, ResourceType.CLUSTER];
    } else if (e.node.type === ResourceType.CLUSTER) {
      tabIdList = [ResourceType.VM, ResourceType.HOST];
    }
    this.tabs.forEach(tab => {
      tab.hidden = !tabIdList.includes(tab.id);
    });
    this.activeIndex = find(this.tabs, item => !item.hidden)?.id;
    this.treeNodeClick = true;
    this.initMoreMenus();
    this.getTableData(null);
  }

  beforeSelected = item => {
    if (this.treeSelection[0].uuid === item.uuid) {
      return false;
    }
  };

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
        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'show-fusion-compute-environment-info'
          )
        ) {
          this.drawModalService.destroyModal(
            'show-fusion-compute-environment-info'
          );
        }
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
    const modalParam = {
      ...MODAL_COMMON.generateDrawerOptions(),
      lvType: 'drawer',
      lvWidth: 750,
      lvHeader: this.i18n.get(
        `${isEmpty(item) ? 'common_register_label' : 'common_modify_label'}`
      ),
      lvOkDisabled: true,
      lvContent: RegisterFusionComputeComponent,
      lvComponentParams: {
        item,
        treeSelection: this.treeSelection,
        resType: this.resType
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as RegisterFusionComputeComponent;
          (isEmpty(item) ? content.create() : content.modify()).subscribe(
            res => {
              resolve(true);
              this.getTreeData();
            },
            error => resolve(false)
          );
        });
      }
    };
    this.drawModalService.create(modalParam);
  }

  // 展示环境信息
  getEnvironment() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'show-fusion-compute-environment-info',
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
        subType: this.resType || ResourceType.FUSION_COMPUTE,
        path: [['=~'], this.treeSelection[0].path],
        type: item.type
      };

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
            if (item.type === ResourceType.VM) {
              assign(item, {
                vmId: last(item.extendInfo?.moReference?.split('/')),
                status: item.extendInfo.status
              });
            }
            assign(item, {
              sub_type: item.subType,
              vmNumber: +item.extendInfo?.vmNumber || 0,
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
            this.fcListComponent.currentDetailItemUuid &&
            find(res.records, {
              uuid: this.fcListComponent.currentDetailItemUuid
            })
          ) {
            this.globalService.emitStore({
              action: 'autoReshResource',
              state: find(res.records, {
                uuid: this.fcListComponent.currentDetailItemUuid
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
    setTimeout(() => {
      this.getTableData();
    });
  }

  afterTabChange = (origin, active) => {
    each(this.tabs, item => {
      if (origin && item.id === origin.lvId) {
        item.protectionStatus = '';
      }
    });
  };
}
