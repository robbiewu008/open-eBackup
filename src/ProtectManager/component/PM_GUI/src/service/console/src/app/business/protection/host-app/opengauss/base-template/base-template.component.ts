import {
  AfterViewInit,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DATE_PICKER_MODE,
  extendSlaInfo,
  getLabelList,
  getPermissionMenuItem,
  getTableOptsItems,
  GROUP_COMMON,
  hasBackupPermission,
  hasProtectPermission,
  hasRecoveryPermission,
  OperateItems,
  ProtectedResourceApiService,
  ProtectResourceAction,
  ProtectResourceCategory
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import {
  CookieService,
  DataMapService,
  I18NService
} from 'app/shared/services';
import { ProtectService } from 'app/shared/services/protect.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  isEmpty,
  isUndefined,
  map,
  size,
  some
} from 'lodash';

@Component({
  selector: 'aui-base-template',
  templateUrl: './base-template.component.html',
  styleUrls: ['./base-template.component.less']
})
export class BaseTemplateComponent implements OnInit, AfterViewInit {
  @Input() resourceSubType: string;
  @Input() columns: TableCols[];

  type = '';
  clusterName;
  selectionData = [];
  tableData: TableData;
  optItems: ProButton[];
  optsConfig: ProButton[];
  tableConfig: TableConfig;

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private protectService: ProtectService,
    private virtualScroll: VirtualScrollService,
    private cookieService: CookieService,
    private detailService: ResourceDetailService,
    private takeManualBackupService: TakeManualBackupService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.virtualScroll.getScrollParam(400);
    this.initConfig();
  }

  initConfig(): void {
    const btns: { [key: string]: ProButton } = {
      protect: {
        id: 'protect',
        permission: OperateItems.Protection,
        label: this.i18n.get('common_protect_label'),
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  isEmpty(val.sla_id) &&
                  val.protection_status !==
                    DataMap.Protection_Status.creating.value &&
                  val.protection_status !==
                    DataMap.Protection_Status.protected.value &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        onClick: data => {
          this.protect(data, ProtectResourceAction.Create);
        }
      },
      modifyProtect: {
        id: 'modifyProtect',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return !isEmpty(val.sla_id) && hasProtectPermission(val);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ModifyProtection,
        label: this.i18n.get('common_resource_protection_modify_label'),
        onClick: data =>
          this.protect(
            data,
            ProtectResourceAction.Modify,
            this.i18n.get('protection_modify_protection_label'),
            data
          )
      },
      removeProtection: {
        id: 'removeProtection',
        permission: OperateItems.RemoveProtection,
        label: this.i18n.get('protection_remove_protection_label'),
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  (!isEmpty(val.sla_id) ||
                    val.protection_status ===
                      DataMap.Protection_Status.protected.value) &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        onClick: data => {
          this.protectService
            .removeProtection(map(data, 'uuid'), map(data, 'name'))
            .subscribe(res => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      },
      activeProtection: {
        id: 'activeProtection',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  !isEmpty(val.sla_id) &&
                  !val.sla_status &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ActivateProtection,
        label: this.i18n.get('protection_active_protection_label'),
        onClick: data => {
          this.protectService
            .activeProtection(map(data, 'uuid'))
            .subscribe(res => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      },
      deactiveProtection: {
        id: 'deactiveProtection',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  !isEmpty(val.sla_id) &&
                  val.sla_status &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.DeactivateProtection,
        label: this.i18n.get('protection_deactive_protection_label'),
        onClick: data => {
          this.protectService
            .deactiveProtection(map(data, 'uuid'), map(data, 'name'))
            .subscribe(res => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      },
      recovery: {
        id: 'recovery',
        disableCheck: data => {
          return (
            !size(data) || some(data, item => !hasRecoveryPermission(item))
          );
        },
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('common_restore_label'),
        onClick: data => {
          this.getResourceDetail({
            ...data[0],
            activeId: 'copydata',
            datePickerMode: DATE_PICKER_MODE.DATE
          });
        }
      },
      manualBackup: {
        id: 'manualBackup',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return !isEmpty(val.sla_id) && hasBackupPermission(val);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('common_manual_backup_label'),
        onClick: data => {
          this.manualBackup(data);
        }
      },
      addTag: {
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
      removeTag: {
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
    };

    this.optsConfig = getPermissionMenuItem(
      [
        btns.protect,
        btns.removeProtection,
        btns.activeProtection,
        btns.deactiveProtection,
        btns.manualBackup,
        btns.addTag,
        btns.removeTag
      ],
      this.cookieService.role
    );

    this.optItems = getPermissionMenuItem(
      [
        btns.protect,
        btns.modifyProtect,
        assign(cloneDeep(btns.removeProtection), {
          divide: true
        }),
        btns.activeProtection,
        assign(cloneDeep(btns.deactiveProtection), {
          divide: true
        }),
        btns.recovery,
        btns.manualBackup,
        btns.addTag,
        btns.removeTag
      ],
      this.cookieService.role
    );

    let baseColumns: TableCols[] = [
      {
        key: 'belong_cluster',
        name: this.i18n.get('insight_report_belong_cluster_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'sync_mode',
        name: this.i18n.get('protection_data_sync_mode_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          options: this.dataMapService.toArray('sync_mode')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('sync_mode')
        },
        hidden:
          this.resourceSubType ===
          DataMap.Resource_Type.OpenGauss_database.value
      },
      {
        key: 'sla_name',
        name: this.i18n.get('common_sla_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            overflow: true,
            click: data => {
              this.slaService.getDetail({
                uuid: data.sla_id,
                name: data.sla_name
              });
            }
          }
        }
      },
      {
        key: 'sla_compliance',
        name: this.i18n.get('common_sla_compliance_label'),
        thExtra: this.slaComplianceExtraTpl,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Sla_Compliance')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Sla_Compliance')
        }
      },
      {
        key: 'protectionStatus',
        name: this.i18n.get('protection_protected_status_label'),
        width: 130,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Protection_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Protection_Status')
        }
      },
      // 新增标签
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
            items: this.optItems
          }
        }
      }
    ];

    if (
      find(this.columns, { key: 'name' }) &&
      find(this.columns, { key: 'name' }).cellRender['config']
    ) {
      assign(find(this.columns, { key: 'name' }).cellRender['config'], {
        click: data => this.getResourceDetail(data)
      });
    }

    this.tableConfig = {
      table: {
        async: true,
        columns: isEmpty(this.columns)
          ? [...baseColumns]
          : [...this.columns, ...baseColumns],
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        compareWith: 'uuid',
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args: {}) => {
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

  getData(filters: Filters, args: any) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType: [this.resourceSubType]
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.belong_cluster) {
        assign(defaultConditions, {
          environment: {
            name: conditionsTemp.belong_cluster
          }
        });
        delete conditionsTemp.belong_cluster;
      }
      if (conditionsTemp.owned_instance) {
        assign(defaultConditions, {
          parentName: conditionsTemp.owned_instance
        });
        delete conditionsTemp.owned_instance;
      }
      if (conditionsTemp.sync_mode) {
        assign(defaultConditions, {
          syncMode: conditionsTemp.sync_mode
        });
        delete conditionsTemp.sync_mode;
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

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService
      .ListResources(params)
      .subscribe((res: any) => {
        each(res.records, item => {
          // 获取标签数据
          const { showList, hoverList } = getLabelList(item);
          assign(item, {
            showLabelList: showList,
            hoverLabelList: hoverList
          });

          if (
            item.environment.extendInfo.clusterState ===
            DataMap.opengauss_Clusterstate.unavailable.value
          ) {
            assign(item, {
              sub_type: item.subType,
              belong_cluster: item['environment']['name'],
              owned_instance: item.parentName,
              instanceStatus: DataMap.openGauss_InstanceStatus.offline.value,
              sync_mode: item.environment.extendInfo.syncMode
            });
          } else {
            assign(item, {
              sub_type: item.subType,
              belong_cluster: item['environment']['name'],
              owned_instance: item.parentName,
              instanceStatus:
                item.extendInfo.instanceState ===
                DataMap.openGauss_InstanceStatus.normal.value
                  ? DataMap.openGauss_InstanceStatus.normal.value
                  : DataMap.openGauss_InstanceStatus.offline.value,
              sync_mode: item.environment.extendInfo.syncMode
            });
          }
          extendSlaInfo(item);
        });
        let records = res.records;
        if (filters.conditions) {
          const conditions = JSON.parse(filters.conditions);
          if (size(conditions.instanceStatus) === 1) {
            records = res.records.filter(item => {
              return item.instanceStatus === conditions.instanceStatus[0];
            });
          }
        }
        this.tableData = {
          data: records,
          total: res.totalCount
        };
      });
  }
  protect(datas, action: ProtectResourceAction, header?: string, refreshData?) {
    const data = size(datas) > 1 ? datas : datas[0];
    this.protectService.openProtectModal(
      ProtectResourceCategory.OpenGauss,
      action,
      {
        width: 780,
        data,
        onOK: () => {
          this.dataTable.fetchData();
          this.selectionData = [];
          this.dataTable.setSelections([]);
        }
      }
    );
  }
  manualBackup(datas) {
    if (size(datas) > 1) {
      each(datas, item => {
        assign(item, {
          host_ip: item.environment_endpoint,
          resource_id: item.uuid,
          resource_type: datas[0].sub_type
        });
      });
      this.takeManualBackupService.batchExecute(datas, () =>
        this.dataTable.fetchData()
      );
    } else {
      assign(datas[0], {
        host_ip: datas[0].environment_endpoint,
        resource_id: datas[0].uuid,
        resource_type: datas[0].sub_type
      });
      this.takeManualBackupService.execute(datas[0], () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      });
    }
  }
  search() {}
  getResourceDetail(resource: any) {
    this.detailService.openDetailModal(resource.subType, {
      data: assign(
        {},
        resource,
        {
          optItems: getTableOptsItems(cloneDeep(this.optItems), resource, this)
        },
        {
          optItemsFn: v => {
            return getTableOptsItems(cloneDeep(this.optItems), v, this);
          }
        }
      )
    });
  }
}
