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
  I18NService,
  MODAL_COMMON,
  ProtectedResourceApiService,
  ResourceType,
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
  find,
  first,
  isEmpty,
  reject,
  size
} from 'lodash';
import { Subject } from 'rxjs';
import { AddDiskComponent } from './add-disk/add-disk.component';

@Component({
  selector: 'aui-select-po',
  templateUrl: './select-po.component.html',
  styleUrls: ['./select-po.component.less']
})
export class SelectPoComponent implements OnInit, AfterViewInit {
  resourceSubType;
  activeIndex = 'selected';
  selectedDataNum = CommonConsts.PAGE_TOTAL;
  total = CommonConsts.PAGE_TOTAL;
  modifyFlag = false;
  singleSelectFlag = false;

  selectionData: any[];

  totalTableConfig: TableConfig;
  totalTableData: TableData;
  selectTableConfig: TableConfig;
  selectTableData: TableData;

  source: any;
  valid$ = new Subject<boolean>();

  @ViewChild('totalDataTable', { static: false })
  totalDataTable: ProTableComponent;
  @ViewChild('selectDataTable', { static: false })
  selectDataTable: ProTableComponent;
  @ViewChild('slaTpl', { static: true }) slaTpl: TemplateRef<any>;
  @ViewChild('deskDeviceTpl', { static: true }) deskDeviceTpl: TemplateRef<any>;
  @ViewChild('diskHelpExtraTpl', { static: true })
  diskHelpExtraTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    if (this.totalDataTable) {
      this.totalDataTable.fetchData();
    }
  }

  ngOnInit(): void {
    this.initConfig();
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
          : item.protectedObject?.extParameters?.all_disk === 'True',
        diskInfo: item.protectedObject?.extParameters?.disk_info
      });
    });

    this.selectTableData = {
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
      subType: [this.resourceSubType],
      rootUuid: this.source?.rootUuid
    };
    if (
      this.source.treeSelection &&
      this.source.treeSelection?.type !== ResourceType.OpenStack
    ) {
      assign(defaultConditions, {
        path: [['=~'], this.source.treeSelection?.path + '/']
      });
    }
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
        if (
          this.resourceSubType ===
          DataMap.Resource_Type.openStackCloudServer.value
        ) {
          assign(item, {
            enableSelectAll: true
          });
        }
      });
      this.totalTableData = {
        data: res.records,
        total: res.totalCount
      };
      this.total = res.totalCount;
      this.selectionData = this.selectTableData.data;
      this.totalDataTable.setSelections(this.selectTableData.data);
      each(this.totalTableData?.data, item => {
        const findData = find(this.selectTableData?.data, { uuid: item.uuid });
        if (findData) {
          assign(item, {
            enableSelectAll: findData.enableSelectAll,
            diskInfo: findData.diskInfo
          });
        }
      });
    });
  }

  initConfig() {
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
        key: 'disk',
        name: this.i18n.get('common_auto_protect_disk_label'),
        cellRender: this.deskDeviceTpl,
        thExtra: this.diskHelpExtraTpl,
        hidden:
          this.resourceSubType !==
          DataMap.Resource_Type.openStackCloudServer.value
      },
      {
        key: 'cloudServerCount',
        name: this.i18n.get('common_cloud_server_num_label'),
        hidden:
          this.resourceSubType !== DataMap.Resource_Type.openStackProject.value
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
                  this.selectTableData = {
                    data: reject(this.selectTableData.data, item => {
                      return item.uuid === data.uuid;
                    }),
                    total: --this.selectTableData.total
                  };
                  this.selectionData = this.selectTableData.data;
                  this.totalDataTable.setSelections(this.selectTableData.data);
                  this.selectedDataNum = size(this.selectTableData.data);
                  this.setValid();
                }
              }
            ]
          }
        }
      }
    ];

    this.totalTableConfig = {
      table: {
        size: 'small',
        columns: [cols[0], cols[1]],
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        compareWith: 'uuid',
        colDisplayControl: false,
        fetchData: (filter: Filters, args: {}) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
          this.selectTableData = {
            data: selection,
            total: size(selection)
          };
          this.selectedDataNum = size(selection);
          this.setValid();
        },
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
    this.selectTableConfig = {
      table: {
        size: 'small',
        columns: cols,
        async: false,
        colDisplayControl: false,
        trackByFn: (_, item) => {
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

  switchChange(item) {
    defer(() => this.setValid());
    if (!this.modifyFlag) {
      each(this.totalTableData?.data, value => {
        if (value.uuid === item.uuid) {
          value.enableSelectAll = item.enableSelectAll;
        }
      });
    }
  }

  selectDisk(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-openstack-disk',
        lvWidth: MODAL_COMMON.largeWidth + 180,
        lvHeader: this.i18n.get('common_fc_select_disk_label'),
        lvContent: AddDiskComponent,
        lvComponentParams: { data },
        lvOkDisabled: false,
        lvOk: modal => {
          const content = modal.getContentComponent() as AddDiskComponent;
          assign(data, {
            diskInfo: content.onOK()
          });
          if (!this.modifyFlag) {
            each(this.totalTableData?.data, value => {
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

  onOK() {
    return { selectedList: this.selectionData };
  }
}
