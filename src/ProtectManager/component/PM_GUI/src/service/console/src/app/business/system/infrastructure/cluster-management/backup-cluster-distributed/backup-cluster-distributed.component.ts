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
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  BackupClustersApiService,
  CAPACITY_UNIT,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  getAccessibleViewList,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  RoleType
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  filter as _filter,
  find,
  first,
  isFunction,
  map,
  reject,
  size,
  toString,
  toUpper
} from 'lodash';
import { BackupNodeDetailDistributedComponent } from '../backup-node-detail-distributed/backup-node-detail-distributed.component';
import { BackupNodeEditDistributedComponent } from '../backup-node-edit-distributed/backup-node-edit-distributed.component';
import { ClusterDetailDistributedComponent } from '../cluster-detail-distributed/cluster-detail-distributed.component';
import { MemberClusterDetailVo } from 'app/shared/api/models/member-cluster-detail-vo';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { NodeNetworkEditDistributedComponent } from '../node-network-edit-distributed/node-network-edit-distributed.component';

@Component({
  selector: 'aui-backup-cluster-distributed',
  templateUrl: './backup-cluster-distributed.component.html',
  styleUrls: ['./backup-cluster-distributed.component.less']
})
export class BackupClusterDistributedComponent
  implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  itemOptsConfig;
  errorNodeCount = 0;
  clusterData: MemberClusterDetailVo;
  deployType = find(DataMap.Deploy_Type, [
    'value',
    this.i18n.get('deploy_type')
  ]).label;
  clusterName = this.i18n.get('common_backup_cluster_label');
  status;
  unitconst = CAPACITY_UNIT;
  isDecouple = this.appUtilsService.isDecouple;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('clusterStatusTpl', { static: true })
  clusterStatusTpl: TemplateRef<any>;
  @ViewChild('clusterRoleTpl', { static: true }) clusterRoleTpl: TemplateRef<
    any
  >;

  constructor(
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    public clustersApiService: ClustersApiService,
    public backupClustersApiService: BackupClustersApiService,
    public dataMapService: DataMapService,
    public appUtilsService: AppUtilsService,
    public cdr?: ChangeDetectorRef,
    public virtualScroll?: VirtualScrollService,
    public cookieService?: CookieService
  ) {}

  ngOnInit(): void {
    if (this.appUtilsService.isDistributed) {
      this.initCluster();
    }
    this.initConfig();
    this.virtualScroll.getScrollParam(200);
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  initCluster() {
    this.backupClustersApiService
      .getBackupClusterLocalDetail({})
      .subscribe(res => {
        this.clusterData = res;
      });
    this.clustersApiService
      .getClustersInfoUsingGET({
        roleList: [7]
      })
      .subscribe(res => {
        this.clusterName = res.records[0].clusterName;
        this.status = res.records[0].status;
      });
  }

  initConfig() {
    const itemOpts: ProButton[] = [
      {
        id: 'edit',
        permission: OperateItems.EditPacificNodeNetwork,
        label: this.i18n.get('system_edit_pacific_node_network_label'),
        onClick: data => this.edit(data[0])
      }
    ];
    this.itemOptsConfig = getPermissionMenuItem(itemOpts);
    const opts: ProButton[] = [];
    if (this.appUtilsService.isDistributed) {
      opts.push({
        id: 'editAll',
        permission: OperateItems.EditPacificNodeNetwork,
        label: this.i18n.get('system_edit_pacific_node_network_label'),
        onClick: () => this.editAll()
      });
    }
    this.optsConfig = getPermissionMenuItem(opts);

    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('system_node_id_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: {
          type: 'text',
          config: {
            click: data => this.detail(data)
          }
        }
      },
      {
        key: 'role',
        name: this.i18n.get('common_role_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('DistributedClusterRole')
        },
        cellRender: this.clusterRoleTpl
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('DistributedClusterStatus')
        },
        cellRender: this.clusterStatusTpl
      },
      {
        key: 'manageIp',
        name: this.i18n.get('system_management_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        width: 144,
        hidden: !this.isDecouple,
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: this.itemOptsConfig
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'name',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        }
      }
    };
  }

  detail(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'memberNodeDetail',
        lvHeader: this.i18n.get('system_backup_node_detail_label'),
        lvWidth: 784,
        lvContent: BackupNodeDetailDistributedComponent,
        lvOkDisabled: false,
        lvComponentParams: {
          drawData: data
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: (modal, button) => {
              modal.close();
            }
          }
        ]
      })
    );
  }

  edit(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'memberNodeEdit',
        lvHeader: this.i18n.get('system_edit_pacific_node_network_label'),
        lvWidth: 784,
        lvContent: BackupNodeEditDistributedComponent,
        lvOkDisabled: false,
        lvComponentParams: {
          drawData: data
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as BackupNodeEditDistributedComponent;
          const modalIns = modal.getInstance();
          content.selectionValid.subscribe(
            res => (modalIns.lvOkDisabled = res)
          );
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as BackupNodeEditDistributedComponent;
            content.onOK().subscribe({
              next: res => resolve(true),
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  editAll() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'nodesEdit',
        lvHeader: this.i18n.get('system_edit_pacific_node_network_label'),
        lvWidth: 784,
        lvContent: NodeNetworkEditDistributedComponent,
        lvOkDisabled: false,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as NodeNetworkEditDistributedComponent;
          const modalIns = modal.getInstance();
          content.selectionInvalid.subscribe(
            res => (modalIns.lvOkDisabled = res)
          );
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as NodeNetworkEditDistributedComponent;
            content.onOK().subscribe({
              next: res => resolve(true),
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  clusterDetail() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'clusterDetail',
        lvHeader: this.i18n.get('system_cluster_detail_label'),
        lvWidth: 784,
        lvContent: ClusterDetailDistributedComponent,
        lvOkDisabled: false,
        lvComponentParams: {
          drawData: {
            ...this.clusterData,
            clusterName: this.clusterName,
            status: this.status
          }
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: (modal, button) => {
              modal.close();
            }
          }
        ]
      })
    );
  }

  getData(filters?: Filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    each(filters.filters, filter => {
      if (filter.value && size(filter.value)) {
        params[filter.key] = filter.value;
      }
    });

    this.clustersApiService.pageQueryPacificNodes(params).subscribe(res => {
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      this.errorNodeCount = this.tableData.data.filter(item => {
        return item.status !== DataMap.DistributedClusterStatus.healthy.value;
      }).length;
      this.cdr.detectChanges();
    });
  }

  clusterRole(item) {
    if (this.isDecouple) {
      return map(item.role, value =>
        this.dataMapService.getLabel('nodeRole', toUpper(value))
      );
    }
    return map(item.role, value =>
      this.dataMapService.getLabel('DistributedClusterRole', value)
    );
  }
}
