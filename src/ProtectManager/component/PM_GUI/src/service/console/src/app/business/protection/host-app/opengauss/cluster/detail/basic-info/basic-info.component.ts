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
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, cloneDeep, each, find, get, map, size } from 'lodash';

@Component({
  selector: 'aui-basic-info',
  templateUrl: './basic-info.component.html',
  styleUrls: ['./basic-info.component.less']
})
export class BasicInfoComponent implements OnInit, AfterViewInit {
  tableData: TableData;
  tableConfig: TableConfig;
  deployType: string;
  isOnline = true;

  @Input() data;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('deployTypeTpl', { static: true }) deployTypeTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    this.initData();
  }

  ngOnInit(): void {
    this.initConfig();
    this.initData();
  }

  initConfig() {
    this.deployType = this.data?.deployType;
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
          config: this.dataMapService.toArray('openGauss_InstanceStatus')
        }
      },
      {
        key: 'role',
        name: this.i18n.get('protection_running_mode_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('opengauss_Role')
        }
      }
    ];

    this.tableConfig = {
      table: {
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

  initData() {
    const nodesData = JSON.parse(this.data.extendInfo.nodes);
    each(nodesData, item => {
      assign(item, {
        linkStatus:
          item.extendInfo.status ===
          DataMap.openGauss_InstanceStatus.normal.value
            ? DataMap.openGauss_InstanceStatus.normal.value
            : DataMap.openGauss_InstanceStatus.offline.value,
        role: item.extendInfo.role
      });
    });
    this.tableData = {
      data: nodesData,
      total: nodesData.length
    };
    if (
      find(
        nodesData,
        item =>
          item.linkStatus === DataMap.openGauss_InstanceStatus.offline.value
      )
    ) {
      this.isOnline = false;
    }
  }

  getHosts() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.data.uuid
      })
      .subscribe(res => {
        const data = map(
          cloneDeep(get(res, ['dependencies', 'agents'])),
          item => ({ ...item, deployType: this.deployType })
        );
        this.tableData = {
          data,
          total: size(res['dependencies']['agents'])
        };
        this.cdr.detectChanges();
      });
  }
}
