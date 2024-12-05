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
import { FormBuilder, FormGroup } from '@angular/forms';
import {
  ApplicationType,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  DaysOfType,
  DmeServiceService,
  I18NService,
  MediaSetApiService,
  NasDistributionStoragesApiService,
  PolicyAction,
  QosService,
  ReplicationModeType,
  ScheduleTrigger,
  SLA_BACKUP_NAME,
  StoragesApiService,
  StorageUnitService,
  StorageUserAuthService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { SlaParseService } from 'app/shared/services/sla-parse.service';
import {
  assign,
  cloneDeep,
  each,
  find,
  get,
  includes,
  intersection,
  isEmpty,
  isEqual,
  isFunction,
  isNumber,
  map,
  set,
  union,
  unionWith
} from 'lodash';

@Component({
  selector: 'aui-sla-info',
  templateUrl: './sla-info.component.html',
  styleUrls: ['./sla-info.component.less'],
  providers: [DatePipe]
})
export class SlaInfoComponent implements OnInit {
  find = find;
  _includes = includes;
  _parse = JSON.parse;
  formGroup: FormGroup;
  qosNames = [];
  storageNames = [];
  externalSystems = [];
  replicationStorageNames = new Map();
  backupTeams = [];
  archiveTeams = [];
  replicationTeams = [];
  clusterNodeNames = [];
  mediaSetOptions = {};
  targetZoneOps = [];
  resourceOps = [];
  _get = get;

  dataMap = DataMap;
  appType = ApplicationType;
  daysOfType = DaysOfType;
  policyAction = PolicyAction;
  archivalProtocol = DataMap.Archival_Protocol;
  archiveTargetType = DataMap.Archive_Target_Type;
  hbaseBackupType = DataMap.Hbase_Backup_Type;
  daysOfMonthType = DataMap.Days_Of_Month_Type;
  scheduleTrigger = ScheduleTrigger;
  slaBackupName = cloneDeep(SLA_BACKUP_NAME);
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  specialApplication = [
    DataMap.Resource_Type.oracleCluster.value,
    ApplicationType.Oracle,
    ApplicationType.TDSQL,
    DataMap.Resource_Type.MongodbClusterInstance.value,
    DataMap.Resource_Type.MongodbSingleInstance.value,
    ApplicationType.Vmware,
    ApplicationType.CNware,
    ApplicationType.FusionCompute,
    ApplicationType.FusionOne,
    ApplicationType.KubernetesDatasetCommon,
    ApplicationType.KubernetesStatefulSet,
    DataMap.Resource_Type.kubernetesNamespaceCommon.value,
    ApplicationType.HCSCloudHost,
    ApplicationType.OpenStack,
    ApplicationType.ApsaraStack,
    ApplicationType.Exchange,
    DataMap.Resource_Type.ExchangeSingle.value,
    DataMap.Resource_Type.ExchangeGroup.value,
    DataMap.Resource_Type.ExchangeEmail.value,
    ApplicationType.ActiveDirectory,
    ApplicationType.NASFileSystem,
    DataMap.Resource_Type.ndmp.value,
    ApplicationType.NASShare,
    ApplicationType.ObjectStorage,
    ApplicationType.Fileset,
    ApplicationType.Volume,
    ApplicationType.HDFS
  ];
  actionSort = [
    'full',
    'difference_increment',
    'cumulative_increment',
    'permanent_increment',
    'log'
  ];
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value,
      DataMap.Deploy_Type.e6000.value,
      DataMap.Deploy_Type.decouple.value
    ],
    this.i18n.get('deploy_type')
  );

  // 备份软件,包含X系列，E6000，软硬解耦
  isOceanProtect =
    this.appUtilsService.isDataBackup ||
    this.appUtilsService.isDecouple ||
    this.appUtilsService.isDistributed;
  replicationModeType = ReplicationModeType;
  isDmeUser = this.cookieService.get('userType') === CommonConsts.DME_USER_TYPE;
  autoIndexForObs = false; // 对象存储下支持自动索引的应用
  autoIndexForTape = false; // 磁带库下支持自动索引的应用
  archiveLogCopy = false; // 归档日志副本
  @Input() sla: any = {};
  @Input() activeIndex = '0';
  @Input() isTask = false;
  @Input() job: any = {};

  constructor(
    public appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    public i18n: I18NService,
    public datePipe: DatePipe,
    private qosServiceApi: QosService,
    public dataMapService: DataMapService,
    public slaParseService: SlaParseService,
    private clusterApiService: ClustersApiService,
    private mediaSetApiService: MediaSetApiService,
    private storageApiService: StoragesApiService,
    private nasDistributionStoragesApiService: NasDistributionStoragesApiService,
    private storageUserAuthService: StorageUserAuthService,
    private storageUnitService: StorageUnitService,
    private cookieService: CookieService,
    private cdr: ChangeDetectorRef,
    private dmeService: DmeServiceService
  ) {}

  ngOnInit() {
    if (
      includes(
        [
          ApplicationType.HBase,
          ApplicationType.Hive,
          ApplicationType.HDFS,
          ApplicationType.KubernetesStatefulSet,
          ApplicationType.Vmware,
          ApplicationType.HCSCloudHost,
          ApplicationType.FusionCompute,
          ApplicationType.TDSQL,
          ApplicationType.HyperV
        ],
        this.sla.application
      )
    ) {
      this.slaBackupName.difference_increment = 'common_permanent_backup_label';
    }
    this.getQosNames();
    this.initForm();
    this.initSla();
    if (this.isDmeUser) {
      this.getTargetZoneOps();
      this.getResourceOps();
    }
    this.autoIndexForObs = includes(
      [
        ApplicationType.NASFileSystem,
        ApplicationType.NASShare,
        ApplicationType.HDFS,
        ApplicationType.ImportCopy,
        ApplicationType.Fileset,
        ApplicationType.Ndmp
      ],
      this.sla.application
    );
    this.archiveLogCopy = includes(
      [ApplicationType.TDSQL],
      this.sla.application
    );
    this.autoIndexForTape = includes(
      [
        ApplicationType.NASFileSystem,
        ApplicationType.NASShare,
        ApplicationType.Fileset,
        ApplicationType.ObjectStorage,
        ApplicationType.Ndmp
      ],
      this.sla.application
    );
  }

  initForm() {
    this.formGroup = this.fb.group({});
  }

  initSla() {
    if (!this.sla) {
      return;
    }

    this.backupTeams = this.slaParseService.getBackupPolicy(
      this.sla,
      []
    ).policyList;

    this.backupTeams.sort((a, b) => {
      return (
        this.actionSort.indexOf(a.action) - this.actionSort.indexOf(b.action)
      );
    });
    assign(this.backupTeams[0], { applicationData: this.sla.application });

    this.archiveTeams = this.slaParseService.getArchival(
      this.sla,
      []
    ).policyList;

    this.replicationTeams = this.slaParseService.getReplication(
      this.sla,
      []
    ).policyList;

    if (isEmpty(this.backupTeams) && this.activeIndex !== '1') {
      this.activeIndex = !isEmpty(this.archiveTeams) ? '2' : '3';
    }

    if (!!this.archiveTeams.length) {
      this.getStorages();
      if (this.isDataBackup) {
        this.getClusterNodes();
      }
    }

    if (!!this.replicationTeams.length) {
      each(this.replicationTeams, item => {
        assign(item, {
          localStorageLabel: this.dataMapService.getLabel(
            'poolStorageDeviceType',
            item?.local_storage_type === 'BasicDisk'
              ? DataMap.poolStorageDeviceType.Server.value
              : item.local_storage_type
          ),
          remoteStorageLabel: this.dataMapService.getLabel(
            'poolStorageDeviceType',
            item?.remote_storage_type === 'BasicDisk'
              ? DataMap.poolStorageDeviceType.Server.value
              : item.remote_storage_type
          )
        });
      });
      this.getExtSystems(() => {
        this.getCrossCloudUnitName();
      });
    }

    if (this.isDataBackup && !this.isDmeUser) {
      this.initReplicationStorageNames();
    }
  }

  private getCrossCloudUnitName() {
    each(this.replicationTeams, item => {
      if (item.replication_target_mode === ReplicationModeType.CROSS_CLOUD) {
        let tmpClusterId = find(
          this.externalSystems,
          val =>
            !!intersection(val.clusterIp.split(','), [item.cluster_ip]).length
        ).clusterId;
        const params = {
          pageNo: CommonConsts.PAGE_START,
          pageSize: CommonConsts.PAGE_SIZE_MAX,
          userId: item.project_id,
          authType:
            item.replication_storage_type ===
            DataMap.backupStorageTypeSla.unit.value
              ? 1
              : 2,
          clusterId: tmpClusterId
        };
        this.storageUserAuthService
          .getRemoteStorageUserAuthRelationsByUserId(params)
          .subscribe((res: any) => {
            item.replication_storage_name = find(res.records, {
              storageId: item.storage_id
            }).storageName;
          });
      }
    });
  }

  initReplicationStorageNames() {
    this.replicationTeams.forEach(item => {
      this.getReplicationStorageName(item);
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

  getReplicationStorageName(policy) {
    if (this.replicationStorageNames.has(policy.storage_id)) {
      policy.replication_storage_name = this.replicationStorageNames.get(
        policy.storage_id
      );
      return;
    }
    if (!!policy.user_id) {
      const params = {
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        userId: policy.user_id,
        authType:
          policy.replication_storage_type ===
          DataMap.backupStorageTypeSla.unit.value
            ? 1
            : 2,
        clusterId: policy.external_system_id
      };
      assign(params, {
        userId: policy.user_id,
        authType:
          policy.replication_storage_type ===
          DataMap.backupStorageTypeSla.unit.value
            ? 1
            : 2,
        clusterId: policy.external_system_id,
        akDoException: !this.isTask
      });
      this.storageUserAuthService
        .getRemoteStorageUserAuthRelationsByUserId(params)
        .subscribe(res => {
          res.records.forEach(item => {
            this.replicationStorageNames.set(item.storageId, item.storageName);
            policy.replication_storage_name = this.replicationStorageNames.get(
              policy.storage_id
            );
          });
        });
    } else {
      this.getLocalStorageName(policy);
    }
  }

  getLocalStorageName(policy) {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      akDoException: !this.isTask
    };
    if (
      policy.replication_storage_type ===
      DataMap.backupStorageTypeSla.unit.value
    ) {
      this.storageUnitService.queryBackUnitGET(params).subscribe(res => {
        res.records.forEach(item => {
          this.replicationStorageNames.set(item.id, item.name);
          policy.replication_storage_name = this.replicationStorageNames.get(
            policy.storage_id
          );
        });
      });
    } else {
      this.nasDistributionStoragesApiService
        .ListNasDistributionStorages(params)
        .subscribe(res => {
          res.records.forEach(item => {
            this.replicationStorageNames.set(item.uuid, item.name);
            policy.replication_storage_name = this.replicationStorageNames.get(
              policy.storage_id
            );
          });
        });
    }
  }

  getMediaName(v) {
    const storageList = v?.storage_list[0];
    if (this.clusterNodeNames.length) {
      const clusterId = find(this.clusterNodeNames, {
        storageEsn: storageList.esn
      })['clusterId'];
      const name = find(this.mediaSetOptions[clusterId], {
        mediaSetId: storageList.storage_id
      })
        ? find(this.mediaSetOptions[clusterId], {
            mediaSetId: storageList.storage_id
          })['mediaSetName']
        : '--';
      return name;
    }
  }

  getMediaType(v) {
    const storageList = v?.storage_list[0];
    if (this.clusterNodeNames.length) {
      const clusterId = find(this.clusterNodeNames, {
        storageEsn: storageList.esn
      })['clusterId'];
      let mediaSet;
      if (
        find(this.mediaSetOptions[clusterId], {
          mediaSetId: storageList.storage_id
        })
      ) {
        mediaSet = find(this.mediaSetOptions[clusterId], {
          mediaSetId: storageList.storage_id
        });
        const retentionType = mediaSet.retentionType;
        const retentionLabel = this.dataMapService.getLabel(
          'Tape_Retention_Type',
          retentionType
        );
        const retentionUnit = this.dataMapService.getLabel(
          'Tape_Retention_Unit',
          mediaSet.retentionUnit
        );
        if (retentionType === DataMap.Tape_Retention_Type.temporary.value) {
          let resultLabel = `${retentionLabel}(${mediaSet.retentionDuration +
            retentionUnit})`;
          if (this.i18n.isEn) {
            return resultLabel.replace('（', '(').replace('）', ')');
          }
          return resultLabel;
        } else {
          return retentionLabel;
        }
      } else {
        return '--';
      }
    }
  }

  getNodeName(v) {
    const storageList = v?.storage_list[0];
    if (this.clusterNodeNames.length && storageList) {
      const cluster = find(this.clusterNodeNames, {
        storageEsn: storageList.esn
      });
      const name = cluster ? cluster['clusterName'] : '--';
      return name;
    }
  }

  getStorages() {
    let storageArr = [];
    this.storageApiService
      .storageUsingGET({
        startPage: 0,
        pageSize: 200,
        akDoException: !this.isTask
      })
      .subscribe(res => {
        if (res.totalCount === 0) {
          this.getMediaSets(storageArr);
          return;
        }
        each(res.records, item => {
          storageArr = union(storageArr, [item]);
        });
        this.getMediaSets(storageArr);
      });
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
      ],
      akDoException: !this.isTask
    };

    this.clusterApiService
      .getClustersInfoUsingGET(params)
      .subscribe((res: any) => {
        const nodes = map(res.records, item => {
          this.getMediaSets([], null, null, item);
          return {
            ...item,
            key: item.clusterId,
            value: item.clusterId,
            label: item.clusterName,
            isLeaf: true
          };
        });
        this.clusterNodeNames = nodes;
      });
  }

  getMediaSets(storageArr, recordsTemp?, startPage?, node?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      memberEsn: node?.storageEsn,
      akDoException: !this.isTask
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
        this.storageNames = unionWith(
          storageArr,
          recordsTemp,
          this.storageNames,
          isEqual
        );
        if (node) {
          set(this.mediaSetOptions, node?.clusterId, this.storageNames);
        }
        return;
      }
      this.getMediaSets(storageArr, recordsTemp, startPage, node);
    });
  }

  getExtSystems(callback?) {
    this.clusterApiService
      .getClustersInfoUsingGET({
        startPage: 0,
        pageSize: 200,
        typeList: [DataMap.Cluster_Type.target.value],
        akDoException: !this.isTask
      })
      .subscribe(res => {
        this.externalSystems = res.records;
        if (isFunction(callback)) {
          callback();
        }
      });
  }

  getQosNames() {
    this.qosServiceApi
      .queryResourcesV1QosGet({
        pageNo: 0,
        pageSize: 100,
        akDoException: !this.isTask
      })
      .subscribe(res => {
        this.qosNames = res.items;
      });
  }
}
