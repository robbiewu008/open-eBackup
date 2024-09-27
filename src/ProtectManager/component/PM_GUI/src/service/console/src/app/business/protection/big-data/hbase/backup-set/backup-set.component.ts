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
import {
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  getTableOptsItems,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectResourceAction,
  ProtectResourceCategory,
  WarningMessageService,
  ProtectedResourceApiService,
  DATE_PICKER_MODE,
  extendSlaInfo,
  GROUP_COMMON,
  RoleOperationMap,
  hasProtectPermission,
  hasRecoveryPermission,
  hasBackupPermission,
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
  trim,
  some
} from 'lodash';
import { map } from 'rxjs/operators';
import { CreateBackupSetComponent } from './create-backup-set/create-backup-set.component';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { CreateBackupsetComponent as CreateHiveBackupSetComponent } from '../../hive/create-backupset/create-backupset.component';
import { CreateBackupsetComponent as CreateElasticSearchBackupSetComponent } from '../../elasticSearch/create-backupset/create-backupset.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';

@Component({
  selector: 'aui-backup-set',
  templateUrl: './backup-set.component.html',
  styleUrls: ['./backup-set.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class BackupSetComponent implements OnInit, AfterViewInit {
  @Input() resSubType;
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
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    public virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private batchOperateService: BatchOperateService,
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
          this.createBackupSet();
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
          this.createBackupSet(data[0]);
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
        key: 'cluster_name',
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
      table: {
        size: 'default',
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

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const environmentObj = this.isClusterDetail
      ? {
          environment: {
            uuid: this.data.uuid
          }
        }
      : {};

    if (!isEmpty(filters.conditions_v2)) {
      const conditions_v2 = JSON.parse(filters.conditions_v2);
      if (conditions_v2.cluster_name) {
        conditions_v2['environment'] = {
          name: conditions_v2.cluster_name
        };
        delete conditions_v2.cluster_name;
      }
      if (conditions_v2.labelList) {
        conditions_v2['labelCondition'] = {
          labelName: conditions_v2.labelList[1]
        };

        delete conditions_v2.labelList;
      }
      assign(params, {
        conditions: JSON.stringify(
          assign(
            {
              ...conditions_v2,
              subType:
                this.resSubType || DataMap.Resource_Type.HBaseBackupSet.value
            },
            environmentObj
          )
        )
      });
    } else {
      assign(params, {
        conditions: JSON.stringify(
          assign(
            {
              subType:
                this.resSubType || DataMap.Resource_Type.HBaseBackupSet.value
            },
            environmentObj
          )
        )
      });
    }

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
              ip: item.extendInfo?.ip,
              shareMode: item.extendInfo?.shareMode,
              equipment: item.environment ? item.environment['name'] : '',
              cluster_name: item.environment ? item.environment['name'] : '',
              equipmentType: item.environment
                ? item.environment['subType']
                : '',
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

  getResourceDetail(params) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          uuid: params.uuid,
          subType:
            this.resSubType ||
            params.subType ||
            params.sub_type ||
            DataMap.Resource_Type.HBaseBackupSet.value
        })
      })
      .pipe(
        map(res => {
          return first(res.records);
        })
      )
      .subscribe((item: any) => {
        item['sub_type'] = item.subType || item.sub_type;
        item['environment_uuid'] = item.environment?.uuid;
        item['environment_name'] = item.environment?.name;
        item['environment_endpoint'] = item.environment?.endpoint;
        if (params.activeId) {
          item['activeId'] = params.activeId;
          item['datePickerMode'] = params.datePickerMode;
        }
        extendSlaInfo(item);
        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'slaDetailModalKey'
          )
        ) {
          this.drawModalService.destroyModal('slaDetailModalKey');
        }
        this.detailService.openDetailModal(
          this.resSubType || DataMap.Resource_Type.HBaseBackupSet.value,
          {
            data: assign(
              item,
              {
                optItems: getTableOptsItems(
                  cloneDeep(this.optItems),
                  item,
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

  createBackupSet(data?) {
    if (
      this.resSubType === DataMap.Resource_Type.HiveBackupSet.value ||
      data?.sub_type === DataMap.Resource_Type.HiveBackupSet.value
    ) {
      this.createHiveBackupSet(data);
    } else if (
      this.resSubType === DataMap.Resource_Type.ElasticsearchBackupSet.value ||
      data?.sub_type === DataMap.Resource_Type.ElasticsearchBackupSet.value
    ) {
      this.createElasticsearchBackupSet(data);
    } else {
      this.drawModalService.create({
        ...MODAL_COMMON.generateDrawerOptions(),
        lvHeader: data
          ? this.i18n.get('common_modify_label')
          : this.i18n.get('common_create_label'),
        lvContent: CreateBackupSetComponent,
        lvOkDisabled: true,
        lvWidth: MODAL_COMMON.largeModal,
        lvComponentParams: {
          data
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateBackupSetComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateBackupSetComponent;
            content.onOK().subscribe({
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
  }

  createHiveBackupSet(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data
        ? this.i18n.get('common_modify_label')
        : this.i18n.get('common_create_label'),
      lvContent: CreateHiveBackupSetComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.smallModal,
      lvComponentParams: {
        data
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateHiveBackupSetComponent;
          content.onOK().subscribe({
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

  createElasticsearchBackupSet(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data
        ? this.i18n.get('common_modify_label')
        : this.i18n.get('common_create_label'),
      lvContent: CreateElasticSearchBackupSetComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvComponentParams: {
        data
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateHiveBackupSetComponent;
          content.onOK().subscribe({
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

  deleteResource(data) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_dorado_system_delete_label', [
        _map(data, 'name').toString()
      ]),
      onOK: () => {
        if (size(data) <= 1) {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: data[0].uuid
            })
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            });
          return;
        }
        this.batchOperateService.selfGetResults(
          item => {
            return this.protectedResourceApiService.DeleteResource({
              resourceId: item.uuid,
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            });
          },
          _map(cloneDeep(data), item => {
            return assign(item, {
              name: item.name,
              isAsyn: false
            });
          }),
          () => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          }
        );
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
      this.takeManualBackupService.execute(datas[0], () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      });
    }
  }

  protect(datas, action: ProtectResourceAction) {
    const data = size(datas) > 1 ? datas : datas[0];
    this.protectService.openProtectModal(
      ProtectResourceCategory.HBase,
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
}
