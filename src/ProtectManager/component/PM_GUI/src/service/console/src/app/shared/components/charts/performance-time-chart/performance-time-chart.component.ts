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
import { Component, OnInit, ElementRef, Input } from '@angular/core';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import { PerformanceApiDescService } from 'app/shared/api/services';
import * as echarts from 'echarts';
import { filter, isEmpty, isNaN, reduce, size, zip } from 'lodash';
import { forkJoin } from 'rxjs';
import { I18NService } from 'app/shared';

@Component({
  selector: 'performance-time-chart',
  templateUrl: './performance-time-chart.component.html',
  styleUrls: ['./performance-time-chart.component.less']
})
export class PerformanceTimeChartComponent implements OnInit {
  @Input() cardInfo: any = {};
  loading = false;
  performanceChart: any;
  performanceOption: any;
  startTime: number;
  endTime: number;
  showChart = false;
  readAvg: any = '--';
  unit = '';
  avgSeriesData = [];
  maxSeriesData = [];
  isNoData = true;
  constructor(
    public el: ElementRef,
    public systemTimeService: SystemTimeService,
    public performanceApiService: PerformanceApiDescService,
    private i18n: I18NService
  ) {}
  ngOnInit(): void {
    this.cardInfo.loading = true;
  }
  createChart() {
    setTimeout(() => {
      this.performanceChart = echarts.init(
        this.el.nativeElement.querySelector('#performance-time-chart'),
        'dark'
      );
      this.performanceOption = {
        backgroundColor: 'transparent',
        grid: {
          left: '3%',
          right: '4%',
          bottom: '3%',
          top: '10px',
          containLabel: true
        },
        xAxis: {
          type: 'category',
          boundaryGap: false,
          data: this.avgSeriesData.map(item => item[0] || 0)
        },
        yAxis: {
          type: 'value',
          splitNumber: 2,
          axisLabel: {
            formatter: `{value} ${this.unit}`
          }
        },
        tooltip: {
          trigger: 'axis',
          borderWidth: 0,
          backgroundColor: 'rgba(0, 0, 0, 0.7)',
          textStyle: {
            color: '#fff',
            fontSize: 14
          },
          axisPointer: {
            lineStyle: {
              color: '#fff',
              type: 'dashed'
            }
          },
          formatter: params => {
            var tooltipContent = params[0].name + '<br/>';
            tooltipContent +=
              '<div style="display: flex; flex-direction: column;">';
            params.forEach(item => {
              var color = item.color;
              var seriesName = item.seriesName;
              var value = item.value + this.unit;
              tooltipContent += `  
                      <div style="display: flex; align-items: center; justify-content: space-between;width:150px">  
                          <span>
                          <span style="display: inline-block; width: 10px; height: 10px; background-color: ${color}; margin-right: 5px;"></span>  
                          
                          ${seriesName}
                          </span>
                          
                          <span>${value}</span>  
                      </div>  
                      `;
            });
            tooltipContent += '</div>';
            return tooltipContent;
          }
        },
        series: [
          {
            name: this.i18n.get('common_home_avg_label'),
            data: this.avgSeriesData.map(item => item[1] || 0),
            type: 'line',

            lineStyle: {
              color: '#3388FF'
            },
            itemStyle: {
              normal: {
                color: '#3388FF'
              }
            },
            areaStyle: {
              color: 'rgba(51, 136, 255, 0.3)'
            }
          },
          {
            name: this.i18n.get('common_home_max_label'),
            data: this.maxSeriesData.map(item => item[1] || 0),
            type: 'line',
            lineStyle: {
              color: '#2EB6E6'
            },
            itemStyle: {
              normal: {
                color: '#2EB6E6'
              }
            },
            areaStyle: {
              color: 'rgba(46, 182, 230, 0.3)'
            }
          }
        ]
      };

      this.performanceChart.setOption(this.performanceOption);
      this.cardInfo.loading = false;
    }, 0);
  }
  refreshData() {
    this.cardInfo.loading = true;
    this.getPerformance();
  }
  getPerformance() {
    this.isNoData = true;
    this.readAvg = '--';
    this.unit = '';
    this.systemTimeService.getSystemTime(false).subscribe(res => {
      this.endTime = new Date(res.time).getTime();
      if (!this.endTime) {
        this.endTime = new Date(res.time.replace(/-/g, '/')).getTime();
      }
      switch (this.cardInfo.selectTime) {
        case 1:
          this.startTime = this.endTime - 300 * 1000; //5min
          break;
        case 2:
          this.startTime = this.endTime - 1800 * 1000; //30min
          break;
        case 3:
          this.startTime = this.endTime - 24 * 3600 * 1000; //24h
          break;
        case 4:
          this.startTime = this.endTime - 7 * 24 * 3600 * 1000; //1week
          break;
        case 5:
          this.startTime = this.endTime - 30 * 24 * 3600 * 1000; //1month
          break;
        case 6:
          this.startTime = this.endTime - 365 * 24 * 3600 * 1000; //1year
          break;
        default:
          break;
      }
      const params = {
        indicatorList: [3],
        staticsMode: 'avg',
        startTime: this.startTime,
        endTime: this.endTime,
        akLoading: false,
        akDoException: false
      };

      forkJoin([
        this.performanceApiService.getPerformanceUsingGET(params),
        this.performanceApiService.getPerformanceUsingGET({
          ...params,
          staticsMode: 'max'
        })
      ]).subscribe(
        res => {
          this.isNoData = false;
          this.handleData(res);
        },
        err => {
          this.isNoData = true;
          this.loading = false;
        }
      );
    });
  }

  handleData(res) {
    this.loading = false;
    this.showChart = true;
    res.forEach((res, index) => {
      if (isEmpty(res) || isEmpty(res[0].indicatorValues)) {
        this.readAvg = 0;
        this.createChart();
        return;
      }
      const xAxisDataTemp = res[0].timestamps.map(item => {
        return this.formatTime(item);
      });
      const seriesReadData = res[0].indicatorValues;
      const seriesReadDataTemp = filter(seriesReadData, item => {
        return !isNaN(+item);
      });
      const maxRead = Math.ceil(Math.max.apply(null, seriesReadDataTemp) * 1.5);
      const allReadData = reduce(
        seriesReadData,
        (sum, n) => {
          return +sum + (isNaN(+n) ? 0 : +n);
        },
        0
      );
      this.unit = res[0].unit;
      if (index === 0) {
        this.readAvg = +(allReadData / size(seriesReadData)).toFixed(2);
        this.avgSeriesData = zip(xAxisDataTemp, seriesReadData);
      } else {
        this.maxSeriesData = zip(xAxisDataTemp, seriesReadData);
      }
    });
    this.createChart();
  }

  formatTime(value) {
    const date = new Date(Number(value));
    var year = date.getFullYear();
    var month = String(date.getMonth() + 1).padStart(2, '0');
    var day = String(date.getDate()).padStart(2, '0');
    var hours = String(date.getHours()).padStart(2, '0');
    var minutes = String(date.getMinutes()).padStart(2, '0');
    var seconds = String(date.getSeconds()).padStart(2, '0');
    return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;
  }
}
