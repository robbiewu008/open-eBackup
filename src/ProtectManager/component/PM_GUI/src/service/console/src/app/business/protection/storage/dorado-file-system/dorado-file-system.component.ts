import {
  AfterViewInit,
  ChangeDetectionStrategy,
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
  I18NService,
  OperateItems,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ProtectResourceAction
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
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  filter as _filter,
  map as _map,
  assign,
  clone,
  cloneDeep,
  each,
  every,
  first,
  includes,
  isEmpty,
  isUndefined,
  mapValues,
  omit,
  reject,
  size,
  some,
  trim,
  values
} from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-dorado-file-system',
  templateUrl: './dorado-file-system.component.html',
  styleUrls: ['./dorado-file-system.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class DoradoFileSystemComponent implements OnInit, AfterViewInit {
  name;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  optItems = [];
  rescanDisabled = false;
  oceanStorDoradoV6Uuid;
  isHyperdetect = includes(
    [DataMap.Deploy_Type.hyperdetect.value],
    this.i18n.get('deploy_type')
  );
  isCloudBackup = includes(
    [
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value
    ],
    this.i18n.get('deploy_type')
  );

  groupCommon = GROUP_COMMON;

  @Input() header = this.i18n.get('common_nas_file_systems_label');
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('storageDeviceTpl', { static: true })
  storageDeviceTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private protectService: ProtectService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    public cookieService: CookieService,
    private cdr: ChangeDetectorRef,
    private detailService: ResourceDetailService,
    private infoMessageService: InfoMessageService,
    private takeManualBackupService: TakeManualBackupService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
    this.getOceanStorDoradoV6();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(340);
    this.initConfig();
  }

  onChange() {
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  checkProtectDisable(data) {
    const filterData = _map(
      reject(data, item => {
        return (
          item.extendInfo?.protocol ===
          DataMap.NasFileSystem_Protocol.nfs_cifs.value
        );
      }),
      'extendInfo.protocol'
    );
    return (
      !this.cookieService.isCloudBackup &&
      !every(
        filterData,
        value => value === DataMap.NasFileSystem_Protocol.nfs.value
      ) &&
      !every(
        filterData,
        value => value === DataMap.NasFileSystem_Protocol.cifs.value
      ) &&
      !every(
        filterData,
        value => value === DataMap.NasFileSystem_Protocol.none.value
      ) &&
      !every(
        filterData,
        value => value === DataMap.NasFileSystem_Protocol.ndmp.value
      )
    );
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      protect: {
        id: 'protect',
        type: this.cookieService.isCloudBackup ? 'default' : 'primary',
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
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
            !size(data) ||
            this.checkProtectDisable(data)
          );
        },
        permission: OperateItems.Protection,
        label: this.i18n.get('common_protect_label'),
        onClick: data => this.protect(data, ProtectResourceAction.Create)
      },
      modifyProtect: {
        id: 'modifyProtect',
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return !isEmpty(val.sla_id) && hasProtectPermission(val);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ModifyProtection,
        label: this.i18n.get('common_resource_protection_modify_label'),
        onClick: data => this.protect(data, ProtectResourceAction.Modify)
      },
      removeProtection: {
        id: 'removeProtection',
        divide: true,
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
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
        permission: OperateItems.RemoveProtection,
        label: this.i18n.get('protection_remove_protection_label'),
        onClick: data => {
          this.protectService
            .removeProtection(_map(data, 'uuid'), _map(data, 'name'))
            .subscribe(res => {
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
              _filter(data, val => {
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
        disabledTips: this.i18n.get(
          'protection_partial_resources_active_label'
        ),
        label: this.i18n.get('protection_active_protection_label'),
        onClick: data => {
          this.protectService
            .activeProtection(_map(data, 'uuid'))
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
              if (
                includes(
                  mapValues(this.drawModalService.modals, 'key'),
                  'detail-modal'
                ) &&
                size(data) === 1
              ) {
                this.getResourceDetail(first(data));
              }
            });
        }
      },
      deactiveProtection: {
        id: 'deactiveProtection',
        divide: true,
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
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
        disabledTips: this.i18n.get(
          'protection_partial_resources_deactive_label'
        ),
        label: this.i18n.get('protection_deactive_protection_label'),
        onClick: data => {
          this.protectService
            .deactiveProtection(_map(data, 'uuid'), _map(data, 'name'))
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
              if (
                includes(
                  mapValues(this.drawModalService.modals, 'key'),
                  'detail-modal'
                ) &&
                size(data) === 1
              ) {
                this.getResourceDetail(first(data));
              }
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
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
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

    this.optItems = cloneDeep(getPermissionMenuItem(values(opts)));
    each(this.optItems, item => {
      if (item.disabledTips) {
        item.disabledTips = '';
      }
    });

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
            click: data => this.getResourceDetail(data)
          }
        }
      },
      {
        key: 'parentName',
        name: this.i18n.get('protection_storage_device_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.storageDeviceTpl
      },
      {
        key: 'protocol',
        name: this.i18n.get('explore_share_protocol_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('NasFileSystem_Protocol')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('NasFileSystem_Protocol')
        }
      },
      {
        key: 'tenantName',
        name: this.i18n.get('common_tenant_label'),
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
      this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
    ) {
      cols = reject(cols, v => {
        return includes(['parentName', 'labelList', 'protocol'], v.key);
      });
    }

    this.tableConfig = {
      table: {
        autoPolling: this.isHyperdetect
          ? CommonConsts.TIME_INTERVAL
          : CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'uuid',
        columns: this.cookieService.isCloudBackup
          ? reject(cols, item => item.key === 'protocol')
          : cols,
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

    const removeProtectionBtn = clone(opts.removeProtection);
    removeProtectionBtn.divide = false;
    const deactiveBtn = clone(opts.deactiveProtection);
    deactiveBtn.divide = false;
    this.optsConfig = getPermissionMenuItem([
      opts.protect,
      removeProtectionBtn,
      opts.activeProtection,
      deactiveBtn,
      opts.manualBackup,
      opts.addTag,
      opts.removeTag
    ]);
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

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    let subType = [];
    if (this.cookieService.isCloudBackup) {
      subType = [DataMap.Resource_Type.LocalFileSystem.value];
    } else {
      subType =
        this.i18n.get('deploy_type') === DataMap.Deploy_Type.e6000.value
          ? [DataMap.Resource_Type.ndmp.value]
          : [
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.ndmp.value
            ];
    }
    const defaultConditions = {
      subType: subType
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

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            // 获取标签数据
            const { showList, hoverList } = getLabelList(item);

            assign(item, {
              tenantName: item.extendInfo?.tenantName,
              sub_type: item.subType,
              protocol: item.extendInfo?.protocol,
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
      this.takeManualBackupService.execute(datas[0], () =>
        this.dataTable?.fetchData()
      );
    }
  }

  getOceanStorDoradoV6() {
    if (!this.cookieService.isCloudBackup) {
      return;
    }
    return this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType: 'OceanStorDoradoV6'
        })
      })
      .pipe(
        map(res => {
          return first(res.records)?.uuid;
        })
      )
      .subscribe(uuid => {
        this.oceanStorDoradoV6Uuid = uuid;
      });
  }

  getResourceDetail(params) {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: params.uuid
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
        this.detailService.openDetailModal(item.subType, {
          data: assign(
            omit(cloneDeep(params), ['sla_id']),
            item,
            {
              optItems: getTableOptsItems(
                cloneDeep(this.optItems),
                assign(omit(cloneDeep(params), ['sla_id']), item),
                this
              )
            },
            {
              optItemsFn: v => {
                return getTableOptsItems(cloneDeep(this.optItems), v, this);
              }
            }
          )
        });
      });
  }

  protect(datas, action: ProtectResourceAction, header?: string, refreshData?) {
    const data = size(datas) > 1 ? datas : datas[0];
    this.protectService.openProtectModal(datas[0].sub_type, action, {
      width: 780,
      data,
      onOK: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      },
      restoreWidth: params => this.getResourceDetail(params)
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

  rescan() {
    if (isEmpty(this.oceanStorDoradoV6Uuid)) {
      this.messageService.error(this.i18n.get('1677930263'));
      return;
    }
    this.infoMessageService.create({
      content: this.i18n.get('protection_rescan_info_label'),
      onOK: () => {
        this.protectedResourceApiService
          .ScanProtectedResources({
            resId: this.oceanStorDoradoV6Uuid
          })
          .subscribe(() => this.dataTable.fetchData());
      }
    });
  }
}
