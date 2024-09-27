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
import { DatePipe } from '@angular/common';
import {
  AfterViewInit,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  CommonConsts,
  CopiesDetectReportService,
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  NASAbnormalFilesService
} from 'app/shared';
import { BackupCopyDetectService } from 'app/shared/api/services/backup-copy-detect.service';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  find,
  includes,
  isEmpty,
  isNumber,
  isUndefined,
  map,
  pick,
  set
} from 'lodash';
import { PoReportDetailComponent } from './po-report-detail/po-report-detail.component';

@Component({
  selector: 'aui-snapshot-report',
  templateUrl: './snapshot-report.component.html',
  styleUrls: ['./snapshot-report.component.less'],
  providers: [DatePipe, CapacityCalculateLabel]
})
export class SnapshotReportComponent implements OnInit, AfterViewInit {
  rowData;
  copyId;
  isProductStorage = true;

  detectionIcon = '';
  detectionResultLabel = '';
  copyDataLabel = '--';
  reportLabel = '';
  formItems = [];
  tableConfig: TableConfig;
  tableData: TableData;
  seriesData = [];

  isEn = this.i18n.isEn;
  _isEmpty = isEmpty;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('nameTpl', { static: true }) nameTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private capacityCalculateLabel: CapacityCalculateLabel,
    private backupCopyDetectService: BackupCopyDetectService,
    private nasAbnormalFilesService: NASAbnormalFilesService,
    private copiesDetectReportService: CopiesDetectReportService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initProductStorage();
    this.initTableConfig();
    this.initBasicInfo();
    this.getReport();
    this.getChartData();
  }

  initProductStorage() {
    this.isProductStorage = !includes(
      ['nbu', 'cv', 'veeam'],
      this.rowData?.backup_software
    );
  }

  initTableConfig() {
    let cols: TableCols[] = [
      {
        key: 'path',
        name: this.i18n.get('common_file_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];
    if (!this.isProductStorage) {
      cols = [
        {
          key: 'protectedObjName',
          name: this.i18n.get('explore_protect_object_name_label'),
          filter: {
            type: 'search',
            filterMode: 'contains'
          },
          cellRender: this.nameTpl
        },
        {
          key: 'software',
          name: this.i18n.get('common_backup_software_label'),
          cellRender: {
            type: 'status',
            config: this.dataMapService.toArray('detectionSoftware')
          }
        },
        {
          key: 'backupCount',
          name:
            this.rowData?.backup_software === 'cv'
              ? this.i18n.get('explore_backup_files_label')
              : this.i18n.get('explore_backup_times_label'),
          sort: true
        },
        {
          key: 'status',
          name: this.i18n.get('explore_safe_status_label'),
          cellRender: {
            type: 'status',
            config: this.dataMapService.toArray('detectionSnapshotStatus')
          },
          filter: {
            type: 'select',
            isMultiple: true,
            showCheckAll: true,
            options: this.dataMapService.toArray('detectionSnapshotStatus')
          }
        }
      ];
    }
    this.tableConfig = {
      table: {
        compareWith: this.isProductStorage ? 'path' : 'name',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          if (this.isProductStorage) {
            this.getAbnormalFile(filter);
          } else {
            this.getObjects(filter);
          }
        },
        trackByFn: (_, item) => {
          return this.isProductStorage ? item.path : item.name;
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  initBasicInfo() {
    this.formItems = [
      [
        {
          label: this.i18n.get('common_name_label'),
          content: this.rowData?.name
        },
        {
          label: this.i18n.get('explore_belong_device_label'),
          content:
            this.rowData?.device_name || this.rowData?.resource_environment_name
        },
        {
          label: this.i18n.get('explore_belong_tenant_label'),
          content: this.rowData?.tenant_name
        }
      ],
      [
        {
          key: 'detection_model',
          label: this.i18n.get('explore_detection_model_new_label'),
          content: ''
        },
        {
          label: this.i18n.get('explore_detection_time_label'),
          content:
            this.rowData?.latest_detection_time || this.rowData?.detection_time
        },
        {
          label: this.i18n.get('explore_total_file_size_label'),
          content: isNumber(this.rowData?.total_file_size)
            ? this.capacityCalculateLabel.transform(
                this.rowData?.total_file_size,
                '1.0-0',
                CAPACITY_UNIT.BYTE,
                true
              )
            : ''
        }
      ]
    ];
  }

  getReport() {
    this.copiesDetectReportService
      .ShowDetectionReport({
        copyId: this.copyId
      })
      .subscribe(res => {
        const result = find(
          this.dataMapService.toArray('Detection_Copy_Status'),
          item => item.value === res.status
        );
        set(this.formItems[1][0], 'content', res.model);
        this.detectionIcon = isUndefined(result) ? '--' : result.resultIcon;
        this.copyDataLabel = `"${this.getFormatDate(res.timestamp)}"`;
        this.detectionResultLabel = `${
          isUndefined(result)
            ? '--'
            : result.value === DataMap.Detection_Copy_Status.exception.value
            ? this.i18n.get('explore_detecte_label') + result.label
            : result.label
        }`;
        this.reportLabel = res.report
          ? JSON.parse(res.report)[this.i18n.language]
          : '';
      });
  }

  getChartData() {
    const params: any = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      orders: ['-display_timestamp'],
      resourceId: this.rowData?.resource_id,
      conditions: JSON.stringify({
        endTime: this.rowData?.display_timestamp,
        anti_status: [
          DataMap.detectionSnapshotStatus.infected.value,
          DataMap.detectionSnapshotStatus.uninfected.value
        ],
        generate_type: [DataMap.snapshotGeneratetype.copyDetect.value]
      })
    };

    this.copiesDetectReportService
      .ShowDetectionDetails(params)
      .subscribe(res => {
        res.items.sort((a, b) => {
          return (
            new Date(a.display_timestamp).getTime() -
            new Date(b.display_timestamp).getTime()
          );
        });
        this.seriesData = map(res.items, item => {
          return {
            timestamp: this.datePipe.transform(
              item.display_timestamp,
              'yyyy-MM-dd HH:mm:ss'
            ),
            ...pick(item, [
              'added_file_count',
              'changed_file_count',
              'deleted_file_count',
              'infected_file_count',
              'anti_status'
            ])
          };
        });
      });
  }

  getAbnormalFile(filters) {
    const params: any = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      copyId: this.copyId
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.path) {
        assign(params, {
          filePath: conditions.path
        });
      } else {
        assign(params, conditions);
      }
    }

    if (!isEmpty(filters.sort) && !isEmpty(filters.sort.key)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }

    this.nasAbnormalFilesService
      .queryNasInfectedFiles(params)
      .subscribe((res: any) => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }

  getObjects(filters) {
    const params: any = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      snapshotId: this.copyId
    };

    if (!isEmpty(filters.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      if (conditionsTemp.protectedObjName) {
        assign(params, {
          name: conditionsTemp.protectedObjName
        });
      }
      if (conditionsTemp.status) {
        assign(params, {
          statusList: conditionsTemp.status
        });
      }
    }

    if (!isEmpty(filters.sort) && !isEmpty(filters.sort.key)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }

    this.backupCopyDetectService
      .QueryObjectsBySnapshotId(params)
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }

  protectObjectDetail(rowData) {
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'object-report-detail',
        lvHeader: rowData?.name,
        lvWidth: MODAL_COMMON.xLargeWidth,
        lvContent: PoReportDetailComponent,
        lvComponentParams: { rowData, snapshotId: this.copyId },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  getFormatDate(date) {
    return this.datePipe.transform(date, 'yyyy-MM-dd HH:mm:ss');
  }
}
