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
import { Component, Input, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup
} from '@angular/forms';
import { MessageService, ModalRef, OptionItem } from '@iux/live';
import { TargetClusterComponent } from 'app/business/system/infrastructure/cluster-management/target-cluster/target-cluster.component';
import {
  ApplicationType,
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  DaysOfType,
  DmeServiceService,
  I18NService,
  MultiCluster,
  NasDistributionStoragesApiService,
  OpHcsServiceApiService,
  PeriodBackupMode,
  ProtectResourceAction,
  QosService,
  ReplicationModeType,
  ScheduleTrigger,
  StoragesApiService,
  StorageUnitService,
  StorageUserAuthService,
  UsersApiService,
  WarningMessageService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  first,
  flatMapDeep,
  get,
  includes,
  intersection,
  isEmpty,
  isUndefined,
  map,
  set,
  size,
  some,
  toLower,
  toNumber,
  toString,
  uniqBy
} from 'lodash';

@Component({
  selector: 'aui-replication-policy',
  templateUrl: './replication-policy.component.html',
  styleUrls: ['./replication-policy.component.less'],
  providers: [DatePipe]
})
export class ReplicationPolicyComponent implements OnInit {
  @Input() activeIndex: number;
  @Input() action: any;
  @Input() data: any;
  @Input() backupData: any;
  @Input() formGroup: FormGroup;
  @Input() type;

  // X3000
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isDmeUser = this.cookieService.get('userType') === CommonConsts.DME_USER_TYPE;

  MultiCluster = MultiCluster;

  // 保留70年
  maxRetentionDay = 25550;
  maxRetentionWeek = 3650;
  maxRetentionMonth = 840;
  maxRetentionYear = 70;

  durationUnitDisable: boolean = false;
  index = 1;
  dataNum = 1;
  logNum = 0; // 用于记录现有的日志副本策略的数量
  dataMap = DataMap;
  replicationModeType = ReplicationModeType;
  applicationType = ApplicationType;
  qosNames = [];
  oldExternalSystems = [];
  externalSystems = [];
  sourceClusterIp;
  hcsReplicationClusterOptions = [];
  allReplicationClusterOptions = [];
  vdcTenantOptions = [];
  projectOptions = [];
  dwsCrossDomain;
  dwsGroup = false;
  dwsParallel = false;
  externalStorage = [];
  externalStorageUnit = [];
  externalSystemDisable = false;
  timePickerDisable = false;
  isDisabled = false;
  isReplica = false; // 判断是否是复制副本
  isDisableLog = true; // 判断是否放开日志复制
  language = this.i18n.language;
  scheduleTrigger = ScheduleTrigger;
  periodBackupMode = PeriodBackupMode;
  protectResourceAction = ProtectResourceAction;
  retentionDurationTips: string;
  deviceTypeOptions = [];
  deviceTypeUseOption = [];
  targetZoneOps = [];
  resourceOps = [];
  isDistributed = this.appUtilsService.isDistributed;
  isDecouple = this.appUtilsService.isDecouple;
  intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 23])
  });
  retentionDurationErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [
      1,
      this.maxRetentionDay
    ])
  };
  startTimeErrorTip = assign({}, this.baseUtilService.requiredErrorTip);
  intervalUnit = this.dataMapService
    .toArray('Interval_Unit')
    .filter((v: OptionItem) => {
      return v.value !== 'y' && v.value !== 'p' && v.value !== 'MO';
    })
    .filter((v: OptionItem) => {
      return (v.isLeaf = true);
    });
  retentionDurations = this.dataMapService
    .toArray('Interval_Unit')
    .filter((v: OptionItem) => {
      return v.value !== 'm' && v.value !== 'h';
    })
    .filter((v: OptionItem) => {
      return (v.isLeaf = true);
    });
  yearTimeRangeOptions = this.dataMapService
    .toArray('Year_Time_Range')
    .filter((v: OptionItem) => (v.isLeaf = true));
  monthTimeRangeOptions = this.dataMapService
    .toArray('Month_Time_Range')
    .filter((v: OptionItem) => (v.isLeaf = true));
  weekTimeRangeOptions = this.dataMapService
    .toArray('Days_Of_Week')
    .filter((v: OptionItem) => (v.isLeaf = true));
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
  backupStorageTypesAll = this.dataMapService.toArray('backupStorageTypeSla');
  backupStorageTypesWithoutGroup = this.backupStorageTypesAll.filter(type => {
    return !(type.value === DataMap.backupStorageTypeSla.group.value);
  });

  specifyUserOptionsMap = {};
  tmpSpecifyUserInfo = {};
  externalStorageMap = {};
  externalStorageUnitMap = {};
  externalStorageUnitMaintainMap = {}; // 用于保存筛选前的所有单元数据
  crossDomainStorageMap = {
    replication: {},
    replication_log: {}
  }; // 用于保存已经使用的系统单元组合
  localStorageType; // 用于本地存储单元类型
  _includes = includes;
  isTargetMulti = false; // 用于判断对端是否为多集群

  generate_time_range_year = DataMap.Year_Time_Range.December.value;
  generate_time_range_week = DataMap.Days_Of_Week.mon.value;
  wormData = 0; // 最大的worm时间
  wormDataList = []; // 备份策略worm时间组
  limitRatePolicyTip = this.i18n.get('common_limit_rate_policy_tip_label');

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public modal: ModalRef,
    public datePipe: DatePipe,
    private cookieService: CookieService,
    private qosServiceApi: QosService,
    private messageService: MessageService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private clusterApiService: ClustersApiService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private infoMessageService: InfoMessageService,
    private nasDistributionStoragesApiService: NasDistributionStoragesApiService,
    public storageApiService: StoragesApiService,
    private opHcsServiceApiService: OpHcsServiceApiService,
    private usersApiService: UsersApiService,
    private storageUserAuthService: StorageUserAuthService,
    private storageUnitService: StorageUnitService,
    public appUtilsService: AppUtilsService,
    private dmeService: DmeServiceService
  ) {}

  ngOnInit() {
    if (this.isDistributed) {
      this.limitRatePolicyTip += this.i18n.get(
        'common_limit_rate_policy_tip_distributed_label'
      );
    }
    this.initDeviceOption();
    this.getLocalStorageType();
    this.isReplica = this.type === DataMap.Resource_Type.Replica.value;
    this.isDisabled = includes(
      [
        DataMap.Resource_Type.GaussDB_DWS.value,
        DataMap.Resource_Type.Replica.value
      ],
      this.type
    );
    this.isDisableLog =
      !includes(
        [
          DataMap.Resource_Type.SQLServer.value,
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.DB2.value
        ],
        this.type
      ) ||
      this.appUtilsService.isDecouple ||
      this.isHcsUser ||
      this.isDmeUser;
    this.updateForm();
    if (
      this.type === DataMap.Resource_Type.GaussDB_DWS.value &&
      get(first(this.backupData), 'ext_parameters.storage_id') &&
      get(first(this.backupData), 'ext_parameters.storage_type') ===
        DataMap.backupStorageType.group.value
    ) {
      this.dwsGroup = true;
      this.getBackupStorageNames(() => this.initRest());
    } else {
      this.initRest();
    }
    if (this.isDmeUser) {
      this.getTargetZoneOps();
      this.getResourceOps();
    }
    this.getMaxWormDuration();
  }

  initRest() {
    this.dwsCrossDomain =
      this.type === DataMap.Resource_Type.GaussDB_DWS.value &&
      get(first(this.backupData), 'ext_parameters.storage_id') &&
      get(first(this.backupData), 'ext_parameters.storage_type') ===
        DataMap.backupStorageType.group.value;
    this.getExtSystemNames();
    this.getProjectNames();
    this.updateData();
    this.getQosNames();
    this.initRetentionDurationTips();
  }

  initRetentionDurationTips() {
    if (this._includesNas()) {
      this.retentionDurationTips = this.i18n.get(
        'protection_replication_policy_nas_retention_duration_tip_label'
      );
    } else if (this._includesPgSQL()) {
      this.retentionDurationTips = this.i18n.get(
        'protection_replication_policy_pgsql_retention_duration_tip_label'
      );
    }
  }

  addStorage() {
    const externalStorageComponent = new TargetClusterComponent(
      this.i18n,
      this.drawModalService,
      this.clusterApiService,
      this.dataMapService,
      this.warningMessageService,
      this.infoMessageService,
      this.cookieService
    );
    externalStorageComponent.addTargetCluster(() => this.getExtSystemNames());
  }

  getSpecifyUser(external_system_id) {
    // 以防触发太多次，做个检测之前有没有获取过同一个集群下的用户
    if (!isEmpty(this.specifyUserOptionsMap[external_system_id])) {
      return;
    }
    this.usersApiService
      .remoteDPUserUsingGET({
        clusterId: external_system_id
      })
      .subscribe((res: any) => {
        // 当前只保留COMMON类型用户并且没有双因子认证
        res.userList = filter(
          res.userList,
          item =>
            [
              DataMap.loginUserType.local.value,
              DataMap.loginUserType.ldap.value
            ].includes(item.userType) && isEmpty(item.dynamicCodeEmail)
        );
        if (isEmpty(this.specifyUserOptionsMap[external_system_id])) {
          assign(this.specifyUserOptionsMap, {
            [external_system_id]: map(res.userList, item => {
              return assign(item, {
                label: item.userName,
                value: item.userId,
                isLeaf: true
              });
            })
          });
        }
        if (!isEmpty(this.tmpSpecifyUserInfo[external_system_id])) {
          this.specifyUserOptionsMap[external_system_id] = uniqBy(
            [
              ...this.specifyUserOptionsMap[external_system_id],
              ...this.tmpSpecifyUserInfo[external_system_id]
            ],
            'userId'
          );
          delete this.tmpSpecifyUserInfo[external_system_id];
        }
      });
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

  getProjectNames() {
    if (!this.isHcsUser) {
      return;
    }
    this.opHcsServiceApiService.getHcsUserProjectsHaveOp({}).subscribe(res => {
      this.projectOptions = this.parseProjectName(res);
      defer(() => this.updateData());
    });
  }

  parseProjectName(res, isCrossCloud = false) {
    const regions = [];
    each(res.projects, item => {
      if (this.isHcsUser && !!this.sourceClusterIp) {
        // 跨云时往下传一个主集群ip用于反向复制时判断
        assign(item, {
          source_cluster_ip: this.sourceClusterIp
        });
      }
      each(item.regions, region => {
        const tmpLabel = this.i18n.isEn
          ? region.region_name.en_us
          : region.region_name.zh_cn;

        if (find(regions, { region_id: region.region_id })) {
          find(regions, { region_id: region.region_id }).children.push(
            assign({}, item, {
              cluster_ip: get(region, 'op_ips', ''),
              isLeaf: true,
              label: item.name,
              region_id: region.region_id,
              key: `${tmpLabel}${item.name}`,
              disabled: isCrossCloud
                ? !find(
                    this.allReplicationClusterOptions,
                    val =>
                      !!intersection(
                        val.clusterIp.split(','),
                        get(region, 'op_ips', '').split(',')
                      ).length
                  )
                : false
            })
          );
        } else {
          assign(region, {
            expanded: true,
            disabled: true,
            isLeaf: false,
            label: this.i18n.isEn
              ? region.region_name.en_us
              : region.region_name.zh_cn,
            children: [
              assign({}, item, {
                cluster_ip: get(region, 'op_ips', ''),
                isLeaf: true,
                label: item.name,
                region_id: region.region_id,
                key: `${tmpLabel}${item.name}`,
                disabled: isCrossCloud
                  ? !find(
                      this.allReplicationClusterOptions,
                      val =>
                        !!intersection(
                          val.clusterIp.split(','),
                          get(region, 'op_ips', '').split(',')
                        ).length
                    )
                  : false
              })
            ]
          });
          regions.push(region);
        }
      });
    });
    return regions;
  }

  getExtSystemNames() {
    if (this.isHcsUser || this.isDmeUser) {
      return;
    }
    this.clusterApiService
      .getClustersInfoUsingGET({
        startPage: 0,
        pageSize: 200,
        typeList: [DataMap.Cluster_Type.target.value],
        statusList: [DataMap.Cluster_Status.online.value]
      })
      .subscribe(res => {
        const clusters = map(res.records, (item: any) => {
          item['clusterId'] = toString(item['clusterId']);
          item['isLeaf'] = true;
          item['label'] = item.clusterName;
          return item;
        });
        this.externalSystems = filter(clusters, item =>
          includes([DataMap.Target_Cluster_Role.replication.value], item.role)
        );
        this.oldExternalSystems = clusters;
        if (!!this.data) {
          // 升级场景下，需要判断集群的username是不是跟传上来的username一样，就需要手动往用户里塞值
          this.parseUpdatedUserName();
        }
      });
  }

  private parseUpdatedUserName() {
    const controls = this.getReplicationTeams().controls;
    each(controls, item => {
      const tmpCluster = find(this.externalSystems, {
        clusterId: item.get('external_system_id').value
      });
      if (!!tmpCluster && item.get('userName').value === tmpCluster.username) {
        const tmpUser = {
          userId: item.get('specifyUser').value,
          label: tmpCluster.username,
          value: item.get('specifyUser').value,
          isLeaf: true,
          userName: tmpCluster.username
        };
        if (!isEmpty(this.specifyUserOptionsMap[tmpCluster.clusterId])) {
          this.specifyUserOptionsMap[tmpCluster.clusterId].push(tmpUser);
        } else {
          assign(this.tmpSpecifyUserInfo, {
            [tmpCluster.clusterId]: [tmpUser]
          });
        }
      }
    });
  }

  initDeviceOption() {
    this.deviceTypeOptions = this.dataMapService
      .toArray('poolStorageDeviceType')
      .map(item => {
        return assign(item, {
          isLeaf: true,
          value:
            item.value === DataMap.poolStorageDeviceType.Server.value
              ? 'BasicDisk'
              : item.value
        });
      });

    this.deviceTypeUseOption = cloneDeep(this.deviceTypeOptions);
  }

  getLocalStorageType() {
    this.storageUserAuthService
      .getStorageUserAuthRelationsByUserId({
        userId: this.cookieService.get('userId'),
        authType: 1
      })
      .subscribe(res => {
        const tmpData: any = find(res.records, {
          storageId: get(first(this.backupData), 'ext_parameters.storage_id')
        });
        const replicationTeams: any = this.formGroup.get('replicationTeams');
        each(replicationTeams.controls, item => {
          item
            .get('local_storage_type')
            .setValue(
              tmpData?.storageType ||
                DataMap.poolStorageDeviceType.OceanProtectX.value
            );
          this.localStorageType =
            tmpData?.storageType ||
            DataMap.poolStorageDeviceType.OceanProtectX.value;
          if (
            ['BasicDisk', DataMap.poolStorageDeviceType.Server.value].includes(
              item.get('local_storage_type').value
            )
          ) {
            // 本地存储单元为服务器类型不支持重删
            item.get('link_deduplication').setValue(false);
            this.isDisableLog = true;
          }
        });
      });
  }

  getSourceCluster() {
    // 跨云获取主集群
    if (!this.isHcsUser) {
      return;
    }
    if (this.isDataBackup) {
      this.clusterApiService
        .getClustersInfoUsingGET({
          startPage: 0,
          pageSize: 200,
          roleList: [DataMap.Target_Cluster_Role.primaryNode.value]
        })
        .subscribe(res => {
          this.sourceClusterIp = first(res.records[0].clusterIp.split(','));
        });
    } else {
      this.clusterApiService
        .pageQueryPacificNodes({
          startPage: 0,
          pageSize: 200,
          role: [toLower(DataMap.nodeRole.primaryNode.value)]
        })
        .subscribe(res => {
          this.sourceClusterIp = res.records[0].manageIp;
        });
    }
  }

  getHcsReplicationCluster() {
    // 跨云获取云环境
    if (!this.isHcsUser) {
      return;
    }
    this.clusterApiService
      .getClustersInfoUsingGET({
        startPage: 0,
        pageSize: 200,
        roleList: [DataMap.Target_Cluster_Role.replication.value]
      })
      .subscribe(res => {
        let clusters = map(res.records, (item: any) => {
          item['clusterId'] = toString(item['clusterId']);
          item['isLeaf'] = true;
          item['label'] = item.clusterName;
          item['value'] = item.clusterId;
          return item;
        });
        this.allReplicationClusterOptions = cloneDeep(clusters);
        clusters = filter(
          clusters,
          item => item['replicationClusterType'] === 1
        );
        this.hcsReplicationClusterOptions = clusters;
      });
  }

  getHcsTenant(res: any) {
    // 跨云获取租户
    if (!this.isHcsUser) {
      return;
    }
    this.opHcsServiceApiService
      .getHcsVdc({
        start: 0,
        limit: 1000,
        clusterId: res
      })
      .subscribe(res => {
        const tenantArray = [];
        each(res.vdcs, item => {
          tenantArray.push({
            ...item,
            value: item.name,
            label: item.name,
            isLeaf: true
          });
        });
        this.vdcTenantOptions = tenantArray;
      });
  }

  getTargetZoneOps() {
    this.dmeService
      .getDmeAzInfos({ AzId: this.cookieService.get('az-id') })
      .subscribe(res => {
        const arr = [];
        each(res, item => {
          arr.push({
            ...item,
            value: item.sn,
            label: item.az_name,
            isLeaf: true
          });
        });
        this.targetZoneOps = arr;
      });
  }
  getResourceOps() {
    this.dmeService
      .getDmeVdcProjectsInfos({
        userId: get(window, 'parent.dmeData.userId', '')
      })
      .subscribe(res => {
        const arr = [];
        each(res.projects, item => {
          arr.push({
            ...item,
            value: item.id,
            label: item.name,
            isLeaf: true
          });
        });
        this.resourceOps = arr;
      });
  }

  getBackupStorageUnitNames() {
    let chosenUnit;
    if (
      get(first(this.backupData), 'ext_parameters.storage_type') ===
      DataMap.backupStorageType.unit.value
    ) {
      chosenUnit = get(first(this.backupData), 'ext_parameters.storage_id');
    }
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      akOperationTips: false,
      akLoading: true
    };
    this.externalStorageUnit = [];
    this.storageUnitService.queryBackUnitGET(params).subscribe(res => {
      if (chosenUnit) {
        const chosenUnitDevice = find(res.records, { id: chosenUnit })
          ?.deviceId;
        res.records = filter(res.records, item => {
          return item.deviceId !== chosenUnitDevice;
        });
      }
      this.externalStorageUnit = map(res.records, item => {
        return {
          isLeaf: true,
          label: item.name,
          disabled: false,
          ...item
        };
      });
      // x系列只能指定本地存储单元
      this.externalStorageUnit = filter(this.externalStorageUnit, item => {
        return (
          this.type === DataMap.Resource_Type.GaussDB_DWS.value ||
          this.appUtilsService.isDecouple ||
          item.generatedType === DataMap.backupStorageGeneratedType.local.value
        );
      });
    });
  }

  getBackupStorageNames(callback?: () => void) {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10
    };

    this.externalStorage = [];
    this.nasDistributionStoragesApiService
      .ListNasDistributionStorages(params as any)
      .subscribe(res => {
        this.externalStorage = map(res.records, item => {
          return {
            isLeaf: true,
            label: item.name,
            ...item
          };
        });
        this.externalStorage = uniqBy(this.externalStorage, 'name');

        if (this.type !== DataMap.Resource_Type.GaussDB_DWS.value) {
          this.externalStorage = this.externalStorage.filter(
            v => v.hasEnableParallelStorage === false
          );
        }
        if (
          this.type === DataMap.Resource_Type.GaussDB_DWS.value &&
          !MultiCluster.isMulti
        ) {
          this.externalStorage = this.externalStorage.filter(
            v => v.hasEnableParallelStorage === true
          );
        }

        if (this.type === DataMap.Resource_Type.GaussDB_DWS.value) {
          const tmp = find(this.externalStorage, item => {
            return item.uuid === this.backupData[0].ext_parameters?.storage_id;
          });
          if (tmp && tmp.hasEnableParallelStorage === false) {
            this.dwsGroup = true;
            this.externalStorage = [];
          } else if (tmp && tmp.hasEnableParallelStorage === true) {
            this.dwsParallel = true;
            this.externalStorage = [];
          }
        }

        !isUndefined(callback) && callback();
      });
  }

  /**
   * 当前应用是否属于 （Nas,）
   */
  _includesNas() {
    return includes(
      [this.applicationType.NASShare, this.applicationType.Fileset],
      this.type
    );
  }

  _includesPgSQL() {
    return includes([this.applicationType.PostgreSQL], this.type);
  }

  updateForm() {
    this.formGroup.addControl(
      'replicationTeams',
      this.fb.array([this.replicationTeam])
    );
  }

  updateData() {
    if (!this.data || !size(this.data)) {
      return;
    }
    const tmpData = cloneDeep(this.data);
    this.getReplicationTeams().clear();
    each(tmpData, item => {
      const replicationTeam = cloneDeep(this.replicationTeam);
      this.listenFormGroup(replicationTeam);
      if (item.replication_target_mode) {
        replicationTeam
          .get('replicationMode')
          .setValue(item.replication_target_mode);
      } else {
        replicationTeam
          .get('replicationMode')
          .setValue(item.replicationMode || ReplicationModeType.CROSS_DOMAIN);
      }
      if (!this.isHcsUser && !this.isDmeUser) {
        replicationTeam
          .get('external_system_id')
          .setValue(item.external_system_id, { emitEvent: false });
        replicationTeam
          .get('action')
          .setValue(item.action, { emitEvent: false });
        if (
          isUndefined(
            this.crossDomainStorageMap[item.action][item.external_system_id]
          )
        ) {
          set(
            this.crossDomainStorageMap[item.action],
            item.external_system_id,
            {}
          );
        }
      }
      replicationTeam
        .get('disableOriginalStorage')
        .setValue(!!item.storage_edit_disable);
      set(
        this.crossDomainStorageMap[replicationTeam.value.action][
          item.external_system_id
        ],
        'disableOriginalStorage',
        !!item.storage_edit_disable
      );
      replicationTeam.get('specifyUser').setValue(item.user_id);
      // 删除是为了不重复触发接口
      delete item.user_id;
      replicationTeam.patchValue({
        ...item,
        disableExternalSystem:
          this.action === this.protectResourceAction.Modify &&
          !!item.external_system_id &&
          !!item.uuid &&
          includes(map(this.data, 'uuid'), item.uuid),
        disableStartTime:
          this.action === this.protectResourceAction.Modify &&
          !!item.start_time &&
          !!item.uuid &&
          includes(map(this.data, 'uuid'), item.uuid),
        disabledRepliMode:
          this.action === this.protectResourceAction.Modify &&
          !!item.uuid &&
          includes(map(this.data, 'uuid'), item.uuid)
      });
      if (this.isHcsUser) {
        if (item.replication_target_mode === ReplicationModeType.CROSS_CLOUD) {
          // 跨云复制无法直接获取projectOption所以先放个对象在那里测试后处理
          replicationTeam.get('external_system_id').setValue([
            {
              region_code: item.region_code,
              project_id: item.project_id
            }
          ]);
        }
        each(this.projectOptions, region => {
          if (
            (region.region_id === item.region_code ||
              region.region_id ===
                item.external_system_id[0]?.parent?.region_id) &&
            !isEmpty(region.children)
          ) {
            each(region.children, child => {
              if (
                child.id === item.project_id ||
                child.id === item.external_system_id[0]?.id
              ) {
                replicationTeam.get('external_system_id').setValue([child]);
              }
            });
          }
        });
      }
      if (
        !this.isHcsUser &&
        !this.isDmeUser &&
        replicationTeam.value.replicationMode ===
          ReplicationModeType.CROSS_DOMAIN
      ) {
        this.getStorageUnitOptions(replicationTeam);
      }
      if (
        replicationTeam.value.replication_storage_type ===
        DataMap.backupStorageTypeSla.group.value
      ) {
        replicationTeam
          .get('external_storage_id')
          .setValue(item.external_storage_id || item.storage_id);
      } else {
        replicationTeam
          .get('replication_storage_id')
          .setValue(item.replication_storage_id || item.storage_id);
      }
      this.dealUnit(replicationTeam);
      // 跨云需要回显远端存储类型，但是前面逻辑会清空，现在暂时在这里重新赋值
      if (
        replicationTeam.get('replicationMode').value ===
        ReplicationModeType.CROSS_CLOUD
      ) {
        replicationTeam
          .get('remote_storage_type')
          .setValue(item.remote_storage_type);
        replicationTeam
          .get('replication_storage_type')
          .setValue(item.replication_storage_type);
      }
      replicationTeam.get('duration_unit').valueChanges.subscribe(res => {
        this.changeTimeUnits(replicationTeam, res, 'retention_duration');
      });
      if (!!item.storage_edit_disable) {
        replicationTeam.get('disableNameIncluded').setValue(true);
      }
      replicationTeam.get('replication_target_type').updateValueAndValidity();
      // 本地不是dws并且上次是单元组的时候可直接判断对端是多集群，是dws时本身也可以展示单元组则留到后续判断
      replicationTeam
        .get('isTargetMulti')
        .setValue(
          item?.replication_storage_type ===
            DataMap.backupStorageTypeSla.group.value &&
            this.type !== DataMap.Resource_Type.GaussDB_DWS.value
        );
      this.getReplicationTeams().push(replicationTeam);
    });
    this.calcPolicyNum();

    this.batchDealCtrl();
  }

  dealUnit(replicationTeam: FormGroup) {
    if (replicationTeam.get('periodExecuteTrigger').value) {
      const interval_unit = replicationTeam.get('interval_unit').value;
      if (interval_unit) {
        this.changeTimeUnits(replicationTeam, interval_unit, 'interval');
      }
    }

    const duration_unit = replicationTeam.get('duration_unit').value;
    if (duration_unit) {
      this.changeTimeUnits(
        replicationTeam,
        duration_unit,
        'retention_duration'
      );
    }
  }

  batchDealCtrl() {
    this.getReplicationTeams().controls.map(control => {
      if (
        control.get('duration_unit').value ===
        DataMap.Interval_Unit.persistent.value
      ) {
        control.get('retention_duration').disable();
        control.get('retention_duration').setValue('');
      }
    });
  }

  get replicationTeam(): FormGroup {
    const replicationTeam: FormGroup = this.fb.group({
      uuid: new FormControl(''),
      action: new FormControl(
        this.dataNum !== 4
          ? DataMap.replicationAction.data.value
          : DataMap.replicationAction.log.value
      ),
      name: new FormControl(
        this.i18n.get('common_replication_params_label', ['01']),
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name()
          ],
          updateOn: 'change'
        }
      ),
      disabledRepliMode: new FormControl(false),
      replicationMode: new FormControl(ReplicationModeType.CROSS_DOMAIN),
      disableExternalSystem: new FormControl(false),
      external_system_id: new FormControl('', {
        validators: this.isDmeUser
          ? null
          : [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      local_storage_type: new FormControl(
        this.isReplica ? DataMap.poolStorageDeviceType.OceanProtectX.value : ''
      ),
      // 远端存储类型暂时按部署形态直接赋值，不可选
      remote_storage_type: new FormControl(
        this.appUtilsService.isDistributed
          ? DataMap.poolStorageDeviceType.OceanPacific.value
          : DataMap.poolStorageDeviceType.OceanProtectX.value,
        {
          validators:
            this.isReplica || this.isHcsUser || this.isDmeUser
              ? null
              : [this.baseUtilService.VALID.required()]
        }
      ),
      replication_storage_type: new FormControl('', {
        validators:
          this.isHcsUser || this.isDmeUser
            ? null
            : [this.baseUtilService.VALID.required()]
      }),
      replication_storage_id: new FormControl(''),
      external_storage_id: new FormControl(''),
      backupExecuteTrigger: new FormControl(false),
      periodExecuteTrigger: new FormControl(true),
      interval: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 23)
        ],
        updateOn: 'change'
      }),
      interval_unit: new FormControl(DataMap.Interval_Unit.hour.value),
      disableStartTime: new FormControl(false),
      start_time: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      start_replicate_time: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      replication_target_type: new FormControl(
        DataMap.slaReplicationRule.all.value
      ),
      is_first_hcs: new FormControl(!!this.data),
      hcs_project_option: new FormControl([]),
      hcs_cluster_id: new FormControl(''),
      tenant_name: new FormControl(''),
      vdc_name: new FormControl(''),
      vdc_password: new FormControl(''),
      copy_type_year: new FormControl(true),
      generate_time_range_year: new FormControl(this.generate_time_range_year),
      retention_duration_year: new FormControl(''),
      copy_type_month: new FormControl(true),
      generate_time_range_month: new FormControl(
        DataMap.Month_Time_Range.first.value
      ),
      retention_duration_month: new FormControl(''),
      copy_type_week: new FormControl(true),
      generate_time_range_week: new FormControl(this.generate_time_range_week),
      retention_duration_week: new FormControl(''),
      retention_duration: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.maxRetentionDay)
        ],
        updateOn: 'change'
      }),
      duration_unit: new FormControl(DataMap.Interval_Unit.day.value),
      qos_id: new FormControl(''),
      link_deduplication: new FormControl(true),
      link_compression: new FormControl(true),
      alarm_after_failure: new FormControl(true),
      specifyUser: new FormControl('', {
        validators:
          !this.isHcsUser && !this.isDmeUser
            ? [this.baseUtilService.VALID.required()]
            : null,
        updateOn: 'change'
      }),
      userName: new FormControl(''),
      authPassword: new FormControl('', {
        validators:
          !this.isHcsUser && !this.isDmeUser
            ? [this.baseUtilService.VALID.required()]
            : null,
        updateOn: 'change'
      }),
      userType: new FormControl(''),
      isAuth: [false],
      disableOriginalStorage: new FormControl(false),
      disableNameIncluded: new FormControl(false),
      cluster_esn: new FormControl('', {
        validators: this.isDmeUser
          ? [this.baseUtilService.VALID.required()]
          : null
      }),
      project_id: new FormControl('', {
        validators: this.isDmeUser
          ? [this.baseUtilService.VALID.required()]
          : null
      }),
      isTargetMulti: new FormControl(false)
    });
    this.listenFormGroup(replicationTeam);
    return replicationTeam;
  }

  getReplicationTeams() {
    return this.formGroup.get('replicationTeams') as FormArray;
  }

  // HCS测试密码
  testPassword(item) {
    const params = {
      clusterId: item.get('hcs_cluster_id').value,
      password: item.get('vdc_password').value,
      username: item.get('vdc_name').value,
      tenantName: item.get('tenant_name').value,
      tenantId: find(
        this.vdcTenantOptions,
        val => val.name === item.get('tenant_name').value
      )?.id
    };
    this.opHcsServiceApiService
      .getHcsUserProjectsHaveOpForHcsCopy({
        param: params
      })
      .subscribe(res => {
        item
          .get('hcs_project_option')
          .setValue(this.parseProjectName(res, true));
        // 修改时在测试后回显，如果获取数据中没有该数据则去除
        this.parseHcsModify(item);
      });
  }

  private parseHcsModify(item: any) {
    if (!!this.data && item.get('is_first_hcs').value) {
      let flag = true;
      const regionCode = item.get('external_system_id').value[0]?.region_code;
      const projectId = item.get('external_system_id').value[0]?.project_id;
      const region = item
        .get('hcs_project_option')
        .value.find(r => r.region_id === regionCode);
      if (region) {
        const project = region.children.find(p => p.id === projectId);
        if (project) {
          item.get('external_system_id').setValue([project]);
          flag = false;
        }
      }
      if (flag) {
        item.get('external_system_id').setValue([]);
      }
      item.get('is_first_hcs').setValue(false);
    } else if (
      !find(flatMapDeep(item.get('hcs_project_option').value[0], 'children'), {
        id: item.get('external_system_id').value[0]?.id
      })
    ) {
      item.get('external_system_id').setValue([]);
    }
  }

  // 认证密码
  authPassword(replicationForm) {
    replicationForm
      .get('userName')
      .setValue(
        find(
          this.specifyUserOptionsMap[replicationForm.value.external_system_id],
          { value: replicationForm.value.specifyUser }
        )?.userName
      );
    const tmpUserType = find(
      this.specifyUserOptionsMap[replicationForm.value.external_system_id],
      { userName: replicationForm.value.userName }
    )?.userType;
    const params = {
      username: replicationForm.value.userName,
      password: replicationForm.value.authPassword,
      clusterId: replicationForm.value.external_system_id,
      userType: tmpUserType
    };
    this.clusterApiService
      .verifyUsernamePasswordAndGetClusterInfoUsingPost({
        verifyUserDto: params,
        akOperationTips: false
      })
      .subscribe((res: any) => {
        replicationForm.get('isAuth').setValue(res?.verify);
        if (isUndefined(res?.backupBaseClusterInfo)) {
          replicationForm.get('isTargetMulti').setValue(false);
        } else if (
          !isUndefined(res?.backupBaseClusterInfo?.clusterEstablished)
        ) {
          replicationForm
            .get('isTargetMulti')
            .setValue(res?.backupBaseClusterInfo?.clusterEstablished);
        } else {
          replicationForm
            .get('isTargetMulti')
            .setValue(!isEmpty(res?.backupBaseClusterInfo?.roleType));
        }
        if (res.verify) {
          this.messageService.success(
            this.i18n.get('protection_user_verify_success_label')
          );
          this.externalStorageMap[
            `${replicationForm.value.external_system_id}+${replicationForm.value.specifyUser}`
          ] = [];
          this.externalStorageUnitMaintainMap[
            `${replicationForm.value.external_system_id}+${replicationForm.value.specifyUser}`
          ] = [];
          this.externalStorageUnitMap[
            `${replicationForm.value.external_system_id}+${replicationForm.value.specifyUser}`
          ] = [];
          this.getStorageUnitOptions(replicationForm);
        } else {
          this.messageService.error(
            this.i18n.get('protection_user_verify_fail_label')
          );
        }
      });
  }

  getStorageUnitOptions(replicationForm: FormGroup) {
    let tmpClusterId;
    if (
      replicationForm.value.replicationMode === ReplicationModeType.CROSS_CLOUD
    ) {
      tmpClusterId = find(
        this.allReplicationClusterOptions,
        item =>
          !!intersection(
            item.clusterIp.split(','),
            replicationForm.value.external_system_id[0].cluster_ip.split(',')
          ).length
      ).clusterId;
    } else {
      tmpClusterId = replicationForm.value.external_system_id;
    }
    this.storageUserAuthService
      .getRemoteStorageUserAuthRelationsByUserId({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        userId: replicationForm.value.specifyUser,
        authType: 2,
        clusterId: tmpClusterId
      })
      .subscribe(res => {
        if (
          isEmpty(
            this.externalStorageMap[
              `${replicationForm.value.external_system_id}+${replicationForm.value.specifyUser}`
            ]
          )
        ) {
          this.externalStorageMap[
            `${replicationForm.value.external_system_id}+${replicationForm.value.specifyUser}`
          ] = map(res.records, (item: any) => {
            // 升级场景直接塞为oceanprotectx
            item.storageType =
              item?.storageType ??
              DataMap.poolStorageDeviceType.OceanProtectX.value;
            return assign(item, {
              ...item,
              label: item.storageName,
              value: item.storageId,
              isLeaf: true
            });
          });
          if (this.type !== DataMap.Resource_Type.GaussDB_DWS.value) {
            this.externalStorageMap[
              `${replicationForm.value.external_system_id}+${replicationForm.value.specifyUser}`
            ] = this.externalStorageMap[
              `${replicationForm.value.external_system_id}+${replicationForm.value.specifyUser}`
            ].filter(v => v.hasEnableParallelStorage === false);
          }
          if (
            this.type === DataMap.Resource_Type.GaussDB_DWS.value &&
            !replicationForm.value.isTargetMulti
          ) {
            this.externalStorageMap[
              `${replicationForm.value.external_system_id}+${replicationForm.value.specifyUser}`
            ] = this.externalStorageMap[
              `${replicationForm.value.external_system_id}+${replicationForm.value.specifyUser}`
            ].filter(v => v.hasEnableParallelStorage === true);
          }
        }
      });
    this.storageUserAuthService
      .getRemoteStorageUserAuthRelationsByUserId({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        userId: replicationForm.value.specifyUser,
        authType: 1,
        clusterId: tmpClusterId
      })
      .subscribe((res: any) => {
        res.records.sort(
          (a, b) =>
            Math.abs(a?.runningStatus - 27) - Math.abs(b?.runningStatus - 27)
        );
        if (
          isEmpty(
            this.externalStorageUnitMaintainMap[
              `${replicationForm.value.external_system_id}+${replicationForm.value.specifyUser}`
            ]
          )
        ) {
          this.externalStorageUnitMaintainMap[
            `${replicationForm.value.external_system_id}+${replicationForm.value.specifyUser}`
          ] = map(res.records, (item: any) => {
            // 升级场景直接塞为oceanprotectx
            item.storageType =
              item?.storageType ??
              DataMap.poolStorageDeviceType.OceanProtectX.value;
            return assign(item, {
              ...item,
              label: item.storageName,
              value: item.storageId,
              isLeaf: true
            });
          }).filter(item => {
            return (
              this.type === DataMap.Resource_Type.GaussDB_DWS.value ||
              this.appUtilsService.isDecouple ||
              this.appUtilsService.isDistributed ||
              item.generatedType ===
                DataMap.backupStorageGeneratedType.local.value
            );
          });
        }
        // 跨云复制通过选择复制目标端直接去获取存储单元
        if (
          replicationForm.get('replicationMode').value ===
          ReplicationModeType.CROSS_CLOUD
        ) {
          replicationForm.get('isAuth').setValue(true);
        }
        this.externalStorageUnitMap = cloneDeep(
          this.externalStorageUnitMaintainMap
        );
        replicationForm.get('remote_storage_type').updateValueAndValidity();
      });
  }

  addReplicationTeam(label) {
    const replicationTeam = cloneDeep(this.replicationTeam);
    const controls = this.getReplicationTeams().controls;
    const nameArr = map(controls, item => item.value.name);

    while (
      includes(
        nameArr,
        this.i18n.get('common_replication_params_label', [`0${this.index}`])
      )
    ) {
      this.index++;
    }
    replicationTeam.patchValue({
      name: this.i18n.get('common_replication_params_label', [
        `0${this.index++}`
      ])
    });

    replicationTeam.get('local_storage_type').setValue(this.localStorageType);
    if (this.localStorageType === 'BasicDisk') {
      // 本地存储单元为服务器类型不支持重删
      replicationTeam.get('link_deduplication').setValue(false);
    }
    this.listenFormGroup(replicationTeam);
    this.getReplicationTeams().push(replicationTeam);
    this.activeIndex = this.getReplicationTeams().controls.length - 1;
    this.calcPolicyNum();
  }

  removeReplicationTeam(tab) {
    const controls = this.getReplicationTeams().controls;
    for (let index = 0; index < controls.length; index++) {
      if (tab.lvId === index) {
        this.getReplicationTeams().removeAt(index);
        this.activeIndex = 0;
      }
    }
    this.calcPolicyNum();
  }

  calcPolicyNum() {
    const controls = this.getReplicationTeams().controls;
    let tmpDataNum = 0;
    let tmpLogNum = 0;
    each(controls, item => {
      if (item.get('action').value === DataMap.replicationAction.data.value) {
        tmpDataNum++;
      } else {
        tmpLogNum++;
      }
    });
    this.dataNum = tmpDataNum;
    this.logNum = tmpLogNum;
  }

  setReplicationStorageTypeValid(res, replicationTeam) {
    if (
      replicationTeam.value.replicationMode !==
        this.replicationModeType.CROSS_CLOUD ||
      !this.isHcsUser
    ) {
      return;
    }
    if (res === DataMap.backupStorageTypeSla.group.value) {
      replicationTeam
        .get('external_storage_id')
        .setValidators([this.baseUtilService.VALID.required()]);
      replicationTeam.get('replication_storage_id').clearValidators();
    } else {
      replicationTeam
        .get('replication_storage_id')
        .setValidators([this.baseUtilService.VALID.required()]);
      replicationTeam.get('external_storage_id').clearValidators();
    }
  }

  listenFormGroup(replicationTeam: FormGroup) {
    replicationTeam
      .get('retention_duration')
      .setValidators([
        this.validProTime(replicationTeam),
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.integer(),
        this.baseUtilService.VALID.rangeValue(1, this.maxRetentionDay)
      ]);
    // 添加校验，如果是修改，立即触发
    if (this.data && size(this.data)) {
      replicationTeam.get('retention_duration').updateValueAndValidity();
      replicationTeam.get('retention_duration').markAsTouched();
    }
    replicationTeam.get('action').valueChanges.subscribe(res => {
      if (res === DataMap.replicationAction.log.value) {
        replicationTeam
          .get('replication_target_type')
          .setValue(DataMap.slaReplicationRule.all.value);
      }
      defer(() => this.calcPolicyNum());
    });
    replicationTeam.get('backupExecuteTrigger').valueChanges.subscribe(res => {
      if (res) {
        this.changeReplicationBackupExecute(replicationTeam);
        replicationTeam.get('interval').disable();
      } else {
        replicationTeam.get('interval').enable();
      }
    });

    replicationTeam.get('periodExecuteTrigger').valueChanges.subscribe(res => {
      if (res) {
        this.changeReplicationPeriodExecute(replicationTeam);
      }
    });

    each(
      [
        'action',
        'specifyUser',
        'authPassword',
        'userName',
        'replication_storage_type',
        'external_storage_id',
        'replication_storage_id',
        'isAuth'
      ],
      item => {
        replicationTeam.get(item).valueChanges.subscribe(res => {
          const externalSystemId = replicationTeam.get('external_system_id')
            .value;
          if (!externalSystemId) {
            return;
          }
          if (
            isUndefined(
              this.crossDomainStorageMap[replicationTeam.value.action][
                externalSystemId
              ]
            )
          ) {
            set(
              this.crossDomainStorageMap[replicationTeam.value.action],
              externalSystemId,
              {}
            );
          }
          const values = this.crossDomainStorageMap[
            replicationTeam.value.action
          ][externalSystemId];
          if (values[item] === res) {
            return;
          }
          values[item] = cloneDeep(res);
          this.getReplicationTeams().controls.forEach(formGroup => {
            if (
              formGroup.value.external_system_id === externalSystemId &&
              formGroup.value.action === replicationTeam.value.action
            ) {
              formGroup.get(item).setValue(res);
            }
          });
        });
      }
    );

    if (!this.isHcsUser && !this.isDmeUser) {
      replicationTeam
        .get('disableOriginalStorage')
        .valueChanges.subscribe(res => {
          if (
            res ||
            replicationTeam.get('replicationMode').value ===
              ReplicationModeType.INTRA_DOMAIN
          ) {
            replicationTeam.get('authPassword').clearValidators();
          } else {
            replicationTeam
              .get('authPassword')
              .setValidators([this.baseUtilService.VALID.required()]);
          }
          replicationTeam.get('authPassword').updateValueAndValidity();
        });
    }

    replicationTeam
      .get('replication_storage_type')
      .valueChanges.subscribe(res => {
        // 是否需要根据单元组类型切换校验
        const isValidStorage =
          this.replicationModeType.INTRA_DOMAIN ===
            replicationTeam.value.replicationMode ||
          (this.replicationModeType.CROSS_DOMAIN ===
            replicationTeam.value.replicationMode &&
            !this.isHcsUser &&
            !this.isDmeUser);
        if (res === DataMap.backupStorageTypeSla.group.value) {
          this.deviceTypeUseOption = cloneDeep(
            this.deviceTypeOptions.filter(
              item =>
                item.value === DataMap.poolStorageDeviceType.OceanProtectX.value
            )
          );
          if (isValidStorage) {
            replicationTeam
              .get('external_storage_id')
              .setValidators([this.baseUtilService.VALID.required()]);
            replicationTeam.get('replication_storage_id').clearValidators();
          } else {
            this.getBackupStorageNames();
          }
        } else if (res === DataMap.backupStorageTypeSla.unit.value) {
          this.deviceTypeUseOption = cloneDeep(this.deviceTypeOptions);
          // 由于现在是默认选择X系列类型单元或pacific，所以需要手动触发一次过滤
          replicationTeam.get('remote_storage_type').updateValueAndValidity();
          if (isValidStorage) {
            replicationTeam
              .get('replication_storage_id')
              .setValidators([this.baseUtilService.VALID.required()]);
            replicationTeam.get('external_storage_id').clearValidators();
          } else {
            this.getBackupStorageUnitNames();
          }
        }
        this.setReplicationStorageTypeValid(res, replicationTeam);
        replicationTeam
          .get('replication_storage_id')
          .setValue(
            this.crossDomainStorageMap[replicationTeam.value.action][
              replicationTeam.value.external_system_id
            ]?.replication_storage_id || ''
          );
        replicationTeam
          .get('external_storage_id')
          .setValue(
            this.crossDomainStorageMap[replicationTeam.value.action][
              replicationTeam.value.external_system_id
            ]?.external_storage_id || ''
          );
      });

    replicationTeam.get('replicationMode').valueChanges.subscribe(res => {
      const changeValidList =
        this.isHcsUser || this.isDmeUser
          ? ['external_system_id']
          : [
              'external_system_id',
              'specifyUser',
              'authPassword',
              'replication_storage_type',
              'external_storage_id',
              'replication_storage_id'
            ];
      const hcsValidList = [
        'hcs_project_option',
        'hcs_cluster_id',
        'tenant_name',
        'vdc_name',
        'vdc_password'
      ];
      const dmeValidList = ['cluster_esn', 'project_id'];
      if (res === ReplicationModeType.CROSS_CLOUD) {
        // 跨云的控件处理
        hcsValidList.forEach(item => {
          replicationTeam
            .get(item)
            .setValidators([this.baseUtilService.VALID.required()]);
        });
        this.getHcsReplicationCluster();
        this.getSourceCluster();
      } else {
        hcsValidList.forEach(item => {
          replicationTeam.get(item).clearValidators();
          replicationTeam.get(item).updateValueAndValidity();
        });
      }
      if (
        res === ReplicationModeType.CROSS_DOMAIN ||
        res === ReplicationModeType.CROSS_CLOUD
      ) {
        replicationTeam.get('replication_storage_type').clearValidators();
        replicationTeam
          .get('replication_storage_type')
          .updateValueAndValidity();
        if (!this.isDmeUser) {
          changeValidList.forEach(item => {
            replicationTeam
              .get(item)
              .setValidators([this.baseUtilService.VALID.required()]);
          });
        } else {
          dmeValidList.forEach(item => {
            replicationTeam
              .get(item)
              .setValidators([this.baseUtilService.VALID.required()]);
            replicationTeam.get(item).updateValueAndValidity();
          });
        }
      } else {
        this.getBackupStorageNames();
        this.getBackupStorageUnitNames();
        changeValidList.forEach(item => {
          replicationTeam.get(item).clearValidators();
        });
        dmeValidList.forEach(item => {
          replicationTeam.get(item).clearValidators();
          replicationTeam.get(item).updateValueAndValidity();
        });
        // 域内只需要有type校验加上就行
        replicationTeam
          .get('replication_storage_type')
          .setValidators([this.baseUtilService.VALID.required()]);
        replicationTeam
          .get('replication_storage_type')
          .updateValueAndValidity();
      }
      changeValidList.forEach(item => {
        replicationTeam.get(item).setValue('');
      });
    });

    replicationTeam.get('remote_storage_type').valueChanges.subscribe(res => {
      const {
        replication_storage_type,
        external_system_id,
        specifyUser
      } = replicationTeam.value;
      const storageKey = `${external_system_id}+${specifyUser}`;

      if (
        replication_storage_type === DataMap.backupStorageTypeSla.unit.value
      ) {
        const storageUnitMaintainMap = this.externalStorageUnitMaintainMap[
          storageKey
        ];
        this.externalStorageUnitMap[storageKey] = filter(
          storageUnitMaintainMap,
          item => item.storageType === res
        );
        // 如果新的下拉框选项里不存在该单元就去掉,否则保留
        if (
          !some(
            this.externalStorageUnitMap[storageKey],
            item =>
              replicationTeam.get('replication_storage_id').value ===
              item.storageId
          )
        ) {
          replicationTeam.get('replication_storage_id').setValue('');
        }
      }
    });

    replicationTeam.get('external_system_id').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }

      each(
        [
          'specifyUser',
          'authPassword',
          'userName',
          'replication_storage_type',
          'external_storage_id',
          'replication_storage_id',
          'isAuth'
        ],
        item => {
          if (
            !isUndefined(
              this.crossDomainStorageMap[replicationTeam.get('action').value][
                res
              ]
            )
          ) {
            replicationTeam
              .get(item)
              .setValue(
                this.crossDomainStorageMap[replicationTeam.get('action').value][
                  res
                ][item] || ''
              );
          } else {
            replicationTeam.get(item).setValue('');
          }
        }
      );

      replicationTeam
        .get('disableOriginalStorage')
        .setValue(
          !isUndefined(
            this.crossDomainStorageMap[replicationTeam.get('action').value][res]
          ) &&
            !!this.crossDomainStorageMap[replicationTeam.get('action').value][
              res
            ]?.disableOriginalStorage
        );

      if (
        replicationTeam.value.replicationMode ===
          ReplicationModeType.CROSS_DOMAIN &&
        !this.isHcsUser &&
        !this.isDmeUser
      ) {
        this.getSpecifyUser(res);
      }
      if (
        replicationTeam.value.replicationMode ===
          ReplicationModeType.CROSS_CLOUD &&
        !!res
      ) {
        if (isEmpty(res)) {
          return;
        }
        replicationTeam.get('specifyUser').setValue(res[0].id);
        this.getStorageUnitOptions(replicationTeam);
      }
    });

    replicationTeam.get('specifyUser').valueChanges.subscribe(res => {
      // 切换用户选择时也要清理下属已选项, 但是得是手动认证后，才能清，不然会导致同集群同步的时候清掉所有值
      if (replicationTeam.get('isAuth').value) {
        replicationTeam.get('isAuth').setValue(false);
        replicationTeam.get('replication_storage_id').setValue('');
        replicationTeam.get('external_storage_id').setValue('');
        replicationTeam.get('authPassword').setValue('');
      }
      const tmpUserType = find(
        this.specifyUserOptionsMap[replicationTeam.value.external_system_id],
        { value: res }
      )?.userType;
      replicationTeam.get('userType').setValue(tmpUserType);
    });

    if (this.isHcsUser) {
      replicationTeam.get('hcs_cluster_id').valueChanges.subscribe(res => {
        if (!res) {
          return;
        }

        this.getHcsTenant(res);
      });
    }

    this.checkReplicationTargetType(replicationTeam);
    this.checkCopyType(replicationTeam);
  }

  dealReplicationTargetType(replicationTeam: FormGroup, archiveTargetType) {
    if (archiveTargetType === DataMap.slaReplicationRule.all.value) {
      replicationTeam.get('retention_duration_year').clearValidators();
      replicationTeam.get('retention_duration_month').clearValidators();
      replicationTeam.get('retention_duration_week').clearValidators();
      replicationTeam
        .get('start_replicate_time')
        .setValidators([this.baseUtilService.VALID.required()]);
      this.changeTimeUnits(
        replicationTeam,
        replicationTeam.value.duration_unit,
        'retention_duration'
      );
    } else {
      replicationTeam.get('start_replicate_time').clearValidators();
      replicationTeam.get('retention_duration').clearValidators();
      if (replicationTeam.value.copy_type_year) {
        replicationTeam
          .get('retention_duration_year')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionYear)
          ]);
      }
      if (replicationTeam.value.copy_type_month) {
        replicationTeam
          .get('retention_duration_month')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionMonth)
          ]);
      }
      if (replicationTeam.value.copy_type_week) {
        replicationTeam
          .get('retention_duration_week')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionWeek)
          ]);
      }
    }
    replicationTeam.get('start_replicate_time').updateValueAndValidity();
    replicationTeam.get('retention_duration').updateValueAndValidity();
    replicationTeam.get('retention_duration_year').updateValueAndValidity();
    replicationTeam.get('retention_duration_month').updateValueAndValidity();
    replicationTeam.get('retention_duration_week').updateValueAndValidity();
  }

  checkReplicationTargetType(replicationTeam: FormGroup) {
    replicationTeam
      .get('replication_target_type')
      .valueChanges.subscribe(res =>
        this.dealReplicationTargetType(replicationTeam, res)
      );
  }

  checkCopyType(replicationTeam: FormGroup) {
    replicationTeam.get('copy_type_year').valueChanges.subscribe(res => {
      if (
        res &&
        replicationTeam.get('replication_target_type').value ===
          DataMap.slaReplicationRule.specify.value
      ) {
        replicationTeam
          .get('retention_duration_year')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionYear)
          ]);
      } else {
        replicationTeam.get('retention_duration_year').clearValidators();
      }
      replicationTeam.get('retention_duration_year').updateValueAndValidity();
    });
    replicationTeam.get('copy_type_month').valueChanges.subscribe(res => {
      if (
        res &&
        replicationTeam.get('replication_target_type').value ===
          DataMap.slaReplicationRule.specify.value
      ) {
        replicationTeam
          .get('retention_duration_month')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionMonth)
          ]);
      } else {
        replicationTeam.get('retention_duration_month').clearValidators();
      }
      replicationTeam.get('retention_duration_month').updateValueAndValidity();
    });
    replicationTeam.get('copy_type_week').valueChanges.subscribe(res => {
      if (
        res &&
        replicationTeam.get('replication_target_type').value ===
          DataMap.slaReplicationRule.specify.value
      ) {
        replicationTeam
          .get('retention_duration_week')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionWeek)
          ]);
      } else {
        replicationTeam.get('retention_duration_week').clearValidators();
      }
      replicationTeam.get('retention_duration_week').updateValueAndValidity();
    });
  }

  changeReplicationPeriodExecute(formGroup: FormGroup) {
    formGroup.get('backupExecuteTrigger').setValue(false);
    formGroup
      .get('interval')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.integer(),
        this.baseUtilService.VALID.rangeValue(1, 23)
      ]);
    formGroup.get('interval').updateValueAndValidity();
    formGroup.get('interval').enable();
  }

  changeReplicationBackupExecute(formGroup: FormGroup) {
    formGroup.get('periodExecuteTrigger').setValue(false);
    formGroup.get('interval').clearValidators();
    formGroup.get('interval').disable();
    formGroup.get('interval').updateValueAndValidity();
  }

  getWormDuration(item) {
    let params;
    if (!isUndefined(item.worm_switch)) {
      // worm_switch有值，提交备份策略后进入复制策略
      params = {
        worm_validity_type: item.worm_switch ? item.worm_validity_type : 0,
        retention: {
          worm_retention_duration: item.worm_switch
            ? item.worm_validity_type === 1
              ? item.retention_duration
              : item.worm_specified_retention_duration
            : null,
          worm_duration_unit: item.worm_switch
            ? item.worm_validity_type === 1
              ? item.duration_unit
              : item.worm_specified_duration_unit
            : null
        }
      };
    } else {
      params = {
        worm_validity_type: item.worm_validity_type || 0,
        retention: {
          worm_retention_duration:
            item.worm_specified_retention_duration || null,
          worm_duration_unit: item.worm_specified_duration_unit || null
        }
      };
    }
    this.getSpecialBackupData(item, params);
    if (
      item.worm_validity_type === 1 &&
      item.specified_duration_unit === 'p' &&
      item.duration_unit === 'p'
    ) {
      // 修改场景永久保留时，worm_specified_retention_duration和worm_duration_unit是null，特殊赋值
      params = {
        worm_validity_type: item.worm_validity_type,
        retention: {
          worm_retention_duration:
            item.worm_specified_retention_duration || null,
          worm_duration_unit: 'p' || null
        }
      };
    }
    return params;
  }

  getSpecialBackupData(item, params) {
    if (
      includes(
        [DaysOfType.DaysOfYear, DaysOfType.DaysOfMonth, DaysOfType.DaysOfWeek],
        item.trigger_action
      )
    ) {
      params.retention.worm_retention_duration = item.worm_switch
        ? item.worm_validity_type === 1
          ? +item.specified_retention_duration
          : item.worm_specified_retention_duration
        : null;
      params.retention.worm_duration_unit = item.worm_switch
        ? item.worm_validity_type === 1
          ? item.specified_duration_unit
          : item.worm_specified_duration_unit
        : null;
    }
  }

  getMaxWormDuration() {
    if (isEmpty(this.backupData)) {
      return;
    }
    this.wormDataList = this.backupData.map(item => {
      return this.getWormDuration(item);
    });
    this.wormDataList.map(item => {
      if (item.worm_validity_type) {
        let itemWormData = 0;
        if (item.retention.worm_duration_unit === 'p') {
          itemWormData = Infinity;
        } else {
          itemWormData =
            item.retention.worm_retention_duration *
            this.dataMap.unitTranslateMap[item.retention.worm_duration_unit];
        }
        if (itemWormData > this.wormData) {
          this.wormData = itemWormData;
        }
      }
    });
  }

  validProTime(formGroup) {
    return (control: AbstractControl): { [key: string]: { value: any } } => {
      const replicationDurationUnit = formGroup.get('duration_unit').value;
      const replicationDurationData =
        this.dataMap.unitTranslateMap[replicationDurationUnit] *
        toNumber(formGroup.get('retention_duration').value);
      if (this.wormData > replicationDurationData) {
        return {
          [this.i18n.get('common_retention_worm_label')]: {
            value: control.value
          }
        };
      }
      return null;
    };
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
      if (formControlName === 'retention_duration') {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionDay),
            this.validProTime(formGroup)
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
      if (formControlName === 'retention_duration') {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionWeek),
            this.validProTime(formGroup)
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
      if (formControlName === 'retention_duration') {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionMonth),
            this.validProTime(formGroup)
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
      if (formControlName === 'retention_duration') {
        formGroup
          .get(formControlName)
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, this.maxRetentionYear),
            this.validProTime(formGroup)
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
