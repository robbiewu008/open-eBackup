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
  Component,
  ElementRef,
  OnDestroy,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  CopiesDetectReportService,
  DataMap,
  DataMapService,
  I18NService
} from 'app/shared';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import * as echarts from 'echarts';
import { each, isEmpty } from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { DetectionModelListComponent } from '../detection-model-list/detection-model-list.component';

@Component({
  selector: 'aui-overview',
  templateUrl: './overview.component.html',
  styleUrls: ['./overview.component.less']
})
export class OverviewComponent implements OnInit, OnDestroy {
  timeSub$: Subscription;
  destroy$ = new Subject();
  timeSub2$: Subscription;
  destroy2$ = new Subject();
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  allDetectionData = [
    {
      key: 'infected_copy_num',
      num: 0,
      color: '#F27272',
      label: this.i18n.get('explore_infected_label')
    },
    {
      key: 'abnormal_copy_num',
      num: 0,
      color: '#FBCC3F',
      label: this.i18n.get('common_status_exception_label')
    },
    {
      key: 'uninfected_copy_num',
      num: 0,
      color: '#86D44E',
      label: this.i18n.get('explore_uninfected_label')
    },
    {
      key: 'detecting_copy_num',
      num: 0,
      color: '#3388ff',
      label: this.i18n.get('explore_detecting_label')
    },
    {
      key: 'prepare_copy_num',
      num: 0,
      color: '#7799d9',
      label: this.i18n.get('explore_ready_to_detect_label')
    }
  ];
  selectedTime; // 用于检测结果的筛选时间
  detectionTimeRangeOption = this.dataMapService.toArray(
    'Detecion_Data_Time_Options'
  );
  detectionDataChart;
  chartOption;
  startTime: number;
  endTime: number;

  @ViewChild('modelList', { static: false })
  modelList: DetectionModelListComponent;

  constructor(
    public el: ElementRef,
    public i18n: I18NService,
    public systemTimeService: SystemTimeService,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private copiesDetectReportService: CopiesDetectReportService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
    this.destroy2$.next(true);
    this.destroy2$.complete();
  }

  ngOnInit() {
    this.getDetectionData();
    this.getEndTime();
    this.selectedTime = this.detectionTimeRangeOption[0].value;
  }

  onChange() {
    this.ngOnInit();
    this.modelList.tableData = {
      data: [],
      total: 0
    };
    this.modelList.dataTable.fetchData();
  }

  getDetectionData() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      resourceSubType: this.dataMapService
        .toArray('Detecting_Resource_Type')
        .map(item => item.value)
    };
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }
    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.copiesDetectReportService.ShowDetectionSummary({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe({
        next: res => {
          this.clearData();
          each(res, item => {
            this.initCountData(item);
          });
        },
        error: () => {
          this.clearData();
        }
      });
  }

  clearData() {
    each(this.allDetectionData, item => (item.num = 0));
  }

  getCurrentDateTimeStamp(res) {
    const now = new Date(res.time);
    let year = now.getFullYear();
    let month = now.getMonth() + 1;
    let day = now.getDate();
    let dateOnly = new Date(year, month - 1, day, 0, 0, 0);
    return dateOnly.getTime();
  }

  getEndTime() {
    if (this.timeSub2$) {
      this.timeSub2$.unsubscribe();
    }
    let loading = true;
    this.timeSub2$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          loading = !index;
          return this.systemTimeService.getSystemTime(!index);
        }),
        takeUntil(this.destroy2$)
      )
      .subscribe(res => {
        this.endTime = this.getCurrentDateTimeStamp(res);
        this.getGraphData(loading);
      });
  }

  getGraphData(loading) {
    this.copiesDetectReportService
      .ShowDetectionSummary({
        period: this.selectedTime,
        akLoading: loading
      })
      .subscribe(res => {
        const infectedData = [];
        const uninfectedData = [];
        const abnormalData = [];
        each(res, item => {
          const [year, month, day] = item.detection_date.split('-').map(Number);
          const tmpDate = new Date(0, 0, 1);
          tmpDate.setFullYear(year, month - 1, day);
          let tmpKey = tmpDate.getTime();
          infectedData.push([Number(tmpKey), String(item.infected_copy_num)]);
          uninfectedData.push([
            Number(tmpKey),
            String(item.uninfected_copy_num)
          ]);
          abnormalData.push([Number(tmpKey), String(item.abnormal_copy_num)]);
        });

        this.createChart();
        this.initChart(infectedData, uninfectedData, abnormalData);
      });
  }

  initCountData(data) {
    each(this.allDetectionData, item => {
      if (item.key === 'prepare_copy_num') {
        item.num += data.prepare_copy_num + data.uninspected_copy_num;
      } else {
        item.num += data[item.key];
      }
    });
  }

  changeTimeRange(e) {
    this.getEndTime();
  }

  createChart() {
    if (isEmpty(this.detectionDataChart)) {
      this.detectionDataChart = echarts.init(
        this.el.nativeElement.querySelector('#detection-data-chart')
      );
    }
  }

  initChart(infectedData, uninfectedData, abnormalData) {
    this.chartOption = {
      color: ['#F27272', '#FBCC3F', '#86D44E'],
      tooltip: {
        trigger: 'axis',
        axisPointer: {
          type: 'cross',
          label: {
            backgroundColor: '#6a7985'
          }
        }
      },
      legend: {
        data: [
          this.i18n.get('explore_infected_label'),
          this.i18n.get('common_status_exception_label'),
          this.i18n.get('explore_uninfected_label')
        ]
      },
      grid: {
        left: '20px',
        right: '35px',
        bottom: '20px',
        height: '100px',
        containLabel: true
      },
      xAxis: {
        type: 'time',
        scale: true,
        axisLabel: {
          formatter: value => {
            return echarts.format.formatTime('yyyy-MM-dd', value);
          }
        },
        boundaryGap: ['2%', '2%']
      },
      yAxis: [
        {
          type: 'value',
          minInterval: 1
        }
      ],
      series: [
        {
          name: this.i18n.get('explore_infected_label'),
          type: 'line',
          stack: 'Total',
          areaStyle: {},
          emphasis: {
            focus: 'series'
          },
          data: infectedData,
          symbol: 'none'
        },
        {
          name: this.i18n.get('common_status_exception_label'),
          type: 'line',
          stack: 'Total',
          areaStyle: {},
          emphasis: {
            focus: 'series'
          },
          data: abnormalData,
          symbol: 'none'
        },
        {
          name: this.i18n.get('explore_uninfected_label'),
          type: 'line',
          stack: 'Total',
          areaStyle: {},
          emphasis: {
            focus: 'series'
          },
          data: uninfectedData,
          symbol: 'none'
        }
      ]
    };
    this.detectionDataChart.setOption(this.chartOption);
    window.onresize = () => {
      this.detectionDataChart.resize();
    };
  }
}
