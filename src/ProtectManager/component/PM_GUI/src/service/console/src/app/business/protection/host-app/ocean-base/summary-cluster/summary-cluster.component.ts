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
  TemplateRef,
  ViewChild,
  ChangeDetectorRef
} from '@angular/core';
import {
  CommonConsts,
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
import { find, get, map, size } from 'lodash';

@Component({
  selector: 'aui-summary-ob-cluster',
  templateUrl: './summary-cluster.component.html',
  styleUrls: ['./summary-cluster.component.less']
})
export class SummaryClusterComponent implements OnInit {
  source;
  sourceType = DataMap.Resource_Type.OceanBaseCluster.value;
  obClientConfig: TableConfig;
  obClientData: TableData;
  obServerConfig: TableConfig;
  obServerData: TableData;

  @ViewChild('serverTable', { static: false }) serverTable: ProTableComponent;
  @ViewChild('clientTable', { static: false }) clientTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.getDetails();
  }

  initDetailData(data) {
    this.source = data;
  }

  initConfig() {
    const clientCols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('protection_proxy_host_label')
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
    const serverCols: TableCols[] = [
      {
        key: 'ip',
        name: this.i18n.get('common_dataplane_ip_label')
      },
      {
        key: 'name',
        name: this.i18n.get('protection_proxy_host_label')
      },
      {
        key: 'port',
        name: this.i18n.get('common_dataplane_port_label')
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

    this.obClientConfig = {
      table: {
        columns: clientCols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: true
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true
      }
    };

    this.obServerConfig = {
      table: {
        columns: serverCols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: true
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true
      }
    };
  }

  getDetails() {
    if (!this.source) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.source.uuid })
      .subscribe(res => {
        const clusterInfo = JSON.parse(get(res, 'extendInfo.clusterInfo'));
        const obClientAgents = get(clusterInfo, 'obClientAgents');
        const obServerAgents = get(clusterInfo, 'obServerAgents');
        const agents: any = get(res, 'dependencies');
        this.obClientData = {
          data: map(obClientAgents, item => {
            const agent = find(agents.clientAgents, { uuid: item.parentUuid });
            return {
              name: `${agent?.name}(${agent?.endpoint})`,
              linkStatus: item?.linkStatus
            };
          }),
          total: size(obClientAgents)
        };
        this.obServerData = {
          data: map(obServerAgents, item => {
            const agent = find(agents.serverAgents, { uuid: item.parentUuid });
            return {
              ip: item?.ip,
              name: `${agent?.name}(${agent?.endpoint})`,
              port: item?.port,
              linkStatus: item?.linkStatus
            };
          }),
          total: size(obServerAgents)
        };
        this.cdr.detectChanges();
      });
  }
}
