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
  CAPACITY_UNIT,
  CapacityCalculateLabel,
  CommonConsts,
  CopiesDetectReportService,
  DataMap,
  DataMapService,
  I18NService,
  IODETECTREPORTService,
  SYSTEM_TIME
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, find, isEmpty, isUndefined, now, set } from 'lodash';

@Component({
  selector: 'aui-io-snapshot-report',
  templateUrl: './io-snapshot-report.component.html',
  styleUrls: ['./io-snapshot-report.component.less'],
  providers: [DatePipe, CapacityCalculateLabel]
})
export class IoSnapshotReportComponent implements OnInit, AfterViewInit {
  rowData;
  copyId;

  isEn = this.i18n.isEn;
  unitconst = CAPACITY_UNIT;
  detectionIcon = '';
  detectionResultLabel = '';
  copyDataLabel = '';
  formItems = [];
  tableConfig: TableConfig;
  tableData: TableData;
  resourceProperties;
  properties;
  reportLabel = '';

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('attrTpl', { static: true })
  attrTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true })
  sizeTpl: TemplateRef<any>;
  @ViewChild('subjectUserTpl', { static: true }) subjectUserTpl: TemplateRef<
    any
  >;
  @ViewChild('fileTpl', { static: true })
  fileTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private ioDetectReportService: IODETECTREPORTService,
    private copiesDetectReportService: CopiesDetectReportService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.initTableConfig();
    this.getReport();
    this.getFormItems();
  }

  initTableConfig() {
    this.resourceProperties = JSON.parse(this.rowData?.resource_properties);
    this.properties = JSON.parse(this.rowData?.properties);
    const cols: TableCols[] = [
      {
        key: 'subjectIp',
        name: this.i18n.get('explore_source_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'subjectUser',
        name: this.i18n.get('explore_source_username_label'),
        thExtra: this.subjectUserTpl,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'fileSize',
        name: this.i18n.get('common_size_label'),
        sort: true,
        cellRender: this.sizeTpl
      },
      {
        key: 'file',
        name: this.i18n.get('common_file_path_label'),
        cellRender: this.fileTpl
      },
      {
        key: 'createDate',
        name: this.i18n.get('explore_infected_file_attr_label'),
        width: 300,
        cellRender: this.attrTpl
      }
    ];
    this.tableConfig = {
      table: {
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filters: Filters) => this.getSuspectFile(filters)
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  getSuspectFile(filters) {
    const params = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      vstoreId: this.properties?.tenantId,
      snapShotName: this.properties?.snapshotName,
      fileSystemName: this.properties?.filesystemName,
      deviceId: this.resourceProperties?.root_uuid
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      assign(params, conditions);
    }

    if (!isEmpty(filters.sort) && !isEmpty(filters.sort.key)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy:
          filters.sort.key === 'fileSize' ? 'suspectFileSize' : filters.sort.key
      });
    }

    this.ioDetectReportService
      .getSuspectFileReport(params)
      .subscribe((res: any) => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }

  exportResult() {
    this.ioDetectReportService
      .exportSuspectFileReportResponse({
        vstoreId: this.properties?.tenantId,
        snapShotName: this.properties?.snapshotName,
        fileSystemName: this.properties?.filesystemName,
        deviceId: this.resourceProperties?.root_uuid,
        lang: this.i18n.isEn ? 'en' : 'zh'
      })
      .subscribe(res => {
        let fileName;
        try {
          fileName = res.headers
            .get('content-disposition')
            .split('filename=')[1];
        } catch (error) {
          fileName = `report_${now()}.csv`;
        }
        const bf = new Blob([res.body], {
          type: 'text/csv'
        });
        this.appUtilsService.downloadFile(fileName, bf);
      });
  }

  getFormatDate(date) {
    return this.datePipe.transform(date, 'yyyy-MM-dd HH:mm:ss');
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
        const detectTime = this.getFormatDate(res.timestamp);
        set(this.formItems[0][1], 'content', detectTime);
        set(this.formItems[1][1], 'content', res.model);
        this.detectionIcon = isUndefined(result) ? '--' : result.resultIcon;
        this.copyDataLabel = detectTime;
        this.detectionResultLabel = `${
          isUndefined(result)
            ? '--'
            : result.value === DataMap.Detection_Copy_Status.exception.value
            ? this.i18n.get('explore_detecte_label') + result.label
            : result.label
        }`;
        if (this.rowData?.handle_false && res.report) {
          try {
            this.reportLabel = JSON.parse(res.report)[this.i18n.language];
          } catch (error) {
            this.reportLabel = '';
          }
        }
      });
  }

  getFormItems() {
    this.formItems = [
      [
        {
          label: this.i18n.get('explore_belong_device_label'),
          content:
            this.rowData?.device_name || this.rowData?.resource_environment_name
        },
        {
          label: this.i18n.get('explore_detection_time_label'),
          content: ''
        }
      ],
      [
        {
          label: this.i18n.get('explore_belong_tenant_label'),
          content: this.rowData?.tenant_name
        },
        {
          key: 'detection_model',
          label: this.i18n.get('explore_detection_model_new_label'),
          content: ''
        }
      ]
    ];
  }
}
