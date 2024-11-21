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
import { Clipboard } from '@angular/cdk/clipboard';
import { DatePipe } from '@angular/common';
import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { Router } from '@angular/router';
import { MessageService } from '@iux/live';
import {
  CookieService,
  DataMap,
  DataMapService,
  GlobalService,
  I18NService,
  JobAPIService,
  MODAL_COMMON,
  RestoreV2LocationType,
  RetentionPolicy,
  RouterUrl,
  SchedulePolicy,
  SLA_BACKUP_NAME,
  SYSTEM_TIME,
  TaskService
} from 'app/shared';
import {
  LiveMountPolicyApiService,
  SlaApiService
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { SlaService } from 'app/shared/services/sla.service';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import {
  assign,
  chunk,
  cloneDeep,
  each,
  filter,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isUndefined,
  map as _map,
  split,
  union,
  values
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { map, switchMap, takeUntil } from 'rxjs/operators';
import { FeedbackResultComponent } from './feedback-result.component';
import { JobEventComponent } from './job-event/job-event.component';
import { JobStrategyComponent } from './job-strategy/job-strategy.component';
import { ReportResultComponent } from './report-result/report-result.component';

@Component({
  selector: 'aui-job-detail',
  templateUrl: './job-detail.component.html',
  styleUrls: ['./job-detail.component.less'],
  providers: [DatePipe]
})
export class JobDetailComponent implements OnInit, OnDestroy {
  time = ['1'];
  job;
  dataMap = DataMap;
  timeZone = SYSTEM_TIME.timeZone;
  jobForms = {};
  extendObject: any = {};
  triggerPolicy: any = {};
  dwsBackupGroupName = '';
  _parse = JSON.parse;
  schedulePolicy = SchedulePolicy;
  retentionPolicy = RetentionPolicy;
  _isEn = this.i18n.isEn;
  sourceSubType;
  _values = values;
  feedbackLog;
  showConfirmButton = false;
  feedbackResultComponent = FeedbackResultComponent;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
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
  isOceanProtect =
    this.isDataBackup ||
    this.appUtilsService.isDecouple ||
    this.appUtilsService.isDistributed;
  jobDestroy$ = new Subject();
  jobSubscription$ = new Subscription();
  snapshotRestore;
  isLiveMount = false;
  speedTipText = this.i18n.get('insight_job_running_speed_desc_label');
  detailJobType = [
    DataMap.Job_type.backup_job.value,
    DataMap.Job_type.copy_data_job.value,
    DataMap.Job_type.archive_job.value,
    DataMap.Job_type.live_mount_job.value
  ];
  isShowTab = true;
  restoreResourceType = [
    DataMap.Resource_Type.APSResourceSet.value,
    DataMap.Resource_Type.APSCloudServer.value,
    DataMap.Resource_Type.APSZone.value,
    DataMap.Resource_Type.HCSCloudHost.value,
    DataMap.Resource_Type.hyperV.value,
    DataMap.Resource_Type.virtualMachine.value,
    DataMap.Resource_Type.tdsqlDistributedInstance.value,
    DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
    DataMap.Resource_Type.oracle.value,
    DataMap.Resource_Type.oracleCluster.value,
    DataMap.Resource_Type.cNwareVm.value,
    DataMap.Resource_Type.MySQL.value,
    DataMap.Resource_Type.MySQLInstance.value,
    DataMap.Resource_Type.MySQLClusterInstance.value,
    DataMap.Resource_Type.MySQLDatabase.value,
    DataMap.Resource_Type.fileset.value,
    DataMap.Resource_Type.volume.value,
    DataMap.Resource_Type.dbTwoDatabase.value,
    DataMap.Resource_Type.dbTwoTableSet.value,
    DataMap.Resource_Type.SQLServerDatabase.value,
    DataMap.Resource_Type.PostgreSQLInstance.value,
    DataMap.Resource_Type.PostgreSQLClusterInstance.value,
    DataMap.Resource_Type.KingBaseInstance.value,
    DataMap.Resource_Type.KingBaseClusterInstance.value,
    DataMap.Resource_Type.FusionCompute.value,
    DataMap.Resource_Type.fusionOne.value,
    DataMap.Resource_Type.KubernetesStatefulset.value,
    DataMap.Resource_Type.openStackCloudServer.value,
    DataMap.Resource_Type.MongodbClusterInstance.value,
    DataMap.Resource_Type.MongodbSingleInstance.value,
    DataMap.Resource_Type.ExchangeSingle.value,
    DataMap.Resource_Type.ExchangeGroup.value,
    DataMap.Resource_Type.ExchangeDataBase.value
  ]; // 可以展示恢复高级参数的资源类型
  spaceLabel = this.i18n.isEn ? ' ' : '';
  executionPeriodLabel = this.i18n.get(
    'protection_execution_period_label',
    [],
    true
  );
  firstExecuteTimeLabel = this.i18n.get(
    'explore_first_execute_label',
    [],
    true
  );
  jumpDisable = false;
  infoUpdated = false;
  isManualBackup = false;
  newName;
  backupScenarioArr = [
    'kubernetes-container',
    'clickhouse',
    'dameng',
    'db2',
    'gaussdbdws',
    'gaussdbt',
    'generaldatabase',
    'informix',
    'kingbase',
    'mongodb',
    'mysql',
    'opengauss',
    'oralce',
    'postgresql',
    'sqlserver'
  ];
  restoreScenarioArr = [
    'kubernetes-container',
    'clickhouse',
    'dameng',
    'db2',
    'gaussdbdws',
    'gaussdbt',
    'lightcloudgaussdb',
    'generaldatabase',
    'goldendb',
    'informix',
    'kingbase',
    'mongodb',
    'mysql',
    'oceanbase',
    'opengauss',
    'oralce',
    'postgresql',
    'sqlserver',
    'tdsql',
    'tidb'
  ];
  currentRoute: string;

  @ViewChild('jobEvent', { static: false }) jobEvent: JobEventComponent;
  @ViewChild(JobStrategyComponent, { static: false })
  jobStrategyComponent: JobStrategyComponent;

  constructor(
    public router: Router,
    public slaService: SlaService,
    public slaApiService: SlaApiService,
    public globalService: GlobalService,
    private i18n: I18NService,
    private datePipe: DatePipe,
    private clipboard: Clipboard,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private jobApiService: JobAPIService,
    private messageService: MessageService,
    private systemTimeService: SystemTimeService,
    private drawModalService: DrawModalService,
    private taskService: TaskService,
    private appUtilsService: AppUtilsService,
    private liveMountPolicyApiService: LiveMountPolicyApiService
  ) {
    this.currentRoute = this.router.url;
  }

  ngOnDestroy() {
    this.jobDestroy$.next(true);
    this.jobDestroy$.complete();
  }

  ngOnInit() {
    this.showManualFeedback();
    this.getJob();
    if (!this.isOceanProtect) {
      this.isShowTab = false;
    }
    if (this.detailJobType.includes(this.job.type) && this.isOceanProtect) {
      if (
        !JSON.parse(this.job?.extendStr || '{}')?.triggerPolicy &&
        !JSON.parse(this.job?.extendStr || '{}')?.jobConfig
      ) {
        this.isShowTab = false;
      } else {
        if (
          [
            DataMap.Job_type.backup_job.value,
            DataMap.Job_type.copy_data_job.value
          ].includes(this.job.type)
        ) {
          this.isManualBackup = true;
        }
      }
      if (
        !this.restoreResourceType.includes(this.job.sourceSubType) &&
        this.job.type === DataMap.Job_type.restore_job.value
      ) {
        this.isShowTab = false;
      }
    }
    if (
      [
        DataMap.Job_type.backup_job.value,
        DataMap.Job_type.copy_data_job.value,
        DataMap.Job_type.archive_job.value
      ].includes(this.job.type) &&
      this.isOceanProtect
    ) {
      this.updateSla();
    }
    if (
      this.job.type === DataMap.Job_type.live_mount_job.value &&
      this.isOceanProtect
    ) {
      this.updateMountName();
    }
    this.snapshotRestore =
      this.isCyberEngine &&
      this.job.type === DataMap.Job_type.live_mount_job.value;
  }

  updateSla() {
    const sla = JSON.parse(this.job?.extendStr || '{}');
    if (!sla || !sla?.slaId) {
      this.jumpDisable = true;
      if (!isUndefined(this.jobStrategyComponent)) {
        this.jobStrategyComponent.isExist = false;
      }
      return;
    }
    this.slaApiService
      .querySLAUsingGET({
        slaId: sla.slaId,
        akDoException: false
      })
      .subscribe({
        next: res => {
          if (!res) {
            this.jumpDisable = true;
            if (!isUndefined(this.jobStrategyComponent)) {
              this.jobStrategyComponent.isExist = false;
            }
          } else if (res.name !== sla.slaName) {
            this.newName = res.name;
            this.infoUpdated = true;
            if (!isUndefined(this.jobStrategyComponent)) {
              this.jobStrategyComponent.slaName = this.newName;
              this.jobStrategyComponent.slaUpdated = true;
            }
          }
        },
        error: err => {
          this.jumpDisable = true;
          if (!isUndefined(this.jobStrategyComponent)) {
            this.jobStrategyComponent.isExist = false;
          }
        }
      });
  }

  updateMountName() {
    const mount = JSON.parse(this.job?.extendStr || '{}')?.triggerPolicy;
    if (!mount) {
      return;
    }
    this.liveMountPolicyApiService
      .getPolicyUsingGET({
        policyId: mount.policyId,
        akDoException: false
      })
      .subscribe({
        next: res => {
          if (res.name !== mount.name) {
            this.newName = res.name;
            this.infoUpdated = true;
          }
        },
        error: error => {
          this.jumpDisable = true;
        }
      });
  }

  getJob() {
    if (this.jobSubscription$) {
      this.jobSubscription$.unsubscribe();
    }
    this.jobSubscription$ = timer(0, 5 * 1e3)
      .pipe(
        switchMap(index => {
          return this.jobApiService
            .queryJobsUsingGET({
              jobId: this.job?.jobId,
              akLoading: !index
            })
            .pipe(
              map(res => {
                return first(res.records);
              })
            );
        }),
        takeUntil(this.jobDestroy$)
      )
      .subscribe(result => {
        this.job = {
          ...cloneDeep(this.job),
          ...result
        };
        if (
          this.job.sourceSubType ===
            DataMap.Resource_Type.tdsqlInstance.value &&
          this.job.type === DataMap.Job_type.live_mount_job.value
        ) {
          assign(this.job, {
            speed: null
          });
        }
        this.initJobForms();
      });
  }

  initJobForms() {
    this.sourceSubType = this.job.sourceSubType;
    const needChangeTipLabelArr = [];
    if (this.job.type === DataMap.Job_type.backup_job.value) {
      each(
        this.appUtilsService.findResourceTypeByKey(),
        (value: string[] | string, key) => {
          if (includes(this.backupScenarioArr, key)) {
            isArray(value)
              ? needChangeTipLabelArr.push(...value)
              : needChangeTipLabelArr.push(value);
          }
        }
      );
    } else if (this.job.type === DataMap.Job_type.restore_job.value) {
      each(
        this.appUtilsService.findResourceTypeByKey(),
        (value: string[] | string, key) => {
          if (includes(this.restoreScenarioArr, key)) {
            isArray(value)
              ? needChangeTipLabelArr.push(...value)
              : needChangeTipLabelArr.push(value);
          }
        }
      );
    }
    if (includes(needChangeTipLabelArr, this.job.sourceSubType)) {
      this.speedTipText = this.i18n.get(
        'insight_job_running_speed_spec_desc_label'
      );
    }
    this.extendObject = isEmpty(this.job.extendStr)
      ? {}
      : JSON.parse(this.job.extendStr);
    this.triggerPolicy = get(this.extendObject, 'triggerPolicy', {});
    // 若dws应用备份到开了并行存储开关的备份存储单元组上，则会有storageUnitGroupName字段
    this.dwsBackupGroupName = find(this.triggerPolicy?.policy_list, {
      type: 'backup'
    })?.ext_parameters?.storage_info?.storageUnitGroupName;
    this.jobForms = {
      basicInfo: {
        title: this.i18n.get('common_basic_info_label'),
        keys: this.getBasicKeys(),
        values: []
      },
      targetInfo: {
        title: this.i18n.get('insight_job_target_object_label'),
        keys: this.getTargetKeys(),
        values: []
      }
    };
    if (this.isManualBackup && this.isOceanProtect) {
      // 手动/自动触发在标题上标明
      let isManual =
        JSON.parse(this.job?.extendStr || '{}')?.executeType === 'MANUAL';
      let showLabel = isManual
        ? this.i18n.get('insight_job_manual_trigger_label')
        : this.i18n.get('insight_job_sla_trigger_label');
      this.jobForms['basicInfo'].title = `${this.i18n.get(
        'common_basic_info_label'
      )} (${showLabel})`;
    }
    // tslint:disable-next-line: forin
    for (const key in this.jobForms) {
      const array = this.jobForms[key];
      each(array.keys, prop => {
        this.jobForms[key]['values'].push({
          key: prop,
          value: this.getValue(prop, this.job[prop]),
          label: this.getLabel(prop)
        });
      });

      const countOfRows =
        this.job.type === DataMap.Job_type.backup_job.value &&
        this.isOceanProtect
          ? 6
          : 5;
      array.values = chunk(
        array.values,
        key === 'targetInfo' ? 2 : countOfRows
      );
    }
  }

  getValue(key, value) {
    if (
      this.isCyberEngine &&
      key === 'sourceSubType' &&
      (value === 'NasFileSystem' || value === 'CloudBackupFileSystem')
    ) {
      value = 'FileSystem';
    }
    const backupType =
      this.extendObject['sourceCopyType'] || this.extendObject['backupType'];
    switch (key) {
      case 'startTime':
      case 'endTime':
      case 'copyTime':
        value = this.datePipe.transform(
          value,
          'yyyy/MM/dd HH:mm:ss',
          this.timeZone
        );
        break;
      case 'type':
        value = this.dataMapService.getLabel('Job_type', value);
        break;
      case 'backupType':
        value = this.i18n.get(SLA_BACKUP_NAME[backupType]);
        if (
          [
            DataMap.Resource_Type.FusionCompute.value,
            DataMap.Resource_Type.fusionOne.value,
            DataMap.Resource_Type.HCSCloudHost.value,
            DataMap.Resource_Type.openStackCloudServer.value,
            DataMap.Resource_Type.tdsqlInstance.value,
            DataMap.Resource_Type.hyperVVm.value
          ].includes(this.sourceSubType)
        ) {
          if (backupType === 'difference_increment') {
            value = this.i18n.get('protection_incremental_forever_full_label');
          }
        }
        if (
          includes(
            [
              DataMap.Resource_Type.virtualMachine.value,
              DataMap.Resource_Type.clusterComputeResource.value,
              DataMap.Resource_Type.hostSystem.value,
              DataMap.Resource_Type.APSCloudServer.value,
              DataMap.Resource_Type.APSResourceSet.value,
              DataMap.Resource_Type.APSZone.value
            ],
            this.sourceSubType
          ) &&
          backupType === 'difference_increment'
        ) {
          value = this.i18n.get('common_permanent_backup_label');
        }
        break;
      case 'sourceSubType':
        value =
          this.cookieService.isCloudBackup &&
          this.i18n.get('deploy_type') !== DataMap.Deploy_Type.hyperdetect.value
            ? this.i18n.get('common_backup_storage_label')
            : includes(
                [
                  DataMap.Resource_Type.FusionCompute.value,
                  DataMap.Resource_Type.fusionOne.value
                ],
                value
              ) &&
              !isEmpty(this.job.sourceType) &&
              this.job.sourceType !== 'VM'
            ? this.dataMapService.getLabel(
                'Job_Target_Type',
                `${this.job.sourceType}__and__${value}`
              )
            : this.dataMapService.getLabel('Job_Target_Type', value);

        break;
      case 'durationTime':
        const systemTime = this.appUtilsService.getCurrentSysTime();
        value = this.job.getDuration(
          includes(
            [
              DataMap.Job_status.running.value,
              DataMap.Job_status.initialization.value,
              DataMap.Job_status.pending.value,
              DataMap.Job_status.aborting.value
            ],
            this.job.status
          )
            ? systemTime - this.job.startTime < 0
              ? 0
              : systemTime - this.job.startTime
            : this.job.endTime
            ? this.job.endTime - this.job.startTime
            : 0
        );
        break;
      case 'dataBeforeReduction':
      case 'dataAfterReduction':
        value = this.extendObject[key];
        break;
      case 'targetLocation':
        value = this.cookieService.isCloudBackup
          ? this.isHyperdetect
            ? `${this.job.targetLocation || ''}`
            : `${this.job.targetName || ''}${this.job.targetLocation || ''}`
          : this.dataMapService.getLabel(
              'Job_Target_Type',
              this.job.sourceSubType
            ) === DataMap.Job_Target_Type.oracle.label
          ? value || this.job.targetName
          : value;
        break;
      case 'slaName':
        value = this.extendObject['slaName'];
        break;
      case 'storageUnitName':
        value = this.getStorageUnitName();
        break;
      case 'mountName':
        value = this.triggerPolicy.name;
        break;
      case 'copyRange':
        value = this.dataMapService.getLabel(
          'CopyData_Selection_Policy',
          this.triggerPolicy.copyDataSelectionPolicy
        );
        break;
      case 'retentionPolicy':
        value = this.triggerPolicy.schedulePolicy;
        break;
      case 'retentionTime':
        value = this.triggerPolicy.retentionPolicy;
        break;
      case 'exerciseName':
        value = this.triggerPolicy.name;
        break;
      case 'exercisePeriod':
        value = this.triggerPolicy.interval;
        break;
      case 'copyType':
        value = this.extendObject?.backupType;
        break;
      default:
        break;
    }
    return value;
  }

  getStorageUnitName() {
    if (this.job.type === DataMap.Job_type.copy_data_job.value) {
      return this.extendObject['rep_destination'] || this.job.unitName;
    }
    if (
      this.job.type === DataMap.Job_type.backup_job.value &&
      this.dwsBackupGroupName
    ) {
      return this.dwsBackupGroupName;
    }
    return this.job.unitName;
  }

  getLabel(key) {
    let label = this.i18n.get(`insight_job_${key.toLowerCase()}_label`);
    // 区分显示“备份存储单元”和“备份存储单元组”
    if (
      key === 'storageUnitName' &&
      ((this.extendObject['rep_destination'] &&
        this.job.type === DataMap.Job_type.copy_data_job.value &&
        find(this.extendObject['triggerPolicy']?.policy_list, {
          type: 'replication'
        })?.ext_parameters?.storage_info?.storage_type ===
          DataMap.storagePoolBackupStorageType.group.value) ||
        (this.job.type === DataMap.Job_type.backup_job.value &&
          this.dwsBackupGroupName))
    ) {
      label = this.i18n.get('system_backup_storage_unit_group_label');
      return label;
    }

    if (key !== 'targetLocation') {
      return label;
    }

    switch (this.job.type) {
      case DataMap.Job_type.restore_job.value:
      case DataMap.Job_type.live_restore_job.value:
        label = this.i18n.get('protection_restore_target_label');
        break;
      case DataMap.Job_type.live_mount_job.value:
        label = this.i18n.get('insight_job_live_mount_target_label');
        break;
      case DataMap.Job_type.copy_data_job.value:
        label = this.i18n.get('insight_job_replication_target_label');
        break;
      case DataMap.Job_type.archive_import_job.value:
        label = this.i18n.get('insight_job_import_target_label');
        break;
      case DataMap.Job_type.migrate.value:
        label = this.i18n.get('insight_job_migration_target_label');
        break;
      case DataMap.Job_type.archive_job.value:
        label = this.i18n.get('insight_job_archive_target_label');
        break;
      default:
        break;
    }
    return label;
  }

  getBasicKeys() {
    let left = ['jobId', 'status', 'startTime', 'endTime', 'durationTime'];
    let right = [];

    switch (this.job.type) {
      case DataMap.Job_type.backup_job.value:
        // 非OP不显示备份存储单元
        if (this.isOceanProtect) {
          left = [...left, 'storageUnitName'];
        }

        if (this.cookieService.isCloudBackup) {
          right =
            this.isHyperdetect &&
            includes(
              [
                DataMap.Resource_Type.LocalFileSystem.value,
                DataMap.Resource_Type.LocalLun.value
              ],
              this.job.sourceSubType
            )
              ? ['copyTime', 'backupType']
              : ['copyTime', 'backupType', 'speed'];
        } else if (this.isCyberEngine) {
          right = ['copyTime', 'backupType'];
        } else {
          right = [
            'copyTime',
            'backupType',
            'speed',
            'dataBeforeReduction',
            'slaName'
          ];
        }
        break;
      case DataMap.Job_type.live_mount_job.value:
        right =
          this.isOceanProtect && !isEmpty(this.extendObject)
            ? [
                'targetLocation',
                'mountName',
                'copyRange',
                'retentionPolicy',
                'retentionTime'
              ]
            : ['targetLocation'];
        break;
      case DataMap.Job_type.live_restore_job.value:
        right = ['targetLocation'];
        break;
      case DataMap.Job_type.restore_job.value:
        right = ['targetLocation', 'speed'];
        break;
      case DataMap.Job_type.copy_data_job.value:
        right = [
          'targetLocation',
          'speed',
          'dataBeforeReduction',
          'storageUnitName'
        ];
        break;
      case DataMap.Job_type.archive_job.value:
        right = ['targetLocation', 'copyTime', 'speed', 'dataBeforeReduction'];
        break;
      case DataMap.Job_type.migrate.value:
        right = ['targetLocation'];
        break;
      case DataMap.Job_type.archive_import_job.value:
        right =
          this.i18n.get('deploy_type') === DataMap.Deploy_Type.a8000.value ||
          this.i18n.get('deploy_type') === DataMap.Deploy_Type.x8000.value
            ? ['targetLocation']
            : [];
        break;
      case DataMap.Job_type.exercise.value:
        right = ['exerciseName', 'exercisePeriod'];
        break;
      default:
        break;
    }

    if (
      includes(
        [DataMap.Resource_Type.NASFileSystem.value],
        this.job.sourceSubType
      ) &&
      get(JSON.parse(this.job.extendStr), 'restoreTargetLocation') ===
        RestoreV2LocationType.ORIGIN
    ) {
      right = filter(right, val => val !== 'speed');
    }

    return union(left, right);
  }

  getTargetKeys() {
    let keys = ['sourceName', 'sourceSubType', 'sourceLocation'];
    switch (this.job.type) {
      case DataMap.Job_type.restore_job.value:
      case DataMap.Job_type.live_mount_job.value:
      case DataMap.Job_type.copies_verify_job.value:
      case DataMap.Job_type.live_restore_job.value:
        keys = ['sourceName', 'sourceSubType', 'sourceLocation', 'copyTime'];
        break;
      case DataMap.Job_type.exercise.value:
        keys = ['sourceName', 'sourceSubType'];
        break;
      case DataMap.Job_type.delete_copy_job.value:
      case DataMap.Job_type.copyExpired.value:
        keys = this.isOceanProtect
          ? ['sourceName', 'sourceSubType', 'sourceLocation', 'copyType']
          : keys;
        break;
      case DataMap.Job_type.updateBackupServiceIp.value:
        keys = ['sourceName', 'sourceSubType'];
      default:
        break;
    }
    return keys;
  }

  feedbackResult() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'backupMessage',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-info-48',
        lvHeader: this.i18n.get('insight_feedback_restore_result_label'),
        lvContent: this.feedbackResultComponent,
        lvWidth: MODAL_COMMON.normalWidth,
        lvComponentParams: {
          askManualBackup: true,
          manualBackup: false
        },
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: false,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvAfterOpen: modal => {},
        lvCancel: modal => {},
        lvOk: modal => {
          return new Promise(resolve => {
            const component = modal.getContentComponent() as FeedbackResultComponent;
            const params = {
              taskId: this.job.jobId,
              status: component.result,
              agents: _map(
                split(first(this.feedbackLog?.logInfoParam), ','),
                item => {
                  return {
                    endpoint: item
                  };
                }
              )
            };
            this.taskService
              .DeliverTaskStatus({ deliverTaskReq: params })
              .subscribe({
                next: () => {
                  this.jobEvent.dataTable.fetchData();
                  resolve(true);
                },
                error: () => resolve(false)
              });
          });
        },
        lvAfterClose: {}
      }
    });
  }

  showManualFeedback(log?) {
    this.isLiveMount =
      log?.logInfo === 'nas_plugin_livemount_cyberengine_label';
    if (this.isLiveMount) {
      this.time = log?.logInfoParam;
    }
    this.showConfirmButton =
      this.job.type === DataMap.Job_type.restore_job.value &&
      this.job.sourceSubType === DataMap.Resource_Type.generalDatabase.value &&
      this.job.status === DataMap.Job_status.running.value &&
      !!log &&
      get(log, 'logInfo') === 'plugin_hana_systemdb_restore_label';
    this.feedbackLog = log;
  }

  report(item) {
    this.drawModalService.create({
      lvType: 'modal',
      lvHeader: this.i18n.get('common_report_jobs_result_title_label'),
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: ReportResultComponent,
      lvComponentParams: {
        id: this.job.associativeId,
        time: this.time
      },
      lvOk: modal => {
        const content = modal.getContentComponent() as ReportResultComponent;
        content.onOK().subscribe(() => {
          this.isLiveMount = false;
        });
      }
    });
  }

  copyJobId(value) {
    this.clipboard.copy(value);
    this.messageService.success(this.i18n.get('common_copy_success_label'), {
      lvShowCloseButton: true
    });
  }

  getSlaDetail() {
    // 跳转到sla并带筛选
    if (this.jumpDisable) {
      return;
    }
    this.appUtilsService.setCacheValue(
      'jobToSla',
      this.infoUpdated ? this.newName : this.extendObject.slaName
    );
    if (this.currentRoute === '/protection/policy/sla') {
      this.router.navigateByUrl('/protection/policy/sla');
      this.globalService.emitStore({
        action: 'jobToSla',
        state: true
      });
    } else {
      this.router.navigateByUrl('/protection/policy/sla');
    }
  }

  getMountDetail() {
    if (this.jumpDisable) {
      return;
    }
    this.appUtilsService.setCacheValue(
      'jobToMount',
      this.infoUpdated ? this.newName : this.triggerPolicy.name
    );
    this.router.navigateByUrl('/explore/policy/mount-update-policy');
  }

  getExerciseDetail() {
    this.appUtilsService.setCacheValue('jobToExercise', this.job?.exerciseId);
    this.router.navigateByUrl(RouterUrl.ExploreRecoveryDrill);
  }
}
