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
  Component,
  OnInit,
  ChangeDetectionStrategy,
  AfterViewInit,
  ViewChild,
  ChangeDetectorRef,
  TemplateRef
} from '@angular/core';
import {
  TableConfig,
  TableData,
  ProTableComponent,
  Filters,
  TableCols
} from 'app/shared/components/pro-table';
import {
  I18NService,
  DataMapService,
  WarningMessageService,
  MODAL_COMMON,
  CommonConsts,
  DataMap,
  ProtectedResourceApiService,
  getPermissionMenuItem,
  OperateItems,
  ResourceType,
  GROUP_COMMON,
  RoleOperationMap,
  hasResourcePermission,
  getLabelList,
  SetTagType
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  toString,
  isUndefined,
  isEmpty,
  assign,
  size,
  cloneDeep,
  reject,
  trim,
  each,
  map as _map,
  includes,
  some
} from 'lodash';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { map } from 'rxjs/operators';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { PostgreRegisterComponent } from './register/postgre-register.component';
import { PostgreDetailComponent } from './detail/postgre-detail.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { GetLabelOptionsService } from 'app/shared/services/get-labels.service';

@Component({
  selector: 'aui-postgre-cluster',
  templateUrl: './postgre-cluster.component.html',
  styleUrls: ['./postgre-cluster.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class PostgreClusterComponent implements OnInit, AfterViewInit {
  name;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  optItems = [];
  selectionData = [];

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService,
    private getLabelOptionsService: GetLabelOptionsService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(400);
    this.initConfig();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'register',
        type: 'primary',
        permission: RoleOperationMap.manageResource,
        label: this.i18n.get('common_register_label'),
        onClick: () => this.register(),
        popoverContent: this.i18n.get('protection_guide_cluster_tip_label'),
        popoverShow: USER_GUIDE_CACHE_DATA.active
      },
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyProtection,
        onClick: ([data]) => {
          this.protectedResourceApiService
            .ShowResource({
              resourceId: data.uuid
            })
            .subscribe((res: any) => {
              this.register(res);
            });
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteResource,
        onClick: data => {
          this.delete(data);
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        }
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
    const optsItem = getPermissionMenuItem(opts);
    this.optsConfig = reject(optsItem, item => {
      return includes(
        ['resourceAuth', 'resourceReclaiming', 'modify'],
        item.id
      );
    });
    this.optItems = cloneDeep(reject(optsItem, { id: 'register' }));

    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        extendParameter: [DataMap.Resource_Type.PostgreSQL.value]
      },
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
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: data => this.getDetail(data)
          }
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
        key: 'installDeployType',
        name: this.i18n.get('protection_cluster_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('PostgreSqlDeployType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('PostgreSqlDeployType')
        }
      },
      {
        key: 'clusterType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('PostgreSql_Cluster_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('PostgreSql_Cluster_Type')
        }
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_virtual_ip_label'),
        filter: {
          type: 'search'
        }
      },
      {
        key: 'labelList',
        name: this.i18n.get('common_tag_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: false,
          showSearch: true,
          options: () => this.getLabelOptionsService.getLabelOptions()
        },
        cellRender: this.resourceTagTpl
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: this.optItems
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'uuid',
        columns: cols,
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: true,
        fetchData: (filters: Filters, args) => {
          this.getData(filters, args);
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
      subType: DataMap.Resource_Type.PostgreSQLCluster.value
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.labelList) {
        conditionsTemp.labelList.shift();
        assign(conditionsTemp, {
          labelCondition: {
            labelList: conditionsTemp.labelList
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

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            // 获取标签数据
            const { showList, hoverList } = getLabelList(item);
            assign(item, {
              clusterType: item.extendInfo?.clusterType,
              installDeployType:
                item.extendInfo?.installDeployType ||
                DataMap.PostgreSqlDeployType.Pgpool.value,
              showLabelList: showList,
              hoverLabelList: hoverList
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          total: res.totalCount,
          data: res.records
        };
        this.cdr.detectChanges();
      });
  }

  getDetail(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data.name,
      lvModalKey: 'postgre_cluster_detail',
      lvWidth: MODAL_COMMON.largeWidth,
      lvContent: PostgreDetailComponent,
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

  register(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data
        ? this.i18n.get('common_modify_label')
        : this.i18n.get('common_register_label'),
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: PostgreRegisterComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as PostgreRegisterComponent;
        const modalIns = modal.getInstance();
        content.formGroup.statusChanges.subscribe(status => {
          modalIns.lvOkDisabled = status === 'INVALID';
        });
      },

      lvComponentParams: {
        data
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as PostgreRegisterComponent;
          content.onOK().subscribe({
            next: () => {
              resolve(true);
              this.dataTable?.fetchData();
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  delete(datas) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_resource_delete_label', [
        toString(_map(datas, 'name'))
      ]),
      onOK: () => {
        if (size(datas) === 1) {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: datas[0].uuid
            })
            .subscribe(res => {
              this.dataTable?.fetchData();
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
              this.dataTable?.fetchData();
            }
          );
        }
      }
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
    this.dataTable?.fetchData();
  }
}
