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
import { Component, ElementRef, Input, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import { MenuItem } from '@iux/live';
import {
  CookieService,
  GlobalService,
  I18NService,
  JobColorConsts,
  RouterUrl
} from 'app/shared';
import {
  ApiMultiClustersService,
  JobAPIService
} from 'app/shared/api/services';
import { DataMap, JOB_ORIGIN_TYPE } from 'app/shared/consts';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, isNil, toString } from 'lodash';

@Component({
  selector: 'mission-overview',
  templateUrl: './mission-overview.component.html',
  styleUrls: ['./mission-overview.component.less']
})
export class MissionOverviewComponent implements OnInit {
  tabType = JOB_ORIGIN_TYPE;
  dataMap = DataMap;
  @Input() cardInfo: any = {};
  timeSelect: string;
  ClusterSelect: string;
  // 所有的数量
  backingUpNum: number = 0;
  duplicateNum: number = 0;
  archivingNum: number = 0;
  // 错误的数量
  backingUpAbnormalNum: number = 0;
  duplicateAbnormalNum: number = 0;
  archivingAbnormalNum: number = 0;
  // 是否配置，未配置置灰
  hasDuplicate: boolean = false;
  hasArchiving: boolean = false;

  jobColorConsts = JobColorConsts;
  jobItem;
  options: MenuItem[];
  jobsOption;
  jobsChart: any;
  jobStatus = DataMap.Job_status;
  currentTime;
  jobText = this.i18n.get('common_last_24hours_jobs_label');
  isMultiCluster = true;
  jobPeriod = 'lastDay';
  timeMap;
  abnormalStatus = [DataMap.Job_status.failed.value];
  selectNum = {
    total: {
      id: 'total',
      activeTab: JOB_ORIGIN_TYPE.EXE,
      status: []
    },
    pending: {
      id: 'pending',
      activeTab: JOB_ORIGIN_TYPE.EXE,
      status: [
        DataMap.Job_status.pending.value,
        DataMap.Job_status.redispatch.value,
        DataMap.Job_status.dispatching.value
      ]
    },
    running: {
      id: 'running',
      activeTab: JOB_ORIGIN_TYPE.EXE,
      status: [
        DataMap.Job_status.running.value,
        DataMap.Job_status.initialization.value,
        DataMap.Job_status.aborting.value
      ]
    },
    success: {
      id: 'success',
      activeTab: JOB_ORIGIN_TYPE.HISTORIC,
      status: [
        DataMap.Job_status.success.value,
        DataMap.Job_status.partial_success.value
      ]
    },
    fail: {
      id: 'fail',
      activeTab: JOB_ORIGIN_TYPE.HISTORIC,
      status: [
        DataMap.Job_status.failed.value,
        DataMap.Job_status.abnormal.value,
        DataMap.Job_status.abort_failed.value,
        DataMap.Job_status.dispatch_failed.value
      ]
    },
    aborted: {
      id: 'aborted',
      activeTab: JOB_ORIGIN_TYPE.HISTORIC,
      status: [
        DataMap.Job_status.aborted.value,
        DataMap.Job_status.cancelled.value
      ]
    }
  };
  taskJobType = {
    proEnvir: {
      taskType: DataMap.Job_type.backup_job.value,
      activeTab: JOB_ORIGIN_TYPE.EXE,
      isProEnvir: true
    },
    backup: {
      taskType: DataMap.Job_type.backup_job.value,
      status: null,
      activeTab: JOB_ORIGIN_TYPE.HISTORIC
    },
    duplicate: {
      taskType: DataMap.Job_type.copy_data_job.value,
      status: null,
      activeTab: JOB_ORIGIN_TYPE.HISTORIC
    },
    archive: {
      taskType: DataMap.Job_type.archive_job.value,
      status: null,
      activeTab: JOB_ORIGIN_TYPE.HISTORIC
    }
  };

  constructor(
    private i18n: I18NService,
    public el: ElementRef,
    public cookieService: CookieService,
    private jobAPIService: JobAPIService,
    private multiClustersServiceApi: ApiMultiClustersService,
    public router: Router,
    public appUtilsService: AppUtilsService,
    public globalService?: GlobalService
  ) {}

  ngOnInit() {
    this.getAllCusterShow();
    this.initData();
    this.refreshData();
  }

  refreshData() {
    this.cardInfo.loading = true;
    this.appUtilsService.getSystemTimeLong().subscribe({
      next: res => {
        this.currentTime = this.appUtilsService.toSystemTimeLong(new Date(res));
        Promise.all([
          this.initAsyncData(),
          this.changeDuplicateState(),
          this.getArchiveState(),
          this.getHistoryBackingUpAbnormalNum(),
          this.getHistoryDuplicateAbnormalNum(),
          this.getHistoryArchivingAbnormalNum(),
          this.getBackingUpNum(),
          this.getDuplicateNum(),
          this.getArchivingNum()
        ]).then(() => {
          this.cardInfo.loading = false;
        });
      },
      error: () => {
        this.cardInfo.loading = false;
      }
    });
  }

  getAllCusterShow() {
    const clusterObj = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isMultiCluster =
      !clusterObj ||
      (clusterObj && clusterObj['icon'] === 'aui-icon-all-cluster');
    return clusterObj;
  }

  initData() {
    this.jobItem = {
      [DataMap.Job_status.success.value]: 0,
      [DataMap.Job_status.running.value]: 0,
      [DataMap.Job_status.failed.value]: 0,
      [DataMap.Job_status.aborted.value]: 0,
      [DataMap.Job_status.initialization.value]: 0,
      [DataMap.Job_status.pending.value]: 0,
      other: 0,
      totalItem: 0
    };
    this.options = [
      {
        id: 'all',
        label: this.i18n.get('common_all_jobs_label'),
        onClick: data => {
          this.jobText = this.i18n.get('common_all_jobs_label');
          this.jobPeriod = 'all';
          this.initAsyncData();
        }
      },
      {
        id: '24hs',
        label: this.i18n.get('common_last_24hours_jobs_label'),
        onClick: data => {
          this.jobText = this.i18n.get('common_last_24hours_jobs_label');
          this.jobPeriod = 'lastDay';
          this.initAsyncData();
        }
      },
      {
        id: '7days',
        label: this.i18n.get('common_last_7days_jobs_label'),
        onClick: data => {
          this.jobText = this.i18n.get('common_last_7days_jobs_label');
          this.jobPeriod = 'lastWeek';
          this.initAsyncData();
        }
      },
      {
        id: '1month',
        label: this.i18n.get('common_last_1month_jobs_label'),
        onClick: data => {
          this.jobText = this.i18n.get('common_last_1month_jobs_label');
          this.jobPeriod = 'lastMonth';
          this.initAsyncData();
        }
      }
    ];
  }

  getMonthDays(): number {
    const year = new Date(this.currentTime).getFullYear();
    const month = new Date(this.currentTime).getMonth();
    return new Date(year, month, 0).getDate();
  }

  getTimeRange() {
    let startTime;
    let endTime;
    let jobPeriod;
    endTime = this.currentTime;
    switch (this.cardInfo.selectTime) {
      case 0:
        jobPeriod = 'all';
        break;
      case 3:
        startTime = this.currentTime - 24 * 3600 * 1000;
        jobPeriod = 'lastDay';
        break;
      case 4:
        startTime = this.currentTime - 7 * 24 * 3600 * 1000;
        jobPeriod = 'lastWeek';
        break;
      case 5:
        startTime = this.currentTime - this.getMonthDays() * 24 * 3600 * 1000;
        jobPeriod = 'lastMonth';
        break;
    }
    return {
      startTime,
      endTime,
      jobPeriod
    };
  }

  handleData(res) {
    this.jobItem[DataMap.Job_status.success.value] =
      res.success + (res.partial_success || 0);
    this.jobItem[DataMap.Job_status.running.value] =
      res.running + res.ready + (res.aborting || 0);
    this.jobItem[DataMap.Job_status.failed.value] =
      res.fail +
      (res.abnormal || 0) +
      (res.abort_failed || 0) +
      (res.dispatch_failed || 0);
    this.jobItem[DataMap.Job_status.aborted.value] =
      res.aborted + (res.cancelled || 0);
    this.jobItem[DataMap.Job_status.pending.value] =
      res.pending + (res.redispatch || 0) + (res.dispatching || 0);
    this.jobItem.totalItem = res.total;
  }

  initAsyncData() {
    return new Promise(resolve => {
      // clusterOption为任务卡片的集群节点，其中'所有集群'的value = -1，此时需要走multi接口
      if (
        isNil(this.cardInfo.selectcluster) ||
        this.cardInfo.selectcluster === -1
      ) {
        const multiParams = {
          akLoading: false,
          jobPeriod: this.getTimeRange().jobPeriod
        };
        this.multiClustersServiceApi
          .getMultiClusterJobs(multiParams)
          .subscribe(res => {
            this.handleData(res);
            resolve(true);
          });
      } else {
        // 如果切换到具体的集群，就需要根据选中的集群id和type做转发
        const clusterObj = this.getAllCusterShow();
        let { startTime, endTime } = this.getTimeRange();
        let params: any = {
          akLoading: false,
          clustersType: toString(this.cardInfo.selectcluster === 1 ? 1 : 2),
          clustersId: toString(this.cardInfo.selectcluster)
        };
        /*
         * clusterObj有三种情况
         * clusterObj = null || icon = all-cluster 此时为所有集群，任务总览需要根据选中的节点走转发
         * clusterObj.clusterType = 1 此时为本地集群，任务总览需要根据选中的节点走转发
         * clusterObj.clusterType = 2 此时为外部集群，这时任务总览的节点clusterOptions的id和type都为1，所以不能用这个值下发。
         * 应该使用interceptor的默认id和type下发
         * */
        if (
          clusterObj?.clusterType === DataMap.Cluster_Type.target.value ||
          this.appUtilsService.isDistributed ||
          this.appUtilsService.isDecouple
        ) {
          delete params.clustersType;
          delete params.clustersId;
        }
        if (startTime) {
          params.startTime = startTime;
          params.endTime = endTime;
        }
        this.jobAPIService.summaryUsingGET(params).subscribe(res => {
          this.handleData(res);
          resolve(true);
        });
      }
    });
  }

  changeDuplicateState() {
    return new Promise(resolve => {
      const params = { akLoading: false };
      if (this.isMultiCluster) {
        assign(params, { isAllCluster: true });
      }
      this.multiClustersServiceApi
        .getMultiClusterReplicationCapacitySummary(params)
        .subscribe(res => {
          this.hasDuplicate = res.some(item => {
            return item.type === 'replication' && item.totalCapacity !== 0;
          });
          resolve(true);
        });
    });
  }

  //获取归档统计信息
  getArchiveState() {
    return new Promise(resolve => {
      const params = { akLoading: false };
      if (this.isMultiCluster) {
        assign(params, { isAllCluster: true });
      }
      this.multiClustersServiceApi
        .getMultiClusterArchiveCapacitySummary(params)
        .subscribe(res => {
          this.changeArchiveState(res);
          resolve(true);
        });
    });
  }

  changeArchiveState(res) {
    this.hasArchiving = false;
    res.forEach(item => {
      if (
        item.type === 'tape' &&
        !(
          this.appUtilsService.isDistributed || this.appUtilsService.isDecouple
        ) &&
        item.usedCapacity !== 0
      ) {
        this.hasArchiving = true;
      }
      if (item.type === 'cloudStorage' && item.totalCapacity !== 0) {
        this.hasArchiving = true;
      }
    });
  }

  getBackingUpNum() {
    return new Promise(resolve => {
      let {
        startTime: fromStartTime,
        endTime: toStartTime
      } = this.getTimeRange();
      this.jobAPIService
        .queryJobsUsingGET({
          akLoading: false,
          pageSize: 1,
          types: ['BACKUP'],
          statusList: [
            this.jobStatus.running.value,
            this.jobStatus.initialization.value,
            this.jobStatus.pending.value,
            this.jobStatus.aborting.value,
            this.jobStatus.dispatching.value,
            this.jobStatus.redispatch.value
          ],
          fromStartTime,
          toStartTime
        })
        .subscribe(res => {
          this.backingUpNum = res.totalCount || 0;
          resolve(true);
        });
    });
  }
  getDuplicateNum() {
    return new Promise(resolve => {
      let {
        startTime: fromStartTime,
        endTime: toStartTime
      } = this.getTimeRange();
      this.jobAPIService
        .queryJobsUsingGET({
          akLoading: false,
          pageSize: 1,
          types: ['copy_replication'],
          statusList: [
            this.jobStatus.running.value,
            this.jobStatus.initialization.value,
            this.jobStatus.pending.value,
            this.jobStatus.aborting.value,
            this.jobStatus.dispatching.value,
            this.jobStatus.redispatch.value
          ],
          fromStartTime,
          toStartTime
        })
        .subscribe(res => {
          this.duplicateNum = res.totalCount || 0;
          resolve(true);
        });
    });
  }
  getArchivingNum() {
    return new Promise(resolve => {
      let {
        startTime: fromStartTime,
        endTime: toStartTime
      } = this.getTimeRange();
      this.jobAPIService
        .queryJobsUsingGET({
          akLoading: false,
          pageSize: 1,
          types: ['archive'],
          statusList: [
            this.jobStatus.running.value,
            this.jobStatus.initialization.value,
            this.jobStatus.pending.value,
            this.jobStatus.aborting.value,
            this.jobStatus.dispatching.value,
            this.jobStatus.redispatch.value
          ],
          fromStartTime,
          toStartTime
        })
        .subscribe(res => {
          this.archivingNum = res.totalCount || 0;
          resolve(true);
        });
    });
  }

  getHistoryBackingUpAbnormalNum() {
    return new Promise(resolve => {
      let {
        startTime: fromStartTime,
        endTime: toStartTime
      } = this.getTimeRange();
      this.jobAPIService
        .queryJobsUsingGET({
          akLoading: false,
          pageSize: 1,
          types: ['BACKUP'],
          statusList: [
            this.jobStatus.failed.value,
            this.jobStatus.abnormal.value,
            this.jobStatus.abort_failed.value,
            this.jobStatus.dispatch_failed.value
          ],
          fromStartTime,
          toStartTime
        })
        .subscribe(res => {
          this.backingUpAbnormalNum = res.totalCount || 0;
          this.taskJobType.backup.status =
            this.backingUpAbnormalNum > 0 ? this.abnormalStatus : null;
          resolve(true);
        });
    });
  }
  getHistoryDuplicateAbnormalNum() {
    return new Promise(resolve => {
      let {
        startTime: fromStartTime,
        endTime: toStartTime
      } = this.getTimeRange();
      this.jobAPIService
        .queryJobsUsingGET({
          akLoading: false,
          pageSize: 1,
          types: ['copy_replication'],
          statusList: [
            this.jobStatus.failed.value,
            this.jobStatus.abnormal.value,
            this.jobStatus.abort_failed.value,
            this.jobStatus.dispatch_failed.value
          ],
          fromStartTime,
          toStartTime
        })
        .subscribe(res => {
          this.duplicateAbnormalNum = res.totalCount || 0;
          this.taskJobType.duplicate.status =
            this.duplicateAbnormalNum > 0 ? this.abnormalStatus : null;
          resolve(true);
        });
    });
  }
  getHistoryArchivingAbnormalNum() {
    return new Promise(resolve => {
      let {
        startTime: fromStartTime,
        endTime: toStartTime
      } = this.getTimeRange();
      this.jobAPIService
        .queryJobsUsingGET({
          akLoading: false,
          pageSize: 1,
          types: ['archive'],
          statusList: [
            this.jobStatus.failed.value,
            this.jobStatus.abnormal.value,
            this.jobStatus.abort_failed.value,
            this.jobStatus.dispatch_failed.value
          ],
          fromStartTime,
          toStartTime
        })
        .subscribe(res => {
          this.archivingAbnormalNum = res.totalCount || 0;
          this.taskJobType.archive.status =
            this.archivingAbnormalNum > 0 ? this.abnormalStatus : null;
          resolve(true);
        });
    });
  }

  navigate(params = {}) {
    if (params['isProEnvir'] && !this.backingUpNum) return;
    let {
      startTime: fromStartTime,
      endTime: toStartTime
    } = this.getTimeRange();
    assign(params, {
      fromStartTime,
      toStartTime
    });
    this.appUtilsService.setCacheValue('homeToJob', params);
    this.globalService.emitStore({
      action: 'homeToJob',
      state: true
    });
    this.router.navigateByUrl(RouterUrl.InsightJobs);
  }
}
