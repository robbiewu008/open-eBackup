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
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  AppService,
  CommonConsts,
  DataMap,
  I18NService,
  MODAL_COMMON,
  ProtectedResourceApiService,
  extendSlaInfo
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  defer,
  each,
  every,
  filter,
  first,
  includes,
  isEmpty,
  isNumber,
  map,
  reject,
  size,
  some
} from 'lodash';
import { Subject } from 'rxjs';
import { AddDiskComponent } from './add-disk/add-disk.component';

@Component({
  selector: 'aui-select-database-list',
  templateUrl: './select-database-list.component.html',
  styleUrls: ['./select-database-list.component.less']
})
export class SelectDatabaseListComponent implements OnInit, AfterViewInit {
  activeIndex = 'selected';
  tableConfig: TableConfig;
  datasTableConfig: TableConfig;
  tableData: TableData;
  selectionData: any[];
  selectData: TableData;
  total = CommonConsts.PAGE_TOTAL;
  selectedDataNum;
  title: string = this.i18n.get('common_selected_label');
  modifyFlag = false;
  resourceSubType: string;
  dataMap = DataMap;
  source: any;
  valid$ = new Subject<boolean>();

  singleSelectFlag = false;
  enableSelectAll = false;
  notAllDiskShow = false;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('selectTable', { static: false }) selectTable: ProTableComponent;
  @ViewChild('slaTpl', { static: true }) slaTpl: TemplateRef<any>;
  @ViewChild('selectDiskTpl', { static: true }) selectDiskTpl: TemplateRef<any>;
  @ViewChild('diskHelpExtraTpl', { static: true })
  diskHelpExtraTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private appService: AppService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit() {
    if (this.dataTable) {
      this.dataTable.fetchData();
    }
    if (this.selectTable) {
      this.selectTable.fetchData();
    }
  }

  ngOnInit(): void {
    this.initTable();
  }

  getData(filters: Filters, args: any) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const defaultConditions = {
      subType:
        this.source?.subType || this.dataMap.Resource_Type.FusionCompute.value,
      path: [['=~'], this.source.environment.path],
      type: [this.resourceSubType]
    };
    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        assign(item, {
          sub_type: item.subType,
          cluster: item.environment?.name,
          endpoint: item.environment?.endpoint,
          parentName: item.parentName,
          disabled: !isEmpty(item.protectedObject?.slaId)
        });
        extendSlaInfo(item);
      });
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      this.total = res.totalCount;
      if (!this.modifyFlag) {
        each(this.selectData.data, item => {
          assign(item, {
            enableSelectAll: true
          });
        });
      }
      this.selectionData = this.selectData.data;
      this.dataTable.setSelections(this.selectData.data);
    });
  }

  // 查询当前虚拟机磁盘信息
  getDisk(agentsId, vmData, recordsTemp?, startPage?) {
    const extParams = {
      agentId: agentsId,
      envId: this.source.environment.uuid,
      pageNo: startPage || 1,
      pageSize: CommonConsts.MAX_PAGE_SIZE,
      resourceIds: [this.source.uuid]
    };
    this.appService.ListResourcesDetails(extParams).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = 1;
      }
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.MAX_PAGE_SIZE) ||
        res.totalCount === 0 ||
        isEmpty(recordsTemp)
      ) {
        each(vmData, item => {
          const uuidMap = map(recordsTemp, 'uuid');
          assign(item, {
            enableSelectAll: isEmpty(
              item.protectedObject?.extParameters?.disk_info
            ),
            diskInfo: filter(
              item.protectedObject?.extParameters?.disk_info,
              disk => {
                return includes(uuidMap, JSON.parse(disk)?.id);
              }
            )
          });
        });
        return;
      }
      startPage++;
      this.getDisk(agentsId, vmData, recordsTemp, startPage);
    });
  }

  getVmDisk(vmData) {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      queryDependency: true,
      conditions: JSON.stringify({
        subType:
          this.source?.subType || DataMap.Resource_Type.FusionCompute.value,
        uuid: this.source.environment.uuid
      })
    };
    this.protectedResourceApiService
      .ListResources(params)
      .subscribe((res: any) => {
        if (res.records?.length) {
          const onlineAgents = res.records[0]?.dependencies?.agents?.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
          if (isEmpty(onlineAgents)) {
            each(vmData, item => {
              assign(item, {
                enableSelectAll: isEmpty(
                  item.protectedObject?.extParameters?.disk_info
                ),
                diskInfo: []
              });
            });
            this.setValid();
            return;
          }
          const agentsId = onlineAgents[0].uuid;
          this.getDisk(agentsId, vmData);
        }
      });
  }

  initData(data: any, resourceType: string) {
    this.resourceSubType = resourceType;
    this.source = first(data);
    this.modifyFlag = size(data) === 1 && !isEmpty(data[0].sla_id);
    this.singleSelectFlag = size(data) === 1;
    this.selectedDataNum = size(data);

    if (
      this.modifyFlag &&
      !isEmpty(this.source?.protectedObject?.extParameters?.disk_info)
    ) {
      this.getVmDisk(data);
    } else {
      each(data, item => {
        assign(item, {
          enableSelectAll: isEmpty(item.protectedObject)
            ? true
            : isEmpty(item.protectedObject?.extParameters?.disk_info),
          diskInfo: item.protectedObject?.extParameters?.disk_info
        });
      });
    }

    this.selectData = {
      data: data,
      total: size(data)
    };

    this.notAllDiskShow = some(data, item => !item.enableSelectAll);
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'sla_name',
        name: 'SLA',
        cellRender: this.slaTpl
      },
      {
        key: 'desk',
        name: this.i18n.get('common_auto_protect_disk_label'),
        thExtra: this.diskHelpExtraTpl,
        hidden:
          this.resourceSubType !==
          DataMap.Resource_Type.fusionComputeVirtualMachine.value,
        cellRender: this.selectDiskTpl
      },
      {
        key: 'vmNumber',
        name: this.i18n.get('protection_vms_label'),
        hidden: ![
          DataMap.Resource_Type.fusionComputeCNA.value,
          DataMap.Resource_Type.fusionComputeCluster.value
        ].includes(this.resourceSubType)
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        width: 110,
        hidden: this.singleSelectFlag,
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 2,
            items: [
              {
                id: 'remove',
                label: this.i18n.get('common_remove_label'),
                onClick: ([data]) => {
                  this.selectData = {
                    data: reject(this.selectData.data, item => {
                      return item.uuid === data.uuid;
                    }),
                    total: --this.selectData.total
                  };
                  this.selectionData = this.selectData.data;
                  this.dataTable.setSelections(this.selectData.data);
                  this.selectedDataNum = size(this.selectData.data);
                  this.setValid();
                }
              }
            ]
          }
        }
      }
    ];

    this.datasTableConfig = {
      table: {
        size: 'small',
        columns: [cols[0], cols[1]],
        async: true,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        compareWith: 'uuid',
        colDisplayControl: false,
        scrollFixed: true,
        fetchData: (filter: Filters, args: {}) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
          this.selectData = {
            data: selection,
            total: size(selection)
          };
          this.selectedDataNum = size(selection);
          this.setValid();
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS,
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
    this.tableConfig = {
      table: {
        size: 'small',
        columns: cols,
        async: false,
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
  }

  switchChange() {
    defer(() => {
      if (this.singleSelectFlag) {
        this.notAllDiskShow = !this.source.enableSelectAll;
      } else {
        this.notAllDiskShow = some(
          this.selectionData,
          item => !item.enableSelectAll
        );
      }
      this.setValid();
    });
  }

  selectDisk(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-fusion-compute-disk',
        lvWidth: MODAL_COMMON.largeWidth + 100,
        lvHeader: this.i18n.get('common_fc_select_disk_label'),
        lvContent: AddDiskComponent,
        lvComponentParams: { data },
        lvOkDisabled: false,
        lvOk: modal => {
          const content = modal.getContentComponent() as AddDiskComponent;
          assign(data, {
            diskInfo: content.onOK().map(item => {
              return {
                id: item.uuid,
                name: item.name,
                extendInfo: JSON.stringify(item.extendInfo),
                type: item.type,
                subType: item.subType
              };
            })
          });
          this.setValid();
        }
      })
    );
  }

  setValid() {
    if (this.singleSelectFlag) {
      this.valid$.next(
        this.source.enableSelectAll || !isEmpty(this.source.diskInfo)
      );
    } else {
      this.valid$.next(
        !!size(this.selectionData) &&
          every(this.selectionData, item => {
            return item.enableSelectAll || !isEmpty(item.diskInfo);
          })
      );
    }
  }

  onOK() {
    return { selectedList: this.selectionData };
  }
}
