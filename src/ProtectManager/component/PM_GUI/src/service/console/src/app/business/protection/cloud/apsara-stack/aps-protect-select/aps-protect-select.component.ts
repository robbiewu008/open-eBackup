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
  CommonConsts,
  DataMap,
  extendSlaInfo,
  I18NService,
  MODAL_COMMON,
  ProtectedResourceApiService
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
  find,
  first,
  includes,
  isEmpty,
  reject,
  size
} from 'lodash';
import { Subject } from 'rxjs';
import { AddApsDiskComponent } from '../add-aps-disk/add-aps-disk.component';

@Component({
  selector: 'aui-aps-protect-select',
  templateUrl: './aps-protect-select.component.html',
  styleUrls: ['./aps-protect-select.component.less']
})
export class ApsProtectSelectComponent implements OnInit, AfterViewInit {
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
  switchStatus = false; // 选择磁盘控制开关

  singleSelectFlag = false;
  enableSelectAll = false;
  valid$ = new Subject<boolean>();
  _includes = includes;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('selectTable', { static: false }) selectTable: ProTableComponent;
  @ViewChild('slaTpl', { static: true }) slaTpl: TemplateRef<any>;
  @ViewChild('deskDeviceTpl', { static: true }) deskDeviceTpl: TemplateRef<any>;
  @ViewChild('diskHelpExtraTpl', { static: true })
  diskHelpExtraTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
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
        name: this.i18n.get('common_hcs_auto_protect_disk_label'),
        hidden:
          this.resourceSubType !== DataMap.Resource_Type.APSCloudServer.value,
        cellRender: this.deskDeviceTpl,
        thExtra: this.diskHelpExtraTpl
      },
      {
        key: 'cloudHostCount',
        name: this.i18n.get('common_cloud_server_num_label'),
        hidden:
          this.resourceSubType !== DataMap.Resource_Type.APSResourceSet.value
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
                      return item.name === data.name;
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

  initData(data: any, resourceType: string) {
    this.resourceSubType = resourceType;
    this.source = first(data);
    this.modifyFlag = size(data) === 1 && !isEmpty(data[0].sla_id);
    this.singleSelectFlag = size(data) === 1;
    this.selectedDataNum = size(data);

    each(data, item => {
      assign(item, {
        enableSelectAll: isEmpty(item.protectedObject)
          ? true
          : isEmpty(item.protectedObject?.extParameters?.disk_info),
        diskInfo: item.protectedObject?.extParameters?.disk_info
      });
      if (this.resourceSubType === DataMap.Resource_Type.APSResourceSet.value) {
        {
          assign(item, {
            cloudHostCount: JSON.parse(item.extendInfo.instanceList).length
          });
        }
      }
    });

    this.selectData = {
      data: data,
      total: size(data)
    };
  }

  getData(filters: Filters, args: any) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    const defaultConditions = {
      subType: [this.resourceSubType]
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
        if (
          this.resourceSubType === DataMap.Resource_Type.APSCloudServer.value
        ) {
          assign(item, {
            enableSelectAll: true
          });
        }
        extendSlaInfo(item);
      });
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      this.total = res.totalCount;
      this.selectionData = this.selectData.data;
      this.dataTable.setSelections(this.selectData.data);
      each(this.tableData?.data, item => {
        const findData = find(this.selectData?.data, { uuid: item.uuid });
        if (findData) {
          assign(item, {
            enableSelectAll: findData.enableSelectAll,
            diskInfo: findData.diskInfo
          });
        }
      });
    });
  }

  switchChange(item) {
    defer(() => this.setValid());
    if (!this.modifyFlag) {
      each(this.tableData?.data, value => {
        if (value.uuid === item.uuid) {
          value.enableSelectAll = item.enableSelectAll;
        }
      });
    }
  }

  selectDisk(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-APS-disk',
        lvWidth: MODAL_COMMON.largeWidth + 180,
        lvHeader: this.i18n.get('common_hcs_select_disk_label'),
        lvContent: AddApsDiskComponent,
        lvComponentParams: { data },
        lvOkDisabled: false,
        lvOk: modal => {
          const content = modal.getContentComponent() as AddApsDiskComponent;
          assign(data, {
            diskInfo: content.onOK()
          });
          if (!this.modifyFlag) {
            each(this.tableData?.data, value => {
              if (value.uuid === data.uuid) {
                value.diskInfo = content.onOK();
              }
            });
          }
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
