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
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import {
  CommonConsts,
  DataMap,
  DataMapService,
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
  SetTagType,
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
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  isEmpty,
  isUndefined,
  map,
  size,
  some,
  trim
} from 'lodash';
import { CreateClusterComponent } from '../../kubernetes-container/create-cluster/create-cluster.component';
import { ClustreDetailComponent } from './clustre-detail/clustre-detail.component';
import { RegisterClusterComponent } from './register-cluster/register-cluster.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';

@Component({
  selector: 'aui-kubernetes-cluster',
  templateUrl: './cluster.component.html',
  styleUrls: ['./cluster.component.less']
})
export class ClusterComponent implements OnInit, AfterViewInit {
  optsConfig: ProButton[];
  selectionData: any[];
  tableConfig: TableConfig;
  tableData: TableData;
  clusterName: string;
  dataMap = DataMap;
  data: any[] = [];
  optItems: ProButton[];
  sourceType = DataMap.Resource_Type.Kubernetes.value;

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @Input() columns: TableCols[];
  @Input() subType: string;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    public virtualScroll: VirtualScrollService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }
  ngOnInit() {
    this.virtualScroll.getScrollParam(400);
    this.initConfig();
  }

  initConfig() {
    this.optsConfig = getPermissionMenuItem([
      {
        id: 'register',
        label: this.i18n.get('common_register_label'),
        permission: RoleOperationMap.manageResource,
        type: 'primary',
        onClick: () => this.registerCluster()
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteResource,
        disableCheck: data => {
          return !size(data);
        },
        onClick: data => this.deleteCluster(data)
      },
      {
        id: 'addTag',
        permission: OperateItems.AddTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
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
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(data)
      }
    ]);

    const tableOpts: ProButton[] = [
      {
        id: 'scan',
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
        id: 'connectionTest',
        label: this.i18n.get('protection_connectivity_test_label'),
        divide: true,
        onClick: ([data]) => {
          this.protectedResourceApiService
            .CheckProtectedResource({ resourceId: data.uuid })
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
        label: this.i18n.get('common_modify_label'),
        onClick: ([resource]) => {
          this.protectedResourceApiService
            .ShowResource({ resourceId: resource.uuid })
            .subscribe(res => {
              this.registerCluster(res);
            });
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        onClick: data => this.deleteCluster(data)
      },
      {
        id: 'addTag',
        permission: OperateItems.AddTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
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
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(data)
      }
    ];

    this.optItems = cloneDeep(tableOpts);

    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: {
          type: 'text',
          config: {
            click: data => this.getResourceDetail(data)
          }
        }
      },
      {
        key: 'version',
        name: this.i18n.get('common_version_label'),
        hidden:
          this.subType !== DataMap.Resource_Type.kubernetesClusterCommon.value,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('resource_LinkStatus_Special')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
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
            items: tableOpts
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'name',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.name;
        }
      },
      pagination: {}
    };
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data ? data : this.selectionData,
      type: SetTagType.Resource,
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
      type: SetTagType.Resource,
      onOk: () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      }
    });
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType:
        this.subType === DataMap.Resource_Type.kubernetesClusterCommon.value
          ? [this.subType]
          : [DataMap.Resource_Type.Kubernetes.value]
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.equipment) {
        assign(conditionsTemp, {
          environment: {
            name: conditionsTemp.equipment
          }
        });
        delete conditionsTemp.equipment;
      }
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

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        // 获取标签数据
        const { showList, hoverList } = getLabelList(item);
        assign(item, {
          showLabelList: showList,
          hoverLabelList: hoverList
        });
      });
      this.tableData = {
        total: res.totalCount,
        data: res.records
      };
    });
  }

  search() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(this.clusterName)
        }
      ]
    });
    this.dataTable.fetchData();
  }

  registerCluster(resource?: object) {
    let registerComponent: any = RegisterClusterComponent;
    let modalWidth = MODAL_COMMON.normalWidth + 100;
    if (this.subType === DataMap.Resource_Type.kubernetesClusterCommon.value) {
      registerComponent = CreateClusterComponent;
      modalWidth = this.i18n.isEn
        ? MODAL_COMMON.normalWidth + 320
        : MODAL_COMMON.normalWidth + 200;
    }
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-kubernetes-cluster',
        lvWidth: modalWidth,
        lvHeader: isEmpty(resource)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: registerComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowItem: { ...resource }
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent();
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.dataTable.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  deleteCluster(datas: any[]) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_resource_delete_label', [
        map(datas, 'name').join(',')
      ]),
      onOK: () => {
        if (size(datas) === 1) {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: datas[0].uuid
            })
            .subscribe(res => {
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
              this.dataTable.fetchData();
            }
          );
        }
      }
    });
  }
  getResourceDetail(resource: any) {
    this.protectedResourceApiService
      .ShowResource({ resourceId: resource.uuid })
      .subscribe(res => {
        this.drawModalService.create(
          assign({}, MODAL_COMMON.generateDrawerOptions(), {
            lvModalKey: 'kubernetes-cluster-detail',
            lvWidth: MODAL_COMMON.normalWidth + 100,
            lvHeader: resource.name,
            lvContent: ClustreDetailComponent,
            lvComponentParams: {
              rowItem: { ...res },
              subType: this.subType
            },
            lvFooter: [
              {
                label: this.i18n.get('common_close_label'),
                onClick: modal => modal.close()
              }
            ]
          })
        );
      });
  }
}
