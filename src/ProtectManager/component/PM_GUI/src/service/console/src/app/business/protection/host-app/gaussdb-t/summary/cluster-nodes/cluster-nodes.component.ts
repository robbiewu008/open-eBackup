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
  Component,
  OnInit,
  ChangeDetectionStrategy,
  ViewChild,
  AfterViewInit,
  Input,
  ChangeDetectorRef
} from '@angular/core';
import {
  TableConfig,
  TableData,
  ProTableComponent,
  TableCols,
  Filters
} from 'app/shared/components/pro-table';
import {
  I18NService,
  DataMapService,
  DataMap,
  ProtectedResourceApiService
} from 'app/shared';
import { assign, defer, get, isArray, map, size } from 'lodash';
import { CdkRow } from '@angular/cdk/table';

@Component({
  selector: 'aui-cluster-nodes',
  templateUrl: './cluster-nodes.component.html',
  styleUrls: ['./cluster-nodes.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ClusterNodesComponent implements OnInit {
  tableConfig: TableConfig;
  tableData: TableData;
  @Input() source;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private cdr: ChangeDetectorRef,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initConfig();
    defer(() => {
      if (this.source.subType === DataMap.Resource_Type.gaussdbTSingle.value) {
        this.getGaussDBTSingleData();
      } else {
        this.getGaussDBTData();
      }
    });
  }

  initConfig() {
    const cols: TableCols[] =
      this.source.subType === DataMap.Resource_Type.gaussdbTSingle.value
        ? [
            {
              key: 'name',
              name: this.i18n.get('common_name_label')
            },
            {
              key: 'endpoint',
              name: this.i18n.get('common_ip_address_label')
            },
            {
              key: 'linkStatus',
              name: this.i18n.get('common_status_label'),
              filter: {
                type: 'select',
                isMultiple: true,
                showCheckAll: true,
                options: this.dataMapService.toArray(
                  'resource_LinkStatus_Special'
                )
              },
              cellRender: {
                type: 'status',
                config: this.dataMapService.toArray(
                  'resource_LinkStatus_Special'
                )
              }
            }
          ]
        : [
            {
              key: 'endpoint',
              name: this.i18n.get('common_ip_address_label')
            },
            {
              key: 'status',
              name: this.i18n.get('common_status_label'),
              filter: {
                type: 'select',
                isMultiple: true,
                showCheckAll: true,
                options: this.dataMapService.toArray('gaussDBT_Node_Status')
              },
              cellRender: {
                type: 'status',
                config: this.dataMapService.toArray('gaussDBT_Node_Status')
              }
            },
            {
              key: 'role',
              name: this.i18n.get('protection_running_mode_label'),
              filter: {
                type: 'select',
                isMultiple: true,
                showCheckAll: true,
                options: this.dataMapService.toArray('gaussDBT_Node_Type')
              },
              cellRender: {
                type: 'status',
                config: this.dataMapService.toArray('gaussDBT_Node_Type')
              }
            }
          ];

    this.tableConfig = {
      table: {
        columns: cols,
        size: 'small',
        colDisplayControl: false,
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getGaussDBTData() {
    const nodes = JSON.parse(this.source.extendInfo.nodes);
    nodes.filter(item => {
      assign(item, {
        status: item.extendInfo?.status,
        role: item.extendInfo?.role
      });
    });
    let _nodes = nodes;

    this.tableData = {
      data: _nodes,
      total: size(_nodes)
    };
  }

  getGaussDBTSingleData() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.source.uuid })
      .subscribe(res => {
        const dataList: any = get(res, 'dependencies.agents');

        this.tableData = {
          data: map(dataList, (item: any) => {
            return {
              name: item.name,
              endpoint: item.endpoint,
              linkStatus: item.linkStatus
            };
          }),
          total: dataList.length
        };
        this.cdr.detectChanges();
      });
  }
}
