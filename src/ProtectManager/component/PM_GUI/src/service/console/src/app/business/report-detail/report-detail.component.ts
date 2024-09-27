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
  ElementRef,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import {
  DataMapService,
  DetectReportAPIService,
  I18NService,
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  Report_Agent_Type,
  CommonConsts,
  DataMap,
  SoftwareType,
  isNotFileSystem
} from 'app/shared';
import { DatePipe } from '@angular/common';
import {
  assign,
  cloneDeep,
  defer,
  each,
  get,
  includes,
  isEmpty,
  isNil,
  isUndefined,
  map,
  pick,
  set,
  size
} from 'lodash';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { StatusConfig } from 'app/shared/components/pro-status';
import * as echarts from 'echarts';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
@Component({
  selector: 'aui-report-detail',
  templateUrl: './report-detail.component.html',
  styleUrls: ['./report-detail.component.less'],
  providers: [DatePipe, CapacityCalculateLabel]
})
export class ReportDetailComponent implements OnInit {
  includes = includes;
  reportName: string;
  reportCreateTime: string;
  detectStartTime: string;
  detectEndTime: string;
  snapshotCount: number;
  fileSystemName: string;
  infectionSnapshotCount: number;
  totalSnapshotCount;
  handleFalseCount: number;
  fileSystemLocation: string;

  loaded: boolean = false;
  tableConfig: TableConfig;
  tableData: TableData;
  statusConfig: StatusConfig = this.dataMapService.getConfig(
    'Detection_Copy_Status',
    true
  );
  chart;
  tips = 'insight_report_others_info_label';
  reportDetailInfectedCopyNum = 0;
  reportDetailContent = '';
  infectedFileTableConfig: TableConfig;
  infectedFileTableData: TableData;
  latestPoTableConfig: TableConfig;
  latestPoTableData: TableData;

  reportDataResponse;
  unitconst = CAPACITY_UNIT;
  isProtectObjectDetail = false;
  capacityData = [];
  seriesData = [];
  assCopyTableConfig: TableConfig;
  assCopyTableData: TableData;
  protectObjectName;
  poTips = 'explore_detect_po_detail_label';
  protectObjectDetectTime;
  protectObjectBackupTimes;
  owningFilesystem;
  protectObjectFileSystemLocation;
  snapshotGenerateTime;
  infectedStartTime;
  infectedEndTime;
  normalFileCapacity;
  infectionFileCapacity;
  poTnfectedFileTableConfig: TableConfig;
  poTnfectedFileTableData: TableData;
  latestInfectedFileNum = 0;
  latestInfectedFileDetail = '';
  protectObjectCopyDetailTitle = '';
  protectObjectCopyDetail = '';
  softwareType;
  hasLatestInfectionCopyInfo;
  softwareTypeEnum = SoftwareType;
  hasAbnormalBackupCount;
  isIoDetectionReport = false;
  ioInfectedFileTableConfig: TableConfig;
  ioInfectedFileTableData: TableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('poTnfectedFileTable', { static: false })
  poTnfectedFileTable: ProTableComponent;
  @ViewChild('assCopyTable', { static: false })
  assCopyTable: ProTableComponent;
  @ViewChild('latestPoTable', { static: false })
  latestPoTable: ProTableComponent;
  @ViewChild('statusTpl', { static: true }) statusTpl: TemplateRef<any>;
  @ViewChild('poNameTpl', { static: true }) poNameTpl: TemplateRef<any>;
  @ViewChild('dateTpl', { static: true }) dateTpl: TemplateRef<any>;
  @ViewChild('fileAttrTpl', { static: true }) fileAttrTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('copySizeTpl', { static: true }) copySizeTpl: TemplateRef<any>;
  @ViewChild('addFileCountTpl', { static: true }) addFileCountTpl: TemplateRef<
    any
  >;
  @ViewChild('changeFileCountTpl', { static: true })
  changeFileCountTpl: TemplateRef<any>;
  @ViewChild('deleteFileCountTpl', { static: true })
  deleteFileCountTpl: TemplateRef<any>;
  @ViewChild('infectedFileCountTpl', { static: true })
  infectedFileCountTpl: TemplateRef<any>;
  @ViewChild('backupTaskExtraTpl', { static: true })
  backupTaskExtraTpl: TemplateRef<any>;

  @ViewChild('copyTypeTpl', { static: true }) copyTypeTpl: TemplateRef<any>;
  @ViewChild('fileCountExtraTpl', { static: true })
  fileCountExtraTpl: TemplateRef<any>;
  @ViewChild('infectedFileCountExtraTpl', { static: true })
  infectedFileCountExtraTpl: TemplateRef<any>;
  @ViewChild('fileCountTpl', { static: true })
  fileCountTpl: TemplateRef<any>;
  @ViewChild('infectFileCountTpl', { static: true })
  infectFileCountTpl: TemplateRef<any>;
  @ViewChild('attrTpl', { static: true })
  attrTpl: TemplateRef<any>;
  @ViewChild('subjectUserTpl', { static: true }) subjectUserTpl: TemplateRef<
    any
  >;

  constructor(
    private detectReportApi: DetectReportAPIService,
    private capacityCalculateLabel: CapacityCalculateLabel,
    private dataMapService: DataMapService,
    private route: ActivatedRoute,
    private datePipe: DatePipe,
    private el: ElementRef,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initTable().loadData();
    this.initInfectedFileTable();
    this.initLatestPoTable();
    this.initAssCopyTable();
    this.initLatestCopyFileTable();
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('protection_hyperdetect_copy_name_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'generated_time',
        name: this.i18n.get('explore_snapshot_create_time_label'),
        sort: true
      },
      {
        key: 'anti_status',
        name: this.i18n.get('explore_safe_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('detectionSnapshotStatus')
            .filter(item =>
              includes(
                [
                  DataMap.detectionSnapshotStatus.infected.value,
                  DataMap.detectionSnapshotStatus.uninfected.value
                ],
                item.value
              )
            )
        },
        cellRender: this.statusTpl
      },
      {
        key: 'model',
        name: this.i18n.get('explore_detection_model_label')
      },
      {
        key: 'generate_type',
        name: this.i18n.get('explore_snapshot_create_way_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('snapshotGeneratetype')
        }
      },
      {
        key: 'total_file_size',
        name: this.i18n.get('explore_total_file_size_label'),
        sort: true
      },
      {
        key: 'added_file_count',
        name: this.i18n.get('explore_new_file_num_label'),
        thExtra: this.addFileCountTpl,
        sort: true
      },
      {
        key: 'changed_file_count',
        name: this.i18n.get('explore_modify_file_count_label'),
        sort: true,
        thExtra: this.changeFileCountTpl
      },
      {
        key: 'deleted_file_count',
        name: this.i18n.get('explore_delete_file_count_label'),
        sort: true,
        thExtra: this.deleteFileCountTpl
      },
      {
        key: 'infected_file_count',
        name: this.i18n.get('explore_suspicious_file_num_label'),
        sort: true,
        thExtra: this.infectedFileCountTpl
      },
      {
        key: 'detection_time',
        name: this.i18n.get('explore_detect_end_time_label'),
        sort: true
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        columns: cols,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        scrollFixed: true
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple'
      }
    };
    return this;
  }

  initInfectedFileTable() {
    const cols: TableCols[] = [
      {
        key: 'fileName',
        name: this.i18n.get('explore_file_name_label')
      },
      {
        key: 'filePath',
        name: this.i18n.get('common_file_path_label')
      }
    ];
    this.infectedFileTableConfig = {
      table: {
        async: false,
        columns: cols,
        colDisplayControl: false,
        scrollFixed: true
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
    this.ioInfectedFileTableConfig = {
      table: {
        async: false,
        columns: [
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
            key: 'fileName',
            name: this.i18n.get('common_file_path_label')
          },
          {
            key: 'createDate',
            name: this.i18n.get('explore_infected_file_attr_label'),
            width: 300,
            cellRender: this.attrTpl
          }
        ],
        colDisplayControl: false,
        scrollFixed: true
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  initLatestPoTable() {
    const cols: TableCols[] = [
      {
        key: 'protectObjectName',
        name: this.i18n.get('common_name_label'),
        cellRender: this.poNameTpl
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
        key: 'detectCounts',
        name: this.i18n.get('explore_backup_times_label'),
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
    this.latestPoTableConfig = {
      table: {
        async: false,
        columns: cols,
        colDisplayControl: false,
        scrollFixed: true
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  initLatestCopyFileTable() {
    if (!this.isProtectObjectDetail) {
      return;
    }
    const cols: TableCols[] = [
      {
        key: 'fileName',
        name: this.i18n.get('explore_file_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'filePath',
        name: this.i18n.get('common_file_path_label')
      },
      {
        key: 'fileSize',
        name: this.i18n.get('explore_capacity_size_label'),
        sort: true,
        cellRender: this.sizeTpl
      },
      {
        key: 'latestModifyTime',
        name: this.i18n.get('explore_infected_file_attr_label'),
        cellRender: this.fileAttrTpl
      }
    ];

    this.poTnfectedFileTableConfig = {
      table: {
        async: false,
        columns: cols,
        colDisplayControl: false,
        scrollFixed: true
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  initAssCopyTable() {
    if (!this.isProtectObjectDetail) {
      return;
    }
    const cols: TableCols[] = [
      {
        key: 'backupTime',
        name: this.i18n.get('explore_copy_generation_time_label'),
        cellRender: this.dateTpl
      },
      {
        key: 'backupType',
        name: this.i18n.get('common_backup_type_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('CopyData_Backup_Type')
        }
      },
      {
        key: 'backupJobId',
        name: this.i18n.get('explore_backup_task_label'),
        thExtra: this.backupTaskExtraTpl,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'copySize',
        name: this.i18n.get('explore_copy_size_label'),
        cellRender: this.copySizeTpl
      },
      {
        key: 'copyPath',
        name: this.i18n.get('common_file_path_label')
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
      },
      {
        key: 'copyType',
        name: this.i18n.get('explore_backup_application_type_label'),
        cellRender: this.copyTypeTpl
      },
      {
        key: 'fileCount',
        name: this.i18n.get('explore_associated_file_count_label'),
        thExtra: this.fileCountExtraTpl,
        cellRender: this.fileCountTpl
      },
      {
        key: 'abnormalFileCount',
        name: this.i18n.get('explore_infected_file_count_label'),
        thExtra: this.infectedFileCountExtraTpl,
        cellRender: this.infectFileCountTpl
      }
    ];
    this.assCopyTableConfig = {
      table: {
        async: false,
        columns: cols,
        colDisplayControl: false,
        scrollFixed: true
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  getAppType(item) {
    return item.copyType & 0xff;
  }

  isVMware(item) {
    return isNotFileSystem(this.getAppType(item));
  }

  getCopyType(item): string {
    return this.appUtilsService.getCopyType(
      this.getAppType(item),
      this.softwareType
    );
  }

  protectObjectDetail(item) {
    window.open(`/console/#/report-detail/po_${item.protectObjectUuid}`);
  }

  initFileCapacityChart(normalCapacity, infectedCapacity) {
    const chartDom = document.getElementById('file-capacity-chart');
    if (!chartDom) {
      return;
    }
    const chart = echarts.init(chartDom);
    const option = {
      graphic: [
        {
          id: 'total',
          type: 'text',
          left: 'center',
          top: '42%',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: this.capacityCalculateLabel.transform(
              normalCapacity + infectedCapacity,
              '1.3-3',
              CAPACITY_UNIT.BYTE
            ),
            textAlign: 'center',
            fill: '#282B33',
            width: 30,
            height: 30,
            fontSize: 22
          }
        },
        {
          type: 'text',
          left: 'center',
          top: '58%',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: this.i18n.get('common_all_label'),
            textAlign: 'center',
            fill: '#B8BECC',
            width: 30,
            height: 30,
            fontSize: 14
          }
        },
        {
          type: 'ring',
          left: 'center',
          top: 'center',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          shape: {
            r: 110,
            r0: 109
          },
          style: {
            fill: '#DDDDDD',
            width: 30,
            height: 30
          }
        }
      ],
      series: [
        {
          name: 'capacity',
          type: 'pie',
          radius: [84, 101],
          color: ['#F67D7E', '#7ADFA0'],
          avoidLabelOverlap: false,
          hoverAnimation: false,
          selectedMode: false,
          legendHoverLink: false,
          cursor: 'unset',
          label: {
            show: false,
            emphasis: {
              show: false,
              textStyle: {
                fontSize: '30',
                fontWeight: 'bold'
              }
            }
          },
          labelLine: {
            normal: {
              show: false
            }
          },
          data: [
            {
              value: infectedCapacity,
              name: this.i18n.get(
                'explore_detection_statistics_infected_file_label'
              )
            },
            {
              value: normalCapacity,
              name: this.i18n.get(
                'explore_detection_statistics_normal_file_label'
              )
            }
          ]
        }
      ]
    };
    chart.setOption(option);
  }

  initStatisticsChart(addCount, changeCount, deleteCount, infectCount?) {
    const chartDom = document.getElementById('statistics-chart');
    if (!chartDom) {
      return;
    }
    const chart = echarts.init(chartDom);
    const option = {
      xAxis: {
        type: 'category',
        data: isUndefined(infectCount)
          ? [
              this.i18n.get('explore_detection_statistics_total_file_label'),
              this.i18n.get('explore_detection_statistics_normal_file_label'),
              this.i18n.get('explore_detection_statistics_infected_file_label')
            ]
          : [
              this.i18n.get('explore_detection_statistics_new_file_label'),
              this.i18n.get('explore_detection_statistics_change_file_label'),
              this.i18n.get('explore_detection_statistics_delete_file_label'),
              this.i18n.get('explore_detection_statistics_infected_file_label')
            ],
        axisLine: {
          show: false
        },
        axisTick: {
          show: false
        }
      },
      yAxis: {
        type: 'value',
        show: false,
        axisLine: {
          show: false
        },
        axisTick: {
          show: false
        }
      },
      series: [
        {
          data: isUndefined(infectCount)
            ? [
                {
                  value: addCount,
                  itemStyle: {
                    color: '#6C92FA'
                  }
                },
                {
                  value: changeCount,
                  itemStyle: {
                    color: '#7ADFA0'
                  }
                },
                {
                  value: deleteCount,
                  itemStyle: {
                    color: '#F67D7E'
                  }
                }
              ]
            : [
                {
                  value: addCount,
                  itemStyle: {
                    color: '#63B6F7'
                  }
                },
                {
                  value: changeCount,
                  itemStyle: {
                    color: '#62DAD0'
                  }
                },
                {
                  value: deleteCount,
                  itemStyle: {
                    color: '#F4B853'
                  }
                },
                {
                  value: infectCount,
                  itemStyle: {
                    color: '#F45C5E'
                  }
                }
              ],
          type: 'bar',
          barWidth: 64,
          itemStyle: {
            label: {
              show: true,
              position: 'top',
              color: '#2D3038'
            }
          }
        }
      ]
    };
    if (chart) {
      chart.setOption(option);
    }
  }

  loadData() {
    const reportId = this.route.snapshot.params.id;
    this.isProtectObjectDetail = includes(reportId, 'po');
    if (this.isProtectObjectDetail) {
      this.detectReportApi
        .ShowProtectObjectDetectReportById({
          protectObjectUuid: reportId.replace('po_', '')
        })
        .subscribe((res: any) => {
          this.loaded = true;
          this.softwareType = res.protectObjectInfo?.software;
          if (this.softwareType === SoftwareType.CV) {
            defer(() =>
              this.poTnfectedFileTable?.setColsDisplay(['fileName', 'filePath'])
            );
          }
          this.protectObjectName = res.protectObjectInfo?.protectedObjName;
          this.protectObjectBackupTimes = res.protectObjectInfo?.backupCount;
          this.owningFilesystem = res.protectObjectInfo?.fileSystemName;
          this.protectObjectFileSystemLocation =
            res.protectObjectInfo?.fileSystemLocation;
          this.protectObjectDetectTime =
            res.protectObjectInfo?.protectObjectDetectTime;
          this.snapshotGenerateTime =
            res.protectObjectInfo?.snapshotGenerateTime;
          this.infectedStartTime =
            res.protectObjectInfo?.latestInfectionStartTime;
          this.infectedEndTime = res.protectObjectInfo?.latestInfectionEndTime;
          if (
            res.protectObjectInfo?.status !==
            DataMap.detectionSnapshotStatus.infected.value
          ) {
            if (this.softwareType === SoftwareType.VEEAM) {
              this.poTips = this.i18n.get(
                'explore_detect_po_detail_no_data_veeam_label',
                [
                  this.owningFilesystem,
                  this.protectObjectDetectTime,
                  this.protectObjectName,
                  this.protectObjectBackupTimes
                ],
                false,
                true
              );
            } else {
              this.poTips = this.i18n.get(
                'explore_detect_po_detail_no_data_label',
                [
                  this.owningFilesystem,
                  this.protectObjectDetectTime,
                  this.protectObjectName,
                  this.protectObjectBackupTimes
                ],
                false,
                true
              );
            }
          } else {
            if (this.softwareType === SoftwareType.VEEAM) {
              this.poTips = this.i18n.get(
                'explore_detect_po_detail_veeam_label',
                [
                  this.owningFilesystem,
                  this.protectObjectDetectTime,
                  this.protectObjectName,
                  this.protectObjectBackupTimes,
                  this.infectedStartTime,
                  this.infectedEndTime
                ],
                false,
                true
              );
            } else if (this.softwareType === SoftwareType.CV) {
              this.poTips = this.i18n.get(
                'explore_detect_po_detail_cv_label',
                [
                  this.owningFilesystem,
                  this.protectObjectDetectTime,
                  this.protectObjectName,
                  this.protectObjectBackupTimes,
                  res.protectObjectInfo?.abnormalBackupCount,
                  res.protectObjectInfo?.latestInfectionBackupCopyTime,
                  res.latestInfectionCopyInfo?.infectionFileCount
                ],
                false,
                true
              );
            } else {
              this.poTips = this.i18n.get(
                'explore_detect_po_detail_not_veeam_label',
                [
                  this.owningFilesystem,
                  this.protectObjectDetectTime,
                  this.protectObjectName,
                  this.protectObjectBackupTimes,
                  res.protectObjectInfo?.abnormalBackupCount,
                  res.protectObjectInfo?.latestInfectionBackupCopyTime,
                  res.latestInfectionCopyInfo?.infectionFileCount,
                  this.capacityCalculateLabel.transform(
                    res.latestInfectionCopyInfo?.infectionFileCapacity,
                    '1.3-3',
                    CAPACITY_UNIT.BYTE
                  )
                ],
                false,
                true
              );
            }
          }

          //容量趋势
          defer(() => {
            this.capacityData = map(
              cloneDeep(res.backupCopyInfoList).sort((a, b) => {
                return (
                  new Date(a.backupTime).getTime() -
                  new Date(b.backupTime).getTime()
                );
              }),
              item => {
                return {
                  time: item.backupTime,
                  size: item.copySize,
                  status: item.status
                };
              }
            );
          });
          // 关联副本文件
          if (this.softwareType === SoftwareType.CV) {
            this.assCopyTable.setColsDisplay([
              'backupTime',
              'backupJobId',
              'copySize',
              'copyPath',
              'status',
              'copyType',
              'fileCount',
              'abnormalFileCount'
            ]);
          } else if (this.softwareType === SoftwareType.VEEAM) {
            this.assCopyTable.setColsDisplay([
              'backupTime',
              'backupType',
              'backupJobId',
              'copyPath',
              'status',
              'copyType'
            ]);
          } else {
            this.assCopyTable.setColsDisplay([
              'backupTime',
              'backupType',
              'backupJobId',
              'copySize',
              'copyPath',
              'status',
              'copyType',
              'fileCount',
              'abnormalFileCount'
            ]);
          }
          this.assCopyTableData = {
            data: res.backupCopyInfoList,
            total: size(res.backupCopyInfoList)
          };
          this.hasAbnormalBackupCount = !!res.protectObjectInfo
            ?.abnormalBackupCount;
          if (!res.protectObjectInfo?.abnormalBackupCount) {
            this.protectObjectCopyDetail = this.i18n.get(
              'explore_po_infected_no_copy_detail_label',
              [
                res.protectObjectInfo?.reportCreateTime,
                res.protectObjectInfo?.fileSystemName,
                res.protectObjectInfo?.protectedObjName
              ],
              false,
              true
            );
          } else {
            if (this.softwareType === SoftwareType.VEEAM) {
              this.protectObjectCopyDetailTitle = this.i18n.get(
                'explore_po_infected_copy_veeam_label',
                [res.protectObjectInfo?.abnormalBackupCount]
              );
              this.protectObjectCopyDetail = this.i18n.get(
                'explore_po_infected_copy_detail_veeam_label',
                [
                  res.protectObjectInfo?.reportCreateTime,
                  res.protectObjectInfo?.fileSystemName,
                  res.protectObjectInfo?.protectedObjName,
                  res.protectObjectInfo?.latestInfectionStartTime,
                  res.protectObjectInfo?.latestInfectionEndTime
                ],
                false,
                true
              );
            } else {
              this.protectObjectCopyDetailTitle = this.i18n.get(
                'explore_po_infected_copy_label',
                [res.protectObjectInfo?.abnormalBackupCount]
              );
              this.protectObjectCopyDetail = this.i18n.get(
                'explore_po_infected_copy_detail_label',
                [
                  res.protectObjectInfo?.reportCreateTime,
                  res.protectObjectInfo?.fileSystemName,
                  res.protectObjectInfo?.protectedObjName,
                  res.protectObjectInfo?.abnormalBackupCount,
                  res.protectObjectInfo?.latestInfectionBackupCopyTime
                ],
                false,
                true
              );
            }
          }

          this.hasLatestInfectionCopyInfo = !isEmpty(
            res.latestInfectionCopyInfo
          );
          // 异常文件趋势图
          defer(() => {
            this.initStatisticsChart(
              res.latestInfectionCopyInfo?.totalFileCount || 0,
              res.latestInfectionCopyInfo?.normalFileCount || 0,
              res.latestInfectionCopyInfo?.infectionFileCount || 0
            );
          });
          // 异常文件容量图
          this.normalFileCapacity =
            res.latestInfectionCopyInfo?.normalFileCapacity || 0;
          this.infectionFileCapacity =
            res.latestInfectionCopyInfo?.infectionFileCapacity || 0;
          defer(() => {
            this.initFileCapacityChart(
              this.normalFileCapacity,
              this.infectionFileCapacity
            );
          });
          // 异常文件列表
          this.poTnfectedFileTableData = {
            data: res.latestInfectionCopyInfo?.infectionFileList,
            total: size(res.latestInfectionCopyInfo?.infectionFileList)
          };
          this.latestInfectedFileNum =
            res.latestInfectionCopyInfo?.infectionFileCount;
          if (this.softwareType === SoftwareType.CV) {
            this.latestInfectedFileDetail = this.i18n.get(
              'explore_detect_latest_cv_po_copy_label',
              [
                res.latestInfectionCopyInfo?.copyDetectTime,
                res.latestInfectionCopyInfo?.fileSystemName,
                res.latestInfectionCopyInfo?.protectObjectName,
                res.latestInfectionCopyInfo?.copyTime,
                res.latestInfectionCopyInfo?.infectionFileCount
              ],
              false,
              true
            );
          } else {
            this.latestInfectedFileDetail = this.i18n.get(
              res.latestInfectionCopyInfo?.changedFileCapacity > 0
                ? 'explore_detect_latest_po_copy_label'
                : 'explore_detect_latest_po_copy_less_label',
              [
                res.latestInfectionCopyInfo?.copyDetectTime,
                res.latestInfectionCopyInfo?.fileSystemName,
                res.latestInfectionCopyInfo?.protectObjectName,
                res.latestInfectionCopyInfo?.copyTime,
                this.capacityCalculateLabel.transform(
                  Math.abs(res.latestInfectionCopyInfo?.changedFileCapacity),
                  '1.3-3',
                  CAPACITY_UNIT.BYTE
                ),
                res.latestInfectionCopyInfo?.infectionFileCount,
                this.capacityCalculateLabel.transform(
                  res.latestInfectionCopyInfo?.infectionFileCapacity,
                  '1.3-3',
                  CAPACITY_UNIT.BYTE
                )
              ],
              false,
              true
            );
          }
        });
    } else {
      this.detectReportApi.ShowDetectReportById({ reportId }).subscribe(
        res => {
          this.reportDataResponse = res;
          if (res?.detectReportType === Report_Agent_Type.built_in) {
            this.tips = 'insight_report_others_info_external_label';
          }
          set(
            this,
            'reportCreateTime',
            this.datePipe.transform(
              get(res, 'reportCreateTime'),
              'yyyy/MM/dd HH:mm:ss'
            )
          );
          each(
            [
              'detectEndTime',
              'detectStartTime',
              'snapshotCount',
              'fileSystemName',
              'infectionSnapshotCount',
              'handleFalseCount',
              'fileSystemLocation',
              'reportName'
            ],
            key => set(this, key, get(res, key))
          );
          each(this.statusConfig, item => {
            set(item as any, 'label', this.i18n.get(get(item, 'label')));
          });

          this.loaded = true;
          this.tableData = {
            data: map(res.snapshotDetectDataList, item => ({
              ...item,
              model: `Model${item.model}`,
              total_file_size: isNil(item.total_file_size)
                ? void 0
                : this.capacityCalculateLabel.transform(
                    item.total_file_size,
                    '1.3-3',
                    CAPACITY_UNIT.BYTE
                  )
            })),
            total: size(res.snapshotDetectDataList)
          };
          this.totalSnapshotCount = size(res.snapshotDetectDataList);
          defer(() => {
            this.seriesData = map(
              cloneDeep(res.snapshotDetectDataList).sort((a, b) => {
                return (
                  new Date(a.generated_time).getTime() -
                  new Date(b.generated_time).getTime()
                );
              }),
              item => {
                return {
                  timestamp: item.generated_time,
                  ...pick(item, [
                    'added_file_count',
                    'changed_file_count',
                    'deleted_file_count',
                    'infected_file_count',
                    'anti_status'
                  ])
                };
              }
            );
          });
          this.isIoDetectionReport =
            res.latestInfectionSnapshotInfo &&
            res.latestInfectionSnapshotInfo['generateType'] ===
              DataMap.snapshotGeneratetype.ioDetect.value;
          this.reportDetailInfectedCopyNum =
            res.latestInfectionSnapshotInfo?.infectedFileCount || 0;
          this.reportDetailContent = this.i18n.get(
            this.isIoDetectionReport
              ? 'explore_io_detect_infected_result_report_label'
              : 'explore_detect_infected_result_report_label',
            [
              res.latestInfectionSnapshotInfo?.snapshotDetectTime,
              res.fileSystemName,
              res.latestInfectionSnapshotInfo?.snapshotName,
              res.latestInfectionSnapshotInfo?.detectModel,
              this.capacityCalculateLabel.transform(
                res.latestInfectionSnapshotInfo?.totalFileSize || 0,
                '1.3-3',
                CAPACITY_UNIT.BYTE
              ),
              res.latestInfectionSnapshotInfo?.infectedFileCount || 0
            ]
          );
          defer(() => {
            this.initStatisticsChart(
              res.latestInfectionSnapshotInfo?.newFileCount || 0,
              res.latestInfectionSnapshotInfo?.modifyFileCount || 0,
              res.latestInfectionSnapshotInfo?.deleteFileCount || 0,
              res.latestInfectionSnapshotInfo?.infectedFileCount || 0
            );
            this.infectedFileTableData = {
              data: res.latestInfectionSnapshotInfo?.infectionFileList,
              total: size(res.latestInfectionSnapshotInfo?.infectionFileList)
            };
            this.ioInfectedFileTableData = {
              data: res.latestInfectionSnapshotInfo?.infectionFileList,
              total: size(res.latestInfectionSnapshotInfo?.infectionFileList)
            };
          });
          this.latestPoTableData = {
            data: res.protectObjectResponseList,
            total: size(res.protectObjectResponseList)
          };
          if (
            res.protectObjectResponseList &&
            res.protectObjectResponseList[0]?.software === SoftwareType.CV
          ) {
            each(this.latestPoTableConfig?.table?.columns, item => {
              if (item.key === 'detectCounts') {
                assign(item, {
                  name: this.i18n.get('explore_backup_files_label')
                });
              }
            });
            this.latestPoTable.init();
          }
        },
        () => (this.loaded = true)
      );
    }
  }
}
