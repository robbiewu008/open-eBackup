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
  OnDestroy,
  OnInit,
  Output,
  SimpleChanges,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  DataMap,
  DataMapService,
  extendSlaInfo,
  GlobalService,
  I18NService,
  ProtectedResourceApiService,
  ResourceSetApiService,
  ResourceSetType,
  ResourceType,
  VirtualResourceService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { SlaService } from 'app/shared/services/sla.service';
import {
  assign,
  cloneDeep,
  defer,
  differenceBy,
  each,
  find,
  includes,
  isEmpty,
  isUndefined,
  map,
  set,
  size
} from 'lodash';
import { Observable, Subscription } from 'rxjs';

@Component({
  selector: 'aui-base-table-template',
  templateUrl: './base-table-template.component.html',
  styleUrls: ['./base-table-template.component.less']
})
export class BaseTableTemplateComponent
  implements OnInit, OnChanges, OnDestroy, AfterViewInit {
  @Input() subUnitType;
  @Input() subType: string;
  @Input() treeSelection: any;
  @Input() extParams;
  @Input() ID;
  @Input() allSelectionMap;
  @Input() appType;
  @Input() data; // 修改时传入的data
  @Input() isDetail;
  @Input() allSelect;
  @Input() showSelect;
  @Output() updateTable = new EventEmitter();
  @Output() baseSelectChange = new EventEmitter<any>();

  resType = DataMap.Resource_Type; // 本页面用到了非常多所以缩减长度
  tableConfig: TableConfig;
  tableData: TableData;
  selectionData: any = [];
  name: string;
  isVmware = false;
  isFc = false;
  isHcs = false;
  isOpenstack = false;
  isVmGroup = false; // 资源组
  parentRelatedFetch$: Subscription = new Subscription();
  syncData$: Subscription = new Subscription();

  groupTipLabel = this.i18n.get('protection_vm_group_tip_label');

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('groupTipTpl', { static: true })
  groupTipTpl: TemplateRef<any>;
  @ViewChild('hypervStatusTpl', { static: true })
  hypervStatusTpl: TemplateRef<any>;
  @ViewChild('rhvVersionTpl', { static: true })
  rhvVersionTpl: TemplateRef<any>;

  constructor(
    public globalService: GlobalService,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private resourceSetService: ResourceSetApiService,
    private appUtilsService: AppUtilsService,
    private virtualResourceService: VirtualResourceService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private slaService: SlaService
  ) {}

  ngOnInit() {
    this.isVmware = this.appType === ResourceSetType.VMware;
    this.isFc = this.appType === ResourceSetType.FusionCompute;
    this.isHcs = this.appType === ResourceSetType.HCSStack;
    this.isOpenstack = this.appType === ResourceSetType.OpenStack;
    this.isVmGroup = this.subType === this.resType.vmGroup.value;
    this.initConfig();
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.treeSelection && this.isVmGroup) {
      // 云平台的虚拟机组最初加载不出dataTable
      defer(() => {
        this.dataTable.stopPolling();
        this.dataTable.fetchData();
      });
    }
    if (changes.treeSelection && this.dataTable && !this.isVmGroup) {
      this.dataTable.stopPolling();
      this.dataTable.fetchData();
    }

    if (changes.showSelect && this.dataTable) {
      // 不是资源组采用这个逻辑
      if (!this.isVmGroup) {
        this.selectionData = cloneDeep(this.allSelectionMap[this.appType].data);
        this.dataTable.setSelections(cloneDeep(this.selectionData));
      } else {
        this.getSelectedData();
      }
    }

    if (changes.allSelect?.currentValue && this.dataTable) {
      // 全选
      this.selectionData = this.tableData.data;
      each(this.tableData.data, item => {
        item.disabled = true;
      });
      this.dataTable.setSelections(cloneDeep(this.selectionData));
    }

    if (
      !changes.allSelect?.currentValue &&
      this.dataTable &&
      changes.allSelect
    ) {
      // 取消全选，并且不能把正常切换的选择项给清掉
      this.selectionData = [];
      each(this.tableData?.data, item => {
        item.disabled = false;
      });
      this.dataTable.setSelections(cloneDeep(this.selectionData));
    }
  }

  ngOnDestroy() {
    this.parentRelatedFetch$.unsubscribe();
    this.syncData$.unsubscribe();
  }

  ngAfterViewInit() {
    this.getFetchState();
  }

  getFetchState() {
    this.parentRelatedFetch$ = this.globalService
      .getState(`${this.appType}parentSelect`)
      .subscribe(res => {
        if (
          !isEmpty(this.allSelectionMap[this.appType]?.data) &&
          !this.allSelectionMap[this.appType]?.isAllSelected
        ) {
          this.parentSelectChild();
        }
      });

    this.syncData$ = this.globalService
      .getState(`${this.appType}syncData`)
      .subscribe(res => {
        this.selectionData = this.selectionData.filter(item =>
          find(this.allSelectionMap[this.appType].data, { uuid: item.uuid })
        );
        this.dataTable.setSelections(cloneDeep(this.selectionData));
      });
  }

  getSelectedData() {
    const params: any = {
      resourceSetId: this.data[0].uuid,
      scopeModule: this.appType,
      type: ResourceSetType.RESOURCE_GROUP
    };
    this.resourceSetService.QueryResourceObjectIdList(params).subscribe(res => {
      set(this.allSelectionMap, ResourceSetType.RESOURCE_GROUP, {
        data: map(res, item => {
          return { uuid: item };
        })
      });
    });
  }

  initConfig() {
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
        cellRender: this.isDetail
          ? null
          : {
              type: 'text',
              config: {
                id: 'outerClosable',
                iconPos: 'flow-text'
              }
            }
      },
      projectNum: {
        key: 'projectCount',
        name: this.isHcs
          ? this.i18n.get('common_project_num_label')
          : this.i18n.get('protection_project_count_label')
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
      osType: {
        key: 'os_type',
        name: this.i18n.get('protection_os_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('vmwareOsType')
        },
        cellRender: {
          type: 'status',

          config: this.dataMapService.toArray('vmwareOsType')
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
            this.resType.HCSCloudHost.value,
            this.resType.openStackCloudServer.value,
            this.resType.APSCloudServer.value
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
          [this.resType.APSCloudServer.value, this.resType.HCSCloudHost.value],
          this.subType
        )
          ? this.i18n.get('protection_cloud_group_label')
          : this.i18n.get('protection_vm_group_label'),
        thExtra: this.groupTipTpl
      }
    };
    this.tableConfig = {
      table: {
        async: true,
        columns: this.getCols(colsMap),
        rows: this.isDetail
          ? null
          : {
              selectionMode: 'multiple',
              selectionTrigger: 'selector',
              showSelector: true
            },
        compareWith: 'uuid',
        scrollFixed: true,
        scroll: { y: '400px' },
        colDisplayControl: this.isDetail
          ? false
          : {
              ignoringColsType: 'hide'
            },
        fetchData: (filter: Filters, args: {}) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          let currentType = this.isVmGroup
            ? ResourceSetType.RESOURCE_GROUP
            : this.appType;
          if (this.isVmGroup) {
            selection = selection.map(item =>
              assign(item, {
                scopeType: this.appType
              })
            );
          }
          if (this.selectionData.length < selection.length) {
            // 新增选中处理
            this.selectionData = selection;
            if (isEmpty(this.allSelectionMap[currentType]?.data)) {
              set(this.allSelectionMap, currentType, {
                data: cloneDeep(this.selectionData)
              });
            } else {
              this.parseSelected(currentType);
            }
          } else {
            // 减少选中时的处理
            const diffArray = differenceBy(
              this.selectionData,
              selection,
              'uuid'
            );
            this.allSelectionMap[currentType].data = this.allSelectionMap[
              currentType
            ].data.filter(item => {
              return !find(diffArray, { uuid: item.uuid });
            });
            this.selectionData = selection;
          }
          // 修改状态下需要在所有的selectionData都把取消的那个删掉，不然父子关联会有问题，导致selectionMap里面没有真正删除
          if (!!this.data) {
            this.globalService.emitStore({
              action: `${this.appType}syncData`,
              state: true
            });
          }
          // 触发父子关联选择
          defer(() => {
            this.globalService.emitStore({
              action: `${this.appType}parentSelect`,
              state: true
            });
          });
          this.baseSelectChange.emit();
        },
        trackByFn: (_, item) => {
          return item.uuid;
        }
      }
    };
  }

  parseSelected(currentType) {
    each(this.selectionData, item => {
      if (!find(this.allSelectionMap[currentType].data, { uuid: item.uuid })) {
        this.allSelectionMap[currentType].data.push(item);
      }
    });
  }

  getStatusExtParams(): any {
    // 映射对应的状态表
    const statusMap = {
      [this.resType.cNwareVm.value]: 'cnwareLinkStatus',
      [this.resType.cNwareHost.value]: 'cnwareLinkStatus',
      [this.resType.APSCloudServer.value]: 'ApsaraStackStatus',
      [this.resType.hyperVVm.value]: 'hypervStatus',
      [this.resType.hyperVHost.value]: 'resource_LinkStatus_Special',
      [this.resType.hostSystem.value]: 'resource_LinkStatus',
      [this.resType.virtualMachine.value]: 'vm_LinkStatus',
      [this.resType.fusionComputeVirtualMachine.value]: 'fcVMLinkStatus',
      [this.resType.HCSCloudHost.value]: 'HCS_Host_LinkStatus',
      [this.resType.openStackCloudServer.value]: 'HCS_Host_LinkStatus'
    };
    let specialTpl; // 处理特殊的cellRender
    switch (this.subType) {
      case this.resType.hyperVVm.value:
        specialTpl = this.hypervStatusTpl;
        break;
      default:
        null;
    }
    if (!statusMap[this.subType]) {
      return {};
    }
    return {
      cellRender: !!specialTpl
        ? specialTpl
        : {
            type: 'status',
            config: this.dataMapService.toArray(statusMap[this.subType])
          },
      filter: {
        type: 'select',
        isMultiple: true,
        showCheckAll: true,
        options: this.dataMapService
          .toArray(statusMap[this.subType])
          .filter(item => {
            return this.isFc
              ? [
                  DataMap.fcVMLinkStatus.running.value,
                  DataMap.fcVMLinkStatus.stopped.value,
                  DataMap.fcVMLinkStatus.unknown.value
                ].includes(item.value)
              : this.isOpenstack || this.isHcs
              ? [
                  DataMap.HCS_Host_LinkStatus.normal.value,
                  DataMap.HCS_Host_LinkStatus.offline.value,
                  DataMap.HCS_Host_LinkStatus.suspended.value,
                  DataMap.HCS_Host_LinkStatus.error.value,
                  DataMap.HCS_Host_LinkStatus.softDelete.value
                ].includes(item.value)
              : true;
          })
      }
    };
  }

  getCols(cols: { [key: string]: TableCols }): TableCols[] {
    const baseClos = [cols.sla, cols.slaCompliance, cols.protectionStatus];
    const nameCols = [cols.uuid, cols.name];
    switch (this.subType) {
      case this.resType.cNwareCluster.value:
        return [...nameCols, cols.location, cols.sla, cols.protectionStatus];
      case this.resType.cNwareHost.value:
        return [
          ...nameCols,
          cols.location,
          cols.status,
          cols.sla,
          cols.protectionStatus
        ];
      case this.resType.cNwareVm.value:
        return [
          ...nameCols,
          cols.location,
          cols.status,
          cols.mark,
          ...baseClos,
          cols.resourceGroupName
        ];
      case this.resType.APSResourceSet.value:
        return [...nameCols, ...baseClos];
      case this.resType.APSZone.value:
        return [...nameCols, cols.location, ...baseClos];
      case this.resType.APSCloudServer.value:
        return [
          ...nameCols,
          cols.location,
          cols.status,
          ...baseClos,
          cols.resourceGroupName
        ];
      case this.resType.hyperVVm.value:
        return [
          ...nameCols,
          cols.version,
          cols.status,
          cols.location,
          ...baseClos
        ];
      case this.resType.hyperVCluster.value:
        return [...nameCols, cols.location];
      case this.resType.hyperVHost.value:
        return [...nameCols, cols.ip, cols.status, ...baseClos];
      case this.resType.vmGroup.value:
        return [...nameCols, cols.numbers, ...baseClos];
      case this.resType.clusterComputeResource.value:
        return [...nameCols, cols.sla, cols.protectionStatus, cols.vmChildren];
      case this.resType.hostSystem.value:
        return [
          ...nameCols,
          cols.location,
          cols.status,
          cols.sla,
          cols.protectionStatus,
          cols.vmChildren
        ];
      case this.resType.virtualMachine.value:
        return [
          ...nameCols,
          cols.location,
          cols.status,
          cols.osType,
          cols.mark,
          ...baseClos,
          cols.resourceGroupName
        ];
      case this.resType.fusionComputeVirtualMachine.value:
        return [
          ...nameCols,
          cols.location,
          cols.status,
          ...baseClos,
          cols.resourceGroupName
        ];
      case DataMap.Job_Target_Type.FusionComputeHost.value:
      case this.resType.HCSProject.value:
      case this.resType.openStackProject.value:
        return [...nameCols, cols.location, ...baseClos];
      case this.resType.HCSTenant.value:
      case ResourceType.OpenStackDomain:
        return [...nameCols, cols.projectNum];
      case this.resType.HCSCloudHost.value:
      case this.resType.openStackCloudServer.value:
        return [
          ...nameCols,
          cols.status,
          cols.location,
          ...baseClos,
          cols.resourceGroupName
        ];
      default:
        return [...nameCols, ...baseClos];
    }
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
    if (this.isVmGroup) {
      assign(extParams, {
        path: this.treeSelection?.path,
        scope_resource_id: this.treeSelection?.uuid
      });
    }

    if (this.isVmware && !this.isVmGroup) {
      // VMware
      return { path: this.treeSelection?.path + '/', type: this.ID };
    }

    if (this.isFc && !this.isVmGroup) {
      // FC
      return {
        subType: ResourceType.FUSION_COMPUTE,
        path: [['=~'], this.treeSelection.path],
        type: this.ID
      };
    }

    if (this.isHcs && !this.isVmGroup) {
      // HCS
      if (this.ID === ResourceType.TENANT) {
        assign(extParams, {
          visible: '1'
        });
      }
      return assign(extParams, {
        subType: this.subType,
        path: [['=~'], this.treeSelection.path + '/'],
        type: this.ID
      });
    }

    assign(extParams, {
      subType: [this.subType]
    });

    if (this.isOpenstack && !this.isVmGroup) {
      // Openstack
      if (this.ID === ResourceType.OpenStackDomain) {
        assign(extParams, {
          visible: '1'
        });
      }
      return assign(extParams, {
        rootUuid: this.treeSelection?.rootUuid,
        subType: [this.subType]
      });
    }
    if (
      this.treeSelection?.rootUuid &&
      this.subType !== this.resType.vmGroup.value
    ) {
      assign(extParams, {
        rootUuid: this.treeSelection?.rootUuid
      });
    }

    if (
      includes(
        [this.resType.APSZone.value, this.resType.APSRegion.value],
        this.treeSelection?.subType
      ) &&
      !this.isVmGroup
    ) {
      assign(extParams, {
        path: [['=~'], this.treeSelection?.path + '/']
      });
    }
    if (
      includes([this.resType.APSResourceSet.value], this.treeSelection?.subType)
    ) {
      assign(extParams, {
        resourceSetName: this.treeSelection.name
      });
    }
    if (
      includes(
        [
          this.resType.cNwareCluster.value,
          this.resType.cNwareHost.value,
          this.resType.cNwareHostPool.value
        ],
        this.treeSelection?.subType
      ) &&
      !this.isVmGroup
    ) {
      assign(extParams, {
        path: [['=~'], this.treeSelection?.path]
      });
    }
    return extParams;
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

    if (this.isVmware) {
      delete defaultConditions.subType;
    }

    if (this.isDetail) {
      // 详情加资源集id
      if (this.isVmware) {
        assign(defaultConditions, {
          resource_set_id: this.data[0].uuid
        });
      } else {
        assign(defaultConditions, {
          resourceSetId: this.data[0].uuid
        });
      }
    }

    if (this.isVmGroup) {
      delete defaultConditions.subType;
      assign(defaultConditions, { source_sub_type: this.subUnitType });
    }

    if (!isEmpty(filters.conditions_v2)) {
      // vmware单独处理
      const conditionsTemp = JSON.parse(
        this.isVmware && !this.isVmGroup
          ? filters.conditions
          : filters.conditions_v2
      );

      if (this.isVmware && !this.isVmGroup) {
        if (conditionsTemp.status) {
          assign(conditionsTemp, {
            link_status: conditionsTemp.status
          });
          delete conditionsTemp.status;
        }
        if (conditionsTemp.protectionStatus) {
          assign(conditionsTemp, {
            protection_status: conditionsTemp.protectionStatus
          });
          delete conditionsTemp.protectionStatus;
        }
        if (conditionsTemp.remark) {
          assign(conditionsTemp, {
            tags: conditionsTemp.remark
          });
          delete conditionsTemp.remark;
        }
      }

      if (this.isVmGroup && conditionsTemp.protectionStatus) {
        assign(conditionsTemp, {
          protection_status: conditionsTemp.protectionStatus
        });
        delete conditionsTemp.protectionStatus;
      }
      if (
        this.subType === this.resType.hyperVHost.value &&
        conditionsTemp.status
      ) {
        assign(conditionsTemp, {
          linkStatus: conditionsTemp.status
        });
        delete conditionsTemp.status;
      }
      assign(defaultConditions, conditionsTemp);
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    // 根据不同应用更改端口
    const queryFunc: Observable<any> = this.isVmGroup
      ? this.protectedResourceApiService.ListResourceGroups(params)
      : this.isVmware
      ? this.virtualResourceService.queryResourcesV1VirtualResourceGet(params)
      : this.protectedResourceApiService.ListResources(params);
    queryFunc.subscribe(res => {
      if (this.isVmware && !this.isVmGroup) {
        // 由于接口不一样，返回的数据结构需要处理一下
        assign(res, {
          records: res.items.map(item =>
            assign(item, {
              status: item.link_status,
              remark: item.tags
            })
          ),
          totalCount: res.total
        });
      }
      each(res.records, (item: any) => {
        if (!this.isVmware) {
          extendSlaInfo(item);
        } else {
          assign(item, {
            protectionStatus: item.protection_status
          });
        }

        this.extendResult(item);
        assign(item, {
          sub_type: item?.subType
        });
      });

      if (this.allSelect) {
        each(res.records, item => {
          item.disabled = true;
          if (!find(this.selectionData, { uuid: item.uuid })) {
            this.selectionData.push(item);
          }
          this.dataTable.setSelections(this.selectionData);
        });
      }

      this.tableData = {
        data: res.records,
        total: res.totalCount
      };

      // 如果父级被选中，则需要把子级选中并且置灰,父级被取消选中则取消置灰,全选状态下不考虑这个
      if (
        !isEmpty(this.allSelectionMap[this.appType]?.data) &&
        !this.allSelectionMap[this.appType]?.isAllSelected
      ) {
        this.parentSelectChild();
      }

      if (isEmpty(filters.conditions_v2)) {
        this.updateTable.emit({ total: res.totalCount });
      }
    });
  }

  private parentSelectChild() {
    let currentType = this.isVmGroup
      ? ResourceSetType.RESOURCE_GROUP
      : this.appType;

    each(this.tableData.data, item => {
      if (
        find(
          this.allSelectionMap[this.appType].data,
          val =>
            val.uuid === item?.parent_uuid ||
            val.uuid === item?.environment_uuid ||
            val.uuid === item?.parentUuid ||
            val.uuid === item?.environment?.uuid
        )
      ) {
        assign(item, {
          disabled: true
        });
        if (!find(this.selectionData, { uuid: item.uuid })) {
          this.selectionData.push(item);
        }
      } else {
        assign(item, {
          disabled: false
        });
      }
    });
    this.dataTable.setSelections(cloneDeep(this.selectionData));
    this.parseSelected(currentType);
    this.baseSelectChange.emit();
  }

  extendResult(item) {
    switch (this.subType) {
      case this.resType.cNwareCluster.value:
      case this.resType.cNwareHost.value:
      case this.resType.cNwareVm.value:
        assign(item, JSON.parse(item.extendInfo?.details));
        break;
      case this.resType.APSCloudServer.value:
      case this.resType.HCSCloudHost.value:
      case this.resType.openStackCloudServer.value:
      case this.resType.fusionComputeVirtualMachine.value:
        assign(item, {
          status: item.extendInfo.status
        });
        break;
      case this.resType.hyperVHost.value:
        assign(item, {
          status: item?.linkStatus ?? '0'
        });
        break;
      case this.resType.HCSTenant.value:
      case ResourceType.OpenStackDomain:
        assign(item, {
          projectCount: +item.extendInfo?.projectCount || 0
        });
        break;
      default:
        break;
    }
  }
}
