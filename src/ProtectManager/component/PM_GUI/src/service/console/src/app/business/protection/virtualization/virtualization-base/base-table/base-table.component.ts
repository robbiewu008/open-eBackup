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
  GlobalService,
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
  cloneDeep,
  each,
  filter,
  find,
  includes,
  isEmpty,
  isUndefined,
  map as _map,
  mapValues,
  reject,
  remove,
  size,
  some,
  tail,
  trim
} from 'lodash';
import { combineLatest, Observable } from 'rxjs';
import { SummaryComponent } from '../../cnware/summary/summary.component';
import { SummaryComponent as nutanixSummaryComponent } from '../../nutanix/summary/summary.component';
import { CreateGroupComponent } from '../../virtualization-group/create-group/create-group.component';

@Component({
  selector: 'aui-vir-base-table',
  templateUrl: './base-table.component.html',
  styleUrls: ['./base-table.component.less']
})
export class BaseTableComponent implements OnInit, OnChanges, AfterViewInit {
  @Input() subUnitType;
  @Input() subType: string;
  @Input() treeSelection: any;
  @Input() extParams;
  @Input() isSummary;
  @Output() updateTable = new EventEmitter();
  dataMap = DataMap;
  maxDisplayItems = 2;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig = [];
  selectionData: any = [];
  optItems = [];
  currentDetailUuid: string;
  name: string;
  groupTipLabel = this.i18n.get('protection_vm_group_tip_label');

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('groupTipTpl', { static: true })
  groupTipTpl: TemplateRef<any>;
  @ViewChild('hypervStatusTpl', { static: true })
  hypervStatusTpl: TemplateRef<any>;
  @ViewChild('rhvVersionTpl', { static: true })
  rhvVersionTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;
  @ViewChild('nutanixStatusTpl', { static: true })
  nutanixStatusTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private cookieService: CookieService,
    private protectService: ProtectService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private takeManualBackupService: TakeManualBackupService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private warningMessageService: WarningMessageService,
    private batchOperateService: BatchOperateService,
    private globalService: GlobalService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.treeSelection && this.dataTable) {
      this.dataTable.reinit();
      this.dataTable.stopPolling();
      this.dataTable.fetchData();
      this.selectionData = [];
      this.dataTable.setSelections([]);
    }
  }

  ngOnInit() {
    this.maxDisplayItems =
      this.subType === DataMap.Resource_Type.vmGroup.value ? 3 : 2;
    this.groupTipLabel = includes(
      [DataMap.Resource_Type.APSCloudServer.value],
      this.subType
    )
      ? this.i18n.get('protection_cloud_group_tip_label')
      : this.i18n.get('protection_vm_group_tip_label');
    this.virtualScroll.getScrollParam(400);
    this.initConfig();
  }

  getOptsConfig(opts) {
    switch (this.subType) {
      case DataMap.Resource_Type.vmGroup.value:
        this.optsConfig = getPermissionMenuItem(
          filter(opts, item => {
            if (item.id === 'manualBackup') {
              delete item.divide;
            }
            return includes(
              [
                'create',
                'protect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'delete',
                'addTag',
                'removeTag'
              ],
              item.id
            );
          })
        );
        break;
      case DataMap.Resource_Type.hyperVCluster.value:
        this.optsConfig = getPermissionMenuItem(
          filter(opts, item => {
            if (item.id === 'manualBackup') {
              delete item.divide;
            }
            return includes(['addTag', 'removeTag'], item.id);
          })
        );
        break;
      default:
        this.optsConfig = getPermissionMenuItem(
          filter(opts, item => {
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
        break;
    }
  }

  getOptItems(opts) {
    switch (this.subType) {
      case DataMap.Resource_Type.hyperVCluster.value:
        this.optItems = reject(
          this.extParams?.extOpts,
          item => item.id === 'register'
        );
        break;
      case DataMap.Resource_Type.cNwareVm.value:
      case DataMap.Resource_Type.nutanixVm.value:
      case DataMap.Resource_Type.APSCloudServer.value:
      case DataMap.Resource_Type.hyperVVm.value:
        this.optItems = getPermissionMenuItem(
          filter(opts, opt =>
            includes(
              [
                'protect',
                'modifyProtection',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'addTag',
                'removeTag'
              ],
              opt.id
            )
          )
        );
        break;
      case DataMap.Resource_Type.cNwareHost.value:
      case DataMap.Resource_Type.cNwareCluster.value:
      case DataMap.Resource_Type.nutanixHost.value:
      case DataMap.Resource_Type.nutanixCluster.value:
      case DataMap.Resource_Type.APSResourceSet.value:
      case DataMap.Resource_Type.APSZone.value:
      case DataMap.Resource_Type.hyperVHost.value:
        this.optItems = getPermissionMenuItem(
          filter(opts, opt =>
            includes(
              [
                'protect',
                'modifyProtection',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'manualBackup',
                'addTag',
                'removeTag'
              ],
              opt.id
            )
          )
        );
        break;
      case DataMap.Resource_Type.vmGroup.value:
        this.optItems = getPermissionMenuItem(
          filter(opts, opt =>
            includes(
              [
                'protect',
                'removeProtection',
                'modifyProtection',
                'activeProtection',
                'deactiveProtection',
                'manualBackup',
                'modify',
                'delete',
                'addTag',
                'removeTag'
              ],
              opt.id
            )
          )
        );
        break;
      default:
        break;
    }
    if (this.isSummary) {
      this.optItems = reject(this.optItems, opt =>
        includes(['recovery'], opt.id)
      );
    }
  }

  getCols(cols: { [key: string]: TableCols }): TableCols[] {
    const baseClos = [
      cols.sla,
      cols.slaCompliance,
      cols.protectionStatus,
      cols.labelList,
      cols.operation
    ];
    switch (this.subType) {
      case DataMap.Resource_Type.cNwareCluster.value:
      case DataMap.Resource_Type.nutanixCluster.value:
        return [
          cols.uuid,
          cols.name,
          cols.location,
          cols.sla,
          cols.protectionStatus,
          cols.labelList,
          cols.operation
        ];
      case DataMap.Resource_Type.cNwareHost.value:
      case DataMap.Resource_Type.nutanixHost.value:
        return [
          cols.uuid,
          cols.name,
          cols.location,
          cols.status,
          cols.sla,
          cols.protectionStatus,
          cols.labelList,
          cols.operation
        ];
      case DataMap.Resource_Type.cNwareVm.value:
      case DataMap.Resource_Type.nutanixVm.value:
        return [
          cols.uuid,
          cols.name,
          cols.location,
          cols.status,
          cols.mark,
          cols.sla,
          cols.slaCompliance,
          cols.protectionStatus,
          cols.resourceGroupName,
          cols.labelList,
          cols.operation
        ];
      case DataMap.Resource_Type.APSResourceSet.value:
        return [
          cols.uuid,
          cols.name,
          cols.sla,
          cols.slaCompliance,
          cols.protectionStatus,
          cols.labelList,
          cols.operation
        ];
      case DataMap.Resource_Type.APSZone.value:
        return [
          cols.uuid,
          cols.name,
          cols.location,
          cols.sla,
          cols.slaCompliance,
          cols.protectionStatus,
          cols.labelList,
          cols.operation
        ];
      case DataMap.Resource_Type.APSCloudServer.value:
        return [
          cols.uuid,
          cols.name,
          cols.location,
          cols.status,
          cols.sla,
          cols.slaCompliance,
          cols.protectionStatus,
          cols.resourceGroupName,
          cols.labelList,
          cols.operation
        ];
      case DataMap.Resource_Type.hyperVVm.value:
        return [
          cols.uuid,
          cols.name,
          cols.version,
          cols.status,
          cols.location,
          cols.sla,
          cols.slaCompliance,
          cols.protectionStatus,
          cols.resourceGroupName,
          cols.labelList,
          cols.operation
        ];
      case DataMap.Resource_Type.hyperVCluster.value:
        return [
          cols.uuid,
          cols.name,
          cols.location,
          cols.labelList,
          cols.operation
        ];
      case DataMap.Resource_Type.hyperVHost.value:
        return [cols.uuid, cols.name, cols.ip, cols.status, ...baseClos];
      case DataMap.Resource_Type.vmGroup.value:
        return [cols.uuid, cols.name, cols.numbers, ...baseClos];
      default:
        return baseClos;
    }
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data ? data : this.selectionData,
      type:
        this.subType === DataMap.Resource_Type.vmGroup.value
          ? SetTagType.ResourceGroup
          : SetTagType.Resource,
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
      type:
        this.subType === DataMap.Resource_Type.vmGroup.value
          ? SetTagType.ResourceGroup
          : SetTagType.Resource,
      onOk: () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      }
    });
  }

  getStatusExtParams(): any {
    switch (this.subType) {
      case DataMap.Resource_Type.cNwareVm.value:
      case DataMap.Resource_Type.cNwareHost.value:
        return {
          cellRender: {
            type: 'status',
            config: this.dataMapService.toArray('cnwareLinkStatus')
          },
          filter: {
            type: 'select',
            isMultiple: true,
            showCheckAll: true,
            options: this.dataMapService.toArray('cnwareLinkStatus')
          }
        };
      case DataMap.Resource_Type.APSCloudServer.value:
        return {
          cellRender: {
            type: 'status',
            config: this.dataMapService.toArray('ApsaraStackStatus')
          },
          filter: {
            type: 'select',
            isMultiple: true,
            showCheckAll: true,
            options: this.dataMapService.toArray('ApsaraStackStatus')
          }
        };
      case DataMap.Resource_Type.hyperVVm.value:
        return {
          cellRender: this.hypervStatusTpl,
          filter: {
            type: 'select',
            isMultiple: true,
            showCheckAll: true,
            options: this.dataMapService.toArray('hypervStatus')
          }
        };
      case DataMap.Resource_Type.hyperVHost.value:
        return {
          cellRender: {
            type: 'status',
            config: this.dataMapService.toArray('hypervHostStatus')
          },
          filter: {
            type: 'select',
            isMultiple: true,
            showCheckAll: true,
            options: this.dataMapService.toArray('resource_LinkStatus_Special')
          }
        };
      case DataMap.Resource_Type.nutanixHost.value:
        return {
          cellRender: this.nutanixStatusTpl,
          filter: {
            type: 'select',
            isMultiple: true,
            showCheckAll: true,
            options: this.dataMapService
              .toArray('nutanixHostStatus')
              .filter(item => {
                return [
                  DataMap.nutanixHostStatus.normal.value,
                  DataMap.nutanixHostStatus.new.value
                ].includes(item.value);
              })
          }
        };
      case DataMap.Resource_Type.nutanixVm.value:
        return {
          cellRender: this.nutanixStatusTpl,
          filter: {
            type: 'select',
            isMultiple: true,
            showCheckAll: true,
            options: this.dataMapService
              .toArray('nutanixVmStatus')
              .filter(item => {
                return [
                  DataMap.nutanixVmStatus.on.value,
                  DataMap.nutanixVmStatus.off.value,
                  DataMap.nutanixVmStatus.unknown.value
                ].includes(item.value);
              })
          }
        };
      default:
        return {};
    }
  }

  initConfig() {
    // 虚拟机（云服务器）组： 组内有资源数量（resourceCount）才能进行保护、修改保护
    const opts: ProButton[] = [
      {
        id: 'create',
        type: 'primary',
        displayCheck: () => {
          return this.subType === DataMap.Resource_Type.vmGroup.value;
        },
        disableCheck: () => {
          return !size(this.treeSelection);
        },
        label: this.i18n.get('common_create_label'),
        permission: RoleOperationMap.manageResource,
        divide: true,
        onClick: () => this.editGroup()
      },
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
                  hasProtectPermission(val) &&
                  this.isProtectable(val)
                );
              })
            ) !== size(data) ||
            !isUndefined(find(data, v => v.inGroup)) ||
            (data[0].subType === DataMap.Resource_Type.APSCloudServer.value &&
              !includes(
                [
                  DataMap.ApsaraStackStatus.running.value,
                  DataMap.ApsaraStackStatus.stopped.value
                ],
                data[0].status
              )) ||
            (this.subType === DataMap.Resource_Type.vmGroup.value &&
              !data[0].resourceCount)
          );
        },
        disabledTipsCheck: data => {
          return size(
            filter(data, val => {
              return this.isProtectable(val);
            })
          ) !== size(data)
            ? this.i18n.get('protection_nutanixvm_protect_tip_label')
            : '';
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
            (data[0].subType === DataMap.Resource_Type.APSCloudServer.value &&
              !includes(
                [
                  DataMap.ApsaraStackStatus.running.value,
                  DataMap.ApsaraStackStatus.stopped.value
                ],
                data[0].status
              )) ||
            (this.subType === DataMap.Resource_Type.vmGroup.value &&
              !data[0].resourceCount)
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
          if (this.subType === DataMap.Resource_Type.vmGroup.value) {
            this.protectService.removeGroupProtection(data).subscribe(() => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
          } else {
            this.protectService
              .removeProtection(_map(data, 'uuid'), _map(data, 'name'))
              .subscribe(() => {
                this.dataTable.fetchData();
                this.selectionData = [];
                this.dataTable.setSelections([]);
              });
          }
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
            !isUndefined(find(data, v => v.inGroup))
          );
        },
        onClick: data => {
          if (this.subType === DataMap.Resource_Type.vmGroup.value) {
            this.protectService
              .activeGroupProtection(_map(data, 'uuid'), this.subType)
              .subscribe(() => {
                this.dataTable.fetchData();
                this.selectionData = [];
                this.dataTable.setSelections([]);
              });
          } else {
            this.protectService
              .activeProtection(_map(data, 'uuid'))
              .subscribe(() => {
                this.dataTable.fetchData();
                this.selectionData = [];
                this.dataTable.setSelections([]);
              });
          }
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
            (data[0].subType === DataMap.Resource_Type.APSCloudServer.value &&
              !includes(
                [
                  DataMap.ApsaraStackStatus.running.value,
                  DataMap.ApsaraStackStatus.stopped.value
                ],
                data[0].status
              ))
          );
        },
        onClick: data => {
          if (this.subType === DataMap.Resource_Type.vmGroup.value) {
            this.protectService
              .deactiveGroupProtection(
                _map(data, 'uuid'),
                _map(data, 'name'),
                this.subType
              )
              .subscribe(() => {
                this.dataTable.fetchData();
                this.selectionData = [];
                this.dataTable.setSelections([]);
              });
          } else {
            this.protectService
              .deactiveProtection(_map(data, 'uuid'), _map(data, 'name'))
              .subscribe(() => {
                this.dataTable.fetchData();
                this.selectionData = [];
                this.dataTable.setSelections([]);
              });
          }
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
                return !isEmpty(val.sla_id) && hasBackupPermission(val);
              })
            ) !== size(data) ||
            !size(data) ||
            (data[0].subType === DataMap.Resource_Type.APSCloudServer.value &&
              !includes(
                [
                  DataMap.ApsaraStackStatus.running.value,
                  DataMap.ApsaraStackStatus.stopped.value
                ],
                data[0].status
              )) ||
            (this.subType === DataMap.Resource_Type.vmGroup.value &&
              !data[0].resourceCount)
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
        id: 'modify',
        displayCheck: () => {
          return this.subType === DataMap.Resource_Type.vmGroup.value;
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ManuallyBackVM,
        divide: true,
        onClick: ([data]) => this.editGroup([data])
      },
      {
        id: 'delete',
        displayCheck: () => {
          return this.subType === DataMap.Resource_Type.vmGroup.value;
        },
        disableCheck: data => {
          return (
            !size(data) ||
            !isUndefined(
              find(
                data,
                v =>
                  v.protectionStatus ===
                    DataMap.Protection_Status.protected.value ||
                  !hasResourcePermission(v)
              )
            )
          );
        },
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.ManuallyBackVM,
        divide: true,
        onClick: data => this.delete(data)
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
    this.getOptsConfig(cloneDeep(opts));
    this.getOptItems(cloneDeep(opts));
    const colsMap: { [key: string]: TableCols } = {
      uuid: {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      name: {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.isSummary
          ? null
          : {
              type: 'text',
              config: {
                id: 'outerClosable',
                iconPos: 'flow-text',
                click: data => {
                  this.getResourceDetail(data);
                }
              }
            }
      },
      ip: {
        key: 'endpoint',
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      version: {
        key: 'version',
        name: this.i18n.get('common_version_label'),
        cellRender: this.rhvVersionTpl
      },
      status: {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        ...this.getStatusExtParams()
      },
      location: {
        key: 'path',
        name: this.i18n.get('common_location_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      mark: {
        key: 'remark',
        name: this.i18n.get('common_mark_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      numbers: {
        key: 'resourceCount',
        name: includes(
          [
            DataMap.Resource_Type.HCSCloudHost.value,
            DataMap.Resource_Type.openStackCloudServer.value,
            DataMap.Resource_Type.APSCloudServer.value
          ],
          this.subUnitType
        )
          ? this.i18n.get('protection_resource_cloud_numbers_label')
          : this.i18n.get('protection_vms_label')
      },
      sla: {
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
      slaCompliance: {
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
      protectionStatus: {
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
      vmChildren: {
        key: 'children',
        name: this.i18n.get('protection_vms_label')
      },
      hostChilren: {
        key: 'children',
        name: this.i18n.get('protection_vms_label')
      },
      resourceGroupName: {
        key: 'resourceGroupName',
        name: includes(
          [DataMap.Resource_Type.APSCloudServer.value],
          this.subType
        )
          ? this.i18n.get('protection_cloud_group_label')
          : this.i18n.get('protection_vm_group_label'),
        thExtra: this.groupTipTpl
      },
      labelList: {
        key: 'labelList',
        name: this.i18n.get('common_tag_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.resourceTagTpl
      },
      operation: {
        key: 'operation',
        width: 144,
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
    };
    this.tableConfig = {
      table: {
        async: true,
        columns: this.getCols(colsMap),
        rows: this.isSummary
          ? null
          : {
              selectionMode: 'multiple',
              selectionTrigger: 'selector',
              showSelector: true
            },
        compareWith: 'uuid',
        autoPolling: CommonConsts.TIME_INTERVAL,
        scrollFixed: true,
        colDisplayControl: this.isSummary
          ? false
          : {
              ignoringColsType: 'hide'
            },
        fetchData: (filter: Filters, args: {}) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        showPageSizeOptions: !this.isSummary,
        winTablePagination: true
      }
    };
  }

  isProtectable(val) {
    if (
      this.subType !== DataMap.Resource_Type.nutanixVm.value ||
      val.extendInfo?.protectable === 'true'
    ) {
      return true;
    }
    return false;
  }

  clearTable() {
    this.tableData = {
      data: [],
      total: 0
    };
    this.updateTable.emit({ total: 0 });
  }

  getExtConditions() {
    const extParams = {};
    if (
      this.treeSelection?.rootUuid &&
      this.subType !== DataMap.Resource_Type.vmGroup.value
    ) {
      assign(extParams, {
        rootUuid: this.treeSelection?.rootUuid
      });
    }
    if (this.subType === DataMap.Resource_Type.vmGroup.value) {
      assign(extParams, {
        path: this.treeSelection?.path
      });
      assign(extParams, {
        scope_resource_id: this.treeSelection?.uuid
      });
    }
    if (
      includes(
        [
          DataMap.Resource_Type.APSZone.value,
          DataMap.Resource_Type.APSRegion.value
        ],
        this.treeSelection?.subType
      ) &&
      this.subType !== DataMap.Resource_Type.vmGroup.value
    ) {
      assign(extParams, {
        path: [['=~'], this.treeSelection?.path + '/']
      });
    }
    if (
      includes(
        [DataMap.Resource_Type.APSResourceSet.value],
        this.treeSelection?.subType
      )
    ) {
      assign(extParams, {
        resourceSetName: this.treeSelection.name
      });
    }
    if (
      includes(
        [
          DataMap.Resource_Type.cNwareCluster.value,
          DataMap.Resource_Type.cNwareHost.value,
          DataMap.Resource_Type.cNwareHostPool.value,
          DataMap.Resource_Type.nutanixCluster.value,
          DataMap.Resource_Type.nutanixHost.value
        ],
        this.treeSelection?.subType
      ) &&
      this.subType !== DataMap.Resource_Type.vmGroup.value
    ) {
      assign(extParams, {
        path: [['=~'], this.treeSelection?.path]
      });
    }
    if (
      includes(
        [
          DataMap.Resource_Type.hyperVScvmm.value,
          DataMap.Resource_Type.hyperVCluster.value,
          DataMap.Resource_Type.hyperVHost.value
        ],
        this.treeSelection?.subType
      ) &&
      this.subType !== DataMap.Resource_Type.vmGroup.value &&
      this.treeSelection?.parentUuid
    ) {
      assign(extParams, {
        parentUuid: this.treeSelection.uuid
      });
    }
    return extParams;
  }

  search() {
    const filters = this.dataTable.filterMap.filters;
    if (!trim(this.name)) {
      remove(filters, { key: 'name' });
    } else {
      const filterName = find(filters, { key: 'name' });
      if (filterName) {
        filterName.value = [trim(this.name)];
      } else {
        filters.push({
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: [trim(this.name)]
        });
      }
    }
    this.dataTable.setFilterMap(this.dataTable.filterMap);
  }

  extendResult(item) {
    switch (this.subType) {
      case DataMap.Resource_Type.cNwareCluster.value:
      case DataMap.Resource_Type.cNwareHost.value:
      case DataMap.Resource_Type.cNwareVm.value:
      case DataMap.Resource_Type.nutanixCluster.value:
      case DataMap.Resource_Type.nutanixHost.value:
        assign(item, JSON.parse(item.extendInfo?.details));
        break;
      case DataMap.Resource_Type.APSCloudServer.value:
        assign(item, {
          status: item.extendInfo.status
        });
        break;
      case DataMap.Resource_Type.hyperVHost.value:
        assign(item, {
          status: item.extendInfo?.State
        });
        break;
      case DataMap.Resource_Type.nutanixVm.value:
        assign(item, JSON.parse(item.extendInfo?.details));
        assign(item, { remark: item.description });
        break;
      default:
        break;
    }
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
      subType: [this.subType],
      ...this.getExtConditions()
    };
    if (this.subType === DataMap.Resource_Type.vmGroup.value) {
      delete defaultConditions.subType;
      assign(defaultConditions, { source_sub_type: this.subUnitType });
    }

    // 对于sla遵从度的全选状态，要求要把所有数据展示，因此全选等于没有过滤数据，在url.interceptor.ts文件中统一处理
    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (this.subType === DataMap.Resource_Type.vmGroup.value) {
        if (conditionsTemp.protectionStatus) {
          assign(conditionsTemp, {
            protection_status: conditionsTemp.protectionStatus
          });
          delete conditionsTemp.protectionStatus;
        }
        if (conditionsTemp?.protectedObject?.slaName) {
          assign(conditionsTemp.protectedObject, {
            sla_name: conditionsTemp?.protectedObject?.slaName
          });
          delete conditionsTemp?.protectedObject?.slaName;
        }
      }

      if (
        this.subType === DataMap.Resource_Type.hyperVHost.value &&
        conditionsTemp.status
      ) {
        assign(conditionsTemp, {
          linkStatus: conditionsTemp.status
        });
        delete conditionsTemp.status;
      }
      if (conditionsTemp.labelList) {
        assign(conditionsTemp, {
          labelCondition: {
            labelName: conditionsTemp.labelList[1]
          }
        });
        delete conditionsTemp.labelList;
      }
      if (conditionsTemp.status) {
        if (
          includes(
            conditionsTemp.status,
            DataMap.nutanixHostStatus.new.value
          ) &&
          this.subType === DataMap.Resource_Type.nutanixHost.value
        ) {
          assign(conditionsTemp, {
            status: [
              ['in'],
              ...tail(conditionsTemp.status),
              DataMap.nutanixHostStatus.notDetachable.value,
              DataMap.nutanixHostStatus.detachable.value
            ]
          });
        }
        if (
          includes(
            conditionsTemp.status,
            DataMap.nutanixVmStatus.unknown.value
          ) &&
          this.subType === DataMap.Resource_Type.nutanixVm.value
        ) {
          assign(conditionsTemp, {
            status: [
              ['in'],
              ...tail(conditionsTemp.status),
              DataMap.nutanixVmStatus.paused.value,
              DataMap.nutanixVmStatus.poweringOn.value,
              DataMap.nutanixVmStatus.shuttingDown.value,
              DataMap.nutanixVmStatus.poweringOff.value,
              DataMap.nutanixVmStatus.suspending.value,
              DataMap.nutanixVmStatus.suspended.value,
              DataMap.nutanixVmStatus.resuming.value,
              DataMap.nutanixVmStatus.resetting.value,
              DataMap.nutanixVmStatus.mirgrating.value
            ]
          });
        }
      }
      assign(defaultConditions, conditionsTemp);
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }
    const queryFunc: Observable<any> =
      this.subType === DataMap.Resource_Type.vmGroup.value
        ? this.protectedResourceApiService.ListResourceGroups(params)
        : this.protectedResourceApiService.ListResources(params);
    queryFunc.subscribe(res => {
      each(res.records, (item: any) => {
        // 获取标签数据
        const { showList, hoverList } = getLabelList(item);
        extendSlaInfo(item);
        this.extendResult(item);
        assign(item, {
          sub_type: item?.subType,
          showLabelList: showList,
          hoverLabelList: hoverList
        });
      });

      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      if (isEmpty(filters.conditions_v2)) {
        this.updateTable.emit({ total: res.totalCount });
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

  protect(datas, action: ProtectResourceAction) {
    this.protectService.openProtectModal(
      this.subType,
      action,
      {
        width: 780,
        data: datas,
        onOK: () => {
          this.dataTable.fetchData();
          this.selectionData = [];
          this.dataTable.setSelections([]);
        }
      },
      this.subUnitType
    );
  }

  getResourceDetail(data) {
    this.currentDetailUuid = data.uuid;
    if (
      includes(
        [
          DataMap.Resource_Type.cNwareHost.value,
          DataMap.Resource_Type.cNwareCluster.value,
          DataMap.Resource_Type.nutanixHost.value,
          DataMap.Resource_Type.nutanixCluster.value
        ],
        this.subType
      )
    ) {
      this.drawModalService.openDetailModal({
        ...MODAL_COMMON.drawerOptions,
        lvModalKey: 'cnware-detail-modal',
        lvOkDisabled: true,
        lvHeader: data.name,
        lvContent: includes(
          [
            DataMap.Resource_Type.nutanixHost.value,
            DataMap.Resource_Type.nutanixCluster.value
          ],
          this.subType
        )
          ? nutanixSummaryComponent
          : SummaryComponent,
        lvComponentParams: {
          item: {
            ...data,
            optItems: getTableOptsItems(cloneDeep(this.optItems), data, this),
            optItemsFn: v => {
              return getTableOptsItems(cloneDeep(this.optItems), v, this);
            }
          }
        },
        lvWidth: 1150,
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ],
        lvAfterClose: () => {
          // 关闭详情框，发布全局消息
          this.globalService.emitStore({
            action: 'detailModalClose',
            state: true
          });
        }
      });
    } else if (this.subType === DataMap.Resource_Type.vmGroup.value) {
      this.detailService.openDetailModal(DataMap.Resource_Type.vmGroup.value, {
        data: assign(cloneDeep(data), {
          optItems: getTableOptsItems(cloneDeep(this.optItems), data, this),
          optItemsFn: v => {
            return getTableOptsItems(cloneDeep(this.optItems), v, this);
          }
        })
      });
    } else {
      this.detailService.openDetailModal(data.subType, {
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
        this.dataTable.fetchData();
        this.selectionData = [];
        this.dataTable.setSelections([]);
      });
    }
  }

  delete(datas) {
    if (size(datas) === 1) {
      this.warningMessageService.create({
        content: this.i18n.get('protection_nas_share_delete_label'),
        onOK: () => {
          this.protectedResourceApiService
            .DeleteResourceGroup({ resourceGroupId: datas[0].uuid })
            .subscribe(res => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get('protection_nas_share_delete_label'),
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item => {
              // 调用删除接口
              return this.protectedResourceApiService.DeleteResourceGroup({
                resourceGroupId: item.uuid,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            cloneDeep(this.selectionData),
            () => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            }
          );
        }
      });
    }
  }

  // 编辑虚拟机组数据
  editGroup(datas?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.largeWidth + 200,
        lvOkDisabled: datas ? false : true,
        lvHeader: datas
          ? this.i18n.get('common_modify_label')
          : this.i18n.get('common_create_label'),
        lvContent: CreateGroupComponent,
        lvComponentParams: {
          rowData: datas,
          treeSelection: this.treeSelection,
          subUnitType: this.subUnitType
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateGroupComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest([
            content.formGroup.statusChanges,
            content.selectValid$
          ]);
          combined.subscribe(latestValues => {
            const [formGroupStatus, valid] = latestValues;
            modalIns.lvOkDisabled = !valid || formGroupStatus !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as CreateGroupComponent;
          // 调用创建、修改接口
          return new Promise(resolve => {
            content.onOK().subscribe(
              res => {
                resolve(true);
                // 刷新表格数据
                this.dataTable.fetchData();
              },
              err => resolve(false)
            );
          });
        }
      }
    });
  }
}
