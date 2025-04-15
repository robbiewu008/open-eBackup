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
import { Component, ElementRef, OnInit } from '@angular/core';
import { MenuItem } from '@iux/live';
import { CookieService, I18NService, JobColorConsts } from 'app/shared';
import {
  ApiMultiClustersService,
  JobAPIService
} from 'app/shared/api/services';
import { DataMap } from 'app/shared/consts';
import * as echarts from 'echarts';
import { assign } from 'lodash';

@Component({
  selector: 'aui-job-chart',
  templateUrl: './job-chart.component.html',
  styleUrls: ['./job-chart.component.css']
})
export class JobChartComponent implements OnInit {
  jobColorConsts = JobColorConsts;
  jobItem;
  options: MenuItem[];
  optionSelected = '24hs'; // time stamp
  jobsOption;
  jobsChart: any;
  jobStatus = DataMap.Job_status;
  startTime = 0;
  endTime = 0;
  jobText = this.i18n.get('common_last_24hours_jobs_label');
  isMultiCluster = true;
  jobPeriod = 'lastDay';

  constructor(
    private i18n: I18NService,
    public el: ElementRef,
    public cookieService: CookieService,
    private jobAPIService: JobAPIService,
    private multiClustersServiceApi: ApiMultiClustersService
  ) {}

  ngOnInit() {
    this.getAllCusterShow();
    this.initData();
    this.createChart();
    this.endTime = new Date().getTime();
    this.startTime = new Date().getTime() - 24 * 3600 * 1000;
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

  initChartData(res) {
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
      res.pending + (res.dispatching || 0) + (res.redispatch || 0);
    this.jobItem.totalItem = res.total;
    this.jobsChart.setOption({
      graphic: {
        id: 'total',
        style: {
          text: this.jobItem.totalItem + ''
        }
      },
      series: {
        data: [
          {
            value: this.jobItem[DataMap.Job_status.success.value],
            name: 'successful'
          },
          {
            value: this.jobItem[DataMap.Job_status.running.value],
            name: 'running'
          },
          {
            value: this.jobItem[DataMap.Job_status.failed.value],
            name: 'failed'
          },
          {
            value: this.jobItem[DataMap.Job_status.aborted.value],
            name: 'aborted'
          },
          {
            value: this.jobItem[DataMap.Job_status.pending.value],
            name: 'pending'
          }
        ]
      }
    });
    // 无数据置灰拼图
    if (this.jobItem.totalItem === 0) {
      this.jobsChart.setOption({
        graphic: {
          id: 'mask',
          type: 'ring',
          left: 'center',
          top: 'center',
          z: 2,
          zlevel: 101,
          cursor: 'unset',
          shape: {
            r: 101,
            r0: 82
          },
          style: {
            fill: '#545a67',
            width: 30,
            height: 30
          }
        }
      });
    }
  }

  initAsyncData() {
    if (this.isMultiCluster) {
      const multiParams = { akLoading: false, jobPeriod: this.jobPeriod };
      this.multiClustersServiceApi
        .getMultiClusterJobs(multiParams)
        .subscribe(res => this.initChartData(res));
    } else {
      const params = { akLoading: false };
      if (!!this.startTime && !!this.endTime) {
        assign(params, {
          startTime: this.startTime,
          endTime: this.endTime
        });
      }
      this.jobAPIService
        .summaryUsingGET(params)
        .subscribe(res => this.initChartData(res));
    }
  }

  createChart() {
    this.jobsChart = echarts.init(
      this.el.nativeElement.querySelector('#job-chart')
    );
    this.jobsOption = {
      graphic: [
        {
          id: 'total',
          type: 'text',
          left: 'center',
          top: '42%',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: '0',
            textAlign: 'center',
            fill: '#FFFFFF',
            width: 30,
            height: 30,
            fontSize: 32
          }
        },
        {
          type: 'text',
          left: 'center',
          top: '58%',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: this.i18n.get('common_all_label'),
            textAlign: 'center',
            fill: '#B8BECC',
            width: 30,
            height: 30,
            fontSize: 14
          }
        },
        {
          type: 'ring',
          left: 'center',
          top: 'center',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          shape: {
            r: 110,
            r0: 109
          },
          style: {
            fill: '#545A67',
            width: 30,
            height: 30
          }
        }
      ],
      series: [
        {
          name: 'Jobs',
          type: 'pie',
          color: [
            this.jobColorConsts.SUCCESSFUL,
            this.jobColorConsts.RUNNING,
            this.jobColorConsts.FAILED,
            this.jobColorConsts.ABORTED,
            this.jobColorConsts.PENDING
          ],
          radius: [84, 101],
          avoidLabelOverlap: false,
          cursor: 'unset',
          label: {
            show: false
          },
          emphasis: {
            label: {
              show: true,
              scale: false
            }
          },
          labelLine: {
            show: false
          },
          itemStyle: {
            borderWidth: 2,
            borderColor: '#24272E'
          },
          data: [
            { value: 0, name: this.i18n.get('common_successful_label') },
            { value: 0, name: this.i18n.get('common_running_label') },
            { value: 0, name: this.i18n.get('common_fail_label') },
            { value: 0, name: this.i18n.get('common_job_stopped_label') },
            { value: 0, name: this.i18n.get('common_pending_label') }
          ]
        }
      ]
    };
    this.jobsChart.setOption(this.jobsOption);
    this.jobsChart.on('mouseover', () => {
      this.jobsChart.dispatchAction({
        type: 'downplay'
      });
    });
  }
}
