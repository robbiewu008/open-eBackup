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
  InstanceType,
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
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { RegisterService } from 'app/shared/services/register.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  filter as _filter,
  map as _map,
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  get,
  has,
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

export interface TemplateParams {
  activeIndex: string; // 资源类型
  tableCols: string[]; // 基本信息
  tableOpts: string[]; // 基本操作
  registerComponent?: any; // 注册组件
}

@Component({
  selector: 'aui-database-template',
  templateUrl: './database-template.component.html',
  styleUrls: ['./database-template.component.less']
})
export class DatabaseTemplateComponent implements OnInit, AfterViewInit {
  name;
  columns;
  maxItems;
  optsConfig;
  placeholder;
  optItems = [];
  dataMap = DataMap;
  selectionData = [];
  tableData: TableData;
  tableConfig: TableConfig;

  @Input() configParams: TemplateParams;

  groupCommon = GROUP_COMMON;

  @ViewChild('typeTpl', { static: true })
  typeTpl: TemplateRef<any>;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('databaseTypeTpl', { static: true })
  databaseTypeTpl: TemplateRef<any>;
  @ViewChild('sapHanaDbDeployType', { static: true })
  sapHanaDbDeployType: TemplateRef<any>;
  @ViewChild('logBackupTpl', { static: true })
  logBackupTpl: TemplateRef<any>;
  @ViewChild('enableLogBackupTpl', { static: true })
  enableLogBackupTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private protectService: ProtectService,
    private messageService: MessageService,
    private registerService: RegisterService,
    private drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private takeManualBackupService: TakeManualBackupService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngAfterViewInit() {
    this.dataTable?.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(400);
    this.initConfig();
    this.getSearchPlaceholder();
  }

  onChange() {
    this.initConfig();
    this.dataTable?.fetchData();
  }

  getPopoverContent(): string {
    switch (this.configParams?.activeIndex) {
      case DataMap.Resource_Type.lightCloudGaussdbProject.value:
        return this.i18n.get('protection_guide_project_tip_label');
      case DataMap.Resource_Type.informixService.value:
      case DataMap.Resource_Type.OceanBaseCluster.value:
      case DataMap.Resource_Type.tdsqlCluster.value:
      case DataMap.Resource_Type.tidbCluster.value:
        return this.i18n.get('protection_guide_cluster_tip_label');
      case DataMap.Resource_Type.informixInstance.value:
      case DataMap.Resource_Type.tdsqlInstance.value:
      case DataMap.Resource_Type.tdsqlDistributedInstance.value:
        return this.i18n.get('protection_guide_instance_tip_label');
      case DataMap.Resource_Type.OceanBaseTenant.value:
        return this.i18n.get('protection_guide_tenant_set_register_label');
      case DataMap.Resource_Type.volume.value:
        return this.i18n.get('protection_create_volume_tip_label');
      case DataMap.Resource_Type.ObjectSet.value:
        return this.i18n.get('protection_register_object_set_tip_label');
      case DataMap.Resource_Type.commonShare.value:
        return this.i18n.get('protection_create_common_share_tip_label');
      case DataMap.Resource_Type.tidbDatabase.value:
        return this.i18n.get('protection_guide_database_tip_label');
      case DataMap.Resource_Type.tidbTable.value:
        return this.i18n.get('protection_register_tidb_tableset_tip_label');
      default:
        return '';
    }
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      register: {
        id: 'register',
        type: 'primary',
        permission: RoleOperationMap.manageResource,
        label: this.getRegisterLabel(),
        onClick: () => this.register(),
        popoverContent: this.getPopoverContent(),
        popoverShow: USER_GUIDE_CACHE_DATA.active
      },
      protect: {
        id: 'protect',
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
            this.getProtectionLinkCheck(data)
          );
        },
        permission: OperateItems.Protection,
        label: this.i18n.get('common_protect_label'),
        onClick: data => {
          if (
            this.configParams.activeIndex ===
              DataMap.Resource_Type.DWS_Database.value &&
            find(data, item => item.version === '8.0.0')
          ) {
            this.messageService.error(
              this.i18n.get('protection_cluster_version_error_tips_label')
            );
            return;
          }
          if (
            this.configParams.activeIndex ===
              DataMap.Resource_Type.DWS_Schema.value &&
            find(data, item => item?.environment?.version === '8.0.0')
          ) {
            this.messageService.error(
              this.i18n.get('protection_schema_version_error_tips_label')
            );
            return;
          }
          this.protect(data, ProtectResourceAction.Create);
        }
      },
      modifyProtect: {
        id: 'modifyProtect',
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return !isEmpty(val.sla_id) && hasProtectPermission(val);
              })
            ) !== size(data) ||
            !size(data) ||
            this.getProtectionLinkCheck(data)
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
            .subscribe(() => {
              this.selectionData = [];
              this.dataTable?.setSelections([]);
              this.dataTable?.fetchData();
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
              this.dataTable?.setSelections([]);
              this.dataTable?.fetchData();
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
              this.dataTable?.setSelections([]);
              this.dataTable?.fetchData();
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
        divide: true,
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return !isEmpty(val.sla_id) && hasBackupPermission(val);
              })
            ) !== size(data) ||
            !size(data) ||
            (includes(
              [
                DataMap.Resource_Type.tidbCluster.value,
                DataMap.Resource_Type.tidbDatabase.value,
                DataMap.Resource_Type.tidbTable.value,
                DataMap.Resource_Type.tdsqlInstance.value,
                DataMap.Resource_Type.tdsqlDistributedInstance.value,
                DataMap.Resource_Type.ExchangeSingle.value,
                DataMap.Resource_Type.ExchangeGroup.value
              ],
              data[0].subType
            ) &&
              data.some(
                item =>
                  item.linkStatus !==
                  DataMap.resource_LinkStatus_Special.normal.value
              )) ||
            this.getProtectionLinkCheck(data)
          );
        },
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('common_manual_backup_label'),
        onClick: data => {
          this.manualBackup(data);
        }
      },
      rescan: {
        id: 'rescan',
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('common_rescan_label'),
        onClick: data => {
          this.protectedResourceApiService
            .ScanProtectedResources({
              resId: data[0].rootUuid
            })
            .subscribe(res => {
              this.dataTable?.fetchData();
            });
        }
      },
      connectivityTest: {
        id: 'connectivityTest',
        divide: true,
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('protection_connectivity_test_label'),
        onClick: data => {
          this.protectedResourceApiService
            .CheckProtectedResource({ resourceId: data[0].uuid })
            .subscribe(res => {
              this.messageService.success(
                this.i18n.get('job_status_success_label'),
                {
                  lvMessageKey: 'successKey',
                  lvShowCloseButton: true
                }
              );
              this.dataTable?.fetchData();
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
      allowRecovery: {
        id: 'allowRecovery',
        disableCheck: data => {
          return (
            !size(data) ||
            some(
              data,
              item =>
                item.isAllowRestore === 'true' || !hasResourcePermission(item)
            )
          );
        },
        permission: OperateItems.Protection,
        label: this.i18n.get('protection_allow_restore_label'),
        onClick: data => {
          this.setRestoreStatus(data, 'true');
        }
      },
      disableRecovery: {
        id: 'disableRecovery',
        divide: true,
        disableCheck: data => {
          return (
            !size(data) ||
            some(
              data,
              item =>
                item.isAllowRestore === 'false' || !hasResourcePermission(item)
            )
          );
        },
        permission: OperateItems.Protection,
        label: this.i18n.get('protection_disable_restore_label'),
        onClick: data => {
          this.setRestoreStatus(data, 'false');
        }
      },
      modify: {
        id: 'modify',
        permission: OperateItems.RegisterNasShare,
        label: this.i18n.get('common_modify_label'),
        onClick: data => {
          this.register(first(data));
        },
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return (
            size(
              _filter(data, item => {
                return (
                  item['rootUuid'] === first(data)['rootUuid'] &&
                  item['shareMode'] === first(data)['shareMode'] &&
                  hasResourcePermission(item)
                );
              })
            ) !== size(data) || !size(data)
          );
        }
      },
      deleteResource: {
        id: 'deleteResource',
        permission: OperateItems.DeleteResource,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return (
                  isEmpty(val.sla_id) &&
                  val.protection_status !==
                    DataMap.Protection_Status.creating.value &&
                  hasResourcePermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        label: this.i18n.get('common_delete_label'),
        onClick: data => this.deleteRes(data)
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

    // 操作
    // 新增设置标签功能
    this.configParams.tableOpts.push('addTag', 'removeTag');

    this.optItems = cloneDeep(
      getPermissionMenuItem(
        values(
          filter(opts, opt => {
            return (
              includes(this.configParams.tableOpts, opt.id) &&
              !includes(['register'], opt.id)
            );
          })
        )
      )
    );

    each(this.optItems, item => {
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
        name: this.i18n.get('common_name_label'),
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: data => {
              this.getDetail(data);
            }
          }
        },
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'pmAddress',
        name: this.i18n.get('protection_project_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'address',
        name: this.i18n.get('common_address_label')
      },
      {
        key: 'linkStatus',
        name: this.getLinkStatusLabel(),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray(this.getLinkStatusConfig())
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray(this.getLinkStatusConfig())
        }
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('gaussDBInstance')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('gaussDBInstance')
        }
      },
      {
        key: 'subType', // 根据资源subType筛选
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.getResourceSubTypeConfigMap()
        },
        cellRender: {
          type: 'status',
          config: this.getResourceSubTypeConfigMap()
        }
      },
      {
        key: 'instanceStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('airgapDeviceStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('airgapDeviceStatus')
        }
      },
      {
        key: 'logBackup',
        name: this.i18n.get('common_log_backup_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('logBackupStatus')
        },
        cellRender: this.logBackupTpl
      },
      {
        key: 'clusterType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('dbTwoType')
        },
        cellRender: this.typeTpl
      },
      {
        key: 'sapHanaDbType',
        name: this.i18n.get('protection_database_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('saphanaDatabaseType')
        },
        cellRender: this.databaseTypeTpl
      },
      {
        key: 'sapHanaDbDeployType',
        name: this.i18n.get('common_database_deploy_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('saphanaDatabaseDeployType')
        },
        hidden:
          this.configParams?.activeIndex !==
          DataMap.Resource_Type.saphanaDatabase.value,
        cellRender: this.sapHanaDbDeployType
      },
      // 所属环境, 默认为environment.name
      {
        key: 'environmentName',
        name: this.getEnvironmentNameLabel(),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'environmentEndpoint',
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'storageType',
        name: this.i18n.get('protection_object_storage_owned_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('objectStorageType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('objectStorageType')
        }
      },
      {
        key: 'instance',
        name: this.i18n.get('commom_owned_instance_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'parentName',
        name: this.getParentNameLabel(),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'region',
        name: this.i18n.get('Region'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'nodeIpAddress',
        name: includes(
          [DataMap.Resource_Type.goldendbInstance.value],
          this.configParams.activeIndex
        )
          ? this.i18n.get('protection_manage_node_ip_address_label')
          : this.i18n.get('protection_node_ip_address_label')
      },
      {
        // 用于informix
        key: 'databaseType',
        name: this.i18n.get('protection_database_type_label')
      },
      {
        key: 'version',
        name: this.i18n.get('common_version_label')
      },
      {
        key: 'mysql_version',
        name: this.i18n.get('protection_mysql_version_label'),
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
        key: 'isAllowRestore',
        name: this.i18n.get('protection_is_allow_restore_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('copyDataSanclient')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('copyDataSanclient')
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

    // 新增标签列
    this.configParams.tableCols.splice(-1, 0, 'labelList');

    this.columns = cols.filter(col => {
      return includes(this.configParams.tableCols, col.key);
    });

    // 批量操作
    this.optsConfig = cloneDeep(
      getPermissionMenuItem(
        values(
          filter(opts, opt => {
            return (
              includes(this.configParams.tableOpts, opt.id) &&
              !includes(
                [
                  'modifyProtect',
                  'recovery',
                  'rescan',
                  'connectivityTest',
                  'modify'
                ],
                opt.id
              )
            );
          })
        )
      )
    );

    (includes(this.configParams.tableOpts, 'register') &&
      includes(this.configParams.tableOpts, 'protect')) ||
    this.optsConfig?.length <= 2
      ? (this.maxItems = 3)
      : (this.maxItems = 2);

    each(this.optsConfig, item => {
      assign(item, {
        divide: false
      });
    });

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'uuid',
        columns: this.columns,
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

  search() {
    this.dataTable.filterChange({
      caseSensitive: false,
      filterMode: 'contains',
      key: 'name',
      value: trim(this.name)
    });
  }

  getDetail(data) {
    this.detailService.openDetailModal(this.configParams.activeIndex, {
      data: {
        ...data,
        optItems: getTableOptsItems(this.optItems, data, this),
        optItemsFn: v => {
          return getTableOptsItems(this.optItems, v, this);
        }
      }
    });
  }

  register(item?) {
    if (!this.configParams.registerComponent) {
      return;
    }
    const resourceType =
      item?.subType || item?.sub_type || this.configParams.activeIndex;

    this.registerService.register({
      subType: resourceType,
      component: this.configParams.registerComponent,
      refreshData: () => {
        this.dataTable?.fetchData();
      },
      refreshDetail: rowData => {
        this.getResourceDetail(rowData);
      },
      rowData: item
    });
  }

  setRestoreStatus(data, type) {
    this.protectedResourceApiService
      .UpdateAllowRestore({
        updateAllowRestoreRequestBody: {
          isAllowRestore: type,
          resourceIds: data.map(item => {
            return item.uuid;
          })
        },
        akDoException: false
      })
      .subscribe(
        res => {
          this.selectionData = [];
          this.dataTable?.setSelections([]);
          this.dataTable?.fetchData();
        },
        error => {
          this.messageService.error(
            this.i18n.get(
              'protection_database_put_restore_allow_restore_fail_label'
            ),
            {
              lvMessageKey: 'restore_not_support_key1',
              lvShowCloseButton: true
            }
          );
        }
      );
  }

  deleteRes(data) {
    if (size(data) === 1) {
      this.warningMessageService.create({
        content: this.i18n.get('protection_nas_share_delete_label'),
        onOK: () => {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: data[0].uuid
            })
            .subscribe(() => {
              this.selectionData = reject(
                this.dataTable?.getAllSelections(),
                item => {
                  return item.uuid === data[0].uuid;
                }
              );
              this.dataTable?.setSelections(this.selectionData);
              this.dataTable?.fetchData();
              if (
                includes(
                  mapValues(this.drawModalService.modals, 'key'),
                  'detail-modal'
                )
              ) {
                this.drawModalService.destroyModal('detail-modal');
              }
            });
        }
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get('protection_nas_share_delete_label'),
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item => {
              return this.protectedResourceApiService.DeleteResource({
                resourceId: item.uuid,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            cloneDeep(this.selectionData),
            () => {
              this.selectionData = [];
              this.dataTable?.setSelections([]);
              this.dataTable?.fetchData();
            }
          );
        }
      });
    }
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = this.getDefaultConditions();

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.isAllowRestore) {
        if (conditionsTemp.isAllowRestore.length === 3) {
          delete conditionsTemp.isAllowRestore;
        } else if (conditionsTemp.isAllowRestore.includes('false')) {
          conditionsTemp.isAllowRestore[0] = ['!='];
          conditionsTemp.isAllowRestore[1] = 'true';
        }
      }

      if (conditionsTemp.instanceStatus) {
        if (conditionsTemp.instanceStatus.length === 3) {
          delete conditionsTemp.instanceStatus;
        } else if (conditionsTemp.instanceStatus.includes('1')) {
          conditionsTemp.instanceStatus[0] = ['!='];
          conditionsTemp.instanceStatus[1] = '0';
        }
      }
      if (conditionsTemp.labelList) {
        assign(conditionsTemp, {
          labelCondition: {
            labelName: conditionsTemp.labelList[1]
          }
        });
        delete conditionsTemp.labelList;
      }

      // environment中的过滤条件需要单独处理
      const { environmentName, environmentEndpoint, ...rest } = conditionsTemp;
      const environmentCondition = {};
      if (environmentName) {
        assign(environmentCondition, {
          name: environmentName
        });
      }
      if (environmentEndpoint) {
        assign(environmentCondition, {
          endpoint: environmentEndpoint
        });
      }
      if (!isEmpty(environmentCondition)) {
        assign(rest, {
          environment: environmentCondition
        });
      }
      assign(defaultConditions, rest);
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
              showLabelList: showList,
              hoverLabelList: hoverList
            });
            this.formatData(item);
            this.formatCustomData(item);
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

  formatData(item) {
    assign(item, {
      sub_type: item.subType,
      environmentName: item.environment?.name
    });
  }

  getResourceDetail(res) {
    this.protectedResourceApiService
      .ShowResource({ resourceId: res.uuid })
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
        this.detailService.openDetailModal(this.configParams.activeIndex, {
          data: assign(
            omit(cloneDeep(res), ['sla_id', 'sla_name']),
            item,
            {
              optItems: getTableOptsItems(
                cloneDeep(this.optItems),
                assign(omit(cloneDeep(res), ['sla_id']), item),
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
    const data = size(datas) > 1 ? datas : first(datas);
    this.protectService.openProtectModal(this.getProtectType(), action, {
      width: 780,
      data,
      onOK: () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        return this.dataTable?.fetchData();
      },
      restoreWidth: params => this.getResourceDetail(params)
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
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
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

  getProtectionLinkCheck(data) {
    // 部分应用需根据在线状态判断功能按钮
    if (
      includes(
        [
          DataMap.Resource_Type.ExchangeSingle.value,
          DataMap.Resource_Type.ExchangeGroup.value
        ],
        data[0].subType
      ) &&
      some(
        data,
        item =>
          item.linkStatus !== DataMap.resource_LinkStatus_Special.normal.value
      )
    ) {
      return true;
    } else if (
      includes(
        [
          DataMap.Resource_Type.ExchangeEmail.value,
          DataMap.Resource_Type.ExchangeDataBase.value
        ],
        data[0].subType
      ) &&
      some(
        data,
        item =>
          item?.environment?.linkStatus !==
          DataMap.resource_LinkStatus_Special.normal.value
      )
    ) {
      return true;
    } else {
      return false;
    }
  }

  getSearchPlaceholder() {
    switch (this.configParams.activeIndex) {
      case DataMap.Resource_Type.tidb.value:
      case DataMap.Resource_Type.informixService.value:
      case DataMap.Resource_Type.goldendbCluter.value:
      case DataMap.Resource_Type.dbTwoCluster.value:
      case DataMap.Resource_Type.SQLServerCluster.value:
      case DataMap.Resource_Type.DWS_Cluster.value:
      case DataMap.Resource_Type.OceanBaseCluster.value:
        this.placeholder = this.i18n.get('common_search_type_label', [
          this.i18n.get('common_cluster_label')
        ]);
        break;
      case DataMap.Resource_Type.DWS_Schema.value:
        this.placeholder = this.i18n.get('common_search_type_label', [
          this.i18n.get('protection_schema_set_label')
        ]);
        break;
      case DataMap.Resource_Type.DWS_Table.value:
        this.placeholder = this.i18n.get('common_search_type_label', [
          this.i18n.get('protection_table_set_label')
        ]);
        break;
      case DataMap.Resource_Type.lightCloudGaussdbProject.value:
      case DataMap.Resource_Type.gaussdbForOpengaussProject.value:
        this.placeholder = this.i18n.get('common_search_type_label', [
          this.i18n.get('common_project_label')
        ]);
        break;
      case DataMap.Resource_Type.informixInstance.value:
      case DataMap.Resource_Type.lightCloudGaussdbInstance.value:
      case DataMap.Resource_Type.gaussdbForOpengaussInstance.value:
      case DataMap.Resource_Type.goldendbInstance.value:
      case DataMap.Resource_Type.dbTwoInstance.value:
      case DataMap.Resource_Type.SQLServerInstance.value:
        this.placeholder = this.i18n.get('common_search_type_label', [
          this.i18n.get('protection_database_instance_label')
        ]);
        break;
      case DataMap.Resource_Type.SQLServerGroup.value:
        this.placeholder = this.i18n.get('common_search_type_label', [
          this.i18n.get('protection_availability_group_label')
        ]);
        break;
      case DataMap.Resource_Type.SQLServerDatabase.value:
      case DataMap.Resource_Type.DWS_Database.value:
      case DataMap.Resource_Type.dbTwoDatabase.value:
        this.placeholder = this.i18n.get('common_search_type_label', [
          this.i18n.get('common_database_label')
        ]);
        break;
      case DataMap.Resource_Type.dbTwoTableSet.value:
        this.placeholder = this.i18n.get('common_search_type_label', [
          this.i18n.get('protection_table_space_set_label')
        ]);
        break;
      case DataMap.Resource_Type.OceanBaseTenant.value:
        this.placeholder = this.i18n.get('common_search_type_label', [
          this.i18n.get('common_object_label')
        ]);
        break;
      default:
        this.placeholder = this.i18n.get('common_search_type_label', ['']);
    }
  }

  getRegisterLabel() {
    switch (this.configParams.activeIndex) {
      case DataMap.Resource_Type.DWS_Schema.value:
      case DataMap.Resource_Type.DWS_Table.value:
      case DataMap.Resource_Type.dbTwoTableSet.value:
      case DataMap.Resource_Type.volume.value:
      case DataMap.Resource_Type.commonShare.value:
        return this.i18n.get('common_create_label');
      default:
        return this.i18n.get('common_register_label');
    }
  }

  getLinkStatusLabel() {
    switch (this.configParams.activeIndex) {
      case DataMap.Resource_Type.SQLServerClusterInstance.value:
      case DataMap.Resource_Type.SQLServerInstance.value:
        // 认证状态
        return this.i18n.get('common_auth_status_label');
      default:
        // 状态
        return this.i18n.get('common_status_label');
    }
  }

  getLinkStatusConfig() {
    switch (this.configParams.activeIndex) {
      case DataMap.Resource_Type.DWS_Cluster.value:
        return 'gaussDBDWS_Resource_LinkStatus';
      case DataMap.Resource_Type.goldendbInstance.value:
        return 'clickHouse_cluster_status';
      case DataMap.Resource_Type.OceanBaseTenant.value:
        return 'resourceLinkStatusTenantSet';
      default:
        return 'resource_LinkStatus_Special';
    }
  }

  getEnvironmentNameLabel() {
    switch (this.configParams.activeIndex) {
      case DataMap.Resource_Type.ObjectSet.value: {
        return this.i18n.get('protection_object_storage_owned_label');
      }
      case DataMap.Resource_Type.volume.value:
      case DataMap.Resource_Type.ActiveDirectory.value:
        return this.i18n.get('protection_host_name_label');
      case DataMap.Resource_Type.SQLServerClusterInstance.value:
      case DataMap.Resource_Type.SQLServerInstance.value:
      case DataMap.Resource_Type.SQLServerDatabase.value:
      case DataMap.Resource_Type.dbTwoInstance.value:
      case DataMap.Resource_Type.dbTwoDatabase.value:
      case DataMap.Resource_Type.dbTwoTableSet.value:
        // 所属主机/集群
        return this.i18n.get('protection_host_cluster_name_label');
      case DataMap.Resource_Type.ExchangeDataBase.value:
      case DataMap.Resource_Type.ExchangeEmail.value:
        // 所属单机/可用性组
        return this.i18n.get('protection_single_node_system_group_tag_label');
      // 所属实例
      case DataMap.Resource_Type.saphanaDatabase.value:
        return this.i18n.get('commom_owned_instance_label');
      default:
        // 所属集群
        return this.i18n.get('insight_report_belong_cluster_label');
    }
  }

  getParentNameLabel() {
    switch (this.configParams.activeIndex) {
      case DataMap.Resource_Type.DWS_Schema.value:
      case DataMap.Resource_Type.DWS_Table.value:
      case DataMap.Resource_Type.dbTwoTableSet.value:
      case DataMap.Resource_Type.tidbTable.value:
      case DataMap.Resource_Type.ExchangeEmail.value:
        // 所属数据库
        return this.i18n.get('protection_host_database_name_label');
      case DataMap.Resource_Type.lightCloudGaussdbInstance.value:
        // 所属项目
        return this.i18n.get('commom_owned_project_label');
      case DataMap.Resource_Type.SQLServerGroup.value:
      case DataMap.Resource_Type.OceanBaseTenant.value:
      case DataMap.Resource_Type.tidbDatabase.value:
        // 所属集群
        return this.i18n.get('insight_report_belong_cluster_label');
      default:
        // 所属实例
        return this.i18n.get('commom_owned_instance_label');
    }
  }

  getDefaultConditions() {
    switch (this.configParams.activeIndex) {
      case DataMap.Resource_Type.SQLServerInstance.value:
        return {
          subType: [
            DataMap.Resource_Type.SQLServerInstance.value,
            DataMap.Resource_Type.SQLServerClusterInstance.value
          ]
        };
      case DataMap.Resource_Type.SQLServerDatabase.value:
        return {
          subType: DataMap.Resource_Type.SQLServerDatabase.value,
          agId: [['=='], '']
        };
      case DataMap.Resource_Type.dbTwoInstance.value:
        return {
          subType: [
            DataMap.Resource_Type.dbTwoClusterInstance.value,
            DataMap.Resource_Type.dbTwoInstance.value
          ],
          isTopInstance: InstanceType.TopInstance
        };
      case DataMap.Resource_Type.informixInstance.value:
        return {
          subType: [
            DataMap.Resource_Type.informixInstance.value,
            DataMap.Resource_Type.informixClusterInstance.value
          ],
          isTopInstance: InstanceType.TopInstance
        };
      case DataMap.Resource_Type.Exchange.value:
        return {
          subType: [
            DataMap.Resource_Type.ExchangeGroup.value,
            DataMap.Resource_Type.ExchangeSingle.value
          ]
        };
      case DataMap.Resource_Type.saphanaInstance.value:
        return {
          subType: [DataMap.Resource_Type.saphanaInstance.value],
          isTopInstance: InstanceType.TopInstance
        };
      default:
        return {
          subType: this.configParams.activeIndex
        };
    }
  }

  formatCustomData(item: { [key: string]: any }) {
    switch (this.configParams.activeIndex) {
      case DataMap.Resource_Type.ActiveDirectory.value:
        assign(item, {
          environmentEndpoint: item.environment?.endpoint,
          environmentName: item.environment?.name
        });
        break;
      case DataMap.Resource_Type.volume.value:
        assign(item, {
          environmentEndpoint: item.environment?.endpoint
        });
        break;
      case DataMap.Resource_Type.SQLServerInstance.value:
        assign(item, {
          nodeIpAddress:
            item.subType ===
            DataMap.Resource_Type.SQLServerClusterInstance.value
              ? item.path
              : item.environment?.endpoint
        });
        break;
      case DataMap.Resource_Type.dbTwoCluster.value:
        assign(item, {
          clusterType: item?.extendInfo?.clusterType
        });
        break;
      case DataMap.Resource_Type.dbTwoInstance.value:
      case DataMap.Resource_Type.dbTwoClusterInstance.value:
        assign(item, {
          environmentName: has(item, 'environment.extendInfo.clusterType')
            ? item.environment?.name
            : `${item.environment?.name}(${item.environment?.endpoint})`,
          linkStatus: item.extendInfo?.linkStatus
        });
        break;
      case DataMap.Resource_Type.dbTwoDatabase.value:
        assign(item, {
          environmentName: has(item, 'environment.extendInfo.clusterType')
            ? item.environment?.name
            : `${item.environment?.name}(${item.environment?.endpoint})`
        });
        break;
      case DataMap.Resource_Type.dbTwoTableSet.value:
        assign(item, {
          environmentName: has(item, 'environment.extendInfo.clusterType')
            ? item.environment?.name
            : `${item.environment?.name}(${item.environment?.endpoint})`,
          instance: item.extendInfo?.instance
        });
        break;
      case DataMap.Resource_Type.goldendbCluter.value:
        assign(item, {
          nodeIpAddress: item.endpoint
        });
        break;
      case DataMap.Resource_Type.goldendbInstance.value:
        assign(item, {
          nodeIpAddress: item.environment?.endpoint,
          linkStatus: item.extendInfo?.linkStatus
        });
        break;
      case DataMap.Resource_Type.gaussdbForOpengaussInstance.value:
        assign(item, {
          status: item.extendInfo?.status,
          region: item.extendInfo?.region
        });
        break;
      case DataMap.Resource_Type.informixInstance.value:
        assign(item, {
          environmentName: item.extendInfo?.clusterName,
          linkStatus: item.extendInfo?.linkStatus,
          databaseType: includes(item.version.toLowerCase(), 'gbase')
            ? DataMap.informixDatabaseType.gbase.value
            : DataMap.informixDatabaseType.informix.value
        });
        break;
      case DataMap.Resource_Type.OceanBaseTenant.value:
        assign(item, {
          environmentName: item.environment?.name,
          linkStatus: item.extendInfo?.linkStatus
        });
        break;
      case DataMap.Resource_Type.OceanBaseCluster.value:
        assign(item, {
          authorizedUser: item?.username
        });
        break;
      case DataMap.Resource_Type.informixService.value:
        assign(item, {
          logBackup: item.extendInfo?.logBackup
        });
        break;
      case DataMap.Resource_Type.tdsqlInstance.value:
      case DataMap.Resource_Type.tdsqlDistributedInstance.value:
        assign(item, {
          linkStatus: item.extendInfo?.linkStatus,
          parentName: item.environment?.name,
          mysql_version: item.extendInfo?.mysql_version
        });
        break;
      case DataMap.Resource_Type.tidbDatabase.value:
        assign(item, {
          linkStatus: item.extendInfo?.linkStatus,
          parentName: item.environment?.name
        });
        break;
      case DataMap.Resource_Type.tidbCluster.value:
        assign(item, {
          authorizedUser: item.extendInfo?.owner
        });
        break;
      case DataMap.Resource_Type.tidbTable.value:
        assign(item, {
          linkStatus: item.extendInfo?.linkStatus
        });
        break;
      case DataMap.Resource_Type.ObjectSet.value:
        assign(item, {
          storageType: Number(item.extendInfo?.storageType)
        });
        break;
      case DataMap.Resource_Type.lightCloudGaussdbProject.value:
      case DataMap.Resource_Type.lightCloudGaussdbInstance.value:
        assign(item, {
          status: item.extendInfo?.status,
          region: item.extendInfo?.region,
          pmAddress: item.extendInfo?.pmAddress,
          isAllowRestore: get(item, 'extendInfo.isAllowRestore', 'false'),
          instanceStatus: get(item, 'extendInfo.instanceStatus', '1')
        });
        break;
      case DataMap.Resource_Type.ExchangeEmail.value:
        assign(item, {
          address: item.extendInfo?.PrimarySmtpAddress
        });
        break;
      case DataMap.Resource_Type.saphanaInstance.value:
        assign(item, {
          enableLogBackup: item?.extendInfo?.enableLogBackup
        });
        break;
      case DataMap.Resource_Type.saphanaDatabase.value:
        assign(item, {
          sapHanaDbType: item?.extendInfo?.sapHanaDbType,
          sapHanaDbDeployType: item?.extendInfo?.sapHanaDbDeployType,
          linkStatus: item.extendInfo?.linkStatus,
          environmentEndpoint: item.environment?.endpoint
        });
        break;
      default:
        break;
    }
  }

  getProtectType() {
    switch (this.configParams.activeIndex) {
      case DataMap.Resource_Type.DWS_Cluster.value:
      case DataMap.Resource_Type.DWS_Database.value:
      case DataMap.Resource_Type.DWS_Schema.value:
      case DataMap.Resource_Type.DWS_Table.value:
        return ProtectResourceCategory.GaussDBDWS;
      case DataMap.Resource_Type.SQLServerInstance.value:
      case DataMap.Resource_Type.SQLServerGroup.value:
      case DataMap.Resource_Type.SQLServerDatabase.value:
        return ProtectResourceCategory.SQLServer;
      case DataMap.Resource_Type.tidbCluster.value:
      case DataMap.Resource_Type.tidbDatabase.value:
      case DataMap.Resource_Type.volume.value:
      case DataMap.Resource_Type.ActiveDirectory.value:
      case DataMap.Resource_Type.ObjectSet.value:
      case DataMap.Resource_Type.Exchange.value:
      case DataMap.Resource_Type.ExchangeEmail.value:
      case DataMap.Resource_Type.ExchangeDataBase.value:
      case DataMap.Resource_Type.tdsqlInstance.value:
        return this.configParams.activeIndex;
      default:
        return ProtectResourceCategory.GeneralDB;
    }
  }

  // 根据类型返回资源子类型的DataMap
  getResourceSubTypeConfigMap() {
    switch (this.configParams.activeIndex) {
      case DataMap.Resource_Type.Exchange.value:
        return this.dataMapService.toArray('exchangeGroupType');
    }
  }
}
