import { Component, Input, ElementRef, OnInit } from '@angular/core';
import { MenuItem } from '@iux/live';
import {
  CookieService,
  I18NService,
  JobColorConsts,
  RouterUrl
} from 'app/shared';
import {
  ApiMultiClustersService,
  JobAPIService
} from 'app/shared/api/services';
import { Router } from '@angular/router';
import { DataMap } from 'app/shared/consts';
import { assign } from 'lodash';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'mission-overview',
  templateUrl: './mission-overview.component.html',
  styleUrls: ['./mission-overview.component.less']
})
export class MissionOverviewComponent implements OnInit {
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
  startTime = 0;
  endTime = 0;
  jobText = this.i18n.get('common_last_24hours_jobs_label');
  isMultiCluster = true;
  jobPeriod = 'lastDay';
  timeMap;

  constructor(
    private i18n: I18NService,
    public el: ElementRef,
    public cookieService: CookieService,
    private jobAPIService: JobAPIService,
    private multiClustersServiceApi: ApiMultiClustersService,
    public router: Router,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.getAllCusterShow();
    this.initData();
    this.endTime = new Date().getTime();
    this.startTime = new Date().getTime() - 24 * 3600 * 1000;
    this.refreshData();
  }

  refreshData() {
    this.cardInfo.loading = true;
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
  }

  getAllCusterShow() {
    const clusterObj = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isMultiCluster =
      !clusterObj ||
      (clusterObj && clusterObj['icon'] === 'aui-icon-all-cluster');
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
    this.endTime = 0;
    this.startTime = 0;
    this.options = [
      {
        id: 'all',
        label: this.i18n.get('common_all_jobs_label'),
        onClick: data => {
          this.jobText = this.i18n.get('common_all_jobs_label');
          this.endTime = 0;
          this.startTime = 0;
          this.jobPeriod = 'all';
          this.initAsyncData();
        }
      },
      {
        id: '24hs',
        label: this.i18n.get('common_last_24hours_jobs_label'),
        onClick: data => {
          this.jobText = this.i18n.get('common_last_24hours_jobs_label');
          this.endTime = new Date().getTime();
          this.startTime = new Date().getTime() - 24 * 3600 * 1000;
          this.jobPeriod = 'lastDay';
          this.initAsyncData();
        }
      },
      {
        id: '7days',
        label: this.i18n.get('common_last_7days_jobs_label'),
        onClick: data => {
          this.jobText = this.i18n.get('common_last_7days_jobs_label');
          this.endTime = new Date().getTime();
          this.startTime = new Date().getTime() - 7 * 24 * 3600 * 1000;
          this.jobPeriod = 'lastWeek';
          this.initAsyncData();
        }
      },
      {
        id: '1month',
        label: this.i18n.get('common_last_1month_jobs_label'),
        onClick: data => {
          this.jobText = this.i18n.get('common_last_1month_jobs_label');
          this.endTime = new Date().getTime();
          this.startTime =
            new Date().getTime() - this.getMonthDays() * 24 * 3600 * 1000;
          this.jobPeriod = 'lastMonth';
          this.initAsyncData();
        }
      }
    ];
  }

  getMonthDays(): number {
    const year = new Date().getFullYear();
    const month = new Date().getMonth();
    return new Date(year, month, 0).getDate();
  }

  getTimeRange() {
    let startTime;
    let endTime;
    let jobPeriod;
    endTime = new Date().getTime();
    switch (this.cardInfo.selectTime) {
      case 0:
        jobPeriod = 'all';
        break;
      case 3:
        startTime = new Date().getTime() - 24 * 3600 * 1000;
        jobPeriod = 'lastDay';
        break;
      case 4:
        startTime = new Date().getTime() - 7 * 24 * 3600 * 1000;
        jobPeriod = 'lastWeek';
        break;
      case 5:
        startTime = this.getMonthDays() * 24 * 3600 * 1000;
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
    this.jobItem[DataMap.Job_status.pending.value] = res.pending;
    this.jobItem.totalItem = res.total;
  }

  initAsyncData() {
    return new Promise(resolve => {
      if (this.isMultiCluster) {
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
        let { startTime, endTime } = this.getTimeRange();
        let params: any = {
          akLoading: false
        };
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
          ]
        })
        .subscribe(res => {
          this.backingUpNum = res.totalCount || 0;
          resolve(true);
        });
    });
  }
  getDuplicateNum() {
    return new Promise(resolve => {
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
          ]
        })
        .subscribe(res => {
          this.duplicateNum = res.totalCount || 0;
          resolve(true);
        });
    });
  }
  getArchivingNum() {
    return new Promise(resolve => {
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
          ]
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
          resolve(true);
        });
    });
  }

  navigate(params = {}) {
    this.router.navigate([RouterUrl.InsightJobs], {
      queryParams: {
        ...params,
        cluster: this.cardInfo.selectcluster,
        time: this.cardInfo.selectTime
      }
    });
  }
}
