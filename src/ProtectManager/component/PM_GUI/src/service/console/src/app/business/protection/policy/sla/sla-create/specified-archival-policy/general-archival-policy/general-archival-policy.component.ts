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
import { ChangeDetectorRef, Component, Input, OnInit } from '@angular/core';
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService, OptionItem } from '@iux/live';
import { ArchiveStorageComponent } from 'app/business/system/infrastructure/archive-storage/archive-storage.component';
import { StoragePoolListComponent } from 'app/business/system/infrastructure/archive-storage/storage-pool-list/storage-pool-list.component';
import {
  ApplicationType,
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  MediaSetApiService,
  PolicyType,
  ProtectResourceAction,
  QosService,
  RouterUrl,
  StoragesApiService,
  WarningMessageService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  includes,
  isEmpty,
  isNil,
  isNumber,
  isUndefined,
  map,
  reject,
  set,
  size
} from 'lodash';
import { distinctUntilChanged } from 'rxjs';

@Component({
  selector: 'aui-general-archival-policy',
  templateUrl: './general-archival-policy.component.html',
  styleUrls: ['./general-archival-policy.component.less'],
  providers: [DatePipe]
})
export class GeneralArchivalPolicyComponent implements OnInit {
  qosNames = [];
  tapeStorageNames = [];
  s3StorageNames = [];
  storageNames = [];
  clusterNodeNames = [];
  mediaSetOptions = {};
  mediaSetData = [];
  language = this.i18n.language;
  applicationType = ApplicationType;
  archiveTargetType = DataMap.Archive_Target_Type;
  dataMap = DataMap;
  index = 1;
  curNodeInfo = {
    esn: '',
    clusterId: '',
    nodeName: ''
  };

  // 保留70年
  maxRetentionDay = 25550;
  maxRetentionWeek = 3650;
  maxRetentionMonth = 840;
  maxRetentionYear = 70;

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

  @Input() data: any;
  @Input() action: any;
  @Input() sla: any;
  @Input() activeIndex: number;
  @Input() application;
  @Input() formGroup: FormGroup;

  protocolOptions = this.dataMapService
    .toArray('Archival_Protocol')
    .filter((v: OptionItem) => {
      return (v.isLeaf = true);
    });
  intervalUnit = this.dataMapService
    .toArray('Interval_Unit')
    .filter((v: OptionItem) => {
      return v.value !== 'y' && v.value !== 'p' && v.value !== 'MO';
    })
    .filter((v: OptionItem) => {
      return (v.isLeaf = true);
    });
  retentionDurationUnit = this.dataMapService
    .toArray('Interval_Unit')
    .filter((v: OptionItem) => {
      return v.value !== 'm' && v.value !== 'h';
    })
    .filter((v: OptionItem) => {
      return (v.isLeaf = true);
    });
  archivingScopeOptions = this.dataMapService
    .toArray('Archive_Scope')
    .filter((v: OptionItem) => (v.isLeaf = true));
  yearTimeRangeOptions = this.dataMapService
    .toArray('Year_Time_Range')
    .filter((v: OptionItem) => (v.isLeaf = true));
  monthTimeRangeOptions = this.dataMapService
    .toArray('Month_Time_Range')
    .filter((v: OptionItem) => (v.isLeaf = true));
  weekTimeRangeOptions = this.dataMapService
    .toArray('Days_Of_Week')
    .filter((v: OptionItem) => (v.isLeaf = true));

  intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 23])
  });
  backupGenerationErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
  });
  retentionDurationErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [
      1,
      this.maxRetentionDay
    ])
  });
  startTimeErrorTip = assign({}, this.baseUtilService.requiredErrorTip);
  retryTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 3])
  });
  waitTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 30])
  });
  retentionDurationYearErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [
      1,
      this.maxRetentionYear
    ])
  };
  retentionDurationMonthErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [
      1,
      this.maxRetentionMonth
    ])
  };
  retentionDurationWeekErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [
      1,
      this.maxRetentionWeek
    ])
  };
  driverCountErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 32])
  });

  autoIndexForObs = false; // 对象存储下支持自动索引的应用
  autoIndexForTape = false; // 磁带库下支持自动索引的应用
  archiveLogCopy = false; // 归档日志副本
  ratePolicyRouterUrl = RouterUrl.ProtectionLimitRatePolicy;
  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private qosServiceApi: QosService,
    private cookieService: CookieService,
    private messageService: MessageService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private virtualScroll: VirtualScrollService,
    private storageApiService: StoragesApiService,
    private clusterApiService: ClustersApiService,
    private mediaSetApiService: MediaSetApiService,
    private batchOperateService: BatchOperateService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.getClusterNodes();
    this.updateForm();
    this.getQosNames();
    if (this.appUtilsService.isDistributed || this.appUtilsService.isDecouple) {
      this.protocolOptions = this.protocolOptions.filter(
        item => item.value !== DataMap.Archival_Protocol.tapeLibrary.value
      );
    }
    this.autoIndexForObs = includes(
      [
        ApplicationType.NASFileSystem,
        ApplicationType.NASShare,
        ApplicationType.HDFS,
        ApplicationType.ImportCopy,
        ApplicationType.Fileset
      ],
      this.application
    );
    this.archiveLogCopy = includes([ApplicationType.TDSQL], this.application);
    this.autoIndexForTape = includes(
      [
        ApplicationType.NASFileSystem,
        ApplicationType.NASShare,
        ApplicationType.Fileset,
        ApplicationType.ObjectStorage
      ],
      this.application
    );
  }

  getClusterNodes() {
    const cluster = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    ) || {
      clusterId: DataMap.Cluster_Type.local.value,
      clusterType: DataMap.Cluster_Type.local.value
    };
    const params = {
      clusterId: cluster.clusterId,
      startPage: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      roleList: [
        DataMap.Target_Cluster_Role.primaryNode.value,
        DataMap.Target_Cluster_Role.backupNode.value,
        DataMap.Target_Cluster_Role.memberNode.value
      ]
    };

    this.clusterApiService
      .getClustersInfoUsingGET(params)
      .subscribe((res: any) => {
        const nodes = map(res.records, item => {
          return {
            ...item,
            key: item.clusterId,
            value: item.clusterId,
            label: item.clusterName,
            isLeaf: true
          };
        });

        this.clusterNodeNames = nodes;
        this.updateData();
        this.cdr.detectChanges();
      });
  }

  policyIndexChange(e) {
    this.getStorageNames(this.getArchiveTeams().controls[e].value.protocol);
  }

  addStorage(type) {
    if (type === DataMap.Archival_Protocol.objectStorage.value) {
      this.addS3Storage();
    } else {
      this.addMediaSet(this.curNodeInfo);
    }
  }

  addS3Storage() {
    const archiveStorageComponent = new ArchiveStorageComponent(
      this.appUtilsService,
      this.i18n,
      this.drawModalService,
      this.baseUtilService,
      this.warningMessageService,
      this.storageApiService,
      this.dataMapService,
      this.cookieService
    );
    archiveStorageComponent.createArchiveStorage(() =>
      this.getS3StorageNames()
    );
  }

  addMediaSet(data?) {
    const node = {
      clusterId: data?.clusterId || '',
      remoteEsn: data?.esn || '',
      clusterName: data?.nodeName || ''
    };
    const storagePoolListComponent = new StoragePoolListComponent(
      this.i18n,
      this.cdr,
      this.dataMapService,
      this.mediaSetApiService,
      this.batchOperateService,
      this.warningMessageService,
      this.drawModalService,
      this.virtualScroll,
      this.cookieService
    );
    storagePoolListComponent.node = node;
    storagePoolListComponent.createPool(() => this.getTapeStorageNames(node));
  }

  updateForm() {
    this.formGroup.addControl(
      'archiveTeams',
      this.fb.array([this.archiveTeam])
    );
  }

  get archiveTeam(): FormGroup {
    const archiveTeam: FormGroup = this.fb.group({
      uuid: new FormControl(''),
      name: new FormControl(
        this.i18n.get('common_archive_params_label', ['01']),
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name()
          ],
          updateOn: 'change'
        }
      ),
      protocol: new FormControl(DataMap.Archival_Protocol.objectStorage.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      storage_id: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      node_id: new FormControl(''),
      mediaSet: new FormControl(''),
      esn: new FormControl(''),
      qos_id: new FormControl(''),
      trigger: new FormControl(DataMap.Archive_Trigger.periodArchive.value),
      interval: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 23)
        ],
        updateOn: 'change'
      }),
      interval_unit: new FormControl(DataMap.Interval_Unit.hour.value),
      start_time: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      backup_generation: new FormControl(''),
      archiving_scope: new FormControl(DataMap.Archive_Scope.latest.value),
      retention_duration: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.maxRetentionDay)
        ],
        updateOn: 'change'
      }),
      duration_unit: new FormControl(DataMap.Interval_Unit.day.value),
      driverCount: new FormControl(1),
      auto_index: new FormControl(false),
      network_access: new FormControl(true),
      alarm_after_failure: new FormControl(true),
      log_archive: new FormControl(false),
      auto_retry: new FormControl(true),
      auto_retry_times: new FormControl(3, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 3)
        ],
        updateOn: 'change'
      }),
      auto_retry_wait_minutes: new FormControl(5, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 30)
        ],
        updateOn: 'change'
      }),
      archive_target_type: new FormControl(
        this.archiveTargetType.archiveAllCopies.value
      ),
      copy_type_year: new FormControl(true),
      generate_time_range_year: new FormControl(
        DataMap.Year_Time_Range.December.value
      ),
      retention_duration_year: new FormControl(''),
      copy_type_month: new FormControl(true),
      generate_time_range_month: new FormControl(
        DataMap.Month_Time_Range.first.value
      ),
      retention_duration_month: new FormControl(''),
      copy_type_week: new FormControl(true),
      generate_time_range_week: new FormControl(DataMap.Days_Of_Week.mon.value),
      retention_duration_week: new FormControl(''),
      delete_import_copy: new FormControl(false)
    });
    this.listenFormGroup(archiveTeam);
    archiveTeam
      .get('protocol')
      .setValue(DataMap.Archival_Protocol.objectStorage.value);
    return archiveTeam;
  }

  getArchiveTeams() {
    return this.formGroup.get('archiveTeams') as FormArray;
  }

  addArchiveTeam() {
    const archiveTeam = cloneDeep(this.archiveTeam);
    const controls = this.getArchiveTeams().controls;
    const nameArr = map(controls, item => item.value.name);

    while (
      includes(
        nameArr,
        this.i18n.get('common_archive_params_label', [`0${this.index}`])
      )
    ) {
      this.index++;
    }
    archiveTeam.patchValue({
      name: this.i18n.get('common_archive_params_label', [`0${this.index++}`])
    });
    this.listenFormGroup(archiveTeam);
    this.getArchiveTeams().push(archiveTeam);
    this.activeIndex = this.getArchiveTeams().controls.length - 1;
  }

  removeArchiveTeam(tab) {
    const controls = this.getArchiveTeams().controls;
    for (let index = 0; index < controls.length; index++) {
      if (tab.lvId === index) {
        this.getArchiveTeams().removeAt(index);
      }
    }
    this.activeIndex = 0;
  }

  checkDepRelationship(id) {
    return !!size(
      reject(this.getArchiveTeams().value, obj => {
        return (
          obj.name === this.i18n.get('common_archive_params_label', [`0${id}`])
        );
      })
    );
  }

  listenFormGroup(archiveTeam: FormGroup) {
    this.checkTrigger(archiveTeam);
    this.checkStorageType(archiveTeam);
    this.checkTargetType(archiveTeam);
    this.checkAutoRetry(archiveTeam);
    this.checkClusterNode(archiveTeam);
  }

  checkClusterNode(archiveTeam: FormGroup) {
    archiveTeam
      .get('node_id')
      .valueChanges.pipe(distinctUntilChanged())
      .subscribe(res => {
        if (!res) {
          this.curNodeInfo = {
            esn: '',
            clusterId: '',
            nodeName: ''
          };
          return;
        }

        archiveTeam.get('mediaSet').setValue('');
        archiveTeam.get('mediaSet').updateValueAndValidity();
        const cluster = find(this.clusterNodeNames, { value: res });
        this.curNodeInfo = {
          esn: cluster?.storageEsn,
          clusterId: cluster?.clusterId,
          nodeName: cluster?.clusterName
        };
        archiveTeam.get('esn').setValue(cluster?.storageEsn);
        if (res && isEmpty(this.mediaSetOptions[res])) {
          const tmpNode = find(this.clusterNodeNames, { value: res });
          this.getTapeStorageNames(tmpNode);
        }
      });
  }

  checkTrigger(archiveTeam: FormGroup) {
    archiveTeam.get('trigger').valueChanges.subscribe(res => {
      this.dealTrigger(archiveTeam, res);
    });
    this.dealTrigger(archiveTeam, archiveTeam.get('trigger').value);
  }

  checkStorageType(archiveTeam: FormGroup) {
    archiveTeam.get('protocol').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      if (res === DataMap.Archival_Protocol.objectStorage.value) {
        archiveTeam
          .get('storage_id')
          .setValidators([this.baseUtilService.VALID.required()]);
        archiveTeam.get('driverCount').clearValidators();
        archiveTeam.get('node_id').clearValidators();
        archiveTeam.get('mediaSet').clearValidators();
      } else {
        archiveTeam
          .get('node_id')
          .setValidators([this.baseUtilService.VALID.required()]);
        archiveTeam
          .get('driverCount')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 32)
          ]);
        archiveTeam.get('storage_id').clearValidators();
        archiveTeam
          .get('mediaSet')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      archiveTeam.get('storage_id').setValue('');
      archiveTeam.get('storage_id').updateValueAndValidity();
      archiveTeam.get('node_id').updateValueAndValidity();
      archiveTeam.get('mediaSet').updateValueAndValidity();
      archiveTeam.get('driverCount').updateValueAndValidity();
      this.dealStorageType(archiveTeam, res);
    });
  }

  dealStorageType(archiveTeam: FormGroup, storageType) {
    if (storageType === DataMap.Archival_Protocol.objectStorage.value) {
      archiveTeam.get('retention_duration').enable();
      archiveTeam
        .get('duration_unit')
        .setValue(DataMap.Interval_Unit.day.value);
    } else if (storageType === DataMap.Archival_Protocol.tapeLibrary.value) {
      archiveTeam.get('qos_id').setValue('');
      archiveTeam
        .get('duration_unit')
        .setValue(DataMap.Interval_Unit.persistent.value);
    }
    archiveTeam.get('storage_id').setValue('');
    archiveTeam.get('storage_id').updateValueAndValidity();
    this.getStorageNames(storageType);
    archiveTeam.get('retention_duration').updateValueAndValidity();
    archiveTeam.get('copy_type_week').updateValueAndValidity();
    archiveTeam.get('copy_type_month').updateValueAndValidity();
    archiveTeam.get('copy_type_year').updateValueAndValidity();
  }

  checkTargetType(archiveTeam: FormGroup) {
    archiveTeam.get('archive_target_type').valueChanges.subscribe(res => {
      this.dealTargetType(archiveTeam, res);
    });
    this.dealTargetType(
      archiveTeam,
      archiveTeam.get('archive_target_type').value
    );
  }

  checkAutoRetry(archiveTeam: FormGroup) {
    archiveTeam.get('auto_retry').valueChanges.subscribe(res => {
      this.dealAutoRetry(archiveTeam, res);
    });
    this.dealAutoRetry(archiveTeam, archiveTeam.get('auto_retry').value);
  }

  dealTrigger(archiveTeam: FormGroup, trigger) {
    if (trigger === DataMap.Archive_Trigger.periodArchive.value) {
      archiveTeam.get('backup_generation').clearValidators();
      archiveTeam
        .get('interval')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 23)
        ]);
      archiveTeam
        .get('start_time')
        .setValidators([this.baseUtilService.VALID.required()]);
    } else if (trigger === DataMap.Archive_Trigger.immediatelyBackup.value) {
      archiveTeam.get('interval').clearValidators();
      archiveTeam.get('start_time').clearValidators();
      archiveTeam.get('backup_generation').clearValidators();
    } else if (trigger === DataMap.Archive_Trigger.archiveSpecifiedTime.value) {
      archiveTeam.get('interval').clearValidators();
      archiveTeam.get('start_time').clearValidators();
      archiveTeam
        .get('backup_generation')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 365)
        ]);
    }
    archiveTeam.get('interval').updateValueAndValidity();
    archiveTeam.get('start_time').updateValueAndValidity();
    archiveTeam.get('backup_generation').updateValueAndValidity();
  }

  dealTargetType(archiveTeam: FormGroup, archiveTargetType) {
    if (archiveTargetType === this.archiveTargetType.archiveAllCopies.value) {
      archiveTeam.get('generate_time_range_year').clearValidators();
      archiveTeam.get('retention_duration_year').clearValidators();
      archiveTeam.get('generate_time_range_month').clearValidators();
      archiveTeam.get('retention_duration_month').clearValidators();
      archiveTeam.get('generate_time_range_week').clearValidators();
      archiveTeam.get('retention_duration_week').clearValidators();
      if (
        archiveTeam.value.protocol ===
        DataMap.Archival_Protocol.tapeLibrary.value
      ) {
        archiveTeam
          .get('duration_unit')
          .setValue(DataMap.Interval_Unit.persistent.value);
      } else {
        archiveTeam
          .get('retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionDay)
          ]);
      }
    } else if (
      archiveTargetType === this.archiveTargetType.specifiedDate.value
    ) {
      archiveTeam.get('retention_duration').clearValidators();
      if (
        this.action === ProtectResourceAction.Modify &&
        !archiveTeam.value.copy_type_year &&
        !archiveTeam.value.copy_type_month &&
        !archiveTeam.value.copy_type_week
      ) {
        archiveTeam.get('copy_type_year').setValue(true);
        archiveTeam.get('copy_type_month').setValue(true);
        archiveTeam.get('copy_type_week').setValue(true);
      }
      this.checkCopyTypeYear(archiveTeam);
      this.checkCopyTypeMonth(archiveTeam);
      this.checkCopyTypeWeek(archiveTeam);
    }
    archiveTeam.get('retention_duration').updateValueAndValidity();
    archiveTeam.get('generate_time_range_year').updateValueAndValidity();
    archiveTeam.get('retention_duration_year').updateValueAndValidity();
    archiveTeam.get('generate_time_range_month').updateValueAndValidity();
    archiveTeam.get('retention_duration_month').updateValueAndValidity();
    archiveTeam.get('generate_time_range_week').updateValueAndValidity();
    archiveTeam.get('retention_duration_week').updateValueAndValidity();
  }

  checkCopyTypeWeek(archiveTeam: FormGroup) {
    archiveTeam.get('copy_type_week').valueChanges.subscribe(res => {
      this.dealCopyTypeWeek(archiveTeam, res);
    });
    this.dealCopyTypeWeek(archiveTeam, archiveTeam.get('copy_type_week').value);
  }

  dealCopyTypeWeek(archiveTeam: FormGroup, copyTypeWeek) {
    if (
      !copyTypeWeek ||
      archiveTeam.value.protocol === DataMap.Archival_Protocol.tapeLibrary.value
    ) {
      archiveTeam.get('retention_duration_week').clearValidators();
    } else {
      archiveTeam
        .get('retention_duration_week')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.maxRetentionWeek)
        ]);
    }

    archiveTeam.get('retention_duration_week').updateValueAndValidity();
  }

  checkCopyTypeMonth(archiveTeam: FormGroup) {
    archiveTeam.get('copy_type_month').valueChanges.subscribe(res => {
      this.dealCopyTypeMonth(archiveTeam, res);
    });
    this.dealCopyTypeMonth(
      archiveTeam,
      archiveTeam.get('copy_type_month').value
    );
  }

  dealCopyTypeMonth(archiveTeam: FormGroup, copyTypeMonth) {
    if (
      !copyTypeMonth ||
      archiveTeam.value.protocol === DataMap.Archival_Protocol.tapeLibrary.value
    ) {
      archiveTeam.get('retention_duration_month').clearValidators();
    } else {
      archiveTeam
        .get('retention_duration_month')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.maxRetentionMonth)
        ]);
    }

    archiveTeam.get('retention_duration_month').updateValueAndValidity();
  }

  checkCopyTypeYear(archiveTeam: FormGroup) {
    archiveTeam.get('copy_type_year').valueChanges.subscribe(res => {
      this.dealCopyTypeYear(archiveTeam, res);
    });
    this.dealCopyTypeYear(archiveTeam, archiveTeam.get('copy_type_year').value);
  }

  dealCopyTypeYear(archiveTeam: FormGroup, copyTypeYear) {
    if (
      !copyTypeYear ||
      archiveTeam.value.protocol === DataMap.Archival_Protocol.tapeLibrary.value
    ) {
      archiveTeam.get('retention_duration_year').clearValidators();
    } else {
      archiveTeam
        .get('retention_duration_year')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.maxRetentionYear)
        ]);
    }

    archiveTeam.get('retention_duration_year').updateValueAndValidity();
  }

  dealAutoRetry(archiveTeam: FormGroup, autoRetry) {
    if (!autoRetry) {
      archiveTeam.get('auto_retry_times').clearValidators();
      archiveTeam.get('auto_retry_wait_minutes').clearValidators();
    } else {
      archiveTeam
        .get('auto_retry_times')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 3)
        ]);
      archiveTeam
        .get('auto_retry_wait_minutes')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 30)
        ]);
    }

    archiveTeam.get('auto_retry_times').updateValueAndValidity();
    archiveTeam.get('auto_retry_wait_minutes').updateValueAndValidity();
  }

  updateData() {
    if (!this.data || !size(this.data)) {
      return;
    }

    this.getArchiveTeams().clear();
    each(this.data, item => {
      const archiveTeam = cloneDeep(this.archiveTeam);
      this.listenFormGroup(archiveTeam);
      archiveTeam.patchValue(item);
      if (isNil(item?.driverCount)) {
        archiveTeam.get('driverCount').setValue(1);
      }
      if (item.storage_list) {
        const arr = [];
        const mediaArr = [];
        map(item.storage_list, v => {
          const nodeId = find(this.clusterNodeNames, { storageEsn: v.esn })[
            'clusterId'
          ];
          const nodeName = find(this.clusterNodeNames, { storageEsn: v.esn })[
            'clusterName'
          ];
          mediaArr.push({
            esn: v.esn,
            clusterId: nodeId,
            nodeName: nodeName,
            mediaSet: v.storage_id
          });
          arr.push(nodeId);
        });
        archiveTeam.get('node_id').setValue(arr[0]);
        this.curNodeInfo = mediaArr[0];
        archiveTeam.get('mediaSet').setValue(mediaArr[0].mediaSet);
      }

      this.dealUnit(archiveTeam);
      this.getArchiveTeams().push(archiveTeam);
    });

    this.batchDealCtrl();
  }

  batchDealCtrl() {
    this.getArchiveTeams().controls.map(control => {
      if (
        control.get('duration_unit').value ===
        DataMap.Interval_Unit.persistent.value
      ) {
        control.get('retention_duration').disable();
        control.get('retention_duration').setValue('');
      }
    });
  }

  dealUnit(archiveTeam: FormGroup) {
    if (
      archiveTeam.get('trigger').value ===
      this.dataMap.Archive_Trigger.periodArchive.value
    ) {
      const interval_unit = archiveTeam.get('interval_unit').value;
      if (interval_unit) {
        this.changeTimeUnits(archiveTeam, interval_unit, 'interval');
      }
    }

    if (
      archiveTeam.get('archive_target_type').value ===
      this.archiveTargetType.archiveAllCopies.value
    ) {
      const duration_unit = archiveTeam.get('duration_unit').value;
      if (duration_unit) {
        this.changeTimeUnits(archiveTeam, duration_unit, 'retention_duration');
      }
    }
  }

  getQosNames() {
    this.qosServiceApi
      .queryResourcesV1QosGet({
        pageNo: 0,
        pageSize: 100
      })
      .subscribe(res => {
        this.qosNames = map(res.items, (item: any) => {
          item['isLeaf'] = true;
          item['label'] = item.name;
          return item;
        });
      });
  }

  getStorageNames(storageType) {
    if (storageType === DataMap.Archival_Protocol.objectStorage.value) {
      this.getS3StorageNames();
    }
  }

  getTapeStorageNames(node, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      memberEsn: node?.storageEsn || node?.remoteEsn
    };
    this.mediaSetApiService.getMediaSetAllUsingGET(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START + 1;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage ===
          Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) + 1 ||
        res.totalCount === 0
      ) {
        const storageNames = [];
        each(recordsTemp, item => {
          storageNames.push({
            ...item,
            isLeaf: true,
            label: item.mediaSetName,
            value: item.mediaSetId
          });
        });
        if (!node) {
          this.tapeStorageNames = storageNames;
        } else {
          set(this.mediaSetOptions, node?.clusterId, storageNames);
        }

        return;
      }
      this.getTapeStorageNames(node, recordsTemp, startPage);
    });
  }

  getMediaRetentionTip(options, v) {
    const mediaSet = find(options, item => item.mediaSetId === v);
    if (isUndefined(mediaSet)) {
      return;
    }
    const retentionLabel = this.dataMapService.getLabel(
      'Tape_Retention_Type',
      mediaSet.retentionType
    );
    const retentionUnit = this.dataMapService.getLabel(
      'Tape_Retention_Unit',
      mediaSet.retentionUnit
    );
    if (
      mediaSet.retentionType !== DataMap.Tape_Retention_Type.temporary.value
    ) {
      return this.i18n.get('protection_archive_media_set_retention_tip_label', [
        retentionLabel
      ]);
    } else {
      let resultLabel = this.i18n.get(
        'protection_archive_media_set_retention_tip_label',
        [`${retentionLabel}（${mediaSet.retentionDuration + retentionUnit}）`]
      );
      if (this.i18n.isEn) {
        return resultLabel.replace('（', '(').replace('）', ')');
      }
      return resultLabel;
    }
  }

  getS3StorageNames() {
    this.storageApiService
      .storageUsingGET({
        startPage: 0,
        pageSize: 200
      })
      .subscribe(res => {
        if (!size(res.records)) {
          this.storageNames = [];
        }
        res.records = filter(res.records, item => {
          return item.status === DataMap.Archive_Storage_Status.online.value;
        });
        this.s3StorageNames = map(res.records, item => {
          return assign(item, {
            isLeaf: true,
            label: item.name,
            value: item.repositoryId
          });
        });
        if (!!this.data.length) {
          const tempTeams = this.getArchiveTeams() as FormArray;
          each(tempTeams.controls, item => {
            if (
              item.value?.storage_id &&
              !find(
                this.s3StorageNames,
                val => val.value === item.value?.storage_id
              )
            ) {
              item.get('storage_id').setValue('');
            }
          });
        }
      });
  }

  checkAutoIndexDisabled(uuid) {
    return (
      this.action === ProtectResourceAction.Modify &&
      includes(
        map(
          filter(this.sla.policy_list, po => po.type === PolicyType.ARCHIVING),
          'uuid'
        ),
        uuid
      )
    );
  }

  changeTimeUnits(formGroup, value, formControlName) {
    formGroup.get(formControlName).enable();
    if (value === DataMap.Interval_Unit.minute.value) {
      formGroup
        .get(formControlName)
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 59)
        ]);
      this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 59])
      });
    } else if (value === DataMap.Interval_Unit.hour.value) {
      formGroup
        .get(formControlName)
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 23)
        ]);
      this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 23])
      });
    } else if (value === DataMap.Interval_Unit.day.value) {
      if ('retention_duration' === formControlName) {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionDay)
          ]);
        this.retentionDurationErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [
              1,
              this.maxRetentionDay
            ])
          }
        );
      } else {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 7)
          ]);
        this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 7])
        });
      }
    } else if (value === DataMap.Interval_Unit.week.value) {
      if ('retention_duration' === formControlName) {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionWeek)
          ]);
        this.retentionDurationErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [
              1,
              this.maxRetentionWeek
            ])
          }
        );
      } else {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 4)
          ]);
        this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 4])
        });
      }
    } else if (value === DataMap.Interval_Unit.month.value) {
      if ('retention_duration' === formControlName) {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionMonth)
          ]);
        this.retentionDurationErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [
              1,
              this.maxRetentionMonth
            ])
          }
        );
      } else {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 12)
          ]);
        this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 12])
        });
      }
    } else if (value === DataMap.Interval_Unit.year.value) {
      if ('retention_duration' === formControlName) {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionYear)
          ]);
        this.retentionDurationErrorTip = assign(
          {},
          this.baseUtilService.rangeErrorTip,
          {
            invalidRang: this.i18n.get('common_valid_rang_label', [
              1,
              this.maxRetentionYear
            ])
          }
        );
      } else {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 5)
          ]);
        this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 5])
        });
      }
    } else if (value === DataMap.Interval_Unit.persistent.value) {
      formGroup.get(formControlName).disable();
      formGroup.get(formControlName).setValue('');
    }
    formGroup.get(formControlName).updateValueAndValidity();
  }
}
