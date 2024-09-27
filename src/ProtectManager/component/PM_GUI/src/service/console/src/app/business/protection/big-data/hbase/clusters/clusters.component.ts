import {
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  ViewChild,
  TemplateRef,
  Input
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  getTableOptsItems,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService,
  ProtectedResourceApiService,
  ProtectedEnvironmentApiService,
  ResourceType,
  ExceptionService,
  LANGUAGE,
  GROUP_COMMON,
  RoleOperationMap,
  hasResourcePermission,
  getLabelList
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
  toString,
  isNumber,
  includes,
  get,
  find,
  each,
  some
} from 'lodash';
import { RegisterClusterComponent } from '../../hdfs/clusters/register-cluster/register-cluster.component';
import { RegisterClusterComponent as HiveRegisterClusterComponent } from '../../hive/register-cluster/register-cluster.component';
import { RegisterClusterComponent as EsRegisterClusterComponent } from '../../elasticSearch/register-cluster/register-cluster.component';

import { combineLatest } from 'rxjs';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { MessageService } from '@iux/live';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';

@Component({
  selector: 'aui-clusters',
  templateUrl: './clusters.component.html',
  styleUrls: ['./clusters.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ClustersComponent implements OnInit, AfterViewInit {
  @Input() resSubType;
  name;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  optItems = [];
  proxyHostOptions = [];
  selectionData = [];
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('agentsTpl', { static: true }) agentsTpl: TemplateRef<any>;
  @ViewChild('esAddressTpl', { static: true }) esAddressTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  groupCommon = GROUP_COMMON;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private exceptionService: ExceptionService,
    private message: MessageService,
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
      subType: this.resSubType || DataMap.Resource_Type.HBase.value
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
          item['authType'] =
            item?.auth?.authType ===
              DataMap.HDFS_Clusters_Auth_Type.ldap.value ||
            item?.auth?.authType === 2
              ? DataMap.HDFS_Clusters_Auth_Type.kerberos.value
              : item?.auth?.authType;
          this.getAgentsName(item);
          item['showLabelList'] = showList;
          item['hoverLabelList'] = hoverList;
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
        id: 'connectTest',
        permission: OperateItems.ModifyHdfsCluster,
        divide: true,
        label: this.i18n.get('protection_connectivity_test_label'),
        onClick: data => {
          this.connectTest(data);
        },
        displayCheck: data => {
          return (
            this.resSubType === DataMap.Resource_Type.Hive.value ||
            this.resSubType === DataMap.Resource_Type.Elasticsearch.value
          );
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
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

    const optsItem = getPermissionMenuItem(opts);
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
        key: 'EelasticSearchAddress',
        name: this.i18n.get('protection_server_ip_label'),
        hidden: !includes(
          [DataMap.Resource_Type.Elasticsearch.value],
          this.resSubType
        ),
        cellRender: this.esAddressTpl
      },
      {
        key: 'endpoint',
        name:
          this.resSubType === DataMap.Resource_Type.Hive.value
            ? this.i18n.get('protection_hive_server_link_label')
            : this.resSubType === DataMap.Resource_Type.Elasticsearch.value
            ? this.i18n.get('protection_server_ip_label')
            : this.i18n.get('common_ip_domain_name_label'),
        hidden: includes(
          [DataMap.Resource_Type.Elasticsearch.value],
          this.resSubType
        ),
        filter: {
          type: 'search',
          filterMode: 'contains'
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
        hidden: includes(
          [
            DataMap.Resource_Type.Elasticsearch.value,
            DataMap.Resource_Type.Hive.value
          ],
          this.resSubType
        ),
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
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filters: Filters, args) => {
          this.getData(filters, args);
        },
        trackByFn: (index, item) => {
          return item.uuid;
        },
        selectionChange: selection => {
          this.selectionData = selection;
        }
      }
    };
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data,
      onOk: () => {
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
    if (this.resSubType === DataMap.Resource_Type.Hive.value) {
      this.hiveRegister(data);
    } else if (this.resSubType === DataMap.Resource_Type.Elasticsearch.value) {
      this.esRegister(data);
    } else {
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
            isHbase: true
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
  }

  hiveRegister(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data
        ? this.i18n.get('common_modify_label')
        : this.i18n.get('common_register_label'),
      lvModalKey: 'register-hive-cluster',
      lvWidth:
        this.i18n.language === LANGUAGE.EN && data?.uuid
          ? MODAL_COMMON.largeWidth
          : MODAL_COMMON.normalWidth + 100,
      lvContent: HiveRegisterClusterComponent,
      lvComponentParams: {
        data: {
          ...data,
          getCluster: () => this.dataTable.fetchData()
        }
      }
    });
  }

  esRegister(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data
        ? this.i18n.get('common_modify_label')
        : this.i18n.get('common_register_label'),
      lvModalKey: 'register-elasticseach-cluster',
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvContent: EsRegisterClusterComponent,
      lvComponentParams: {
        data: {
          ...data,
          getCluster: () => this.dataTable.fetchData()
        }
      }
    });
  }

  unRegister(data) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_hbase_resource_delete_warn_label', [
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
    this.detailService.openDetailModal(DataMap.Resource_Type.Hive.value, {
      data: {
        ...data,
        optItems: getTableOptsItems(this.optItems, data, this),
        optItemsFn: v => {
          return getTableOptsItems(this.optItems, v, this);
        }
      }
    });
  }
  addData(array: any[], item) {
    array.push({
      ...item,
      key: item.uuid,
      label: item.environment?.endpoint,
      value: item.rootUuid || item.parentUuid,
      isLeaf: true
    });
  }
  getAgents(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: 200,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          subType: [`${DataMap.Resource_Type.HBaseBackupSet.value}Plugin`]
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
          const agentArr = [];
          each(recordsTemp, item => {
            this.addData(agentArr, item);
          });
          this.proxyHostOptions = agentArr;
          this.dataTable?.fetchData();
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
      return includes(cluster.extendInfo?.agents.split(';'), item.value);
    });

    if (!!size(filters)) {
      const label = toString(map(filters, 'label')) as any;
      if (label) {
        cluster['proxyHosts'] = label.replace(/,/g, '/');
      }
    }

    return cluster['proxyHosts'];
  }

  connectTest(data) {
    const params =
      this.resSubType === DataMap.Resource_Type.Hive.value
        ? {
            name: data[0].name,
            type: ResourceType.BIG_DATA,
            subType: DataMap.Resource_Type.Hive.value,
            extendInfo: {
              hiveServerUrl: data[0].extendInfo?.hiveServerUrl,
              hiveServerPrincipal: data[0].extendInfo?.hiveServerPrincipal,
              zookeeperNameSpace: data[0].extendInfo?.zookeeperNameSpace,
              hdfsSite: data[0].extendInfo?.hdfsSite,
              coreSite: data[0].extendInfo?.coreSite,
              hiveSite: data[0].extendInfo?.hiveSite,
              hiveClient: data[0].extendInfo?.hiveClient,
              hiveVersion: data[0].extendInfo?.hiveVersion,
              kerberosId: data[0].extendInfo?.kerberosId,
              agents: data[0].extendInfo.agents
            },
            auth: {
              authType: data[0].authType,
              authKey: data[0].auth.authKey,
              extendInfo: {}
            },
            dependencies: {
              agents: map(data[0].extendInfo.agents.split(';'), item => {
                return {
                  uuid: item
                };
              })
            }
          }
        : {
            name: data[0].name,
            type: ResourceType.BIG_DATA,
            subType: DataMap.Resource_Type.Elasticsearch.value,
            extendInfo: {
              kerberosId: data[0].extendInfo?.kerberosId,
              agents: data[0].extendInfo.agents,
              ElasticSearchAddress: data[0].extendInfo.ElasticSearchAddress,
              ElasticSearchUser: data[0].extendInfo.ElasticSearchUser,
              ElasticSearchGroup: data[0].extendInfo.ElasticSearchGroup
            },
            auth: {
              authType: data[0].authType,
              authKey: data[0].auth.authKey,
              extendInfo: {}
            },
            dependencies: {
              agents: map(data[0].extendInfo.agents.split(';'), item => {
                return {
                  uuid: item
                };
              })
            }
          };
    if (this.resSubType === DataMap.Resource_Type.Elasticsearch.value) {
      this.protectedResourceApiService
        .CheckProtectedResource({
          resourceId: first(data)['uuid']
        })
        .subscribe(res => {
          let returnRes;
          try {
            returnRes = JSON.parse(res);
          } catch (error) {
            returnRes = [];
          }
          const idx = returnRes.findIndex(item => item.code !== 0);
          if (idx !== -1) {
            this.message.error(this.i18n.get(returnRes[idx].code), {
              lvMessageKey: 'errorKey',
              lvShowCloseButton: true
            });
          } else {
            this.message.success(
              this.i18n.get('common_operate_success_label'),
              {
                lvMessageKey: 'successKey',
                lvShowCloseButton: true
              }
            );
          }
        });
    } else {
      this.protectedEnvironmentApiService
        .CheckEnvironment({
          checkEnvironmentRequestBody: params,
          akOperationTips: false
        })
        .subscribe(res => {
          const error =
            find(JSON.parse(res), item => item.code !== 0) ||
            first(JSON.parse(res));

          if (get(error, 'code') !== 0) {
            this.exceptionService.alertMsg({
              errorCode: get(error, 'code'),
              errorMessage: get(error, 'message')
            });
          } else {
            this.message.success(this.i18n.get('common_operate_success_label'));
          }
          this.dataTable.fetchData();
        });
    }
  }
}
