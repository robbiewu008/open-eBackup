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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  ViewChild,
  Input
} from '@angular/core';
import {
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, each, get, size, map as _map } from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-proxy-host',
  templateUrl: './proxy-host.component.html',
  styleUrls: ['./proxy-host.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ProxyHostComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;

  @Input() data;
  @Input() isInstance;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

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
        name: this.i18n.get('common_business_ip_label'),
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
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label')
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
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

  getHosts() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.data.uuid
      })
      .pipe(
        map(res => {
          each(get(res, 'dependencies.agents'), item => {
            assign(item, { type: get(res, 'dependencies.clusterType') });
          });
          return res;
        })
      )
      .subscribe(res => {
        if (
          this.data.subType === DataMap.Resource_Type.MySQLClusterInstance.value
        ) {
          this.tableData = {
            data: _map(get(res, 'dependencies.children'), (item: any) => {
              return {
                name: item.name,
                endpoint: item.path,
                linkStatus: item.extendInfo?.linkStatus,
                port: item.extendInfo?.instancePort
              };
            }),
            total: size(get(res, 'dependencies.children'))
          };
        } else if (
          this.data.subType === DataMap.Resource_Type.MySQLInstance.value
        ) {
          this.tableData = {
            data: [
              {
                name: res.name,
                endpoint: res.path,
                linkStatus: res.extendInfo?.linkStatus,
                port: res.extendInfo?.instancePort
              }
            ],
            total: 1
          };
        } else {
          this.tableData = {
            data: get(res, 'dependencies.agents'),
            total: size(get(res, 'dependencies.agents'))
          };
        }
        this.cdr.detectChanges();
      });
  }
}
