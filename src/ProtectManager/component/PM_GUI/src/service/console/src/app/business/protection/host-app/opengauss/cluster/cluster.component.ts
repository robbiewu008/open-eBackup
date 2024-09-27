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
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import {
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  extendSlaInfo,
  getLabelList,
  getPermissionMenuItem,
  GROUP_COMMON,
  hasResourcePermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RoleOperationMap,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  first,
  isEmpty,
  isUndefined,
  map,
  size,
  some,
  trim
} from 'lodash';
import { DetailComponent } from './detail/detail.component';
import { RegisterClusterComponent } from './register-cluster/register-cluster.component';

@Component({
  selector: 'aui-openGauss-cluster',
  templateUrl: './cluster.component.html',
  styleUrls: ['./cluster.component.less']
})
export class ClusterComponent implements OnInit, AfterViewInit {
  name;
  deleteNames;
  data: any[] = [];
  dataMap = DataMap;
  optsConfig: ProButton[];
  optsConfigSearch: ProButton[];
  selectionData: any[];
  tableConfig: TableConfig;
  tableData: TableData;
  clusterName: string;

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private messageService: MessageService,
    private cdr: ChangeDetectorRef,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private virtualScroll: VirtualScrollService,
    private drawModalService: DrawModalService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.virtualScroll.getScrollParam(400);
    this.initConfig();
  }

  onChange() {
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  initConfig() {
    this.optsConfig = getPermissionMenuItem(
      [
        {
          id: 'register',
          permission: RoleOperationMap.manageResource,
          label: this.i18n.get('common_register_label'),
          type: 'primary',
          onClick: () => this.registerCluster(),
          popoverContent: this.i18n.get('protection_guide_cluster_tip_label'),
          popoverShow: USER_GUIDE_CACHE_DATA.active
        },
        {
          id: 'deleteResource',
          permission: OperateItems.DeleteResource,
          label: this.i18n.get('common_delete_label'),
          disableCheck: data => {
            return !size(data) || some(data, v => !hasResourcePermission(v));
          },
          onClick: data => {
            this.delete(data);
          }
        },
        {
          id: 'addTag',
          permission: OperateItems.AddTag,
          displayCheck: data => {
            return true;
          },
          disableCheck: data => {
            return !size(data);
          },
          label: this.i18n.get('common_add_tag_label'),
          onClick: data => this.addTag(data)
        },
        {
          id: 'removeTag',
          permission: OperateItems.RemoveTag,
          displayCheck: data => {
            return true;
          },
          disableCheck: data => {
            return !size(data);
          },
          label: this.i18n.get('common_remove_tag_label'),
          onClick: data => this.removeTag(data)
        }
      ],
      this.cookieService.role
    );

    const tableOpts: ProButton[] = [
      {
        id: 'rescan',
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('common_rescan_label'),
        onClick: data => {
          this.protectedResourceApiService
            .ScanProtectedResources({
              resId: data[0].uuid
            })
            .subscribe(res => {
              this.dataTable.fetchData();
            });
        }
      },
      {
        id: 'connectivityTest',
        divide: true,
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('protection_connectivity_test_label'),
        onClick: ([data]) => {
          this.connectTest(data);
        }
      },
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyingCluster,
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => {
          this.registerCluster(first(data));
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.Deletingcluster,
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => {
          this.delete(data);
        }
      },
      {
        id: 'addTag',
        permission: OperateItems.AddTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return !size(data);
        },
        label: this.i18n.get('common_add_tag_label'),
        onClick: data => this.addTag(data)
      },
      {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return !size(data);
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(data)
      }
    ];
    this.optsConfigSearch = tableOpts;

    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        sort: true,
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: data => this.getDetail(data)
          }
        }
      },
      {
        key: 'clusterState',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('opengauss_Clusterstate')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('opengauss_Clusterstate')
        }
      },
      {
        key: 'clusterVersion',
        name: this.i18n.get('common_version_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'deployType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Opengauss_Deployment_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Opengauss_Deployment_Type')
        }
      },
      {
        key: 'labelList',
        name: this.i18n.get('common_tag_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.resourceTagTpl
      },
      {
        key: 'operation',
        width: 130,
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: getPermissionMenuItem(tableOpts, this.cookieService.role)
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'uuid',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data ? data : this.selectionData,
      onOk: () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      }
    });
  }

  removeTag(data) {
    this.setResourceTagService.setTag({
      isAdd: false,
      rowDatas: data ? data : this.selectionData,
      onOk: () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      }
    });
  }

  getData(filters: Filters, args: { isAutoPolling: any }) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType: DataMap.Resource_Type.OpenGauss.value
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.equipmentType) {
        if (isEmpty(conditionsTemp.environment)) {
          assign(conditionsTemp, {
            environment: {
              subType: conditionsTemp.equipmentType
            }
          });
        } else {
          assign(conditionsTemp.environment, {
            subType: conditionsTemp.equipmentType
          });
        }
        delete conditionsTemp.equipmentType;
      }
      if (conditionsTemp.labelList) {
        assign(conditionsTemp, {
          labelCondition: {
            labelName: conditionsTemp.labelList[1]
          }
        });
        delete conditionsTemp.labelList;
      }
      assign(defaultConditions, conditionsTemp);
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        // 获取标签数据
        const { showList, hoverList } = getLabelList(item);
        assign(item, {
          sub_type: item.subType,
          deployType: item.extendInfo.deployType,
          clusterVersion: item.extendInfo.clusterVersion,
          clusterState: item.extendInfo.clusterState,
          showLabelList: showList,
          hoverLabelList: hoverList
        });
        extendSlaInfo(item);
      });
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      this.cdr.detectChanges();
    });
  }

  registerCluster(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-openGauss-cluster',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(data)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterClusterComponent,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterClusterComponent;
          content.formGroup.statusChanges.subscribe(status => {
            modal.getInstance().lvOkDisabled = status !== 'VALID';
          });
        },
        lvComponentParams: {
          data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterClusterComponent;
            content.onOK().subscribe(
              res => {
                resolve(true);
                this.dataTable.fetchData();
              },
              err => {
                resolve(false);
              }
            );
          });
        }
      })
    );
  }
  delete(datas: any[]) {
    this.deleteNames = map(datas, item => {
      return item.name;
    });
    this.warningMessageService.create({
      content: this.i18n.get('protection_resource_delete_label', [
        this.deleteNames.join(',')
      ]),
      onOK: () => {
        if (size(datas) === 1) {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: datas[0].uuid
            })
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            });
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.protectedResourceApiService.DeleteResource({
                resourceId: item.uuid
              });
            },
            datas,
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            }
          );
        }
      }
    });
  }

  getDetail(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data.name,
      lvModalKey: 'openGauss_cluster_detail',
      lvWidth: MODAL_COMMON.normalWidth + 70,
      lvContent: DetailComponent,
      lvComponentParams: {
        data
      },
      lvFooter: [
        {
          id: 'close',
          label: this.i18n.get('common_close_label'),
          onClick: modal => {
            modal.close();
          }
        }
      ]
    });
  }

  connectTest(data) {
    this.protectedResourceApiService
      .CheckProtectedResource({ resourceId: data.uuid })
      .subscribe(res => {
        this.messageService.success(this.i18n.get('job_status_success_label'), {
          lvMessageKey: 'successKey',
          lvShowCloseButton: true
        });
        this.dataTable.fetchData();
      });
  }

  search() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(this.name)
        }
      ]
    });
    this.dataTable.fetchData();
  }
}
