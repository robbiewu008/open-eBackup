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
  disableDeactiveProtectionTips,
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
  OperateItems,
  ProtectedResourceApiService,
  ProtectResourceAction,
  ResourceType,
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
  get,
  includes,
  isEmpty,
  isUndefined,
  map,
  mapValues,
  reject,
  size,
  some,
  tail,
  trim
} from 'lodash';
import { GetLabelOptionsService } from '../../../../../shared/services/get-labels.service';

@Component({
  selector: 'aui-base-template',
  templateUrl: './base-template.component.html',
  styleUrls: ['./base-template.component.less']
})
export class BaseTemplateComponent implements OnInit, AfterViewInit {
  @Input() resourceSubType: string;
  @Input() extraConfig: any;
  @Input() columns: TableCols[];

  name;
  optsConfig: ProButton[];
  selectionData: any[];
  tableConfig: TableConfig;
  tableData: TableData;
  optItems: ProButton[];
  maxDisplayItems = 2;
  currentDetailUuid;

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private protectService: ProtectService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private takeManualBackupService: TakeManualBackupService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService,
    private getLabelOptionsService: GetLabelOptionsService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.setTableScroll();
    this.initConfig();
  }

  setTableScroll() {
    const otherHeight = includes(
      [
        DataMap.Resource_Type.HCSCloudHost.value,
        DataMap.Resource_Type.MongoDB.value
      ],
      this.resourceSubType
    )
      ? 340
      : 400;
    this.virtualScroll.getScrollParam(otherHeight);
  }

  initConfig() {
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
            .activeProtection(map(data, 'uuid'))
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
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
            ) !== size(data) ||
            !size(data) ||
            size(data) > CommonConsts.DEACTIVE_PROTECTION_MAX
          );
        },
        disabledTipsCheck: data =>
          disableDeactiveProtectionTips(data, this.i18n),
        permission: OperateItems.DeactivateProtection,
        label: this.i18n.get('protection_deactive_protection_label'),
        onClick: data => {
          this.protectService
            .deactiveProtection(map(data, 'uuid'), map(data, 'name'))
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
        onClick: data => {
          this.getResourceDetail({
            ...data[0],
            activeId: 'copydata',
            datePickerMode: DATE_PICKER_MODE.DATE
          });
          this.currentDetailUuid = data[0]?.uuid;
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
      connectivityTest: {
        id: 'connectivityTest',
        divide: true,
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('protection_connectivity_test_label'),
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => {
          this.protectedResourceApiService
            .CheckProtectedResource({ resourceId: data[0].uuid })
            .subscribe(res => {
              let returnRes;
              try {
                returnRes = JSON.parse(res);
              } catch (error) {
                returnRes = [];
              }
              const idx = returnRes.findIndex(item => item.code !== 0);
              if (idx !== -1) {
                this.messageService.error(this.i18n.get(returnRes[idx].code), {
                  lvMessageKey: 'errorKey',
                  lvShowCloseButton: true
                });
              } else {
                this.messageService.success(
                  this.i18n.get('common_operate_success_label'),
                  {
                    lvMessageKey: 'successKey',
                    lvShowCloseButton: true
                  }
                );
              }
            });
        }
      },
      delete: {
        id: 'delete',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return isEmpty(val.sla_id) && hasResourcePermission(val);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.DeleteDatabase,
        label: this.i18n.get('common_delete_label'),
        onClick: data => {
          this.deleteRes(data);
        }
      },
      addTag: {
        id: 'addTag',
        permission: OperateItems.AddTag,
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        label: this.i18n.get('common_add_tag_label'),
        onClick: data => this.addTag(data)
      },
      removeTag: {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(data)
      }
    };

    this.optItems = [
      btns.protect,
      btns.modifyProtect,
      assign(cloneDeep(btns.removeProtection), {
        divide: true
      }),
      btns.activeProtection,
      assign(cloneDeep(btns.deactiveProtection), {
        divide: true
      }),
      btns.manualBackup,
      btns.addTag,
      btns.removeTag
    ];

    if (
      this.resourceSubType === DataMap.Resource_Type.KubernetesStatefulset.value
    ) {
      this.optItems = [...this.optItems, btns.recovery];
    }

    if (this.resourceSubType === DataMap.Resource_Type.MongoDB.value) {
      this.optItems = [
        ...this.optItems,
        assign(cloneDeep(btns.recovery), {
          divide: true
        }),
        btns.connectivityTest,
        this.extraConfig?.modify,
        btns.delete
      ];
    }

    if (
      includes(
        [
          DataMap.Resource_Type.kubernetesNamespaceCommon.value,
          DataMap.Resource_Type.HCSCloudHost.value
        ],
        this.resourceSubType
      )
    ) {
      this.optItems = [...this.optItems, btns.recovery];
    }

    if (
      this.resourceSubType ===
      DataMap.Resource_Type.kubernetesDatasetCommon.value
    ) {
      this.optItems = [
        ...this.optItems,
        assign(cloneDeep(btns.recovery), {
          divide: true
        }),
        this.extraConfig?.modify,
        btns.delete
      ];
    }

    this.optItems = getPermissionMenuItem(this.optItems);

    let baseColumns: TableCols[] = [
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
      // 新增标签
      {
        key: 'labelList',
        name: this.i18n.get('common_tag_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: false,
          showSearch: true,
          options: () => this.getLabelOptionsService.getLabelOptions()
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
        click: data => {
          this.getResourceDetail(data);
          this.currentDetailUuid = data.uuid;
        }
      });
    }

    this.optsConfig = getPermissionMenuItem([
      btns.protect,
      btns.removeProtection,
      btns.activeProtection,
      btns.deactiveProtection,
      btns.manualBackup,
      btns.addTag,
      btns.removeTag
    ]);

    if (
      includes(
        [
          DataMap.Resource_Type.MongoDB.value,
          DataMap.Resource_Type.kubernetesDatasetCommon.value
        ],
        this.resourceSubType
      )
    ) {
      this.maxDisplayItems = 3;
      this.optsConfig = getPermissionMenuItem([
        this.extraConfig?.register,
        btns.protect,
        btns.removeProtection,
        btns.activeProtection,
        btns.deactiveProtection,
        btns.manualBackup,
        btns.delete,
        btns.addTag,
        btns.removeTag
      ]);
    }

    if (this.resourceSubType === DataMap.Resource_Type.HCSCloudHost.value) {
      this.maxDisplayItems = 3;
      this.optsConfig = getPermissionMenuItem([
        this.extraConfig?.scan,
        btns.protect,
        btns.removeProtection,
        btns.activeProtection,
        btns.deactiveProtection,
        btns.manualBackup,
        btns.addTag,
        btns.removeTag
      ]);
    }

    this.tableConfig = {
      table: {
        async: true,
        columns: [...this.columns, ...baseColumns],
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

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data,
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
      rowDatas: data,
      type: SetTagType.Resource,
      onOk: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
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

    if (this.resourceSubType === DataMap.Resource_Type.MongoDB.value) {
      assign(defaultConditions, {
        subType: [
          DataMap.Resource_Type.MongodbSingleInstance.value,
          DataMap.Resource_Type.MongodbClusterInstance.value
        ],
        isTopInstance: [['=='], '1']
      });
    }

    if (this.resourceSubType === DataMap.Resource_Type.HCSCloudHost.value) {
      delete defaultConditions.subType;
      assign(defaultConditions, {
        type: 'CloudHost',
        path: [
          ['~~'],
          decodeURI(
            get(window, 'parent.hcsData.ProjectName', '') ||
              this.cookieService.get('projectName')
          )
        ]
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
      if (
        conditionsTemp.status &&
        this.resourceSubType === DataMap.Resource_Type.HCSCloudHost.value
      ) {
        if (
          includes(
            conditionsTemp.status,
            DataMap.HCS_Host_LinkStatus.error.value
          )
        ) {
          assign(conditionsTemp, {
            status: [
              ['in'],
              ...tail(conditionsTemp.status),
              DataMap.HCS_Host_LinkStatus.abnormal2.value,
              DataMap.HCS_Host_LinkStatus.abnormal3.value,
              DataMap.HCS_Host_LinkStatus.abnormal4.value,
              DataMap.HCS_Host_LinkStatus.abnormal5.value,
              DataMap.HCS_Host_LinkStatus.reboot1.value,
              DataMap.HCS_Host_LinkStatus.hardReboot2.value,
              DataMap.HCS_Host_LinkStatus.rebuild.value
            ]
          });
        }
      }
      if (conditionsTemp.labelList) {
        conditionsTemp.labelList.shift();
        assign(conditionsTemp, {
          labelCondition: {
            labelList: conditionsTemp.labelList
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
        assign(item, {
          sub_type: item.subType,
          cluster: item.environment?.name,
          endpoint: item.environment?.endpoint,
          parentName: item.parentName,
          showLabelList: showList,
          hoverLabelList: hoverList
        });
        extendSlaInfo(item);
        if (this.resourceSubType === DataMap.Resource_Type.MongoDB.value) {
          assign(item, {
            clusterType: item.extendInfo?.clusterType,
            singleType:
              item.extendInfo?.singleType ||
              DataMap.mongoDBSingleInstanceType.single.value
          });
        }
        if (this.resourceSubType === DataMap.Resource_Type.HCSCloudHost.value) {
          assign(item, {
            status:
              item.extendInfo?.status ||
              JSON.parse(item.extendInfo?.host || '{}')?.status,
            isWorkspace: item.extendInfo?.isWorkspace || '0',
            computerName: item.extendInfo?.computerName,
            vm_ip: item.extendInfo?.vm_ip
          });
        }
      });
      this.tableData = {
        data: res.records,
        total: res.totalCount
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
    });
  }

  protect(datas, action: ProtectResourceAction, header?: string, refreshData?) {
    if (
      action === ProtectResourceAction.Create &&
      this.resourceSubType === DataMap.Resource_Type.HCSCloudHost.value
    ) {
      let flag = false;
      each(datas, item => {
        const diskInfos = JSON.parse(item.extendInfo?.host || '{}');
        if (isEmpty(diskInfos.diskInfo)) {
          flag = true;
        }
      });

      if (flag) {
        this.messageService.error(
          this.i18n.get('common_select_hcs_disk_protect_label'),
          {
            lvMessageKey: 'hcs_disk_protect',
            lvShowCloseButton: true
          }
        );

        return;
      }
    }
    this.protectService.openProtectModal(
      this.resourceSubType === DataMap.Resource_Type.HCSCloudHost.value
        ? ResourceType.CLOUD_HOST
        : this.resourceSubType,
      action,
      {
        width: 780,
        data: datas,
        onOK: () => {
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.dataTable.fetchData();
        }
      }
    );
  }

  getResourceDetail(resource: any) {
    this.detailService.openDetailModal(this.resourceSubType, {
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

  deleteRes(data) {
    if (size(data) === 1) {
      this.warningMessageService.create({
        content: this.i18n.get(
          'protection_resource_delete_label',
          [data[0].name],
          false,
          true
        ),
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
        content: this.i18n.get(
          'protection_resource_delete_label',
          [map(data, 'name').join(',')],
          false,
          true
        ),
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
}
