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
import { DataMap, I18NService, ProtectedResourceApiService } from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, each, get, size } from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-postgre-proxy-host',
  templateUrl: './postgre-proxy-host.component.html',
  styleUrls: ['./postgre-proxy-host.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class PostgreProxyHostComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;

  @Input() data;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    this.getHosts();
  }

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    const installDeployType = this.data.extendInfo?.installDeployType;
    const subType = this.data.sub_type;
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
        width: 130,
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'clientPath',
        name: this.i18n.get('common_database_client_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'archiveDir',
        name: this.i18n.get('common_database_archive_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        hidden: this.getArchiveDirHidden(subType)
      },
      {
        key: 'pgpoolClientPath',
        name: this.getClientPathName(installDeployType),
        hidden: this.getClientPathHidden(installDeployType),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'adbhamgrPath',
        name: this.i18n.get('common_config_file_full_path_label'),
        hidden:
          this.data.sub_type !==
          DataMap.Resource_Type.AntDBClusterInstance.value,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'serviceIp',
        width: 180,
        name: this.i18n.get('common_dataplane_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        width: 125,
        name: this.i18n.get('common_database_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
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

  private getClientPathHidden(installDeployType) {
    return ![
      DataMap.PostgreSqlDeployType.Pgpool.value,
      DataMap.PostgreSqlDeployType.Patroni.value
    ].includes(installDeployType);
  }

  private getArchiveDirHidden(subType) {
    return ![
      DataMap.Resource_Type.PostgreSQLInstance.value,
      DataMap.Resource_Type.PostgreSQLClusterInstance.value
    ].includes(subType);
  }

  private getClientPathName(installDeployType) {
    switch (installDeployType) {
      case DataMap.PostgreSqlDeployType.Patroni.value:
        return this.i18n.get('common_patroni_path_label');
      case DataMap.PostgreSqlDeployType.Pgpool.value:
        return this.i18n.get('common_pg_pool_path_label');
      default:
        return '';
    }
  }

  getHosts() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.data.uuid,
        akDoException: false
      })
      .pipe(
        map(res => {
          if (
            [
              DataMap.Resource_Type.PostgreSQLClusterInstance.value,
              DataMap.Resource_Type.AntDBClusterInstance.value
            ].includes(this.data.sub_type)
          ) {
            each(get(res, 'dependencies.children', []), item => {
              assign(item, {
                serviceIp: get(item, 'extendInfo.serviceIp', ''),
                port: get(item, 'extendInfo.instancePort', ''),
                clientPath: get(item, 'extendInfo.clientPath', ''),
                archiveDir: get(item, 'extendInfo.archiveDir', ''),
                pgpoolClientPath: get(item, 'extendInfo.pgpoolClientPath', ''),
                adbhamgrPath: get(item, 'extendInfo.adbhamgrPath', ''),
                endpoint: get(item, 'dependencies.agents.0.endpoint', ''),
                name: get(item, 'dependencies.agents.0.name', '')
              });
            });
          } else {
            assign(res.extendInfo, {
              endpoint: get(res, 'environment.endpoint'),
              name: get(res, 'environment.name'),
              port: get(res, 'extendInfo.instancePort')
            });
          }
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          data: get(res, 'dependencies.children') || [get(res, 'extendInfo')],
          total:
            size(get(res, 'dependencies.children')) ||
            size([get(res, 'extendInfo')])
        };
        this.cdr.detectChanges();
      });
  }
}
