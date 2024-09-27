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
  EventEmitter,
  Input,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import { GroupJobDetailComponent } from 'app/business/protection/virtualization/virtualization-group/group-job-detail/group-job-detail.component';
import {
  AnonyControllerService,
  ApplicationType,
  CommonConsts,
  CookieService,
  CopiesDetectReportService,
  DataMap,
  DataMapService,
  filterJobType,
  getPermissionMenuItem,
  getTableOptsItems,
  I18NService,
  JobAPIService,
  JobColorConsts,
  JOB_ORIGIN_TYPE,
  LANGUAGE,
  MODAL_COMMON,
  OperateItems,
  RestoreType,
  RestoreV2Type,
  SlaApiService,
  SLA_BACKUP_NAME,
  Table_Size,
  WarningMessageService,
  SupportLicense
} from 'app/shared';
import { JobBo } from 'app/shared/api/models';
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
  cloneDeep,
  concat,
  each,
  eq,
  filter,
  find,
  first,
  get,
  includes,
  isArray,
  isDate,
  isEmpty,
  isNil,
  isUndefined,
  map,
  now,
  omit,
  reduce,
  reject,
  remove,
  set,
  size,
  uniq
} from 'lodash';
import { combineLatest } from 'rxjs';
import { finalize } from 'rxjs/operators';
import { JobDetailComponent } from './job-detail/job-detail.component';
import { ModifyRemarksComponent } from './modify-remarks/modify-remarks.component';
import { ModifyHandleComponent } from './modify-handle/modify-handle.component';
import { BatchRetryComponent } from './batch-retry/batch-retry.component';

@Component({
  selector: 'aui-job-table',
  templateUrl: './job-table.component.html',
  styleUrls: ['./job-table.component.less'],
  providers: [DatePipe],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class JobTableComponent implements OnInit, AfterViewInit {
  dataMap = DataMap;
  showExpiredCopy = true;
  fromStartTime;
  toStartTime;
  optsConfig;
  groupOpts;
  sysTime;
  timeOffset;
  timeZone = 'UTC+08:00';
  tableConfig: TableConfig;
  tableData: TableData;
  rangeDate = [];
  selectionData = [];
  statusArr = [];
  slaNameOps = [];
  slaNameData = [];
  colors = [[0, JobColorConsts.SUCCESSFUL]];
  abortingColors = [[0, JobColorConsts.ABORTED]];
  isHandled = false;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;

  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  @Input() activeIndex;
  @Input() statusList;
  @Input() selectedResource;
  @Input() subType;
  // 发射selections数组
  @Output() selectionChange = new EventEmitter();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('typeTpl', { static: true }) typeTpl: TemplateRef<any>;
  @ViewChild('filterTypeTpl', { static: false }) filterTypeTpl: TemplateRef<
    any
  >;
  @ViewChild('statusTpl', { static: true }) statusTpl: TemplateRef<any>;
  @ViewChild('durationTpl', { static: true }) durationTpl: TemplateRef<any>;
  @ViewChild('sourceNameTpl', { static: true }) sourceNameTpl: TemplateRef<any>;
  @ViewChild('startTimeTpl', { static: true }) startTimeTpl: TemplateRef<any>;
  @ViewChild('sourceSubTypeTpl', { static: true })
  sourceSubTypeTpl: TemplateRef<any>;
  @ViewChild('eventStatusTpl', { static: true }) eventStatusTpl: TemplateRef<
    any
  >;
  @ViewChild('modalTtitleTpl', { static: true }) modalTtitleTpl: TemplateRef<
    any
  >;
  @ViewChild('handleHeaderTpl', { static: true }) handleHeaderTpl: TemplateRef<
    any
  >;
  @ViewChild('markStatusTpl', { static: true }) markStatusTpl: TemplateRef<any>;
  jobOriginType = JOB_ORIGIN_TYPE;
  isXSeries = includes(
    [
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );

  // 备份软件,包含X系列，E6000，软硬解耦
  isOceanProtect =
    this.appUtilsService.isDataBackup ||
    this.appUtilsService.isDecouple ||
    this.appUtilsService.isDistributed;

  showRetry = false;

  constructor(
    public appUtilsService: AppUtilsService,
    private i18n: I18NService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    private dataMapService: DataMapService,
    private copyActionService: CopyActionService,
    private copiesDetectReportService: CopiesDetectReportService,
    public slaApiService: SlaApiService,
    private datePipe?: DatePipe,
    private cdr?: ChangeDetectorRef,
    public cookieService?: CookieService,
    private jobApiService?: JobAPIService,
    private systemTimeService?: SystemTimeService,
    private batchOperateService?: BatchOperateService,
    private warningMessageService?: WarningMessageService,
    private sqlVerificateApiService?: AnonyControllerService,
    private virtualScroll?: VirtualScrollService
  ) {}

  ngAfterViewInit() {
    if (this.isHandled) {
      this.dataTable.fetchData();
    }
  }

  ngOnInit() {
    this.isHandled = this.activeIndex === JOB_ORIGIN_TYPE.HANDLED;
    this.querySlas();
    this.initConfig();
    this.statusArr = cloneDeep(this.statusList);
    if (this.isDataBackup || this.appUtilsService.isDecouple) {
      if (this.activeIndex === JOB_ORIGIN_TYPE.HISTORIC) {
        this.statusArr.push(DataMap.Job_status.dispatch_failed.value);
      }
    }
    this.showRetry =
      this.isOceanProtect &&
      (!this.selectedResource ||
        (!!this.selectedResource &&
          includes(
            [DataMap.Resource_Type.virtualMachine.value],
            this.subType
          )));
  }

  jobTypeMapping(val) {
    const mapping = this.dataMapService.toArray('Job_type');

    if (this.isHyperdetect) {
      each(mapping, item => {
        if (item.value === DataMap.Job_type.backup_job.value) {
          item.label = this.i18n.get('common_anti_detection_backup_label');
        }
      });
    }

    if (this.isCyberEngine) {
      each(mapping, item => {
        if (item.value === DataMap.Job_type.backup_job.value) {
          item.label = this.i18n.get('common_anti_detection_gen_label');
        }
        if (item.value === DataMap.Job_type.delete_copy_job.value) {
          item.label = this.i18n.get('protection_snap_delete_label');
        }
        if (item.value === DataMap.Job_type.restore_job.value) {
          item.label = this.i18n.get(
            'common_restore_to_origin_location_cyber_label'
          );
        }
      });
    }
    if (val === 'ManualScanEnvironment') {
      return this.i18n.get(
        DataMap.Job_type.job_type_manual_scan_environment.label
      );
    }
    return find(mapping, item => {
      return item.value == val;
    })?.label;
  }

  initConfig() {
    const opts: ProButton[] = getPermissionMenuItem(
      [
        {
          id: 'stop',
          permission: OperateItems.AbortJob,
          label: this.i18n.get('common_job_abort_label'),
          displayCheck: () => {
            return (
              this.activeIndex === JOB_ORIGIN_TYPE.EXE &&
              this.cookieService.role !== 5
            );
          },
          disableCheck: items =>
            !size(items) ||
            !isUndefined(
              find(items, d => {
                return (
                  d.status === DataMap.Job_status.aborting.value ||
                  d.type === DataMap.Job_type.archive_import_job.value ||
                  !d.enableStop ||
                  (d.sourceSubType ===
                    DataMap.Resource_Type.lightCloudGaussdbInstance.value &&
                    d.type === DataMap.Job_type.backup_job.value)
                );
              })
            ),
          onClick: items => this.stop(items)
        },
        {
          id: 'export',
          permission: OperateItems.ExportJob,
          label: this.i18n.get('common_download_label'),
          displayCheck: () => {
            return this.cookieService.role !== 5;
          },
          onClick: items => this.exportJob(items)
        },
        {
          id: 'remark',
          permission: OperateItems.ExportJob,
          label: this.i18n.get('common_set_remarks_label'),
          displayCheck: () => {
            return this.cookieService.role !== 5;
          },
          onClick: (items: JobBo[]) => this.setRemarks(first(items))
        },
        {
          id: 'handle',
          permission: OperateItems.ExportJob,
          label: this.i18n.get('common_handle_label'),
          displayCheck: ([data]) => {
            return (
              this.showRetry &&
              this.activeIndex === JOB_ORIGIN_TYPE.HISTORIC &&
              this.cookieService.role !== 5 &&
              data.markStatus !== DataMap.markStatus.notSupport.value
            );
          },
          onClick: (items: JobBo[]) => this.handle(first(items))
        },
        {
          id: 'retry',
          permission: OperateItems.ExportJob,
          label: this.i18n.get('common_retry_label'),
          displayCheck: ([data]) => {
            return (
              this.showRetry &&
              this.activeIndex === JOB_ORIGIN_TYPE.HISTORIC &&
              this.cookieService.role !== 5 &&
              [
                DataMap.markStatus.handled.value,
                DataMap.markStatus.notHandled.value
              ].includes(data.markStatus)
            );
          },
          disableCheck: items =>
            items[0]?.markStatus !== DataMap.markStatus.handled.value,
          disabledTips: this.i18n.get('common_job_retry_tip_label'),
          onClick: items => this.retry(items[0])
        }
      ],
      this.cookieService.role
    );

    let cols: TableCols[] = [
      {
        key: 'jobId',
        name: this.i18n.get('common_job_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'clusterName',
        name: this.i18n.get('system_servers_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'type',
        name: this.i18n.get('insight_job_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          showSearch: true,
          template: this.filterTypeTpl,
          options:
            this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value
              ? reject(this.getTypeOptions(), item =>
                  includes(
                    [
                      DataMap.Job_type.db_desesitization.value,
                      DataMap.Job_type.db_identification.value
                    ],
                    item.value
                  )
                )
              : this.getTypeOptions()
        },
        cellRender: this.typeTpl
      },
      {
        key: 'sourceName',
        name: this.i18n.get('common_object_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.sourceNameTpl
      },
      {
        key: 'sourceSubType',
        name: this.i18n.get('protection_object_type_label'),
        filter: !!this.selectedResource
          ? null
          : {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              showSearch: true,
              options: this.getSourceSubTypeOptions()
            },
        hidden: !!this.selectedResource,
        cellRender: this.sourceSubTypeTpl
      },
      {
        key: 'status',
        name: this.i18n.get('insight_job_status_label'),
        filter: this.isHandled
          ? null
          : {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              showSearch: this.activeIndex === JOB_ORIGIN_TYPE.HISTORIC,
              options: reject(
                this.dataMapService
                  .toArray('Job_status')
                  .filter(item => includes(this.statusList, item.value)),
                item =>
                  includes(
                    [
                      DataMap.Job_status.initialization.value,
                      DataMap.Job_status.aborting.value,
                      DataMap.Job_status.cancelled.value,
                      DataMap.Job_status.abnormal.value,
                      DataMap.Job_status.abort_failed.value,
                      DataMap.Job_status.redispatch.value
                    ],
                    item.value
                  )
              )
            },
        sort: this.activeIndex === JOB_ORIGIN_TYPE.EXE,
        cellRender: this.statusTpl
      },
      {
        key: 'markStatus',
        name: this.i18n.get('common_handle_status_label'),
        cellRender: this.markStatusTpl,
        filter: this.isHandled
          ? null
          : {
              type: 'select',
              options: this.dataMapService.toArray('markStatus'),
              isMultiple: true,
              showCheckAll: true
            }
      },
      {
        key: 'mark',
        name: this.i18n.get('common_handling_opinion_label')
      },
      {
        key: 'startTime',
        name: this.i18n.get('common_start_time_label'),
        sort: true,
        cellRender: this.startTimeTpl
      },
      {
        key: 'durationTime',
        name: this.i18n.get('protection_duration_time_label')
      },
      {
        key: 'eventStatus',
        name: this.i18n.get('common_event_status_label'),
        hidden: true,
        cellRender: this.eventStatusTpl,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('Job_Log_Level')
            .filter(
              item => !includes([DataMap.Job_Log_Level.info.value], item.value)
            )
        }
      },
      {
        key: 'remark',
        hidden: true,
        name: this.i18n.get('common_remarks_label')
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
            items: opts
          }
        }
      }
    ];

    if (!this.isXSeries) {
      // 只有 X 系列支持任务备注功能
      remove(cols, col => eq(col.key, 'remark'));
      remove(opts, opt => eq(opt.id, 'remark'));
      // 只有 X 系列支持多节点
      remove(cols, col => eq(col.key, 'clusterName'));
    }

    if (this.isHandled) {
      remove(cols, { key: 'type' });
      remove(cols, { key: 'sourceSubType' });
      remove(cols, { key: 'operation' });
    }

    // 非OP不支持处理状态和意见
    if (!this.isOceanProtect) {
      remove(cols, col => includes(['markStatus', 'mark'], col.key));
    }

    if (this.activeIndex === JOB_ORIGIN_TYPE.EXE) {
      cols = filter(cols, item => !includes(['eventStatus'], item.key));
    }
    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'jobId',
        columns: cols,
        showLoading: false,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: [JOB_ORIGIN_TYPE.EXE, JOB_ORIGIN_TYPE.HANDLED].includes(
            this.activeIndex
          )
        },
        selectionChange: selection => {
          this.selectionData = selection;
          this.selectionChange.emit(selection);
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        trackByFn: (index, item) => {
          return item.jobId;
        }
      },
      pagination: {
        mode: !!this.selectedResource || this.isHandled ? 'simple' : 'default',
        winTablePagination: this.isHandled
      }
    };
    this.optsConfig = filter(opts, item => includes(['stop'], item.id));
    this.groupOpts = cloneDeep(opts);
  }

  getTypeOptions() {
    const options = this.dataMapService
      .toArray(
        'Job_type',
        this.isHyperdetect
          ? [
              DataMap.Job_type.backup_job.value,
              DataMap.Job_type.restore_job.value,
              DataMap.Job_type.delete_copy_job.value,
              DataMap.Job_type.resource_protection.value,
              DataMap.Job_type.resource_protection_modify.value,
              DataMap.Job_type.job_type_manual_scan_environment.value,
              DataMap.Job_type.copyExpired.value
            ]
          : [
              DataMap.Job_type.backup_job.value,
              DataMap.Job_type.restore_job.value,
              DataMap.Job_type.delete_copy_job.value,
              DataMap.Job_type.archive_import_job.value,
              DataMap.Job_type.resource_protection.value,
              DataMap.Job_type.resource_protection_modify.value,
              DataMap.Job_type.job_type_manual_scan_environment.value,
              DataMap.Job_type.copyExpired.value
            ]
      )
      .filter(item => {
        if (item.value === DataMap.Job_type.migrate.value) return;
        return !!this.selectedResource
          ? filterJobType(
              item,
              this.selectedResource?.resourceType ===
                DataMap.Resource_Type.vmGroup.value
                ? this.selectedResource?.resourceType
                : includes(
                    [
                      DataMap.Resource_Type.FusionCompute.value,
                      DataMap.Resource_Type.fusionOne.value
                    ],
                    this.selectedResource?.sub_type
                  )
                ? this.selectedResource?.type
                : this.selectedResource?.sub_type
            )
          : item;
      })
      .filter(item => {
        return this.showExpiredCopy
          ? true
          : item.value !== DataMap.Job_type.copyExpired.value;
      });

    if (this.isHyperdetect) {
      each(options, item => {
        if (item.value === DataMap.Job_type.backup_job.value) {
          item.label = this.i18n.get('common_anti_detection_backup_label');
        }
      });
    }
    if (this.isCyberEngine) {
      each(options, item => {
        if (item.value === DataMap.Job_type.backup_job.value) {
          item.label = this.i18n.get('common_anti_detection_gen_label');
        }
        if (item.value === DataMap.Job_type.delete_copy_job.value) {
          item.label = this.i18n.get('protection_snap_delete_label');
        }
        if (item.value === DataMap.Job_type.restore_job.value) {
          item.label = this.i18n.get(
            'common_restore_to_origin_location_cyber_label'
          );
        }
      });
      remove(
        options,
        option =>
          !includes(
            [
              DataMap.Job_type.job_type_manual_scan_environment.value,
              DataMap.Job_type.backup_job.value,
              DataMap.Job_type.restore_job.value,
              DataMap.Job_type.antiRansomware.value,
              DataMap.Job_type.delete_copy_job.value,
              DataMap.Job_type.resource_protection.value,
              DataMap.Job_type.resource_protection_modify.value,
              DataMap.Job_type.live_mount_job.value,
              DataMap.Job_type.copyExpired.value
            ],
            option.value
          )
      );
    } else {
      remove(options, option =>
        eq(option.value, DataMap.Job_type.antiRansomware.value)
      );
    }

    if (this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value) {
      remove(options, option =>
        includes(
          [
            DataMap.Job_type.db_desesitization.value,
            DataMap.Job_type.db_identification.value
          ],
          option.value
        )
      );
    }

    if (this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE) {
      remove(options, option =>
        includes([DataMap.Job_type.archive_job.value], option.value)
      );
    }

    if (
      includes(
        [
          DataMap.Resource_Type.fileset.value,
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value,
          DataMap.Resource_Type.NASShare.value
        ],
        this.selectedResource?.subType
      )
    ) {
      return filter(options, item =>
        includes(
          [
            DataMap.Job_type.backup_job.value,
            DataMap.Job_type.restore_job.value,
            DataMap.Job_type.live_mount_job.value,
            DataMap.Job_type.copy_data_job.value,
            DataMap.Job_type.archive_job.value,
            DataMap.Job_type.delete_copy_job.value,
            DataMap.Job_type.resource_protection.value,
            DataMap.Job_type.resource_protection_modify.value,
            DataMap.Job_type.copyExpired.value
          ],
          item.value
        )
      );
    }
    if (
      includes(
        [
          DataMap.Resource_Type.KubernetesStatefulset.value,
          DataMap.Resource_Type.ClickHouse.value,
          DataMap.Resource_Type.DWS_Database.value,
          DataMap.Resource_Type.DWS_Schema.value,
          DataMap.Resource_Type.DWS_Table.value,
          DataMap.Resource_Type.SQLServerClusterInstance.value,
          DataMap.Resource_Type.SQLServerInstance.value,
          DataMap.Resource_Type.SQLServerDatabase.value,
          DataMap.Resource_Type.SQLServerGroup.value,
          DataMap.Resource_Type.generalDatabase.value,
          DataMap.Resource_Type.informixInstance.value,
          DataMap.Resource_Type.informixClusterInstance.value,
          DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
          DataMap.Resource_Type.lightCloudGaussdbInstance.value,
          DataMap.Resource_Type.OceanBaseCluster.value,
          DataMap.Resource_Type.OceanBaseTenant.value,
          DataMap.Resource_Type.saphanaDatabase.value
        ],
        this.selectedResource?.subType
      )
    ) {
      return filter(options, item =>
        includes(
          [
            DataMap.Job_type.backup_job.value,
            DataMap.Job_type.restore_job.value,
            DataMap.Job_type.copy_data_job.value,
            DataMap.Job_type.archive_job.value,
            DataMap.Job_type.delete_copy_job.value,
            DataMap.Job_type.resource_protection.value,
            DataMap.Job_type.resource_protection_modify.value,
            DataMap.Job_type.copyExpired.value
          ],
          item.value
        )
      );
    }
    if (
      this.selectedResource?.subType ===
      DataMap.Resource_Type.KubernetesNamespace.value
    ) {
      return filter(options, item =>
        includes(
          [
            DataMap.Job_type.resource_protection.value,
            DataMap.Job_type.resource_protection_modify.value,
            DataMap.Job_type.copyExpired.value
          ],
          item.value
        )
      );
    }

    if (
      includes(
        [DataMap.Resource_Type.DWS_Cluster.value],
        this.selectedResource?.subType
      )
    ) {
      return filter(options, item =>
        includes(
          [
            DataMap.Job_type.backup_job.value,
            DataMap.Job_type.restore_job.value,
            DataMap.Job_type.copy_data_job.value,
            DataMap.Job_type.archive_job.value,
            DataMap.Job_type.delete_copy_job.value,
            DataMap.Job_type.resource_protection.value,
            DataMap.Job_type.resource_protection_modify.value,
            DataMap.Job_type.job_type_manual_scan_environment.value,
            DataMap.Job_type.copyExpired.value
          ],
          item.value
        )
      );
    }

    if (
      includes(
        [
          DataMap.Resource_Type.dbTwoDatabase.value,
          DataMap.Resource_Type.dbTwoTableSet.value
        ],
        this.selectedResource?.subType
      )
    ) {
      return filter(options, item =>
        includes(
          [
            DataMap.Job_type.backup_job.value,
            DataMap.Job_type.restore_job.value,
            DataMap.Job_type.copy_data_job.value,
            DataMap.Job_type.archive_job.value,
            DataMap.Job_type.delete_copy_job.value,
            DataMap.Job_type.copies_verify_job.value,
            DataMap.Job_type.resource_protection.value,
            DataMap.Job_type.resource_protection_modify.value,
            DataMap.Job_type.copyExpired.value
          ],
          item.value
        )
      );
    }

    if (
      includes(
        [
          DataMap.Resource_Type.MySQLClusterInstance.value,
          DataMap.Resource_Type.MySQLInstance.value,
          DataMap.Resource_Type.MySQLDatabase.value
        ],
        this.selectedResource?.subType
      )
    ) {
      return filter(options, item =>
        includes(
          [
            DataMap.Job_type.backup_job.value,
            DataMap.Job_type.restore_job.value,
            DataMap.Job_type.copy_data_job.value,
            DataMap.Job_type.archive_job.value,
            DataMap.Job_type.delete_copy_job.value,
            DataMap.Job_type.resource_protection.value,
            DataMap.Job_type.resource_protection_modify.value,
            DataMap.Job_type.live_mount_job.value,
            DataMap.Job_type.copyExpired.value
          ],
          item.value
        )
      );
    }
    if (
      DataMap.Resource_Type.virtualMachine.value ===
        this.selectedResource?.subType ||
      DataMap.Resource_Type.virtualMachine.value ===
        this.selectedResource?.sub_type
    ) {
      return filter(options, item =>
        includes(
          [
            DataMap.Job_type.backup_job.value,
            DataMap.Job_type.restore_job.value,
            DataMap.Job_type.copy_data_job.value,
            DataMap.Job_type.archive_job.value,
            DataMap.Job_type.delete_copy_job.value,
            DataMap.Job_type.resource_protection.value,
            DataMap.Job_type.resource_protection_modify.value,
            DataMap.Job_type.live_mount_job.value,
            DataMap.Job_type.copyExpired.value
          ],
          item.value
        )
      );
    }

    if (
      includes(
        [
          DataMap.Resource_Type.DBBackupAgent.value,
          DataMap.Resource_Type.VMBackupAgent.value,
          DataMap.Resource_Type.UBackupAgent.value,
          DataMap.Resource_Type.SBackupAgent.value
        ],
        this.selectedResource?.sub_type
      )
    ) {
      return filter(options, item =>
        includes(
          [
            DataMap.Job_type.host_register.value,
            DataMap.Job_type.protect_agent_update.value,
            DataMap.Job_type.job_type_manual_scan_environment.value,
            DataMap.Job_type.modifyApplications.value
          ],
          item.value
        )
      );
    }

    return options;
  }

  getSourceSubTypeOptions() {
    const options = this.dataMapService
      .toArray('Job_Target_Type', [
        DataMap.Job_Target_Type.LocalFileSystem.value,
        DataMap.Job_Target_Type.s3Storage.value
      ])
      .map(item => {
        if (
          this.cookieService.isCloudBackup &&
          item.value === DataMap.Job_Target_Type.s3Storage.value
        ) {
          item.label = this.i18n.get('common_backup_storage_label');
        }
        return item;
      })
      .filter(item => {
        if (this.isCyberEngine) {
          return includes(
            [
              DataMap.Job_Target_Type.OceanStorDorado.value,
              DataMap.Job_Target_Type.cyberOceanStorPacific.value,
              DataMap.Job_Target_Type.OceanProtect.value,
              DataMap.Job_Target_Type.LocalFileSystem.value
            ],
            item.value
          );
        }
        if (
          this.appUtilsService.isDistributed ||
          this.appUtilsService.isDecouple
        ) {
          return !includes(
            [
              DataMap.Job_Target_Type.NASFileSystem.value,
              DataMap.Job_Target_Type.commonShare.value
            ],
            item.value
          );
        }
        return !includes(
          [
            DataMap.Job_Target_Type.ImportCopy.value,
            DataMap.Job_Target_Type.DBBackupAgent.value,
            DataMap.Job_Target_Type.OceanStorDorado.value,
            DataMap.Job_Target_Type.cyberOceanStorPacific.value,
            DataMap.Job_Target_Type.OceanProtect.value,
            DataMap.Job_Target_Type.fileSystem.value,
            DataMap.Job_Target_Type.VMwarevCenterServer.value,
            DataMap.Job_Target_Type.LocalLun.value
          ],
          item.value
        );
      });

    if (this.isCyberEngine) {
      options.push({
        ...DataMap.Job_Target_Type.fileSystem,
        label: this.i18n.get('common_file_system_label'),
        key: DataMap.Job_Target_Type.fileSystem.value
      });
    }
    if (this.isHyperdetect) {
      options.push({
        ...DataMap.Job_Target_Type.LocalLun,
        label: this.i18n.get('protection_local_lun_label'),
        key: DataMap.Job_Target_Type.LocalLun.value
      });
      if (!SupportLicense.isFile) {
        return reject(options, item =>
          includes([DataMap.Job_Target_Type.LocalFileSystem.value], item.value)
        );
      }
      if (!SupportLicense.isSan) {
        return reject(options, item =>
          includes([DataMap.Job_Target_Type.LocalLun.value], item.value)
        );
      }
    }
    return options;
  }

  hasLogEvent(logLevel, type?) {
    logLevel = parseInt(logLevel, 10);
    if (isNaN(logLevel)) {
      return false;
    }
    let levelBinary = logLevel.toString(2);
    while (levelBinary.length < 3) {
      levelBinary = '0' + levelBinary;
    }
    if (type === DataMap.Job_Log_Level.warning.value) {
      return levelBinary[0] == '1';
    } else if (type === DataMap.Job_Log_Level.error.value) {
      return levelBinary[1] == '1';
    } else if (type === DataMap.Job_Log_Level.fatal.value) {
      return levelBinary[2] === '1';
    } else {
      return includes(levelBinary, '1');
    }
  }

  hasWarning(logLevel) {
    return this.hasLogEvent(logLevel, DataMap.Job_Log_Level.warning.value);
  }

  hasError(logLevel) {
    return this.hasLogEvent(logLevel, DataMap.Job_Log_Level.error.value);
  }

  hasFatal(logLevel) {
    return this.hasLogEvent(logLevel, DataMap.Job_Log_Level.fatal.value);
  }

  exportJob(items?) {
    const params = {
      lang: this.i18n.language === LANGUAGE.CN ? 'zh_CN' : 'en',
      isSystem: false,
      isVisible: true
    };
    const filters: any = this.getParams(this.dataTable.getFilterMap(), {});
    if (!isEmpty(items)) {
      assign(params, {
        jobId: items[0]['jobId']
      });
    } else {
      // 资源ID
      if (!isEmpty(filters.sourceId)) {
        assign(params, {
          sourceId: filters.sourceId
        });
      }
      // 任务ID
      if (!isEmpty(filters.jobId)) {
        assign(params, {
          jobId: filters.jobId
        });
      }
      // 节点
      if (!isEmpty(filters.clusterName)) {
        assign(params, {
          clusterName: filters.clusterName
        });
      }
      // 任务类型
      if (!isEmpty(filters.types)) {
        assign(params, {
          types: filters.types
        });
      }
      // 排查副本过期
      if (!isUndefined(filters.excludeTypes)) {
        assign(params, {
          excludeTypes: filters.excludeTypes
        });
      }
      // 对象名称
      if (!isEmpty(filters.sourceName)) {
        assign(params, {
          sourceName: filters.sourceName
        });
      }
      // 对象类型
      if (!isEmpty(filters.sourceTypes)) {
        assign(params, {
          sourceTypes: filters.sourceTypes
        });
      }
      // 任务状态
      if (!isEmpty(filters.statusList)) {
        assign(params, {
          statusList: filters.statusList
        });
      }
      // 事件状态
      if (filters.logLevels) {
        assign(params, {
          logLevels: filters.logLevels
        });
      }
      // 开始时间排序
      if (filters.orderBy && filters.orderType) {
        assign(params, {
          orderBy: filters.orderBy,
          orderType: filters.orderType
        });
      }
      //开始时间
      if (
        !isEmpty(this.rangeDate) &&
        (isDate(this.rangeDate[0]) || isDate(this.rangeDate[1]))
      ) {
        assign(params, {
          fromStartTime: new Date(this.rangeDate[0]).getTime(),
          toStartTime: new Date(this.rangeDate[1]).getTime()
        });
      }
    }
    this.messageService.info(
      this.i18n.get('common_file_download_processing_label'),
      {
        lvDuration: 0,
        lvShowCloseButton: true,
        lvMessageKey: 'jobDownloadKey'
      }
    );
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

  stop(items?) {
    const data = items || this.selectionData;
    if (
      !isUndefined(
        find(data, d => {
          return !d.enableStop;
        })
      )
    ) {
      return;
    }

    let warnTip = this.i18n.get('common_task_stop_label');
    if (
      find(
        data,
        d =>
          DataMap.Resource_Type.gaussdbForOpengaussInstance.value ===
          d.sourceSubType
      )
    ) {
      warnTip = this.i18n.get('insight_stop_job_gaussdb_warning_label', [
        'protection_gaussdb_for_opengauss_label'
      ]);
    } else if (
      find(
        data,
        d =>
          DataMap.Resource_Type.lightCloudGaussdbInstance.value ===
          d.sourceSubType
      )
    ) {
      warnTip = this.i18n.get('insight_stop_job_gaussdb_warning_label', [
        'protection_light_cloud_gaussdb_label'
      ]);
    } else {
      warnTip = this.i18n.get('common_task_stop_label');
    }
    this.warningMessageService.create({
      width: 500,
      content: warnTip,
      onOK: () => {
        if (data.length === 1) {
          if (
            includes(
              [
                DataMap.Job_type.db_desesitization.value,
                DataMap.Job_type.db_identification.value
              ],
              data[0].type
            )
          ) {
            this.sqlVerificateApiService
              .stopTaskUsingPOST({
                request: {
                  request_id: data[0].jobId
                }
              })
              .subscribe(() => this.dataTable.fetchData());
          } else {
            this.jobApiService
              .stopTaskUsingPUT({ jobId: data[0].jobId })
              .subscribe(() => this.dataTable.fetchData());
          }
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.jobApiService.stopTaskUsingPUT({
                jobId: item.jobId,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            map(cloneDeep(data), d => {
              return assign(d, { name: d.sourceName, isAsyn: false });
            }),
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            },
            '',
            true
          );
        }
      }
    });
  }
  setRemarks(row: JobBo) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_set_remarks_label'),
      lvModalKey: 'modify_remarks',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: ModifyRemarksComponent,
      lvComponentParams: {
        row
      },
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as ModifyRemarksComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as ModifyRemarksComponent;
          content.onOK().subscribe({
            next: () => {
              this.dataTable.fetchData();
              resolve(true);
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  clickSourceName(item) {
    if (!this.isHandled) {
      return;
    }
    this.getDetail(item);
  }

  handle(row: JobBo) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.handleHeaderTpl,
      lvModalKey: 'handle',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: ModifyHandleComponent,
      lvComponentParams: {
        row
      },
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as ModifyHandleComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as ModifyHandleComponent;
          content.onOK().subscribe({
            next: () => {
              this.dataTable.fetchData();
              resolve(true);
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  retry(row) {
    this.jobApiService
      .retryJobUsingPost({
        jobId: row?.jobId
      })
      .subscribe(() => this.dataTable.fetchData());
  }

  batchRetry() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_batch_retry_label'),
      lvWidth: MODAL_COMMON.xLargeWidth,
      lvModalKey: 'batch_retry',
      lvContent: BatchRetryComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as BatchRetryComponent;
        content.invalidEmitter.subscribe(res => {
          modal.lvOkDisabled = res;
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as BatchRetryComponent;
          content.onOK().subscribe({
            next: () => {
              this.dataTable.fetchData();
              resolve(true);
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  retriedJobDetail(job) {
    if (job.markStatus !== DataMap.markStatus.retried.value) {
      return;
    }
    const retriedJobId = JSON.parse(job.extendStr)?.retry_job_id;
    this.jobApiService
      .queryJobUsingGET({
        jobId: retriedJobId
      })
      .subscribe(res => this.openDetail(res));
  }

  getEventStatusParams(eventStatus) {
    const eventMap = {
      [DataMap.Job_Log_Level.warning.value]: 100,
      [DataMap.Job_Log_Level.error.value]: 10,
      [DataMap.Job_Log_Level.fatal.value]: 1
    };
    let logLevel = 0;
    each(eventStatus, item => {
      logLevel += eventMap[item];
    });
    return parseInt(`${logLevel}`, 2);
  }

  getParams(filter, args) {
    const params = {
      isSystem: false,
      isVisible: true,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true,
      startPage: filter.paginator.pageIndex + 1,
      pageSize: filter.paginator.pageSize,
      statusList: this.statusArr,
      orderType: 'desc',
      orderBy: 'startTime'
    };

    if (this.isHandled) {
      assign(params, { markStatus: DataMap.markStatus.handled.value });
    }

    if (!isEmpty(filter.conditions)) {
      const conditions = JSON.parse(filter.conditions);
      if (conditions.type) {
        assign(params, {
          types: includes(
            conditions.type,
            DataMap.Job_type.job_type_manual_scan_environment.value
          )
            ? ['ManualScanEnvironment', ...conditions.type]
            : conditions.type
        });
      } else {
        set(params, 'excludeTypes', !this.showExpiredCopy);
        if (!this.showExpiredCopy) {
          set(params, 'types', [DataMap.Job_type.copyExpired.value]);
        }
      }

      if (conditions.status) {
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
                    DataMap.Job_status.dispatch_failed.value
                  );
                  if (this.isDataBackup) {
                    array.push(DataMap.Job_status.dispatch_failed.value);
                  }
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

      if (conditions.sourceSubType) {
        if (
          includes(
            conditions.sourceSubType,
            DataMap.Job_Target_Type.fileSystem.value
          )
        ) {
          remove(
            conditions.sourceSubType,
            item => item === DataMap.Job_Target_Type.fileSystem.value
          );
          conditions.sourceSubType = concat(
            conditions.sourceSubType,
            DataMap.Job_Target_Type.NASFileSystem.value,
            DataMap.Job_Target_Type.LocalFileSystem.value
          );
        }
        if (
          includes(
            conditions.sourceSubType,
            DataMap.Job_Target_Type.VMware.value
          )
        ) {
          conditions.sourceSubType = [
            ...conditions.sourceSubType,
            DataMap.Job_Target_Type.VMwarevCenterServer.value
          ];
        }
        assign(params, {
          sourceTypes: conditions.sourceSubType.map(item => {
            if (
              includes(
                [
                  DataMap.Job_Target_Type.FusionCompute.value,
                  DataMap.Job_Target_Type.FusionOneCompute.value
                ],
                item
              )
            ) {
              return `VM__and__${item}`;
            }
            return item;
          })
        });
      }

      if (conditions.eventStatus) {
        assign(params, {
          logLevels: this.getEventStatusParams(conditions.eventStatus)
        });
      }

      const others = omit(conditions, [
        'type',
        'status',
        'sourceSubType',
        'eventStatus'
      ]);
      if (!isEmpty(others)) {
        assign(params, {
          ...others
        });
      }
    } else {
      set(params, 'excludeTypes', !this.showExpiredCopy);
      if (!this.showExpiredCopy) {
        set(params, 'types', [DataMap.Job_type.copyExpired.value]);
      }
    }

    if (!!size(filter.orders)) {
      assign(params, {
        orderType: filter.sort.direction,
        orderBy: filter.sort.key === 'status' ? 'progress' : filter.sort.key
      });
    }

    if (this.fromStartTime && this.toStartTime) {
      assign(params, {
        fromStartTime: this.fromStartTime,
        toStartTime: this.toStartTime
      });
    }
    if (this.slaNameData) {
      const arr = [];
      map(this.slaNameData, item => {
        arr.push(item.id);
      });
      assign(params, {
        slaIds: arr
      });
    }
    if (this.selectedResource) {
      assign(params, {
        sourceId: this.selectedResource.uuid
      });
    }

    return params;
  }

  getData(filter, args) {
    combineLatest([
      this.systemTimeService.getSystemTime(
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
      ),
      this.jobApiService.queryJobsUsingGET({
        ...this.getParams(filter, args)
      })
    ]).subscribe(res => {
      this.timeOffset = res[0].offset;
      this.timeZone = res[0].displayName;
      this.sysTime = new Date(
        `${res[0].time.replace(/-/g, '/')} ${res[0].displayName}`
      ).getTime();
      this.tableData = {
        data: map(res[1].records, item => {
          item['durationTime'] = this.getDuration(
            includes(
              [
                DataMap.Job_status.running.value,
                DataMap.Job_status.initialization.value,
                DataMap.Job_status.pending.value,
                DataMap.Job_status.aborting.value
              ],
              item.status
            )
              ? this.sysTime - item.startTime < 0
                ? 0
                : this.sysTime - item.startTime
              : item.endTime
              ? item.endTime - item.startTime
              : 0
          );
          const tag = get(JSON.parse(item.extendStr), 'tag', void 0);
          !isNil(tag) && set(item, 'remark', tag);
          return item;
        }),
        total: res[1].totalCount
      };
      if (this.isCyberEngine) {
        each(this.tableData.data, item => {
          if (
            item.sourceSubType ===
              DataMap.Job_Target_Type.NASFileSystem.value ||
            item.sourceSubType === DataMap.Job_Target_Type.LocalFileSystem.value
          ) {
            item.sourceSubType = DataMap.Job_Target_Type.fileSystem.value;
          }
        });
      }
      this.cdr.detectChanges();
    });
  }

  openDetail(job, detectionCopy?) {
    const params = {
      job: {
        ...job,
        colors:
          job.status === DataMap.Job_status.aborting.value
            ? this.abortingColors
            : this.colors,
        getDuration: this.getDuration
      }
    };
    if (this.dataTable) {
      this.dataTable.activeItem = job;
    }
    const footer = isArray(detectionCopy)
      ? [
          {
            label: this.i18n.get('common_view_detection_report_label'),
            type: 'primary',
            disabled:
              isEmpty(detectionCopy) ||
              !includes(
                [
                  DataMap.detectionSnapshotStatus.infected.value,
                  DataMap.detectionSnapshotStatus.uninfected.value
                ],
                detectionCopy[0].anti_status
              ),
            tips: isEmpty(detectionCopy)
              ? this.i18n.get('common_anti_copy_deleted_label')
              : !includes(
                  [
                    DataMap.detectionSnapshotStatus.infected.value,
                    DataMap.detectionSnapshotStatus.uninfected.value
                  ],
                  detectionCopy[0].anti_status
                )
              ? this.i18n.get('common_anti_copy_detecting_label')
              : '',
            onClick: () =>
              this.copyActionService.detectionReport(detectionCopy[0])
          },
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      : [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ];

    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader:
          job.type === 'ManualScanEnvironment'
            ? this.i18n.get(
                DataMap.Job_type.job_type_manual_scan_environment.label
              )
            : job.type === 'RESTORE' && this.isCyberEngine
            ? this.i18n.get('common_restore_to_origin_location_cyber_label')
            : this.dataMapService.getLabel('Job_type', job.type),
        lvContent:
          job.type === DataMap.Job_type.groupBackup.value
            ? GroupJobDetailComponent
            : JobDetailComponent,
        lvModalKey: 'jobDetailModalKey',
        lvWidth: this.i18n.isEn
          ? MODAL_COMMON.largeWidth + 100
          : MODAL_COMMON.largeWidth,
        lvComponentParams:
          job.type === DataMap.Job_type.groupBackup.value
            ? assign(params, {
                isSelectedResource: this.selectedResource,
                detailData: {
                  name: this.dataMapService.getLabel('Job_type', job.type),
                  optItems: getTableOptsItems(
                    cloneDeep(this.groupOpts),
                    job,
                    this
                  ),
                  optItemsFn: v => {
                    return getTableOptsItems(
                      cloneDeep(this.groupOpts),
                      v,
                      this
                    );
                  }
                }
              })
            : params,
        lvFooter: footer,
        lvAfterClose: () => {
          if (this.dataTable) {
            this.dataTable.setActiveItemEmpty();
          }
        }
      })
    );
  }

  getDetail(job) {
    if (
      this.isCyberEngine &&
      job.type === DataMap.Job_type.antiRansomware.value &&
      job.status === DataMap.Job_status.success.value
    ) {
      this.copiesDetectReportService
        .ShowDetectionDetails({
          akDoException: false,
          resourceId: job.sourceId,
          pageNo: 0,
          pageSize: 20,
          conditions: JSON.stringify({
            uuid: job.copyId
          })
        })
        .subscribe({
          next: res => {
            this.openDetail(job, res.items || []);
          },
          error: () => this.openDetail(job)
        });
    } else {
      this.openDetail(job);
    }
  }

  getDuration(msTime) {
    const time = msTime / 1000;
    let hour: any = Math.floor(time / 60 / 60);
    hour = hour.toString().padStart(2, '0');
    let minute: any = Math.floor(time / 60) % 60;
    minute = minute.toString().padStart(2, '0');
    let second: any = Math.floor(time) % 60;
    second = second.toString().padStart(2, '0');
    return `${hour}:${minute}:${second}`;
  }

  getSourceNameDesc(job) {
    let desc;
    const extendObject = isEmpty(job.extendStr)
      ? {}
      : JSON.parse(job.extendStr);
    const backupType = extendObject.sourceCopyType || extendObject.backupType;
    switch (job.type) {
      case DataMap.Job_type.backup_job.value:
        if (
          [
            DataMap.Resource_Type.FusionCompute.value,
            DataMap.Resource_Type.fusionOne.value,
            DataMap.Resource_Type.HCSCloudHost.value,
            DataMap.Resource_Type.virtualMachine.value,
            DataMap.Resource_Type.clusterComputeResource.value,
            DataMap.Resource_Type.hostSystem.value,
            DataMap.Resource_Type.openStackCloudServer.value,
            DataMap.Resource_Type.tdsqlInstance.value
          ].includes(job.sourceSubType)
        ) {
          desc =
            extendObject.slaName && backupType
              ? `${this.i18n.get('SLA', [], true)}${
                  extendObject.slaName
                } | ${this.i18n.get(
                  SLA_BACKUP_NAME[
                    backupType === 'difference_increment'
                      ? 'permanent_increment'
                      : backupType
                  ]
                )}`
              : '';
        } else {
          desc =
            extendObject.slaName && backupType
              ? `${this.i18n.get('SLA', [], true)}${
                  extendObject.slaName
                } | ${this.i18n.get(SLA_BACKUP_NAME[backupType])}`
              : '';
        }
        break;
      case DataMap.Job_type.copy_data_job.value:
      case DataMap.Job_type.archive_job.value:
      case DataMap.Job_type.resource_protection.value:
      case DataMap.Job_type.resource_protection_modify.value:
      case DataMap.Job_type.groupBackup.value:
        desc = extendObject.slaName
          ? `${this.i18n.get('SLA', [], true)}${extendObject.slaName}`
          : '';
        break;
      case DataMap.Job_type.copyExpired.value:
      case DataMap.Job_type.restore_job.value:
      case DataMap.Job_type.live_mount_job.value:
      case DataMap.Job_type.live_restore_job.value:
      case DataMap.Job_type.delete_copy_job.value:
      case DataMap.Job_type.archive_import_job.value:
        desc =
          extendObject.restoreType === RestoreType.FileRestore
            ? `${this.i18n.get(
                'common_restore_type_label',
                [],
                true
              )}${this.i18n.get('common_file_level_restore_label')}`
            : extendObject.restoreType === RestoreV2Type.FileRestore
            ? `${this.i18n.get('common_restore_type_label', [], true)}${
                includes(
                  [
                    DataMap.Job_Target_Type.HBaseBackupSet.value,
                    DataMap.Job_Target_Type.ClickHouse.value,
                    DataMap.Job_Target_Type.HiveBackupSet.value,
                    DataMap.Job_Target_Type.dwsCluster.value,
                    DataMap.Job_Target_Type.dwsSchema.value,
                    DataMap.Job_Target_Type.dwsTable.value,
                    DataMap.Job_Target_Type.oceanBaseCluster.value,
                    DataMap.Job_Target_Type.oracle.value,
                    DataMap.Job_Target_Type.oracleCluster.value
                  ],
                  job.sourceSubType
                )
                  ? this.i18n.get('protection_table_level_restore_label')
                  : includes(
                      [DataMap.Job_Target_Type.damengSingleNode.value],
                      job.sourceSubType
                    )
                  ? this.i18n.get('common_file_table_level_restore_label')
                  : includes(
                      [
                        DataMap.Job_Target_Type.SQLServerInstance.value,
                        DataMap.Job_Target_Type.sqlServerClusterInstance.value,
                        DataMap.Job_Target_Type.SQLServerDatabase.value
                      ],
                      job.sourceSubType
                    )
                  ? this.i18n.get('explore_database_restore_label')
                  : includes(
                      [DataMap.Job_Target_Type.ElasticSearch.value],
                      job.sourceSubType
                    )
                  ? this.i18n.get('explore_index_level_restore_label')
                  : includes(
                      [
                        DataMap.Job_Target_Type.ActiveDirectory.value,
                        DataMap.Job_Target_Type.ObjectSet.value
                      ],
                      job.sourceSubType
                    )
                  ? this.i18n.get('common_object_level_restore_label')
                  : includes(
                      [DataMap.Resource_Type.ExchangeEmail.value],
                      job.sourceSubType
                    )
                  ? this.i18n.get('common_email_level_restore_label')
                  : includes(
                      [DataMap.Resource_Type.ExchangeDataBase.value],
                      job.sourceSubType
                    )
                  ? this.i18n.get('common_user_level_restore_label')
                  : includes(
                      [
                        DataMap.Resource_Type.cNwareVm.value,
                        DataMap.Resource_Type.FusionCompute.value,
                        DataMap.Resource_Type.fusionOne.value,
                        DataMap.Resource_Type.openStackCloudServer.value
                      ],
                      job.sourceSubType
                    )
                  ? this.i18n.get('common_disk_restore_label')
                  : this.i18n.get('common_file_level_restore_label')
              }`
            : job.copyTime
            ? `${this.i18n.get(
                'common_time_stamp_label',
                [],
                true
              )}${this.datePipe.transform(
                job.copyTime,
                'yyyy/MM/dd HH:mm:ss',
                this.timeZone
              )}`
            : '';
        break;
      case DataMap.Job_type.restore_job.value:
      case DataMap.Job_type.migrate.value:
      case DataMap.Job_type.resource_scan.value:
      case DataMap.Job_type.unmout.value:
      case DataMap.Job_type.host_register.value:
      case DataMap.Job_type.protect_agent_update.value:
      case DataMap.Job_type.job_type_manual_scan_environment.value:
        desc = job.sourceLocation
          ? `${this.i18n.get('common_location_label', [], true)}${
              job.sourceLocation
            }`
          : '';
        break;
      default:
        break;
    }
    return desc;
  }

  changePickerMode(dates) {
    if (dates[0] && dates[1]) {
      this.fromStartTime = new Date(dates[0]).getTime();
      this.toStartTime = new Date(dates[1]).getTime();
    }
    if (!dates[0] && !dates[1]) {
      this.fromStartTime = null;
      this.toStartTime = null;
    }

    this.dataTable.fetchData();
  }

  querySlas() {
    // 从资源详情进入的任务需要过滤当前应用SLA
    const resource = this.appUtilsService.getApplicationConfig();
    let appList = [];
    for (let key in resource) {
      each(resource[key], item => {
        if (includes(item.key, this.subType)) {
          appList.push(item.slaId);
        }
      });
    }
    if (
      includes(
        [
          DataMap.Resource_Type.generalDatabase.value,
          DataMap.Resource_Type.virtualMachine.value,
          DataMap.Resource_Type.cNwareVm.value,
          DataMap.Resource_Type.msVirtualMachine.value,
          DataMap.Resource_Type.lightCloudGaussdbInstance.value,
          DataMap.Resource_Type.GaussDB_T.value,
          DataMap.Resource_Type.GaussDB_DWS.value,
          DataMap.Resource_Type.DB2.value,
          ApplicationType.KubernetesStatefulSet,
          DataMap.Resource_Type.MySQL.value,
          DataMap.Resource_Type.informixService.value,
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.fusionOne.value,
          DataMap.Resource_Type.goldendb.value,
          DataMap.Resource_Type.HCSCloudHost.value,
          DataMap.Resource_Type.Dameng.value,
          DataMap.Resource_Type.OpenGauss.value,
          DataMap.Resource_Type.ElasticsearchBackupSet.value,
          ApplicationType.OpenStack,
          ApplicationType.Oracle,
          ApplicationType.OceanBase,
          ApplicationType.Exchange,
          ApplicationType.ApsaraStack
        ],
        appList[0]
      )
    ) {
      appList.push(ApplicationType.Common);
    }

    this.slaApiService
      .pageQueryUsingGET({
        pageNo: CommonConsts.PAGE_START,
        pageSize: 10000,
        akLoading: false,
        applications: appList
      })
      .subscribe(res => {
        const arr = [];
        each(res.items, item => {
          arr.push({
            id: item.uuid,
            label: item.name,
            isLeaf: true,
            expand: false
          });
        });
        this.slaNameOps = arr;
        this.cdr.detectChanges();
      });
  }

  changeSlaName(e) {
    this.dataTable?.fetchData();
  }

  hideExeJobBtn(): boolean {
    return (
      this.activeIndex === JOB_ORIGIN_TYPE.EXE &&
      !isEmpty(this.selectedResource) &&
      (includes(
        [
          DataMap.Resource_Type.openStackProject.value,
          DataMap.Resource_Type.HCSProject.value
        ],
        this.selectedResource?.subType
      ) ||
        (includes(
          [
            DataMap.Resource_Type.FusionComputeCluster.value,
            DataMap.Resource_Type.FusionComputeCNA.value
          ],
          this.selectedResource?.type
        ) &&
          this.selectedResource?.subType ===
            DataMap.Resource_Type.FusionCompute.value))
    );
  }
}
