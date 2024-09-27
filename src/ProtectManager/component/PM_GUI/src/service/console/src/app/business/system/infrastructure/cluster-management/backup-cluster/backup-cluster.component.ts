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
  OnDestroy,
  OnInit,
  ViewChild
} from '@angular/core';
import { MenuItem } from '@iux/live';
import {
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  ColorConsts,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  getAccessibleViewList,
  getPermissionMenuItem,
  I18NService,
  LocalStorageApiService,
  MODAL_COMMON,
  MultiClusterStatus,
  OperateItems,
  WarningMessageService,
  GROUP_COMMON
} from 'app/shared';
import {
  BackupClustersApiService,
  BackupClustersHaApiService,
  BackupClustersNetplaneService,
  ClustersApiService
} from 'app/shared/api/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  extend,
  find,
  includes,
  isEmpty,
  isFunction,
  reject,
  size
} from 'lodash';
import { AddBackupNodeComponent } from '../add-backup-node/add-backup-node.component';
import { AddHaComponent } from '../add-ha/add-ha.component';
import { AddNetworkComponent } from '../add-network/add-network.component';
import { BackupNodeDetailComponent } from '../backup-node-detail/backup-node-detail.component';
import { DeleteHaComponent } from '../delete-ha/delete-ha.component';

@Component({
  selector: 'aui-backup-cluster',
  templateUrl: './backup-cluster.component.html',
  styleUrls: ['./backup-cluster.component.less'],
  providers: [CapacityCalculateLabel]
})
export class BackupClusterComponent implements OnInit, OnDestroy {
  clusterData: any[];
  clusterSelection = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE / 2;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  unitconst = CAPACITY_UNIT;
  dataMap = DataMap;
  theadFilterMap = {};
  clusterSearchName;
  ipSearchName;
  localCluster;
  autoPolling;
  storageInfo = {} as any;
  isHaDisabled = true;
  haMenus: MenuItem[];
  componentData = DataMap.Target_Cluster_Role.managed.value;
  headerData = {} as any;
  progressBarColor = [[0, '#6C92FA']];

  addMemberNodeLabel = this.i18n.get('common_create_label');
  primaryNodeLabel = this.i18n.get('system_backup_cluster_primary_node_label');
  memberNodeLabel = this.i18n.get('system_backup_cluster_member_node_label');
  standbyNodeLabel = this.i18n.get('system_backup_cluster_standby_node_label');
  closeLabel = this.i18n.get('common_close_label');

  CLUSTER_TYPE = this.dataMapService.getConfig('Cluster_Type');
  ROLE_TYPE = this.dataMapService.getConfig('Target_Cluster_Role');
  roleFilters = this.dataMapService.toArray('Target_Cluster_Role').slice(0, 3);
  roleList = [
    this.dataMap.Target_Cluster_Role.primaryNode.value,
    this.dataMap.Target_Cluster_Role.backupNode.value,
    this.dataMap.Target_Cluster_Role.memberNode.value
  ];
  statusFilters: any = this.dataMapService.toArray('Cluster_Status');

  hasClusterDetailPermission = false;
  modifyMemberNode = true;
  disabledDelete = true;
  hasMemberNode = false;

  isEn = this.i18n.language === 'zh-cn';
  _empty = isEmpty;

  groupCommon = GROUP_COMMON;

  constructor(
    public i18n: I18NService,
    public drawmodalservice: DrawModalService,
    public clusterApiService: ClustersApiService,
    public haApiService: BackupClustersHaApiService,
    public backupClustersApiService: BackupClustersApiService,
    public backupClusterNetplaneService: BackupClustersNetplaneService,
    public dataMapService: DataMapService,
    public warningMessageService: WarningMessageService,
    private cookieService: CookieService,
    private localStorageApiService: LocalStorageApiService,
    private infoMessageService: InfoMessageService,
    public virtualScroll?: VirtualScrollService,
    private capacityCalculateLabel?: CapacityCalculateLabel,
    private cdr?: ChangeDetectorRef
  ) {}

  ngOnInit(): void {
    this.getAbnormalNode();
    this.getPermission();
    this.getNode();
    this.getHaInfo();
    this.initfilter();

    this.autoPolling = setInterval(() => {
      this.getNode(false);
      this.getHaInfo(false);
    }, CommonConsts.TIME_INTERVAL * 3);

    this.virtualScroll.getScrollParam(220);
  }

  ngOnDestroy(): void {
    clearInterval(this.autoPolling);
  }

  getAbnormalNode() {
    if (isEmpty(MultiClusterStatus.nodeStatus)) return;

    this.statusFilters = each(this.statusFilters, val => {
      if (
        val.value === DataMap.Cluster_Status.offline.value ||
        val.value === DataMap.Cluster_Status.partOffline.value
      ) {
        val.selected = true;
      }
    });
    this.theadFilterMap = {
      statusList: [
        DataMap.Cluster_Status.offline.value,
        DataMap.Cluster_Status.partOffline.value
      ]
    };
    MultiClusterStatus.nodeStatus = [];
  }

  initfilter() {
    this.statusFilters.push({
      value: 26,
      key: 'setting',
      label: this.i18n.get('system_net_plane_setting_label'),
      color: ColorConsts.RUNNING
    });
    this.statusFilters.push({
      value: 29,
      key: 'deleting',
      label: this.i18n.get('common_status_deleting_label'),
      color: ColorConsts.RUNNING
    });
    this.statusFilters = this.statusFilters.filter(
      item =>
        item.value !== DataMap.Cluster_Status.unknown.value &&
        item.value !== DataMap.Cluster_Status.partOffline.value
    );
  }

  memberNodeDetail(data) {
    if (!this.hasClusterDetailPermission) {
      return;
    }
    this.drawmodalservice.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'memberNodeDetail',
        lvHeader: this.i18n.get('system_backup_node_detail_label'),
        lvWidth: 784,
        lvContent: BackupNodeDetailComponent,
        lvOkDisabled: false,
        lvComponentParams: {
          drawData: {
            ...data,
            ipArr:
              data.role === DataMap.Target_Cluster_Role.primaryNode.value
                ? data.ipArr
                : data.displayIp
                ? [data.displayIp, data.ip]
                : data.ip
          }
        },
        lvFooter: [
          {
            label: this.closeLabel,
            onClick: (modal, button) => {
              modal.close();
            }
          }
        ]
      })
    );
  }

  addMemberNode() {
    this.drawmodalservice.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'addBackupNode',
        lvHeader: this.addMemberNodeLabel,
        lvWidth: MODAL_COMMON.normalWidth,
        lvContent: AddBackupNodeComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          modifyMemberNode: false
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddBackupNodeComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            this.warningMessageService.create({
              content: this.i18n.get('system_add_backup_node_warning_label'),
              width: 700,
              onOK: () => {
                const content = modal.getContentComponent() as AddBackupNodeComponent;
                content.registerCluster().subscribe(
                  () => {
                    resolve(true);
                    this.getNode();
                  },
                  () => {
                    resolve(false);
                  }
                );
              },
              onCancel: () => resolve(false),
              lvAfterClose: result => {
                if (result && result.trigger === 'close') {
                  resolve(false);
                }
              }
            });
          });
        }
      })
    );
  }

  deleteMemberNode() {
    const promises = [];
    const names = [];
    this.clusterSelection.forEach(item => {
      names.push(item.clusterName);
    });
    this.warningMessageService.create({
      content: this.i18n.get('system_remove_backup_node_warning_label', [
        names.join(',')
      ]),
      onOK: () => {
        each(this.clusterSelection, item => {
          promises.push(
            new Promise((resolve, reject) => {
              const params = { clusterId: item.clusterId };
              if (item.status === DataMap.Cluster_Status.offline.value) {
                assign(params, {
                  forceDelete: true
                });
              }
              this.clusterApiService
                .deleteTargetClusterUsingDELETE(params)
                .subscribe(
                  res => {
                    resolve(res);
                  },
                  err => {
                    reject(err);
                  }
                );
            })
          );
        });
        Promise.all(promises).then(() => {
          this.hasMemberNode = false;
          this.getNode();
          this.clusterSelection = [];
        });
      }
    });
  }

  getNode(hasLoading?, callback?: () => void) {
    this.clusterApiService
      .getClustersInfoUsingGET(
        assign(
          {
            startPage: this.pageIndex,
            pageSize: this.pageSize,
            clusterName: this.clusterSearchName,
            clusterIp: this.ipSearchName,
            akLoading: hasLoading ?? true,
            roleList: this.roleList
          },
          this.theadFilterMap
        )
      )
      .subscribe(res => {
        each(res.records, item => {
          if (item.role === this.ROLE_TYPE.primaryNode.value) {
            assign(item, {
              ipArr: item['clusterIp']
                .replace(/\s/g, '')
                .replace('[', '')
                .replace(']', '')
                .split(',')
                .sort((a, b) => {
                  return -1;
                }),
              progressBarColor: [[0, ColorConsts.NORMAL]],
              sizePercent: this.getSizePercent(item)
            });
          } else {
            if (
              item.role === this.ROLE_TYPE.memberNode.value ||
              item.role === this.ROLE_TYPE.backupNode.value
            ) {
              this.hasMemberNode = true;
            }
            const ips = item['clusterIp']
              .replace(/\s/g, '')
              .replace('[', '')
              .replace(']', '')
              .split(',')
              .sort((a, b) => {
                return -1;
              });
            assign(item, {
              displayIps: item['clusterIp']
                .replace(/\s/g, '')
                .replace('[', '')
                .replace(']', ''),
              displayIp: ips[0],
              ipArr: reject(ips, (v, index) => {
                return !index;
              }).sort((a, b) => {
                return -1;
              }),
              progressBarColor: [[0, ColorConsts.NORMAL]],
              sizePercent: this.getSizePercent(item)
            });
          }
        });
        if (!this.localCluster) {
          this.localCluster = find(res.records, {
            clusterType: DataMap.Cluster_Type.local.value
          });
        }
        this.clusterData = res.records.sort((a, b) => b.role - a.role);
        this.total = res.totalCount;
        this.cdr.detectChanges();
      });
    this.backupClustersApiService
      .getBackupClustersSummary({ akLoading: false })
      .subscribe(res => {
        this.headerData = res;
      });
    isFunction(callback) && callback();
  }

  getSizePercent(source): string {
    const sizePercent = parseFloat(
      (source.usedCapacity / source.capacity) * 100 + ''
    );
    return this.capacityCalculateLabel.formatDecimalPoint(sizePercent, 3);
  }

  onChange() {
    this.ngOnInit();
  }

  refreshTargetCluster() {
    this.getNode();
  }

  selectionChange(e) {
    for (let item of this.clusterSelection) {
      if (
        item.role === this.dataMap.Target_Cluster_Role.backupNode.value ||
        (item.role === this.dataMap.Target_Cluster_Role.memberNode.value &&
          (item.status === 29 || item.status === 26))
      ) {
        this.disabledDelete = true;
        return;
      }
    }
    this.disabledDelete = !size(this.clusterSelection);
  }

  trackByUuid(index: number, list: any) {
    return list.clusterId;
  }

  filterChange(e) {
    extend(this.theadFilterMap, {
      [e.key]: e.value
    });
    each(this.theadFilterMap, (value, key) => {
      if (isEmpty(value)) {
        delete this.theadFilterMap[key];
      }
    });
    this.getNode();
  }

  searchByName(name) {
    this.clusterSearchName = name;
    this.getNode();
  }

  searchByIp(ip) {
    this.ipSearchName = ip;
    this.getNode();
  }

  getPermission() {
    const authMap = getAccessibleViewList(this.cookieService.role);
    this.hasClusterDetailPermission =
      authMap[OperateItems.QueryingLocalClusterDetails];
  }

  clusterPageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getNode();
  }

  isNetplaneStatusLoading(status): boolean {
    return [
      DataMap.netplaneStatus.setting.value,
      DataMap.netplaneStatus.modify.value,
      DataMap.netplaneStatus.delete.value
    ].includes(status);
  }

  isNetplaneStatusFailed(status): boolean {
    return [
      DataMap.netplaneStatus.settingFail.value,
      DataMap.netplaneStatus.modifyFailed.value,
      DataMap.netplaneStatus.deleteFailed.value
    ].includes(status);
  }

  BackupPrimaryOptsCallback = data => {
    const menus = [
      {
        id: 'add-network',
        label: this.i18n.get(
          'system_add_internal_communication_network_plane_label'
        ),
        permission: OperateItems.DeletingTargetCluster,
        hidden: !includes(
          [DataMap.netplaneStatus.settingFail.value, null],
          data.netPlaneSettingStatus
        ),
        onClick: (d: any) => {
          this.drawmodalservice.create(
            assign({}, MODAL_COMMON.drawerOptions, {
              lvModalKey: 'addNetwork',
              lvHeader: this.i18n.get(
                'system_add_internal_communication_network_plane_label'
              ),
              lvWidth: MODAL_COMMON.xLargeWidth - 50,
              lvContent: AddNetworkComponent,
              lvOkDisabled: true,
              lvComponentParams: {
                drawData: data,
                isModify: [
                  DataMap.netplaneStatus.settingCompleted.value,
                  DataMap.netplaneStatus.modifyFailed.value,
                  DataMap.netplaneStatus.deleteFailed.value
                ].includes(data.netPlaneSettingStatus)
              },
              lvOk: modal => {
                return new Promise(resolve => {
                  this.warningMessageService.create({
                    content: this.i18n.get(
                      'system_add_internal_network_plane_info_label'
                    ),
                    width: 700,
                    onOK: () => {
                      const content = modal.getContentComponent() as AddNetworkComponent;
                      content.onOK().subscribe(
                        () => {
                          resolve(true);
                          this.getNode();
                        },
                        () => {
                          resolve(false);
                        }
                      );
                    },
                    onCancel: () => resolve(false),
                    lvAfterClose: result => {
                      if (result && result.trigger === 'close') {
                        resolve(false);
                      }
                    }
                  });
                });
              }
            })
          );
        }
      },
      {
        id: 'modifyInternalCommmunicationNetworkPlane',
        label: this.i18n.get(
          'system_modify_internal_communication_network_plane_label'
        ),
        permission: OperateItems.DeletingTargetCluster,
        hidden:
          !includes(
            [
              DataMap.netplaneStatus.settingCompleted.value,
              DataMap.netplaneStatus.modifyFailed.value,
              DataMap.netplaneStatus.deleteFailed.value
            ],
            data.netPlaneSettingStatus
          ) ||
          !data.netPlaneSettingStatus ||
          this.hasMemberNode,
        onClick: (d: any) => {
          this.drawmodalservice.create(
            assign({}, MODAL_COMMON.drawerOptions, {
              lvModalKey: 'modifyInternalCommunicationNetworkPlane',
              lvHeader: this.i18n.get(
                'system_modify_internal_communication_network_plane_label'
              ),
              lvWidth: MODAL_COMMON.xLargeWidth - 50,
              lvContent: AddNetworkComponent,
              lvOkDisabled: true,
              lvComponentParams: {
                drawData: data,
                isModify: [
                  DataMap.netplaneStatus.settingCompleted.value,
                  DataMap.netplaneStatus.modifyFailed.value,
                  DataMap.netplaneStatus.deleteFailed.value
                ].includes(data.netPlaneSettingStatus)
              },
              lvOk: modal => {
                return new Promise(resolve => {
                  this.warningMessageService.create({
                    content: this.i18n.get(
                      'system_modify_internal_network_plane_info_label'
                    ),
                    width: 700,
                    onOK: () => {
                      const content = modal.getContentComponent() as AddNetworkComponent;
                      content.onOK().subscribe(
                        () => {
                          resolve(true);
                          this.getNode();
                        },
                        () => {
                          resolve(false);
                        }
                      );
                    },
                    onCancel: () => resolve(false),
                    lvAfterClose: result => {
                      if (result && result.trigger === 'close') {
                        resolve(false);
                      }
                    }
                  });
                });
              }
            })
          );
        }
      },
      {
        id: 'deleteInternalCommmunicationNetworkPlane',
        label: this.i18n.get(
          'system_delete_internal_communication_network_plane_label'
        ),
        permission: OperateItems.DeletingTargetCluster,
        hidden:
          !includes(
            [
              DataMap.netplaneStatus.settingCompleted.value,
              DataMap.netplaneStatus.modifyFailed.value,
              DataMap.netplaneStatus.deleteFailed.value
            ],
            data.netPlaneSettingStatus
          ) ||
          !data.netPlaneSettingStatus ||
          this.hasMemberNode,
        onClick: (d: any) => {
          this.infoMessageService.create({
            content: this.i18n.get(
              'system_delete_internal_network_plane_info_label'
            ),
            onOK: () => {
              this.backupClusterNetplaneService
                .deleteInternalNetPlaneRelationUsingDelete({})
                .subscribe(res => {
                  this.getNode();
                });
            }
          });
        }
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  BackupMemberOptsCallback = data => {
    const menus = [
      {
        id: 'modifyMemberNode',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyingTargetClusterAuth,
        hidden: data.status === 26 || data.status === 29,
        onClick: (d: any) => {
          this.drawmodalservice.create(
            assign({}, MODAL_COMMON.drawerOptions, {
              lvModalKey: 'modifyMemberNode',
              lvHeader: this.i18n.get('common_modify_label'),
              lvWidth: MODAL_COMMON.normalWidth,
              lvContent: AddBackupNodeComponent,
              lvOkDisabled: true,
              lvAfterOpen: modal => {
                const content = modal.getContentComponent() as AddBackupNodeComponent;
                const modalIns = modal.getInstance();
                content.formGroup.statusChanges.subscribe(res => {
                  modalIns.lvOkDisabled = res !== 'VALID';
                });
              },
              lvComponentParams: {
                drawData: data,
                modifyMemberNode: this.modifyMemberNode
              },
              lvOk: modal => {
                const content = modal.getContentComponent() as AddBackupNodeComponent;
                content.modifyCluster().subscribe(() => this.getNode());
              }
            })
          );
        }
      },
      {
        id: 'deleteMemberNode',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeletingTargetCluster,
        hidden:
          data.status === 26 ||
          data.status === 29 ||
          data.role === this.dataMap.Target_Cluster_Role.backupNode.value,
        onClick: (d: any) => {
          this.warningMessageService.create({
            content: this.i18n.get('system_remove_backup_node_warning_label', [
              data.clusterName
            ]),
            onOK: () => {
              const params = { clusterId: data.clusterId };
              if (data.status === DataMap.Cluster_Status.offline.value) {
                assign(params, {
                  forceDelete: true
                });
              }
              this.clusterApiService
                .deleteTargetClusterUsingDELETE(params)
                .subscribe(res => {
                  this.hasMemberNode = false;
                  this.getNode();
                  this.clusterSelection = reject(
                    this.clusterSelection,
                    cluster => {
                      return cluster.clusterId === data.clusterId;
                    }
                  );
                });
            }
          });
        }
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  getHaInfo(hasLoading?) {
    this.backupClustersApiService
      .queryClusterInfo(assign({ akLoading: hasLoading ?? true }))
      .subscribe(res => {
        if (res.roleType === 'PRIMARY') {
          this.queryHaInfo(false);
        } else {
          this.isHaDisabled = true;
        }
      });
  }

  queryHaInfo(hasLoading?) {
    this.haApiService
      .getHaConfig(assign({ akLoading: hasLoading ?? true }))
      .subscribe(
        (res: any) => {
          if (res.clusters.length) {
            this.isHaDisabled = false;
            const clusters = res.clusters;
            const standbyInfo = clusters.filter(
              item => item.backupRoleType === 'STANDBY'
            );
            const memberInfo = clusters.filter(
              item => item.backupRoleType === 'MEMBER'
            );
            if (standbyInfo.length) {
              this.getMenus('edit', res);
            } else {
              this.getMenus('add', res, memberInfo.length);
            }
          } else {
            this.isHaDisabled = true;
          }
          this.cdr.detectChanges();
        },
        () => {
          this.isHaDisabled = true;
          this.cdr.detectChanges();
        }
      );
  }

  getMenus(action, data, MemberFlag?) {
    let isForceDel = false;
    const standbyInfo = data.clusters.filter(
      v => v.backupRoleType === 'STANDBY'
    );
    if (standbyInfo.length) {
      isForceDel = standbyInfo[0].status === 27 ? false : true;
    }

    const menus = [
      {
        id: 'addHa',
        label: this.i18n.get('system_add_ha_label'),
        hidden: action === 'add' ? false : true,
        permission: OperateItems.AddHA,
        disabled: !MemberFlag,
        onClick: (d: any) => {
          this.drawmodalservice.create(
            assign({}, MODAL_COMMON.drawerOptions, {
              lvModalKey: 'addHA',
              lvHeader: this.i18n.get('system_add_ha_label'),
              lvWidth: 810,
              lvOkDisabled: true,
              lvContent: AddHaComponent,
              lvAfterOpen: modal => {
                const content = modal.getContentComponent() as AddHaComponent;
                const modalIns = modal.getInstance();
                content.addForm.statusChanges.subscribe(res => {
                  modalIns.lvOkDisabled = res !== 'VALID';
                });
                content.addForm.updateValueAndValidity();
              },
              lvComponentParams: {
                isEdit: false,
                drawData: data
              },
              lvOk: modal => {
                const content = modal.getContentComponent() as AddHaComponent;
                content.saveHaInfo().subscribe(() => this.getNode());
              }
            })
          );
        }
      },
      {
        id: 'editHa',
        label: this.i18n.get('system_edit_ha_label'),
        permission: OperateItems.ModifyHA,
        hidden: action === 'edit' ? false : true,
        onClick: (d: any) => {
          this.drawmodalservice.create(
            assign({}, MODAL_COMMON.drawerOptions, {
              lvModalKey: 'editHA',
              lvHeader: this.i18n.get('system_edit_ha_label'),
              lvWidth: 810,
              lvOkDisabled: true,
              lvContent: AddHaComponent,
              lvAfterOpen: modal => {
                const content = modal.getContentComponent() as AddHaComponent;
                const modalIns = modal.getInstance();
                content.addForm.statusChanges.subscribe(res => {
                  modalIns.lvOkDisabled = res !== 'VALID';
                });
                content.addForm.updateValueAndValidity();
              },
              lvOk: modal => {
                const content = modal.getContentComponent() as AddHaComponent;
                content.saveHaInfo().subscribe(() => this.getNode());
              },
              lvComponentParams: {
                isEdit: true,
                drawData: data
              }
            })
          );
        }
      },
      {
        id: 'deleteHa',
        label: this.i18n.get('system_delete_ha_label'),
        permission: OperateItems.DeleteHA,
        hidden: action === 'edit' ? false : true,
        onClick: (d: any) => {
          this.drawmodalservice.create({
            ...MODAL_COMMON.generateDrawerOptions(),
            lvModalKey: 'warningMessage',
            ...{
              lvType: 'dialog',
              lvDialogIcon: 'lv-icon-popup-danger-48',
              lvHeader: this.i18n.get('common_danger_label'),
              lvContent: DeleteHaComponent,
              lvComponentParams: {
                drawData: standbyInfo[0]
              },
              lvWidth: MODAL_COMMON.normalWidth,
              lvCloseButtonDisplay: true,
              lvFocusButtonId: 'cancelBtn',
              lvFooter: [
                {
                  label: isForceDel
                    ? this.i18n.get('system_forcibly_delete_label')
                    : this.i18n.get('common_ok_label'),
                  onClick: modal => {
                    const content = modal.getContentComponent() as DeleteHaComponent;
                    content.deleteHaInfo(isForceDel).subscribe(() => {
                      this.getNode();
                      modal.close();
                    });
                  }
                },
                {
                  id: 'cancelBtn',
                  label: this.i18n.get('common_cancel_label'),
                  type: 'primary',
                  onClick: modal => {
                    modal.close();
                  }
                }
              ]
            }
          });
        }
      }
    ];
    this.haMenus = getPermissionMenuItem(menus, this.cookieService.role);
  }
}
