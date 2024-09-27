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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  ScheduleReportService,
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
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  includes,
  intersection,
  isEmpty,
  reject,
  set,
  size,
  trim,
  values
} from 'lodash';
import { finalize, map } from 'rxjs/operators';
import { CreateSubscriptionComponent } from './create-subscription/create-subscription.component';

@Component({
  selector: 'aui-reports-subscription',
  templateUrl: './reports-subscription.component.html',
  styleUrls: ['./reports-subscription.component.less'],
  providers: [DatePipe],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ReportsSubscriptionComponent implements OnInit, AfterViewInit {
  name;
  formGroup;
  tableConfig: TableConfig;
  tableData: TableData;
  intervalUnit = DataMap.reportGeneratedIntervalUnit;
  daysOfWeek = this.dataMapService
    .toArray('dayOfWeek')
    .filter(item => (item.isLeaf = true));
  optsConfig;
  selectionData = [];
  optItems = [];
  dataMap = DataMap;
  showScopeAndFrequencyList = [
    DataMap.Report_Type.backupJob.value,
    DataMap.Report_Type.recoveryJob.value,
    DataMap.Report_Type.recoveryDrillJob.value
  ];

  @Input() activeIndex;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('typeTpl', { static: true })
  typeTpl: TemplateRef<any>;
  @ViewChild('fileFormatTpl', { static: true })
  fileFormatTpl: TemplateRef<any>;
  @ViewChild('timeUnitTpl', { static: true })
  timeUnitTpl: TemplateRef<any>;
  @ViewChild('scopeTpl', { static: true })
  scopeTpl: TemplateRef<any>;
  @ViewChild('preExecTimeTpl', { static: true })
  preExecTimeTpl: TemplateRef<any>;
  @ViewChild('nextExecTimeTpl', { static: true })
  nextExecTimeTpl: TemplateRef<any>;
  @ViewChild('emailsTpl', { static: true })
  emailsTpl: TemplateRef<any>;
  @ViewChild('generatedPlanTpl', { static: true })
  generatedPlanTpl: TemplateRef<any>;
  constructor(
    public i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private datePipe: DatePipe,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private scheduleReportService: ScheduleReportService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  onChange() {
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      create: {
        id: 'create',
        type: 'primary',
        label: this.i18n.get('common_create_label'),
        onClick: () => this.create()
      },
      modify: {
        id: 'modify',
        type: 'primary',
        label: this.i18n.get('common_modify_label'),
        onClick: ([data]) => this.create(data)
      },
      delete: {
        id: 'delete',
        disableCheck: data => {
          return !size(data) || size(data) > 100;
        },
        disabledTips: this.i18n.get('insight_report_delete_tips_label'),
        label: this.i18n.get('common_delete_label'),
        onClick: data => this.deleteRes(data)
      },
      executeTask: {
        id: 'execute',
        // permission: OperateItems.Protection,
        label: this.i18n.get('insight_execute_now_label'),
        disableCheck: data => {
          return size(data) !== 1;
        },
        onClick: ([data]) =>
          this.executeTassk(data, 'RunOnceReportTaskUsingGet')
      }
    };
    this.optItems = cloneDeep(
      getPermissionMenuItem(
        values(reject(opts, item => includes(['create', 'execute'], item.id)))
      )
    );
    each(this.optItems, item => {
      if (item.disabledTips) {
        item.disabledTips = '';
      }
    });

    const cols: TableCols[] = [
      {
        key: 'policyId',
        name: this.i18n.get('ID'),
        hidden: true
      },
      {
        key: 'policyName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'type',
        name: this.i18n.get('insight_report_type_label'),
        cellRender: this.typeTpl
      },
      {
        key: 'externalClusterName',
        name: this.i18n.get('common_data_source_label'),
        width: 400
      },
      {
        key: 'timeRange',
        name: this.i18n.get('insight_report_scope_label'),
        cellRender: this.scopeTpl
      },
      {
        key: 'timeUnit',
        hidden: true,
        name: this.i18n.get('insight_report_frequency_label'),
        cellRender: this.timeUnitTpl
      },
      {
        key: 'fileFormat',
        hidden: true,
        name: this.i18n.get('insight_report_format_label'),
        cellRender: this.fileFormatTpl
      },
      {
        key: 'email',
        name: this.i18n.get('system_recipient_email_label'),
        cellRender: this.emailsTpl
      },
      {
        key: 'generatedPlan',
        name: this.i18n.get('insight_generate_paln_label'),
        cellRender: this.generatedPlanTpl
      },
      {
        key: 'preExecTime',
        name: this.i18n.get('insight_report_pre_task_time_label'),
        cellRender: this.preExecTimeTpl
      },
      {
        key: 'nextExecTime',
        name: this.i18n.get('insight_report_next_task_time_label'),
        cellRender: this.nextExecTimeTpl
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
            items: this.optItems
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'policyId',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };

    this.optsConfig = getPermissionMenuItem([
      opts.create,
      opts.executeTask,
      opts.delete
    ]);
  }

  search() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'policyName',
          value: trim(this.name)
        }
      ]
    });

    set(
      this.dataTable.filterMap,
      'paginator.pageIndex',
      CommonConsts.PAGE_START
    );
    this.dataTable.fetchData();
  }

  formatTaskType(data) {
    const type = data.type.map(item =>
      this.dataMapService.getLabel('Report_Type', item)
    );
    return type.join(',');
  }

  showScopeAndFrequency(data) {
    return intersection(this.showScopeAndFrequencyList, data.type).length > 0;
  }

  formatGeneratedPlan(data) {
    let plan = this.i18n.get('protection_every_label');
    switch (data.intervalUnit) {
      case DataMap.reportGeneratedIntervalUnit.day.value:
        plan += data.execInterval + this.i18n.get('common_day_label');
        break;
      case DataMap.reportGeneratedIntervalUnit.week.value:
        plan += data.daysOfWeek
          .map(item => this.dataMapService.getLabel('dayOfWeek', String(item)))
          .join(' ');
        break;
      case DataMap.reportGeneratedIntervalUnit.month.value:
        plan +=
          this.i18n.get('common_month_lower_label') +
          data.daysOfMonth.join(',');
        break;
    }
    plan += ' ' + this.datePipe.transform(data.execTime, 'HH:mm:ss');
    return plan;
  }

  create(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'create-report',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: data
          ? this.i18n.get('common_modify_label')
          : this.i18n.get('common_create_label'),
        lvContent: CreateSubscriptionComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateSubscriptionComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(formGroupStatus => {
            modalIns.lvOkDisabled = formGroupStatus === 'INVALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateSubscriptionComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable.fetchData();
              },
              error: () => resolve(false)
            });
          });
        },
        lvCancel: modal => {}
      })
    );
  }

  deleteRes(data) {
    if (size(data) > 1) {
      this.batchDeleteReport(this.selectionData);
    } else {
      this.batchDeleteReport(data);
    }
  }

  private batchDeleteReport(data) {
    this.warningMessageService.create({
      content: this.i18n.get('insight_report_delete_label'),
      onOK: () => {
        this.batchOperateService.selfGetResults(
          item => {
            return this.scheduleReportService.DeleteReportSubscriptionPolicyUsingDelete(
              {
                policyId: item.policyId,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              }
            );
          },
          cloneDeep(data),
          () => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          }
        );
      }
    });
  }

  executeTassk(data, action: string) {
    this.scheduleReportService
      .RunReportSubscriptionPolicyUsingGost({
        policyId: data?.policyId
      })
      .pipe(
        finalize(() => {
          this.dataTable.fetchData();
        })
      )
      .subscribe();
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex + 1 || CommonConsts.PAGE_START + 1,
      pageSize: filters.paginator.pageSize || CommonConsts.PAGE_SIZE,
      akLoading: !args?.isAutoPolling ?? true
    };
    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.policyName) {
        assign(params, {
          name: conditionsTemp.policyName[1]
        });
      }
    }

    this.scheduleReportService
      .ListReportSubscriptionPoliciesUsingPost(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            assign(item, {
              ...item.reportParam,
              emailsDisplay: item.reportParam.emails.join(','),
              generatedPlan: this.formatGeneratedPlan(item)
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          total: res.total,
          data: res.records
        };
        this.cdr.detectChanges();
      });
  }
}
