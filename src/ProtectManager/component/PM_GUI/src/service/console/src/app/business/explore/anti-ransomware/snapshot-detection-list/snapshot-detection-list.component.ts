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
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  ApiFilesystemPluginService,
  DataMapService,
  DetectionGroupField,
  I18NService,
  SupportLicense,
  WarningMessageService
} from 'app/shared';
import {
  TableData,
  TableConfig,
  ProTableComponent,
  TableCols,
  Filters
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign } from 'lodash';

@Component({
  selector: 'aui-snapshot-detection-list',
  templateUrl: './snapshot-detection-list.component.html',
  styleUrls: ['./snapshot-detection-list.component.less']
})
export class SnapshotDetectionListComponent implements OnInit, AfterViewInit {
  tableData: TableData;
  tableConfig: TableConfig;

  @ViewChild('numTpl', { static: true }) numTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('statisticsTpl', { static: true }) statisticsTpl: TemplateRef<any>;
  @ViewChild('lunStatisticsTpl', { static: true })
  lunStatisticsTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private warningMessageService: WarningMessageService,
    private apiFilesystemPluginService: ApiFilesystemPluginService
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
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'totalCount',
        name: this.i18n.get('explore_file_system_count_label'),
        sort: true,
        hidden: !SupportLicense.isFile
      },
      {
        key: 'condition',
        name: this.i18n.get('explore_detection_condition_label'),
        width: 280,
        cellRender: this.statisticsTpl,
        hidden: !SupportLicense.isFile
      },
      {
        key: 'totalCount',
        name: this.i18n.get('explore_lun_numbers_label'),
        sort: true,
        hidden: !SupportLicense.isSan
      },
      {
        key: 'condition',
        name: this.i18n.get('explore_lun_protection_label'),
        width: 280,
        cellRender: this.lunStatisticsTpl,
        hidden: !SupportLicense.isSan
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'id',
        columns: cols,
        scrollFixed: true,
        scroll: { y: '45vh' },
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        }
      }
    };
  }

  getData(filters?: Filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      groupBy: DetectionGroupField.TenantId
    };

    if (filters.conditions) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.name) {
        assign(params, {
          vstoreNameFilter: conditions.name,
          groupBy: 'tenantName'
        });
      }
    }

    if (filters.sort?.key === 'totalCount') {
      assign(params, {
        orderByFsNum: filters.sort?.direction,
        groupBy: 'tenantName'
      });
    }

    this.apiFilesystemPluginService
      .ListFileSystemSummayProtection(params)
      .subscribe(res => {
        res.records.filter(item => {
          assign(item, {
            totalCount: item.protectedCount + item.unprotectedCount
          });
        });
        this.tableData = {
          data: res.records,
          total: +res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  getRepicasDetail(data) {}
}
