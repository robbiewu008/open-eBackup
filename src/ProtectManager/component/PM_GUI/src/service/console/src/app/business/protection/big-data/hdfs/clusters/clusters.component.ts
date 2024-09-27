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
  ViewChild,
  TemplateRef
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  getLabelList,
  getPermissionMenuItem,
  getTableOptsItems,
  GROUP_COMMON,
  hasResourcePermission,
  I18NService,
  LANGUAGE,
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
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  filter as _filter,
  first,
  isEmpty,
  isUndefined,
  reject,
  size,
  trim,
  map,
  isNumber,
  toString,
  includes,
  some
} from 'lodash';
import { combineLatest } from 'rxjs';
import { ClusterDetailComponent } from './cluster-detail/cluster-detail.component';
import { RegisterClusterComponent } from './register-cluster/register-cluster.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';

@Component({
  selector: 'aui-clusters',
  templateUrl: './clusters.component.html',
  styleUrls: ['./clusters.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ClustersComponent implements OnInit, AfterViewInit {
  name;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  optItems = [];
  proxyHostOptions = [];
  selectionData: any[];

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('agentsTpl', { static: true }) agentsTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private cookieService: CookieService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(400);
    this.initConfig();
    this.getAgents();
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType: DataMap.Resource_Type.HDFS.value
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
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
      this.tableData = {
        total: res.totalCount,
        data: map(res.records, item => {
          // 获取标签数据
          const { showList, hoverList } = getLabelList(item);
          assign(item, {
            showLabelList: showList,
            hoverLabelList: hoverList
          });

          item['authType'] =
            item?.auth?.authType === DataMap.HDFS_Clusters_Auth_Type.ldap.value
              ? DataMap.HDFS_Clusters_Auth_Type.kerberos.value
              : item?.auth?.authType;
          return item;
        })
      };
      this.cdr.detectChanges();
    });
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'register',
        type: 'primary',
        permission: RoleOperationMap.manageResource,
        label: this.i18n.get('common_register_label'),
        onClick: () => {
          this.register();
        }
      },
      {
        id: 'modify',
        permission: OperateItems.ModifyHdfsCluster,
        label: this.i18n.get('common_modify_label'),
        onClick: data => {
          this.register(first(data));
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        }
      },
      {
        id: 'delete',
        permission: OperateItems.UnRegisterHdfsCluster,
        label: this.i18n.get('common_delete_label'),
        onClick: data => {
          this.unRegister(first(data));
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        }
      },
      {
        id: 'addTag',
        permission: OperateItems.AddTag,
        disableCheck: data => {
          return !size(data);
        },
        label: this.i18n.get('common_add_tag_label'),
        onClick: data => this.addTag(data)
      },
      {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        disableCheck: data => {
          return !size(data);
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(data)
      }
    ];

    const optsItem = getPermissionMenuItem(opts, this.cookieService.role);
    this.optsConfig = _filter(optsItem, v =>
      includes(['register', 'addTag', 'removeTag'], v.id)
    );
    this.optItems = cloneDeep(reject(optsItem, { id: 'register' }));

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
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: data => {
              this.getDetail(data);
            }
          }
        }
      },
      {
        key: 'endpoint',
        name: this.i18n.get('fs.defaultFS'),
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
        key: 'authType',
        name: this.i18n.get('protection_auth_mode_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('HDFS_Clusters_Auth_Type')
        }
      },
      {
        key: 'agents',
        name: this.i18n.get('protection_proxy_host_label'),
        cellRender: this.agentsTpl
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
        scrollFixed: true,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
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
      rowDatas: data,
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
      rowDatas: data,
      onOk: () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      }
    });
  }

  search(value) {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(value)
        }
      ]
    });
    this.dataTable.fetchData();
  }

  register(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data
        ? this.i18n.get('common_modify_label')
        : this.i18n.get('common_register_label'),
      lvModalKey: 'register-hdfs-cluster',
      lvOkLoadingText: this.i18n.get('common_loading_label'),
      lvWidth:
        this.i18n.language === LANGUAGE.EN && data?.uuid
          ? MODAL_COMMON.largeWidth
          : MODAL_COMMON.normalWidth + 100,
      lvContent: RegisterClusterComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as RegisterClusterComponent;
        const modalIns = modal.getInstance();
        const combined: any = data?.uuid
          ? combineLatest([content.formGroup.statusChanges])
          : combineLatest([
              content.formGroup.statusChanges,
              content.validHdfsSite$,
              content.validCoreSite$
            ]);

        if (data?.uuid) {
          modalIns.lvOkDisabled = content.formGroup.invalid;
          combined.subscribe(latestValues => {
            const [formGroupStatus] = latestValues;
            modalIns.lvOkDisabled = !(formGroupStatus === 'VALID');
          });
        } else {
          combined.subscribe(latestValues => {
            const [
              formGroupStatus,
              validHdfsSite,
              validCoreSite
            ] = latestValues;
            modalIns.lvOkDisabled = !(
              formGroupStatus === 'VALID' &&
              validHdfsSite &&
              validCoreSite
            );
          });
        }
      },
      lvComponentParams: {
        data: {
          ...data,
          isHbase: false
        }
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as RegisterClusterComponent;
          content.onOK().subscribe({
            next: () => {
              resolve(true);
              this.dataTable.fetchData();
            },
            error: error => resolve(false)
          });
        });
      }
    });
  }

  unRegister(data) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_hdf_fileset_delete_warn_label', [
        data.name
      ]),
      onOK: () => {
        this.protectedEnvironmentApiService
          .DeleteProtectedEnvironment({
            envId: data.uuid
          })
          .subscribe(() => this.dataTable.fetchData());
      }
    });
  }

  getDetail(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data.name,
      lvModalKey: 'hdfs_cluster_detail',
      lvWidth: MODAL_COMMON.largeWidth - 100,
      lvContent: ClusterDetailComponent,
      lvComponentParams: {
        data: {
          ...data,
          optItems: getTableOptsItems(this.optItems, data, this),
          optItemsFn: v => {
            return getTableOptsItems(this.optItems, v, this);
          }
        }
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

  getAgents(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: 200,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          subType: [`${DataMap.Resource_Type.HDFSFileset.value}Plugin`],
          environment: {
            linkStatus: [
              ['in'],
              toString(DataMap.resource_LinkStatus.normal.value)
            ]
          }
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
          startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
          res.totalCount === 0
        ) {
          this.proxyHostOptions = map(recordsTemp, item => {
            return {
              ...item,
              key: item.uuid,
              label: item.environment?.endpoint,
              value: item.rootUuid || item.parentUuid,
              isLeaf: true
            };
          });
          return;
        }
        this.getAgents(recordsTemp, startPage);
      });
  }

  getAgentsName(cluster) {
    cluster['proxyHosts'] = '--';
    if (!size(this.proxyHostOptions)) {
      return cluster['proxyHosts'];
    }

    const filters = this.proxyHostOptions.filter(item => {
      return includes(cluster.extendInfo?.agents?.split(';'), item.value);
    });

    if (!!size(filters)) {
      const label = toString(map(filters, 'label')) as any;
      if (label) {
        cluster['proxyHosts'] = label.replace(/,/g, '/');
      }
    }

    return cluster['proxyHosts'];
  }
}
