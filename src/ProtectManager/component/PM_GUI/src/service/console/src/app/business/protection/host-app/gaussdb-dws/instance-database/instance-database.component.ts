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
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import { RegisterComponent as RegisterGbaseClusterComponent } from 'app/business/protection/database/gbase/register/register.component';
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
  MODAL_COMMON,
  OperateItems,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ProtectResourceAction,
  ProtectResourceCategory,
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
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  clone,
  cloneDeep,
  each,
  filter as _filter,
  find,
  first,
  get,
  has,
  includes,
  isEmpty,
  isUndefined,
  map as _map,
  mapValues,
  omit,
  reject,
  size,
  some,
  trim,
  values
} from 'lodash';
import { combineLatest } from 'rxjs';
import { map } from 'rxjs/operators';
import { CreateTablesetComponent } from '../../db-two/create-tabelset/create-tableset.component';
import { RegisterClusterComponent as RegisterDbTwoClusterComponent } from '../../db-two/register-cluster/register-cluster.component';
import { RegisterInstanceComponent as RegisterDbTwoInstanceComponent } from '../../db-two/register-instance/register-instance.component';
import { RegisterComponent as RegisterGaussdbForOpengaussComponent } from '../../gaussdb-for-opengauss/register/register.component';
import { RegisterComponent as RegisterGoldendbClusterComponent } from '../../goldendb/register-cluster/register.component';
import { RegisterComponent as RegisterGoldendbComponent } from '../../goldendb/register/register.component';
import { RegisterInstanceComponent as RegisterGbaseInformixInstanceComponent } from '../../informix/register-instance/register-instance.component';
import { RegisterClusterComponent as RegisterSQLServerClusterComponent } from '../../sql-server/register-cluster/register-cluster.component';
import { RegisterInstanceComponent } from '../../sql-server/register-instance/register-instance.component';
import { CreateSchemaComponent } from './create-schema/create-schema.component';
import { RegisterClusterComponent as RegisterDWSClusterComponent } from './register-cluster/register-cluster.component';

@Component({
  selector: 'aui-instance-database',
  templateUrl: './instance-database.component.html',
  styleUrls: ['./instance-database.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class InstanceDatabaseComponent implements OnInit, AfterViewInit {
  name;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  optItems = [];
  columns;
  placeholder;
  dataMap = DataMap;
  maxItems;
  colOpts = [];

  groupCommon = GROUP_COMMON;

  currentDetailUuid: string;

  @Input() subType;
  @Input() activeIndex;
  @Input() tableCols;
  @Input() tableOpts;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('typeTpl', { static: true })
  typeTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;
  @ViewChild('allowRestoreTpl', { static: true })
  allowRestoreTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private protectService: ProtectService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    public virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private batchOperateService: BatchOperateService,
    private takeManualBackupService: TakeManualBackupService,
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
  }

  onChange() {
    this.initConfig();
    this.dataTable.fetchData();
  }

  getPopoverContent(): string {
    switch (this.activeIndex) {
      case DataMap.Resource_Type.dbTwoCluster.value:
      case DataMap.Resource_Type.goldendbCluter.value:
      case DataMap.Resource_Type.SQLServerCluster.value:
        return this.i18n.get('protection_guide_cluster_tip_label');
      case DataMap.Resource_Type.dbTwoInstance.value:
      case DataMap.Resource_Type.goldendbInstance.value:
      case DataMap.Resource_Type.SQLServerInstance.value:
        return this.i18n.get('protection_guide_instance_tip_label');
      case DataMap.Resource_Type.dbTwoTableSet.value:
        return this.i18n.get('protection_guide_create_tableset_tip_label');
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
        label: includes(
          [
            DataMap.Resource_Type.dbTwoCluster.value,
            DataMap.Resource_Type.DWS_Cluster.value,
            DataMap.Resource_Type.SQLServerCluster.value,
            DataMap.Resource_Type.SQLServerInstance.value,
            DataMap.Resource_Type.SQLServerClusterInstance.value,
            DataMap.Resource_Type.gaussdbForOpengaussProject.value,
            DataMap.Resource_Type.goldendbCluter.value,
            DataMap.Resource_Type.dbTwoInstance.value,
            DataMap.Resource_Type.gbaseCluster.value,
            DataMap.Resource_Type.gbaseInstance.value
          ],
          this.activeIndex
        )
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_create_label'),
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
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.Protection,
        label: this.i18n.get('common_protect_label'),
        onClick: data => {
          if (
            this.activeIndex === DataMap.Resource_Type.DWS_Database.value &&
            find(data, item => item.version === '8.0.0')
          ) {
            this.messageService.error(
              this.i18n.get('protection_cluster_version_error_tips_label')
            );
            return;
          }
          if (
            this.activeIndex === DataMap.Resource_Type.DWS_Schema.value &&
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
                return !isEmpty(val.sla_id) && hasBackupPermission(data);
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
              this.dataTable.fetchData();
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
              this.dataTable.fetchData();
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
                  !(
                    val.subType === DataMap.Resource_Type.DWS_Cluster.value &&
                    val.linkStatus === '0' &&
                    val.sourceType === 'autoscan'
                  ) &&
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
          return !size(data) || some(data, v => !hasResourcePermission(v));
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
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(data)
      }
    };

    this.optItems = cloneDeep(
      getPermissionMenuItem(
        values(
          reject(opts, opt => {
            if (this.activeIndex === DataMap.Resource_Type.DWS_Cluster.value) {
              return includes(['register'], opt.id);
            } else if (
              includes(
                [
                  DataMap.Resource_Type.DWS_Database.value,
                  DataMap.Resource_Type.gaussdbForOpengaussInstance.value
                ],
                this.activeIndex
              )
            ) {
              return includes(
                [
                  'register',
                  'rescan',
                  'connectivityTest',
                  'modify',
                  'deleteResource'
                ],
                opt.id
              );
            } else if (
              includes(
                [
                  DataMap.Resource_Type.DWS_Schema.value,
                  DataMap.Resource_Type.DWS_Table.value,
                  DataMap.Resource_Type.dbTwoTableSet.value
                ],
                this.activeIndex
              )
            ) {
              return includes(
                ['register', 'rescan', 'connectivityTest'],
                opt.id
              );
            } else if (
              includes(
                [
                  DataMap.Resource_Type.SQLServerCluster.value,
                  DataMap.Resource_Type.dbTwoCluster.value
                ],
                this.activeIndex
              )
            ) {
              return includes(
                [
                  'register',
                  'protect',
                  'modifyProtect',
                  'removeProtection',
                  'activeProtection',
                  'deactiveProtection',
                  'recovery',
                  'manualBackup',
                  'rescan',
                  'connectivityTest'
                ],
                opt.id
              );
            } else if (
              includes(
                [DataMap.Resource_Type.goldendbCluter.value],
                this.activeIndex
              )
            ) {
              return includes(
                [
                  'register',
                  'protect',
                  'modifyProtect',
                  'removeProtection',
                  'activeProtection',
                  'deactiveProtection',
                  'recovery',
                  'manualBackup',
                  'rescan'
                ],
                opt.id
              );
            } else if (
              includes(
                [DataMap.Resource_Type.goldendbInstance.value],
                this.activeIndex
              )
            ) {
              return includes(['register', 'rescan'], opt.id);
            } else if (
              this.activeIndex === DataMap.Resource_Type.SQLServerInstance.value
            ) {
              return includes(['register'], opt.id);
            } else if (
              includes(
                [
                  DataMap.Resource_Type.SQLServerGroup.value,
                  DataMap.Resource_Type.SQLServerDatabase.value,
                  DataMap.Resource_Type.dbTwoDatabase.value
                ],
                this.activeIndex
              )
            ) {
              return includes(
                [
                  'register',
                  'rescan',
                  'connectivityTest',
                  'modify',
                  'deleteResource'
                ],
                opt.id
              );
            } else if (
              includes(
                [DataMap.Resource_Type.dbTwoInstance.value],
                this.activeIndex
              )
            ) {
              return includes(
                [
                  'register',
                  'protect',
                  'modifyProtect',
                  'removeProtection',
                  'activeProtection',
                  'deactiveProtection',
                  'recovery',
                  'manualBackup'
                ],
                opt.id
              );
            } else if (
              includes(
                [DataMap.Resource_Type.gaussdbForOpengaussProject.value],
                this.activeIndex
              )
            ) {
              return includes(
                [
                  'register',
                  'protect',
                  'modifyProtect',
                  'removeProtection',
                  'activeProtection',
                  'deactiveProtection',
                  'recovery',
                  'manualBackup'
                ],
                opt.id
              );
            } else if (
              includes(
                [DataMap.Resource_Type.gbaseCluster.value],
                this.activeIndex
              )
            ) {
              return includes(
                [
                  'register',
                  'protect',
                  'modifyProtect',
                  'removeProtection',
                  'activeProtection',
                  'deactiveProtection',
                  'recovery',
                  'manualBackup',
                  'rescan'
                ],
                opt.id
              );
            } else if (
              includes(
                [DataMap.Resource_Type.gbaseInstance.value],
                this.activeIndex
              )
            ) {
              return includes(['register', 'rescan'], opt.id);
            }
          })
        )
      )
    );

    if (
      ![
        DataMap.Resource_Type.DWS_Cluster.value,
        DataMap.Resource_Type.DWS_Database.value,
        DataMap.Resource_Type.DWS_Schema.value,
        DataMap.Resource_Type.DWS_Table.value
      ].includes(this.activeIndex)
    ) {
      // 除dws外其他应用都不支持允许恢复
      this.optItems = reject(this.optItems, item =>
        includes(['allowRecovery', 'disableRecovery'], item.id)
      );
    }

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
        key: 'linkStatus',
        name: includes(
          [
            DataMap.Resource_Type.SQLServerClusterInstance.value,
            DataMap.Resource_Type.SQLServerInstance.value
          ],
          this.activeIndex
        )
          ? this.i18n.get('common_auth_status_label')
          : this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: includes(
            [DataMap.Resource_Type.DWS_Cluster.value],
            this.activeIndex
          )
            ? this.dataMapService.toArray('gaussDBDWS_Resource_LinkStatus')
            : includes(
                [DataMap.Resource_Type.goldendbInstance.value],
                this.activeIndex
              )
            ? this.dataMapService.toArray('clickHouse_cluster_status')
            : includes(
                [DataMap.Resource_Type.gaussdbForOpengaussInstance.value],
                this.activeIndex
              )
            ? this.dataMapService.toArray('gaussDBInstance')
            : this.dataMapService.toArray('resource_LinkStatus_Special')
        },
        cellRender: {
          type: 'status',
          config: includes(
            [DataMap.Resource_Type.DWS_Cluster.value],
            this.activeIndex
          )
            ? this.dataMapService.toArray('gaussDBDWS_Resource_LinkStatus')
            : includes(
                [DataMap.Resource_Type.goldendbInstance.value],
                this.activeIndex
              )
            ? this.dataMapService.toArray('clickHouse_cluster_status')
            : includes(
                [DataMap.Resource_Type.gaussdbForOpengaussInstance.value],
                this.activeIndex
              )
            ? this.dataMapService.toArray('gaussDBInstance')
            : this.dataMapService.toArray('resource_LinkStatus_Special')
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
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('logBackupStatus')
        }
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
      // 所属环境, 默认为environment.name
      {
        key: 'clusterOrHostName',
        name: includes(
          [
            DataMap.Resource_Type.SQLServerClusterInstance.value,
            DataMap.Resource_Type.SQLServerInstance.value,
            DataMap.Resource_Type.SQLServerDatabase.value,
            DataMap.Resource_Type.dbTwoInstance.value,
            DataMap.Resource_Type.dbTwoDatabase.value,
            DataMap.Resource_Type.dbTwoTableSet.value
          ],
          this.activeIndex
        )
          ? this.i18n.get('protection_host_cluster_name_label')
          : this.i18n.get('insight_report_belong_cluster_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      // 所属一级或二级父资源，默认为parentName
      {
        key: 'ownedInstance',
        name: this.i18n.get('commom_owned_instance_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'ownedProject',
        name: this.i18n.get('commom_owned_project_label'),
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
        key: 'database',
        name: this.i18n.get('protection_host_database_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'instance',
        name: this.i18n.get('protection_database_instance_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'nodeIpAddress',
        name: this.i18n.get('protection_node_ip_address_label')
      },
      {
        key: 'version',
        name: this.i18n.get('common_version_label')
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
        thExtra:
          this.activeIndex === DataMap.Resource_Type.DWS_Cluster.value
            ? this.allowRestoreTpl
            : null,
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

    switch (this.activeIndex) {
      case DataMap.Resource_Type.goldendbCluter.value:
      case DataMap.Resource_Type.dbTwoCluster.value:
      case DataMap.Resource_Type.SQLServerCluster.value:
      case DataMap.Resource_Type.DWS_Cluster.value:
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
      case DataMap.Resource_Type.gaussdbForOpengaussProject.value:
        this.placeholder = this.i18n.get('common_search_type_label', [
          this.i18n.get('common_project_label')
        ]);
        break;
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
      default:
        this.placeholder = this.i18n.get('common_search_type_label', ['']);
    }

    (includes(this.tableOpts, 'register') &&
      includes(this.tableOpts, 'protect')) ||
    this.tableOpts?.length <= 2
      ? (this.maxItems = 3)
      : (this.maxItems = 2);

    this.tableCols?.splice(-1, 0, 'labelList');

    this.columns = cols.filter(col => {
      return includes(this.tableCols, col.key);
    });

    this.tableOpts?.push('addTag', 'removeTag');

    this.optsConfig = cloneDeep(
      getPermissionMenuItem(
        values(
          reject(opts, opt => {
            return !includes(this.tableOpts, opt.id);
          })
        )
      )
    );

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

    const removeProtectionBtn = clone(opts.removeProtection);
    removeProtectionBtn.divide = false;
    const deactiveBtn = clone(opts.deactiveProtection);
    deactiveBtn.divide = false;
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data ? data : this.selectionData,
      type: SetTagType.Resource,
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
      type: SetTagType.Resource,
      onOk: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
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
  }

  getDetail(data) {
    this.currentDetailUuid = data.uuid;
    this.detailService.openDetailModal(this.activeIndex, {
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
    const resourceType = item?.subType || item?.sub_type || this.activeIndex;

    switch (resourceType) {
      case DataMap.Resource_Type.dbTwoCluster.value:
        this.registerDbTwoCluster(item);
        break;
      case DataMap.Resource_Type.dbTwoClusterInstance.value:
      case DataMap.Resource_Type.dbTwoInstance.value:
        this.registerDbTwoInstance(item);
        break;
      case DataMap.Resource_Type.dbTwoTableSet.value:
        this.createDbTwoTableset(item);
        break;
      case DataMap.Resource_Type.DWS_Cluster.value:
        this.registerDWSCluster(item);
        break;
      case DataMap.Resource_Type.SQLServerCluster.value:
        this.registerSQLServerCluster(item);
        break;
      case DataMap.Resource_Type.SQLServerClusterInstance.value:
      case DataMap.Resource_Type.SQLServerInstance.value:
        this.registerInstance(item);
        break;
      case DataMap.Resource_Type.DWS_Schema.value:
      case DataMap.Resource_Type.DWS_Table.value:
        this.createSet(item);
        break;
      case DataMap.Resource_Type.goldendbCluter.value:
        this.registerGoldendbCluster(item);
        break;
      case DataMap.Resource_Type.goldendbInstance.value:
        this.registerGoldenDBInstance(item);
        break;
      case DataMap.Resource_Type.gaussdbForOpengaussProject.value:
        this.registerGaussdbForOpengauss(item);
        break;
      case DataMap.Resource_Type.gbaseCluster.value:
      case DataMap.Resource_Type.gbaseInstance.value:
        this.registerGbase(item);
        break;
      default:
        break;
    }
  }

  registerGbase(item) {
    const winComponent =
      this.activeIndex === DataMap.Resource_Type.gbaseCluster.value
        ? RegisterGbaseClusterComponent
        : RegisterGbaseInformixInstanceComponent;
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-gbase',
        lvWidth: MODAL_COMMON.normalWidth + 120,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: winComponent,
        lvOkDisabled: isEmpty(item),
        lvComponentParams: {
          rowData: item,
          subType: this.activeIndex
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterGoldendbClusterComponent;
          const modalIns = modal.getInstance();
          if (this.activeIndex === DataMap.Resource_Type.gbaseInstance.value) {
            content.formGroup.statusChanges.subscribe(res => {
              modalIns.lvOkDisabled = res !== 'VALID';
            });
          }
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent();
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable?.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  registerGaussdbForOpengauss(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-gaussdb-for-opengauss',
        lvWidth: MODAL_COMMON.largeWidth + 80,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterGaussdbForOpengaussComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData: item
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterGaussdbForOpengaussComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest(
            content.formGroup.statusChanges,
            content.valid$
          );
          combined.subscribe(latestValues => {
            const [formGroupStatus, fileStatus] = latestValues;
            modalIns.lvOkDisabled = !(
              formGroupStatus === 'VALID' && fileStatus
            );
          });
          content.valid$.next(false);
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterGaussdbForOpengaussComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable?.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  registerGoldendbCluster(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-goldendb-cluster',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterGoldendbClusterComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData: item
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterGoldendbClusterComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest(content.formGroup.statusChanges);
          combined.subscribe(latestValues => {
            const [formGroupStatus] = latestValues;
            modalIns.lvOkDisabled = !(formGroupStatus === 'VALID');
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterGoldendbClusterComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable?.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  registerGoldenDBInstance(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-goldendb-cluster',
        lvWidth: MODAL_COMMON.normalWidth + 200,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_create_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterGoldendbComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData: item
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterGoldendbComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest([
            content.formGroup.statusChanges,
            content.valid$,
            content.validNode$
          ]);
          combined.subscribe(latestValues => {
            const [formGroupStatus, validFile, validNode] = latestValues;

            if (!!content.rowData) {
              modalIns.lvOkDisabled = formGroupStatus !== 'VALID' || !validNode;
            } else {
              modalIns.lvOkDisabled = !(
                formGroupStatus === 'VALID' &&
                validFile &&
                validNode
              );
            }
          });
          content.valid$.next(false);
          content.formGroup.updateValueAndValidity();
          content.validNode$.next(false);
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterGoldendbComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable?.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  registerDbTwoCluster(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-db-two-cluster',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterDbTwoClusterComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData: item
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterDbTwoClusterComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest(content.formGroup.statusChanges);
          combined.subscribe(latestValues => {
            const [formGroupStatus] = latestValues;
            modalIns.lvOkDisabled = !(formGroupStatus === 'VALID');
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterDbTwoClusterComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable?.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  registerDbTwoInstance(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-db-two-instance',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterDbTwoInstanceComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          item
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterDbTwoInstanceComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest(content.formGroup.statusChanges);
          combined.subscribe(latestValues => {
            const [formGroupStatus] = latestValues;
            modalIns.lvOkDisabled = !(formGroupStatus === 'VALID');
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterDbTwoInstanceComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable?.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  createDbTwoTableset(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-db-two-tableset',
        lvWidth: MODAL_COMMON.largeWidth + 50,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_create_label')
          : this.i18n.get('common_modify_label'),
        lvContent: CreateTablesetComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data: item
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateTablesetComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest(
            content.formGroup.statusChanges,
            content.valid$
          );
          combined.subscribe(latestValues => {
            const [formGroupStatus, valid] = latestValues;
            modalIns.lvOkDisabled = !(formGroupStatus === 'VALID' && valid);
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateTablesetComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable?.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  registerDWSCluster(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-dws-cluster',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterDWSClusterComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData: item,
          clusterData: this.tableData?.data || []
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterDWSClusterComponent;
          const modalIns = modal.getInstance();

          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterDWSClusterComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable?.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  registerSQLServerCluster(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-nas-shared',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterSQLServerClusterComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData: item
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterSQLServerClusterComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest(
            content.formGroup.statusChanges,
            content.valid$
          );
          combined.subscribe(latestValues => {
            const [formGroupStatus, valid] = latestValues;
            modalIns.lvOkDisabled = !(formGroupStatus === 'VALID' && valid);
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterSQLServerClusterComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable?.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  createSet(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-nas-shared',
        lvWidth: MODAL_COMMON.largeWidth,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_create_label')
          : this.i18n.get('common_modify_label'),
        lvContent: CreateSchemaComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          type: item?.subType || item?.sub_type || this.activeIndex,
          data: item
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateSchemaComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest(
            content.formGroup.statusChanges,
            content.valid$
          );
          combined.subscribe(latestValues => {
            const [formGroupStatus, valid] = latestValues;
            modalIns.lvOkDisabled = !(formGroupStatus === 'VALID' && valid);
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateSchemaComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable?.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  registerInstance(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-nas-shared',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterInstanceComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData: item,
          getInstance: () => this.dataTable.fetchData()
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterInstanceComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest(
            content.formGroup.statusChanges,
            content.valid$
          );
          combined.subscribe(latestValues => {
            const [formGroupStatus, valid] = latestValues;
            modalIns.lvOkDisabled = !(formGroupStatus === 'VALID' && valid);
          });
          content.formGroup.updateValueAndValidity();
        }
      })
    );
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
        content:
          data[0].subType === DataMap.Resource_Type.SQLServerInstance.value
            ? this.i18n.get('protection_sqlserver_instance_delete_label')
            : this.i18n.get('protection_nas_share_delete_label'),
        onOK: () => {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: data[0].uuid
            })
            .subscribe(() => {
              this.selectionData = reject(
                this.dataTable.getAllSelections(),
                item => {
                  return item.uuid === data[0].uuid;
                }
              );
              this.dataTable.setSelections(this.selectionData);
              this.dataTable.fetchData();
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
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
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

    const defaultConditions = {
      subType:
        this.activeIndex === DataMap.Resource_Type.SQLServerInstance.value
          ? [
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value
            ]
          : this.activeIndex === DataMap.Resource_Type.dbTwoInstance.value
          ? [
              DataMap.Resource_Type.dbTwoClusterInstance.value,
              DataMap.Resource_Type.dbTwoInstance.value
            ]
          : this.activeIndex === DataMap.Resource_Type.gbaseInstance.value
          ? [
              DataMap.Resource_Type.gbaseInstance.value,
              DataMap.Resource_Type.gbaseClusterInstance.value
            ]
          : this.activeIndex
    };

    if (this.activeIndex === DataMap.Resource_Type.dbTwoInstance.value) {
      assign(defaultConditions, { isTopInstance: InstanceType.TopInstance });
    }

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.clusterOrHostName) {
        if (
          this.activeIndex === DataMap.Resource_Type.SQLServerDatabase.value
        ) {
          assign(conditionsTemp, {
            hostName: conditionsTemp.clusterOrHostName
          });
        } else {
          assign(conditionsTemp, {
            environment: {
              name: conditionsTemp.clusterOrHostName
            }
          });
        }
        delete conditionsTemp.clusterOrHostName;
      }
      if (conditionsTemp.ownedInstance) {
        if (this.activeIndex === DataMap.Resource_Type.dbTwoTableSet.value) {
          assign(conditionsTemp, {
            instance: conditionsTemp.ownedInstance
          });
        } else {
          assign(conditionsTemp, {
            parentName: conditionsTemp.ownedInstance
          });
        }
        delete conditionsTemp.ownedInstance;
      }
      if (conditionsTemp.ownedProject) {
        assign(conditionsTemp, {
          parentName: conditionsTemp.ownedProject
        });
        delete conditionsTemp.ownedProject;
      }
      if (
        conditionsTemp.database &&
        includes(
          [
            DataMap.Resource_Type.DWS_Schema.value,
            DataMap.Resource_Type.DWS_Table.value
          ],
          this.activeIndex
        )
      ) {
        assign(conditionsTemp, {
          parentName: conditionsTemp.database
        });
        delete conditionsTemp.database;
      }
      if (
        conditionsTemp.linkStatus &&
        includes(
          [DataMap.Resource_Type.gaussdbForOpengaussInstance.value],
          this.activeIndex
        )
      ) {
        assign(conditionsTemp, {
          status: conditionsTemp.linkStatus
        });
        delete conditionsTemp.linkStatus;
      }
      if (conditionsTemp.isAllowRestore) {
        if (conditionsTemp.isAllowRestore.length === 3) {
          delete conditionsTemp.isAllowRestore;
        } else if (conditionsTemp.isAllowRestore.includes('false')) {
          conditionsTemp.isAllowRestore[0] = ['!='];
          conditionsTemp.isAllowRestore[1] = 'true';
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
      assign(defaultConditions, conditionsTemp);
    }

    if (this.activeIndex === DataMap.Resource_Type.SQLServerDatabase.value) {
      assign(defaultConditions, {
        agId: [['=='], '']
      });
    }
    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map((res: any) => {
          each(res.records, item => {
            switch (this.activeIndex) {
              case DataMap.Resource_Type.SQLServerInstance.value:
                this.formatSQLServerInstanceData(item);
                break;
              case DataMap.Resource_Type.SQLServerGroup.value:
                this.formatSQLServerGroupData(item);
                break;
              case DataMap.Resource_Type.SQLServerDatabase.value:
                this.formatSQLServerDatabaseData(item);
                break;
              case DataMap.Resource_Type.DWS_Database.value:
                this.formatDWSDatabaseData(item);
                break;
              case DataMap.Resource_Type.DWS_Schema.value:
                this.formatDWSSchemaData(item);
                break;
              case DataMap.Resource_Type.DWS_Table.value:
                this.formatDWSTableData(item);
                break;
              case DataMap.Resource_Type.dbTwoCluster.value:
                this.formatDbTwoClusterData(item);
                break;
              case DataMap.Resource_Type.dbTwoInstance.value:
              case DataMap.Resource_Type.dbTwoClusterInstance.value:
                this.formatDbTwoInstanceData(item);
                break;
              case DataMap.Resource_Type.dbTwoDatabase.value:
                this.formatDbTwoDatabaseData(item);
                break;
              case DataMap.Resource_Type.dbTwoTableSet.value:
                this.formatDbTwoTablesetData(item);
                break;
              case DataMap.Resource_Type.goldendbCluter.value:
                this.formatGoldenDBClusterData(item);
                break;
              case DataMap.Resource_Type.goldendbInstance.value:
                this.formatGoldenDBData(item);
                break;
              case DataMap.Resource_Type.gaussdbForOpengaussInstance.value:
                this.formatGaussDBProjectData(item);
                break;
              case DataMap.Resource_Type.gbaseCluster.value:
                this.formatGbaseClusterData(item);
                break;
              case DataMap.Resource_Type.gbaseInstance.value:
                this.formatGbaseInstanceData(item);
                break;
              default:
                break;
            }

            // 获取标签数据
            const { showList, hoverList } = getLabelList(item);
            assign(item, {
              sub_type: item.subType,
              auth_status: DataMap.Verify_Status.true.value,
              showLabelList: showList,
              hoverLabelList: hoverList
            });
            if (
              [
                DataMap.Resource_Type.DWS_Database.value,
                DataMap.Resource_Type.DWS_Schema.value,
                DataMap.Resource_Type.DWS_Table.value,
                DataMap.Resource_Type.DWS_Cluster.value
              ].includes(this.activeIndex)
            ) {
              assign(item, {
                isAllowRestore: get(item, 'extendInfo.isAllowRestore', 'false')
              });
            }
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
        this.cdr.detectChanges();
      });
  }

  formatGbaseClusterData(item) {
    assign(item, {
      logBackup: item.extendInfo?.logBackup
    });
  }

  formatGbaseInstanceData(item) {
    assign(item, {
      linkStatus: item.extendInfo?.linkStatus,
      clusterOrHostName: item.environment?.name
    });
  }

  formatGaussDBProjectData(item) {
    assign(item, {
      linkStatus: item.extendInfo?.status,
      ownedProject: item.parentName,
      region: item.extendInfo?.region
    });
  }

  formatGoldenDBClusterData(item) {
    assign(item, {
      nodeIpAddress: item.endpoint
    });
  }

  formatGoldenDBData(item) {
    assign(item, {
      nodeIpAddress: item.environment?.endpoint,
      linkStatus: item.extendInfo?.linkStatus
    });
  }

  formatSQLServerInstanceData(item) {
    assign(item, {
      clusterOrHostName: item.environment?.name,
      nodeIpAddress:
        item.subType === DataMap.Resource_Type.SQLServerClusterInstance.value
          ? item.path
          : item.environment?.endpoint
    });
  }

  formatDbTwoClusterData(item) {
    assign(item, {
      clusterType: item?.extendInfo?.clusterType
    });
  }

  formatDbTwoInstanceData(item: { [key: string]: any }) {
    assign(item, {
      clusterOrHostName: has(item, 'environment.extendInfo.clusterType')
        ? item.environment?.name
        : `${item.environment?.name}(${item.environment?.endpoint})`,
      linkStatus: item.extendInfo?.linkStatus
    });
  }

  formatDbTwoDatabaseData(item: { [key: string]: any }) {
    assign(item, {
      clusterOrHostName: has(item, 'environment.extendInfo.clusterType')
        ? item.environment?.name
        : `${item.environment?.name}(${item.environment?.endpoint})`,
      ownedInstance: item.parentName
    });
  }

  formatDbTwoTablesetData(item: { [key: string]: any }) {
    assign(item, {
      clusterOrHostName: has(item, 'environment.extendInfo.clusterType')
        ? item.environment?.name
        : `${item.environment?.name}(${item.environment?.endpoint})`,
      ownedInstance: item.extendInfo?.instance,
      database: item.parentName
    });
  }

  formatSQLServerGroupData(item) {
    assign(item, {
      clusterOrHostName: item.parentName
    });
  }

  formatSQLServerDatabaseData(item) {
    assign(item, {
      ownedInstance: item.extendInfo?.instanceName,
      clusterOrHostName: item.extendInfo?.hostName
    });
  }

  formatDWSDatabaseData(item) {
    assign(item, {
      clusterOrHostName: item.environment?.name
    });
  }

  formatDWSSchemaData(item) {
    assign(item, {
      clusterOrHostName: item.environment?.name,
      database: item.parentName
    });
  }

  formatDWSTableData(item) {
    assign(item, {
      clusterOrHostName: item.environment?.name,
      database: item.parentName
    });
  }

  getResourceDetail(res) {
    this.currentDetailUuid = res.uuid;
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
        this.detailService.openDetailModal(this.activeIndex, {
          data: assign(
            omit(cloneDeep(res), ['sla_id']),
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
    this.protectService.openProtectModal(
      includes(
        [
          DataMap.Resource_Type.DWS_Cluster.value,
          DataMap.Resource_Type.DWS_Database.value,
          DataMap.Resource_Type.DWS_Schema.value,
          DataMap.Resource_Type.DWS_Table.value
        ],
        this.subType
      )
        ? ProtectResourceCategory.GaussDBDWS
        : includes(
            [
              DataMap.Resource_Type.dbTwoCluster.value,
              DataMap.Resource_Type.dbTwoClusterInstance.value,
              DataMap.Resource_Type.dbTwoInstance.value,
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.dbTwoTableSet.value,
              DataMap.Resource_Type.goldendbInstance.value
            ],
            this.subType
          )
        ? ProtectResourceCategory.db2
        : includes(
            [DataMap.Resource_Type.gaussdbForOpengaussInstance.value],
            this.subType
          )
        ? ProtectResourceCategory.GaussdbForOpengauss
        : ProtectResourceCategory.SQLServer,
      action,
      {
        width: 780,
        data,
        onOK: () => {
          this.selectionData = [];
          this.dataTable.setSelections([]);
          return this.dataTable.fetchData();
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
}
