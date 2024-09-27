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
import { Component, OnInit } from '@angular/core';
import { MessageService } from '@iux/live';
import {
  CommonConsts,
  CookieService,
  DataMap,
  getPermissionMenuItem,
  getTableOptsItems,
  hasResourcePermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  RoleOperationMap,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  includes,
  isEmpty,
  isNumber,
  mapValues
} from 'lodash';
import { EnvironmentInfoComponent } from './environment-info/environment-info.component';
import { RegisterOpenstackComponent } from './register-openstack/register-openstack.component';

@Component({
  selector: 'aui-openstack',
  templateUrl: './openstack.component.html',
  styleUrls: ['./openstack.component.less']
})
export class OpenstackComponent implements OnInit {
  treeData = [];
  treeSelection = [];
  resourceType = ResourceType;
  dataMap = DataMap;
  expandedNodeList = [];

  optsConfig: ProButton[];

  activeIndex = 'domain';
  domainTotal = 0;
  projectTotal = 0;
  cloudHostTotal = 0;
  cloudGroupTotal = 0;
  currentUuid: string;

  constructor(
    private i18n: I18NService,
    private cookieService: CookieService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.getTreeData();
  }

  onChange() {
    this.treeSelection = [];
    this.treeData = [];
    this.expandedNodeList = [];
    this.ngOnInit();
  }

  initConfig() {
    const btns: ProButton[] = [
      {
        label: this.i18n.get('common_register_label'),
        permission: RoleOperationMap.manageResource,
        id: 'register',
        type: 'primary',
        onClick: () => this.register()
      },
      {
        label: this.i18n.get('common_environment_info_label'),
        id: 'environmentInfo',
        permission: OperateItems.HCSEnvironmentInfo,
        divide: true,
        disableCheck: data => {
          return (
            !data.length ||
            data[0].type !== ResourceType.OpenStack ||
            data[0].disableAuth
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
            !hasResourcePermission(data[0])
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
            data[0].type !== ResourceType.OpenStack ||
            data[0].disableAuth ||
            !hasResourcePermission(data[0])
          );
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
            data[0].type !== ResourceType.OpenStack ||
            data[0].disableAuth ||
            !hasResourcePermission(data[0])
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
            data[0].type !== ResourceType.OpenStack ||
            data[0].disableAuth ||
            !hasResourcePermission(data[0])
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
  }

  register(item?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'register-huawei-cloud-stack',
        lvWidth: MODAL_COMMON.normalWidth + 200,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterOpenstackComponent,
        lvOkDisabled: true,
        lvComponentParams: { item },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterOpenstackComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.getTreeData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  getEnvironmentDetail(rowItem) {
    this.currentUuid = rowItem?.uuid;
    const optItems = filter(cloneDeep(this.optsConfig), item =>
      includes(['rescan', 'connectivityTest', 'modify', 'delete'], item.id)
    );
    const modalParams = assign({}, MODAL_COMMON.generateDrawerOptions(), {
      lvModalKey: 'environment-info-modal',
      lvWidth: MODAL_COMMON.normalWidth + 200,
      lvHeader: rowItem.name,
      lvContent: EnvironmentInfoComponent,
      lvComponentParams: {
        rowItem: {
          ...rowItem,
          optItems: getTableOptsItems(optItems, rowItem, this),
          optItemsFn: v => {
            return getTableOptsItems(optItems, v, this);
          }
        }
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ]
    });
    if (
      includes(
        mapValues(this.drawModalService.modals, 'key'),
        'environment-info-modal'
      )
    ) {
      this.drawModalService.update('environment-info-modal', modalParams);
    } else {
      this.drawModalService.create(modalParams);
    }
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

  updateDomainTotal(event) {
    this.domainTotal = event?.total;
  }

  updateProjectTotal(event) {
    this.projectTotal = event?.total;
  }

  updateHostTotal(event) {
    this.cloudHostTotal = event?.total;
  }

  updateGroupTotal(event) {
    this.cloudGroupTotal = event?.total;
  }

  getResourceIcon(node) {
    return node.type === ResourceType.OpenStack
      ? 'aui-icon-hcs-platform'
      : node.type === ResourceType.StackDomain
      ? 'aui-icon-hcs-region'
      : 'aui-icon-hcs-project';
  }

  beforeSelected = item => {
    if (this.treeSelection[0]?.uuid === item.uuid) {
      return false;
    }
  };

  nodeCheck(e) {
    if (e.node.type === ResourceType.OpenStack) {
      this.activeIndex = 'domain';
    } else if (e.node.type === ResourceType.StackDomain) {
      this.activeIndex = 'project';
    } else {
      this.activeIndex = 'cloudHost';
    }
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

  getExpandedChangeData(startPage, event) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10
    };
    const extParmas = {
      parentUuid: event.uuid
    };
    if (event.subType === ResourceType.OPENSTACK_CONTAINER) {
      assign(extParmas, {
        visible: ['1']
      });
    }
    assign(params, {
      conditions: JSON.stringify(extParmas)
    });

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      res.records.forEach((item: any) => {
        const rootNode = this.getRootNode(event),
          node = {
            label: item.name,
            contentToggleIcon: this.getResourceIcon(item),
            type: item.type as string,
            rootNodeSubType: event.rootNodeType,
            rootUuid: item.rootUuid,
            uuid: item.uuid,
            path: item.path,
            children: [],
            rootNodeLinkStatus: DataMap.resource_LinkStatus.normal.value,
            isLeaf: item.type === ResourceType.StackProject,
            expanded: this.getExpandedIndex(item.uuid) !== -1,
            resourceRoleAuth: item.resourceRoleAuth
          };
        if (node.expanded) {
          this.getExpandedChangeData(CommonConsts.PAGE_START, node);
        }
        event.children.push(node);
      });
      startPage++;
      if (res.totalCount - startPage * CommonConsts.PAGE_SIZE * 10 > 0) {
        this.getExpandedChangeData(startPage, event);
        return;
      }
      this.treeData = [...this.treeData];
    });
  }

  getProject(recordsTemp?: any[], startPage?: number) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: 200,
        conditions: JSON.stringify({
          subType: [ResourceType.OpenStackProject]
        })
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
          each(recordsTemp, node => {
            assign(node, {
              label: node.name,
              contentToggleIcon: this.getResourceIcon(node),
              userName: node['username'],
              link_status: node['linkStatus'],
              children: [],
              isLeaf: true
            });
          });
          this.createFakeTreeData(recordsTemp);
          this.virtualScroll.getScrollParam(280);
          return;
        }
        this.getProject(recordsTemp, startPage);
      });
  }

  createFakeEnvironment(project) {
    return {
      label: project.parentName,
      contentToggleIcon: 'aui-icon-hcs-platform',
      type: ResourceType.OpenStack,
      subType: ResourceType.OPENSTACK_CONTAINER,
      rootUuid: project.parentUuid,
      uuid: project.parentUuid,
      path: project.path?.replace(`/${project.name}`, ''),
      children: [],
      isLeaf: false,
      expanded: this.getExpandedIndex(project.parentUuid) !== -1,
      disableAuth: true
    };
  }

  createFakeTreeData(projects) {
    const fakeTreeData = [];
    each(projects, item => {
      if (find(fakeTreeData, { uuid: item.parentUuid })) {
        find(fakeTreeData, { uuid: item.parentUuid }).children?.push(item);
      } else {
        const environment = this.createFakeEnvironment(item);
        environment.children?.push(item);
        fakeTreeData.push(environment);
      }
    });
    this.treeData = fakeTreeData;
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
    }
  }

  getTreeData(recordsTemp?: any[], startPage?: number) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: 200,
        conditions: JSON.stringify({
          type: ResourceType.OpenStack,
          subType: ResourceType.OPENSTACK_CONTAINER
        })
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
            // 如果没有查询到环境，再查询项目
            this.getProject();
            return;
          }
          this.virtualScroll.getScrollParam(280);
          each(recordsTemp, node => {
            assign(node, {
              label: node.name,
              contentToggleIcon: this.getResourceIcon(node),
              userName: node['username'],
              link_status: node['linkStatus'],
              children: [],
              isLeaf: false,
              expanded: this.getExpandedIndex(node.uuid) !== -1
            });
            if (node.expanded) {
              this.getExpandedChangeData(CommonConsts.PAGE_START, node);
            }
          });
          this.treeData = recordsTemp;
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
            // 如果详情框打开了，需要刷新详情弹窗
            if (
              includes(
                mapValues(this.drawModalService.modals, 'key'),
                'environment-info-modal'
              ) &&
              find(recordsTemp, { uuid: this.currentUuid })
            ) {
              this.getEnvironmentDetail(
                find(recordsTemp, { uuid: this.currentUuid })
              );
            }
          }
          return;
        }
        this.getTreeData(recordsTemp, startPage);
      });
  }
}
