import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { SlaService } from 'app/shared/services/sla.service';
import { MessageService } from '@iux/live';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import {
  Component,
  OnInit,
  ViewChild,
  AfterViewInit,
  TemplateRef,
  ChangeDetectorRef
} from '@angular/core';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData,
  Filters
} from 'app/shared/components/pro-table';
import {
  OperateItems,
  I18NService,
  DataMap,
  getPermissionMenuItem,
  DataMapService,
  ProtectResourceAction,
  extendSlaInfo,
  DATE_PICKER_MODE,
  ResourceType,
  getTableOptsItems,
  CommonConsts,
  ProtectResourceCategory,
  ProtectedResourceApiService,
  GROUP_COMMON,
  hasProtectPermission,
  hasRecoveryPermission,
  hasBackupPermission,
  getLabelList
} from 'app/shared';
import {
  filter,
  each,
  size,
  map as __map,
  isEmpty,
  cloneDeep,
  clone,
  values,
  assign,
  trim,
  isNil,
  includes,
  mapValues,
  omit,
  first,
  some
} from 'lodash';
import { map } from 'rxjs/operators';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';

@Component({
  selector: 'aui-database',
  templateUrl: './database.component.html',
  styles: ['']
})
export class DatabaseComponent implements OnInit, AfterViewInit {
  selectionData;
  opts: ProButton[];
  optsConfig: ProButton[];
  tableConfig: TableConfig;
  tableData: TableData;
  searchKey: string;

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable') dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private slaService: SlaService,
    private messageService: MessageService,
    private protectService: ProtectService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private takeManualBackupService: TakeManualBackupService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }
  initConfig() {
    const optItems: { [key: string]: ProButton } = {
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
        onClick: data => this.protect(data, ProtectResourceAction.Create)
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
        divide: true,
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
            .removeProtection(__map(data, 'uuid'), __map(data, 'name'))
            .subscribe(() => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
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
            .activeProtection(__map(data, 'uuid'))
            .subscribe(() => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            });
        }
      },
      deactiveProtection: {
        id: 'deactiveProtection',
        divide: true,
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
            .deactiveProtection(__map(data, 'uuid'), __map(data, 'name'))
            .subscribe(() => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
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
        onClick: data =>
          this.getResourceDetail({
            ...data[0],
            activeId: 'copydata',
            datePickerMode: DATE_PICKER_MODE.DATE
          })
      },
      manualBackup: {
        id: 'manualBackup',
        divide: true,
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

    this.opts = cloneDeep(getPermissionMenuItem(values(optItems)));

    each(this.opts, item => {
      if (item.disabledTips) {
        item.disabledTips = '';
      }
    });

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
        sort: true,
        name: this.i18n.get('common_name_label'),
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: (data: any) => {
              this.getResourceDetail(data);
            }
          }
        }
      },
      {
        key: 'parentName',
        name: this.i18n.get('insight_report_belong_cluster_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
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
            click: (data: { sla_id: any; sla_name: any }) => {
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
        key: 'protection_status',
        name: this.i18n.get('protection_protected_status_label'),
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
            items: this.opts
          }
        }
      }
    ];

    const removeProtectionBtn = clone(optItems.removeProtection);
    removeProtectionBtn.divide = false;
    const deactiveBtn = clone(optItems.deactiveProtection);
    deactiveBtn.divide = false;
    this.optsConfig = getPermissionMenuItem([
      optItems.protect,
      removeProtectionBtn,
      optItems.activeProtection,
      deactiveBtn,
      assign(cloneDeep(optItems.manualBackup), {
        divide: false
      }),
      optItems.addTag,
      optItems.removeTag
    ]);

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
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (_, item) => {
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

  protect(
    datas,
    action: ProtectResourceAction,
    _header?: string,
    _refreshData?
  ) {
    const data = size(datas) > 1 ? datas : first(datas);
    assign(data, { sub_type: DataMap.Resource_Type.ClickHouseDatabase.value });
    this.protectService.openProtectModal(
      ProtectResourceCategory.ClickHouse,
      action,
      {
        width: 780,
        data,
        onOK: () => {
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.dataTable.fetchData();
        },
        restoreWidth: params => this.getResourceDetail(params)
      }
    );
  }

  search() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(this.searchKey)
        }
      ]
    });
    this.dataTable.fetchData();
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
      this.takeManualBackupService.batchExecute(datas, () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      });
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

  getResourceDetail(res) {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: res.uuid
      })
      .subscribe(item => {
        if (!item || isEmpty(item)) {
          this.messageService.error(
            this.i18n.get('common_resource_not_exist_label'),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'resNotExistMesageKey'
            }
          );
          return;
        }
        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'slaDetailModalKey'
          )
        ) {
          this.drawModalService.destroyModal('slaDetailModalKey');
        }
        extendSlaInfo(item);
        this.detailService.openDetailModal(
          DataMap.Resource_Type.ClickHouseDatabase.value,
          {
            data: assign(
              omit(cloneDeep(res), ['sla_id']),
              item,
              {
                optItems: getTableOptsItems(
                  cloneDeep(this.opts),
                  assign(omit(cloneDeep(res), ['sla_id']), item),
                  this
                )
              },
              {
                optItemsFn: v => {
                  return getTableOptsItems(cloneDeep(this.opts), v, this);
                }
              }
            )
          }
        );
      });
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading: !isNil(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType: DataMap.Resource_Type.ClickHouse.value,
      type: ResourceType.DATABASE
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
              sub_type: item.subType,
              parentName: item.environment?.name,
              auth_status: DataMap.Verify_Status.true.value,
              showLabelList: showList,
              hoverLabelList: hoverList
            });
            extendSlaInfo(item);
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
}
