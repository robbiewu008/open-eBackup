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
  Component,
  EventEmitter,
  Input,
  OnChanges,
  OnDestroy,
  OnInit,
  Output,
  SimpleChanges
} from '@angular/core';
import {
  ApplicationType,
  CommonConsts,
  DataMap,
  GlobalService,
  I18NService,
  ProtectedResourceApiService,
  ResourceSetApiService,
  ResourceSetType,
  ResourceType,
  VirtualResourceService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  defer,
  differenceBy,
  each,
  filter,
  find,
  get,
  includes,
  isEmpty,
  isNumber,
  isUndefined,
  map,
  set,
  size,
  some
} from 'lodash';
import { forkJoin, Observable, Subscription } from 'rxjs';

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
  selector: 'aui-virtual-cloud-template',
  templateUrl: './virtual-cloud-template.component.html',
  styleUrls: ['./virtual-cloud-template.component.less']
})
export class VirtualCloudTemplateComponent
  implements OnInit, OnDestroy, AfterViewInit, OnChanges {
  @Input() resourceType;
  @Input() subName;
  @Input() allSelectionMap;
  @Input() appType;
  @Input() data;
  @Input() isDetail;
  @Input() allSelect;
  @Output() allSelectChange = new EventEmitter<any>();
  @Output() resourceGroupChange = new EventEmitter<any>();

  header = '';
  tabs: Tab[] = [];
  activeIndex = '';
  treeData = [];
  treeSelection = [];
  treeOldSelection = []; // 用来对比然后从map里去除取消的选中项
  treeNodeChecked;
  expandedNodeList = [];
  showSelect = false; // 回显子组件数据

  isVmware = false; // Vmware有部分单独处理
  isFc = false;

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

  dataFetch$: Subscription = new Subscription();
  syncData$: Subscription = new Subscription();

  constructor(
    public globalService: GlobalService,
    public virtualScroll: VirtualScrollService,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private resourceSetService: ResourceSetApiService,
    private virtualResourceService: VirtualResourceService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.isVmware = this.resourceType === ApplicationType.Vmware;
    this.isFc = this.appType === ResourceSetType.FusionCompute;
    this.getAllTabs();
  }

  ngOnDestroy(): void {
    this.dataFetch$.unsubscribe();
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.allSelect?.currentValue) {
      set(this.allSelectionMap, this.appType, {
        isAllSelected: true
      });
      this.treeSelection = cloneDeep(this.treeData);
      this.treeSelectionChange();
    }
    if (
      !changes.allSelect?.currentValue &&
      this.appType in this.allSelectionMap
    ) {
      set(this.allSelectionMap, this.appType, {
        isAllSelected: false
      });
      this.treeSelection = [];
      this.treeSelectionChange();
      set(this.allSelectionMap, this.appType, { data: [] });
    }
  }

  ngAfterViewInit() {
    this.getFetchState();
  }

  getFetchState() {
    this.dataFetch$ = this.globalService
      .getState(this.subName)
      .subscribe(res => {
        this.getTreeData();
      });

    this.syncData$ = this.globalService
      .getState(`${this.appType}syncData`)
      .subscribe(res => {
        // 右表里取消的，这里也需要取消
        this.treeSelection = this.treeSelection.filter(
          item =>
            !isEmpty(this.allSelectionMap[this.appType]?.data) &&
            find(this.allSelectionMap[this.appType].data, { uuid: item.uuid })
        );
        if (!isEmpty(this.allSelectionMap[this.appType]?.data)) {
          this.syncSelectData();
        }
      });
  }

  syncSelectData(data?) {
    // 同步右侧表格选中后，左树展示的选项也要选中
    const tmpData = data ?? this.treeData;

    // 右表里选中的，这里也需要选中
    each(tmpData, item => {
      if (
        some(this.allSelectionMap[this.appType].data, { uuid: item.uuid }) &&
        !some(this.treeSelection, { uuid: item.uuid })
      ) {
        this.treeSelection.push(item);
        this.treeSelection = [...this.treeSelection];
        this.treeOldSelection = cloneDeep(this.treeSelection);
      }

      if (item?.children && item.children.length) {
        this.syncSelectData(item.children);
      }
    });
  }

  onChange() {
    this.treeSelection = [];
    this.treeNodeChecked = null;
    this.treeData = [];
    this.expandedNodeList = [];
    this.ngOnInit();
  }

  groupChange(e) {
    this.resourceGroupChange.emit(e);
  }

  selectChange() {
    this.allSelectChange.emit();
  }

  getAllTabs() {
    switch (this.resourceType) {
      case ApplicationType.OpenStack:
        this.tabs = [
          {
            id: ResourceType.OpenStackDomain,
            label: this.i18n.get('protection_openstack_domain_label'),
            resourceTotal: 0,
            hidden: false,
            hiddenFn: (node: any) => {
              return !includes(
                [DataMap.Job_Target_Type.Openstack.value],
                node.subType
              );
            },
            subType: ResourceType.OpenStackDomain
          },
          {
            id: ResourceType.OpenStackProject,
            label: this.i18n.get('common_project_label'),
            resourceTotal: 0,
            hidden: false,
            hiddenFn: (node: any) => {
              return includes(
                [DataMap.Resource_Type.openStackProject.value],
                node.subType
              );
            },
            subType: DataMap.Resource_Type.openStackProject.value
          },
          {
            id: ResourceType.OpenStackCloudServer,
            label: this.i18n.get('common_cloud_server_label'),
            resourceTotal: 0,
            hidden: false,
            hiddenFn: (node: any) => {
              return false;
            },
            subType: DataMap.Resource_Type.openStackCloudServer.value
          },
          {
            id: 'group',
            subType: DataMap.Resource_Type.vmGroup.value,
            subUnitType: DataMap.Resource_Type.openStackCloudServer.value,
            label: this.i18n.get('protection_cloud_group_label'),
            hidden: true,
            hiddenFn: (node: any) => {
              return !includes(
                [DataMap.Resource_Type.openStackProject.value],
                node.subType
              );
            },
            resourceTotal: 0
          }
        ];
        break;
      case ApplicationType.HCSCloudHost:
        this.tabs = [
          {
            id: ResourceType.TENANT,
            label: this.i18n.get('common_tenant_else_label'),
            resourceTotal: 0,
            hidden: false,
            hiddenFn: (node: any) => {
              return !includes(
                [DataMap.Resource_Type.HCSContainer.value],
                node.subType
              );
            },
            subType: DataMap.Resource_Type.HCSTenant.value
          },
          {
            id: ResourceType.PROJECT,
            label: this.i18n.get('common_project_resource_label'),
            resourceTotal: 0,
            hidden: false,
            hiddenFn: (node: any) => {
              return includes(
                [DataMap.Resource_Type.HCSProject.value],
                node.subType
              );
            },
            subType: DataMap.Resource_Type.HCSProject.value
          },
          {
            id: ResourceType.CLOUD_HOST,
            label: this.i18n.get('common_cloud_server_label'),
            resourceTotal: 0,
            hidden: false,
            hiddenFn: (node: any) => {
              return false;
            },
            subType: DataMap.Resource_Type.HCSCloudHost.value
          },
          {
            id: 'group',
            subType: DataMap.Resource_Type.vmGroup.value,
            subUnitType: DataMap.Resource_Type.HCSCloudHost.value,
            label: this.i18n.get('protection_cloud_group_label'),
            hidden: true,
            hiddenFn: (node: any) => {
              return !includes(
                [DataMap.Resource_Type.HCSProject.value],
                node.subType
              );
            },
            resourceTotal: 0
          }
        ];
        break;
      case ApplicationType.FusionCompute:
      case ApplicationType.FusionOne:
        this.tabs = [
          {
            id: ResourceType.CLUSTER,
            label: this.i18n.get('common_clusters_label'),
            resourceTotal: 0,
            subType: DataMap.Resource_Type.fusionComputeCluster.value,
            hidden: false,
            hiddenFn: (node: any) => {
              return !includes([ResourceType.PLATFORM], node.type);
            }
          },
          {
            id: ResourceType.HOST,
            label: this.i18n.get('common_host_label'),
            resourceTotal: 0,
            subType: DataMap.Job_Target_Type.FusionComputeHost.value,
            hidden: false,
            hiddenFn: (node: any) => {
              return !includes(
                [ResourceType.PLATFORM, ResourceType.CLUSTER],
                node.type
              );
            }
          },
          {
            id: ResourceType.VM,
            label: this.i18n.get('common_virtual_machine_label'),
            resourceTotal: 0,
            subType: DataMap.Resource_Type.fusionComputeVirtualMachine.value,
            hidden: false,
            hiddenFn: (node: any) => {
              return false;
            }
          },
          {
            id: 'group',
            subType: DataMap.Resource_Type.vmGroup.value,
            subUnitType:
              this.resourceType === ApplicationType.FusionCompute
                ? ResourceType.FUSION_COMPUTE
                : ResourceType.FUSION_ONE,
            label: this.i18n.get('protection_vm_groups_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return !includes([ResourceType.PLATFORM], node.type);
            },
            resourceTotal: 0
          }
        ];
        break;
      case ApplicationType.Vmware:
        this.tabs = [
          {
            id: ResourceType.CLUSTER,
            label: this.i18n.get('common_clusters_label'),
            resourceTotal: 0,
            subType: DataMap.Resource_Type.clusterComputeResource.value,
            hidden: false,
            hiddenFn: (node: any) => {
              return (
                includes(
                  [
                    DataMap.Resource_Type.virtualApp.value,
                    DataMap.Resource_Type.resourcePool.value,
                    DataMap.Resource_Type.vmwareEsx.value,
                    DataMap.Resource_Type.vmwareEsxi.value
                  ],
                  node.subType
                ) ||
                includes([ResourceType.HOST, ResourceType.CLUSTER], node.type)
              );
            }
          },
          {
            id: ResourceType.HOST,
            label: this.i18n.get('common_host_label'),
            resourceTotal: 0,
            subType: DataMap.Resource_Type.hostSystem.value,
            hidden: false,
            hiddenFn: (node: any) => {
              return (
                includes(
                  [
                    DataMap.Resource_Type.virtualApp.value,
                    DataMap.Resource_Type.resourcePool.value
                  ],
                  node.subType
                ) || includes([ResourceType.HOST], node.type)
              );
            }
          },
          {
            id: ResourceType.VM,
            label: this.i18n.get('common_virtual_machine_label'),
            resourceTotal: 0,
            subType: DataMap.Resource_Type.virtualMachine.value,
            hidden: false,
            hiddenFn: (node: any) => {
              return false;
            }
          },
          {
            id: 'group',
            subType: DataMap.Resource_Type.vmGroup.value,
            subUnitType: DataMap.Resource_Type.virtualMachine.value,
            label: this.i18n.get('protection_vm_groups_label'),
            hidden: false,
            hiddenFn: (node: any) => {
              return !includes(
                [DataMap.Resource_Type.vmwareVcenterServer.value],
                node?.subType
              );
            },
            resourceTotal: 0
          }
        ];
        break;
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
            hiddenFn: (node: any) => {
              return (
                node.subType === DataMap.Resource_Type.hyperVCluster.value &&
                !!node?.parentUuid
              );
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
      default:
        break;
    }
    this.activeIndex = find(this.tabs, item => !item.hidden)?.id;
  }

  getConditions() {
    switch (this.resourceType) {
      case ResourceType.CNWARE:
      case ResourceType.NUTANIX:
        return {
          subType: this.resourceType,
          type: this.resourceType
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
      case ApplicationType.Vmware:
        return {
          type: ResourceType.VSPHERE
        };
      case ApplicationType.FusionCompute:
        return {
          subType: ResourceType.FUSION_COMPUTE,
          type: ResourceType.PLATFORM
        };
      case ApplicationType.FusionOne:
        return {
          subType: ResourceType.FUSION_ONE,
          type: ResourceType.PLATFORM
        };
      case ApplicationType.HCSCloudHost:
        return {
          subType: ResourceType.HCS_CONTAINER,
          type: ResourceType.HCS
        };
      case ApplicationType.OpenStack:
        return {
          type: ResourceType.OpenStack,
          subType: ResourceType.OPENSTACK_CONTAINER
        };
      default:
        return {
          subType: this.resourceType
        };
    }
  }

  getTreeData(recordsTemp?: any[], startPage?: number) {
    const defaultParams = {
      ...this.getConditions()
    };
    if (this.isDetail) {
      assign(defaultParams, {
        resourceSetId: this.data[0].uuid
      });
    }
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: 200,
        conditions: JSON.stringify(defaultParams)
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
              rootNodeSubType: node.sub_type,
              width: 170,
              expanded: this.getExpandedIndex(node.uuid) !== -1
            });
            if (node.expanded) {
              this.getExpandedChangeData(CommonConsts.PAGE_START, node);
            }
          });
          if (this.resourceType === ResourceType.NUTANIX) {
            recordsTemp = recordsTemp.filter(
              item =>
                !includes([DataMap.Resource_Type.nutanixVm.value], item.subType)
            );
          }
          this.treeData = recordsTemp;
          if (this.treeData.length) {
            if (!this.treeNodeChecked) {
              this.treeNodeChecked = this.treeData[0];
              this.treeNodeCheck(this.treeNodeChecked);
              this.nodeCheck({ node: this.treeData[0] }); // 初次选中时，lv-tree还未初始化，所以这里手动触发一次nodeCheck，去除不要的tab
            } else {
              this.treeData.forEach(node => {
                if (node.uuid === this.treeNodeChecked.uuid) {
                  this.treeNodeChecked = node;
                }
              });
            }
            this.clearTreeNodeCheck(this.treeData);
            this.treeData = [...this.treeData];
          }
          if (
            !!this.data &&
            isEmpty(this.allSelectionMap[this.appType]?.data) &&
            !this.isDetail
          ) {
            // 只有在修改场景时第一次进入组件会获取一次
            this.getAllSelectedData();
            this.getResourceGroupSelectedData();
          }
          if (
            [
              DataMap.Resource_Type.vmwareEsx.value,
              DataMap.Resource_Type.vmwareEsxi.value
            ].includes(this.treeNodeChecked.subType)
          ) {
            this.tabs[0].hidden = true;
            this.tabs[0].resourceTotal = 0;
            this.activeIndex = this.tabs[1].id;
          }
          return;
        }
        this.getTreeData(recordsTemp, startPage);
      });
  }

  getAllSelectedData() {
    forkJoin([
      this.getSelectedData(),
      this.getResourceGroupSelectedData()
    ]).subscribe(response => {
      const [resourceData, resourceGroupData] = response;
      set(this.allSelectionMap, this.appType, {
        data: map(resourceData, item => {
          return { uuid: item };
        })
      });
      set(this.allSelectionMap, ResourceSetType.RESOURCE_GROUP, {
        data: map(resourceGroupData, item => {
          return { uuid: item, scopeType: this.appType };
        })
      });
      this.showSelect = true;
      if (!!size(resourceData)) {
        this.syncModifySelectedData();
      }
      this.allSelectChange.emit();
    });
  }

  getSelectedData() {
    const params: any = {
      resourceSetId: this.data[0].uuid,
      scopeModule: this.appType,
      type: 'RESOURCE'
    };
    return this.resourceSetService.queryResourceObjectIdList(params);
  }

  getResourceGroupSelectedData() {
    const params: any = {
      resourceSetId: this.data[0].uuid,
      scopeModule: this.appType,
      type: ResourceSetType.RESOURCE_GROUP
    };
    return this.resourceSetService.queryResourceObjectIdList(params);
  }

  syncModifySelectedData(data?) {
    // 用来同步修改时左树的选中
    const tmpData = data ?? this.treeData;
    each(tmpData, item => {
      if (
        some(this.allSelectionMap[this.appType].data, { uuid: item.uuid }) &&
        !some(this.treeSelection, { uuid: item.uuid })
      ) {
        this.treeSelection.push(item);
      }
      if (item?.children && !!item.children.length) {
        this.syncModifySelectedData(item.children);
      }
    });
    this.treeSelection = [...this.treeSelection];
    this.treeOldSelection = cloneDeep(this.treeSelection);
  }

  getResourceIcon(node) {
    if (this.isVmware) {
      const nodeResource = find(DataMap.Resource_Type, {
        value: node.sub_type || node.subType
      });
      return nodeResource['icon'] + '';
    }
    if (this.isFc || this.appType === ResourceSetType.FusionOne) {
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
        return includes(
          [
            DataMap.hypervHostStatus.Up.value,
            DataMap.hypervHostStatus.Ok.value
          ],
          node.extendInfo.status
        ) ||
          node.linkStatus === DataMap.resource_LinkStatus_Special.normal.value // 单主机有linkStatus，主机在集群下走extendInfo
          ? 'aui-host-online-16'
          : 'aui-host-offline-16';
      case DataMap.Resource_Type.HCSRegion.value:
      case ResourceType.OpenStackDomain:
        return 'aui-icon-hcs-region';
      case DataMap.Resource_Type.HCSProject.value:
      case DataMap.Resource_Type.openStackProject.value:
        return 'aui-icon-hcs-project';
      case DataMap.Resource_Type.HCSTenant.value:
        return 'aui-icon-hcs-tenant';
      case DataMap.Resource_Type.HCSContainer.value:
      case DataMap.Job_Target_Type.Openstack.value:
        return 'aui-icon-hcs-platform';
      default:
        return 'aui-icon-host';
    }
  }

  updateProjectTotal(event, tab: Tab) {
    tab.resourceTotal = event?.total;
  }

  treeSelectionChange(e?) {
    if (this.treeSelection.length < this.treeOldSelection.length) {
      const diffData = differenceBy(
        this.treeOldSelection,
        this.treeSelection,
        'uuid'
      );
      if (!isEmpty(this.allSelectionMap[this.appType]?.data)) {
        this.allSelectionMap[this.appType].data = this.allSelectionMap[
          this.appType
        ].data.filter(item => item.uuid !== diffData[0].uuid);
      }
    }
    this.treeOldSelection = cloneDeep(this.treeSelection);

    each(this.treeSelection, item => {
      if (isEmpty(this.allSelectionMap[this.appType]?.data)) {
        if (isUndefined(this.allSelectionMap[this.appType])) {
          set(this.allSelectionMap, this.appType, {});
        }
        assign(this.allSelectionMap[this.appType], {
          data: [item]
        });
      } else if (
        !some(this.allSelectionMap[this.appType]?.data, { uuid: item.uuid })
      ) {
        this.allSelectionMap[this.appType].data.push(item);
      }
    });

    // 触发父子关联选择
    defer(() => {
      this.globalService.emitStore({
        action: `${this.appType}parentSelect`,
        state: true
      });
    });
    this.parentSelect();
    this.globalService.emitStore({
      action: `${this.appType}syncData`,
      state: true
    });
    this.allSelectChange.emit();
  }

  parentSelect(data?) {
    // 用于判断左树的父子选择关系
    const tmpData = data ?? this.treeData;
    each(tmpData, item => {
      if (
        (!isEmpty(this.allSelectionMap[this.appType]?.data) &&
          find(
            this.allSelectionMap[this.appType].data,
            val =>
              val.uuid === item?.parent_uuid ||
              val.uuid === item?.environment_uuid ||
              val.uuid === item?.parentUuid ||
              val.uuid === item?.environment?.uuid
          )) ||
        !!this.allSelectionMap[this.appType]?.isAllSelected
      ) {
        item.disabled = true;
        if (!some(this.treeSelection, { uuid: item.uuid })) {
          this.treeSelection.push(item);
        }
        if (
          !some(this.allSelectionMap[this.appType].data, { uuid: item.uuid })
        ) {
          this.allSelectionMap[this.appType].data.push(item);
        }
      } else {
        item.disabled = false;
      }
      if (item?.children && !!item.children.length) {
        this.parentSelect(item.children);
      }
    });
    this.treeSelection = [...this.treeSelection];
    this.treeOldSelection = cloneDeep(this.treeSelection);
    this.treeData = [...this.treeData];
  }

  nodeCheck(e) {
    each(this.tabs, tab => {
      tab.hidden = tab.hiddenFn(e.node);
    });
    this.tabs = [...this.tabs];
    this.activeIndex = find(this.tabs, item => !item.hidden)?.id;
  }

  treeNodeCheck(item) {
    if (this.treeNodeChecked.uuid === item.uuid) {
      return;
    }
    item.checked = true;
    this.treeNodeChecked = item;
    each(this.tabs, tab => {
      tab.hidden = tab.hiddenFn(item);
    });
    this.tabs = [...this.tabs];
    this.activeIndex = find(this.tabs, item => !item.hidden)?.id;
    this.clearTreeNodeCheck(this.treeData);
  }

  clearTreeNodeCheck(data) {
    each(data, val => {
      val.checked = val.uuid === this.treeNodeChecked.uuid;
      if (val?.children.length) {
        this.clearTreeNodeCheck(val.children);
      }
    });
  }

  getExpandedIndex(id) {
    return this.expandedNodeList.findIndex(node => node.uuid === id);
  }

  isLeaf(node) {
    return (
      includes(
        [
          DataMap.Resource_Type.cNwareHost.value,
          DataMap.Resource_Type.nutanixHost.value,
          DataMap.Resource_Type.APSZone.value,
          DataMap.Resource_Type.HCSProject.value,
          DataMap.Resource_Type.openStackProject.value,
          DataMap.Resource_Type.hyperVVm.value,
          DataMap.Resource_Type.hyperVHost.value
        ],
        node.subType
      ) ||
      node.type === ResourceType.VM ||
      includes([DataMap.Resource_Type.hostSystem.value], node.sub_type)
    );
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

  getExpandedChangeData(startPage, event) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10
    };

    const extParmas = {
      parentUuid: event.uuid
    };

    if (this.isVmware) {
      delete extParmas.parentUuid;
      set(extParmas, 'parent_uuid', event.uuid);
    }

    if (this.appType === ResourceSetType.HCSStack) {
      assign(extParmas, {
        subType: get(
          this.queryTreeMap,
          event.subType,
          DataMap.Resource_Type.HCSContainer.value
        ),
        visible: event.subType === ResourceType.HCS_CONTAINER ? '1' : null
      });
    }

    if (
      this.appType === ResourceSetType.OpenStack &&
      event.subType === ResourceType.OPENSTACK_CONTAINER
    ) {
      assign(extParmas, {
        visible: ['1']
      });
    }

    if (this.isDetail) {
      if (this.isVmware) {
        assign(extParmas, {
          resource_set_id: this.data[0].uuid
        });
      } else {
        assign(extParmas, {
          resourceSetId: this.data[0].uuid
        });
      }
    }

    assign(params, {
      conditions: JSON.stringify(extParmas)
    });

    const queryFunc: Observable<any> = this.isVmware
      ? this.virtualResourceService.queryResourcesV1VirtualResourceGet(params)
      : this.protectedResourceApiService.ListResources(params);
    queryFunc.subscribe(res => {
      if (this.isVmware) {
        assign(res, {
          records: res.items,
          total: res.totalCount
        });
      }
      if (this.resourceType === ResourceType.ApsaraStack) {
        res.records = filter(res.records, item => {
          return item.subType !== DataMap.Resource_Type.APSResourceSet.value;
        });
      }
      res.records.forEach(item => {
        if (
          item.subType === DataMap.Resource_Type.hyperVCluster.value &&
          item.linkStatus === null &&
          event.subType === DataMap.Resource_Type.hyperVScvmm.value
        ) {
          // SCVMM下面的集群没有linkstatus，集群使用SCVMM的状态
          item.linkStatus = event.linkStatus;
        }
        if (item.type !== ResourceType.VM) {
          const isParentSelected =
            !isEmpty(this.allSelectionMap[this.appType]?.data) &&
            !!find(
              this.allSelectionMap[this.appType].data,
              val =>
                val.uuid === item?.parent_uuid ||
                val.uuid === item?.environment_uuid ||
                val.uuid === item?.parentUuid ||
                val.uuid === item?.environment?.uuid
            ) &&
            !this.isDetail;
          const node = {
            ...item,
            label: item.name,
            contentToggleIcon: this.getResourceIcon(item),
            type: item.type as string,
            subType: item.subType || item.sub_type,
            rootNodeSubType: event.rootNodeType,
            rootUuid: item.rootUuid,
            uuid: item.uuid,
            path: item.path,
            children: [],
            isLeaf: this.isLeaf(item),
            expanded: this.getExpandedIndex(item.uuid) !== -1
          };
          if (node.uuid === this.treeNodeChecked.uuid) {
            node.checked = true;
          }
          if (
            isParentSelected &&
            !some(this.treeSelection, { uuid: item.uuid })
          ) {
            this.treeSelection.push(item);
          }
          if (node.expanded) {
            this.getExpandedChangeData(CommonConsts.PAGE_START, node);
          }
          if (
            !includes([DataMap.Resource_Type.nutanixVm.value], node.subType)
          ) {
            event.children.push(node);
          }
        }
      });
      if (!!this.data && !this.isDetail) {
        this.syncModifySelectedData();
      }
      if (!this.isDetail) {
        this.treeSelectionChange();
      }

      startPage++;
      if (res.totalCount - startPage * CommonConsts.PAGE_SIZE * 10 > 0) {
        this.getExpandedChangeData(startPage, event);
        return;
      }
      this.treeData = [...this.treeData];
    });
  }
}
