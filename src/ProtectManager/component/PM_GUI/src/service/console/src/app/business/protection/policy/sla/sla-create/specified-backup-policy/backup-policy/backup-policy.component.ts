import { DatePipe } from '@angular/common';
import { Component, Input, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MenuItem, MessageService, ModalRef } from '@iux/live';
import { ArchiveStorageComponent } from 'app/business/system/infrastructure/archive-storage/archive-storage.component';
import {
  ApplicationType,
  BaseUtilService,
  ClustersApiService,
  CookieService,
  DataMap,
  DataMapService,
  DaysOfType,
  defaultWindowTime,
  I18NService,
  PolicyAction,
  ProtectResourceAction,
  ScheduleTrigger,
  StoragesApiService,
  WarningMessageService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  get,
  includes,
  isEmpty,
  isUndefined,
  map,
  size,
  split,
  toString,
  uniq
} from 'lodash';

@Component({
  selector: 'aui-backup-policy',
  templateUrl: './backup-policy.component.html',
  styleUrls: ['./backup-policy.component.less']
})
export class BackupPolicyComponent implements OnInit {
  onlyIncrement = false;
  dateItems = [];
  weekItems = [];
  storageNames = [];
  protectResourceAction = ProtectResourceAction;
  appType = ApplicationType;
  monthDaysItems = [];
  optsItems: MenuItem[];
  daysOfType = DaysOfType;
  policyAction = PolicyAction;
  hbaseBackupType = DataMap.Hbase_Backup_Type;
  daysOfMonthType = DataMap.Days_Of_Month_Type;
  intervalUnit = DataMap.Interval_Unit;
  scheduleTrigger = ScheduleTrigger;
  _includes = includes;
  slaBackupName = {
    full: 'protection_full_label',
    log: 'common_log_label',
    snapshot: 'common_production_snapshot_label',
    difference_increment: 'protection_incremental_label',
    cumulative_increment: 'common_diff_label',
    permanent_increment: 'protection_incremental_forever_label'
  };
  policyCount = {
    full: 0,
    log: 0,
    snapshot: 0,
    difference_increment: 0,
    cumulative_increment: 0,
    permanent_increment: 0
  };
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;

  @Input() data: any;
  @Input() action;
  @Input() activeIndex: number;
  @Input() formGroup: FormGroup;
  @Input() applicationType: ApplicationType;
  @Input() isSpecialSense: boolean;

  intervalUnitOptions = this.dataMapService
    .toArray('Interval_Unit')
    .filter(item => {
      return [
        DataMap.Interval_Unit.minute.value,
        DataMap.Interval_Unit.hour.value,
        DataMap.Interval_Unit.day.value
      ].includes(item.value);
    })
    .filter(item => {
      return (item.isLeaf = true);
    });
  fullIntervalUnitOptions = this.dataMapService
    .toArray('Interval_Unit')
    .filter(item => {
      return [
        DataMap.Interval_Unit.hour.value,
        DataMap.Interval_Unit.day.value
      ].includes(item.value);
    })
    .filter(item => {
      return (item.isLeaf = true);
    });
  durationUnitOptions = this.dataMapService
    .toArray('Interval_Unit')
    .filter(item => {
      return [
        DataMap.Interval_Unit.day.value,
        DataMap.Interval_Unit.week.value,
        DataMap.Interval_Unit.month.value,
        DataMap.Interval_Unit.year.value,
        DataMap.Interval_Unit.persistent.value
      ].includes(item.value);
    })
    .filter(item => {
      return (item.isLeaf = true);
    });
  daysOfMonthOptions = this.dataMapService
    .toArray('Days_Of_Month_Type')
    .filter(item => (item.isLeaf = true));
  daysOfWeekOptions = this.dataMapService
    .toArray('Days_Of_Week')
    .filter(item => (item.isLeaf = true));

  startTimeErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  intervalErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
  };
  retentionErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
  };
  specifiedRetentionErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
  };
  windowStartTimeErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  windowEndTimeErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  daysOfYearErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  daysOfMonthErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidDupInput: this.i18n.get('common_duplicate_input_label')
  };
  daysOfWeekErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_required_label')
  };
  walConfigNumErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 99])
  };

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private datePipe: DatePipe,
    public cookieService: CookieService,
    private messageService: MessageService,
    public baseUtilService: BaseUtilService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private storageApiService: StoragesApiService,
    private clusterApiService: ClustersApiService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.slaBackupName.difference_increment = includes(
      [
        ApplicationType.NASFileSystem,
        ApplicationType.HBase,
        ApplicationType.Hive,
        ApplicationType.HDFS,
        ApplicationType.Elasticsearch,
        ApplicationType.KubernetesStatefulSet,
        ApplicationType.KubernetesDatasetCommon,
        ApplicationType.Vmware,
        ApplicationType.CNware,
        ApplicationType.HCSCloudHost,
        ApplicationType.FusionCompute,
        ApplicationType.FusionOne,
        ApplicationType.OpenStack,
        ApplicationType.ApsaraStack,
        ApplicationType.TDSQL
      ],
      this.applicationType
    )
      ? 'protection_incremental_forever_label'
      : 'protection_incremental_label';
    this.updateMonthDays();
    this.updateOptItems();
    this.initTitle();
    this.updateForm();
    this.updateData();
    this.getStorageNames();
  }

  getSlaTips() {
    let tips: string;
    switch (this.applicationType) {
      case ApplicationType.TDSQL:
        tips = this.i18n.get('protection_tdsql_sla_tips_label');
        break;
      case ApplicationType.TiDB:
        tips = this.i18n.get('protection_tidb_sla_tip_label');
        break;
      default:
        tips = '';
        break;
    }
    return tips;
  }

  initTitle() {
    includes([ApplicationType.NASFileSystem], this.applicationType)
      ? this.policyCount.permanent_increment++
      : this.isHyperdetect
      ? this.policyCount.snapshot++
      : includes([ApplicationType.LocalFileSystem], this.applicationType)
      ? this.policyCount.difference_increment++
      : this.policyCount.full++;
  }

  addStorage() {
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
    archiveStorageComponent.createArchiveStorage(() => this.getStorageNames());
  }

  getStorageNames() {
    this.storageApiService
      .storageUsingGET({
        startPage: 0,
        pageSize: 200
      })
      .subscribe(res => {
        this.storageNames = map(res.records, item => {
          assign(item, {
            isLeaf: true,
            label: item.name
          });
          return item;
        });
      });
  }

  updateData() {
    if (!this.data || !size(this.data)) {
      return;
    }
    this.getBackupTeams().clear();
    each(this.data, item => {
      const backupTeam = cloneDeep(this.backupTeams);
      backupTeam.patchValue(item);
      this.listenFormGroup(backupTeam);
      this.dealUnit(
        backupTeam,
        this.action === ProtectResourceAction.Create
          ? includes(
              [
                DaysOfType.DaysOfDay,
                DaysOfType.DaysOfHour,
                DaysOfType.DaysOfMinute
              ],
              item.trigger_action
            )
            ? ScheduleTrigger.PERIOD_EXECUTE
            : ScheduleTrigger.SPECIFIED_TIME
          : item.trigger
      );
      this.getBackupTeams().push(backupTeam);
    });
    this.batchDealCtrl();
    if (
      includes(
        [
          ApplicationType.Fileset,
          ApplicationType.NASShare,
          ApplicationType.Volume,
          ApplicationType.ObjectStorage
        ],
        this.applicationType
      )
    ) {
      this.getPermanentBackupCheckboxValue();
    }
  }

  batchDealCtrl() {
    this.getBackupTeams().controls.map(control => {
      if (
        control.get('duration_unit').value ===
        DataMap.Interval_Unit.persistent.value
      ) {
        control.get('retention_duration').disable();
        control.get('retention_duration').setValue('');
      }
    });
  }

  updateForm() {
    this.formGroup.addControl('backupTeams', this.fb.array([this.backupTeams]));
    this.formGroup.get('backupTeams').valueChanges.subscribe(res => {
      this.checkOptsState(res);
    });
  }

  private replaceSpaces(str: string) {
    return str.replace(/[\s（）]/g, '_').replace(/[()]/g, '');
  }

  get backupTeams(): FormGroup {
    const backupTeam: FormGroup = this.fb.group({
      uuid: new FormControl(''),
      storage_id:
        this.applicationType === this.appType.LocalFileSystem &&
        this.cookieService.isCloudBackup &&
        !this.isHyperdetect
          ? new FormControl('', {
              validators: [
                this.baseUtilService.VALID.required(),
                this.validBackupStorage()
              ]
            })
          : new FormControl(''),
      action: new FormControl(
        includes([ApplicationType.NASFileSystem], this.applicationType)
          ? PolicyAction.PERMANENT
          : this.isHyperdetect
          ? PolicyAction.SNAPSHOT
          : includes([ApplicationType.LocalFileSystem], this.applicationType)
          ? PolicyAction.INCREMENT
          : PolicyAction.FULL
      ),
      name: new FormControl(
        this.replaceSpaces(
          includes([ApplicationType.NASFileSystem], this.applicationType)
            ? `${this.i18n.get(this.slaBackupName[PolicyAction.PERMANENT])}01`
            : this.isHyperdetect
            ? `${this.i18n.get(this.slaBackupName[PolicyAction.SNAPSHOT])}`
            : includes([ApplicationType.LocalFileSystem], this.applicationType)
            ? `${this.i18n.get(this.slaBackupName[PolicyAction.INCREMENT])}01`
            : `${this.i18n.get(this.slaBackupName[PolicyAction.FULL])}01`
        ),
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name()
          ]
        }
      ),
      permanentBackup: new FormControl(true),
      title: new FormControl(
        includes([ApplicationType.NASFileSystem], this.applicationType)
          ? `${this.i18n.get(this.slaBackupName[PolicyAction.PERMANENT])}01`
          : this.isHyperdetect
          ? `${this.i18n.get(this.slaBackupName[PolicyAction.SNAPSHOT])}01`
          : includes([ApplicationType.LocalFileSystem], this.applicationType)
          ? `${this.i18n.get(this.slaBackupName[PolicyAction.INCREMENT])}01`
          : `${this.i18n.get(this.slaBackupName[PolicyAction.FULL])}01`
      ),
      trigger: new FormControl(ScheduleTrigger.PERIOD_EXECUTE),
      start_time: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      interval: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 365)
        ]
      }),
      interval_unit: new FormControl(DataMap.Interval_Unit.hour.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      interval_error_tip: new FormControl(this.intervalErrorTip),
      retention_duration: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 365)
        ]
      }),
      retention_duration_error_tip: new FormControl(this.retentionErrorTip),
      special_retention_duration_error_tip: new FormControl(
        this.specifiedRetentionErrorTip
      ),
      duration_unit: new FormControl(DataMap.Interval_Unit.day.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      window_start: new FormControl(defaultWindowTime(), {
        validators: [this.baseUtilService.VALID.required()]
      }),
      window_end: new FormControl(defaultWindowTime(), {
        validators: [this.baseUtilService.VALID.required()]
      }),
      specified_retention_duration: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 10)
        ]
      }),
      specified_duration_unit: new FormControl(
        DataMap.Interval_Unit.year.value,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      specified_window_start: new FormControl(defaultWindowTime(), {
        validators: [this.baseUtilService.VALID.required()]
      }),
      specified_window_end: new FormControl(defaultWindowTime(), {
        validators: [this.baseUtilService.VALID.required()]
      }),
      trigger_action: new FormControl(DaysOfType.DaysOfDay),
      days_of_year: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      days_of_month_type: new FormControl(
        DataMap.Days_Of_Month_Type.specifiedDate.value
      ),
      days_of_months: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      days_of_month: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      days_of_week: new FormControl([], {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1)
        ]
      }),
      is_reserved_latest_snapshot: new FormControl(false)
    });

    this.listenFormGroup(backupTeam);
    return backupTeam;
  }

  listenFormGroup(backupTeam: FormGroup) {
    this.checkPolicyType(backupTeam);
    this.checkTrigger(backupTeam);

    if (
      includes(
        [
          ApplicationType.Fileset,
          ApplicationType.NASShare,
          ApplicationType.Volume,
          ApplicationType.ObjectStorage
        ],
        this.applicationType
      )
    ) {
      backupTeam.get('permanentBackup').valueChanges.subscribe(res => {
        if (backupTeam.get('action').value === PolicyAction.FULL) {
          return;
        }

        const controls = this.getBackupTeams().controls;

        each(controls, item => {
          item.get('permanentBackup').setValue(res, { emitEvent: false });
        });
      });
    }
  }

  getBackupTeams() {
    return this.formGroup.get('backupTeams') as FormArray;
  }

  removeBackupTeam(tab) {
    if (
      this.isSpecialSense &&
      includes(
        [
          ApplicationType.Fileset,
          ApplicationType.NASShare,
          ApplicationType.Volume
        ],
        this.applicationType
      ) &&
      this.action === ProtectResourceAction.Modify
    ) {
      return;
    }

    const controls = this.getBackupTeams().controls;
    if (controls.length === 1) {
      this.messageService.error(
        this.i18n.get('common_at_least_retain_label', [
          this.i18n.get('common_backup_policy_label')
        ])
      );

      return;
    }

    for (let index = 0; index < controls.length; index++) {
      if (
        !this.checkDepRelationship(tab.lvId) &&
        this.applicationType !== ApplicationType.Fileset
      ) {
        this.messageService.error(
          this.i18n.get('common_at_least_retain_label', [
            this.cookieService.isCloudBackup && !this.isHyperdetect
              ? this.slaBackupName['difference_increment']
              : this.slaBackupName[tab.lvId]
          ]),
          {
            lvMessageKey: 'lvMsg_retain_num',
            lvShowCloseButton: true
          }
        );
        break;
      }
      if (tab.lvId === controls[index].value.name + tab.index) {
        this.getBackupTeams().removeAt(index);
      }
    }
    this.activeIndex = first(controls).value.name + 0;

    if (
      includes(
        [
          ApplicationType.Fileset,
          ApplicationType.NASShare,
          ApplicationType.Volume,
          ApplicationType.ObjectStorage
        ],
        this.applicationType
      )
    ) {
      if (
        !find(
          this.getBackupTeams().value,
          item => item.action === PolicyAction.FULL
        )
      ) {
        this.onlyIncrement = true;
        each(this.getBackupTeams().controls, item => {
          item.get('permanentBackup').setValue(true);
        });
      } else {
        this.onlyIncrement = false;
      }
    }
  }

  checkDepRelationship(id) {
    let isOK = true;
    if (this.cookieService.isCloudBackup) {
      isOK =
        size(
          filter(this.getBackupTeams().value, obj => {
            return includes([PolicyAction.INCREMENT], obj.action);
          })
        ) > 1;
    } else {
      if (id === PolicyAction.FULL) {
        isOK = !!size(
          filter(this.getBackupTeams().value, obj => {
            return includes(
              [PolicyAction.INCREMENT, PolicyAction.DIFFERENCE],
              obj.action
            );
          })
        );
      } else if (
        includes([PolicyAction.INCREMENT, PolicyAction.DIFFERENCE], id)
      ) {
        isOK = !!size(
          filter(this.getBackupTeams().value, obj => {
            return includes(
              [PolicyAction.FULL, PolicyAction.SNAPSHOT],
              obj.action
            );
          })
        );
      } else if (includes([PolicyAction.SNAPSHOT], id)) {
        isOK = !!size(
          filter(this.getBackupTeams().value, obj => {
            return includes([PolicyAction.INCREMENT], obj.action);
          })
        );
      }
    }

    return isOK;
  }

  checkPolicyType(backupTeam: FormGroup) {
    if (backupTeam.get('action').value === PolicyAction.LOG) {
      backupTeam.get('window_start').clearValidators();
      backupTeam.get('window_end').clearValidators();
      backupTeam.get('days_of_year').clearValidators();
      backupTeam.get('days_of_month').clearValidators();
      backupTeam.get('days_of_week').clearValidators();
      backupTeam.get('specified_window_start').clearValidators();
      backupTeam.get('specified_window_end').clearValidators();
      backupTeam.get('specified_retention_duration').clearValidators();
      backupTeam.get('specified_duration_unit').clearValidators();
      if (this.applicationType === this.appType.HBase) {
        backupTeam
          .get('duration_unit')
          .setValue(DataMap.Interval_Unit.persistent.value);
        this.changeUnit(
          backupTeam,
          DataMap.Interval_Unit.persistent.value,
          'retention_duration'
        );
      }
    } else {
      backupTeam
        .get('window_start')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('window_end')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('days_of_year')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('days_of_month')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('days_of_week')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1)
        ]);
      backupTeam
        .get('specified_window_start')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('specified_window_end')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('specified_retention_duration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 10)
        ]);
      backupTeam
        .get('specified_duration_unit')
        .setValidators([this.baseUtilService.VALID.required()]);
    }
    backupTeam.get('window_start').updateValueAndValidity();
    backupTeam.get('window_end').updateValueAndValidity();
    backupTeam.get('days_of_year').updateValueAndValidity();
    backupTeam.get('days_of_month').updateValueAndValidity();
    backupTeam.get('days_of_week').updateValueAndValidity();
    backupTeam.get('specified_window_start').updateValueAndValidity();
    backupTeam.get('specified_window_end').updateValueAndValidity();
    backupTeam.get('specified_retention_duration').updateValueAndValidity();
    backupTeam.get('specified_duration_unit').updateValueAndValidity();
  }

  checkTrigger(backupTeam: FormGroup) {
    this.checkDaysOfType(backupTeam);
  }

  dealUnit(backupTeam: FormGroup, trigger) {
    if (trigger === ScheduleTrigger.PERIOD_EXECUTE) {
      const duration_unit = backupTeam.get('duration_unit').value;
      if (duration_unit) {
        this.changeUnit(backupTeam, duration_unit, 'retention_duration');
      }
    } else {
      const specified_duration_unit = backupTeam.get('specified_duration_unit')
        .value;
      if (specified_duration_unit) {
        this.changeUnit(
          backupTeam,
          specified_duration_unit,
          'specified_retention_duration'
        );
      }
    }

    this.listenUnit(backupTeam, trigger);
  }

  listenUnit(backupTeam: FormGroup, trigger: ScheduleTrigger) {
    backupTeam.get('duration_unit').valueChanges.subscribe(res => {
      if (trigger !== ScheduleTrigger.PERIOD_EXECUTE) {
        return;
      }
      this.changeUnit(backupTeam, res, 'retention_duration');
    });
    backupTeam.get('specified_duration_unit').valueChanges.subscribe(res => {
      if (trigger === ScheduleTrigger.PERIOD_EXECUTE) {
        return;
      }
      this.changeUnit(backupTeam, res, 'specified_retention_duration');
    });
  }

  checkDaysOfType(backupTeam: FormGroup) {
    backupTeam.get('trigger_action').valueChanges.subscribe(res => {
      this.dealDaysOfType(backupTeam, res);
      // 修改时从天切换到小时，虽然校验住了但如果不符合校验第一次不会报错误信息
      backupTeam.get('interval').markAsTouched();
    });
    this.dealDaysOfType(backupTeam, backupTeam.get('trigger_action').value);
  }

  dealDaysOfType(backupTeam: FormGroup, daysOfType) {
    each(
      [
        'start_time',
        'interval',
        'interval_unit',
        'retention_duration',
        'duration_unit',
        'window_start',
        'window_end',
        'days_of_year',
        'days_of_month',
        'days_of_months',
        'days_of_week',
        'specified_duration_unit',
        'specified_window_start',
        'specified_window_end'
      ],
      item => {
        backupTeam.get(item)?.clearValidators();
      }
    );
    if (
      [
        DaysOfType.DaysOfYear,
        DaysOfType.DaysOfMonth,
        DaysOfType.DaysOfWeek
      ].includes(daysOfType) &&
      !!this.data &&
      backupTeam.value.specified_duration_unit !==
        DataMap.Interval_Unit.persistent.value
    ) {
      // 修改时为保留时间添加校验逻辑和正确错误提示
      const specified_unit_table = {
        y: [1, 10],
        MO: [1, 24],
        w: [1, 54],
        d: [1, 365],
        h: [1, 23],
        m: [1, 10]
      };
      let unitValue = backupTeam.value.specified_duration_unit;
      backupTeam
        .get('specified_retention_duration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(
            specified_unit_table[unitValue][0],
            specified_unit_table[unitValue][1]
          )
        ]);
      backupTeam.get('special_retention_duration_error_tip').setValue({
        ...this.baseUtilService.rangeErrorTip,
        invalidRang: this.i18n.get('common_valid_rang_label', [
          specified_unit_table[unitValue][0],
          specified_unit_table[unitValue][1]
        ])
      });
    }
    if (daysOfType === DaysOfType.DaysOfYear) {
      backupTeam
        .get('specified_window_start')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('specified_window_end')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('days_of_year')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam.get('days_of_month').clearValidators();
      backupTeam.get('days_of_months').clearValidators();
      backupTeam.get('days_of_week').clearValidators();
      if (!this.data || !size(this.data)) {
        backupTeam
          .get('specified_retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 10)
          ]);
        backupTeam
          .get('specified_duration_unit')
          .setValue(DataMap.Interval_Unit.year.value);
        backupTeam.get('special_retention_duration_error_tip').setValue({
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
        });
      }
    } else if (daysOfType === DaysOfType.DaysOfMonth) {
      backupTeam
        .get('specified_window_start')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('specified_window_end')
        .setValidators([this.baseUtilService.VALID.required()]);
      this.checkDaysOfMonthType(backupTeam);
      backupTeam.get('days_of_year').clearValidators();
      backupTeam.get('days_of_week').clearValidators();
      if (!this.data || !size(this.data)) {
        backupTeam
          .get('specified_retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 24)
          ]);
        backupTeam
          .get('specified_duration_unit')
          .setValue(DataMap.Interval_Unit.month.value);
        backupTeam.get('special_retention_duration_error_tip').setValue({
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 24])
        });
      }
    } else if (daysOfType === DaysOfType.DaysOfWeek) {
      backupTeam
        .get('specified_window_start')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('specified_window_end')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('days_of_week')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1)
        ]);
      backupTeam.get('days_of_year').clearValidators();
      backupTeam.get('days_of_month').clearValidators();
      backupTeam.get('days_of_months').clearValidators();
      if (!this.data || !size(this.data)) {
        backupTeam
          .get('specified_retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 54)
          ]);
        backupTeam
          .get('specified_duration_unit')
          .setValue(DataMap.Interval_Unit.week.value);
        backupTeam.get('special_retention_duration_error_tip').setValue({
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 54])
        });
      }
    } else {
      const validInterval =
        daysOfType === DaysOfType.DaysOfDay
          ? 365
          : daysOfType === DaysOfType.DaysOfHour
          ? 23
          : 59;
      const minInterval =
        daysOfType === DaysOfType.DaysOfMinute &&
        backupTeam.get('action').value === PolicyAction.LOG
          ? 5
          : 1;
      backupTeam
        .get('start_time')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('interval')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(minInterval, validInterval)
        ]);
      backupTeam.get('interval_error_tip').setValue({
        ...this.baseUtilService.rangeErrorTip,
        invalidRang: this.i18n.get('common_valid_rang_label', [
          minInterval,
          validInterval
        ])
      });
      backupTeam
        .get('interval_unit')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam
        .get('retention_duration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 365)
        ]);
      backupTeam
        .get('duration_unit')
        .setValidators([this.baseUtilService.VALID.required()]);
      backupTeam.get('specified_retention_duration').clearValidators();
      this.changeUnit(
        backupTeam,
        backupTeam.get('duration_unit').value,
        'retention_duration'
      );
      if (backupTeam.get('action').value !== PolicyAction.LOG) {
        backupTeam
          .get('window_start')
          .setValidators([this.baseUtilService.VALID.required()]);
        backupTeam
          .get('window_end')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
    }
    each(
      [
        'start_time',
        'interval',
        'interval_unit',
        'retention_duration',
        'duration_unit',
        'window_start',
        'window_end',
        'days_of_year',
        'days_of_month',
        'days_of_months',
        'days_of_week',
        'specified_retention_duration',
        'specified_duration_unit',
        'specified_window_start',
        'specified_window_end'
      ],
      item => {
        backupTeam.get(item)?.updateValueAndValidity();
      }
    );
  }

  checkDaysOfMonthType(backupTeam: FormGroup) {
    backupTeam.get('days_of_month_type').valueChanges.subscribe(res => {
      this.dealDaysOfMonthType(backupTeam, res);
    });
    this.dealDaysOfMonthType(
      backupTeam,
      backupTeam.get('days_of_month_type').value
    );
  }

  dealDaysOfMonthType(backupTeam: FormGroup, daysOfMonthType) {
    if (daysOfMonthType === this.daysOfMonthType.specifiedDate.value) {
      this.checkDaysOfMonth(backupTeam);
      backupTeam
        .get('days_of_month')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.validDaysOfMonth()
        ]);
      backupTeam
        .get('days_of_months')
        .setValidators([this.baseUtilService.VALID.required()]);
    } else {
      backupTeam.get('days_of_month').clearValidators();
      backupTeam.get('days_of_months').clearValidators();
    }
    backupTeam.get('days_of_month').updateValueAndValidity();
    backupTeam.get('days_of_months').updateValueAndValidity();
  }

  checkDaysOfMonth(backupTeam: FormGroup) {
    backupTeam.get('days_of_months').valueChanges.subscribe(res => {
      backupTeam
        .get('days_of_month')
        .setValue(toString((res || []).sort((a, b) => a - b)));
    });
  }

  updateOptItems() {
    const isPermanentBackupResource = includes(
      [
        ApplicationType.NASFileSystem,
        ApplicationType.HBase,
        ApplicationType.Hive,
        ApplicationType.HDFS,
        ApplicationType.Elasticsearch,
        ApplicationType.KubernetesStatefulSet,
        ApplicationType.KubernetesDatasetCommon,
        ApplicationType.Vmware,
        ApplicationType.CNware,
        ApplicationType.HCSCloudHost,
        ApplicationType.FusionCompute,
        ApplicationType.FusionOne,
        ApplicationType.OpenStack,
        ApplicationType.ApsaraStack,
        ApplicationType.TDSQL,
        ApplicationType.Volume
      ],
      this.applicationType
    );
    this.optsItems = [
      {
        id: PolicyAction.FULL,
        disabled: false,
        hidden: this.hiddenFull(),
        tips: this.i18n.get('common_full_backup_tips_label'),
        label: this.i18n.get('common_full_backup_label'),
        onClick: () => {
          this.addBackupItem(PolicyAction.FULL);
        }
      },
      {
        id: includes(
          [ApplicationType.NASFileSystem, ApplicationType.Volume],
          this.applicationType
        )
          ? PolicyAction.PERMANENT
          : PolicyAction.INCREMENT,
        disabled: this.disabledIncrement(),
        hidden: this.hiddenPercrement() || this.isHyperdetect,
        label: isPermanentBackupResource
          ? this.i18n.get('common_permanent_backup_label')
          : this.i18n.get('common_incremental_backup_label'),
        tips: isPermanentBackupResource
          ? this.i18n.get('common_permanent_backup_tips_label')
          : this.i18n.get('common_incremental_backup_tips_label'),
        onClick: () => {
          this.addBackupItem(
            includes(
              [ApplicationType.NASFileSystem, ApplicationType.Volume],
              this.applicationType
            )
              ? PolicyAction.PERMANENT
              : PolicyAction.INCREMENT
          );
        }
      },
      {
        id: includes(
          [
            ApplicationType.Fileset,
            ApplicationType.NASShare,
            ApplicationType.Exchange
          ],
          this.applicationType
        )
          ? PolicyAction.PERMANENT
          : '',
        hidden: this.hiddenIncrement(),
        label: this.i18n.get('common_permanent_backup_label'),
        tips: this.i18n.get('common_permanent_backup_tips_label'),
        onClick: () => {
          this.addBackupItem(PolicyAction.PERMANENT);
        }
      },
      {
        id: PolicyAction.DIFFERENCE,
        disabled: false,
        hidden: this.hiddenDifference(),
        label: this.i18n.get('common_diff_backup_label'),
        tips: this.i18n.get('common_diff_backup_tips_label'),
        onClick: () => {
          this.addBackupItem(PolicyAction.DIFFERENCE);
        }
      },
      {
        id: PolicyAction.LOG,
        disabled: false,
        hidden: this.hiddenLog(),
        label: this.i18n.get('common_log_backup_label'),
        tips: this.i18n.get('common_log_backup_tips_label'),
        onClick: () => {
          this.addBackupItem(PolicyAction.LOG);
        }
      },
      {
        id: PolicyAction.SNAPSHOT,
        disabled: true,
        hidden: !this.isHyperdetect,
        label: this.i18n.get('common_anti_detection_snapshot_label'),
        onClick: () => {
          this.addBackupItem(PolicyAction.SNAPSHOT);
        }
      }
    ];
  }

  updateMonthDays() {
    const monthDays = Array.from({ length: 31 }, (_, i) => i + 1);
    const chunkSize = Math.ceil(monthDays.length / 5);
    this.monthDaysItems = Array.from({ length: 5 }, (_, i) => {
      const start = i * chunkSize;
      const end = start + chunkSize;
      return { key: monthDays.slice(start, end) };
    });
  }

  disabledIncrement() {
    return includes([], this.applicationType);
  }

  hiddenFull() {
    return includes(
      [ApplicationType.NASFileSystem, ApplicationType.LocalFileSystem],
      this.applicationType
    );
  }

  hiddenLog() {
    return !includes(
      [
        ApplicationType.HBase,
        ApplicationType.Oracle,
        ApplicationType.GaussDBT,
        ApplicationType.SQLServer,
        ApplicationType.KubernetesMySQL,
        ApplicationType.MySQL,
        ApplicationType.KingBase,
        ApplicationType.PostgreSQL,
        ApplicationType.Dameng,
        ApplicationType.Informix,
        ApplicationType.GeneralDatabase,
        ApplicationType.DB2,
        ApplicationType.GaussDBForOpenGauss,
        ApplicationType.LightCloudGaussDB,
        ApplicationType.MongoDB,
        ApplicationType.TDSQL,
        ApplicationType.OceanBase,
        ApplicationType.TiDB,
        ApplicationType.Exchange,
        ApplicationType.GoldenDB,
        ApplicationType.SapHana,
        ApplicationType.OpenGauss
      ],
      this.applicationType
    );
  }

  hiddenPercrement() {
    return includes(
      [
        ApplicationType.SQLServer,
        ApplicationType.ClickHouse,
        ApplicationType.PostgreSQL,
        ApplicationType.Redis,
        ApplicationType.GaussDBForOpenGauss,
        ApplicationType.MongoDB,
        ApplicationType.TiDB,
        ApplicationType.ActiveDirectory
      ],
      this.applicationType
    );
  }

  hiddenIncrement() {
    return !includes([ApplicationType.Exchange], this.applicationType);
  }

  hiddenDifference() {
    return !includes(
      [
        ApplicationType.SQLServer,
        ApplicationType.DB2,
        ApplicationType.Oracle,
        ApplicationType.GaussDBT,
        ApplicationType.MySQL,
        ApplicationType.GaussDBDWS,
        ApplicationType.Dameng,
        ApplicationType.GaussDBForOpenGauss,
        ApplicationType.Informix,
        ApplicationType.GeneralDatabase,
        ApplicationType.SapHana
      ],
      this.applicationType
    );
  }

  checkOptsState(res) {
    each(this.optsItems, opt => {
      if (opt.id === PolicyAction.DIFFERENCE) {
        opt.disabled =
          !isUndefined(find(res, { action: PolicyAction.INCREMENT })) ||
          size(filter(res, res => res.action === PolicyAction.DIFFERENCE)) > 3;
      } else if (opt.id === PolicyAction.INCREMENT) {
        opt.disabled = this.cookieService.isCloudBackup
          ? false
          : !isUndefined(find(res, { action: PolicyAction.DIFFERENCE })) ||
            size(filter(res, res => res.action === PolicyAction.INCREMENT)) > 3;
      } else if (opt.id === PolicyAction.PERMANENT) {
        opt.disabled =
          size(filter(res, res => res.action === PolicyAction.PERMANENT)) > 3;
      } else if (opt.id === PolicyAction.FULL) {
        opt.disabled =
          size(filter(res, res => res.action === PolicyAction.FULL)) > 3;
      } else {
        opt.disabled = !isUndefined(find(res, { action: opt.id }));
      }
    });
  }

  addBackupItem(id) {
    if (
      this.isSpecialSense &&
      includes(
        [ApplicationType.Fileset, ApplicationType.NASShare],
        this.applicationType
      ) &&
      this.action === ProtectResourceAction.Modify
    ) {
      return;
    }

    const backupItem = find(this.optsItems, { id });
    const backupTeam = cloneDeep(this.backupTeams);
    const nameArr = map(this.getBackupTeams().value, item => item.name);

    if (this.policyCount[id] === 0) {
      this.policyCount[id]++;
    }

    while (
      includes(
        nameArr,
        this.replaceSpaces(
          `${this.i18n.get(this.slaBackupName[backupItem.id])}0${
            this.policyCount[id]
          }`
        )
      )
    ) {
      this.policyCount[id]++;
    }

    backupTeam.patchValue({
      name: this.replaceSpaces(
        `${this.i18n.get(this.slaBackupName[backupItem.id])}0${
          this.policyCount[id]
        }`
      ),
      title: `${this.i18n.get(this.slaBackupName[backupItem.id])}0${
        this.policyCount[id]
      }`,
      action: backupItem.id
    });
    this.listenFormGroup(backupTeam);
    this.getBackupTeams().push(backupTeam);
    this.activeIndex = size(this.getBackupTeams().controls) - 1;
    if (
      includes(
        [
          ApplicationType.Fileset,
          ApplicationType.NASShare,
          ApplicationType.Volume,
          ApplicationType.ObjectStorage
        ],
        this.applicationType
      )
    ) {
      backupTeam
        .get('permanentBackup')
        .setValue(this.getPermanentBackupCheckboxValue());
    }
  }

  getPermanentBackupCheckboxValue() {
    const formValue = this.getBackupTeams().value;

    this.onlyIncrement = false;

    if (!find(formValue, item => item.action === PolicyAction.FULL)) {
      this.onlyIncrement = true;
      return true;
    } else if (
      !!find(formValue, item => item.action === PolicyAction.INCREMENT)
    ) {
      return get(
        find(formValue, item => item.action === PolicyAction.INCREMENT),
        'permanentBackup'
      );
    } else {
      return false;
    }
  }

  changeUnit(formGroup, value, formControlName) {
    formGroup.get(formControlName).enable();
    if (value === DataMap.Interval_Unit.minute.value) {
      if (formGroup.get('action').value === PolicyAction.LOG) {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(5, 59)
          ]);
        formGroup.get('interval_error_tip').setValue({
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [5, 59])
        });
      } else {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 59)
          ]);
        formGroup.get('interval_error_tip').setValue({
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 59])
        });
      }
    } else if (value === DataMap.Interval_Unit.hour.value) {
      formGroup
        .get(formControlName)
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 23)
        ]);
      formGroup.get('interval_error_tip').setValue({
        ...this.baseUtilService.rangeErrorTip,
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 23])
      });
    } else if (value === DataMap.Interval_Unit.day.value) {
      formGroup
        .get(formControlName)
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 365)
        ]);
      const errorTip = {
        ...this.baseUtilService.rangeErrorTip,
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
      };
      if ('retention_duration' === formControlName) {
        formGroup.get('retention_duration_error_tip').setValue(errorTip);
      } else if ('specified_retention_duration' === formControlName) {
        formGroup
          .get('special_retention_duration_error_tip')
          .setValue(errorTip);
      } else if ('interval' === formControlName) {
        formGroup.get('interval_error_tip').setValue(errorTip);
      }
    } else if (value === DataMap.Interval_Unit.week.value) {
      if (
        'retention_duration' === formControlName ||
        'specified_retention_duration' === formControlName
      ) {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 54)
          ]);
        if ('retention_duration' === formControlName) {
          formGroup.get('retention_duration_error_tip').setValue({
            ...this.baseUtilService.rangeErrorTip,
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 54])
          });
        } else if ('specified_retention_duration' === formControlName) {
          formGroup.get('special_retention_duration_error_tip').setValue({
            ...this.baseUtilService.rangeErrorTip,
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 54])
          });
        }
      } else if ('interval' === formControlName) {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 4)
          ]);
        formGroup.get('interval_error_tip').setValue({
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 4])
        });
      }
    } else if (value === DataMap.Interval_Unit.month.value) {
      if (
        'retention_duration' === formControlName ||
        'specified_retention_duration' === formControlName
      ) {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 24)
          ]);
        if ('retention_duration' === formControlName) {
          formGroup.get('retention_duration_error_tip').setValue({
            ...this.baseUtilService.rangeErrorTip,
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 24])
          });
        } else if ('specified_retention_duration' === formControlName) {
          formGroup.get('special_retention_duration_error_tip').setValue({
            ...this.baseUtilService.rangeErrorTip,
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 24])
          });
        }
      } else if ('interval' === formControlName) {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 12)
          ]);
        formGroup.get('interval_error_tip').setValue({
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 12])
        });
      }
    } else if (value === DataMap.Interval_Unit.year.value) {
      if (
        'retention_duration' === formControlName ||
        'specified_retention_duration' === formControlName
      ) {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 10)
          ]);
        if ('retention_duration' === formControlName) {
          formGroup.get('retention_duration_error_tip').setValue({
            ...this.baseUtilService.rangeErrorTip,
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
          });
        } else if ('specified_retention_duration' === formControlName) {
          formGroup.get('special_retention_duration_error_tip').setValue({
            ...this.baseUtilService.rangeErrorTip,
            invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
          });
        }
      }
    } else if (value === DataMap.Interval_Unit.persistent.value) {
      formGroup.get(formControlName).disable();
      formGroup.get(formControlName).setValue('');
    }
    formGroup.get(formControlName).updateValueAndValidity();
  }

  validDaysOfMonth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }

      if (
        size(uniq(split(control.value, ','))) !==
        size(split(control.value, ','))
      ) {
        return { invalidDupInput: { value: control.value } };
      }

      if (
        !isUndefined(
          split(control.value, ',').find(item => !/^[1-9]\d*$/.test(item))
        ) ||
        !isUndefined(
          split(control.value, ',').find(
            item => !(+item > 0 && +item < 32) || isNaN(+item)
          )
        )
      ) {
        return { invalidInteger: { value: control.value } };
      }

      return null;
    };
  }

  validBackupStorage(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }

      const controls = this.getBackupTeams().controls,
        len = size(
          filter(
            controls,
            ctrl => ctrl.get('storage_id').value === control.value
          )
        );
      return len > 1 ? { invalidRepeat: { value: control.value } } : null;
    };
  }
}
