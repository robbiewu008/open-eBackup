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
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
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
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { SlaService } from 'app/shared/services/sla.service';
import { assign, size, get, map, each } from 'lodash';

@Component({
  selector: 'aui-summary-tenant',
  templateUrl: './summary-tenant.component.html',
  styleUrls: ['./summary-tenant.component.less']
})
export class SummaryTenantComponent implements OnInit {
  source = {} as any;
  tableConfig: TableConfig;
  tableData: TableData;
  sourceType = DataMap.Resource_Type.OceanBaseTenant.value;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('tenantTable', { static: false }) tenantTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private virtualScroll: VirtualScrollService,
    private slaService: SlaService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private cdr: ChangeDetectorRef
  ) {}

  ngOnInit() {
    this.initConfig();
    this.getDetails();
  }

  initDetailData(data: any) {
    this.source = data;
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      },
      {
        key: 'sla_name',
        name: this.i18n.get('common_sla_label'),
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            overflow: true,
            click: data => {
              this.slaService.getDetail({
                uuid: data.sla_id,
                name: data.sla_name
              });
            }
          }
        }
      },
      {
        key: 'sla_compliance',
        name: this.i18n.get('common_sla_compliance_label'),
        thExtra: this.slaComplianceExtraTpl,
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Sla_Compliance')
        }
      },
      {
        key: 'protection_status',
        name: this.i18n.get('protection_protected_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Protection_Status')
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'uuid',
        columns: cols,
        virtualScroll: true,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        colDisplayControl: false
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
        const tenantInfos = get(clusterInfo, 'tenantInfos');
        each(tenantInfos, item => {
          assign(item, {
            protection_status: this.source?.protectionStatus,
            sla_name: this.source?.protectedObject?.slaName,
            sla_compliance: this.source?.protectedObject?.slaCompliance,
            sla_id: this.source?.protectedObject?.slaId
          });
        });
        this.tableData = {
          data: map(tenantInfos, item => {
            return {
              name: item.name,
              linkStatus: item.linkStatus,
              ...item
            };
          }),
          total: size(tenantInfos)
        };
        this.cdr.detectChanges();
      });
  }
}
