import { Component, OnInit, Input } from '@angular/core';
import { CookieService, I18NService, ALARM_NAVIGATE_STATUS } from 'app/shared';
import {
  AlarmAndEventApiService,
  ApiMultiClustersService
} from 'app/shared/api/services';
import { AlarmColorConsts } from 'app/shared/consts';
import { DataMap, RouterUrl } from 'app/shared';
import { Router } from '@angular/router';
import { assign } from 'lodash';

@Component({
  selector: 'alarm',
  templateUrl: './alarm.component.html',
  styleUrls: ['./alarm.component.less']
})
export class AlarmComponent implements OnInit {
  @Input() cardInfo: any = {};
  alarmList: any = [];
  totalItem = 0; //全部
  criticalItem = 0; //紧急
  majorItem = 0; //重要
  warningItem = 0; //警告
  minorItem = 0;
  alarmsOption;
  alarmConsts = AlarmColorConsts;
  isMultiCluster = true;
  alarmSeverityType = DataMap.Alarm_Severity_Type;
  now = new Date();

  constructor(
    public i18n: I18NService,
    public cookieService: CookieService,
    public apiService: AlarmAndEventApiService,
    public router: Router,
    private multiClustersServiceApi: ApiMultiClustersService
  ) {}

  ngOnInit() {
    this.getAllCusterShow();
    this.initAsyncData();
  }

  getAllCusterShow() {
    const clusterObj = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isMultiCluster =
      !clusterObj ||
      (clusterObj && clusterObj['icon'] === 'aui-icon-all-cluster');
  }

  initChartData(res) {
    this.criticalItem = res.critical;
    this.warningItem = res.warning;
    this.majorItem = res.major;
    this.totalItem =
      this.criticalItem + this.warningItem + this.minorItem + this.majorItem;
  }

  initAsyncData() {
    this.cardInfo.loading = true;
    Promise.all([this.getData(), this.getList()]).then(() => {
      this.cardInfo.loading = false;
    });
  }

  getData() {
    return new Promise(resolve => {
      if (this.isMultiCluster) {
        this.multiClustersServiceApi
          .getMultiClusterAlarms({ akLoading: false })
          .subscribe(res => {
            this.initChartData(res);
            resolve(true);
          });
      } else {
        this.apiService
          .queryAlarmCountBySeverityUsingGET({ akLoading: false })
          .subscribe(res => {
            this.initChartData(res);
            resolve(true);
          });
      }
    });
  }

  getList() {
    return new Promise(resolve => {
      this.apiService
        .findPageUsingGET(
          assign({
            startIndex: 0,
            pageSize: 4,
            pageNo: 0,
            isVisible: true,
            akLoading: false,
            akDoException: false,
            shouldAllNodes: true,
            language: this.i18n.language === 'zh-cn' ? 'ZH' : 'EN'
          })
        )
        .subscribe(res => {
          this.alarmList = res.records;
          resolve(true);
        });
    });
  }

  formatTimeAgo(timestamp: number): string {
    const date = new Date(timestamp);
    const diff = this.now.getTime() - date.getTime();
    const seconds = Math.floor(diff / 1000);
    const minutes = Math.ceil(seconds / 60);
    const hours = Math.floor(minutes / 60);
    const days = Math.floor(hours / 24);

    // 根据时间差返回不同的字符串
    if (days > 0) {
      return `${days} ${this.i18n.get('common_home_day_ago_label')}`;
    } else if (hours > 0) {
      return `${hours} ${this.i18n.get('common_home_hour_ago_label')}`;
    } else if (minutes > 0) {
      return `${minutes} ${this.i18n.get('common_home_minute_ago_label')}`;
    }
  }

  alarmDetailClick(alarm) {
    ALARM_NAVIGATE_STATUS.sequence = alarm.sequence.toString();
    this.router.navigate([RouterUrl.InsightAlarms]);
  }

  navigate(params = {}) {
    this.router.navigate([RouterUrl.InsightAlarms], { queryParams: params });
  }

  convertToHTML(alarm) {
    return this.i18n.get(alarm.detail, alarm.params || []);
  }
}
