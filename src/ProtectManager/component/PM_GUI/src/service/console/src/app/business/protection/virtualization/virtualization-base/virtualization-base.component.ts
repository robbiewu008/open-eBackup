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
import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import { MessageService } from '@iux/live';
import {
  CommonConsts,
  CookieService,
  DataMap,
  getPermissionMenuItem,
  GlobalService,
  hasResourcePermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  Page_Size_Options,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  RoleOperationMap,
  Table_Size,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  filter,
  find,
  first,
  includes,
  isEmpty,
  isNil,
  isNumber
} from 'lodash';
import { Subject, takeUntil } from 'rxjs';
import { EnvironmentInfoApsaraStackComponent } from '../../cloud/apsara-stack/environment-info-apsara-stack/environment-info-apsara-stack.component';
import { RegisterApsaraStackComponent } from '../../cloud/apsara-stack/register-apsara-stack/register-apsara-stack.component';
import { RegisterComponent as HyperVRegisterComponent } from '../hyper-v/register/register.component';
import { RegisterVmComponent } from '../vmware/register-vm/register-vm.component';
import { EnvironmentInfoComponent } from './environment-info/environment-info.component';

interface Tab {
  id: string;
  subType: string;
  subUnitType?: string;
  label: string;
  hidden: boolean;
  hiddenFn: (type?: string) => boolean;
  resourceTotal: number;
}

@Component({
  selector: 'aui-virtualization-base',
  templateUrl: './virtualization-base.component.html',
  styleUrls: ['./virtualization-base.component.less']
})
export class VirtualizationBaseComponent implements OnInit, OnDestroy {
  @Input() type: string;
  resourceType = ResourceType;

  header = '';
  tabs: Tab[] = [];
  activeIndex = '';
  treeData = [];
  treeSelection = [];
  expandedNodeList = [];

  optsConfig: ProButton[] = [];

  extParams = {};

  destroy$ = new Subject();

  constructor(
    private i18n: I18NService,
    public globalService: GlobalService,
    private cookieService: CookieService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.appInit();
    this.initConfig();
    this.getTreeData();
    this.showGuideTab();
    this.getUserGuideState();
  }

  getUserGuideState() {
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        this.showGuideTab();
      });
  }

  showGuideTab() {
    if (USER_GUIDE_CACHE_DATA.active && USER_GUIDE_CACHE_DATA.activeTab) {
      setTimeout(() => {
        this.activeIndex = <any>USER_GUIDE_CACHE_DATA.activeTab;
      });
    }
  }

  onChange() {
    this.treeSelection = [];
    this.treeData = [];
    this.expandedNodeList = [];
    this.ngOnInit();
  }

  appInit() {
    this.getHeader();
    this.getAllTabs();
  }

  getHeader() {
    switch (this.type) {
      default:
        break;
    }
  }

  getAllTabs() {
    switch (this.type) {
      case ResourceType.CNWARE:
        this.tabs = [
          {
            id: 'cluster',
            subType: DataMap.Resource_Type.cNwareCluster.value,
            label: this.i18n.get('common_clusters_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return includes(
                [
                  DataMap.Resource_Type.cNwareHost.value,
                  DataMap.Resource_Type.cNwareCluster.value
                ],
                node.subType
              );
            },
            resourceTotal: 0
          },
          {
            id: 'host',
            subType: DataMap.Resource_Type.cNwareHost.value,
            label: this.i18n.get('common_host_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return includes(
                [DataMap.Resource_Type.cNwareHost.value],
                node.subType
              );
            },
            resourceTotal: 0
          },
          {
            id: 'vm',
            subType: DataMap.Resource_Type.cNwareVm.value,
            label: this.i18n.get('common_virtual_machine_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return false;
            },
            resourceTotal: 0
          },
          {
            id: 'group',
            subType: DataMap.Resource_Type.vmGroup.value,
            subUnitType: DataMap.Resource_Type.cNwareVm.value,
            label: this.i18n.get('protection_vm_groups_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return !includes(
                [DataMap.Resource_Type.cNware.value],
                node.subType
              );
            },
            resourceTotal: 0
          }
        ];
        break;
      case ResourceType.ApsaraStack:
        this.tabs = [
          {
            id: 'Zone',
            subType: DataMap.Resource_Type.APSZone.value,
            label: this.i18n.get('protection_available_zone_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return includes(
                [DataMap.Resource_Type.APSZone.value],
                node.subType
              );
            },
            resourceTotal: 0
          },
          {
            id: 'resourceSet',
            subType: DataMap.Resource_Type.APSResourceSet.value,
            label: this.i18n.get('common_resource_set_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return !includes(
                [DataMap.Resource_Type.ApsaraStack.value],
                node.subType
              );
            },
            resourceTotal: 0
          },
          {
            id: 'serverInstance',
            subType: DataMap.Resource_Type.APSCloudServer.value,
            label: this.i18n.get('common_cloud_server_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return includes(
                [DataMap.Resource_Type.APSCloudServer.value],
                node.subType
              );
            },
            resourceTotal: 0
          },
          {
            id: 'group',
            subType: DataMap.Resource_Type.vmGroup.value,
            subUnitType: DataMap.Resource_Type.APSCloudServer.value,
            label: this.i18n.get('protection_cloud_group_label'),
            hidden: true,
            hiddenFn: (node: any) => {
              return !includes(
                [DataMap.Resource_Type.APSZone.value],
                node.subType
              );
            },
            resourceTotal: 0
          }
        ];
        break;
      case ResourceType.HYPERV:
        this.tabs = [
          {
            id: 'cluster',
            subType: DataMap.Resource_Type.hyperVCluster.value,
            label: this.i18n.get('common_cluster_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              // ScVMM 下面可能有集群 可能直接是主机，所以不需要隐藏cluster这一层
              return includes(
                [
                  DataMap.Resource_Type.hyperVCluster.value,
                  DataMap.Resource_Type.hyperVHost.value
                ],
                node?.subType
              );
            },
            resourceTotal: 0
          },
          {
            id: 'host',
            subType: DataMap.Resource_Type.hyperVHost.value,
            label: this.i18n.get('common_host_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return includes(
                [
                  DataMap.Resource_Type.hyperVVm.value,
                  DataMap.Resource_Type.hyperVHost.value
                ],
                node?.subType
              );
            },
            resourceTotal: 0
          },
          {
            id: 'vm',
            subType: DataMap.Resource_Type.hyperVVm.value,
            label: this.i18n.get('common_virtual_machine_label'),
            hidden: false,
            hiddenFn: () => {
              return false;
            },
            resourceTotal: 0
          },
          {
            id: 'group',
            subType: DataMap.Resource_Type.vmGroup.value,
            subUnitType: DataMap.Resource_Type.hyperVVm.value,
            label: this.i18n.get('protection_vm_groups_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return !!node?.parentUuid;
            },
            resourceTotal: 0
          }
        ];
        break;
      case ResourceType.NUTANIX:
        this.tabs = [
          {
            id: 'cluster',
            subType: DataMap.Resource_Type.nutanixCluster.value,
            label: this.i18n.get('common_clusters_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return includes(
                [
                  DataMap.Resource_Type.nutanixHost.value,
                  DataMap.Resource_Type.nutanixCluster.value
                ],
                node.subType
              );
            },
            resourceTotal: 0
          },
          {
            id: 'host',
            subType: DataMap.Resource_Type.nutanixHost.value,
            label: this.i18n.get('common_host_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return includes(
                [DataMap.Resource_Type.nutanixHost.value],
                node.subType
              );
            },
            resourceTotal: 0
          },
          {
            id: 'vm',
            subType: DataMap.Resource_Type.nutanixVm.value,
            label: this.i18n.get('common_virtual_machine_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return false;
            },
            resourceTotal: 0
          },
          {
            id: 'group',
            subType: DataMap.Resource_Type.vmGroup.value,
            subUnitType: DataMap.Resource_Type.nutanixVm.value,
            label: this.i18n.get('protection_vm_groups_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return !includes(
                [DataMap.Resource_Type.nutanix.value],
                node.subType
              );
            },
            resourceTotal: 0
          }
        ];
        break;
      default:
        break;
    }
    this.activeIndex = find(this.tabs, item => !item.hidden)?.id;
  }

  getRegisterModalExt(item) {
    switch (this.type) {
      case ResourceType.CNWARE:
      case ResourceType.NUTANIX:
        return {
          lvWidth: this.i18n.isEn
            ? MODAL_COMMON.normalWidth + 150
            : MODAL_COMMON.normalWidth + 100,
          lvContent: RegisterVmComponent,
          lvComponentParams: {
            isModify: !isEmpty(item),
            treeSelection: this.treeSelection,
            resourceType: this.type,
            item
          }
        };
      case ResourceType.ApsaraStack:
        return {
          lvContent: RegisterApsaraStackComponent,
          lvComponentParams: {
            isModify: !isEmpty(item),
            treeSelection: this.treeSelection,
            resourceType: ResourceType.ApsaraStack,
            item
          },
          lvAfterOpen: modal => {
            const content = modal.getContentComponent();
            content.formGroup.statusChanges.subscribe(status => {
              modal.getInstance().lvOkDisabled = status !== 'VALID';
            });
          }
        };
      case ResourceType.HYPERV:
        return {
          lvContent: HyperVRegisterComponent
        };
      default:
        return {};
    }
  }

  register(item?) {
    this.drawModalService.create(
      assign(
        {},
        MODAL_COMMON.generateDrawerOptions(),
        {
          lvModalKey: 'register-virtualization-base',
          lvWidth: MODAL_COMMON.normalWidth + 100,
          lvHeader: isEmpty(item)
            ? this.i18n.get('common_register_label')
            : this.i18n.get('common_modify_label'),
          lvOkDisabled: true,
          lvComponentParams: { item },
          lvOk: modal => {
            return new Promise(resolve => {
              const content = modal.getContentComponent();
              if (
                includes([ResourceType.CNWARE, ResourceType.NUTANIX], this.type)
              ) {
                (!isEmpty(item)
                  ? content.modify()
                  : content.create()
                ).subscribe(
                  () => {
                    resolve(true);
                    this.getTreeData();
                  },
                  () => resolve(false)
                );
              } else {
                content.onOK().subscribe(
                  () => {
                    resolve(true);
                    this.getTreeData();
                  },
                  () => resolve(false)
                );
              }
            });
          }
        },
        this.getRegisterModalExt(item)
      )
    );
  }

  getEnvironmentDetailComponent() {
    switch (this.type) {
      case ResourceType.ApsaraStack:
        return EnvironmentInfoApsaraStackComponent;
      default:
        return EnvironmentInfoComponent;
    }
  }

  getEnvironmentDetail(rowItem) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'environment-info-modal',
        lvWidth: MODAL_COMMON.normalWidth + 200,
        lvHeader: rowItem.name,
        lvContent: this.getEnvironmentDetailComponent(),
        lvComponentParams: {
          rowItem
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

  delete() {
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
      });
  }

  disableBtn(data) {
    return !includes(
      [
        ResourceType.CNWARE,
        ResourceType.NUTANIX,
        ResourceType.ApsaraStack,
        DataMap.Resource_Type.hyperVHost.value,
        DataMap.Resource_Type.hyperVScvmm.value,
        DataMap.Resource_Type.hyperVCluster.value
      ],
      data?.subType
    );
  }

  hiddenResourceScan(data) {
    return (
      includes(
        [
          DataMap.Resource_Type.hyperVHost.value,
          DataMap.Resource_Type.hyperVCluster.value,
          DataMap.Resource_Type.hyperVScvmm.value
        ],
        data?.subType
      ) && !isNil(data.parentUuid)
    );
  }

  initConfig() {
    const btns: ProButton[] = [
      {
        label: this.i18n.get('common_register_label'),
        permission: RoleOperationMap.manageResource,
        id: 'register',
        type: 'primary',
        onClick: () => this.register(),
        popoverContent: this.i18n.get('protection_register_resource_label'),
        popoverShow: USER_GUIDE_CACHE_DATA.active
      },
      {
        label: this.i18n.get('common_environment_info_label'),
        id: 'environmentInfo',
        permission: OperateItems.HCSEnvironmentInfo,
        divide: true,
        disableCheck: data => {
          return (
            !data.length || data[0].disableAuth || this.disableBtn(first(data))
          );
        },
        onClick: ([data]) => this.getEnvironmentDetail(data)
      },
      {
        id: 'rescan',
        disableCheck: data => {
          return (
            !data.length ||
            data[0].disableAuth ||
            this.disableBtn(first(data)) ||
            !hasResourcePermission(first(data)) ||
            this.hiddenResourceScan(data[0])
          );
        },
        permission: OperateItems.RescanVirtualizationPlatform,
        label: this.i18n.get('common_rescan_label'),
        onClick: () => {
          this.protectedResourceApiService
            .ScanProtectedResources({
              resId: this.treeSelection[0].uuid
            })
            .subscribe(() => this.getTreeData());
        }
      },
      {
        id: 'connectivityTest',
        disableCheck: data => {
          return (
            !data.length ||
            data[0].disableAuth ||
            this.disableBtn(first(data)) ||
            !hasResourcePermission(first(data))
          );
        },
        displayCheck: () => {
          return this.type !== ResourceType.CNWARE;
        },
        permission: OperateItems.RescanVirtualizationPlatform,
        divide: true,
        label: this.i18n.get('protection_connectivity_test_label'),
        onClick: () => {
          this.protectedResourceApiService
            .CheckProtectedResource({ resourceId: this.treeSelection[0].uuid })
            .subscribe(res => {
              let returnRes;
              try {
                returnRes = JSON.parse(res);
              } catch (error) {
                returnRes = [];
              }
              const idx = returnRes.findIndex(item => item.code !== 0);
              if (idx !== -1) {
                this.messageService.error(this.i18n.get(returnRes[idx].code), {
                  lvMessageKey: 'errorKey',
                  lvShowCloseButton: true
                });
              } else {
                this.messageService.success(
                  this.i18n.get('common_operate_success_label'),
                  {
                    lvMessageKey: 'successKey',
                    lvShowCloseButton: true
                  }
                );
              }
            });
        }
      },
      {
        id: 'modify',
        disableCheck: data => {
          return (
            !data.length ||
            data[0].disableAuth ||
            this.disableBtn(first(data)) ||
            !hasResourcePermission(first(data))
          );
        },
        permission: OperateItems.ModifyHCSTenant,
        label: this.i18n.get('common_modify_register_label'),
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
        disableCheck: data => {
          return (
            !data.length ||
            data[0].disableAuth ||
            this.disableBtn(first(data)) ||
            !hasResourcePermission(first(data))
          );
        },
        permission: OperateItems.UnRegisterHCSTenant,
        label: this.i18n.get('protection_unregister_label'),
        onClick: ([data]) => {
          this.warningMessageService.create({
            content: this.i18n.get('protection_hcs_resource_unregister_label', [
              data.name,
              this.i18n.get('protection_canncel_huawei_stack_project_label')
            ]),
            onOK: () => this.delete()
          });
        }
      }
    ];
    this.optsConfig = getPermissionMenuItem(btns, this.cookieService.role);
    assign(this.extParams, {
      extOpts: getPermissionMenuItem(btns, this.cookieService.role)
    });
    this.extParams = { ...this.extParams };
  }

  getConditions() {
    switch (this.type) {
      case ResourceType.CNWARE:
      case ResourceType.NUTANIX:
        return {
          subType: this.type,
          type: this.type
        };
      case ResourceType.ApsaraStack:
        return {
          subType: DataMap.Resource_Type.ApsaraStack.value,
          type: ResourceType.ApsaraStack
        };
      case ResourceType.HYPERV:
        return {
          type: ResourceType.Virtualization,
          subType: [
            DataMap.Resource_Type.hyperVScvmm.value,
            DataMap.Resource_Type.hyperVCluster.value,
            DataMap.Resource_Type.hyperVHost.value
          ]
        };
      default:
        return {
          subType: this.type
        };
    }
  }

  getResourceIcon(node) {
    switch (node.subType) {
      case ResourceType.CNWARE:
      case ResourceType.NUTANIX:
        return node.linkStatus ===
          DataMap.resource_LinkStatus_Special.normal.value
          ? 'aui-icon-vCenter'
          : 'aui-icon-vCenter-offine';
      case DataMap.Resource_Type.cNwareHostPool.value:
        return 'aui-icon-host-pool';
      case DataMap.Resource_Type.cNwareHostPool:
        return 'aui-icon-dataCenter';
      case DataMap.Resource_Type.cNwareCluster.value:
      case DataMap.Resource_Type.nutanixCluster.value:
        return 'aui-icon-cluster';
      case DataMap.Resource_Type.cNwareHost.value:
      case DataMap.Resource_Type.nutanixHost.value:
        return 'aui-icon-host';
      case DataMap.Resource_Type.APSRegion.value:
        return 'aui-icon-hcs-region';
      case DataMap.Resource_Type.APSZone.value:
        return 'aui-icon-hcs-project';
      case DataMap.Resource_Type.hyperVCluster.value:
      case DataMap.Resource_Type.hyperVScvmm.value:
        return node.linkStatus ===
          DataMap.resource_LinkStatus_Special.normal.value
          ? 'aui-cluster-online-16'
          : 'aui-cluster-offline-16';
      case DataMap.Resource_Type.hyperVHost.value:
        return node.linkStatus ===
          DataMap.resource_LinkStatus_Special.normal.value
          ? 'aui-host-online-16'
          : 'aui-host-offline-16';
      default:
        return 'aui-icon-host';
    }
  }

  isLeaf(node) {
    return includes(
      [
        DataMap.Resource_Type.cNwareHost.value,
        DataMap.Resource_Type.nutanixHost.value,
        DataMap.Resource_Type.APSZone.value,
        DataMap.Resource_Type.hyperVVm.value,
        DataMap.Resource_Type.hyperVHost.value
      ],
      node.subType
    );
  }

  getExpandedIndex(id) {
    return this.expandedNodeList.findIndex(node => node.uuid === id);
  }

  getRootNode(node) {
    if (!!node.parent) {
      return this.getRootNode(node.parent);
    } else {
      return node;
    }
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

  getTreeData(recordsTemp?: any[], startPage?: number) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: 200,
        conditions: JSON.stringify(this.getConditions())
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
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          if (isEmpty(recordsTemp)) {
            // 如果没有查询到环境
            this.treeData = [];
            return;
          }
          each(recordsTemp, node => {
            assign(node, {
              label: node.name,
              contentToggleIcon: this.getResourceIcon(node),
              userName: node['username'],
              link_status: node['linkStatus'],
              children: [],
              isLeaf: this.isLeaf(node),
              expanded: this.getExpandedIndex(node.uuid) !== -1
            });
            if (node.expanded) {
              this.getExpandedChangeData(CommonConsts.PAGE_START, node);
            }
          });
          if (this.type === ResourceType.NUTANIX) {
            recordsTemp = recordsTemp.filter(
              item =>
                !includes([DataMap.Resource_Type.nutanixVm.value], item.subType)
            );
          }
          this.treeData = recordsTemp;
          if (this.treeData.length) {
            if (!this.treeSelection.length) {
              this.treeSelection = [this.treeData[0]];
              this.nodeCheck({ node: this.treeData[0] }); // 初次选中时，lv-tree还未初始化，所以这里手动触发一次nodeCheck，去除不要的tab
            } else {
              this.treeData.forEach(node => {
                if (node.uuid === this.treeSelection[0].uuid) {
                  this.treeSelection = [node];
                }
              });
            }
          }
          this.virtualScroll.getScrollParam(
            260,
            Page_Size_Options.Three,
            Table_Size.Default,
            'cnware-tree'
          );
          return;
        }
        this.getTreeData(recordsTemp, startPage);
      });
  }

  getExpandedChangeData(startPage, event) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10
    };
    const extParmas = {
      parentUuid: event.uuid
    };
    assign(params, {
      conditions: JSON.stringify(extParmas)
    });
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (this.type === ResourceType.ApsaraStack) {
        res.records = filter(res.records, item => {
          return item.subType !== DataMap.Resource_Type.APSResourceSet.value;
        });
      }
      res.records.forEach(item => {
        const node = {
          ...item,
          label: item.name,
          contentToggleIcon: this.getResourceIcon(item),
          type: item.type as string,
          subType: item.subType,
          rootNodeSubType: event.rootNodeType,
          rootUuid: item.rootUuid,
          uuid: item.uuid,
          path: item.path,
          children: [],
          isLeaf: this.isLeaf(item),
          expanded: this.getExpandedIndex(item.uuid) !== -1
        };
        if (node.expanded) {
          this.getExpandedChangeData(CommonConsts.PAGE_START, node);
        }
        if (!includes([DataMap.Resource_Type.nutanixVm.value], node.subType)) {
          event.children.push(node);
        }
      });
      startPage++;
      if (res.totalCount - startPage * CommonConsts.PAGE_SIZE * 10 > 0) {
        this.getExpandedChangeData(startPage, event);
        return;
      }
      this.treeData = [...this.treeData];
    });
  }

  updateProjectTotal(event, tab: Tab) {
    tab.resourceTotal = event?.total;
  }

  beforeSelected = item => {
    if (this.treeSelection[0]?.uuid === item.uuid) {
      return false;
    }
  };

  nodeCheck(e) {
    each(this.tabs, tab => {
      tab.hidden = tab.hiddenFn(e.node);
    });
    this.tabs = [...this.tabs];
    this.activeIndex = find(this.tabs, item => !item.hidden)?.id;
  }
}
