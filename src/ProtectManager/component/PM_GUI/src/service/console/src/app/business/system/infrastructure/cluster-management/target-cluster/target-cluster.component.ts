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
  OnInit
} from '@angular/core';
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
  GROUP_COMMON,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  RoleType,
  WarningMessageService
} from 'app/shared';
import {
  ApiMultiClustersService,
  ClustersApiService
} from 'app/shared/api/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  extend,
  find,
  get,
  includes,
  isEmpty,
  reject,
  set,
  size
} from 'lodash';
import { combineLatest } from 'rxjs';
import { AddTargetClusterComponent } from '../add-target-cluster/add-target-cluster.component';
import { AsManagementClusterComponent } from '../as-management-cluster/as-management-cluster.component';
import { AuthUserComponent } from '../auth-user/auth-user.component';
import { CancleAuthUserComponent } from '../cancle-auth-user/cancle-auth-user.component';
import { ClusterDetailComponent } from '../cluster-detail/cluster-detail.component';

@Component({
  selector: 'aui-target-cluster',
  templateUrl: './target-cluster.component.html',
  styleUrls: ['./target-cluster.component.less'],
  providers: [CapacityCalculateLabel]
})
export class TargetClusterComponent implements OnInit, OnDestroy {
  @Input() componentData;
  clusterData: any[];
  clusterSelection = [];
  disabledDelete = true;
  CLUSTER_TYPE = this.dataMapService.getConfig('Cluster_Type');
  CLUSTER_ROLE = this.dataMapService.getConfig('Target_Cluster_Role');
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE / 2;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  unitconst = CAPACITY_UNIT;
  dataMap = DataMap;
  theadFilterMap = {};
  clusterSearchName;
  ipSearchName;
  progressBarColor = [[0, '#6C92FA']];

  selectQosLabel = this.i18n.get('system_select_qos_label');

  closeLabel = this.i18n.get('common_close_label');

  localClusterLabel = this.i18n.get('system_local_cluster_label');
  targetClusterLabel = this.i18n.get('common_target_cluster_label');
  typeFilters = this.dataMapService.toArray('Cluster_Type');
  roleFilters = this.dataMapService.toArray('Target_Cluster_Role').slice(3, 4);
  roleList = [this.dataMap.Target_Cluster_Role.replication.value];
  typeList = [];
  statusFilters = this.dataMapService.toArray('Cluster_Status');

  currentHostName = location.hostname;
  _includes = includes;
  localCluster;
  _size = size;
  autoPolling;

  hasClusterDetailPermission = false;

  groupCommon = GROUP_COMMON;

  constructor(
    public i18n: I18NService,
    public drawmodalservice: DrawModalService,
    public clusterApiService: ClustersApiService,
    public dataMapService: DataMapService,
    public warningMessageService: WarningMessageService,
    public infoMessageService: InfoMessageService,
    private cookieService: CookieService,
    public virtualScroll?: VirtualScrollService,
    private cdr?: ChangeDetectorRef,
    private capacityCalculateLabel?: CapacityCalculateLabel,
    private multiClustersServiceApi?: ApiMultiClustersService
  ) {}

  ngOnInit(): void {
    this.getPermission();
    this.getRolelist();
    this.getCluster();

    this.autoPolling = setInterval(() => {
      this.getCluster(false);
    }, CommonConsts.TIME_INTERVAL * 3);
  }

  ngOnDestroy(): void {
    clearInterval(this.autoPolling);
  }

  onChange() {
    this.ngOnInit();
  }

  getRolelist() {
    if (this.componentData) {
      this.roleFilters = this.dataMapService
        .toArray('Target_Cluster_Role')
        .slice(4, 6);
      this.roleList = [
        this.dataMap.Target_Cluster_Role.managed.value,
        this.dataMap.Target_Cluster_Role.management.value
      ];
      this.typeList.push(2);
    }
  }

  localClusterDetail(data) {
    if (!this.hasClusterDetailPermission) {
      return;
    }
    this.drawmodalservice.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'localClusterDetail',
        lvHeader: data.clusterName,
        lvWidth: 784,
        lvContent: ClusterDetailComponent,
        lvOkDisabled: false,
        lvComponentParams: {
          drawData: {
            ...data,
            ipArr: data.ipArr.sort((a, b) => {
              return -1;
            })
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

  targetOptsCallback = data => {
    const menus = [
      {
        id: 'modifyCluster',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyingTargetClusterAuth,
        onClick: (d: any) => {
          this.drawmodalservice.create(
            assign({}, MODAL_COMMON.drawerOptions, {
              lvModalKey: 'modifyTargetCluster',
              lvHeader: this.i18n.get('common_modify_colon_label', [
                data.clusterName
              ]),
              lvWidth: MODAL_COMMON.normalWidth,
              lvContent: AddTargetClusterComponent,
              lvOkDisabled: false,
              lvAfterOpen: modal => {
                const content = modal.getContentComponent() as AddTargetClusterComponent;
                const modalIns = modal.getInstance();
                content.formGroup.statusChanges.subscribe(res => {
                  modalIns.lvOkDisabled = res !== 'VALID';
                });
              },
              lvComponentParams: {
                drawData: data,
                addType: this.componentData ? this.componentData : 0
              },
              lvOk: modal => {
                const content = modal.getContentComponent() as AddTargetClusterComponent;
                content.modifyCluster().subscribe(() => this.getCluster());
              }
            })
          );
        }
      },
      {
        id: 'deleteCluster',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeletingTargetCluster,
        onClick: (d: any) => {
          this.warningMessageService.create({
            content:
              data.role === DataMap.Target_Cluster_Role.backupStorage.value
                ? data.status === DataMap.Cluster_Status.offline.value
                  ? this.i18n.get('system_delete_off_target_cluster_label', [
                      data.clusterName
                    ])
                  : this.i18n.get('system_delete_backup_cluster_label', [
                      data.clusterName
                    ])
                : data.status === DataMap.Cluster_Status.offline.value
                ? this.i18n.get('system_delete_off_target_cluster_label', [
                    data.clusterName
                  ])
                : this.i18n.get('system_delete_target_cluster_label', [
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
                  this.getCluster();
                  this.clusterSelection = reject(
                    this.clusterSelection,
                    cluster => {
                      return cluster.clusterId === data.clusterId;
                    }
                  );
                });
            }
          });
        },
        divide:
          this.cookieService.role === RoleType.SysAdmin &&
          !includes(
            [DataMap.Target_Cluster_Role.replication.value, 0],
            data.role
          )
      },
      {
        id: 'managedCluster',
        label: this.i18n.get('system_specifies_management_cluster_label'),
        permission: OperateItems.ManagedTargetCluster,
        hidden:
          includes(
            [
              DataMap.Target_Cluster_Role.replication.value,
              DataMap.Target_Cluster_Role.backupStorage.value,
              0
            ],
            data.role
          ) || this.cookieService.role !== RoleType.SysAdmin,
        disabled:
          data.enableManage ||
          data.status !== DataMap.Cluster_Status.online.value ||
          data.role === DataMap.Target_Cluster_Role.backupStorage.value,
        onClick: (d: any) => this.asManagementCluster(data)
      },
      {
        id: 'unManagedCluster',
        label: this.i18n.get('system_cluster_role_unmanaged_label'),
        permission: OperateItems.UnManagedTargetCluster,
        hidden:
          includes(
            [
              DataMap.Target_Cluster_Role.replication.value,
              DataMap.Target_Cluster_Role.backupStorage.value,
              0
            ],
            data.role
          ) || this.cookieService.role !== RoleType.SysAdmin,
        disabled:
          !data.enableManage ||
          data.status !== DataMap.Cluster_Status.online.value ||
          data.role === DataMap.Target_Cluster_Role.backupStorage.value,
        onClick: (d: any) => {
          this.warningMessageService.create({
            content: this.i18n.get('system_cluster_role_unmanaged_desc_label', [
              data.clusterName
            ]),
            onOK: () => {
              this.multiClustersServiceApi
                .revokeManager({ clusterId: data.clusterId })
                .subscribe(res => this.getCluster());
            }
          });
        },
        divide:
          !data.enableManage &&
          data.role === DataMap.Target_Cluster_Role.managed.value
      },
      {
        id: 'auth',
        label: this.i18n.get('system_user_auth_label'),
        permission: OperateItems.ManagedTargetCluster,
        hidden:
          includes(
            [
              DataMap.Target_Cluster_Role.replication.value,
              DataMap.Target_Cluster_Role.backupStorage.value,
              0
            ],
            data.role
          ) || this.cookieService.role !== RoleType.SysAdmin,
        disabled:
          data.status !== DataMap.Cluster_Status.online.value ||
          data.role === DataMap.Target_Cluster_Role.backupStorage.value,
        onClick: (d: any) => this.authUser(data)
      },
      {
        id: 'modifyAuth',
        label: this.i18n.get('system_modify_auth_label'),
        permission: OperateItems.ManagedTargetCluster,
        hidden:
          includes(
            [
              DataMap.Target_Cluster_Role.replication.value,
              DataMap.Target_Cluster_Role.backupStorage.value,
              0
            ],
            data.role
          ) || this.cookieService.role !== RoleType.SysAdmin,
        disabled:
          !size(data.authUserList) ||
          data.status !== DataMap.Cluster_Status.online.value ||
          data.role === DataMap.Target_Cluster_Role.backupStorage.value,
        onClick: (d: any) => this.modifyAuth(data)
      },
      {
        id: 'cancleAuth',
        label: this.i18n.get('system_cancle_auth_label'),
        permission: OperateItems.ManagedTargetCluster,
        hidden:
          includes(
            [
              DataMap.Target_Cluster_Role.replication.value,
              DataMap.Target_Cluster_Role.backupStorage.value,
              0
            ],
            data.role
          ) || this.cookieService.role !== RoleType.SysAdmin,
        disabled:
          !size(data.authUserList) ||
          data.status !== DataMap.Cluster_Status.online.value ||
          data.role === DataMap.Target_Cluster_Role.backupStorage.value,
        onClick: (d: any) => this.cancleAuthUser(data)
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  asManagementCluster(data) {
    this.drawmodalservice.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvType: 'modal',
        lvOkDisabled: true,
        lvModalKey: 'asManagementCluster',
        lvHeader: this.i18n.get('system_specifies_management_cluster_label'),
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeight: MODAL_COMMON.smallWidth,
        lvContent: AsManagementClusterComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AsManagementClusterComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvComponentParams: {
          data
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as AsManagementClusterComponent;
          content.onOK().subscribe(() => this.getCluster());
        }
      })
    );
  }

  authorizedUser(data) {
    if (!size(data.authUserList)) {
      return;
    }
    this.drawmodalservice.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvOkDisabled: true,
        lvModalKey: 'authorizedUserModal',
        lvHeader: this.i18n.get('system_authorized_user_label'),
        lvWidth: MODAL_COMMON.normalWidth,
        lvContent: CancleAuthUserComponent,
        lvComponentParams: {
          rowItem: assign({}, data, { isView: true })
        },
        lvFooter: [
          {
            label: this.closeLabel,
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  authUser(data) {
    this.drawmodalservice.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvOkDisabled: true,
        lvModalKey: 'authUserModal',
        lvHeader: data.isModify
          ? this.i18n.get('system_modify_auth_label')
          : this.i18n.get('system_user_auth_label'),
        lvWidth: MODAL_COMMON.normalWidth,
        lvContent: AuthUserComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AuthUserComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest(
            content.formGroup.statusChanges,
            content.userValid$
          );
          combined.subscribe(latestValues => {
            const [formGroupStatus, valid] = latestValues;
            modalIns.lvOkDisabled = !valid || formGroupStatus !== 'VALID';
          });
        },
        lvComponentParams: {
          rowItem: data,
          localCluster: this.localCluster || {}
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as AuthUserComponent;
          return new Promise(resolve => {
            content.onOK().subscribe(
              res => {
                resolve(true);
                this.getCluster();
              },
              err => resolve(false)
            );
          });
        }
      })
    );
  }

  modifyAuth(data) {
    this.authUser(assign({}, data, { isModify: true }));
  }

  cancleAuthUser(data) {
    this.drawmodalservice.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvOkDisabled: true,
        lvModalKey: 'cancleAuthUserModal',
        lvHeader: this.i18n.get('system_cancle_auth_label'),
        lvWidth: MODAL_COMMON.normalWidth,
        lvContent: CancleAuthUserComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CancleAuthUserComponent;
          const modalIns = modal.getInstance();
          content.userValid$.subscribe(res => {
            modalIns.lvOkDisabled = !res;
          });
        },
        lvComponentParams: {
          rowItem: data,
          localCluster: this.localCluster || {}
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as CancleAuthUserComponent;
          return new Promise(resolve => {
            content.onOK().subscribe(
              res => {
                resolve(true);
                this.getCluster();
              },
              err => resolve(false)
            );
          });
        }
      })
    );
  }

  addTargetCluster(callback?: () => void) {
    this.drawmodalservice.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'addTargetCluster',
        lvHeader: this.componentData
          ? this.i18n.get('system_add_multi_domain_cluster_label')
          : this.i18n.get('system_add_replication_cluster_label'),
        lvWidth: MODAL_COMMON.normalWidth,
        lvContent: AddTargetClusterComponent,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddTargetClusterComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvComponentParams: {
          addType: this.componentData ? this.componentData : 0
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddTargetClusterComponent;
            if (callback) {
              content.registerCluster().subscribe(
                () => {
                  resolve(true);
                  callback();
                },
                () => {
                  resolve(false);
                }
              );
            } else {
              content.registerCluster().subscribe(
                () => {
                  resolve(true);
                  this.getCluster();
                },
                () => {
                  resolve(false);
                }
              );
            }
          });
        }
      })
    );
  }

  deleteTargetCluster() {
    const promises = [];
    const names = [];
    this.clusterSelection.forEach(item => {
      names.push(item.clusterName);
    });
    let lableKey = 'system_delete_target_cluster_label';
    if (
      find(this.clusterSelection, {
        status: DataMap.Cluster_Status.offline.value
      })
    ) {
      lableKey = 'system_delete_off_target_cluster_label';
    }
    this.warningMessageService.create({
      content: this.i18n.get(lableKey, [names.join(',')]),
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
          this.getCluster();
          this.clusterSelection = [];
        });
      }
    });
  }

  getCluster(hasLoading?) {
    const filterParams = cloneDeep(this.theadFilterMap);
    const roleListFilter = get(filterParams, 'roleList', []);

    if (!!size(roleListFilter)) {
      set(
        filterParams,
        'roleList',
        includes(roleListFilter, DataMap.Target_Cluster_Role.replication.value)
          ? [...roleListFilter, 0]
          : roleListFilter
      );
    }
    const params = assign(
      {
        startPage: this.pageIndex,
        pageSize: this.pageSize,
        clusterName: this.clusterSearchName,
        clusterIp: this.ipSearchName,
        akLoading: hasLoading ?? true,
        roleList: this.roleList,
        typeList: this.typeList
      },
      this.theadFilterMap
    );
    if (!this.componentData) {
      delete params.typeList;
    }

    this.clusterApiService
      .getClustersInfoUsingGET(assign(params))
      .subscribe(res => {
        each(res.records, item => {
          if (item.clusterType === DataMap.Cluster_Type.local.value) {
            assign(item, {
              ipArr: item.ip
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
            if (item['clusterIp']) {
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
          }
        });
        if (!this.localCluster) {
          this.localCluster = find(res.records, {
            clusterType: DataMap.Cluster_Type.local.value
          });
        }
        this.clusterData = res.records;
        this.total = res.totalCount;
        this.cdr.detectChanges();
      });
  }

  getSizePercent(source): string {
    const sizePercent = parseFloat(
      (source.usedCapacity / source.capacity) * 100 + ''
    );
    return this.capacityCalculateLabel.formatDecimalPoint(sizePercent, 3);
  }

  refreshTargetCluster() {
    this.getCluster();
  }

  selectionChange(e) {
    this.disabledDelete = !size(this.clusterSelection);
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
    this.getCluster();
  }

  searchByName(name) {
    this.clusterSearchName = name;
    this.getCluster();
  }

  searchByIp(ip) {
    this.ipSearchName = ip;
    this.getCluster();
  }

  trackByUuid(index: number, list: any) {
    return list.clusterId;
  }

  clusterPageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getCluster();
  }

  getPermission() {
    const authMap = getAccessibleViewList(this.cookieService.role);
    this.hasClusterDetailPermission =
      authMap[OperateItems.QueryingLocalClusterDetails];
  }
}
