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
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { includes, size } from 'lodash';

@Component({
  selector: 'aui-vir-environment-info',
  templateUrl: './environment-info.component.html',
  styleUrls: ['./environment-info.component.less']
})
export class EnvironmentInfoComponent implements OnInit {
  rowItem;
  online = false;
  onlineIcon;
  offlineIcon;
  formItems = [];
  tableConfig: TableConfig;
  tableData: TableData;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.getIcon();
    this.initConfig();
    this.getFormItems();
  }

  getIcon() {
    if (
      includes(
        [
          DataMap.Resource_Type.hyperVScvmm.value,
          DataMap.Resource_Type.hyperVCluster.value
        ],
        this.rowItem.subType
      )
    ) {
      this.onlineIcon = 'aui-sla-cluster-online';
      this.offlineIcon = 'aui-sla-cluster-offline';
    } else if (
      includes([DataMap.Resource_Type.hyperVHost.value], this.rowItem.subType)
    ) {
      this.onlineIcon = 'aui-host-online-48';
      this.offlineIcon = 'aui-host-offline-48';
    } else {
      this.onlineIcon = 'aui-sla-fusion-compute-online';
      this.offlineIcon = 'aui-sla-fusion-compute-offline';
    }
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
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        showLoading: false,
        columns: cols,
        size: 'small',
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getFormItems() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowItem?.uuid })
      .subscribe((res: any) => {
        this.formItems = [
          [
            {
              label: this.i18n.get('common_name_label'),
              content: this.rowItem?.name
            },
            {
              label:
                this.rowItem.subType === DataMap.Resource_Type.cNware.value
                  ? this.i18n.get('common_ip_address_domain_label')
                  : this.i18n.get('common_ip_label'),
              content: this.rowItem?.endpoint
            },
            {
              label: this.i18n.get('common_username_label'),
              content: this.rowItem?.auth?.authKey,
              hide: includes(
                [DataMap.Resource_Type.hyperVHost.value],
                this.rowItem.subType
              )
            }
          ],
          [
            {
              label: this.i18n.get('common_port_label'),
              content: this.rowItem?.port,
              hide: includes(
                [
                  DataMap.Resource_Type.hyperVCluster.value,
                  DataMap.Resource_Type.hyperVScvmm.value,
                  DataMap.Resource_Type.hyperVHost.value,
                  DataMap.Resource_Type.hyperVVm.value
                ],
                this.rowItem.subType
              )
            }
          ]
        ];
        this.tableData = {
          data: res.dependencies?.agents,
          total: size(res.dependencies?.agents)
        };
        this.online =
          res.linkStatus === DataMap.resource_LinkStatus_Special.normal.value;
      });
  }
}
