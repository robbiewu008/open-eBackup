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
import { Component, Input, OnInit, ViewChild } from '@angular/core';
import {
  DatatableComponent,
  MessageboxService,
  MessageService
} from '@iux/live';
import { AntiRansomwarePolicyApiService, compareVersion } from 'app/shared';
import { IODETECTPOLICYService, SlaApiService } from 'app/shared/api/services';
import {
  ApplicationType,
  CommonConsts,
  DataMap,
  PolicyAction,
  PolicyType,
  ProtectResourceAction,
  SLA_BACKUP_NAME
} from 'app/shared/consts';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import {
  DataMapService,
  GlobalService,
  I18NService
} from 'app/shared/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { SlaService } from 'app/shared/services/sla.service';
import {
  assign,
  difference,
  each,
  find,
  first,
  forEach,
  get,
  includes,
  intersection,
  isArray,
  isEmpty,
  isUndefined,
  lowerCase,
  map as _map,
  reject,
  size,
  some,
  toString,
  trim,
  uniq
} from 'lodash';
import { Subject } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-select-sla',
  templateUrl: './select-sla.component.html',
  styleUrls: ['./select-sla.component.less'],
  providers: [DatePipe]
})
export class SelectSlaComponent implements OnInit {
  @Input() filesetTemplate;
  disableSLA = false;
  querySlaName;
  applications;
  resourceData;
  originalSLAId;
  subResourceType;
  pageSize = 10;
  slaList = [];
  slaCardData;
  slaDatas = [];
  sizeOptions = [10];
  selectedSlaView = 0;
  actions = [];
  generalDbSupportBackupType = [];
  generalDbType;
  total = CommonConsts.PAGE_TOTAL;
  pageIndex1 = CommonConsts.PAGE_START;
  pageIndex2 = CommonConsts.PAGE_START;
  valid$ = new Subject<boolean>();
  policyAction = PolicyAction;
  hasRansomware = false; // 用于判断该资源是否有已经存在的防勒索策略，目前只用于文件集和nas共享
  hasAggregationApp = false; // 用于标识该应用的保护高级参数里是否有小文件聚合
  _intersection = intersection;
  isBatchProtect = false; // 用于判断批量保护场景
  actionIconMap = {
    [DataMap.Sla_Type.gold.value]: 'aui-sla-icon-gold',
    [DataMap.Sla_Type.silver.value]: 'aui-sla-icon-silver',
    [DataMap.Sla_Type.bronze.value]: 'aui-sla-icon-bronze'
  };
  actionsFilterMap = [
    {
      label: this.i18n.get('common_full_backup_label'),
      value: PolicyAction.FULL,
      key: PolicyAction.FULL
    },
    {
      label: this.i18n.get('common_incremental_backup_label'),
      value: PolicyAction.INCREMENT,
      key: PolicyAction.INCREMENT
    },
    {
      label: this.i18n.get('common_diff_backup_label'),
      value: PolicyAction.DIFFERENCE,
      key: PolicyAction.DIFFERENCE
    },
    {
      label: this.i18n.get('common_permanent_backup_label'),
      value: PolicyAction.PERMANENT,
      key: PolicyAction.PERMANENT
    },
    {
      label: this.i18n.get('common_log_backup_label'),
      value: PolicyAction.LOG,
      key: PolicyAction.LOG
    },
    {
      label: this.i18n.get('common_archive_label'),
      value: PolicyType.ARCHIVING,
      key: PolicyType.ARCHIVING
    },
    {
      label: this.i18n.get('common_replicate_label'),
      value: PolicyType.REPLICATION,
      key: PolicyType.REPLICATION
    }
  ];
  isBatchModify = false;
  @ViewChild(DatatableComponent, { static: false })
  lvTable: DatatableComponent;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private slaApiService: SlaApiService,
    private dataMapService: DataMapService,
    public slaService: SlaService,
    private messageBox: MessageboxService,
    private messageService: MessageService,
    private appUtilsService: AppUtilsService,
    private ioDetectPolicyService: IODETECTPOLICYService,
    private antiRansomwarePolicyApiService: AntiRansomwarePolicyApiService,
    private globalService: GlobalService
  ) {}

  everyLabel = this.i18n.get('common_every_label');
  operationLabel = this.i18n.get('common_operation_label');
  backupModeLabel = this.i18n.get('protection_backup_mode_label');
  executionPeriodLabel = this.i18n.get(
    'protection_execution_period_label',
    [],
    true
  );
  retentionPeriodLabel = this.i18n.get(
    'protection_retention_period_label',
    [],
    true
  );
  persistentLabel = this.i18n.get('common_persistent_label');
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isCyber = includes(
    [
      DataMap.Deploy_Type.cyberengine.value,
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cloudbackup.value
    ],
    this.i18n.get('deploy_type')
  );
  cyberCols: any[] = [
    {
      key: 'name',
      label: this.i18n.get('common_name_label'),
      show: true,
      width: '150px'
    },
    {
      key: 'policy1',
      label: this.i18n.get('common_policy_params_label', ['01']),
      show: true
    },
    {
      key: 'policy2',
      label: this.i18n.get('common_policy_params_label', ['02']),
      show: false
    },
    {
      key: 'policy3',
      label: this.i18n.get('common_policy_params_label', ['03']),
      show: false
    },
    {
      key: 'policy4',
      label: this.i18n.get('common_policy_params_label', ['04']),
      show: false
    }
  ];
  columnSelection = [];

  isRealDetection = false;
  sortKey;
  sortDirection;
  honeypotDetectStatus;
  wormTag = [{ label: 'WORM' }];
  isWormSLA = false;
  isWormData = false;

  ngOnInit() {
    this.disableSLA =
      !isUndefined(get(this.resourceData, 'disableSla')) &&
      this.resourceData.disableSla;
    if (
      this.resourceData?.subType === DataMap.Resource_Type.generalDatabase.value
    ) {
      this.getGeneralDbSupportBackupType();
    }
    this.initColumnSelection();
  }

  slaSelectedEvent(sla) {
    if (this.isCyberEngine) {
      return;
    }
    this.globalService.emitStore({
      action: 'slaSelectedEvent',
      state: sla
    });
  }

  initColumnSelection() {
    if (!this.isCyberEngine) {
      return;
    }
    if (this.isRealDetection) {
      this.cyberCols = [
        {
          key: 'name',
          label: this.i18n.get('common_name_label'),
          show: true,
          width: '150px'
        },
        {
          key: 'retentionDuration',
          label: this.i18n.get('explore_snapshot_lock_time_label'),
          isSort: true,
          show: true
        },
        {
          key: 'isIoEnhancedEnabled',
          label: this.i18n.get('explore_alarm_analysis_label'),
          filter: true,
          filterMap: this.dataMapService.toArray('switchStatus'),
          show: true
        },
        {
          key: 'isHoneypotDetectEnable',
          label: this.i18n.get('explore_decoy_detection_status_label'),
          filter: true,
          filterMap: this.dataMapService.toArray('switchStatus'),
          show: true
        },
        {
          key: 'period',
          label: this.i18n.get('explore_decoy_update_frequency_label'),
          isSort: true,
          show: true
        }
      ];
    }
    each(this.cyberCols, column => {
      this.columnSelection.push(column);
    });
    this.columnSelection = reject(this.columnSelection, item => !item.show);
  }

  columnCheck(source) {
    source.node.show = !source.node.show;
  }

  getDetectionPolicy(backupItem): string {
    return this.appUtilsService.getDetectionPolicy(backupItem, this.datePipe);
  }

  sortChange(source) {
    this.sortKey = source.key;
    this.sortDirection = source.direction;
    this.getSlaData();
  }

  cyberFilterChange(source) {
    this.honeypotDetectStatus = source.value;
    this.getSlaData();
  }

  getIconIdByAction(name: string) {
    if (this.isCyber) {
      return get(
        this.actionIconMap,
        name,
        'aui-sla-icon-cyber-detection-policy'
      );
    }
    return get(this.actionIconMap, name, 'aui-icon-sla-user-define');
  }

  getSlaList(createSla?) {
    if (createSla) {
      this.pageIndex1 = CommonConsts.PAGE_START;
    }
    if (this.isRealDetection) {
      const params = {
        pageNum: this.pageIndex1,
        pageSize: this.pageSize
      };
      this.ioDetectPolicyService
        .pageQueryIoDetectPolicy(params)
        .subscribe(res => {
          each(res.records, item => {
            assign(item, {
              uuid: item.id
            });
          });
          this.slaList = res.records;
          this.total = res.totalCount;
          if (find(res.records, { uuid: createSla })) {
            this.slaChange(createSla);
          }
        });
    } else {
      const params = {
        pageNo: this.pageIndex1,
        pageSize: this.pageSize,
        applications: this.applications
      };
      if (this.querySlaName) {
        assign(params, { name: this.querySlaName });
      }

      if (!!this.actions.length) {
        assign(params, { actions: this.actions });
      }
      this.slaApiService
        .pageQueryUsingGET(params)
        .pipe(
          map(result => {
            return this.convertAction(result);
          })
        )
        .subscribe(res => {
          this.slaList = res.items;
          this.total = res.total;
          const tmpSla = find(res.items, { uuid: createSla });
          if (
            find(res.items, { uuid: createSla }) &&
            !(
              this.hasAggregationApp &&
              this.hasRansomware &&
              !!find(tmpSla.policy_list, { action: PolicyAction.INCREMENT })
            )
          ) {
            this.slaChange(createSla);
          }
          this.slaList.forEach(item => {
            item.isWormSLAList = some(
              item.policy_list,
              policy => policy?.worm_validity_type >= 1
            );
            item.disabled =
              this.hasAggregationApp &&
              this.hasRansomware &&
              find(item.policy_list, { action: PolicyAction.INCREMENT });
          });
        });
    }
  }

  getSlaData(createSla?) {
    if (createSla) {
      this.pageIndex2 = CommonConsts.PAGE_START;
    }
    if (this.isRealDetection) {
      this.getRealDetectionPolicyData(createSla);
    } else {
      this.getCommonSlaData(createSla);
    }
  }

  private getRealDetectionPolicyData(createSla) {
    const params = {
      pageNum: this.pageIndex1,
      pageSize: this.pageSize
    };
    if (this.querySlaName) {
      assign(params, { name: this.querySlaName });
    }
    if (!isEmpty(this.honeypotDetectStatus)) {
      assign(params, { honeypotDetectStatus: this.honeypotDetectStatus });
    }
    if (this.sortKey && this.sortDirection) {
      assign(params, {
        orderBy: this.sortKey,
        orderType: this.sortDirection
      });
    }
    this.ioDetectPolicyService
      .pageQueryIoDetectPolicy(params)
      .subscribe(res => {
        each(res.records, item => {
          assign(item, {
            uuid: item.id
          });
        });
        this.slaDatas = res.records;
        this.total = res.totalCount;
        if (find(res.records, { uuid: createSla })) {
          this.selectionRow(createSla);
        }
      });
  }

  private getCommonSlaData(createSla) {
    const params = {
      pageNo: this.pageIndex2,
      pageSize: this.pageSize,
      applications: this.applications
    };
    if (this.querySlaName) {
      assign(params, { name: this.querySlaName });
    }

    if (!!this.actions.length) {
      assign(params, { actions: this.actions });
    }

    const processSlaData = () => {
      if (this.isCyberEngine) {
        each(this.slaDatas, item => {
          assign(item, {
            policy1: find(item.policy_list, item => includes(item.name, '01')),
            policy2: find(item.policy_list, item => includes(item.name, '02')),
            policy3: find(item.policy_list, item => includes(item.name, '03')),
            policy4: find(item.policy_list, item => includes(item.name, '04'))
          });
        });
      }
      // 要判断部署形态
      if (!this.isCyber) {
        this.slaDatas.forEach(item => {
          assign(item, {
            isWormSLAList: item.policy_list?.some(
              e => e?.worm_validity_type >= 1
            ),
            disabled:
              this.hasAggregationApp &&
              this.hasRansomware &&
              find(item.policy_list, { action: PolicyAction.INCREMENT })
          });
        });
      }
      const tmpSla = find(this.slaDatas, { uuid: createSla });
      if (
        find(this.slaDatas, { uuid: createSla }) &&
        !(
          this.hasAggregationApp &&
          this.hasRansomware &&
          !!find(tmpSla.policy_list, { action: PolicyAction.INCREMENT })
        )
      ) {
        this.selectionRow(createSla);
      }
    };

    this.slaApiService
      .pageQueryUsingGET(params)
      .pipe(
        map(result => {
          return this.convertAction(result);
        })
      )
      .subscribe(res => {
        this.slaDatas = res.items;
        this.total = res.total;
        processSlaData();
      });
  }

  getDetail(e, item) {
    e.stopPropagation();
    if (this.isCyberEngine) {
      if (this.isRealDetection) {
        this.slaService.getRealDetectionPolicyDetail(item);
      } else {
        this.slaService.getAntiDetail(item);
      }
    } else {
      this.slaService.getDetail(item);
    }
  }

  convertAction(result) {
    const res = result || ({ items: [], total: 0 } as any);
    forEach(res.items, item => {
      const full = find(item.policy_list, policy => {
        return policy.action === PolicyAction.FULL;
      }) as any;
      const diff = find(item.policy_list, policy => {
        return policy.action === PolicyAction.DIFFERENCE;
      }) as any;
      const increment = find(item.policy_list, policy => {
        return policy.action === PolicyAction.INCREMENT;
      }) as any;
      const permanent = find(item.policy_list, policy => {
        return policy.action === PolicyAction.PERMANENT;
      }) as any;
      const log = find(item.policy_list, policy => {
        return policy.action === PolicyAction.LOG;
      }) as any;
      const snapshot = find(item.policy_list, policy => {
        return policy.action === PolicyAction.SNAPSHOT;
      }) as any;
      const archiving = find(item.policy_list, policy => {
        return policy.type === PolicyType.ARCHIVING;
      }) as any;
      const replication = find(item.policy_list, policy => {
        return policy.type === PolicyType.REPLICATION;
      }) as any;
      const backupModes = [];
      if (full) {
        backupModes.push(this.i18n.get('common_full_backup_label'));
      }
      if (diff) {
        backupModes.push(this.i18n.get('common_diff_backup_label'));
      }
      if (increment) {
        if (
          includes(
            [
              ApplicationType.HBase,
              ApplicationType.Hive,
              ApplicationType.HDFS,
              ApplicationType.KubernetesStatefulSet,
              ApplicationType.KubernetesDatasetCommon,
              ApplicationType.Vmware,
              ApplicationType.HCSCloudHost,
              ApplicationType.FusionCompute,
              ApplicationType.FusionOne,
              ApplicationType.OpenStack,
              ApplicationType.ApsaraStack,
              ApplicationType.TDSQL,
              ApplicationType.HyperV,
              ApplicationType.CNware,
              ApplicationType.Nutanix
            ],
            item.application
          )
        ) {
          backupModes.push(this.i18n.get('common_permanent_backup_label'));
        } else {
          backupModes.push(this.i18n.get('common_incremental_backup_label'));
        }
      }
      if (permanent) {
        backupModes.push(this.i18n.get('common_permanent_backup_label'));
      }
      if (log) {
        backupModes.push(this.i18n.get('common_log_backup_label'));
      }
      if (archiving) {
        backupModes.push(this.i18n.get('common_archive_label'));
      }
      if (replication) {
        backupModes.push(this.i18n.get('common_replicate_label'));
      }
      if (snapshot) {
        backupModes.push(this.i18n.get('common_anti_detection_snapshot_label'));
      }

      item['backup_mode'] = toString(backupModes).replace(/,/g, ' + ');
    });
    return res;
  }

  chunkArray(array, size) {
    const chunks = [];
    for (let i = 0; i < array.length; i += size) {
      chunks.push(array.slice(i, i + size));
    }
    return chunks;
  }

  getDataWormStatus(data) {
    const resourceIdList = _map(isArray(data) ? data : [data], 'uuid');
    // 因为接口最多一次查100个，所以判断一下
    if (size(resourceIdList) > 100) {
      const chunks = this.chunkArray(resourceIdList, 100);
      each(chunks, item => {
        this.getWormStatus({
          resourceIds: item
        });
      });
    } else {
      this.getWormStatus({
        resourceIds: resourceIdList
      });
    }
  }

  private getWormStatus(resourceParams) {
    this.antiRansomwarePolicyApiService
      .ShowAntiRansomwarePolicies(resourceParams)
      .subscribe(res => {
        this.isWormData = !!find(res.records, item => item.schedule.setWorm);
        this.hasRansomware = !!find(
          res.records,
          item => item.schedule.needDetect
        );
        if (!this.hasAggregationApp) {
          return;
        }
        // 如果后获取完防勒索策略信息再做一次判断
        this.setSlaDisabled(this.slaList);
        this.setSlaDisabled(this.slaDatas);
        // 获取之后通知高级组件
        this.globalService.emitStore({
          action: 'syncRansomwareStatus',
          state: this.hasRansomware
        });
      });
  }

  setSlaDisabled(slaArray) {
    each(slaArray, item => {
      item.disabled =
        this.hasRansomware &&
        find(item.policy_list, { action: PolicyAction.INCREMENT });
    });
  }

  initData(data: any, _?, action?: ProtectResourceAction) {
    if (!this.isCyber) {
      this.getDataWormStatus(data);
    }
    this.resourceData = isArray(data) ? data[0] : data;
    this.isBatchProtect = isArray(data);
    this.subResourceType =
      this.resourceData.sub_type || this.resourceData.sourceSubType;
    this.isBatchModify =
      includes([DataMap.Resource_Type.fileset.value], this.subResourceType) &&
      isArray(data) &&
      data.length > 1 &&
      action === ProtectResourceAction.Modify;
    this.originalSLAId = this.resourceData.sla_id;
    if (this.isBatchModify) {
      this.resourceData.sla_id = '';
    }
    this.isRealDetection =
      this.isCyberEngine && this.resourceData.isRealDetection === true;
    if (this.resourceData.sla_id) {
      this.getOriginalSlaInfo();
    }
    const applicationTypes = this.dataMapService.toArray('Application_Type', [
      DataMap.Application_Type.LocalFileSystem.value
    ]);
    if (
      includes(
        [DataMap.Resource_Type.generalDatabase.value],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.generalDatabase.value
      ];
    } else if (
      includes(
        [
          DataMap.Resource_Type.SQLServerClusterInstance.value,
          DataMap.Resource_Type.SQLServerInstance.value,
          DataMap.Resource_Type.SQLServerCluster.value,
          DataMap.Resource_Type.SQLServerGroup.value,
          DataMap.Resource_Type.SQLServerDatabase.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [DataMap.Resource_Type.SQLServer.value];
    } else if (
      includes(
        [
          DataMap.Resource_Type.hostSystem.value,
          DataMap.Resource_Type.clusterComputeResource.value,
          DataMap.Resource_Type.virtualMachine.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [DataMap.Resource_Type.virtualMachine.value];
    } else if (
      includes(
        [
          DataMap.Resource_Type.cNwareCluster.value,
          DataMap.Resource_Type.cNwareHost.value,
          DataMap.Resource_Type.cNwareVm.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.Common, ApplicationType.CNware];
    } else if (
      includes(
        [
          DataMap.Resource_Type.nutanixCluster.value,
          DataMap.Resource_Type.nutanixHost.value,
          DataMap.Resource_Type.nutanixVm.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.Common, ApplicationType.Nutanix];
    } else if (
      includes(
        [DataMap.Resource_Type.ActiveDirectory.value],
        this.subResourceType
      )
    ) {
      this.applications = [DataMap.Resource_Type.ActiveDirectory.value];
    } else if (
      includes(
        [
          DataMap.Resource_Type.msVirtualMachine.value,
          DataMap.Resource_Type.msHostSystem.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.msVirtualMachine.value
      ];
    } else if (
      includes(
        [
          DataMap.Resource_Type.Replica.value,
          DataMap.Resource_Type.NASShare.value,
          DataMap.Resource_Type.ImportCopy.value,
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.LocalFileSystem.value,
          DataMap.Resource_Type.Redis.value,
          DataMap.Resource_Type.commonShare.value,
          DataMap.Resource_Type.LocalLun.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [this.subResourceType];
    } else if (
      includes(
        [
          DataMap.Resource_Type.gaussdbForOpengaussProject.value,
          DataMap.Resource_Type.gaussdbForOpengaussInstance.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [
        DataMap.Resource_Type.gaussdbForOpengaussInstance.value
      ];
    } else if (
      includes(
        [DataMap.Resource_Type.lightCloudGaussdbInstance.value],
        this.subResourceType
      )
    ) {
      this.applications = [
        DataMap.Resource_Type.lightCloudGaussdbInstance.value
      ];
    } else if (
      includes([DataMap.Resource_Type.GaussDB_T.value], this.subResourceType)
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.GaussDB_T.value
      ];
    } else if (
      includes(
        [DataMap.Resource_Type.gaussdbTSingle.value],
        this.subResourceType
      )
    ) {
      this.applications = [DataMap.Resource_Type.GaussDB_T.value];
    } else if (
      includes(
        [
          DataMap.Resource_Type.GaussDB_DWS.value,
          DataMap.Resource_Type.DWS_Database.value,
          DataMap.Resource_Type.DWS_Schema.value,
          DataMap.Resource_Type.DWS_Table.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [DataMap.Resource_Type.GaussDB_DWS.value];
    } else if (
      includes(
        [
          DataMap.Resource_Type.dbTwoDatabase.value,
          DataMap.Resource_Type.dbTwoTableSet.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.DB2.value
      ];
    } else if (
      includes(
        [
          DataMap.Resource_Type.ClickHouse.value,
          DataMap.Resource_Type.ClickHouseCluster.value,
          DataMap.Resource_Type.ClickHouseDatabase.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [DataMap.Resource_Type.ClickHouse.value];
    } else if (
      includes(
        [
          DataMap.Resource_Type.KubernetesNamespace.value,
          DataMap.Resource_Type.KubernetesStatefulset.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        ApplicationType.KubernetesStatefulSet
      ];
    } else if (
      includes(
        [
          DataMap.Resource_Type.kubernetesNamespaceCommon.value,
          DataMap.Resource_Type.kubernetesDatasetCommon.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.KubernetesDatasetCommon];
    } else if (
      includes(
        [
          DataMap.Resource_Type.MySQLInstance.value,
          DataMap.Resource_Type.MySQLClusterInstance.value,
          DataMap.Resource_Type.MySQLDatabase.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.MySQL.value
      ];
    } else if (
      includes(
        [
          DataMap.Resource_Type.PostgreSQLInstance.value,
          DataMap.Resource_Type.PostgreSQLClusterInstance.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [DataMap.Resource_Type.PostgreSQL.value];
    } else if (
      includes(
        [
          DataMap.Resource_Type.AntDBInstance.value,
          DataMap.Resource_Type.AntDBClusterInstance.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [DataMap.Resource_Type.AntDB.value];
    } else if (
      includes(
        [
          DataMap.Resource_Type.informixInstance.value,
          DataMap.Resource_Type.informixClusterInstance.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.informixService.value
      ];
    } else if (
      includes(
        [DataMap.Resource_Type.FusionCompute.value],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.FusionCompute.value
      ];
    } else if (
      includes([DataMap.Resource_Type.fusionOne.value], this.subResourceType)
    ) {
      this.applications = [ApplicationType.Common, ApplicationType.FusionOne];
    } else if (
      includes(
        [DataMap.Resource_Type.goldendbInstance.value],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.goldendb.value
      ];
    } else if (
      includes(
        [
          DataMap.Resource_Type.KingBaseInstance.value,
          DataMap.Resource_Type.KingBaseClusterInstance.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [DataMap.Resource_Type.KingBase.value];
    } else if (
      includes(
        [
          DataMap.Resource_Type.HCSProject.value,
          DataMap.Resource_Type.HCSCloudHost.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.HCSCloudHost.value
      ];
    } else if (
      includes(
        [
          DataMap.Resource_Type.Dameng_cluster.value,
          DataMap.Resource_Type.Dameng_singleNode.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.Dameng.value
      ];
    } else if (
      includes(
        [
          DataMap.Resource_Type.OpenGauss_database.value,
          DataMap.Resource_Type.OpenGauss_instance.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.OpenGauss.value
      ];
    } else if (
      includes(
        [
          DataMap.Resource_Type.tdsqlInstance.value,
          DataMap.Resource_Type.tdsqlDistributedInstance.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [DataMap.Resource_Type.tdsqlInstance.value];
    } else if (
      includes(
        [DataMap.Resource_Type.ElasticsearchBackupSet.value],
        this.subResourceType
      )
    ) {
      this.applications = [
        ApplicationType.Common,
        DataMap.Resource_Type.ElasticsearchBackupSet.value
      ];
    } else if (
      includes([DataMap.Resource_Type.fileset.value], this.subResourceType)
    ) {
      this.applications = [DataMap.Resource_Type.fileset.value];
    } else if (
      includes([DataMap.Resource_Type.volume.value], this.subResourceType)
    ) {
      this.applications = [DataMap.Resource_Type.volume.value];
    } else if (
      includes(
        [
          DataMap.Resource_Type.openStackProject.value,
          DataMap.Resource_Type.openStackCloudServer.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.Common, ApplicationType.OpenStack];
    } else if (
      includes(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value,
          DataMap.Resource_Type.oraclePDB.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.Common, ApplicationType.Oracle];
    } else if (
      includes(
        [
          DataMap.Resource_Type.MongodbSingleInstance.value,
          DataMap.Resource_Type.MongodbClusterInstance.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.MongoDB];
    } else if (
      includes(
        [
          DataMap.Resource_Type.OceanBaseCluster.value,
          DataMap.Resource_Type.OceanBaseTenant.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.Common, ApplicationType.OceanBase];
    } else if (
      includes(
        [
          DataMap.Resource_Type.tidbCluster.value,
          DataMap.Resource_Type.tidbDatabase.value,
          DataMap.Resource_Type.tidbTable.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.TiDB];
    } else if (
      includes([DataMap.Resource_Type.ObjectSet.value], this.subResourceType)
    ) {
      this.applications = [ApplicationType.ObjectStorage];
    } else if (
      includes(
        [
          DataMap.Resource_Type.ExchangeDataBase.value,
          DataMap.Resource_Type.ExchangeGroup.value,
          DataMap.Resource_Type.ExchangeSingle.value,
          DataMap.Resource_Type.ExchangeEmail.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.Common, ApplicationType.Exchange];
    } else if (
      includes(
        [
          DataMap.Resource_Type.APSCloudServer.value,
          DataMap.Resource_Type.APSResourceSet.value,
          DataMap.Resource_Type.APSZone.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.Common, ApplicationType.ApsaraStack];
    } else if (
      includes(
        [
          DataMap.Resource_Type.hyperVHost.value,
          DataMap.Resource_Type.hyperVVm.value
        ],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.HyperV];
    } else if (
      includes([DataMap.Resource_Type.ndmp.value], this.subResourceType)
    ) {
      this.applications = [ApplicationType.Ndmp];
    } else if (
      includes(
        [DataMap.Resource_Type.saphanaDatabase.value],
        this.subResourceType
      )
    ) {
      this.applications = [DataMap.Resource_Type.saphanaDatabase.value];
    } else if (
      includes(
        [DataMap.Resource_Type.saponoracleDatabase.value],
        this.subResourceType
      )
    ) {
      this.applications = [ApplicationType.Saponoracle];
    } else {
      this.applications = _map(
        applicationTypes.filter(
          v =>
            lowerCase(v.key) === lowerCase(this.subResourceType) ||
            v.value === ApplicationType.Common
        ),
        'value'
      );
    }
    // 判断这些应用是有小文件聚合的
    if (
      !!intersection(this.applications, [
        ApplicationType.Fileset,
        ApplicationType.NASShare
      ]).length
    ) {
      this.hasAggregationApp = true;
    }
    this.viewChange(this.selectedSlaView);
  }

  getOriginalSlaInfo() {
    if (!this.resourceData.sla_id) {
      return;
    }
    // 区分oceancyber实时侦测
    if (this.isRealDetection) {
      this.ioDetectPolicyService
        .getIoDetectPolicyById({
          akDoException: false,
          policyId: this.resourceData.sla_id
        })
        .subscribe({
          next: res => {
            assign(this.resourceData, {
              slaObject: res
            });
          },
          error: () => {
            this.valid$.next(false);
          }
        });
    } else {
      this.slaApiService
        .querySLAUsingGET({
          akDoException: false,
          slaId: this.resourceData.sla_id
        })
        .subscribe({
          next: res => {
            assign(this.resourceData, {
              slaObject: res
            });
            this.globalService.emitStore({
              action: 'slaObjectUpdated',
              state: true
            });
            this.slaSelectedEvent(res);
          },
          error: () => {
            this.valid$.next(false);
          }
        });
    }
  }

  viewChange(val, createSla?) {
    this.selectedSlaView = val;
    if (!this.selectedSlaView) {
      this.getSlaList(createSla);
    } else {
      this.getSlaData(createSla);
    }
  }

  beforeChange(sla_id) {
    const originalPolicy = this.slaList.find(
      item => item.uuid === this.originalSLAId
    );
    const currentPolicy = this.slaList.find(item => item.uuid === sla_id);
    if (
      this.subResourceType === DataMap.Resource_Type.oracle.value &&
      !isUndefined(originalPolicy) &&
      !isUndefined(
        originalPolicy.policy_list.find(
          item => item.action === PolicyAction.LOG
        )
      ) &&
      !isUndefined(currentPolicy) &&
      isUndefined(
        currentPolicy.policy_list.find(item => item.action === PolicyAction.LOG)
      )
    ) {
      return true;
    }
    return false;
  }

  slaChange(sla_id) {
    if (!this.vaildBackupPolicy(sla_id)) {
      return;
    }

    const tmpSla = this.slaList.find(item => item.uuid === sla_id);
    this.isWormSLA =
      !this.isCyber &&
      !!(
        tmpSla && tmpSla.policy_list.find(item => item.worm_validity_type >= 1)
      );

    if (this.beforeChange(sla_id)) {
      this.messageBox.confirm({
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvContent: this.i18n.get('protection_sla_change_label'),
        lvOk: () => {
          assign(this.resourceData, {
            sla_id,
            slaObject: this.slaList.find(item => item.uuid === sla_id)
          });
          this.valid$.next(!isEmpty(sla_id));
        },
        lvCancel: () => {
          this.resourceData.sla_id = this.originalSLAId;
          this.valid$.next(!isEmpty(this.originalSLAId));
        },
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            this.resourceData.sla_id = this.originalSLAId;
            this.valid$.next(!isEmpty(this.originalSLAId));
          }
        }
      });
    } else {
      assign(this.resourceData, {
        sla_id,
        slaObject: this.slaList.find(item => item.uuid === sla_id)
      });
      this.valid$.next(!isEmpty(sla_id));
    }
    this.slaSelectedEvent(this.slaList.find(item => item.uuid === sla_id));
    this.compareSlaEncryption();
  }

  compareSlaEncryption() {
    if (
      !isEmpty(this.originalSLAId) &&
      this.subResourceType !== DataMap.Resource_Type.oracle.value
    ) {
      return;
    }

    const originalSlaInfo = this.slaList.find(
      item => item.uuid === this.originalSLAId
    );
    const oldSlaEncryption = get(
      originalSlaInfo,
      'policy_list[0].ext_parameters.encryption'
    );
    const currentSlaEncryption = get(
      this.resourceData.slaObject,
      'policy_list[0].ext_parameters.encryption'
    );

    if (
      oldSlaEncryption &&
      currentSlaEncryption &&
      oldSlaEncryption !== currentSlaEncryption
    ) {
      this.messageService.info(
        this.i18n.get('protection_sla_diff_encryption_label'),
        {
          lvMessageKey: 'diff_encryption_key',
          lvShowCloseButton: true
        }
      );
    }
  }

  slaDataPageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex2 = page.pageIndex;
    this.getSlaData();
  }

  slaListPageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex1 = page.pageIndex;
    this.getSlaList();
  }

  selectionRow(sla_id, notManualClick?) {
    const tmpSla = this.slaDatas.find(item => item.uuid === sla_id);
    if (tmpSla.disabled) {
      return;
    }
    this.isWormSLA =
      !this.isCyber &&
      !!(
        (
          tmpSla &&
          tmpSla.policy_list.find(item => item.worm_validity_type >= 1)
        ) // item.worm_validity_type取值是null 0 1 2
      );
    if (!this.vaildBackupPolicy(sla_id)) {
      assign(this.resourceData, {
        sla_id,
        slaObject: this.slaDatas.find(item => item.uuid === sla_id)
      });
      return;
    }

    if (this.beforeChange(sla_id) && !notManualClick) {
      this.handleSlaChangeConfirm(sla_id);
    } else {
      this.toggleSelectionAndValid(sla_id);
    }
    this.slaSelectedEvent(this.slaDatas.find(item => item.uuid === sla_id));
    this.compareSlaEncryption();
  }

  private handleSlaChangeConfirm(sla_id) {
    this.messageBox.confirm({
      lvOkType: 'primary',
      lvCancelType: 'default',
      lvContent: this.i18n.get('protection_sla_change_label'),
      lvOk: () => {
        this.toggleSelectionAndValid(sla_id);
      },
      lvCancel: () => {
        this.toggleSelectionAndValid(sla_id);
        setTimeout(() => {
          this.selectionRow(this.originalSLAId, true);
        });
      },
      lvAfterClose: result => {
        if (result && result.trigger === 'close') {
          this.toggleSelectionAndValid(sla_id);
          setTimeout(() => {
            this.selectionRow(this.originalSLAId, true);
          });
        }
      }
    });
  }

  private toggleSelectionAndValid(sla_id) {
    assign(this.resourceData, {
      sla_id,
      slaObject: this.slaDatas.find(item => item.uuid === sla_id)
    });
    this.lvTable.toggleSelection(sla_id);
    this.valid$.next(!isEmpty(sla_id));
  }

  searchByName(value) {
    this.querySlaName = trim(value);
    this.getSlaData();
  }

  filterChange = event => {
    this.actions = event.value;
    this.getSlaData();
  };

  createSla() {
    const handleResponse = (policyKey: string) => res => {
      try {
        res = JSON.parse(res);
      } catch (error) {}
      this.viewChange(this.selectedSlaView, res[policyKey]);
    };
    if (this.isCyberEngine) {
      if (this.isRealDetection) {
        this.slaService.createRealDetectionPolicy(
          handleResponse('policyId'),
          null,
          null
        );
      } else {
        this.slaService.createDetectionPolicy(
          handleResponse('uuid'),
          null,
          null,
          true
        );
      }
      return;
    }
    this.slaService.create(handleResponse('uuid'), {
      application: this.getApplicationType()
    });
  }

  getApplicationType() {
    let applicationType = this.subResourceType;
    if (
      includes([DataMap.Resource_Type.msHostSystem.value], this.subResourceType)
    ) {
      applicationType = DataMap.Resource_Type.msVirtualMachine.value;
    } else if (
      includes(
        [
          DataMap.Resource_Type.hostSystem.value,
          DataMap.Resource_Type.clusterComputeResource.value
        ],
        this.subResourceType
      )
    ) {
      applicationType = DataMap.Resource_Type.virtualMachine.value;
    } else if (
      includes(
        [
          DataMap.Resource_Type.DWS_Cluster.value,
          DataMap.Resource_Type.DWS_Database.value,
          DataMap.Resource_Type.DWS_Schema.value,
          DataMap.Resource_Type.DWS_Table.value
        ],
        this.subResourceType
      )
    ) {
      applicationType = DataMap.Resource_Type.GaussDB_DWS.value;
    } else if (
      includes(
        [
          DataMap.Resource_Type.ClickHouse.value,
          DataMap.Resource_Type.ClickHouseCluster.value,
          DataMap.Resource_Type.ClickHouseDatabase.value,
          DataMap.Resource_Type.ClickHouseTableset.value
        ],
        this.subResourceType
      )
    ) {
      applicationType = DataMap.Resource_Type.ClickHouse.value;
    } else if (
      includes(
        [
          DataMap.Resource_Type.ExchangeDataBase.value,
          DataMap.Resource_Type.ExchangeEmail.value,
          DataMap.Resource_Type.ExchangeGroup.value,
          DataMap.Resource_Type.ExchangeSingle.value,
          DataMap.Resource_Type.Exchange.value
        ],
        this.subResourceType
      )
    ) {
      applicationType = DataMap.Resource_Type.ExchangeDataBase.value;
    } else {
      const resource = this.appUtilsService.findResourceTypeByKey('slaId');
      each(resource, (key: any[] | string, slaId) => {
        if (
          (isArray(key) && key.includes(this.subResourceType)) ||
          key === this.subResourceType
        ) {
          applicationType = slaId;
          return false; // 找到后跳出each循环
        }
      });
    }
    return applicationType;
  }
  onOK() {
    return this.resourceData;
  }

  cardEnter(item) {
    this.slaCardData = item;
  }

  vaildBackupPolicy(sla_id) {
    if (
      includes([DataMap.Resource_Type.oraclePDB.value], this.subResourceType)
    ) {
      let result = true;
      const policy = get(
        find(
          this.selectedSlaView ? this.slaDatas : this.slaList,
          item => item.uuid === sla_id
        ),
        'policy_list'
      );
      if (
        find(policy, item =>
          [PolicyType.REPLICATION, PolicyType.ARCHIVING].includes(item.type)
        )
      ) {
        result = false;
      }

      if (!result) {
        this.messageService.error(
          this.i18n.get(
            'protection_oracle_pdb_not_support_repl_and_archive_label'
          )
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      includes(
        [
          DataMap.Resource_Type.ExchangeSingle.value,
          DataMap.Resource_Type.ExchangeGroup.value,
          DataMap.Resource_Type.ExchangeDataBase.value
        ],
        this.subResourceType
      )
    ) {
      let result = true;

      if (this.selectedSlaView) {
        const policy = get(
          find(this.slaDatas, item => item.uuid === sla_id),
          'policy_list'
        );

        if (find(policy, item => item.action === PolicyAction.INCREMENT)) {
          result = false;
        }
      } else {
        const policy = get(
          find(this.slaList, item => item.uuid === sla_id),
          'policy_list'
        );

        if (find(policy, item => item.action === PolicyAction.INCREMENT)) {
          result = false;
        }
      }

      if (!result) {
        const policyStr: string = this.i18n.get(
          'common_incremental_backup_label'
        );
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            this.i18n.getJobTargetType(this.subResourceType),
            '',
            policyStr.toLowerCase()
          ])
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      includes(
        [DataMap.Resource_Type.ExchangeEmail.value],
        this.subResourceType
      )
    ) {
      let result = true;
      const policy = get(
        find(
          this.selectedSlaView ? this.slaDatas : this.slaList,
          item => item.uuid === sla_id
        ),
        'policy_list'
      );
      if (
        find(policy, item =>
          [PolicyAction.LOG, PolicyAction.PERMANENT].includes(item.action)
        )
      ) {
        result = false;
      }

      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_exchange_email_sla_tips_label')
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      includes(
        [
          DataMap.Resource_Type.DWS_Database.value,
          DataMap.Resource_Type.DWS_Schema.value,
          DataMap.Resource_Type.DWS_Table.value
        ],
        this.subResourceType
      )
    ) {
      let result = true;
      let action = '';
      // schema集,且版本为8.2.1及以上时，可以选择增量、差异备份；小于8.2.1时选择增量、差异备份弹窗报错；
      const version = this.resourceData?.environment?.version;
      const schemaEnableFlag =
        includes(
          [DataMap.Resource_Type.DWS_Schema.value],
          this.resourceData.subType
        ) && compareVersion(version, '8.2.1') !== -1;

      if (this.selectedSlaView) {
        const policy = find(
          get(
            find(this.slaDatas, item => item.uuid === sla_id),
            'policy_list'
          ),
          item =>
            includes(
              [PolicyAction.DIFFERENCE, PolicyAction.INCREMENT],
              item.action
            )
        );

        if (!!policy && !schemaEnableFlag) {
          result = false;
          action = policy.action;
        }
      } else {
        const policy = find(
          get(
            find(this.slaList, item => item.uuid === sla_id),
            'policy_list'
          ),
          item =>
            includes(
              [PolicyAction.DIFFERENCE, PolicyAction.INCREMENT],
              item.action
            )
        );

        if (!!policy && !schemaEnableFlag) {
          result = false;
          action = policy.action;
        }
      }

      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            DataMap.Resource_Type.GaussDB_DWS.label,
            includes(
              [DataMap.Resource_Type.DWS_Database.value],
              this.resourceData.subType
            )
              ? this.i18n.get('common_database_label')
              : includes(
                  [DataMap.Resource_Type.DWS_Schema.value],
                  this.resourceData.subType
                )
              ? this.i18n.get('protection_schema_set_label')
              : this.i18n.get('protection_table_set_label'),
            action === PolicyAction.DIFFERENCE
              ? this.i18n.get('common_diff_backup_label')
              : this.i18n.get('common_incremental_backup_label')
          ])
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      includes(
        [DataMap.Resource_Type.Dameng_cluster.value],
        this.resourceData.subType
      )
    ) {
      let result = true;
      if (this.selectedSlaView) {
        const policy = get(
          find(this.slaDatas, item => item.uuid === sla_id),
          'policy_list'
        );

        if (
          find(
            policy,
            item =>
              item.action === PolicyAction.LOG ||
              item.action === PolicyType.ARCHIVING
          )
        ) {
          result = false;
        }
      } else {
        const policy = get(
          find(this.slaList, item => item.uuid === sla_id),
          'policy_list'
        );

        if (
          find(
            policy,
            item =>
              item.action === PolicyAction.LOG ||
              item.action === PolicyType.ARCHIVING
          )
        ) {
          result = false;
        }
      }
      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_dameng_policy_label')
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      includes(
        [DataMap.Resource_Type.dbTwoTableSet.value],
        this.subResourceType
      )
    ) {
      let result = true;

      if (this.selectedSlaView) {
        const policy = get(
          find(this.slaDatas, item => item.uuid === sla_id),
          'policy_list'
        );

        if (find(policy, item => item.action === PolicyAction.LOG)) {
          result = false;
        }
      } else {
        const policy = get(
          find(this.slaList, item => item.uuid === sla_id),
          'policy_list'
        );

        if (find(policy, item => item.action === PolicyAction.LOG)) {
          result = false;
        }
      }

      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            DataMap.Resource_Type.DB2.label,
            this.i18n.get('protection_table_space_set_label'),
            this.i18n.get('common_log_backup_label')
          ])
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      includes(
        [DataMap.Resource_Type.dbTwoTableSet.value],
        this.resourceData?.resource_sub_type
      ) &&
      this.subResourceType === ApplicationType.Replica
    ) {
      let result = true;

      if (this.selectedSlaView) {
        const policy = get(
          find(this.slaDatas, item => item.uuid === sla_id),
          'policy_list'
        );

        if (find(policy, item => item.action === PolicyType.ARCHIVING)) {
          result = false;
        }
      } else {
        const policy = get(
          find(this.slaList, item => item.uuid === sla_id),
          'policy_list'
        );

        if (find(policy, item => item.action === PolicyType.ARCHIVING)) {
          result = false;
        }
      }

      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            this.i18n.get('protection_db_two_label'),
            this.i18n.get('common_copy_a_copy_label'),
            this.i18n.get('common_archive_label')
          ])
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      includes(
        [DataMap.Resource_Type.dbTwoDatabase.value],
        this.subResourceType
      ) &&
      this.resourceData?.extendInfo?.clusterType ===
        DataMap.dbTwoType.rhel.value
    ) {
      let result = true;
      if (this.selectedSlaView) {
        const policy = get(
          find(this.slaDatas, item => item.uuid === sla_id),
          'policy_list'
        );

        if (
          find(
            policy,
            item =>
              item.action === PolicyAction.INCREMENT ||
              item.action === PolicyAction.DIFFERENCE
          )
        ) {
          result = false;
        }
      } else {
        const policy = get(
          find(this.slaList, item => item.uuid === sla_id),
          'policy_list'
        );

        if (
          find(
            policy,
            item =>
              item.action === PolicyAction.INCREMENT ||
              item.action === PolicyAction.DIFFERENCE
          )
        ) {
          result = false;
        }
      }
      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_rhel_policy_label')
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      includes(
        [DataMap.Resource_Type.generalDatabase.value],
        this.resourceData.subType
      )
    ) {
      const slaArr = this.selectedSlaView ? this.slaDatas : this.slaList;
      const selectedSla = get(
        find(slaArr, item => item.uuid === sla_id),
        'policy_list'
      );
      const selectedBackupTypes = uniq(_map(selectedSla, 'action'));
      const support =
        size(
          intersection(this.generalDbSupportBackupType, selectedBackupTypes)
        ) === size(selectedBackupTypes);

      const unSupport = difference(
        selectedBackupTypes,
        this.generalDbSupportBackupType
      );

      if (!support) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            this.generalDbType,
            '',
            _map(unSupport, item => this.i18n.get(SLA_BACKUP_NAME[item])).join(
              this.i18n.isEn ? ',' : '、'
            )
          ])
        );
        this.valid$.next(false);
      }

      return support;
    } else if (
      includes(
        [DataMap.Resource_Type.MongodbSingleInstance.value],
        this.resourceData.subType
      )
    ) {
      if (
        this.resourceData.extendInfo.singleType ===
        DataMap.mongoDBSingleInstanceType.copySet.value
      ) {
        // 只有单节点副本集的单实例才支持日志备份SLA
        this.valid$.next(true);
        return;
      }
      let result = true;
      if (this.selectedSlaView) {
        const policy = get(
          find(this.slaDatas, item => item.uuid === sla_id),
          'policy_list'
        );

        if (find(policy, item => item.action === PolicyAction.LOG)) {
          result = false;
        }
      } else {
        const policy = get(
          find(this.slaList, item => item.uuid === sla_id),
          'policy_list'
        );

        if (find(policy, item => item.action === PolicyAction.LOG)) {
          result = false;
        }
      }
      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_mongodb_sla_tips_label')
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      includes(
        [DataMap.Resource_Type.gaussdbTSingle.value],
        this.resourceData.subType
      )
    ) {
      const slaArr = this.selectedSlaView ? this.slaDatas : this.slaList;
      const selectedSla = get(
        find(slaArr, item => item.uuid === sla_id),
        'policy_list'
      );
      let result = true;
      let invalidPolicy = '';

      if (find(selectedSla, item => item.action === PolicyAction.INCREMENT)) {
        invalidPolicy = this.i18n.get('common_incremental_backup_label');
        result = false;
      } else if (
        find(selectedSla, item => item.action === PolicyAction.DIFFERENCE)
      ) {
        invalidPolicy = this.i18n.get('common_diff_backup_label');
        result = false;
      }

      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            this.i18n.get('common_gaussdbt_single_label'),
            '',
            invalidPolicy
          ])
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      this.resourceData.subType ===
      DataMap.Resource_Type.tdsqlDistributedInstance.value
    ) {
      const slaArr = this.selectedSlaView ? this.slaDatas : this.slaList;
      const selectedSla = get(
        find(slaArr, item => item.uuid === sla_id),
        'policy_list'
      );
      let result = true;
      let invalidPolicy = '';
      if (find(selectedSla, item => item.action === PolicyAction.INCREMENT)) {
        invalidPolicy = this.i18n.get('common_permanent_backup_label');
        result = false;
      }
      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            this.i18n.get('protection_tdsql_distributed_instance_label'),
            '',
            invalidPolicy
          ])
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      includes(
        [DataMap.Resource_Type.OceanBaseTenant.value],
        this.resourceData.subType
      )
    ) {
      const slaArr = this.selectedSlaView ? this.slaDatas : this.slaList;
      const selectedSla = get(
        find(slaArr, item => item.uuid === sla_id),
        'policy_list'
      );
      let result = true;
      let invalidPolicy = '';
      const logBackupPolicy = this.i18n.get('common_log_backup_label');
      const incrementBackupPolicy = this.i18n.get(
        'common_incremental_backup_label'
      );
      // 租户集选择sla过滤包含增量和日志
      const hasAction = opt => some(selectedSla, { action: opt });
      if (hasAction(PolicyAction.LOG) && hasAction(PolicyAction.INCREMENT)) {
        invalidPolicy =
          this.i18n.language === 'en-us'
            ? `${logBackupPolicy} ${this.i18n.get(
                'common_and_label'
              )} ${incrementBackupPolicy}`
            : `${logBackupPolicy}${this.i18n.get(
                'common_and_label'
              )}${incrementBackupPolicy}`;
        result = false;
      } else if (hasAction(PolicyAction.INCREMENT)) {
        invalidPolicy = incrementBackupPolicy;
        result = false;
      } else if (hasAction(PolicyAction.LOG)) {
        invalidPolicy = logBackupPolicy;
        result = false;
      }

      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            this.i18n.get('protection_oceanbase_label'),
            this.i18n.get('protection_single_tenant_set_label'),
            invalidPolicy
          ])
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      includes(
        [
          DataMap.Resource_Type.tidbDatabase.value,
          DataMap.Resource_Type.tidbTable.value
        ],
        this.resourceData.subType
      )
    ) {
      const slaArr = this.selectedSlaView ? this.slaDatas : this.slaList;
      const selectedSla = get(
        find(slaArr, item => item.uuid === sla_id),
        'policy_list'
      );
      let result = true;
      let invalidPolicy = '';

      if (
        includes(
          [
            DataMap.Resource_Type.tidbDatabase.value,
            DataMap.Resource_Type.tidbTable.value
          ],
          this.resourceData.subType
        )
      ) {
        if (find(selectedSla, item => item.action === PolicyAction.LOG)) {
          invalidPolicy = this.i18n.get('common_log_backup_label');
          result = false;
        }
      }

      let tip = '';
      if (this.resourceData.subType === DataMap.Resource_Type.tidbTable.value) {
        tip = this.i18n.get('protection_tidb_table_label');
      } else {
        tip = this.i18n.get('protection_tidb_database_label');
      }

      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            tip,
            '',
            invalidPolicy
          ])
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      this.resourceData.subType ===
        DataMap.Resource_Type.MySQLClusterInstance.value &&
      this.resourceData.extendInfo?.clusterType ===
        DataMap.Mysql_Cluster_Type.eapp.value
    ) {
      const slaArr = this.selectedSlaView ? this.slaDatas : this.slaList;
      const selectedSla = get(
        find(slaArr, item => item.uuid === sla_id),
        'policy_list'
      );
      let result = true;
      if (find(selectedSla, item => item.action === PolicyAction.LOG)) {
        result = false;
      }
      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            '',
            '',
            this.i18n.get('common_log_backup_label')
          ]),
          {
            lvMessageKey: 'msg_mysql_eapp_laog_error',
            lvShowCloseButton: true
          }
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      [
        DataMap.Resource_Type.informixClusterInstance.value,
        DataMap.Resource_Type.informixInstance.value
      ].includes(this.resourceData.subType) &&
      this.resourceData.databaseType ===
        DataMap.informixDatabaseType.gbase.value
    ) {
      // GBase 8s数据库不支持日志备份
      const slaArr = this.selectedSlaView ? this.slaDatas : this.slaList;
      const selectedSla = get(
        find(slaArr, item => item.uuid === sla_id),
        'policy_list'
      );
      let result = true;
      let invalidPolicy = '';
      if (find(selectedSla, item => item.action === PolicyAction.LOG)) {
        result = false;
        invalidPolicy = this.i18n.get('common_log_backup_label');
      }
      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            '',
            DataMap.informixDatabaseType.gbase.label,
            invalidPolicy
          ])
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      this.resourceData.subType ===
        DataMap.Resource_Type.OpenGauss_instance.value &&
      !this.resourceData.extendInfo?.clusterVersion.includes('PanWeiDB')
    ) {
      // 不是panweiDB就不支持日志备份
      const slaArr = this.selectedSlaView ? this.slaDatas : this.slaList;
      const selectedSla = get(
        find(slaArr, item => item.uuid === sla_id),
        'policy_list'
      );
      let result = true;
      let invalidPolicy = '';
      if (find(selectedSla, item => item.action === PolicyAction.LOG)) {
        result = false;
        invalidPolicy = this.i18n.get('common_log_backup_label');
      }
      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            '',
            this.i18n.get('protection_opengauss_sla_instance_tips_valid_label'),
            this.i18n.get('common_log_backup_label')
          ])
        );
        this.valid$.next(false);
      }
      return result;
    } else if (
      this.resourceData.subType ===
      DataMap.Resource_Type.OpenGauss_database.value
    ) {
      const slaArr = this.selectedSlaView ? this.slaDatas : this.slaList;
      const selectedSla = get(
        find(slaArr, item => item.uuid === sla_id),
        'policy_list'
      );
      let result = true;
      let invalidPolicy = '';
      if (find(selectedSla, item => item.action === PolicyAction.INCREMENT)) {
        result = false;
        invalidPolicy = this.i18n.get('common_incremental_backup_label');
      }
      if (find(selectedSla, item => item.action === PolicyAction.LOG)) {
        result = false;
        if (!invalidPolicy) {
          invalidPolicy = this.i18n.get('common_log_backup_label');
        } else {
          invalidPolicy = `${invalidPolicy}、${this.i18n.get(
            'common_log_backup_label'
          )}`;
        }
      }
      if (!result) {
        this.messageService.error(
          this.i18n.get('protection_unsupport_backup_policy_label', [
            DataMap.Resource_Type.OpenGauss.label,
            this.i18n.get('common_database_label'),
            invalidPolicy
          ])
        );
        this.valid$.next(false);
      }
      return result;
    } else {
      return true;
    }
  }

  private getGeneralDbSupportBackupType() {
    const resource = isArray(this.resourceData)
      ? first(this.resourceData)
      : this.resourceData;
    this.generalDbType = get(resource, 'databaseTypeDisplay');
    const config = JSON.parse(get(resource, 'extendInfo.scriptConf') || '{}');
    const version = get(resource, 'version');
    const supportBackupType = config?.backup?.support;

    each(supportBackupType, item => {
      if (
        (!item.minVersion && !item.maxVersion) ||
        (item.minVersion && !item.maxVersion && version >= item.minVersion) ||
        (!item.minVersion && item.maxVersion && version <= item.maxVersion) ||
        (item.minVersion &&
          item.maxVersion &&
          version <= item.maxVersion &&
          version >= item.minVersion)
      ) {
        this.generalDbSupportBackupType.push(item.backupType);
      }
    });

    this.generalDbSupportBackupType.push(
      PolicyType.ARCHIVING,
      PolicyType.REPLICATION
    );
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.slas, item.uuid)
    );
  }
}
