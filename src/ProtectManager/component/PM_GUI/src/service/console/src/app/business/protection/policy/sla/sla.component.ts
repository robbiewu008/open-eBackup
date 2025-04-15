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
import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnDestroy,
  OnInit
} from '@angular/core';
import { MenuItem } from '@iux/live';
import {
  ApplicationType,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  E6000SupportApplication,
  getPermissionMenuItem,
  GlobalService,
  GROUP_COMMON,
  hasSlaPermission,
  I18NService,
  OperateItems,
  PolicyAction,
  PolicyType,
  RoleOperationAuth,
  RoleOperationMap,
  SupportLicense,
  WarningMessageService
} from 'app/shared';
import { SlaApiService, UsersApiService } from 'app/shared/api/services';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { SlaService } from 'app/shared/services/sla.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  find,
  forEach,
  get,
  includes,
  isEmpty,
  map as _map,
  mapValues,
  reject,
  set,
  toString,
  trim,
  union,
  uniq
} from 'lodash';
import { Subject, Subscription } from 'rxjs';
import { map, takeUntil } from 'rxjs/operators';

@Component({
  selector: 'sla',
  templateUrl: './sla.component.html',
  styleUrls: ['./sla.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class SlaComponent implements OnInit, OnDestroy {
  policyType = PolicyType;
  name;
  userName;
  sortFilter;
  userList = [];
  slaData = [];
  actions = [];
  slaType = [];
  applications = [];
  slaStatus = [];
  selection = [];
  durationTimeList = [];
  disableDelete = true;
  disableDeleteTip = '';
  scoptMap = {
    year: {
      label: 'protection_choose_every_year_label',
      config: 'Year_Time_Range'
    },
    month: {
      label: 'protection_choose_every_month_label',
      config: 'Month_Time_Range'
    },
    week: {
      label: 'protection_choose_every_week_label',
      config: 'Days_Of_Week'
    }
  };
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isOtherDeployType = includes(
    [
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  pageNo = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
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
      label: this.i18n.get('common_permanent_backup_label'),
      value: PolicyAction.PERMANENT,
      key: PolicyAction.PERMANENT
    },
    {
      label: this.i18n.get('common_diff_backup_label'),
      value: PolicyAction.DIFFERENCE,
      key: PolicyAction.DIFFERENCE
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
    },
    {
      label: this.i18n.get('common_anti_detection_snapshot_label'),
      value: PolicyAction.SNAPSHOT,
      key: PolicyAction.SNAPSHOT
    }
  ].filter(item =>
    this.isHcsUser
      ? !includes(
          [PolicyType.ARCHIVING, PolicyType.REPLICATION, PolicyAction.SNAPSHOT],
          item.key
        )
      : this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
      ? item.key === PolicyAction.SNAPSHOT
      : this.cookieService.isCloudBackup
      ? item.key === PolicyAction.INCREMENT
      : item
  );
  applicationFilterMap = this.dataMapService
    .toArray('Application_Type', [ApplicationType.LocalFileSystem])
    .filter(item => {
      return includes(
        [
          ApplicationType.Common,
          ApplicationType.AntDB,
          ApplicationType.DB2,
          ApplicationType.Fileset,
          ApplicationType.Oracle,
          ApplicationType.MySQL,
          ApplicationType.PostgreSQL,
          ApplicationType.SQLServer,
          ApplicationType.GaussDBDWS,
          ApplicationType.Vmware,
          ApplicationType.CNware,
          ApplicationType.HDFS,
          ApplicationType.Hive,
          ApplicationType.HBase,
          ApplicationType.NASFileSystem,
          ApplicationType.Ndmp,
          ApplicationType.GaussDBT,
          ApplicationType.Redis,
          ApplicationType.KingBase,
          ApplicationType.FusionCompute,
          ApplicationType.FusionOne,
          ApplicationType.NASShare,
          ApplicationType.Replica,
          ApplicationType.LocalFileSystem,
          ApplicationType.HCSCloudHost,
          ApplicationType.Dameng,
          ApplicationType.OpenGauss,
          ApplicationType.Elasticsearch,
          ApplicationType.ClickHouse,
          ApplicationType.KubernetesStatefulSet,
          ApplicationType.KubernetesDatasetCommon,
          ApplicationType.GoldenDB,
          ApplicationType.GeneralDatabase,
          ApplicationType.OpenStack,
          ApplicationType.GaussDBForOpenGauss,
          ApplicationType.LightCloudGaussDB,
          ApplicationType.MongoDB,
          ApplicationType.Informix,
          ApplicationType.TDSQL,
          ApplicationType.OceanBase,
          ApplicationType.TiDB,
          ApplicationType.Volume,
          ApplicationType.CommonShare,
          ApplicationType.ObjectStorage,
          ApplicationType.Exchange,
          ApplicationType.ApsaraStack,
          ApplicationType.HyperV,
          ApplicationType.ActiveDirectory,
          ApplicationType.SapHana,
          ApplicationType.Saponoracle,
          ApplicationType.Nutanix
        ],
        item.value
      );
    })
    .filter(item => {
      if (this.appUtilsService.isDistributed) {
        E6000SupportApplication.push(ApplicationType.Common);
        return includes(E6000SupportApplication, item.value);
      }
      if (this.appUtilsService.isDecouple) {
        return !includes(
          [ApplicationType.NASFileSystem, ApplicationType.CommonShare],
          item.value
        );
      }
      return item.value;
    });

  slaStatusMap = this.dataMapService.toArray('slaStatus');
  jump$: Subscription = new Subscription();
  destroy$ = new Subject();

  groupCommon = GROUP_COMMON;
  activeItem;

  roleOperationMap = RoleOperationMap;
  roleOperationAuth = RoleOperationAuth;

  registerTipShow = false;

  constructor(
    public i18n: I18NService,
    public slaService: SlaService,
    public slaApiService: SlaApiService,
    public globalService: GlobalService,
    public dataMapService: DataMapService,
    public drawModalService: DrawModalService,
    public usersApiService: UsersApiService,
    public warningMessageService: WarningMessageService,
    public cookieService: CookieService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    public appUtilsService: AppUtilsService
  ) {}

  everyLabel = this.i18n.get('common_every_label');
  persistentLabel = this.i18n.get('common_persistent_label');
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

  ngOnInit() {
    if (this.isHyperdetect) {
      this.applicationFilterMap = this.dataMapService
        .toArray('Application_Type')
        .filter(item => {
          if (SupportLicense.isBoth) {
            return includes(
              [ApplicationType.LocalFileSystem, ApplicationType.LocalLun],
              item.value
            );
          }
          if (!SupportLicense.isBoth && SupportLicense.isFile) {
            return includes([ApplicationType.LocalFileSystem], item.value);
          }

          if (!SupportLicense.isBoth && SupportLicense.isSan) {
            return includes([ApplicationType.LocalLun], item.value);
          }
        });
    }
    this.getSlaList();
    this.getTaskJump();
    this.virtualScroll.getScrollParam(230);
    this.showRegisterTip();
    this.getUserGuideState();
  }

  getUserGuideState() {
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(res => {
        if (res.showTips) {
          this.showRegisterTip();
        }
      });
  }

  showRegisterTip() {
    if (
      USER_GUIDE_CACHE_DATA.active &&
      USER_GUIDE_CACHE_DATA.showTips &&
      includes(this.roleOperationAuth, this.roleOperationMap.sla)
    ) {
      setTimeout(() => {
        this.registerTipShow = true;
        USER_GUIDE_CACHE_DATA.showTips = false;
        this.cdr.detectChanges();
      });
    }
  }

  lvPopoverBeforeClose = () => {
    this.registerTipShow = false;
    this.cdr.detectChanges();
  };

  onChange() {
    this.ngOnInit();
  }

  ngOnDestroy() {
    this.jump$.unsubscribe();
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  getTaskJump() {
    this.jump$ = this.globalService.getState('jobToSla').subscribe(res => {
      this.getSlaList();
    });
  }

  getSlaList(refreshData?) {
    const params = { pageNo: this.pageNo, pageSize: this.pageSize };
    if (!!this.appUtilsService.getCacheValue('jobToSla', false)) {
      this.name = this.appUtilsService.getCacheValue('jobToSla');
    }
    if (this.name) {
      assign(params, { name: trim(this.name) });
    }

    if (this.userName) {
      assign(params, {
        userName: this.userName
      });
    }

    // 非分布式环境this.slaStatus都是空数组
    if (!isEmpty(this.slaStatus) && this.slaStatus.length === 1) {
      assign(params, {
        isEnabled: this.slaStatus[0] // true-已激活 false-已禁用 全选则不传isEnabled
      });
    }

    if (!!this.applications.length) {
      assign(params, {
        applications: this.applications
      });
    } else {
      if (this.cookieService.isCloudBackup) {
        assign(params, {
          applications: [DataMap.Application_Type.LocalFileSystem.value]
        });
      }
      if (this.isHyperdetect) {
        if (SupportLicense.isBoth) {
          assign(params, {
            applications: [
              DataMap.Application_Type.LocalFileSystem.value,
              DataMap.Application_Type.LocalLun.value
            ]
          });
        }

        if (!SupportLicense.isBoth && SupportLicense.isFile) {
          assign(params, {
            applications: [DataMap.Application_Type.LocalFileSystem.value]
          });
        }

        if (!SupportLicense.isBoth && SupportLicense.isSan) {
          assign(params, {
            applications: [DataMap.Application_Type.LocalLun.value]
          });
        }
      }
    }

    if (this.sortFilter?.key) {
      assign(params, {
        orderType: this.sortFilter.direction,
        orderBy: this.sortFilter.key
      });
    }

    if (!!this.actions.length) {
      assign(params, { actions: this.actions });
    }

    if (!!this.slaType.length) {
      assign(params, { types: this.slaType });
    }

    this.slaApiService
      .pageQueryUsingGET(params)
      .pipe(
        map(result => {
          return this.convertAction(result);
        })
      )
      .subscribe((res: any) => {
        this.slaData = res.items;
        this.total = res.total;
        if (
          refreshData &&
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'slaDetailModalKey'
          )
        ) {
          this.refreshDetail(refreshData, res.items);
        }
        this.cdr.detectChanges();
      });
  }

  // 刷新资源详情
  refreshDetail(target, tableData) {
    if (find(tableData, { uuid: target.uuid })) {
      this.getSlaDetail(find(tableData, { uuid: target.uuid }));
    } else {
      this.drawModalService.destroyModal('slaDetailModalKey');
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
      if (permanent) {
        backupModes.push(this.i18n.get('common_permanent_backup_label'));
      }
      if (
        increment &&
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
            ApplicationType.TDSQL,
            ApplicationType.ApsaraStack,
            ApplicationType.HyperV,
            ApplicationType.CNware,
            ApplicationType.Nutanix
          ],
          item.application
        )
      ) {
        backupModes.push(this.i18n.get('common_permanent_backup_label'));
      }
      if (
        increment &&
        !includes(
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
            ApplicationType.TDSQL,
            ApplicationType.ApsaraStack,
            ApplicationType.HyperV,
            ApplicationType.CNware,
            ApplicationType.Nutanix
          ],
          item.application
        )
      ) {
        backupModes.push(this.i18n.get('common_incremental_backup_label'));
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

  getSlaDetail(data, isViewResource?) {
    this.activeItem = data;
    this.slaService.getDetail(
      data,
      this.getOptItems(data),
      isViewResource,
      () => {
        this.activeItem = {};
        this.cdr.detectChanges();
      }
    );
  }

  createSla() {
    if (USER_GUIDE_CACHE_DATA.active && USER_GUIDE_CACHE_DATA.slaType) {
      this.slaService.create(() => this.getSlaList(), {
        application: USER_GUIDE_CACHE_DATA.slaType,
        isOnlyGuide: true
      });
    } else {
      this.slaService.create(() => this.getSlaList());
    }
  }

  optCallBack: (data) => Array<MenuItem> = sla => {
    return this.getOptItems(sla);
  };

  getOptItems(sla) {
    const menus = [
      {
        id: 'clone',
        label: this.i18n.get('common_clone_label'),
        permission: OperateItems.CloneSLA,
        disabled: !hasSlaPermission(sla),
        onClick: (d: any) => this.slaService.clone(sla, () => this.getSlaList())
      },
      {
        id: 'modify',
        disabled: sla.is_global || !hasSlaPermission(sla),
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifySLA,
        onClick: (d: any) =>
          this.slaService.modify(sla, () => this.getSlaList(sla))
      },
      {
        id: 'delete',
        disabled: sla.is_global || !hasSlaPermission(sla),
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteSLA,
        onClick: (d: any) =>
          this.slaService.delete(sla, () => {
            this.selection = reject(
              this.selection,
              item => item.uuid === sla.uuid
            );
            this.disableDelete = !this.selection.length;
            this.getSlaList(sla);
          })
      },
      {
        id: 'activate',
        disabled: sla.enabled,
        hidden: !this.appUtilsService.isDistributed,
        label: this.i18n.get('common_active_label'),
        permission: OperateItems.DeleteSLA,
        onClick: (d: any) =>
          this.slaService.activate(sla, () => this.getSlaList(sla))
      },
      {
        id: 'disable',
        disabled: !sla.enabled,
        hidden: !this.appUtilsService.isDistributed,
        label: this.i18n.get('common_disable_label'),
        permission: OperateItems.DeleteSLA,
        onClick: (d: any) =>
          this.slaService.disable(sla, () => this.getSlaList(sla))
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageNo = page.pageIndex;
    this.getSlaList();
  }

  searchByName(name) {
    this.name = name;
    this.getSlaList();
  }

  searchByUserName(userName) {
    this.userName = userName;
    this.getSlaList();
  }

  actionFilterChange = event => {
    this.actions = includes(event.value, PolicyAction.LOG)
      ? uniq(union(event.value, [PolicyAction.INCREMENT]))
      : event.value;

    this.getSlaList();
  };

  appliationFilterChange = event => {
    this.applications = event.value;
    this.getSlaList();
  };
  slaStatusFilterChange = event => {
    this.slaStatus = event.value;
    this.getSlaList();
  };

  sortData(filter) {
    this.sortFilter = filter;
    this.getSlaList();
  }

  selectionChange() {
    if (
      find(
        this.selection,
        item =>
          includes(['Gold', 'Silver', 'Bronze'], item.name) &&
          item.application === ApplicationType.Common
      )
    ) {
      this.disableDelete = true;
      this.disableDeleteTip = this.i18n.get(
        'protection_delete_sla_preset_label'
      );
    } else if (find(this.selection, item => !!item.resource_count)) {
      this.disableDelete = true;
      this.disableDeleteTip = this.i18n.get(
        'protection_delete_sla_resource_label'
      );
    } else if (this.selection?.length > 100) {
      this.disableDelete = true;
      this.disableDeleteTip = this.i18n.get('protection_delete_sla_max_label');
    } else {
      this.disableDelete = !this.selection?.length;
      this.disableDeleteTip = '';
    }
  }

  batchDelete() {
    this.slaService.batchDelete(this.selection, () => {
      this.getSlaList();
      this.selection = [];
      this.disableDelete = true;
    });
  }

  openPopover(item) {
    const policyList = get(item, 'policy_list', []);
    const dataList = [];

    each(policyList, policy => {
      let policyName = '';
      if (item.application === ApplicationType.Common) {
        policyName = this.i18n.get(
          get(find(this.actionsFilterMap, { value: policy.action }), 'label')
        );
      } else {
        if (
          policy.action === PolicyAction.INCREMENT &&
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
              ApplicationType.TDSQL,
              ApplicationType.ApsaraStack,
              ApplicationType.HyperV,
              ApplicationType.CNware,
              ApplicationType.Nutanix
            ],
            item.application
          )
        ) {
          policyName = `${policy.name} (${this.i18n.get(
            get(
              find(this.actionsFilterMap, { value: PolicyAction.PERMANENT }),
              'label'
            )
          )})`;
        } else {
          policyName = `${policy.name} (${this.i18n.get(
            get(find(this.actionsFilterMap, { value: policy.action }), 'label')
          )})`;
        }
      }

      if (policy.type === PolicyType.BACKUP) {
        // 备份策略保留时间
        const durationUnit = this.i18n.get(
          this.dataMapService.getLabel(
            'Interval_Unit',
            get(policy, 'retention.duration_unit')
          )
        );
        const durationTime = `${get(policy, 'retention.retention_duration') ||
          ''}${this.i18n.isEn ? ' ' : ''}${durationUnit}`;

        const data = {
          name: policyName,
          type: policy.type,
          action: policy.action,
          durationTime: durationTime
        };

        dataList.push(data);
      } else {
        // 复制归档保留时间
        const data = {
          name: policyName,
          type: policy.type,
          action: policy.action
        };

        if (
          get(policy, 'ext_parameters.replication_target_type') ===
            DataMap.slaReplicationRule.specify.value ||
          get(policy, 'ext_parameters.archive_target_type') ===
            DataMap.slaReplicationRule.specify.value
        ) {
          // 指定副本
          const specifiedScope = get(
            policy,
            'ext_parameters.specified_scope',
            []
          );
          const scopeArr = _map(specifiedScope, item => {
            const name = `${this.i18n.get(
              get(this.scoptMap, `${item.copy_type}.label`)
            )} ${this.dataMapService.getLabel(
              get(this.scoptMap, `${item.copy_type}.config`),
              item?.generate_time_range
            )} ${this.i18n.get(
              policy.type === PolicyType.ARCHIVING
                ? 'protection_copy_archiving_label'
                : 'protection_copy_rep_label'
            )}`;
            return {
              name: name,
              durationTime: `${get(item, 'retention_duration')}${
                this.i18n.isEn ? ' ' : ''
              }${this.i18n.get(
                this.dataMapService.getLabel(
                  'Interval_Unit',
                  get(item, 'retention_unit')
                )
              )}`
            };
          });

          set(data, 'durationTime', scopeArr);
        } else {
          // 所有副本
          const name =
            policy.action === PolicyType.ARCHIVING
              ? this.dataMapService.getLabel(
                  'Archive_Scope',
                  get(policy, 'ext_parameters.archiving_scope')
                )
              : this.i18n.get('protection_copy_detail_label', [
                  get(policy, 'ext_parameters.start_replicate_time')
                ]);
          const scope = {
            name: name,
            durationTime: `${get(policy, 'retention.retention_duration') ||
              ''}${this.i18n.isEn ? ' ' : ''}${this.i18n.get(
              this.dataMapService.getLabel(
                'Interval_Unit',
                get(policy, 'retention.duration_unit')
              )
            )}`
          };

          set(data, 'durationTime', [scope]);
        }

        dataList.push(data);
      }
    });

    const policyWeight = {
      [PolicyAction.FULL]: 1,
      [PolicyAction.INCREMENT]: 2,
      [PolicyAction.DIFFERENCE]: 3,
      [PolicyAction.PERMANENT]: 4,
      [PolicyAction.SNAPSHOT]: 5,
      [PolicyAction.LOG]: 6,
      [PolicyType.ARCHIVING]: 7,
      [PolicyType.REPLICATION]: 8
    };

    this.durationTimeList = dataList.sort((a, b) => {
      return policyWeight[a.action] > policyWeight[b.action] ? 1 : -1;
    });
  }

  isActive(item): boolean {
    return item.uuid === this.activeItem?.uuid;
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.slas, item.uuid)
    );
  }
}
