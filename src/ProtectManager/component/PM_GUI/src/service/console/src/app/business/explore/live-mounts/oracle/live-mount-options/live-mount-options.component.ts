import {
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  QueryList,
  ViewChildren
} from '@angular/core';
import {
  AbstractControl,
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { OptionItem, SelectComponent } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DatabasesService,
  DataMap,
  DataMapService,
  extendParams,
  GlobalService,
  I18NService,
  LabelApiService,
  LiveMountAction,
  LiveMountApiService,
  MountTargetLocation,
  ProtectedResourceApiService
} from 'app/shared';
import { TableConfig, TableData } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  countBy,
  each,
  endsWith,
  filter,
  find,
  first,
  get,
  includes,
  intersection,
  isArray,
  isEmpty,
  isNumber,
  isUndefined,
  map,
  omit,
  reject,
  size,
  split,
  toLower,
  toString,
  trim,
  uniq,
  uniqBy,
  values
} from 'lodash';
import { pairwise } from 'rxjs/operators';

@Component({
  selector: 'aui-live-mount-oracle-options',
  templateUrl: './live-mount-options.component.html',
  styleUrls: ['./live-mount-options.component.less']
})
export class LiveMountOptionsComponent implements OnInit {
  offlineWarnTip;
  oracleOfflineWarnTip;
  dataMap = DataMap;
  formGroup: FormGroup;
  targetFormGroup: FormGroup;
  instanceConfig: TableConfig;
  instanceData: TableData;
  targetHostOptions = [];
  targetTdsqlInstanceIps = [];
  latencyOptions = this.dataMapService
    .toArray('LiveMount_Latency')
    .filter(v => (v.isLeaf = true));
  @Input() activeIndex;
  @Input() componentData;
  isDrill;
  @Output() selectMountOptionChange = new EventEmitter<any>();
  isMySQL: boolean;
  isTDSQL: boolean;
  isOracle: boolean;
  validRepeatHostFlag = true;
  minBandwidthErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999]),
    invalidMin: this.i18n.get('explore_min_max_valid_label')
  });
  maxBandwidthErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999]),
    invalidMax: this.i18n.get('explore_max_min_valid_label')
  });
  burstBandwidthErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999]),
    invalidBurst: this.i18n.get('explore_burst_valid_label')
  });
  minIopsErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [100, 999999999]),
    invalidMin: this.i18n.get('explore_min_max_valid_label')
  });
  maxIopsErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [100, 999999999]),
    invalidMax: this.i18n.get('explore_max_min_valid_label')
  });
  burstIopsErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [100, 999999999]),
    invalidBurst: this.i18n.get('explore_burst_valid_label')
  });
  burstTimeErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999])
  });
  targetHostErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidSameDb: this.i18n.get('protection_target_host_oneline_db_label'),
    invalidSameHost: this.i18n.get('explore_target_same_host_label'),
    invalidHaveDb: this.i18n.get('protection_target_host_install_db_label')
  });
  scriptNameErrorTip = assign({}, this.baseUtilService.scriptNameErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  });
  mysqlPortErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1024, 65535])
  });
  targetIpLabel = this.i18n.get('common_ip_address_label', [], true);
  versionLabel = this.i18n.get('protection_database_version_label', [], true);
  scriptTipsLabel = this.i18n.get('common_script_agent_position_label');
  instanceInputLabel = this.i18n.get(
    'protection_target_instance_input_tip_label'
  );
  instanceInputErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_target_instance_input_error_label')
  };
  originalInstanceOptions: OptionItem[] = [];
  iopsItems = [
    {
      id: 850000,
      header: '16KB'
    },
    {
      id: 680000,
      header: '32KB'
    },
    {
      id: 450000,
      header: '64KB'
    },
    {
      id: 225000,
      header: '128KB'
    },
    {
      id: 112500,
      header: '256KB'
    },
    {
      id: 56250,
      header: '512KB'
    },
    {
      id: 28125,
      header: '1024KB'
    },
    {
      id: 14063,
      header: '2048KB'
    }
  ];
  latencyItems = [
    {
      id: 850000,
      header: '16KB'
    },
    {
      id: 680000,
      header: '32KB'
    },
    {
      id: 450000,
      header: '64KB'
    },
    {
      id: 225000,
      header: '128KB'
    },
    {
      id: 112500,
      header: '256KB'
    },
    {
      id: 56250,
      header: '512KB'
    },
    {
      id: 28125,
      header: '1024KB'
    },
    {
      id: 14063,
      header: '2048KB'
    }
  ];

  labelOptions = [];

  @ViewChildren('selectHost')
  selectHost: QueryList<SelectComponent>;

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private globalService: GlobalService,
    private dataMapService: DataMapService,
    private liveMountApiService: LiveMountApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private databasesService: DatabasesService,
    private labelApiService: LabelApiService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.getResourceType();
    this.initForm();
    this.getLabelOptions();
  }

  getLabelOptions() {
    const extParams = {
      startPage: CommonConsts.PAGE_START_EXTRA,
      akLoading: true
    };

    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.labelApiService.queryLabelUsingGET(params),
      res => {
        const arr = res?.map(item => {
          return {
            id: item.uuid,
            label: item.name,
            value: item.uuid,
            isLeaf: true
          };
        });
        this.labelOptions = arr;
      }
    );
  }

  // 回显主机选择
  addInstanceForm(item) {
    const form: FormGroup = this.getTargetLocationFormGroup();
    form.get('host_id').valueChanges.subscribe(res => {
      const index = this.targetInstances.controls.indexOf(form);
      // 选标签时会清空选择的主机，oracle主机的相关联动也需清空
      if (!res) {
        (this.targetInstances
          .at(index)
          .get('instance_array') as FormArray).clear();
      }
      this.formatTableDataById(res, index, item);
    });
    (this.formGroup.get('targetInstances') as FormArray).push(form);
    const host = find(this.targetHostOptions, {
      uuid: item[0]?.target_object_id
    });
    setTimeout(() => {
      form.patchValue({
        host_id: host?.rootUuid || item[0]?.agent_id
      });
    });
  }

  // oracle存在单机和集群
  parseOracleData(instances) {
    const uuids = uniq(map(instances, 'target_object_id'));
    const uuidMap = {};
    each(uuids, uuid => {
      const instance = filter(instances, { target_object_id: uuid });
      uuidMap[uuid] = instance;
    });
    return values(uuidMap);
  }

  // 演练计划回显
  updateDrillData() {
    if (
      this.isDrill &&
      !isEmpty(this.componentData?.rowCopy?.drillRecoveryConfig)
    ) {
      const config = this.componentData?.rowCopy?.drillRecoveryConfig;
      if (this.isOracle) {
        const instances = JSON.parse(config.parameters?.instances);
        (this.formGroup.get('targetInstances') as FormArray).clear();
        each(this.parseOracleData(instances), item =>
          this.addInstanceForm(item)
        );
      } else {
        this.updateDrillHost(config);
      }
      this.updateDrillForm(config);
    }
  }

  updateDrillHost(config) {
    // 先去掉重复校验，回显以后加回来，否则界面会校验不过。
    this.validRepeatHostFlag = false;
    (this.formGroup.get('targetHosts') as FormArray).clear();
    each(config.target_resource_uuid_list, uuid => {
      const form = this.getTargetHostFormGroup();
      (this.formGroup.get('targetHosts') as FormArray).push(form);
      const host = find(this.targetHostOptions, { key: uuid });
      setTimeout(() => {
        form.patchValue({
          host_id: uuid,
          ip: host?.ip,
          version: host?.version
        });
      });
    });
  }

  updateDrillForm(config) {
    this.formGroup.patchValue({
      mysql_port: config?.parameters?.mysql_port,
      bindWidthStatus: isNumber(config?.parameters?.performance?.min_bandwidth),
      iopsStatus: isNumber(config?.parameters?.performance?.min_iops),
      latencyStatus: isNumber(config?.parameters?.performance?.latency),
      power_on: config?.parameters?.config?.power_on ?? true,
      min_bandwidth: config?.parameters?.performance?.min_bandwidth || '',
      max_bandwidth: config?.parameters?.performance?.max_bandwidth || '',
      burst_bandwidth: config?.parameters?.performance?.burst_bandwidth || '',
      min_iops: config?.parameters?.performance?.min_iops || '',
      max_iops: config?.parameters?.performance?.max_iops || '',
      burst_iops: config?.parameters?.performance?.burst_iops || '',
      burst_time: config?.parameters?.performance?.burst_time || '',
      latency: config?.parameters?.performance?.latency || '',
      bctStatus: config?.parameters?.bctStatus === 'true',
      isModify:
        !isEmpty(config?.parameters?.mount_target_host) &&
        config?.parameters?.mount_target_host !== '{}'
    });
  }

  // 标签搜索
  filterChange(e, index, item) {
    item.get('host_id').setValue('');
    if (this.isMySQL) {
      this.getMysqlInstanceOptions(null, null, index, {
        labelCondition: { labelEnvironmentList: e }
      });
    } else if (this.isTDSQL) {
      this.getTdsqlInstanceOptions(null, null, index, {
        labelCondition: { labelList: e }
      });
    } else {
      this.getOracleHostOptions(null, null, index, {
        labelCondition: { labelEnvironmentList: e }
      });
      this.getOracleOriginalInstanceOptions();
    }
  }

  getResourceType() {
    // 注意检查副本详情进入即时挂载和数据利用进入即时挂载
    this.isOracle = !!size(
      intersection(this.componentData.childResourceType, [
        DataMap.Resource_Type.oracle.value,
        DataMap.Resource_Type.oracleCluster.value
      ])
    );
    this.isMySQL = includes(
      this.componentData.childResourceType,
      DataMap.Resource_Type.MySQLInstance.value
    );
    this.isTDSQL = includes(
      this.componentData.childResourceType,
      DataMap.Resource_Type.tdsqlInstance.value
    );
    if (this.isTDSQL) {
      this.scriptTipsLabel = this.i18n.get(
        'common_script_protect_agent_position_label'
      );
    }
  }

  updateIopsItems(value, type: 'min' | 'max' | 'burst') {
    each(this.iopsItems, item => {
      const obj = {};
      obj[type] =
        isNaN(+value) || !trim(value)
          ? '--'
          : Math.round((item.id * value) / 1000000);
      assign(item, obj);
    });
  }

  updateLatencyData(value) {
    each(this.latencyItems, item => {
      const max = isNaN(+value)
        ? '--'
        : Math.round((1000000 * value) / item.id) / 1000 + 'ms';
      assign(item, { max });
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      targetHosts: this.fb.array([this.getTargetHostFormGroup()]),
      targetInstances: this.fb.array([]),
      bindWidthStatus: new FormControl(false),
      iopsStatus: new FormControl(false),
      latencyStatus: new FormControl(false),
      power_on: new FormControl(true),
      min_bandwidth: new FormControl(''),
      max_bandwidth: new FormControl(''),
      burst_bandwidth: new FormControl(''),
      min_iops: new FormControl(''),
      max_iops: new FormControl(''),
      burst_iops: new FormControl(''),
      burst_time: new FormControl(''),
      latency: new FormControl(''),
      pre_script: new FormControl(''),
      post_script: new FormControl(''),
      failed_script: new FormControl(''),
      dbConfig: this.fb.array([]),
      bctStatus: new FormControl(false)
    });
    if (this.isOracle) {
      (this.formGroup.get('targetHosts') as FormArray).clear();
      this.addInstanceRow();
    }
    if (
      this.componentData.childResourceType &&
      this.dataMap.Resource_Type.tdsqlInstance.value.includes(
        this.componentData.childResourceType[0]
      )
    ) {
      this.formGroup.addControl(
        'mysql_port',
        new FormControl('33060', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1024, 65535)
          ]
        })
      );
    }
    this.formGroup.get('bindWidthStatus').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('min_bandwidth').clearValidators();
        this.formGroup.get('max_bandwidth').clearValidators();
        if (this.formGroup.get('burst_bandwidth')) {
          this.formGroup.get('burst_bandwidth').clearValidators();
        }
        if (
          !(
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          )
        ) {
          this.formGroup.get('burst_time').clearValidators();
        }
      } else {
        this.formGroup
          .get('min_bandwidth')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 999999999),
            this.validMinBandWidth()
          ]);
        this.formGroup
          .get('max_bandwidth')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 999999999),
            this.validMaxBandWidth()
          ]);
        this.updateMinAndMaxBandwidthValidity('min_bandwidth');
        if (this.formGroup.get('burst_bandwidth')) {
          this.formGroup
            .get('burst_bandwidth')
            .setValidators([
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 999999999),
              this.validBurstBandWidth()
            ]);
          this.formGroup.get('burst_bandwidth').updateValueAndValidity();
        }

        if (
          (this.formGroup.value.iopsStatus &&
            this.formGroup.value.burst_iops) ||
          (this.formGroup.value.bindWidthStatus &&
            this.formGroup.value.burst_bandwidth)
        ) {
          this.formGroup
            .get('burst_time')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 999999999)
            ]);
        }
      }
      this.formGroup.get('burst_time').updateValueAndValidity();
      this.formGroup.get('min_bandwidth').updateValueAndValidity();
      this.formGroup.get('max_bandwidth').updateValueAndValidity();
    });
    this.formGroup.get('iopsStatus').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('min_iops').clearValidators();
        this.formGroup.get('max_iops').clearValidators();
        if (this.formGroup.get('burst_iops')) {
          this.formGroup.get('burst_iops').clearValidators();
        }
        if (
          !(
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          )
        ) {
          this.formGroup.get('burst_time').clearValidators();
        }
      } else {
        this.formGroup
          .get('min_iops')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(100, 999999999),
            this.validMinIops()
          ]);
        this.formGroup
          .get('max_iops')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(100, 999999999),
            this.validMaxIops()
          ]);
        this.updateMinAndMaxIopsValidity('min_iops');
        if (this.formGroup.get('burst_iops')) {
          this.formGroup
            .get('burst_iops')
            .setValidators([
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(100, 999999999),
              this.validBurstIops()
            ]);
          this.formGroup.get('burst_iops').updateValueAndValidity();
        }

        if (
          (this.formGroup.value.iopsStatus &&
            this.formGroup.value.burst_iops) ||
          (this.formGroup.value.bindWidthStatus &&
            this.formGroup.value.burst_bandwidth)
        ) {
          this.formGroup
            .get('burst_time')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 999999999)
            ]);
        }
      }
      this.formGroup.get('burst_time').updateValueAndValidity();
      this.formGroup.get('min_iops').updateValueAndValidity();
      this.formGroup.get('max_iops').updateValueAndValidity();
    });
    this.formGroup.get('latencyStatus').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('latency').clearValidators();
      } else {
        this.formGroup
          .get('latency')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('latency').updateValueAndValidity();
    });

    this.formGroup
      .get('min_bandwidth')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxBandwidthValidity('min_bandwidth');
          if (
            this.formGroup.value.min_bandwidth &&
            this.formGroup.value.max_bandwidth
          ) {
            this.formGroup.get('max_bandwidth').markAsTouched();
            this.formGroup.get('max_bandwidth').updateValueAndValidity();
            if (this.formGroup.value.burst_bandwidth) {
              this.formGroup.get('burst_bandwidth').markAsTouched();
              this.formGroup.get('burst_bandwidth').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('max_bandwidth')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxBandwidthValidity('max_bandwidth');
          if (
            this.formGroup.value.max_bandwidth &&
            this.formGroup.value.min_bandwidth
          ) {
            this.formGroup.get('min_bandwidth').markAsTouched();
            this.formGroup.get('min_bandwidth').updateValueAndValidity();
            if (this.formGroup.value.burst_bandwidth) {
              this.formGroup.get('burst_bandwidth').markAsTouched();
              this.formGroup.get('burst_bandwidth').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('burst_bandwidth')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (this.formGroup.value.min_bandwidth) {
            this.formGroup.get('min_bandwidth').markAsTouched();
            this.formGroup.get('min_bandwidth').updateValueAndValidity();
          }

          if (this.formGroup.value.max_bandwidth) {
            this.formGroup.get('max_bandwidth').markAsTouched();
            this.formGroup.get('max_bandwidth').updateValueAndValidity();
          }

          if (
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          ) {
            this.formGroup
              .get('burst_time')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(1, 999999999)
              ]);
          } else {
            this.formGroup.get('burst_time').clearValidators();
          }
          this.formGroup.get('burst_time').updateValueAndValidity();
        }, 0);
      });
    this.formGroup
      .get('min_iops')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('min_iops').invalid) {
          this.updateIopsItems('--', 'min');
        } else {
          this.updateIopsItems(res[1], 'min');
        }
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxIopsValidity('min_iops');
          if (this.formGroup.value.min_iops && this.formGroup.value.max_iops) {
            this.formGroup.get('max_iops').markAsTouched();
            this.formGroup.get('max_iops').updateValueAndValidity();
            if (this.formGroup.value.burst_iops) {
              this.formGroup.get('burst_iops').markAsTouched();
              this.formGroup.get('burst_iops').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('max_iops')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('max_iops').invalid) {
          this.updateIopsItems('--', 'max');
        } else {
          this.updateIopsItems(res[1], 'max');
        }
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxIopsValidity('max_iops');
          if (this.formGroup.value.max_iops && this.formGroup.value.min_iops) {
            this.formGroup.get('min_iops').markAsTouched();
            this.formGroup.get('min_iops').updateValueAndValidity();
            if (this.formGroup.value.burst_iops) {
              this.formGroup.get('burst_iops').markAsTouched();
              this.formGroup.get('burst_iops').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('burst_iops')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('burst_iops').invalid) {
          this.updateIopsItems('--', 'burst');
        } else {
          this.updateIopsItems(res[1], 'burst');
        }
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (this.formGroup.value.min_iops) {
            this.formGroup.get('min_iops').markAsTouched();
            this.formGroup.get('min_iops').updateValueAndValidity();
          }
          if (this.formGroup.value.max_iops) {
            this.formGroup.get('max_iops').markAsTouched();
            this.formGroup.get('max_iops').updateValueAndValidity();
          }

          if (
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          ) {
            this.formGroup
              .get('burst_time')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(1, 999999999)
              ]);
          } else {
            this.formGroup.get('burst_time').clearValidators();
          }
          this.formGroup.get('burst_time').updateValueAndValidity();
        }, 0);
      });

    this.formGroup.statusChanges.subscribe(res => {
      this.selectMountOptionChange.emit(res === 'VALID');
    });

    this.globalService
      .getState(LiveMountAction.SelectResource)
      .subscribe(res => {
        this.formGroup
          .get('pre_script')
          .setValidators([
            this.baseUtilService.VALID.maxLength(8192),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.linuxScript,
              false
            )
          ]);
        this.formGroup
          .get('post_script')
          .setValidators([
            this.baseUtilService.VALID.maxLength(8192),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.linuxScript,
              false
            )
          ]);
        this.formGroup
          .get('failed_script')
          .setValidators([
            this.baseUtilService.VALID.maxLength(8192),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.linuxScript,
              false
            )
          ]);
        this.formGroup.get('pre_script').updateValueAndValidity();
        this.formGroup.get('post_script').updateValueAndValidity();
        this.formGroup.get('failed_script').updateValueAndValidity();
      });

    this.formGroup
      .get('latency')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('latency').invalid) {
          this.updateLatencyData('--');
        } else {
          this.updateLatencyData(res[1]);
        }
      });
  }

  updateMinAndMaxBandwidthValidity(focusKey) {
    this.formGroup.get('min_bandwidth').clearValidators();
    this.formGroup.get('max_bandwidth').clearValidators();
    if (
      (focusKey === 'min_bandwidth' &&
        !trim(this.formGroup.value.max_bandwidth)) ||
      (focusKey === 'max_bandwidth' &&
        !trim(this.formGroup.value.max_bandwidth) &&
        trim(this.formGroup.value.min_bandwidth))
    ) {
      this.formGroup
        .get('min_bandwidth')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMinBandWidth()
        ]);
      this.formGroup
        .get('max_bandwidth')
        .setValidators([
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMaxBandWidth()
        ]);
    } else {
      this.formGroup
        .get('min_bandwidth')
        .setValidators([
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMinBandWidth()
        ]);
      this.formGroup
        .get('max_bandwidth')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMaxBandWidth()
        ]);
    }
    this.formGroup.get('min_bandwidth').updateValueAndValidity();
    this.formGroup.get('max_bandwidth').updateValueAndValidity();
  }

  updateMinAndMaxIopsValidity(focusKey) {
    this.formGroup.get('min_iops').clearValidators();
    this.formGroup.get('max_iops').clearValidators();
    if (
      (focusKey === 'min_iops' && !trim(this.formGroup.value.max_iops)) ||
      (focusKey === 'max_iops' &&
        !trim(this.formGroup.value.max_iops) &&
        trim(this.formGroup.value.min_iops))
    ) {
      this.formGroup
        .get('min_iops')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMinIops()
        ]);
      this.formGroup
        .get('max_iops')
        .setValidators([
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMaxIops()
        ]);
    } else {
      this.formGroup
        .get('min_iops')
        .setValidators([
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMinIops()
        ]);
      this.formGroup
        .get('max_iops')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMaxIops()
        ]);
    }
    this.formGroup.get('min_iops').updateValueAndValidity();
    this.formGroup.get('max_iops').updateValueAndValidity();
  }

  validTargetHost(flag = false): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup || !this.validRepeatHostFlag) {
        return null;
      }
      // oracle和其他数据库取得不是同一个formGroup
      return find(
        !!flag
          ? this.formGroup.get('targetInstances').value
          : this.formGroup.get('targetHosts').value,
        host => {
          return host.host_id === control.value;
        }
      )
        ? { invalidSameHost: { value: control.value } }
        : null;
    };
  }

  asyncValidSameNameDB() {
    return (
      control: AbstractControl
    ): Promise<{ [key: string]: any } | null> => {
      return new Promise(resolve => {
        this.protectedResourceApiService
          .ListResources({
            pageNo: 0,
            pageSize: CommonConsts.PAGE_SIZE * 5,
            akLoading: false,
            conditions: JSON.stringify({
              subType: [
                DataMap.Resource_Type.oracle.value,
                DataMap.Resource_Type.oracleCluster.value
              ],
              rootUuid: control.value
            })
          })
          .subscribe(res => {
            if (this.isDrill) {
              resolve(null);
            }
            const normalDB = find(res.records, item => {
              return (
                item.extendInfo?.linkStatus ===
                  DataMap.resource_LinkStatus_Special.normal.value &&
                toLower(this.componentData.selectionResource.resource_name) ===
                  toLower(item.name)
              );
            });
            const offlineDB = find(res.records, item => {
              return (
                item.extendInfo?.linkStatus ===
                  DataMap.resource_LinkStatus_Special.offline.value &&
                toLower(this.componentData.selectionResource.resource_name) ===
                  toLower(item.name)
              );
            });
            const hostName = find(this.targetHostOptions, {
              key: control.value
            })?.label;

            this.oracleOfflineWarnTip = this.i18n.get(
              'protection_oracle_target_host_offline_db_label',
              [hostName, this.componentData.selectionResource?.resource_name]
            );

            if (normalDB) {
              this.targetHostErrorTip = assign({}, this.targetHostErrorTip, {
                invalidSameDb: this.i18n.get(
                  'protection_target_host_oneline_db_label',
                  [hostName, normalDB.name]
                )
              });
              resolve({ invalidSameDb: { value: control.value } });
            }
            this.offlineWarnTip = offlineDB
              ? this.i18n.get('protection_target_host_offline_db_label', [
                  hostName,
                  offlineDB.name
                ])
              : undefined;
            resolve(null);
          });
      });
    };
  }

  validMinBandWidth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.max_bandwidth)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.max_bandwidth)
      ) {
        return null;
      }

      return +control.value <= +this.formGroup.value.max_bandwidth
        ? null
        : {
            invalidMin: { value: control.value }
          };
    };
  }

  validMaxBandWidth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.min_bandwidth)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.min_bandwidth)
      ) {
        return null;
      }

      return +control.value >= +this.formGroup.value.min_bandwidth
        ? null
        : {
            invalidMax: { value: control.value }
          };
    };
  }

  validBurstBandWidth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (!trim(control.value)) {
        return null;
      }

      if (
        !isNumber(+control.value) ||
        isEmpty(toString(this.formGroup.value.max_bandwidth)) ||
        !isNumber(+this.formGroup.value.max_bandwidth)
      ) {
        return null;
      }

      return +control.value > +this.formGroup.value.max_bandwidth
        ? null
        : {
            invalidBurst: { value: control.value }
          };
    };
  }

  validMinIops(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.max_iops)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.max_iops)
      ) {
        return null;
      }

      return +control.value <= +this.formGroup.value.max_iops
        ? null
        : {
            invalidMin: { value: control.value }
          };
    };
  }

  validMaxIops(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.min_iops)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.min_iops)
      ) {
        return null;
      }

      return +control.value >= +this.formGroup.value.min_iops
        ? null
        : {
            invalidMax: { value: control.value }
          };
    };
  }

  validBurstIops(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (!trim(control.value)) {
        return null;
      }

      if (
        !isNumber(+control.value) ||
        isEmpty(toString(this.formGroup.value.max_iops)) ||
        !isNumber(+this.formGroup.value.max_iops)
      ) {
        return null;
      }

      return +control.value > +this.formGroup.value.max_iops
        ? null
        : {
            invalidBurst: { value: control.value }
          };
    };
  }

  validRepeatInstance(index: number): ValidatorFn {
    // index用于标识当前是在targetInstances的第几项
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const instArr = this.targetInstances
        .at(index)
        .get('instance_array') as FormArray;
      // 从array中取出每个targetInstance
      const filterArr = map(
        instArr.controls,
        ctrl => ctrl.get('target_inst').value
      ).filter(target => !!target);
      // 根据出现的次数进行分组
      const countObj = countBy(filterArr);
      // 取出出现次数大于1的key作为新数组
      const repeatInstances = filter(
        uniq(filterArr),
        item => countObj[item] > 1
      );
      if (includes(repeatInstances, control.value)) {
        assign(this.instanceInputErrorTip, {
          repeatInstance: this.i18n.get('protection_same_instance_tips_label', [
            control.value
          ])
        });
        return { repeatInstance: control.value };
      }
      return null;
    };
  }

  getTargetHostOptions(recordsTemp?, startPage?, index?, labelParams?) {
    this.targetHostOptions = [];

    if (this.isMySQL) {
      this.getMysqlInstanceOptions(null, null, index, labelParams);
    } else if (this.isTDSQL) {
      this.getTdsqlInstanceOptions(null, null, index, labelParams);
    } else {
      this.getOracleHostOptions(null, null, index, labelParams);
      this.getOracleOriginalInstanceOptions();
    }
  }

  getProxyOptions(recordsTemp?, startPage?, index?, labelParams?: any) {
    const conditions = {
      type: 'Host',
      subType: [DataMap.Resource_Type.UBackupAgent.value],
      scenario: [['!='], '1'],
      isCluster: [['=='], false]
    };
    extendParams(conditions, labelParams);
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        const hostArray = [];
        each(recordsTemp, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            ip: item.endpoint,
            isLeaf: true
          });
        });
        const arr = filter(hostArray, item => {
          return includes(this.targetTdsqlInstanceIps, item?.endpoint);
        });

        if (isUndefined(index)) {
          this.targetHostOptions = arr;
          (this.formGroup.get('targetHosts') as FormArray)
            .at(0)
            .get('hostOptions')
            .setValue(this.targetHostOptions);
        } else {
          (this.formGroup.get('targetHosts') as FormArray)
            .at(index)
            .get('hostOptions')
            .setValue(arr);
        }

        // 演练计划回显
        if (isUndefined(index)) {
          this.updateDrillData();
        }
        return;
      }
      this.getProxyOptions(recordsTemp, startPage, index, labelParams);
    });
  }

  getTdsqlInstanceOptions(recordsTemp?, startPage?, index?, labelParams?: any) {
    const conditions = {
      subType: [
        DataMap.Resource_Type.tdsqlInstance.value,
        DataMap.Resource_Type.tdsqlDistributedInstance.value
      ]
    };
    extendParams(conditions, labelParams);
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        each(recordsTemp, item => {
          this.targetTdsqlInstanceIps = [
            ...this.targetTdsqlInstanceIps,
            ...split(get(item, 'path', ''), ',')
          ];
        });
        this.targetTdsqlInstanceIps = uniq(this.targetTdsqlInstanceIps);
        this.getProxyOptions(null, null, index, labelParams);

        return;
      }
      this.getTdsqlInstanceOptions(recordsTemp, startPage, index, labelParams);
    });
  }

  getMysqlInstanceOptions(recordsTemp?, startPage?, index?, labelParams?: any) {
    const conditions = {
      subType: DataMap.Resource_Type.MySQLInstance.value,
      isTopInstance: [['=='], '1']
    };
    extendParams(conditions, labelParams);
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        const optionsArray = [];
        each(recordsTemp, item => {
          optionsArray.push({
            ...item,
            key: item.uuid,
            label: `${item.name}(${item.environment?.endpoint})`,
            ip: item.environment?.endpoint,
            isLeaf: true
          });
        });
        this.targetHostOptions = optionsArray;
        if (isUndefined(index)) {
          this.targetHostOptions = optionsArray;
          (this.formGroup.get('targetHosts') as FormArray)
            .at(0)
            .get('hostOptions')
            .setValue(this.targetHostOptions);
        } else {
          (this.formGroup.get('targetHosts') as FormArray)
            .at(index)
            .get('hostOptions')
            .setValue(optionsArray);
        }
        // 演练回显
        if (isUndefined(index)) {
          this.updateDrillData();
        }

        const resourceData = JSON.parse(
          get(this.componentData, 'selectionCopy.resource_properties') ||
            get(this.componentData, 'rowCopy.resource_properties')
        );

        if (!!resourceData) {
          this.targetHostOptions = filter(this.targetHostOptions, item => {
            let sameVaild = true;
            if (
              !!resourceData?.version &&
              resourceData?.version !== item?.version &&
              !endsWith(resourceData?.version, item?.version) &&
              !endsWith(item?.version, resourceData?.version)
            ) {
              sameVaild = false;
            }

            if (
              !!resourceData?.extendInfo?.deployOperatingSystem &&
              resourceData?.extendInfo?.deployOperatingSystem !==
                item?.extendInfo?.deployOperatingSystem
            ) {
              sameVaild = false;
            }

            return sameVaild;
          });
        }
        return;
      }
      this.getMysqlInstanceOptions(recordsTemp, startPage, index, labelParams);
    });
  }

  getOracleHostOptions(recordsTemp?, startPage?, index?, labelParams?: any) {
    const conditions = {
      subType:
        this.componentData.selectionResource?.resource_sub_type ===
        DataMap.Resource_Type.oracle.value
          ? [DataMap.Resource_Type.oracle.value]
          : [
              DataMap.Resource_Type.oracle.value,
              DataMap.Resource_Type.oracleCluster.value
            ]
    };
    extendParams(conditions, labelParams);

    this.protectedResourceApiService
      .ListResources({
        pageSize: CommonConsts.PAGE_SIZE,
        pageNo: startPage || 0,
        conditions: JSON.stringify(conditions)
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
          res.totalCount === 0
        ) {
          recordsTemp = uniqBy(recordsTemp, 'rootUuid');
          // 过滤掉原主机
          recordsTemp = reject(recordsTemp, item => {
            return (
              item.rootUuid ===
              this.componentData.selectionResource?.environment_uuid
            );
          });
          // 创建的场景
          if (this.componentData.selectionResource?.resource_properties) {
            const resourceProperties = JSON.parse(
              this.componentData.selectionResource?.resource_properties
            );
            recordsTemp = reject(recordsTemp, item => {
              return item.rootUuid === resourceProperties.environment_uuid;
            });
          }
          // oracle即时挂载不支持Windows主机
          recordsTemp = reject(
            recordsTemp,
            item => item.environment?.osType === DataMap.Os_Type.windows.value
          );
          let options = [];
          options = map(recordsTemp, item => {
            return assign(item, {
              isLeaf: true,
              key: item.rootUuid,
              value: item.rootUuid || item.parentUuid,
              label: `${item.environment?.name}(${item.environment?.endpoint})`
            });
          });

          if (isUndefined(index)) {
            this.targetHostOptions = options;
            this.targetInstances
              .at(0)
              .get('hostOptions')
              .setValue(this.targetHostOptions);
          } else {
            this.targetInstances
              .at(index)
              .get('hostOptions')
              .setValue(options);
          }

          this.selectMountOptionChange.emit(this.formGroup.valid);
          // 演练回显数据
          if (isUndefined(index)) {
            this.updateDrillData();
          }
          return;
        }
        this.getOracleHostOptions(recordsTemp, startPage, index, labelParams);
      });
  }

  getOracleOriginalInstanceOptions() {
    const { selectionResource } = this.componentData;
    const { resource_sub_type, resource_properties } = selectionResource;
    const properties = JSON.parse(resource_properties || '{}') || {};
    if (resource_sub_type === DataMap.Resource_Type.oracle.value) {
      this.originalInstanceOptions = this.formatSelectionOptions(
        [properties.extendInfo],
        'inst_name'
      );
    } else {
      const instancesArr = JSON.parse(
        get(properties, 'extendInfo.instances', '[]')
      );
      this.originalInstanceOptions = this.formatSelectionOptions(
        instancesArr,
        'inst_name'
      );
    }
  }

  getTargetHostFormGroup() {
    return this.fb.group({
      host_id: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validTargetHost()
        ],
        asyncValidators: [this.asyncValidSameNameDB()],
        updateOn: 'change'
      }),
      ip: new FormControl(''),
      version: new FormControl(''),
      searchKey: new FormControl('label'),
      searchValue: new FormControl(''),
      searchSelect: new FormControl([]),
      hostOptions: new FormControl(this.targetHostOptions)
    });
  }

  // 外层FormArray结构为[host_id,version,options,instance_array]
  // 其中instance_array结构为[origin_inst,ip,target_inst]
  // 监听其中origin_inst,改变时修改外层的options
  getTargetLocationFormGroup() {
    this.targetFormGroup = this.fb.group({
      host_id: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validTargetHost(true)
        ],
        asyncValidators: [this.asyncValidSameNameDB()]
      }),
      version: new FormControl(''),
      options: new FormControl([]),
      instance_array: this.fb.array([]),
      searchKey: new FormControl('label'),
      searchValue: new FormControl(''),
      searchSelect: new FormControl([]),
      hostOptions: new FormControl(this.targetHostOptions)
    });
    return this.targetFormGroup;
  }

  getTargetInstanceFormGroup(index: number, data) {
    const formGroup = this.fb.group({
      agent_id: new FormControl(data.rootUuid),
      origin_inst: new FormControl('', this.baseUtilService.VALID.required()),
      ip: new FormControl(data['path'] || data['endpoint']),
      target_inst: new FormControl('', [
        this.baseUtilService.VALID.name(RegExp("^[^|;&$><`'@!+\\n]*$"), false),
        this.validRepeatInstance(index)
      ])
    });
    formGroup.get('origin_inst').valueChanges.subscribe(res => {
      // 通过parent去获取当前formArray选中了哪些数据，此时选中的数据还不会显示在界面上
      const rowSelection = map(
        formGroup.parent?.controls,
        (ctrl: AbstractControl) => {
          return ctrl.get('origin_inst').value;
        }
      ).filter(item => !!item);
      // 获取当前使用的options，根据选中的数据去禁用
      const options = cloneDeep(
        this.targetInstances.at(index).get('options').value
      );
      each(
        options,
        item => (item.disabled = includes(rowSelection, item.value))
      );
      // 返回最新的数据
      this.targetInstances
        .at(index)
        .get('options')
        .setValue(options);
    });
    return formGroup;
  }

  targetHostChange(index, value) {
    const host = includes(
      [
        DataMap.Resource_Type.oracle.value,
        DataMap.Resource_Type.oracleCluster.value
      ],
      this.componentData.selectionResource?.resource_sub_type
    )
      ? find(this.targetHostOptions, { rootUuid: value })
      : find(this.targetHostOptions, { uuid: value });
    if (!host) {
      return '--';
    }

    (this.formGroup.get('targetHosts') as FormArray)
      .at(index)
      .get('ip')
      .setValue(host.ip || host.environment?.endpoint || '--');
    (this.formGroup.get('targetHosts') as FormArray)
      .at(index)
      .get('version')
      .setValue(host.version || '--');
  }

  targetHostOpenChange(index, value) {}

  addRow() {
    (this.formGroup.get('targetHosts') as FormArray).push(
      this.getTargetHostFormGroup()
    );
    this.validRepeatHostFlag = true;
  }

  removeRow(i) {
    (this.formGroup.get('targetHosts') as FormArray).removeAt(i);
  }

  get targetHosts() {
    return (this.formGroup.get('targetHosts') as FormArray).controls;
  }

  get targetInstances() {
    return this.formGroup.get('targetInstances') as FormArray;
  }

  addInstanceRow() {
    const instanceItem = this.getTargetLocationFormGroup();
    instanceItem.get('host_id').valueChanges.subscribe(res => {
      const index = this.targetInstances.controls.indexOf(instanceItem);

      // 选标签时会清空选择的主机，oracle主机的相关联动也需清空
      if (!res) {
        (this.targetInstances
          .at(index)
          .get('instance_array') as FormArray).clear();
      }

      this.formatTableDataById(res, index);
    });

    this.targetInstances.push(instanceItem);
  }

  isLiveMountCopy(): boolean {
    if (!isEmpty(this.componentData.rowCopy)) {
      return (
        this.componentData.rowCopy.generated_by ===
        DataMap.CopyData_generatedType.liveMount.value
      );
    }
    if (!isEmpty(this.componentData.selectionCopy)) {
      return (
        this.componentData.selectionCopy.generated_by ===
        DataMap.CopyData_generatedType.liveMount.value
      );
    }
    return false;
  }

  getFilesystemName(): string {
    let fileSystemName;
    try {
      if (!isEmpty(this.componentData.rowCopy)) {
        fileSystemName = JSON.parse(this.componentData.rowCopy.properties)
          ?.fileSystemShareInfo[0]?.fileSystemName;
      }
      if (!isEmpty(this.componentData.selectionCopy)) {
        fileSystemName = JSON.parse(this.componentData.selectionCopy.properties)
          ?.fileSystemShareInfo[0]?.fileSystemName;
      }
    } catch (error) {
      fileSystemName = '';
    }
    return fileSystemName;
  }

  updateDrillClusterInstance(drillItem, agent, form) {
    if (this.isDrill && !isEmpty(drillItem)) {
      const single = find(drillItem, { agent_id: agent.rootUuid });
      form.patchValue({
        origin_inst: single?.src_instance_name,
        target_inst: single?.instance_name
      });
    }
  }

  formatCluster(root_uuid, index, instArr, drillItem) {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: root_uuid
      })
      .subscribe(res => {
        const dependencies = map(get(res, 'dependencies.agents', []));
        if (!isEmpty(dependencies)) {
          // queryDependency查回来的数据中有些dependencies为空会导致异常
          // 所以这里需要手动查询一次
          each(dependencies, agent => {
            const form = this.getTargetInstanceFormGroup(index, agent);
            this.updateDrillClusterInstance(drillItem, agent, form);
            instArr.push(form);
          });
        }
      });
  }

  formatTableDataById(root_uuid: string, index: number, drillItem?) {
    if (!root_uuid || index === -1) {
      return;
    }
    const selectedHost = find(this.targetHostOptions, { value: root_uuid });
    const instArr = this.targetInstances
      .at(index)
      .get('instance_array') as FormArray;
    this.targetInstances
      .at(index)
      .get('version')
      .setValue(selectedHost.version);
    this.targetInstances
      .at(index)
      .get('options')
      .setValue(cloneDeep(this.originalInstanceOptions));
    // 切换目标位置时先把原来的数据清空
    instArr.clear();
    if (selectedHost.subType === DataMap.Resource_Type.oracle.value) {
      const formTemp = this.getTargetInstanceFormGroup(index, selectedHost);
      this.updateDrillInstance(formTemp, root_uuid, drillItem);
      instArr.push(formTemp);
      return;
    }
    this.formatCluster(root_uuid, index, instArr, drillItem);
  }

  updateDrillInstance(formTemp, root_uuid, drillItem) {
    if (this.isDrill && !isEmpty(drillItem)) {
      formTemp.patchValue({
        agent_id: root_uuid,
        ip: drillItem[0]?.agent_ip,
        origin_inst: drillItem[0]?.src_instance_name,
        target_inst: drillItem[0]?.instance_name
      });
    }
  }

  formatSelectionOptions(data, targetKey) {
    const options: OptionItem[] = [];
    if (isArray(data)) {
      each(data, item => {
        options.push({
          value: item[targetKey],
          label: item[targetKey],
          isLeaf: true
        });
      });
    }
    return options;
  }

  getTargetLocation(): string {
    const hostArr = this.isOracle
      ? this.formGroup.value.targetInstances
      : this.formGroup.value.targetHosts;
    const paths = map(hostArr, item => {
      const targetHost = find(this.targetHostOptions, host => {
        return host.key === item.host_id;
      });
      return targetHost ? `${targetHost?.name}(${targetHost?.path})` : '';
    });
    return paths.join(',');
  }

  getComponentData() {
    const mountTargetHost = {};
    const dbConfigControls = (this.formGroup.get('dbConfig') as FormArray)
      .controls;
    dbConfigControls.forEach(control => {
      mountTargetHost[control.value.key] = control.value.newParam
        ? control.value.newParam
        : control.value.originParam;
    });
    const targetHostList = [];
    const target_resource_uuid_list = [];
    const hostArr = this.isOracle
      ? this.formGroup.value.targetInstances
      : this.formGroup.value.targetHosts;
    each(hostArr, res => {
      const targetHost = find(this.targetHostOptions, host => {
        return host.key === res.host_id;
      });
      if (targetHost) {
        targetHostList.push({
          name: targetHost.name,
          ip: targetHost.ip || targetHost.environment?.endpoint,
          version: targetHost.version
        });
      }
      target_resource_uuid_list.push(
        this.isMySQL ? res.host_id : targetHost?.uuid
      );
    });

    assign(this.componentData.requestParams, {
      target_resource_uuid_list,
      target_location: MountTargetLocation.Others
    });

    const parameters = {
      performance: {},
      mount_target_host: JSON.stringify(mountTargetHost),
      config: {
        power_on: this.formGroup.value.power_on
      }
    };
    if (this.isOracle) {
      const instancesArr = [];
      each(this.targetInstances.value, (item, index) => {
        each(item.instance_array, instance => {
          instancesArr.push({
            agent_ip: instance.ip,
            agent_id: instance.agent_id,
            instance_name: instance.target_inst,
            target_object_id: find(this.targetHostOptions, {
              value: item.host_id
            })['uuid'],
            src_instance_name: get(instance, 'origin_inst', '')
          });
        });
      });
      assign(parameters, {
        instances: JSON.stringify(instancesArr)
      });
    }
    const advanceParameters = omit(this.formGroup.value, [
      'targetHosts',
      'targetInstances',
      'bindWidthStatus',
      'iopsStatus',
      'latencyStatus',
      'isModify',
      'power_on'
    ]);
    each(advanceParameters, (v, k) => {
      if (isEmpty(trim(String(v)))) {
        return;
      }

      if (!includes(['pre_script', 'post_script', 'failed_script'], k)) {
        if (
          includes(
            [DataMap.Resource_Type.tdsqlInstance.value],
            first(this.componentData.childResourceType)
          ) &&
          k === 'mysql_port'
        ) {
          parameters[k] = v;
        } else {
          parameters.performance[k] = +v;
        }
      } else {
        parameters[k] = v;
      }
    });

    if (!isEmpty(parameters)) {
      delete parameters.performance['dbConfig'];
      delete parameters.performance['bctStatus'];
      assign(this.componentData.requestParams, {
        parameters
      });
    }

    if (
      includes(
        [
          DataMap.Resource_Type.MySQL.value,
          DataMap.Resource_Type.MySQLCluster.value,
          DataMap.Resource_Type.MySQLClusterInstance.value,
          DataMap.Resource_Type.MySQLClusterInstanceNode.value,
          DataMap.Resource_Type.MySQLDatabase.value,
          DataMap.Resource_Type.MySQLInstance.value
        ],
        first(this.componentData.childResourceType)
      )
    ) {
      assign(parameters, {
        preScript: this.formGroup.value.pre_script || '',
        postScript: this.formGroup.value.post_script || '',
        failPostScript: this.formGroup.value.failed_script || ''
      });
      delete parameters['pre_script'];
      delete parameters['post_script'];
      delete parameters['failed_script'];

      return assign(this.componentData, {
        requestParams: {
          source_resource_id: this.componentData.requestParams
            .source_resource_id,
          copy_id: this.componentData.requestParams.copy_id,
          target_resource_uuid_list: [
            ...this.componentData.requestParams.target_resource_uuid_list
          ],
          target_location: this.componentData.requestParams.target_location,
          name: `mysql_mount_${Date.now()}`,
          file_system_share_info_list: [
            {
              fileSystemName: `mysql_mount_${Date.now()}`,
              type: 1,
              accessPermission: 1,
              advanceParams: {
                clientType: 0,
                clientName: '*',
                squash: 1,
                rootSquash: 1,
                portSecure: 1
              }
            }
          ],
          parameters: parameters
        },
        selectionMount: assign({ targetHostList }, this.formGroup.value)
      });
    } else if (
      includes(
        [DataMap.Resource_Type.tdsqlInstance.value],
        first(this.componentData.childResourceType)
      )
    ) {
      return assign(this.componentData, {
        requestParams: {
          source_resource_id: this.componentData.requestParams
            .source_resource_id,
          copy_id: this.componentData.requestParams.copy_id,
          target_resource_uuid_list: [
            ...this.componentData.requestParams.target_resource_uuid_list
          ],
          target_location: this.componentData.requestParams.target_location,
          name: `tdsql_mount_${Date.now()}`,
          file_system_share_info_list: [
            {
              fileSystemName: `tdsql_mount_${Date.now()}`,
              type: 1,
              accessPermission: 1,
              advanceParams: {
                clientType: 0,
                clientName: '*',
                squash: 1,
                rootSquash: 1,
                portSecure: 1
              }
            }
          ],
          parameters: parameters
        },
        selectionMount: assign({ targetHostList }, this.formGroup.value)
      });
    } else {
      assign(parameters, {
        isStartDB: this.formGroup.value.power_on ? 1 : 0,
        preScript: this.formGroup.value.pre_script || '',
        postScript: this.formGroup.value.post_script || '',
        failPostScript: this.formGroup.value.failed_script || '',
        bctStatus: this.formGroup.value.bctStatus ? 'true' : 'false'
      });
      delete parameters['pre_script'];
      delete parameters['post_script'];
      delete parameters['failed_script'];
      return assign(this.componentData, {
        requestParams: {
          source_resource_id: this.componentData.requestParams
            .source_resource_id,
          copy_id: this.componentData.requestParams.copy_id,
          policy_id: this.componentData.requestParams.policy_id,
          target_resource_uuid_list: [
            ...this.componentData.requestParams.target_resource_uuid_list
          ],
          target_location: this.componentData.requestParams.target_location,
          name:
            this.isLiveMountCopy() && this.getFilesystemName()
              ? this.getFilesystemName()
              : `oracle_mount_${Date.now()}`,
          file_system_share_info_list: [
            {
              fileSystemName:
                this.isLiveMountCopy() && this.getFilesystemName()
                  ? this.getFilesystemName()
                  : `oracle_mount_${Date.now()}`,
              type: 1,
              accessPermission: 1,
              advanceParams: {
                clientType: 0,
                clientName: '*',
                squash: 1,
                rootSquash: 1,
                portSecure: 1
              }
            }
          ],
          parameters: parameters
        },
        selectionMount: assign({ targetHostList }, this.formGroup.value)
      });
    }
  }
}
