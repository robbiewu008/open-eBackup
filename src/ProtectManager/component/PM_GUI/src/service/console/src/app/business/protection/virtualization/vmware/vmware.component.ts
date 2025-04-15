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
  OnDestroy,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  DataMap,
  EnvironmentsService,
  GlobalService,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  PROTECTION_NAVIGATE_STATUS,
  Page_Size_Options,
  ProtectedEnvironmentService,
  ResourceType,
  RoleOperationAuth,
  RoleOperationMap,
  Table_Size,
  VirtualResourceService,
  WarningMessageService,
  getLabelList,
  getPermissionMenuItem,
  hasResourcePermission
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  find,
  includes,
  isEmpty,
  isNumber,
  map,
  trimEnd,
  trim,
  mapValues
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { EnvironmentInfoComponent } from './environment-info/environment-info.component';
import { RegisterVmComponent } from './register-vm/register-vm.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-vmware',
  templateUrl: './vmware.component.html',
  styleUrls: ['./vmware.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class VmwareComponent implements OnInit, OnDestroy {
  //  接入hyperv
  @Input() resType = ResourceType.VM;
  resLabel = '';
  assginLabel = this.i18n.get('common_assigned_label', ['']);
  unAssignLabel = this.i18n.get('common_unassigned_label');
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

  tabs: any = [
    {
      id: ResourceType.CLUSTER,
      label: this.i18n.get('common_clusters_label'),
      total: 0,
      sub: null
    },
    {
      id: ResourceType.HOST,
      label: this.i18n.get('common_host_label'),
      total: 0,
      sub: null
    },
    {
      id: ResourceType.VM,
      label: this.i18n.get('common_virtual_machine_label'),
      total: 0,
      sub: null
    }
  ];
  treeNodeClick = false;
  activeIndex = this.tabs[0].id;

  roleOperationMap = RoleOperationMap;
  roleOperationAuth = RoleOperationAuth;

  registerTipShow = false;

  @ViewChild('vmList', { static: false }) vmListComponent;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private environmentsService: EnvironmentsService,
    private protectedEnvironmentService: ProtectedEnvironmentService,
    private virtualResourceService: VirtualResourceService,
    private cookieService: CookieService,
    public globalService: GlobalService,
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
    this.getUserGuideState();
    this.showRegisterTip();
    this.showGuideTab();
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

  getMoreMenus() {
    const menus = [
      {
        id: 'environmentInfo',
        disabled: !this.treeSelection.length,
        permission: OperateItems.RescanVirtualizationPlatform,
        label: this.i18n.get('common_environment_info_label'),
        onClick: () => {
          this.getEnvironment();
        }
      },
      {
        id: 'rescan',
        disabled: !this.treeSelection.length,
        divide: true,
        permission: OperateItems.RescanVirtualizationPlatform,
        label: this.i18n.get('common_rescan_label'),
        onClick: () => this.rescanEnv()
      },
      {
        id: 'modifyRegister',
        label: this.i18n.get('common_modify_register_label'),
        disabled: !this.treeSelection.length,
        permission: OperateItems.ModifyVirtualizationRegister,
        onClick: () => {
          this.modifyRegister();
        }
      },
      {
        id: 'unregister',
        label: this.i18n.get('common_unregister_label'),
        disabled: !this.treeSelection.length,
        permission: OperateItems.DeregisterVirtualizationPlatform,
        onClick: () => {
          this.warningMessageService.create({
            content: this.i18n.get(
              this.resType === ResourceType.HYPERV
                ? 'protection_hyperv_resource_unregister_label'
                : 'protection_vm_resource_unregister_label',
              [
                this.treeSelection[0].name,
                this.i18n.get('protection_cluster_host_vm_label')
              ]
            ),
            onOK: () => {
              this.deleteRegister();
            }
          });
        }
      }
    ];
    this.moreMenus = getPermissionMenuItem(menus, this.cookieService.role);
  }

  getEnvironment() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'show-vm-information',
        lvWidth: MODAL_COMMON.normalWidth + 200,
        lvContent: EnvironmentInfoComponent,
        lvHeader: this.treeSelection[0].name,
        lvComponentParams: {
          treeSelection: this.treeSelection[0]
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

  getResInfo() {
    // 接入HyperV
    this.resLabel = {
      [ResourceType.VM]: this.i18n.get('common_vmware_label'),
      [ResourceType.FUSIONSPHERE]: 'FusionSphere',
      [ResourceType.HYPERV]: this.i18n.get('common_hyperv_label')
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
      if (tab.id === ResourceType.CLUSTER) {
        tab.hidden = this.resType === ResourceType.HYPERV;
      }
    });
  }

  getTree() {
    this.pageSize = CommonConsts.PAGE_SIZE;
    this.pageIndex = CommonConsts.PAGE_START;
    this.getTreeData();
  }

  getTreeData(event?, startPage?) {
    if (startPage === undefined) {
      this.treeData = [];
      startPage = CommonConsts.PAGE_START;
    }
    this.environmentsService
      .queryResourcesV1EnvironmentsGet({
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        pageNo: startPage,
        conditions: JSON.stringify({
          type:
            this.resType === ResourceType.HYPERV
              ? ResourceType.HYPERV
              : ResourceType.VSPHERE
        })
      })
      .subscribe(res => {
        each(res.items, (item: any) => {
          const node = {
            label: item.name,
            name: item.name,
            authorized_user: item.authorized_user,
            contentToggleIcon: this.getResourceIcon(item),
            type: item.type as string,
            rootNodeSubType: item.sub_type,
            uuid: item.uuid,
            rootUuid: item.root_uuid,
            path: item.path,
            endpoint: item.endpoint,
            port: item.port,
            slaId: item.sla_id,
            userId: item.user_id,
            userName: item.user_name,
            link_status: item.link_status,
            rootNodeLinkStatus: includes(
              [
                DataMap.Resource_Type.vmwareVcenterServer.value,
                DataMap.Resource_Type.vmwareEsxi.value
              ],
              item.sub_type
            )
              ? item.link_status
              : DataMap.resource_LinkStatus.normal.value,
            children: [],
            isLeaf: false,
            expanded: this.getExpandedIndex(item.uuid) !== -1,
            resourceRoleAuth: item.resourceRoleAuth
          };
          if (this.resType === ResourceType.VM) {
            assign(node, {
              cert_name: item.cert_name
            });
          }
          if (node.expanded) {
            this.getExpandedChangeData(CommonConsts.PAGE_START, node);
          }

          if (!find(this.treeData, { uuid: node.uuid })) {
            this.treeData.push(node);
          }
        });
        startPage++;
        if (res.total - startPage * CommonConsts.PAGE_SIZE_MAX > 0) {
          this.getTreeData(null, startPage);
          return;
        }
        this.treeData = [...this.treeData];
        if (this.treeData.length) {
          if (!this.treeSelection.length) {
            this.treeSelection = [this.treeData[0]];
            this.nodeCheck({ node: this.treeData[0] });
          } else {
            this.treeData.forEach(node => {
              if (node.uuid === this.treeSelection[0].uuid) {
                this.treeSelection = [node];
              }
            });
          }
          if (
            [
              DataMap.Resource_Type.vmwareEsx.value,
              DataMap.Resource_Type.vmwareEsxi.value
            ].includes(this.treeSelection[0].rootNodeSubType)
          ) {
            this.tabs[0].hidden = true;
            this.tabs[0].total = 0;
            this.tabs[0].resourceTotal = 0;
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
          'vmware-tree'
        );
        this.globalService.emitStore({
          action: 'emitRefreshApp',
          state: true
        });
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
    this.virtualResourceService
      .queryResourcesV1VirtualResourceGet({
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        pageNo: startPage,
        conditions: JSON.stringify({
          parent_uuid: event.uuid
        })
      })
      .subscribe(res => {
        res.items.forEach((item: any) => {
          if (item.type !== ResourceType.VM) {
            const rootNode = this.getRootNode(event),
              node = {
                label: item.name,
                contentToggleIcon: this.getResourceIcon(item),
                type: item.type as string,
                sub_type: item.sub_type,
                rootNodeSubType: event.rootNodeSubType,
                rootUuid: item.root_uuid,
                uuid: item.uuid,
                path: item.path,
                children: [],
                rootNodeLinkStatus: includes(
                  [
                    DataMap.Resource_Type.vmwareVcenterServer.value,
                    DataMap.Resource_Type.vmwareEsxi.value
                  ],
                  rootNode.rootNodeSubType
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
        if (res.total - startPage * CommonConsts.PAGE_SIZE_MAX > 0) {
          this.getExpandedChangeData(startPage, event);
          return;
        }
        // 如果没有值+去掉
        if (isEmpty(event.children)) {
          event.children = null;
          event.isLeaf = true;
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
      item => item.value === node.sub_type
    );
    if (
      nodeResource?.value === DataMap.Resource_Type.vmwareVcenterServer.value &&
      trim(node.link_status) ===
        DataMap.resource_LinkStatus_Special.offline.value
    ) {
      return 'aui-icon-vCenter-offine';
    }
    return nodeResource['icon'] + '';
  }

  nodeCheck(e) {
    let tabIdList = [ResourceType.VM, ResourceType.HOST, ResourceType.CLUSTER];
    // 树节点的选中项为主机时，右侧只展示VMware，新增资源池和VAPP只展示虚拟机
    if (
      e.node.type === ResourceType.HOST ||
      includes(
        [
          DataMap.Resource_Type.virtualApp.value,
          DataMap.Resource_Type.resourcePool.value
        ],
        e.node.sub_type
      )
    ) {
      tabIdList = [ResourceType.VM];
      if (this.activeIndex !== ResourceType.VM) {
        this.pageSize = CommonConsts.PAGE_SIZE;
        this.pageIndex = CommonConsts.PAGE_START;
      }
    } else if (
      e.node.type === ResourceType.HYPERV ||
      e.node.type === ResourceType.CLUSTER ||
      [
        DataMap.Resource_Type.vmwareEsx.value,
        DataMap.Resource_Type.vmwareEsxi.value
      ].includes(e.node.rootNodeSubType)
    ) {
      tabIdList = [ResourceType.VM, ResourceType.HOST];
    }

    this.tabs.forEach(tab => {
      tab.hidden = !tabIdList.includes(tab.id);
    });
    this.activeIndex = find(this.tabs, item => !item.hidden)?.id;
    this.treeNodeClick = true;
    this.initMoreMenus();
    this.getTableData(null);
    this.showGuideTab();
  }

  beforeSelected = item => {
    if (this.treeSelection[0].uuid === item.uuid) {
      return false;
    }
  };

  register() {
    this.openRegisterModal();
  }

  deleteRegister() {
    this.protectedEnvironmentService
      .deleteEnvV1EnvironmentsEnvIdDelete({
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
    this.protectedEnvironmentService
      .rescanEnvV1EnvironmentsRescanEnvIdPut({
        envId: this.treeSelection[0].uuid
      })
      .subscribe(() => this.getTreeData());
  }

  modifyRegister() {
    this.openRegisterModal(true);
  }

  openRegisterModal(isModify = false) {
    const modalParam = {
      ...MODAL_COMMON.generateDrawerOptions(),
      lvType: 'drawer',
      lvWidth: this.i18n.isEn
        ? MODAL_COMMON.largeWidth
        : MODAL_COMMON.largeWidth - 100,
      lvHeader: this.i18n.get(
        `${isModify ? 'common_modify_register_label' : 'common_register_label'}`
      ),
      lvOkDisabled: true,
      lvCloseButtonDisplay: false,
      //  接入HyperV
      lvContent: {
        [ResourceType.VM]: RegisterVmComponent
      }[this.resType],
      lvComponentParams: {
        isModify,
        treeSelection: this.treeSelection,
        hostArr: this.tabs[1].tableData
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as RegisterVmComponent;
          (isModify ? content.modify() : content.create()).subscribe(
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

  getTableData(event?) {
    if (!this.treeSelection.length) {
      return;
    }

    let hasProtectionStatus = false;
    if (event) {
      each(event.source, (value, key) => {
        if (key === 'protection_status') {
          hasProtectionStatus = true;
        }
        if (isEmpty(value)) {
          delete event.source[key];
        }
      });
    }

    this.tabs.forEach(item => {
      if (item.sub as Subscription) {
        item.sub.unsubscribe();
      }
      if (item.hidden) {
        return;
      }
      if (item.id === this.activeIndex && hasProtectionStatus) {
        item.protectionStatus = '';
      }
      const conditions: any = {
        path: `${this.treeSelection[0].path}/`,
        type: item.id
      };
      if (
        includes(
          [
            DataMap.Resource_Type.resourcePool.value,
            DataMap.Resource_Type.virtualApp.value
          ],
          this.treeSelection[0]?.sub_type
        ) &&
        item.id === ResourceType.VM
      ) {
        assign(conditions, { parent_uuid: this.treeSelection[0]?.uuid });
      }
      let firstInitStatus = false;
      if (item.id === this.activeIndex && isNumber(item.protectionStatus)) {
        firstInitStatus = true;
        assign(conditions, {
          protection_status: [item.protectionStatus]
        });
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
            return this.virtualResourceService.queryResourcesV1VirtualResourceGet(
              {
                pageNo: this.pageIndex,
                pageSize: this.pageSize,
                orders: event && event.orders,
                conditions: JSON.stringify(conditions),
                akLoading: !index
              }
            );
          }),
          takeUntil(this.destroy$)
        )
        .subscribe(res => {
          item.tableData = map(res.items, res => {
            // 获取标签数据
            const { showList, hoverList } = getLabelList(res);
            assign(res, {
              showLabelList: showList,
              hoverLabelList: hoverList
            });
            res['tags'] = decodeURIComponent(res['tags'] || '');
            res['path'] = trimEnd(trim(res['path']), '/');
            return res;
          });
          item.total = res.total;
          if (
            includes(
              mapValues(this.drawModalService.modals, 'key'),
              'detail-modal'
            ) &&
            this.vmListComponent.currentDetailItemUuid &&
            find(res.items, {
              uuid: this.vmListComponent.currentDetailItemUuid
            })
          ) {
            this.globalService.emitStore({
              action: 'autoReshResource',
              state: find(res.items, {
                uuid: this.vmListComponent.currentDetailItemUuid
              })
            });
          }
          if (
            (event && event.tabType === item.id && !isEmpty(event.source)) ||
            firstInitStatus
          ) {
            this.virtualResourceService
              .queryResourcesV1VirtualResourceGet({
                pageNo: 0,
                pageSize: 1,
                conditions: JSON.stringify({
                  path: `${this.treeSelection[0].path}/`,
                  type: item.id
                }),
                akLoading: false
              })
              .subscribe(resource => {
                item.resourceTotal = resource.total;
                this.cdr.detectChanges();
              });
          } else {
            item.resourceTotal = res.total;
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
