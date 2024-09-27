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
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { MessageService } from '@iux/live';
import { JobTableComponent } from 'app/business/insight/job/job-table/job-table.component';
import {
  CopiesDetectReportService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  JobAPIService,
  LANGUAGE,
  MODAL_COMMON,
  OperateItems,
  SlaApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { CopyActionService } from 'app/shared/services/copy-action.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import { assign, first, includes, isEmpty, isNumber, map, now } from 'lodash';
import { combineLatest } from 'rxjs';
import { finalize } from 'rxjs/operators';
import { ScriptLogComponent } from '../script-log/script-log.component';

@Component({
  selector: 'aui-drill-execute-log',
  templateUrl: './drill-execute-log.component.html',
  styleUrls: ['./drill-execute-log.component.less']
})
export class DrillExecuteLogComponent implements OnInit, AfterViewInit {
  dataTableConfig: TableConfig;
  dataTableData: TableData;
  exerciseId;
  exerciseJob;
  _isNumber = isNumber;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('startTimeTpl', { static: true }) startTimeTpl: TemplateRef<any>;
  @ViewChild('resourceType', { static: true }) resourceType: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private route: ActivatedRoute,
    private jobApiService: JobAPIService,
    private dataMapService: DataMapService,
    public appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private copyActionService: CopyActionService,
    private systemTimeService: SystemTimeService,
    private copiesDetectReportService: CopiesDetectReportService,
    private messageService: MessageService,
    public slaApiService: SlaApiService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable?.fetchData();
  }

  ngOnInit(): void {
    this.initTableConfig();
  }

  initTableConfig() {
    this.dataTableConfig = {
      table: {
        compareWith: 'uuid',
        columns: [
          {
            key: 'sourceName',
            name: this.i18n.get('common_name_label')
          },
          {
            key: 'type',
            name: this.i18n.get('insight_job_type_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('Job_type')
            },
            filter: {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              showSearch: true,
              options: this.dataMapService
                .toArray('Job_type')
                .filter(item =>
                  includes(
                    [
                      DataMap.Job_type.restore_job.value,
                      DataMap.Job_type.live_mount_job.value,
                      DataMap.Job_type.unmout.value
                    ],
                    item.value
                  )
                )
            }
          },
          {
            key: 'sourceSubType',
            name: this.i18n.get('common_resource_type_label'),
            cellRender: this.resourceType
          },
          {
            key: 'status',
            name: this.i18n.get('explore_drill_result_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('Job_status')
            },
            filter: {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              showSearch: true,
              options: this.dataMapService
                .toArray('Job_status')
                .filter(item =>
                  includes(
                    [
                      DataMap.Job_status.running.value,
                      DataMap.Job_status.success.value,
                      DataMap.Job_status.partial_success.value,
                      DataMap.Job_status.failed.value
                    ],
                    item.value
                  )
                )
            }
          },
          {
            key: 'durationTime',
            name: this.i18n.get('explore_drill_recovery_time_label')
          },
          {
            key: 'startTime',
            name: this.i18n.get('explore_drill_start_time_label'),
            cellRender: this.startTimeTpl,
            sort: true
          },
          {
            key: 'operation',
            width: 130,
            hidden: 'ignoring',
            name: this.i18n.get('common_operation_label'),
            cellRender: {
              type: 'operation',
              config: {
                maxDisplayItems: 1,
                items: getPermissionMenuItem([
                  {
                    id: 'taskDetail',
                    permission: OperateItems.CreateDrillPlan,
                    label: this.i18n.get('insight_task_details_label'),
                    onClick: ([data]) => this.getJobDetail(data)
                  },
                  {
                    id: 'scriptLog',
                    permission: OperateItems.CreateDrillPlan,
                    label: this.i18n.get('explore_drill_script_log_view_label'),
                    onClick: ([data]) => this.viewScriptLog(data)
                  }
                ])
              }
            }
          }
        ],
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filters: Filters) => this.getSubTask(filters),
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true
      }
    };
  }

  getResourceType(resource): string {
    return this.appUtilsService.getResourceType(resource.sourceSubType);
  }

  exportJob() {
    this.messageService.info(
      this.i18n.get('common_file_download_processing_label'),
      {
        lvDuration: 0,
        lvShowCloseButton: true,
        lvMessageKey: 'jobDownloadKey'
      }
    );
    const params = {
      lang: this.i18n.language === LANGUAGE.CN ? 'zh_CN' : 'en',
      isSystem: false,
      isVisible: true,
      jobId: this.route?.snapshot?.params?.uuid
    };
    this.jobApiService
      .exportUsingPOST({ ...params, akLoading: false })
      .pipe(
        finalize(() => {
          this.messageService.destroy('jobDownloadKey');
        })
      )
      .subscribe(blob => {
        const bf = new Blob([blob], {
          type: 'text/csv'
        });
        this.appUtilsService.downloadFile(`job_${now()}.csv`, bf);
      });
  }

  getSubTask(filters) {
    const params = {
      startPage: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      isSystem: false,
      isVisible: true,
      orderType: 'desc',
      orderBy: 'startTime',
      exerciseJobId: this.route?.snapshot?.params?.uuid
    };
    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.type) {
        assign(params, {
          types: conditions.type
        });
      }
      if (conditions.status) {
        assign(params, {
          statusList: conditions.status
        });
      }
    }
    if (filters.sort?.key) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }
    combineLatest([
      this.systemTimeService.getSystemTime(false),
      this.jobApiService.queryJobsUsingGET(params),
      this.jobApiService.queryJobsUsingGET({
        isSystem: false,
        isVisible: true,
        jobId: this.route?.snapshot?.params?.uuid
      })
    ]).subscribe(res => {
      const sysTime = new Date(
        `${res[0].time.replace(/-/g, '/')} ${res[0].displayName}`
      ).getTime();
      this.dataTableData = {
        data: map(res[1].records, item => {
          return assign(item, {
            durationTime: this.appUtilsService.getDuration(
              includes(
                [
                  DataMap.Job_status.running.value,
                  DataMap.Job_status.initialization.value,
                  DataMap.Job_status.pending.value,
                  DataMap.Job_status.aborting.value
                ],
                item.status
              )
                ? sysTime - item.startTime < 0
                  ? 0
                  : sysTime - item.startTime
                : item.endTime
                ? item.endTime - item.startTime
                : 0
            )
          });
        }),
        total: res[1].totalCount
      };
      this.exerciseId = first(res[1].records)
        ? first(res[1].records)['exerciseId']
        : first(res[2].records)
        ? first(res[2].records)['exerciseId']
        : '';
      this.exerciseJob = first(res[2].records);
    });
  }

  getJobDetail(job) {
    const jobTable = new JobTableComponent(
      this.appUtilsService,
      this.i18n,
      this.messageService,
      this.drawModalService,
      this.dataMapService,
      this.copyActionService,
      this.copiesDetectReportService,
      this.slaApiService
    );
    jobTable.getDetail(job);
  }

  viewScriptLog(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'view-script-execute-log-modal',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('explore_drill_script_log_view_label'),
        lvContent: ScriptLogComponent,
        lvComponentParams: { item, isExecuteDetail: true },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  checkInProgress(job): boolean {
    if (!job) {
      return true;
    }
    return includes(
      [
        DataMap.Job_status.running.value,
        DataMap.Job_status.initialization.value,
        DataMap.Job_status.pending.value,
        DataMap.Job_status.aborting.value,
        DataMap.Job_status.dispatching.value,
        DataMap.Job_status.redispatch.value
      ],
      job.status
    );
  }

  gotoDetail() {
    window.history.back();
  }
}
