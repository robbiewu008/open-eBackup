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
import { CreateFilesetComponent } from 'app/business/protection/host-app/fileset/create-fileset/create-fileset.component';
import {
  CommonConsts,
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
  ProtectResourceCategory,
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
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  filter as _filter,
  find,
  first,
  includes,
  isEmpty,
  isUndefined,
  map as _map,
  mapValues,
  reject,
  size,
  toString,
  trim,
  some
} from 'lodash';
import { combineLatest } from 'rxjs';
import { map } from 'rxjs/operators';
import { MessageService } from '@iux/live';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';

@Component({
  selector: 'aui-filesets',
  templateUrl: './filesets.component.html',
  styleUrls: ['./filesets.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class FilesetsComponent implements OnInit, AfterViewInit {
  name;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  optItems = [];
  selectionData = [];

  groupCommon = GROUP_COMMON;

  @Input() data: any;
  @Input() isClusterDetail: boolean;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private protectService: ProtectService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    public batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private takeManualBackupService: TakeManualBackupService,
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
    const opts: ProButton[] = [
      {
        id: 'create',
        type: 'primary',
        permission: RoleOperationMap.manageResource,
        label: this.i18n.get('common_create_label'),
        onClick: () => {
          this.createFileset();
        }
      },
      {
        id: 'protect',
        permission: OperateItems.Protection,
        label: this.i18n.get('common_protect_label'),
        onClick: data => {
          this.protect(data, ProtectResourceAction.Create);
        },
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
            size(
              _filter(data, val => {
                return val.environment_os_type === data[0].environment_os_type;
              })
            ) !== size(data)
          );
        }
      },
      {
        id: 'modifyProtection',
        permission: OperateItems.ModifyFilesetProtection,
        label: this.i18n.get('protection_modify_protection_label'),
        onClick: data => {
          this.protect(data, ProtectResourceAction.Modify);
        },
        disableCheck: data => {
          return !!find(
            data,
            item => isEmpty(item.sla_id) || !hasProtectPermission(item)
          );
        }
      },
      {
        id: 'removeProtection',
        divide: true,
        permission: OperateItems.RemoveFilesetProtection,
        label: this.i18n.get('protection_remove_protection_label'),
        onClick: data => {
          this.protectService
            .removeProtection(_map(data, 'uuid'), _map(data, 'name'))
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            });
        },
        disableCheck: data => {
          return (
            !size(data) ||
            !!find(
              data,
              item =>
                (isEmpty(item.sla_id) &&
                  item.protection_status !==
                    DataMap.Protection_Status.protected.value) ||
                !hasProtectPermission(item)
            )
          );
        }
      },
      {
        id: 'active',
        permission: OperateItems.ActivateProtection,
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
                this.getResourceDetail(data);
              }
            });
        },
        disableCheck: data => {
          return (
            !size(data) ||
            !isUndefined(
              find(
                data,
                d => !(d.sla_id && !d.sla_status && hasProtectPermission(d))
              )
            )
          );
        }
      },
      {
        id: 'deactive',
        divide: true,
        permission: OperateItems.DeactivateProtection,
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
                this.getResourceDetail(data);
              }
            });
        },
        disableCheck: data => {
          return (
            !size(data) ||
            !isUndefined(
              find(
                data,
                d => !(d.sla_id && d.sla_status && hasProtectPermission(d))
              )
            )
          );
        }
      },
      {
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
      {
        id: 'manualBackup',
        divide: true,
        permission: OperateItems.ManuallyBackFileset,
        label: this.i18n.get('common_manual_backup_label'),
        onClick: data => {
          this.manualBackup(data);
        },
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return !isEmpty(val.sla_id) && hasBackupPermission(val);
              })
            ) !== size(data) || !size(data)
          );
        }
      },
      {
        id: 'modify',
        permission: OperateItems.ModifyHostFileset,
        label: this.i18n.get('common_modify_label'),
        onClick: data => {
          this.modifyFileset(first(data));
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        }
      },
      {
        id: 'delete',
        permission: OperateItems.DeleteHostFileset,
        label: this.i18n.get('common_delete_label'),
        onClick: data => {
          this.deleteResource(data);
        },
        disableCheck: data => {
          return (
            !size(data) ||
            !isUndefined(
              find(data, item => {
                return (
                  !isEmpty(item.sla_id) ||
                  item.protection_status ===
                    DataMap.Protection_Status.creating.value ||
                  !hasResourcePermission(item)
                );
              })
            )
          );
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
    this.optsConfig = _filter(cloneDeep(optsItem), item => {
      if (includes(['removeProtection', 'deactive', 'manualBackup'], item.id)) {
        item.divide = false;
      }
      return includes(
        [
          'create',
          'protect',
          'delete',
          'active',
          'deactive',
          'removeProtection',
          'manualBackup',
          'addTag',
          'removeTag'
        ],
        item.id
      );
    });

    this.optItems = cloneDeep(reject(optsItem, { id: 'create' }));

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
        cellRender: !this.isClusterDetail
          ? {
              type: 'text',
              config: {
                id: 'outerClosable',
                iconPos: 'flow-text',
                click: data => {
                  this.getResourceDetail(data);
                }
              }
            }
          : null
      },
      {
        key: 'environment_name',
        name: this.i18n.get('protection_cluster_label'),
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
            items: this.optItems
          }
        }
      }
    ];

    if (this.isClusterDetail) {
      cols = _filter(cols, col => {
        return [
          'name',
          'sla_name',
          'sla_compliance',
          'sla_status',
          'protection_status'
        ].includes(col.key);
      });
    }

    this.tableConfig = {
      pagination: {
        winTablePagination: this.isClusterDetail,
        showPageSizeOptions: !this.isClusterDetail,
        mode: !this.isClusterDetail ? 'default' : 'simple'
      },
      table: {
        size: !this.isClusterDetail ? 'default' : 'small',
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'uuid',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: !this.isClusterDetail
        },
        scrollFixed: true,
        colDisplayControl: !this.isClusterDetail
          ? {
              ignoringColsType: 'hide'
            }
          : false,
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

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    } as any;

    if (!isEmpty(filters.conditions_v2)) {
      const conditions_v2 = JSON.parse(filters.conditions_v2);
      if (conditions_v2.environment_name) {
        conditions_v2['environment'] = {
          name: conditions_v2.environment_name
        };
        delete conditions_v2.environment_name;
      }
      if (conditions_v2.labelList) {
        conditions_v2['labelCondition'] = {
          labelName: conditions_v2.labelList[1]
        };
        delete conditions_v2.labelList;
      }

      assign(params, {
        conditions: JSON.stringify({
          ...conditions_v2,
          subType: DataMap.Resource_Type.HDFSFileset.value
        })
      });
    } else {
      assign(params, {
        conditions: JSON.stringify({
          subType: DataMap.Resource_Type.HDFSFileset.value
        })
      });
    }

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    if (this.isClusterDetail) {
      assign(params, {
        conditions: JSON.stringify({
          ...(params?.conditions ? JSON.parse(params?.conditions) : {}),
          environment: {
            uuid: this.data?.uuid
          }
        })
      });
    }

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      this.tableData = {
        total: res.totalCount,
        data: _map(res.records, (item: any) => {
          // 获取标签数据
          const { showList, hoverList } = getLabelList(item);
          assign(item, {
            showLabelList: showList,
            hoverLabelList: hoverList
          });
          item['sub_type'] = item.subType;
          item['environment_uuid'] = item.environment?.uuid;
          item['environment_name'] = item.environment?.name;
          item['environment_endpoint'] = item.environment?.endpoint;
          extendSlaInfo(item);
          return item;
        })
      };
      this.cdr.detectChanges();
    });
  }

  getResourceDetail(params) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          uuid: params.uuid
        })
      })
      .pipe(
        map(res => {
          return first(res.records) || {};
        })
      )
      .subscribe((item: any) => {
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

        if (params.activeId) {
          item['activeId'] = params.activeId;
          item['datePickerMode'] = params.datePickerMode;
        }

        extendSlaInfo(item);
        this.detailService.openDetailModal(
          DataMap.Resource_Type.HDFSFileset.value,
          {
            data: assign(
              item,
              {
                ...item,
                sub_type: item.subType,
                environment_uuid: item.environment?.uuid,
                environment_name: item.environment?.name,
                environment_endpoint: item.environment?.endpoint
              },
              {
                optItems: getTableOptsItems(
                  cloneDeep(this.optItems),
                  {
                    ...item,
                    sub_type: item.subType,
                    environment_uuid: item.environment?.uuid,
                    environment_name: item.environment?.name,
                    environment_endpoint: item.environment?.endpoint
                  },
                  this
                )
              },
              {
                optItemsFn: v => {
                  return getTableOptsItems(cloneDeep(this.optItems), v, this);
                }
              }
            )
          }
        );
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

  createFileset(item?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_create_label'),
      lvContent: CreateFilesetComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.largeWidth + 80,
      lvComponentParams: {
        sub_type: DataMap.Resource_Type.HDFS.value
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateFilesetComponent;
        const modalIns = modal.getInstance();
        const combined: any = combineLatest(
          content.formGroup.statusChanges,
          content.fileValid$
        );
        combined.subscribe(latestValues => {
          const [formGroupStatus, fileValid] = latestValues;
          modalIns.lvOkDisabled = !(formGroupStatus === 'VALID' && fileValid);
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateFilesetComponent;
          content.createHDFS().subscribe({
            next: () => {
              resolve(true);
              this.dataTable.fetchData();
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  modifyFileset(item) {
    if (item.extendInfo.paths) {
      item.extendInfo.paths = JSON.stringify(
        _map(JSON.parse(item.extendInfo.paths), 'name')
      );
    }
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_modify_label'),
      lvContent: CreateFilesetComponent,
      lvOkDisabled: false,
      lvComponentParams: {
        rowItem: item,
        sub_type: DataMap.Resource_Type.HDFS.value
      },
      lvWidth: MODAL_COMMON.largeWidth + 80,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateFilesetComponent;
        const modalIns = modal.getInstance();
        const combined: any = combineLatest(
          content.formGroup.statusChanges,
          content.fileValid$
        );
        combined.subscribe(latestValues => {
          const [formGroupStatus, fileValid] = latestValues;
          modalIns.lvOkDisabled = !(formGroupStatus === 'VALID' && fileValid);
        });
        content.formGroup.get('name').updateValueAndValidity();
        content.fileValid$.next(true);
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateFilesetComponent;
          content.modifyHDFS().subscribe({
            next: () => {
              resolve(true);
              if (
                !isEmpty(item) &&
                includes(
                  mapValues(this.drawModalService.modals, 'key'),
                  'detail-modal'
                )
              ) {
                this.getResourceDetail(content.rowItem);
              } else {
                this.dataTable.fetchData();
              }
            },
            error: () => resolve(false)
          });
        });
      },
      lvCancel: modal => {
        const content = modal.getContentComponent() as CreateFilesetComponent;
        if (
          !isEmpty(item) &&
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'detail-modal'
          )
        ) {
          this.getResourceDetail(content.rowItem);
        }
      }
    });
  }

  deleteResource(data) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_dorado_system_delete_label', [
        toString(_map(data, 'name'))
      ]),
      onOK: () => {
        if (data.length === 1) {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: data[0].uuid
            })
            .subscribe(res => {
              this.dataTable.fetchData();
            });
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.protectedResourceApiService.DeleteResource({
                resourceId: item.uuid,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            data,
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
      this.takeManualBackupService.execute(datas[0], () =>
        this.dataTable?.fetchData()
      );
    }
  }

  protect(datas, action: ProtectResourceAction) {
    const data = size(datas) > 1 ? datas : datas[0];
    this.protectService.openProtectModal(ProtectResourceCategory.HDFS, action, {
      width: 780,
      data,
      onOK: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      },
      restoreWidth: params => {
        this.getResourceDetail(params);
      }
    });
  }
}
