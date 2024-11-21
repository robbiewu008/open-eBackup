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
import { Component, OnInit, ChangeDetectorRef } from '@angular/core';
import {
  I18NService,
  DataMapService,
  ProtectedResourceApiService,
  DataMap,
  CommonConsts
} from 'app/shared';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';
import { get, map, size, split, toString as _toString } from 'lodash';

@Component({
  selector: 'aui-cluster-detail',
  templateUrl: './cluster-detail.component.html',
  styleUrls: ['./cluster-detail.component.less']
})
export class ClusterDetailComponent implements OnInit {
  data = {} as any;
  formItems = [];
  tableConfig: TableConfig;
  tableData;
  hosts;
  resSubType;

  constructor(
    private i18n: I18NService,
    private dataMap: DataMapService,
    private cdr: ChangeDetectorRef,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initFormItems();
    this.initConfig();
    this.getDetails();
  }

  initDetailData(data: any) {
    this.data = data;
    this.hosts = split(this.data?.endpoint, ',');
    this.data.subType === DataMap.Resource_Type.goldendbCluter.value;
  }

  initFormItems() {
    this.formItems = [
      [
        {
          key: 'name',
          value: this.data.name,
          label: this.i18n.get('common_name_label')
        }
      ],
      [
        {
          key: 'version',
          value: this.data.version,
          label: this.i18n.get('common_version_label')
        }
      ]
    ];
  }

  initConfig() {
    const cols: TableCols[] = [
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
        cellRender: {
          type: 'status',
          config: this.dataMap.toArray('resource_LinkStatus_Special')
        }
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'ip',
        columns: cols,
        virtualScroll: true,
        scrollFixed: true,
        scroll: { y: '420px' },
        colDisplayControl: false
      },
      pagination: null
    };
  }

  getDetails() {
    if (!this.data) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.data.uuid })
      .subscribe(res => {
        const nodes = map(get(res, 'dependencies.agents'), (item: any) => {
          return {
            uuid: item.uuid,
            name: item.name,
            linkStatus: item.linkStatus,
            endpoint: item.endpoint
          };
        });

        this.tableData = {
          data: nodes,
          total: size(nodes)
        };
      });
  }
}
