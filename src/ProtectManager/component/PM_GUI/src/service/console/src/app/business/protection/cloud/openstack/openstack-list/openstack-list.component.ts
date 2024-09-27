import {
  AfterViewInit,
  Component,
  EventEmitter,
  Input,
  OnChanges,
  OnInit,
  Output,
  SimpleChanges,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import {
  ColorConsts,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  DATE_PICKER_MODE,
  extendSlaInfo,
  getLabelList,
  getPermissionMenuItem,
  getTableOptsItems,
  GROUP_COMMON,
  hasBackupPermission,
  hasProtectPermission,
  hasRecoveryPermission,
  hasResourcePermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectedResourceApiService,
  ProtectResourceAction,
  ResourceType
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
import { ProtectService } from 'app/shared/services/protect.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  filter,
  includes,
  isEmpty,
  isUndefined,
  reject,
  size,
  map as _map,
  cloneDeep,
  trim,
  mapValues,
  find,
  some
} from 'lodash';
import { SetQuotaComponent } from './set-quota/set-quota.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';

@Component({
  selector: 'aui-openstack-list',
  templateUrl: './openstack-list.component.html',
  styleUrls: ['./openstack-list.component.less']
})
export class OpenstackListComponent
  implements OnInit, OnChanges, AfterViewInit {
  @Input() resType: string;
  @Input() treeSelection: any;
  @Output() updateTable = new EventEmitter();

  name;

  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData: any = [];
  optItems = [];

  lessThanLabel = this.i18n.get('common_less_than_label');
  progressBarColor = [[0, ColorConsts.NORMAL]];
  currentDetailUuid;

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('statusTpl', { static: true })
  statusTpl: TemplateRef<any>;
  @ViewChild('groupTipTpl', { static: true })
  groupTipTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private cookieService: CookieService,
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

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnChanges(changes: SimpleChanges): void {
    if (changes.treeSelection && this.dataTable) {
      this.dataTable.stopPolling();
      this.dataTable.fetchData();
      this.selectionData = [];
      this.dataTable.setSelections([]);
    }
  }

  ngOnInit(): void {
    this.virtualScroll.getScrollParam(400);
    this.initConfig();
  }

  disableProtectStatus(data): boolean {
    return !isEmpty(
      find(
        data,
        item =>
          item.subType === DataMap.Resource_Type.openStackCloudServer.value &&
          !includes(
            [
              DataMap.HCS_Host_LinkStatus.normal.value,
              DataMap.HCS_Host_LinkStatus.offline.value,
              DataMap.HCS_Host_LinkStatus.suspended.value
            ],
            item.status
          )
      )
    );
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'protect',
        label: this.i18n.get('common_protect_label'),
        permission: OperateItems.ProtectVM,
        disableCheck: data => {
          return (
            !size(data) ||
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
            ) !== size(data) ||
            !isUndefined(find(data, v => v.inGroup)) ||
            this.disableProtectStatus(data)
          );
        },
        onClick: data => this.protect(data, ProtectResourceAction.Create)
      },
      {
        id: 'modifyProtection',
        label: this.i18n.get('protection_modify_protection_label'),
        permission: OperateItems.ModifyVMProtection,
        disableCheck: data => {
          return (
            !size(data) ||
            size(
              filter(data, val => {
                return !isEmpty(val.sla_id) && hasProtectPermission(val);
              })
            ) !== size(data) ||
            !isUndefined(find(data, v => v.inGroup)) ||
            this.disableProtectStatus(data)
          );
        },
        onClick: data => this.protect(data, ProtectResourceAction.Modify)
      },
      {
        id: 'removeProtection',
        label: this.i18n.get('protection_remove_protection_label'),
        divide: true,
        permission: OperateItems.RemoveVMProtection,
        disableCheck: data => {
          return (
            !size(data) ||
            size(
              filter(data, val => {
                return (
                  (!isEmpty(val.sla_id) ||
                    val.protection_status ===
                      DataMap.Protection_Status.protected.value) &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) ||
            !isUndefined(find(data, v => v.inGroup))
          );
        },
        onClick: data => {
          this.protectService
            .removeProtection(_map(data, 'uuid'), _map(data, 'name'))
            .subscribe(() => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      },
      {
        id: 'activeProtection',
        label: this.i18n.get('protection_active_protection_label'),
        permission: OperateItems.ActivateVMProtection,
        disableCheck: data => {
          return (
            !size(data) ||
            size(
              filter(data, val => {
                return (
                  !isEmpty(val.sla_id) &&
                  !val.sla_status &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) ||
            !isUndefined(find(data, v => v.inGroup)) ||
            this.disableProtectStatus(data)
          );
        },
        onClick: data => {
          this.protectService
            .activeProtection(_map(data, 'uuid'))
            .subscribe(() => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      },
      {
        id: 'deactiveProtection',
        label: this.i18n.get('protection_deactive_protection_label'),
        divide: true,
        permission: OperateItems.DeactivateVMProtection,
        disableCheck: data => {
          return (
            !size(data) ||
            size(
              filter(data, val => {
                return (
                  !isEmpty(val.sla_id) &&
                  val.sla_status &&
                  hasProtectPermission(val)
                );
              })
            ) !== size(data) ||
            !isUndefined(find(data, v => v.inGroup)) ||
            this.disableProtectStatus(data)
          );
        },
        onClick: data => {
          this.protectService
            .deactiveProtection(_map(data, 'uuid'), _map(data, 'name'))
            .subscribe(() => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      },
      {
        id: 'recovery',
        label: this.i18n.get('common_restore_label'),
        permission: OperateItems.RestoreCopy,
        disableCheck: data => {
          return (
            !size(data) || some(data, item => !hasRecoveryPermission(item))
          );
        },
        onClick: data =>
          this.getResourceDetail({
            ...data[0],
            activeId: 'copydata',
            datePickerMode: DATE_PICKER_MODE.DATE
          })
      },
      {
        id: 'manualBackup',
        label: this.i18n.get('common_manual_backup_label'),
        divide: true,
        permission: OperateItems.ManuallyBackVM,
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  !isEmpty(val.sla_id) &&
                  ((this.resType === ResourceType.OpenStackProject &&
                    val.extendInfo?.cloudServerCount !== '0') ||
                    this.resType !== ResourceType.OpenStackProject) &&
                  hasBackupPermission(val)
                );
              })
            ) !== size(data) ||
            !size(data) ||
            this.disableProtectStatus(data)
          );
        },
        onClick: data => this.manualBackup(data)
      },
      {
        id: 'rescan',
        label: this.i18n.get('common_rescan_label'),
        permission: OperateItems.ScanHCSProject,
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: ([data]) => {
          this.protectedResourceApiService
            .ScanProtectedResources({
              resId: data.uuid
            })
            .subscribe(() => this.dataTable.fetchData());
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
    this.optsConfig = getPermissionMenuItem(
      filter(cloneDeep(opts), item => {
        if (item.id === 'manualBackup') {
          delete item.divide;
        }
        return includes(
          [
            'protect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'manualBackup',
            'addTag',
            'removeTag'
          ],
          item.id
        );
      })
    );

    this.optItems =
      this.resType === ResourceType.OpenStackProject
        ? getPermissionMenuItem(reject(opts, opt => opt.id === 'recovery'))
        : getPermissionMenuItem(
            reject(opts, opt =>
              includes(
                ['resourceAuth', 'resourceReclaiming', 'rescan', 'setQuota'],
                opt.id
              )
            )
          );

    let cols: TableCols[] = [
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
              this.getResourceDetail(data);
              this.currentDetailUuid = data.uuid;
            }
          }
        }
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        cellRender: this.statusTpl
      },
      {
        key: 'path',
        name: this.i18n.get('common_location_label'),
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
        key: 'resourceGroupName',
        name: this.i18n.get('protection_cloud_group_label'),
        thExtra: this.groupTipTpl
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
            items: getPermissionMenuItem(this.optItems, this.cookieService.role)
          }
        }
      }
    ];

    if (this.resType === ResourceType.OpenStackProject) {
      cols = reject(cols, item =>
        includes(['status', 'location', 'resourceGroupName'], item.key)
      );
    } else if (this.resType === ResourceType.OpenStackCloudServer) {
      cols = reject(cols, item =>
        includes(['quota', 'authorizedUser'], item.key)
      );
    } else {
      cols = reject(cols, item =>
        includes(['quota', 'authorizedUser', 'resourceGroupName'], item.key)
      );
    }

    this.tableConfig = {
      table: {
        async: true,
        columns: cols,
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
      },
      pagination: {
        winTablePagination: true
      }
    };
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data ? data : this.selectionData,
      onOk: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      }
    });
  }

  removeTag(data) {
    this.setResourceTagService.setTag({
      isAdd: false,
      rowDatas: data ? data : this.selectionData,
      onOk: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      }
    });
  }

  clearTable() {
    this.tableData = {
      data: [],
      total: 0
    };
    this.updateTable.emit({ resType: this.resType, total: 0 });
  }

  getData(filters: Filters, args: any) {
    if (!this.treeSelection) {
      if (!isEmpty(this.tableData?.data)) {
        this.clearTable();
      }
      return;
    }
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType: [this.resType]
    };

    if (this.treeSelection.type !== ResourceType.OpenStack) {
      assign(defaultConditions, {
        path: [['=~'], this.treeSelection?.path + '/']
      });
    }

    if (this.treeSelection?.rootUuid) {
      assign(defaultConditions, {
        rootUuid: this.treeSelection?.rootUuid
      });
    }

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
      if (conditionsTemp.cluster) {
        assign(conditionsTemp, {
          environment: {
            name: conditionsTemp.cluster
          }
        });
        delete conditionsTemp.cluster;
      }
      if (conditionsTemp.endpoint) {
        assign(conditionsTemp, {
          environment: {
            endpoint: conditionsTemp.endpoint
          }
        });
        delete conditionsTemp.endpoint;
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
        extendSlaInfo(item);
        assign(item, {
          sub_type: item.subType,
          showLabelList: showList,
          hoverLabelList: hoverList
        });
        if (this.resType === ResourceType.OpenStackCloudServer) {
          assign(item, {
            status: item.extendInfo?.status
          });
        }
        if (this.resType === DataMap.Resource_Type.openStackProject.value) {
          assign(item, {
            cloudServerCount: item.extendInfo?.cloudServerCount || 0
          });
        }
      });

      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      if (filters.filterChangeFlag) {
        this.selectionData = [];
        this.dataTable.setSelections(this.selectionData);
        assign(this.dataTable.filterMap, {
          filterChangeFlag: false
        });
      }
      if (isEmpty(filters.conditions_v2)) {
        this.updateTable.emit({ resType: this.resType, total: res.totalCount });
      }
      if (
        !args?.isAutoPolling &&
        includes(
          mapValues(this.drawModalService.modals, 'key'),
          'detail-modal'
        ) &&
        find(res.records, { uuid: this.currentDetailUuid })
      ) {
        this.getResourceDetail(
          find(res.records, { uuid: this.currentDetailUuid })
        );
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
    this.dataTable.fetchData();
    this.selectionData = [];
    this.dataTable.setSelections([]);
  }

  getResourceDetail(data) {
    this.detailService.openDetailModal(this.resType, {
      data: assign(
        cloneDeep(data),
        {
          optItems: getTableOptsItems(cloneDeep(this.optItems), data, this)
        },
        {
          optItemsFn: v => {
            return getTableOptsItems(cloneDeep(this.optItems), v, this);
          }
        }
      )
    });
  }

  protect(datas, action: ProtectResourceAction) {
    if (
      action === ProtectResourceAction.Create &&
      this.resType === ResourceType.OpenStackCloudServer
    ) {
      const hasEmptyDisk = !isEmpty(
        filter(datas, item => {
          return isEmpty(JSON.parse(item.extendInfo?.volInfo || '[]'));
        })
      );
      if (hasEmptyDisk) {
        this.messageService.error(
          this.i18n.get('common_select_hcs_disk_protect_label'),
          {
            lvMessageKey: 'openstack_no_disk_protect',
            lvShowCloseButton: true
          }
        );
        return;
      }
    }
    if (action === ProtectResourceAction.Create) {
      each(datas, item => {
        assign(item, {
          treeSelection: cloneDeep(this.treeSelection)
        });
      });
    }
    this.protectService.openProtectModal(this.resType, action, {
      width: 780,
      data: datas,
      onOK: () => {
        this.dataTable.fetchData();
        this.selectionData = [];
        this.dataTable.setSelections([]);
      }
    });
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
        this.dataTable.fetchData();
        this.selectionData = [];
        this.dataTable.setSelections([]);
      });
    } else {
      assign(datas[0], {
        host_ip: datas[0].environment_endpoint,
        resource_id: datas[0].uuid,
        resource_type: datas[0].sub_type
      });
      this.takeManualBackupService.execute(datas[0], () => {
        this.dataTable?.fetchData();
        this.selectionData = [];
        this.dataTable?.setSelections([]);
      });
    }
  }

  setQuota(rowData) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'set-project-quota',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: this.i18n.get('protection_set_quota_label'),
        lvContent: SetQuotaComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as SetQuotaComponent;
          content.formGroup.statusChanges.subscribe(res => {
            modal.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as SetQuotaComponent;
          });
        }
      })
    );
  }
}
