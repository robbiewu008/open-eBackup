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
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import {
  ApplicationType,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  PolicyAction,
  PolicyType,
  ResourceSetApiService,
  ResourceSetType,
  SlaApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  find,
  forEach,
  includes,
  isEmpty,
  map as _map,
  set,
  toString,
  trim,
  union,
  uniq
} from 'lodash';
import { Subscription } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-sla-template',
  templateUrl: './sla-template.component.html',
  styleUrls: ['./sla-template.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class SlaTemplateComponent implements OnInit {
  @Input() subName;
  @Input() allSelectionMap;
  @Input() data;
  @Input() isDetail;
  @Output() allSelectChange = new EventEmitter<any>();

  policyType = PolicyType;
  name;
  sortFilter;
  userList = [];
  slaData = [];
  actions = [];
  slaType = [];
  applications = [];
  slaStatus = [];
  selection = [];
  durationTimeList = [];
  isAllSelect = false; // 用来标记是否全选
  buttonLabel = this.i18n.get('system_resourceset_all_select_label');
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
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
          ApplicationType.ActiveDirectory,
          ApplicationType.ApsaraStack,
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
          ApplicationType.GaussDBT,
          ApplicationType.Redis,
          ApplicationType.KingBase,
          ApplicationType.FusionCompute,
          ApplicationType.NASShare,
          ApplicationType.Replica,
          ApplicationType.LocalFileSystem,
          ApplicationType.HCSCloudHost,
          ApplicationType.HyperV,
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
          ApplicationType.SapHana,
          ApplicationType.Saponoracle,
          ApplicationType.TiDB,
          ApplicationType.Ndmp,
          ApplicationType.Volume,
          ApplicationType.CommonShare,
          ApplicationType.ObjectStorage,
          ApplicationType.Exchange
        ],
        item.value
      );
    })
    .filter(item => {
      if (this.appUtilsService.isDistributed) {
        return !includes(
          [
            ApplicationType.NASFileSystem,
            ApplicationType.Replica,
            ApplicationType.CommonShare
          ],
          item.value
        );
      }
      if (this.appUtilsService.isDecouple) {
        return !includes(
          [ApplicationType.NASFileSystem, ApplicationType.CommonShare],
          item.value
        );
      }
      return item.value;
    });

  dataFetch$: Subscription = new Subscription();

  @ViewChild('namePopover', { static: false }) namePopover;

  constructor(
    public cookieService: CookieService,
    public slaApiService: SlaApiService,
    public dataMapService: DataMapService,
    public appUtilsService: AppUtilsService,
    public drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private resourceSetService: ResourceSetApiService
  ) {}

  ngOnInit(): void {
    this.getSlaList();
    if (!!this.allSelectionMap[ResourceSetType.SLA]?.isAllSelected) {
      this.isAllSelect = true;
    }
    if (this.appUtilsService.isDistributed) {
      this.actionsFilterMap = this.actionsFilterMap.filter(
        item => item.value !== PolicyType.REPLICATION
      );
    }
  }

  getSlaList(refreshData?) {
    const params = {
      pageNo: this.pageNo,
      pageSize: this.pageSize
    };

    if (!this.isDetail) {
      assign(params, {
        isGlobal: false
      });
    }

    if (this.isDetail) {
      assign(params, {
        resourceSetId: this.data[0].uuid
      });
    }

    if (this.name) {
      assign(params, { name: trim(this.name) });
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
    }

    if (!!this.slaStatus.length) {
      assign(params, {
        enabled: this.slaStatus
      });
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
        this.cdr.detectChanges();
        if (!isEmpty(this.allSelectionMap[ResourceSetType.SLA]?.data)) {
          // 重新进入时回显选中的数据
          this.selection = this.allSelectionMap[ResourceSetType.SLA]?.data;
        }

        if (!!this.allSelectionMap[ResourceSetType.SLA]?.isAllSelected) {
          this.allSelect(false);
        }
        if (
          this.data &&
          !this.isDetail &&
          !(ResourceSetType.SLA in this.allSelectionMap)
        ) {
          this.getSelectedData();
        }
      });
  }

  getSelectedData() {
    const params: any = {
      resourceSetId: this.data[0].uuid,
      scopeModule: ResourceSetType.SLA,
      type: ResourceSetType.SLA
    };

    this.resourceSetService.queryResourceObjectIdList(params).subscribe(res => {
      set(this.allSelectionMap, ResourceSetType.SLA, {
        data: _map(res, item => {
          return { uuid: item };
        })
      });
      this.selection = this.allSelectionMap[ResourceSetType.SLA]?.data;
      this.cdr.detectChanges();
    });
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
            ApplicationType.Vmware,
            ApplicationType.HCSCloudHost,
            ApplicationType.FusionCompute,
            ApplicationType.TDSQL
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
            ApplicationType.Vmware,
            ApplicationType.HCSCloudHost,
            ApplicationType.FusionCompute,
            ApplicationType.TDSQL
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

  allSelect(turnPage?) {
    const isAllSelected = !!turnPage ? !this.isAllSelect : this.isAllSelect;
    set(this.allSelectionMap, ResourceSetType.SLA, { isAllSelected });
    this.selection = isAllSelected ? [...this.slaData] : [];
    each(this.slaData, item => {
      item.disabled = isAllSelected;
    });
    this.isAllSelect = isAllSelected;
    this.buttonLabel = this.i18n.get(
      isAllSelected
        ? 'system_resourceset_cancel_all_select_label'
        : 'system_resourceset_all_select_label'
    );
    this.allSelectChange.emit();
    this.cdr.detectChanges();
  }

  selectionChange() {
    set(this.allSelectionMap, ResourceSetType.SLA, {
      data: this.selection
    });
    this.allSelectChange.emit();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageNo = page.pageIndex;
    this.getSlaList();
  }

  searchByName(value) {
    if (this.namePopover) {
      this.namePopover.hide();
    }
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
}
