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
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { Router } from '@angular/router';
import { MenuItem, MessageService } from '@iux/live';
import { JobTableComponent } from 'app/business/insight/job/job-table/job-table.component';
import {
  ApiService,
  CommonConsts,
  CopiesDetectReportService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  GROUP_COMMON,
  I18NService,
  JobAPIService,
  OperateItems,
  RoleOperationMap,
  SlaApiService,
  SYSTEM_TIME,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { CopyActionService } from 'app/shared/services/copy-action.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  defer,
  find,
  includes,
  isEmpty,
  isNumber,
  isUndefined,
  map,
  reject,
  size,
  trim
} from 'lodash';
import { combineLatest, Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-recovery-drill',
  templateUrl: './recovery-drill.component.html',
  styleUrls: ['./recovery-drill.component.less']
})
export class RecoveryDrillComponent
  implements OnInit, AfterViewInit, OnDestroy {
  optsConfig = [];
  tableConfig: TableConfig;
  tableData: TableData;
  _isEn = this.i18n.isEn;
  _isNumber = isNumber;
  _round = Math.round;
  dataMap = DataMap;
  selectionData = [];
  selectedViewType = '0';
  overviewTimeText = this.i18n.get('explore_all_time_label');
  options: MenuItem[];
  executeCount: number;
  successRate: number;
  averageTime: number;

  groupCommon = GROUP_COMMON;
  roleOperationMap = RoleOperationMap;

  timeZone = SYSTEM_TIME.timeZone;

  joibTimeText = this.i18n.get('explore_all_time_label');
  joibOptions: MenuItem[];
  abnormalResultTotal = 0;
  abnormalResult = [];
  abPageSize = 5;
  abPageIndex = CommonConsts.PAGE_START;

  drillName: string;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  total = 0;
  planData = [];

  destroy$ = new Subject();
  summrySub$: Subscription;
  abnormalJobSub$: Subscription;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('nameTpl', { static: true }) nameTpl: TemplateRef<any>;
  @ViewChild('intervalTpl', { static: true }) intervalTpl: TemplateRef<any>;
  @ViewChild('recentJobStartTimeTpl', { static: true })
  recentJobStartTimeTpl: TemplateRef<any>;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private exerciseService: ApiService,
    private jobApiService: JobAPIService,
    private dataMapService: DataMapService,
    public appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private copyActionService: CopyActionService,
    private systemTimeService: SystemTimeService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private copiesDetectReportService: CopiesDetectReportService,
    private messageService: MessageService,
    public slaApiService: SlaApiService
  ) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngAfterViewInit(): void {
    if (!!this.appUtilsService.getCacheValue('jobToExercise', false)) {
      this.tableConfig.table.columns[0].hidden = false;
      this.dataTable.reinit();
    }
    this.dataTable?.fetchData();
    this.getCardDrillData();
  }

  ngOnInit(): void {
    this.virtualScroll.getScrollParam(570);
    this.initConfig();
    this.getSummary();
    this.getRecentAbnormalJobs();
  }

  activeDrill(data) {
    if (size(data) === 1) {
      this.exerciseService
        .activeExercise({ exerciseId: data[0].uuid })
        .subscribe(() => this.dataTable?.fetchData());
    } else {
      this.batchOperateService.selfGetResults(
        item => {
          return this.exerciseService.activeExercise({
            exerciseId: item.uuid,
            akDoException: false,
            akOperationTips: false,
            akLoading: false
          });
        },
        [...this.selectionData],
        () => {
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.dataTable.fetchData();
        }
      );
    }
  }

  disableDrill(data) {
    if (size(data) === 1) {
      this.exerciseService
        .deactiveExercise({ exerciseId: data[0].uuid })
        .subscribe(() => this.dataTable?.fetchData());
    } else {
      this.batchOperateService.selfGetResults(
        item => {
          return this.exerciseService.deactiveExercise({
            exerciseId: item.uuid,
            akDoException: false,
            akOperationTips: false,
            akLoading: false
          });
        },
        [...this.selectionData],
        () => {
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.dataTable.fetchData();
        }
      );
    }
  }

  initConfig() {
    this.options = [
      {
        id: 'all',
        label: this.i18n.get('explore_all_time_label'),
        onClick: () => {
          this.overviewTimeText = this.i18n.get('explore_all_time_label');
          this.getSummary();
        }
      },
      {
        id: 'day',
        label: this.i18n.get('common_last_24_hour_label'),
        onClick: () => {
          this.overviewTimeText = this.i18n.get('common_last_24_hour_label');
          this.getSummary('1d');
        }
      },
      {
        id: 'week',
        label: this.i18n.get('common_last_week_label'),
        onClick: () => {
          this.overviewTimeText = this.i18n.get('common_last_week_label');
          this.getSummary('1w');
        }
      },
      {
        id: 'month',
        label: this.i18n.get('common_last_month_label'),
        onClick: () => {
          this.overviewTimeText = this.i18n.get('common_last_month_label');
          this.getSummary('1m');
        }
      },
      {
        id: 'year',
        label: this.i18n.get('explore_half_year_label'),
        onClick: () => {
          this.overviewTimeText = this.i18n.get('explore_half_year_label');
          this.getSummary('0.5y');
        }
      }
    ];

    this.joibOptions = [
      {
        id: 'all',
        label: this.i18n.get('explore_all_time_label'),
        onClick: () => {
          this.joibTimeText = this.i18n.get('explore_all_time_label');
          this.getRecentAbnormalJobs();
        }
      },
      {
        id: 'day',
        label: this.i18n.get('common_last_24_hour_label'),
        onClick: () => {
          this.joibTimeText = this.i18n.get('common_last_24_hour_label');
          this.getRecentAbnormalJobs('1d');
        }
      },
      {
        id: 'week',
        label: this.i18n.get('common_last_week_label'),
        onClick: () => {
          this.joibTimeText = this.i18n.get('common_last_week_label');
          this.getRecentAbnormalJobs('1w');
        }
      },
      {
        id: 'month',
        label: this.i18n.get('common_last_month_label'),
        onClick: () => {
          this.joibTimeText = this.i18n.get('common_last_month_label');
          this.getRecentAbnormalJobs('1m');
        }
      },
      {
        id: 'year',
        label: this.i18n.get('explore_half_year_label'),
        onClick: () => {
          this.joibTimeText = this.i18n.get('explore_half_year_label');
          this.getRecentAbnormalJobs('0.5y');
        }
      }
    ];

    const opts: { [key: string]: ProButton } = {
      create: {
        id: 'create',
        type: 'primary',
        permission: RoleOperationMap.restoreExercise,
        label: this.i18n.get('explore_create_drill_plan_label'),
        onClick: () => this.create()
      },
      modify: {
        id: 'modify',
        permission: RoleOperationMap.restoreExercise,
        label: this.i18n.get('common_modify_label'),
        disableCheck: data => {
          return (
            isEmpty(data) ||
            !isEmpty(
              find(data, item =>
                includes([DataMap.drillStatus.running.value], item.status)
              )
            ) ||
            !isEmpty(
              find(
                data,
                item =>
                  includes([DataMap.drillStatus.finished.value], item.status) &&
                  includes([DataMap.drillType.single.value], item.type)
              )
            )
          );
        },
        onClick: ([data]) => {
          this.router.navigateByUrl(`/explore/modify-drill/${data.uuid}`);
        }
      },
      active: {
        id: 'active',
        permission: RoleOperationMap.restoreExercise,
        label: this.i18n.get('common_active_label'),
        disableCheck: data => {
          return (
            isEmpty(data) ||
            !isEmpty(
              find(
                data,
                item =>
                  includes(
                    [
                      DataMap.drillStatus.finished.value,
                      DataMap.drillStatus.waiting.value,
                      DataMap.drillStatus.running.value
                    ],
                    item.status
                  ) || item.type === DataMap.drillType.single.value
              )
            )
          );
        },
        onClick: data => this.activeDrill(data)
      },
      disable: {
        id: 'disable',
        permission: RoleOperationMap.restoreExercise,
        label: this.i18n.get('common_disable_label'),
        disableCheck: data => {
          return (
            isEmpty(data) ||
            !isEmpty(
              find(
                data,
                item =>
                  includes(
                    [
                      DataMap.drillStatus.disabled.value,
                      DataMap.drillStatus.running.value
                    ],
                    item.status
                  ) || item.type === DataMap.drillType.single.value
              )
            )
          );
        },
        onClick: data => {
          this.disableDrill(data);
        }
      },
      delete: {
        id: 'delete',
        permission: RoleOperationMap.restoreExercise,
        label: this.i18n.get('common_delete_label'),
        disableCheck: data => {
          return (
            isEmpty(data) ||
            !isEmpty(
              find(data, item =>
                includes([DataMap.drillStatus.running.value], item.status)
              )
            )
          );
        },
        onClick: data => this.deletePlan(data)
      }
    };
    this.optsConfig = getPermissionMenuItem([
      opts.create,
      opts.active,
      opts.disable,
      opts.delete
    ]);

    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('explore_drill_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        cellRender: this.nameTpl,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'type',
        name: this.i18n.get('explore_drill_plan_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('drillType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('drillType')
        }
      },
      {
        key: 'status',
        name: this.i18n.get('protection_running_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('drillStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('drillStatus')
        }
      },
      {
        key: 'interval',
        name: this.i18n.get('explore_drill_period_label'),
        cellRender: this.intervalTpl
      },
      {
        key: 'nextExecuteTime',
        name: this.i18n.get('explore_next_drill_time_label'),
        cellRender: this.recentJobStartTimeTpl
      },
      {
        key: 'recentJobStatus',
        name: this.i18n.get('explore_recent_result_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Job_status')
        },
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('Job_status')
            .filter(item =>
              includes(
                [
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
        key: 'operation',
        width: 144,
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: getPermissionMenuItem(
              reject(opts, item => includes(['create'], item.id))
            )
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'uuid',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filters: Filters, args) => this.getDrillData(filters, args),
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true
      }
    };
  }

  getSummary(summaryLimitTime?: string) {
    const params = {};
    if (summaryLimitTime) {
      assign(params, {
        summaryLimitTime
      });
    }
    if (this.summrySub$) {
      this.summrySub$.unsubscribe();
    }
    this.summrySub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.exerciseService.exerciseSummary({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.executeCount = res.executeCount;
        this.successRate = res.successRate;
        this.averageTime = res.averageTime;
      });
  }

  getRecentAbnormalJobs(fromStartTime?) {
    const params = {
      pageNo: this.abPageIndex + 1,
      pageSize: this.abPageSize,
      limitTime: fromStartTime
    };
    if (this.abnormalJobSub$) {
      this.abnormalJobSub$.unsubscribe();
    }
    this.abnormalJobSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return combineLatest([
            this.systemTimeService.getSystemTime(false),
            this.exerciseService.exerciseFail({
              ...params,
              akLoading: !index
            })
          ]);
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.abnormalResult = res[1].records || [];
        this.abnormalResultTotal = res[1].totalCount || 0;
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

  abPageChange(page) {
    this.abPageIndex = page.pageIndex;
    this.abPageSize = page.pageSize;
    this.getRecentAbnormalJobs();
  }

  gotoReport() {
    this.router.navigateByUrl('/insight/reports');
  }

  searchPlan() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(this.drillName)
        }
      ]
    });
    this.dataTable.fetchData();
    this.getCardDrillData();
  }

  refresh() {
    if (this.selectedViewType === '0') {
      defer(() => this.dataTable?.fetchData());
    } else {
      this.getCardDrillData();
    }
  }

  getDrillData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions_v2)) {
      assign(params, {
        conditions: filters.conditions_v2.replace(
          'recentJobStatus',
          'latest_job_status'
        )
      });
    }

    this.exerciseService.queryExercise(params).subscribe(res => {
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      if (!!this.appUtilsService.getCacheValue('jobToExercise', false)) {
        this.dataTable.filterChange({
          caseSensitive: false,
          filterMode: 'contains',
          key: 'uuid',
          value: this.appUtilsService.getCacheValue('jobToExercise')
        });
      }
    });
  }

  getCardDrillData() {
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize
    };
    if (trim(this.drillName)) {
      assign(params, {
        conditions: JSON.stringify({ name: [['~~'], trim(this.drillName)] })
      });
    }
    this.exerciseService.queryExercise(params).subscribe(res => {
      this.planData = res.records;
      this.total = res.totalCount;
    });
  }

  getDetail(item) {
    this.router.navigateByUrl(`/explore/drill-detail/${item.uuid}`);
  }

  pageChange(page) {
    this.pageIndex = page.pageIndex;
    this.pageSize = page.pageSize;
    this.getCardDrillData();
  }

  deletePlan(datas: any[]) {
    if (datas.length > 1) {
      this.warningMessageService.create({
        content: this.i18n.get('explore_drill_delete_warn_label', [
          map(datas, 'name').join(this._isEn ? ',' : '，')
        ]),
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item => {
              return this.exerciseService.deleteExercise({
                exerciseId: item.uuid,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            [...this.selectionData],
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            }
          );
        }
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get('explore_drill_delete_warn_label', [
          datas[0].name
        ]),
        onOK: () => {
          this.exerciseService
            .deleteExercise({
              exerciseId: datas[0].uuid
            })
            .subscribe(() => {
              this.selectionData = [];
              this.dataTable.setSelections(this.selectionData);
              this.dataTable.fetchData();
            });
        }
      });
    }
  }

  create() {
    this.router.navigateByUrl('/explore/create-drill');
  }
}
