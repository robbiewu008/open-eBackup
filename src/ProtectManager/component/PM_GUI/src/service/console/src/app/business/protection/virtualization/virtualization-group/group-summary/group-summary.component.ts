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
import { Component, OnInit } from '@angular/core';
import {
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  ResourceType,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { difference, map, size } from 'lodash';

@Component({
  selector: 'aui-group-summary',
  templateUrl: './group-summary.component.html',
  styleUrls: ['./group-summary.component.less']
})
export class GroupSummaryComponent implements OnInit {
  unitconst = CAPACITY_UNIT;
  resourceType = ResourceType;
  dataMap = DataMap;
  source;
  tableData: TableData;
  tableConfig: TableConfig;
  selectionData = [];
  optsConfig;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    public warningMessageService: WarningMessageService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.showTableData();
  }

  initDetailData(data: any) {
    this.source = data;
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'remove',
        type: 'primary',
        label: this.i18n.get('common_remove_label'),
        disableCheck: () => {
          return !size(this.selectionData);
        },
        onClick: () => {
          this.remove('multiple');
        }
      }
    ];
    this.optsConfig = opts;
    const cols: TableCols[] = [
      {
        key: 'sourceName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'path',
        name: this.i18n.get('common_location_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'remove',
                label: this.i18n.get('common_remove_label'),
                onClick: data => {
                  this.remove('single', data);
                }
              }
            ]
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        compareWith: 'uuid',
        colDisplayControl: false,
        selectionChange: selection => {
          this.selectionData = selection;
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
  }

  remove(type, item?) {
    let retainedData = [];
    if (type === 'single') {
      retainedData = difference(this.tableData.data, item);
    } else {
      retainedData = difference(this.tableData.data, this.selectionData);
    }

    this.warningMessageService.create({
      content: this.i18n.get('protection_group_remove_tip_label', [
        this.source.name
      ]),
      onOK: () => {
        this.protectedResourceApiService
          .UpdateResourceGroup({
            UpdateResourceGroupRequestBody: {
              name: this.source.name,
              resourceIds: retainedData.map(item => item.sourceId)
            },
            resourceGroupId: this.source.uuid
          })
          .subscribe(res => {
            this.selectionData = [];
            this.showTableData();
          });
      }
    });
  }

  showTableData() {
    this.protectedResourceApiService
      .ShowResourceGroup({ resourceGroupId: this.source?.uuid })
      .subscribe(res => {
        const data = map(res.resourceGroupMembers, item => ({ ...item }));
        this.tableData = {
          data: data,
          total: size(data)
        };
      });
  }
}
