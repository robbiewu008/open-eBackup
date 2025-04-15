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
  CopiesDetectReportService,
  DataMap,
  DetectionCopyAction,
  I18NService,
  MODAL_COMMON
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, isEmpty, isUndefined, size, trim } from 'lodash';
import { DetectionRepicasListComponent } from '../../resource-statistic/detection-repicas-list/detection-repicas-list.component';

@Component({
  selector: 'aui-base-detection-table',
  templateUrl: './base-detection-table.component.html',
  styleUrls: ['./base-detection-table.component.less']
})
export class BaseDetectionTableComponent implements OnInit, AfterViewInit {
  @Input() subType: string;

  name: string;
  optsConfig;
  dataMap = DataMap;
  tableData: TableData;
  tableConfig: TableConfig;
  selectionData = [];

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('sourceNamePopover', { static: false }) sourceNamePopover;
  @ViewChild('repicasTpl', { static: true }) repicasTpl: TemplateRef<any>;
  @ViewChild('statisticsTpl', { static: true }) statisticsTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private copiesDetectReportService: CopiesDetectReportService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      resourceSubType: this.subType,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      if (isUndefined(conditionsTemp.name)) {
        this.name = '';
      } else {
        this.name = conditionsTemp.name;
      }
      assign(params, { conditions: filters.conditions });
    } else {
      this.name = '';
    }

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.copiesDetectReportService
      .ShowDetectionStatistics({
        ...params
      })
      .subscribe(res => {
        this.tableData = {
          data: res.items,
          total: res.total
        };
      });
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'maunalDetect',
        label: this.i18n.get('explore_maunal_detect_label'),
        disableCheck: data => {
          return !size(data);
        },
        onClick: data => {
          this.maunalDetect(data[0]);
        }
      },
      {
        id: 'feedback',
        label: this.i18n.get('explore_error_feedbac_label'),
        onClick: data => {
          this.feedback(data[0]);
        }
      }
    ];
    this.optsConfig = [];
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
        key: 'tenant_name',
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'total_copy_num',
        name: this.i18n.get('common_anti_detection_snapshot_label'),
        thAlign: 'right',
        sort: true,
        cellRender: this.repicasTpl
      },
      {
        key: 'condition',
        name: this.i18n.get('explore_detection_condition_label'),
        width: 640,
        cellRender: this.statisticsTpl
      },
      {
        key: 'latest_detection_time',
        name: this.i18n.get('explore_last_detection_time_label'),
        sort: true
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: opts
          }
        }
      }
    ];

    this.tableConfig = {
      filterTags: true,
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'resource_id',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filters: Filters, args) => {
          this.getData(filters, args);
        },
        trackByFn: (index, item) => {
          return item.resource_id;
        }
      }
    };
  }

  getRepicasDetail(rowData, copyType) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_anti_detection_snapshot_label'),
      lvModalKey: 'anti-copies' + copyType,
      lvWidth: MODAL_COMMON.largeWidth,
      lvContent: DetectionRepicasListComponent,
      lvComponentParams: {
        action: DetectionCopyAction.View,
        resourceType: this.subType,
        rowData,
        copyType
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ]
    });
  }

  maunalDetect(rowData) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_maunal_detect_label'),
      lvModalKey: 'anti-copy-maunalDetect',
      lvWidth: MODAL_COMMON.largeWidth,
      lvContent: DetectionRepicasListComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as DetectionRepicasListComponent;
        content.valid$.subscribe(res => {
          modal.lvOkDisabled = !res;
        });
      },
      lvComponentParams: {
        action: DetectionCopyAction.DetectionSelect,
        resourceType: this.subType,
        rowData
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as DetectionRepicasListComponent;
          content.batchMaunalDetect().subscribe({
            next: () => {
              resolve(true);
              this.dataTable.fetchData();
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  feedback(rowData) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_error_feedbac_label'),
      lvModalKey: 'anti-copy-feedback',
      lvWidth: MODAL_COMMON.largeWidth,
      lvContent: DetectionRepicasListComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as DetectionRepicasListComponent;
        content.valid$.subscribe(res => {
          modal.lvOkDisabled = !res;
        });
      },
      lvComponentParams: {
        action: DetectionCopyAction.FeedbackSelect,
        resourceType: this.subType,
        rowData
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as DetectionRepicasListComponent;
          content.batchFeedback().subscribe({
            next: () => {
              resolve(true);
              this.dataTable.fetchData();
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  search(value) {
    this.dataTable.filterChange({
      filterMode: 'contains',
      caseSensitive: false,
      key: 'name',
      value: [trim(value)]
    });
  }
}
