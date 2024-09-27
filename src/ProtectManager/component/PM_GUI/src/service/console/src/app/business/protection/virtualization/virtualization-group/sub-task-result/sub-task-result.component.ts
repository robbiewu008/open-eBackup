import { DatePipe } from '@angular/common';
import {
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import { JobDetailComponent } from 'app/business/insight/job/job-table/job-detail/job-detail.component';
import { JobTableComponent } from 'app/business/insight/job/job-table/job-table.component';
import { HuaWeiStackListComponent } from 'app/business/protection/cloud/huawei-stack/stack-list/huawei-stack-list.component';
import { OpenstackListComponent } from 'app/business/protection/cloud/openstack/openstack-list/openstack-list.component';
import {
  CommonConsts,
  CookieService,
  CopiesDetectReportService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  JobAPIService,
  JobColorConsts,
  MODAL_COMMON,
  OperateItems,
  RestoreV2Type,
  SlaApiService,
  SLA_BACKUP_NAME,
  WarningMessageService
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
import {
  assign,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isNil,
  isUndefined,
  map,
  omit,
  reduce,
  reject,
  set,
  size,
  uniq
} from 'lodash';
import { combineLatest } from 'rxjs';
import { FusionListComponent } from '../../fusion-compute/fusion-list/fusion-list.component';
import { BaseTableComponent } from '../../virtualization-base/base-table/base-table.component';
import { VmListComponent } from '../../vmware/vm-list/vm-list.component';

@Component({
  selector: 'aui-sub-task-result',
  templateUrl: './sub-task-result.component.html',
  styleUrls: ['./sub-task-result.component.less']
})
export class SubTaskResultComponent implements OnInit, AfterViewInit {
  @Input() groupJobInfo;
  tableConfig: TableConfig;
  tableData: TableData;
  timeZone = 'UTC+08:00';
  sysTime;
  timeOffset;
  optsConfig;
  selectionData = [];

  colors = [[0, JobColorConsts.SUCCESSFUL]];
  abortingColors = [[0, JobColorConsts.ABORTED]];
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

  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  runningStatusList = this.isOceanProtect
    ? [
        DataMap.Job_status.running.value,
        DataMap.Job_status.initialization.value,
        DataMap.Job_status.pending.value,
        DataMap.Job_status.aborting.value,
        DataMap.Job_status.dispatching.value,
        DataMap.Job_status.redispatch.value
      ]
    : [
        DataMap.Job_status.running.value,
        DataMap.Job_status.initialization.value,
        DataMap.Job_status.pending.value,
        DataMap.Job_status.aborting.value
      ];
  historicStatusList = map(
    reject(this.dataMapService.toArray('Job_status'), item =>
      includes(
        [
          DataMap.Job_status.running.value,
          DataMap.Job_status.initialization.value,
          DataMap.Job_status.pending.value,
          DataMap.Job_status.aborting.value,
          DataMap.Job_status.dispatching.value,
          DataMap.Job_status.redispatch.value,
          DataMap.Job_status.dispatch_failed.value
        ],
        item.value
      )
    ),
    'value'
  );
  statusList;
  jobTable: JobTableComponent;
  fcListComponent: FusionListComponent;
  vmListComponent: VmListComponent;
  hcsListComponent: HuaWeiStackListComponent;
  openstackListComponent: OpenstackListComponent;
  baseTableComponent: BaseTableComponent;

  @ViewChild('jobIdTpl', { static: true }) jobIdTpl: TemplateRef<any>;
  @ViewChild('durationTpl', { static: true }) durationTpl: TemplateRef<any>;
  @ViewChild('sourceNameTpl', { static: true }) sourceNameTpl: TemplateRef<any>;
  @ViewChild('startTimeTpl', { static: true }) startTimeTpl: TemplateRef<any>;
  @ViewChild('statusTpl', { static: true }) statusTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private drawModalService: DrawModalService,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private copyActionService: CopyActionService,
    private jobApiService: JobAPIService,
    private copiesDetectReportService: CopiesDetectReportService,
    private appUtilsService: AppUtilsService,
    public messageService: MessageService,
    public slaApiService: SlaApiService,
    private datePipe?: DatePipe,
    private cdr?: ChangeDetectorRef,
    public cookieService?: CookieService,
    private warningMessageService?: WarningMessageService,
    private systemTimeService?: SystemTimeService,
    private batchOperateService?: BatchOperateService
  ) {}

  ngOnInit(): void {
    this.statusList = this.runningStatusList.concat(this.historicStatusList);
    this.initConfig();
    this.jobTable = new JobTableComponent(
      this.appUtilsService,
      this.i18n,
      this.messageService,
      this.drawModalService,
      this.dataMapService,
      this.copyActionService,
      this.copiesDetectReportService,
      this.slaApiService,
      this.datePipe,
      this.cdr,
      this.cookieService,
      this.jobApiService,
      this.systemTimeService,
      this.batchOperateService,
      this.warningMessageService
    );
  }

  ngAfterViewInit() {
    this.jobTable.dataTable = this.dataTable;
    this.dataTable.fetchData();
  }

  initConfig() {
    const opts: ProButton[] = getPermissionMenuItem(
      [
        {
          id: 'stop',
          permission: OperateItems.AbortJob,
          label: this.i18n.get('common_job_abort_label'),
          displayCheck: () => {
            return this.cookieService.role !== 5;
          },
          disableCheck: items =>
            !size(items) ||
            !isUndefined(
              find(items, d => {
                return (
                  d.status === DataMap.Job_status.aborting.value ||
                  d.type === DataMap.Job_type.archive_import_job.value ||
                  !d.enableStop ||
                  !includes(
                    [
                      DataMap.Job_status.running.value,
                      DataMap.Job_status.initialization.value,
                      DataMap.Job_status.pending.value,
                      DataMap.Job_status.dispatching.value,
                      DataMap.Job_status.redispatch.value
                    ],
                    d.status
                  )
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
        }
      ],
      this.cookieService.role
    );
    const cols: TableCols[] = [
      {
        key: 'jobId',
        name: this.i18n.get('common_job_id_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.jobIdTpl
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
        key: 'status',
        name: this.i18n.get('insight_job_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          showSearch: true,
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
        cellRender: this.statusTpl
      },
      {
        key: 'speed',
        name: this.i18n.get('insight_job_speed_label'),
        hidden: true
      },
      {
        key: 'dataBeforeReduction',
        name: this.i18n.get('insight_job_databeforereduction_label'),
        hidden: true
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
        key: 'operation',
        width: 90,
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
    this.tableConfig = {
      table: {
        async: true,
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'jobId',
        columns: cols,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector'
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        trackByFn: (index, item) => {
          return item.jobId;
        }
      },
      pagination: {
        mode: 'simple',
        winTablePagination: true
      }
    };
    this.optsConfig = reject(opts, item =>
      includes(['export', 'remark'], item.id)
    );
  }
  getParams(filter, args) {
    const params = {
      startPage: filter.paginator.pageIndex + 1,
      pageSize: filter.paginator.pageSize,
      orderType: 'desc',
      orderBy: 'startTime',
      groupBackupJobId: this.groupJobInfo.jobId,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    if (!isEmpty(filter.conditions)) {
      const conditions = JSON.parse(filter.conditions);
      if (conditions.status) {
        assign(params, {
          statusList: this.getParamsStatusList(conditions)
        });
      }
      const others = omit(conditions, ['status']);
      if (!isEmpty(others)) {
        assign(params, {
          ...others
        });
      }
    }

    if (!!size(filter.orders)) {
      assign(params, {
        orderType: filter.sort.direction,
        orderBy: filter.sort.key
      });
    }

    return params;
  }

  getParamsStatusList(conditions) {
    const statusList = uniq(
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
    );
    return statusList;
  }

  getData(filter, args) {
    const params = this.getParams(filter, args);
    combineLatest([
      this.systemTimeService.getSystemTime(false),
      this.jobApiService.queryJobsUsingGET(params)
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
          const extendObject = isEmpty(item.extendStr)
            ? {}
            : JSON.parse(item.extendStr);
          const tag = get(extendObject, 'tag', void 0);
          !isNil(tag) && set(item, 'remark', tag);
          set(item, 'dataBeforeReduction', extendObject.dataBeforeReduction);
          return item;
        }),
        total: res[1].totalCount
      };
    });
  }

  stop(e) {
    this.jobTable.stop(e);
  }
  exportJob(items) {
    this.jobTable.exportJob(items);
  }
  setRemarks(items) {
    this.jobTable.setRemarks(items);
  }

  getDetail(job) {
    this.openDetail(job);
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

  openDetail(job, detectionCopy?) {
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.dataMapService.getLabel('Job_type', job.type),
        lvContent: JobDetailComponent,
        lvModalKey: 'subjobDetailModalKey',
        lvWidth: this.i18n.isEn
          ? MODAL_COMMON.largeWidth + 100
          : MODAL_COMMON.largeWidth,
        lvComponentParams: {
          job: {
            ...job,
            colors:
              job.status === DataMap.Job_status.aborting.value
                ? this.abortingColors
                : this.colors,
            getDuration: this.getDuration
          }
        },
        lvFooter: isArray(detectionCopy)
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
            ]
      })
    );
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
          extendObject.restoreType === RestoreV2Type.FileRestore
            ? `${this.i18n.get('common_restore_type_label', [], true)}${
                includes(
                  [
                    DataMap.Job_Target_Type.HBaseBackupSet.value,
                    DataMap.Job_Target_Type.ClickHouse.value,
                    DataMap.Job_Target_Type.HiveBackupSet.value,
                    DataMap.Job_Target_Type.dwsCluster.value,
                    DataMap.Job_Target_Type.dwsSchema.value,
                    DataMap.Job_Target_Type.dwsTable.value,
                    DataMap.Job_Target_Type.oceanBaseCluster.value
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
}
