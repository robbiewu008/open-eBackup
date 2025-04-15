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
  Input,
  ChangeDetectorRef,
  AfterViewInit
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  Filters,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, each, size } from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-postgre-basic-info',
  templateUrl: './postgre-basic-info.component.html',
  styleUrls: ['./postgre-basic-info.component.less']
})
export class PostgreBasicInfoComponent implements OnInit, AfterViewInit {
  @Input() data;
  online;
  dataMap = DataMap;
  tableConfig: TableConfig;
  tableData: TableData;
  clusterServerData: TableData;
  nodeLabel = this.i18n.get('protection_statefulset_node_label');
  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    this.getHosts();
  }

  ngOnInit(): void {
    this.initConfig();
  }
  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('resource_LinkStatus_Special')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        columns: cols,
        size: 'small',
        colDisplayControl: false,
        autoPolling: CommonConsts.TIME_INTERVAL,
        fetchData: (filter: Filters) => {
          this.getHosts();
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getHosts() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.data.uuid,
        akLoading: false
      })
      .pipe(
        map(res => {
          each(res['dependencies']['agents'], item => {
            assign(item, { type: res['extendInfo']['clusterType'] });
          });
          return res;
        })
      )
      .subscribe((res: any) => {
        this.online =
          res.linkStatus === DataMap.resource_LinkStatus_Special.normal.value;
        this.tableData = {
          data: res['dependencies']['agents'],
          total: size(res['dependencies']['agents'])
        };
        if (
          res.extendInfo.installDeployType ===
          DataMap.PostgreSqlDeployType.CLup.value
        ) {
          this.nodeLabel = this.i18n.get('protection_cluster_node_label');
          this.clusterServerData = {
            data: res['dependencies']['clupServers'],
            total: size(res['dependencies']['clupServers'])
          };
        }
        this.cdr.detectChanges();
      });
  }
}
