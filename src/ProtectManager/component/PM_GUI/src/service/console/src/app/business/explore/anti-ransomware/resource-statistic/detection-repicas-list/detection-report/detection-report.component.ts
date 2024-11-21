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
import { AfterViewInit, Component, OnInit, ViewChild } from '@angular/core';
import {
  AbnormalFileService,
  CapacityCalculateLabel,
  CommonConsts,
  CopiesDetectReportService,
  DataMap,
  DataMapService,
  I18NService,
  NASAbnormalFilesService,
  SanConfigManagementService,
  SYSTEM_TIME
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { TimeTransformService } from 'app/shared/services/time-transform.service';
import {
  assign,
  find,
  get,
  includes,
  isEmpty,
  isNil,
  isString,
  isUndefined,
  now,
  set
} from 'lodash';

@Component({
  selector: 'aui-detection-report',
  templateUrl: './detection-report.component.html',
  styleUrls: ['./detection-report.component.less'],
  providers: [CapacityCalculateLabel, DatePipe]
})
export class DetectionReportComponent implements OnInit, AfterViewInit {
  copyId;
  resourceType;
  entity; //实时勒索检测报告--告警详情页跳过来的
  dataMap = DataMap;
  rowData: any;
  reportLabel = '';
  _isEmpty = isEmpty;
  lastTestLabel = this.i18n.get('explore_last_test_label', [], true);
  DurationLabel = this.i18n.get('explore_detection_duration_label', [], true);
  fileDurationLabel = this.i18n.get(
    'explore_file_detection_duration_label',
    [],
    true
  );
  detectionIcon = '';
  timeUnitLabel = '';
  fileTimeUnitLabel = '';
  detectingTime = '--';
  copyDataLabel = '--';
  detectionResultLabel = '';
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  filesystemName: string;
  columnItems = [];
  isEn = this.i18n.isEn;
  isInfected = false;
  ishiddenDeploy = includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );

  hideAbnormalFile = includes(
    [
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.hyperdetect.value
    ],
    this.i18n.get('deploy_type')
  );

  tableConfig: TableConfig;
  tableData: TableData;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private dataMapService: DataMapService,
    public appUtilsService: AppUtilsService,
    private capacityCalculateLabel: CapacityCalculateLabel,
    private copiesDetectReportService: CopiesDetectReportService,
    private timeTransformServicve: TimeTransformService,
    private nasAbnormalFilesService: NASAbnormalFilesService,
    private abnormalFileService: AbnormalFileService,
    private sanConfigManagementService: SanConfigManagementService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable?.fetchData();
  }

  ngOnInit() {
    if (this.entity) {
      this.getSanIoReport();
    } else {
      this.initTableConfig();
      this.initData();
    }
  }

  initCyberData(res) {
    if (!this.isCyberEngine) {
      return;
    }
    this.filesystemName = this.rowData?.resource_name || this.rowData?.name;
    this.columnItems = [
      [
        {
          key: this.i18n.get('explore_belong_device_label'),
          value: `${this.rowData?.resource_environment_name ||
            this.rowData?.device_name}(${this.rowData
            ?.resource_environment_ip || this.rowData?.device_ip})`
        },
        {
          key: this.i18n.get('explore_belong_tenant_label'),
          value: this.rowData?.tenant_name
        }
      ],
      [
        {
          key: this.i18n.get('explore_detection_time_label'),
          value:
            this.rowData?.detection_time || this.rowData?.latest_detection_time
        }
      ]
    ];
  }

  initData() {
    this.copiesDetectReportService
      .ShowDetectionReport({
        copyId: this.copyId
      })
      .subscribe(res => {
        const result = find(
          this.dataMapService.toArray('Detection_Copy_Status'),
          item => item.value === res.status
        );
        this.detectionIcon = isUndefined(result) ? '--' : result.resultIcon;
        this.copyDataLabel = `"${this.getFormatDate(res.timestamp)}"`;
        this.lastTestLabel = `${this.lastTestLabel}${this.getFormatDate(
          res.detection_time
        )}`;
        this.detectionResultLabel = `${
          isUndefined(result)
            ? '--'
            : result.value === DataMap.Detection_Copy_Status.exception.value
            ? this.i18n.get('explore_detecte_label') + result.label
            : result.label
        }`;
        const report = res.report ? JSON.parse(res.report) : {};
        if (
          !isEmpty(report['error-code']) &&
          report['error-code'] !== '1677929221'
        ) {
          this.reportLabel = isString(this.i18n.get(report['error-code']))
            ? this.i18n.get(report['error-code'])?.replace('<br>', '')
            : this.i18n.get(report['error-code']);
        } else {
          this.reportLabel = report[this.i18n.language] || '';
        }
        this.timeUnitLabel = this.timeTransformServicve.transformTime(
          res.detection_duration
        );
        this.isInfected =
          result.value === DataMap.Detection_Copy_Status.infected.value &&
          !isNil(res?.infected_file_detect_duration);
        this.fileTimeUnitLabel = this.timeTransformServicve.transformTime(
          res?.infected_file_detect_duration
        );
        this.initCyberData(res);
      });
  }

  getFormatDate(date) {
    return this.datePipe.transform(date, 'yyyy-MM-dd HH:mm:ss');
  }

  initTableConfig() {
    if (this.hideAbnormalFile) {
      return;
    }
    this.tableConfig = {
      table: {
        compareWith: 'path',
        columns: [
          {
            key: 'path',
            name: this.i18n.get('common_file_path_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          }
        ],
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getAbnormalFile(filter);
        },
        trackByFn: (_, item) => {
          return item.path;
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

    this.nasAbnormalFilesService
      .queryNasInfectedFiles(params)
      .subscribe((res: any) => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }

  exportResult() {
    this.abnormalFileService
      .downloadAbnormalFileResponse({
        copyId: this.copyId
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

  getSanIoReport() {
    this.detectionIcon = 'aui-icon-detection-infected';
    this.detectionResultLabel = this.i18n.get('explore_infected_label');
    this.sanConfigManagementService
      .getSanIoReport({ entity: this.entity })
      .subscribe(res => {
        const report = JSON.parse(get(res?.records, 'report', '{}'));
        this.reportLabel = report[this.i18n.language] || '';
      });
  }
}
