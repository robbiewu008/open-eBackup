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
import { Component, OnInit, ElementRef, Input, OnDestroy } from '@angular/core';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import { PerformanceApiDescService } from 'app/shared/api/services';
import * as echarts from 'echarts';
import {
  assign,
  chain,
  every,
  filter,
  find,
  isEmpty,
  isNaN,
  reduce,
  size,
  zip
} from 'lodash';
import { Subject, forkJoin, takeUntil } from 'rxjs';
import {
  CAPACITY_UNIT,
  CapacityCalculateLabel,
  GlobalService,
  I18NService,
  SYSTEM_TIME,
  THEME_TRIGGER_ACTION,
  ThemeEnum,
  getAppTheme
} from 'app/shared';
import GetPerformanceUsingGETParams = PerformanceApiDescService.GetPerformanceUsingGETParams;
import { DatePipe } from '@angular/common';

@Component({
  selector: 'performance-time-chart',
  templateUrl: './performance-time-chart.component.html',
  styleUrls: ['./performance-time-chart.component.less'],
  providers: [DatePipe, CapacityCalculateLabel]
})
export class PerformanceTimeChartComponent implements OnInit, OnDestroy {
  @Input() cardInfo: any = {};
  loading = false;
  performanceChart: any;
  performanceOption: any;
  startTime: number;
  endTime: number;
  showChart = false;
  readAvg: any = '--';
  readAvgLabel: any = '--';
  unit = '';
  avgSeriesData = [];
  maxSeriesData = [];
  maxYAxis = 0;
  isNoData = true;
  destroy$ = new Subject();
  constructor(
    public el: ElementRef,
    public systemTimeService: SystemTimeService,
    public performanceApiService: PerformanceApiDescService,
    private i18n: I18NService,
    private datePipe: DatePipe,
    private globalService: GlobalService,
    private appUtilsService: AppUtilsService,
    private capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit(): void {
    this.cardInfo.loading = false;
    this.globalService
      .getState(THEME_TRIGGER_ACTION)
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        if (isEmpty(this.performanceOption)) {
          return;
        }
        this.performanceOption.yAxis.splitLine.lineStyle.color = this.getYAxisLine();
        this.performanceOption.tooltip.axisPointer.lineStyle.color = this.getAxisPointerLine();
        this.performanceOption.tooltip.backgroundColor = this.getTooltipBkcolor();
        this.performanceOption.xAxis.axisLabel.color = this.getAxisLabelColor();
        this.performanceOption.yAxis.axisLabel.color = this.getAxisLabelColor();
        this.performanceChart.setOption(this.performanceOption);
      });
  }

  isThemeLight(): boolean {
    return getAppTheme(this.i18n) === ThemeEnum.light;
  }

  getYAxisLine() {
    return this.isThemeLight() ? '#E6E6E6' : '#262626';
  }

  getAxisPointerLine() {
    return this.isThemeLight() ? '#d9d9d9' : '#4d4d4d';
  }

  getAxisLabelColor() {
    return this.isThemeLight() ? '#4D4D4D' : '#B3B3B3';
  }

  getTooltipBkcolor() {
    return this.isThemeLight() ? '#ffffff' : '#272933';
  }

  createChart() {
    setTimeout(() => {
      this.performanceChart = echarts.init(
        this.el.nativeElement.querySelector('#performance-time-chart'),
        'dark'
      );
      const interval = this.maxYAxis / 2;
      this.performanceOption = {
        backgroundColor: 'transparent',
        grid: {
          left: '3%',
          right: '10%',
          bottom: '3%',
          top: '10px',
          containLabel: true
        },
        xAxis: {
          type: 'category',
          boundaryGap: false,
          data: this.avgSeriesData.map(item => item[0] || 0),
          axisLabel: {
            formatter: value => {
              return value
                .split(' ')
                .reverse()
                .join('\n');
            },
            showMaxLabel: true,
            color: this.getAxisLabelColor()
          }
        },
        yAxis: {
          type: 'value',
          min: 0,
          max: this.maxYAxis,
          splitNumber: 2,
          interval,
          axisLabel: {
            color: this.getAxisLabelColor(),
            showMaxLabel: true,
            formatter: value => {
              return `${this.capacityCalculateLabel.transform(
                value,
                '1.0-3',
                CAPACITY_UNIT.KB,
                false
              )}/s`;
            }
          },
          splitLine: {
            lineStyle: {
              type: 'solid',
              color: this.getYAxisLine(),
              width: 1
            }
          }
        },
        tooltip: {
          trigger: 'axis',
          borderWidth: 0,
          backgroundColor: this.getTooltipBkcolor(),
          textStyle: {
            color: '#808080',
            fontSize: 14
          },
          axisPointer: {
            lineStyle: {
              color: this.getAxisPointerLine(),
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
                          
                          <span>${this.capacityCalculateLabel.transform(
                            value,
                            '1.0-3',
                            CAPACITY_UNIT.KB,
                            false
                          )}/s</span>  
                      </div>  
                      `;
            });
            tooltipContent += '</div>';
            return tooltipContent;
          }
        },
        series: [
          {
            name: this.i18n.get('common_home_max_label'),
            data: this.maxSeriesData.map(item => item[1] || 0),
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
            name: this.i18n.get('common_home_avg_label'),
            data: this.avgSeriesData.map(item => item[1] || 0),
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
    this.readAvgLabel = '';
    this.unit = '';
    this.systemTimeService.getSystemTime(false).subscribe(res => {
      this.endTime = this.appUtilsService.toSystemTimeLong(res.time);
      if (!this.endTime) {
        this.endTime = this.appUtilsService.toSystemTimeLong(
          res.time.replace(/-/g, '/')
        );
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
      const params: GetPerformanceUsingGETParams = {
        indicatorList: [3],
        staticsMode: 'avg',
        startTime: this.startTime,
        endTime: this.endTime,
        akLoading: false,
        akDoException: false,
        clustersType: this.cardInfo.clusterType
      };
      if (this.appUtilsService.isDecouple) {
        const [clusterId] = this.cardInfo?.selectNode;
        const node = find(this.cardInfo.clusterNodesOptions, {
          value: clusterId
        });
        delete params.clustersType; // e1000直接使用targetEsn就能查看性能
        assign(params, {
          targetEsn: node?.storageEsn
        });
      } else {
        const [clusterId, NodeId] = this.cardInfo?.selectNode;
        const node = find(
          find(this.cardInfo.clusterNodesOptions, { value: clusterId })
            ?.children || [],
          { value: NodeId }
        );
        assign(params, {
          clustersId: clusterId,
          memberEsn: node?.remoteEsn
        });
      }

      forkJoin([
        this.performanceApiService.getPerformanceUsingGET(params),
        this.performanceApiService.getPerformanceUsingGET({
          ...params,
          staticsMode: 'max'
        })
      ]).subscribe({
        next: res => {
          if (every(res, isEmpty)) {
            this.cardInfo.loading = false;
            this.isNoData = true;
            return;
          }
          this.handleData(res);
        },
        error: err => {
          this.isNoData = true;
          this.loading = false;
        }
      });
    });
  }

  handleData(res) {
    this.isNoData = false;
    this.showChart = true;
    res.forEach((res, index) => {
      if (isEmpty(res) || isEmpty(res[0].indicatorValues)) {
        this.readAvg = 0;
        this.readAvgLabel = '';
        if (index === 0) {
          this.avgSeriesData = [];
        } else {
          this.maxYAxis = 0;
          this.maxSeriesData = [];
        }
        this.createChart();
        return;
      }
      const xAxisDataTemp = res[0].timestamps.map(item => {
        return this.datePipe.transform(
          item,
          'yyyy-MM-dd HH:mm:ss',
          SYSTEM_TIME.timeZone
        );
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
        const avgData = +(allReadData / size(seriesReadData)).toFixed(2);
        const bindLabel = this.capacityCalculateLabel
          .transform(avgData, '1.0-3', CAPACITY_UNIT.KB, false)
          .split(' ');
        this.readAvg = bindLabel[0] || 0;
        this.readAvgLabel = bindLabel[1] ? `${bindLabel[1]}/s` : '';
        this.avgSeriesData = zip(xAxisDataTemp, seriesReadData);
      } else {
        this.maxSeriesData = zip(xAxisDataTemp, seriesReadData);
        this.maxYAxis = chain(seriesReadData)
          .map(Number)
          .compact()
          .max()
          .value();
      }
    });
    this.createChart();
  }
}
