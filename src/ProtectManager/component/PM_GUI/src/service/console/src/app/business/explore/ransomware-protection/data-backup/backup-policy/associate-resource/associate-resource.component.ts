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
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  I18NService,
  ProjectedObjectApiService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, each, isEmpty } from 'lodash';

@Component({
  selector: 'aui-anti-associate-resource',
  templateUrl: './associate-resource.component.html',
  styleUrls: ['./associate-resource.component.less']
})
export class AssociateResourceComponent implements OnInit, AfterViewInit {
  @Input() policy: any;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('storageDeviceTpl', { static: true })
  storageDeviceTpl: TemplateRef<any>;
  @ViewChild('tenantNameTpl', { static: true }) tenantNameTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private projectedObjectApiService: ProjectedObjectApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
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
        key: 'parentName',
        name: this.i18n.get('protection_storage_device_label'),

        cellRender: this.storageDeviceTpl
      },
      {
        key: 'tenantName',
        name: this.i18n.get('common_tenant_label'),
        cellRender: this.tenantNameTpl
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'uuid',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showPageSizeOptions: false
      }
    };
  }

  getData(filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      slaId: this.policy?.uuid
    };
    if (!isEmpty(filters.conditions)) {
      assign(params, JSON.parse(filters.conditions));
    }
    this.projectedObjectApiService
      .pageQueryV1ProtectedObjectsGet(params)
      .subscribe(res => {
        each(res.items, item => {
          this.protectedResourceApiService
            .ShowResource({ resourceId: item.resource_id })
            .subscribe(resource => {
              assign(item, resource);
              this.tableData = {
                data: res.items,
                total: res.total
              };
            });
        });
      });
  }
}
