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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import {
  ApiService,
  DataMap,
  DataMapService,
  I18NService,
  JobAPIService,
  LANGUAGE,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService,
  getPermissionMenuItem
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  first,
  includes,
  isEmpty,
  isNumber,
  now,
  reduce,
  size,
  uniq
} from 'lodash';
import { ScriptLogComponent } from './script-log/script-log.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { MessageService } from '@iux/live';
import { finalize } from 'rxjs/operators';

@Component({
  selector: 'aui-drill-detail',
  templateUrl: './drill-detail.component.html',
  styleUrls: ['./drill-detail.component.less']
})
export class DrillDetailComponent implements OnInit {
  drillPlan;
  _isEn = this.i18n.isEn;
  _isNumber = isNumber;
  _round = Math.round;
  dataMap = DataMap;
  basicInfoTableConfig: TableConfig;
  resultTableConfig: TableConfig;
  basicInfoTableData: TableData;
  resultTableData: TableData;

  @ViewChild('resultTable', { static: false }) resultTable: ProTableComponent;
  @ViewChild('startTimeTpl', { static: true }) startTimeTpl: TemplateRef<any>;
  @ViewChild('endTimeTpl', { static: true }) endTimeTpl: TemplateRef<any>;
  @ViewChild('resourceType', { static: true }) resourceType: TemplateRef<any>;
  @ViewChild('drillTypeTpl', { static: true }) drillTypeTpl: TemplateRef<any>;
  @ViewChild('destroyTpl', { static: true }) destroyTpl: TemplateRef<any>;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private route: ActivatedRoute,
    private exerciseService: ApiService,
    private jobApiService: JobAPIService,
    private dataMapService: DataMapService,
    private messageService: MessageService,
    public appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    this.initTableConfig();
    this.getPlanDetail();
    this.getDrillResource();
  }

  initTableConfig() {
    this.basicInfoTableConfig = {
      table: {
        compareWith: 'uuid',
        async: false,
        columns: [
          {
            key: 'resourceName',
            name: this.i18n.get('common_name_label')
          },
          {
            key: 'resourceType',
            name: this.i18n.get('common_resource_type_label'),
            cellRender: this.resourceType
          },
          {
            key: 'type',
            name: this.i18n.get('explore_drill_type_label'),
            cellRender: this.drillTypeTpl
          },
          {
            key: 'shouldDestroy',
            name: this.i18n.get('explore_destroy_drill_label'),
            cellRender: this.destroyTpl
          },
          {
            key: 'location',
            name: this.i18n.get('common_location_label')
          },
          {
            key: 'targetLocation',
            name: this.i18n.get('explore_target_location_label')
          },
          {
            key: 'operation',
            width: 130,
            hidden: 'ignoring',
            name: this.i18n.get('common_operation_label'),
            cellRender: {
              type: 'operation',
              config: {
                maxDisplayItems: 2,
                items: getPermissionMenuItem([
                  {
                    id: 'logDetail',
                    permission: OperateItems.CreateDrillPlan,
                    label: this.i18n.get('explore_drill_script_log_label'),
                    onClick: ([data]) => this.viewScriptLog(data)
                  }
                ])
              }
            }
          }
        ],
        scrollFixed: true,
        colDisplayControl: false
      },
      pagination: {
        winTablePagination: true
      }
    };
    this.resultTableConfig = {
      table: {
        compareWith: 'jobId',
        columns: [
          {
            key: 'startTime',
            name: this.i18n.get('explore_drill_start_time_label'),
            cellRender: this.startTimeTpl,
            filter: {
              type: 'date',
              showTime: true,
              format: 'yyyy-MM-dd HH:mm:ss'
            }
          },
          {
            key: 'exerciseId',
            name: this.i18n.get('explore_drill_id_label')
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
            key: 'endTime',
            name: this.i18n.get('explore_drill_end_time_label'),
            cellRender: this.endTimeTpl
          },
          {
            key: 'operation',
            width: 130,
            hidden: 'ignoring',
            name: this.i18n.get('common_operation_label'),
            cellRender: {
              type: 'operation',
              config: {
                maxDisplayItems: 2,
                items: [
                  {
                    id: 'export',
                    label: this.i18n.get('common_download_label'),
                    onClick: data => this.exportJob(first(data))
                  }
                ]
              }
            }
          }
        ],
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filters: Filters) => this.getDrillResult(filters),
        trackByFn: (_, item) => {
          return item.jobId;
        }
      },
      pagination: {
        winTablePagination: true
      }
    };
  }

  optsCallback = data => {
    if (!data) {
      return [];
    }
    return getPermissionMenuItem([
      {
        id: 'modify',
        permission: OperateItems.CreateDrillPlan,
        disabled:
          includes([DataMap.drillStatus.running.value], data.status) ||
          (includes([DataMap.drillStatus.finished.value], data.status) &&
            includes([DataMap.drillType.single.value], data.type)),
        label: this.i18n.get('common_modify_label'),
        onClick: () =>
          this.router.navigateByUrl(`/explore/modify-drill/${data.uuid}`)
      },
      {
        id: 'active',
        permission: OperateItems.CreateDrillPlan,
        disabled:
          includes(
            [
              DataMap.drillStatus.finished.value,
              DataMap.drillStatus.waiting.value,
              DataMap.drillStatus.running.value
            ],
            data.status
          ) || data.type === DataMap.drillType.single.value,
        label: this.i18n.get('common_active_label'),
        onClick: () => {
          this.exerciseService
            .activeExercise({ exerciseId: data.uuid })
            .subscribe(() => this.getPlanDetail());
        }
      },
      {
        id: 'disable',
        permission: OperateItems.CreateDrillPlan,
        disabled:
          includes(
            [
              DataMap.drillStatus.disabled.value,
              DataMap.drillStatus.running.value
            ],
            data.status
          ) || data.type === DataMap.drillType.single.value,
        label: this.i18n.get('common_disable_label'),
        onClick: () => {
          this.exerciseService
            .deactiveExercise({ exerciseId: data.uuid })
            .subscribe(() => this.getPlanDetail());
        }
      },
      {
        id: 'delete',
        permission: OperateItems.CreateDrillPlan,
        disabled: includes([DataMap.drillStatus.running.value], data.status),
        label: this.i18n.get('common_delete_label'),
        onClick: () => {
          this.warningMessageService.create({
            content: this.i18n.get('explore_drill_delete_warn_label', [
              data.name
            ]),
            onOK: () => {
              this.exerciseService
                .deleteExercise({
                  exerciseId: data.uuid
                })
                .subscribe(() =>
                  this.router.navigateByUrl(`/explore/recovery-drill`)
                );
            }
          });
        }
      }
    ]);
  };

  exportJob(data?) {
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
      types: [DataMap.Job_type.exercise.value],
      exerciseId: this.route?.snapshot?.params?.uuid
    };
    const filters: any = this.getParams(this.resultTable.getFilterMap());
    if (data) {
      assign(params, {
        jobId: data.jobId
      });
    }
    if (!isEmpty(filters.statusList)) {
      assign(params, {
        statusList: filters.statusList
      });
    }
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

  viewScriptLog(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'view-script-log-modal',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('explore_drill_script_log_label'),
        lvContent: ScriptLogComponent,
        lvComponentParams: { item },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  gotoRecoveryDrill() {
    this.router.navigateByUrl('/explore/recovery-drill');
  }

  getPlanDetail() {
    this.exerciseService
      .queryExercise({
        pageNo: 0,
        pageSize: 1,
        conditions: JSON.stringify({
          uuid: this.route?.snapshot?.params?.uuid
        })
      })
      .subscribe(res => {
        this.drillPlan = first(res.records);
      });
  }

  getDrillResource() {
    this.exerciseService
      .queryExerciseResourceDetail({
        exerciseId: this.route?.snapshot?.params?.uuid
      })
      .subscribe((res: any) => {
        this.basicInfoTableData = {
          data: res,
          total: size(res)
        };
      });
  }

  getResourceType(resource): string {
    return this.appUtilsService.getResourceType(resource.resourceType);
  }

  getParams(filters: Filters) {
    const params = {};
    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (!isEmpty(conditions.status)) {
        assign(params, {
          statusList: uniq(
            reduce(
              conditions.status,
              (array, item) => {
                array.push(item);
                if (item === DataMap.Job_status.running.value) {
                  array.push(
                    DataMap.Job_status.initialization.value,
                    DataMap.Job_status.aborting.value
                  );
                }
                if (item === DataMap.Job_status.aborted.value) {
                  array.push(DataMap.Job_status.cancelled.value);
                }
                if (item === DataMap.Job_status.failed.value) {
                  array.push(
                    DataMap.Job_status.abnormal.value,
                    DataMap.Job_status.abort_failed.value,
                    DataMap.Job_status.dispatch_failed.value,
                    DataMap.Job_status.dispatch_failed.value
                  );
                }
                if (item === DataMap.Job_status.dispatching.value) {
                  array.push(DataMap.Job_status.redispatch.value);
                }
                return array;
              },
              []
            )
          )
        });
      }
      if (!isEmpty(conditions.startTime)) {
        assign(params, {
          fromStartTime: new Date(conditions.startTime[0]).getTime(),
          toStartTime: new Date(conditions.startTime[1]).getTime()
        });
      }
    }
    if (filters.sort?.key) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }
    return params;
  }

  getDrillResult(filters: Filters) {
    const params = {
      startPage: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      types: [DataMap.Job_type.exercise.value],
      isSystem: false,
      isVisible: true,
      orderType: 'desc',
      orderBy: 'startTime',
      exerciseId: this.route?.snapshot?.params?.uuid,
      ...this.getParams(filters)
    };
    this.jobApiService.queryJobsUsingGET(params).subscribe(res => {
      this.resultTableData = {
        data: res.records,
        total: res.totalCount
      };
    });
  }

  indexChange(id) {
    if (id === 'result') {
      this.resultTable?.fetchData();
    }
  }

  executeDetail(item) {
    this.router.navigateByUrl(`/explore/drill-execute-log/${item.jobId}`);
  }
}
