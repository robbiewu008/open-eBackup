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
import { Component, OnInit, ElementRef, Input, OnDestroy } from '@angular/core';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import * as echarts from 'echarts';
import {
  CapacityApiService,
  DataMap,
  GlobalService,
  THEME_TRIGGER_ACTION,
  ThemeEnum,
  getAppTheme,
  SYSTEM_TIME
} from 'app/shared';
import {
  filter,
  first,
  floor,
  maxBy,
  size,
  union,
  isUndefined,
  find,
  assign,
  toString,
  isEmpty
} from 'lodash';
import { I18NService } from 'app/shared';
import QueryNodesStorageTendencyUsingGETParams = CapacityApiService.QueryNodesStorageTendencyUsingGETParams;
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'capacity-diction-chart',
  templateUrl: './capacity-diction-chart.component.html',
  styleUrls: ['./capacity-diction-chart.component.less'],
  providers: [DatePipe]
})
export class CapacityDictionChartComponent implements OnInit, OnDestroy {
  @Input() cardInfo: any = {};
  chart: any;
  isNoData = true;
  existingDatas = [];
  forecastDatas = [];
  reachTo80 = '--';
  reachTo100 = '--';
  reachTo80Symbol;
  reachTo100Symbol;
  systemTime = new Date().getTime() / 1000 / 3600 / 24;
  chartOptions: any;
  destroy$ = new Subject();
  constructor(
    private i18n: I18NService,
    public el: ElementRef,
    private datePipe: DatePipe,
    public capacityApiService: CapacityApiService,
    private appUtilsService: AppUtilsService,
    private globalService: GlobalService
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
        if (isEmpty(this.chartOptions)) {
          return;
        }
        this.chartOptions.xAxis.axisLabel.color = this.getAxisLabelColor();
        this.chartOptions.yAxis.axisLabel.color = this.getAxisLabelColor();
        this.chartOptions.yAxis.splitLine.lineStyle.color = this.getYAxisLine();
        this.chartOptions.tooltip.axisPointer.lineStyle.color = this.getAxisPointerLine();
        this.chartOptions.tooltip.backgroundColor = this.getTooltipBkcolor();
        this.chart.setOption(this.chartOptions);
      });
  }

  isThemeLight(): boolean {
    return getAppTheme(this.i18n) === ThemeEnum.light;
  }

  getAxisLabelColor() {
    return this.isThemeLight() ? '#4D4D4D' : '#B3B3B3';
  }

  getYAxisLine() {
    return this.isThemeLight() ? '#E6E6E6' : '#262626';
  }

  getTooltipBkcolor() {
    return this.isThemeLight() ? '#ffffff' : '#272933';
  }

  getAxisPointerLine() {
    return this.isThemeLight() ? '#d9d9d9' : '#4d4d4d';
  }

  getCapcacityTendency() {
    this.isNoData = true;
    this.reachTo80 = '--';
    this.reachTo100 = '--';
    this.reachTo80Symbol = undefined;
    this.reachTo100Symbol = undefined;
    const params: QueryNodesStorageTendencyUsingGETParams = {
      akDoException: false,
      akLoading: false,
      clustersType: this.cardInfo?.clusterType,
      clustersId: toString(DataMap.Cluster_Type.local.value)
    };
    if (this.appUtilsService.isDecouple) {
      const [clusterId] = this.cardInfo?.selectNode;
      const node = find(this.cardInfo.clusterNodesOptions, {
        value: clusterId
      });
      assign(params, {
        clustersType: DataMap.Cluster_Type.local.value,
        esnList: [node?.storageEsn]
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
        esnList: [node?.remoteEsn]
      });
    }
    this.capacityApiService.queryNodesStorageTendencyUsingGET(params).subscribe(
      res => {
        this.isNoData = false;
        this.cardInfo.loading = false;
        this.existingDatas = this.formatData(res[0].existingDatas);
        this.forecastDatas = this.formatData(res[0].forecastDatas);
        this.handleData(res[0]);
        this.createChart();
      },
      error => {
        this.cardInfo.loading = false;
        this.isNoData = true;
      }
    );
  }

  handleData(res) {
    const existingMaxPercentageObj: any = maxBy(
      res.existingDatas,
      'percentage'
    );
    // 如果forecastDatas为[]maxBy结果为undefined
    const forecastMaxPercentageObj: any = maxBy(
      res.forecastDatas,
      'percentage'
    );
    const maxPercentageObj: any = maxBy(
      union(res.existingDatas, res.forecastDatas),
      'percentage'
    );

    if (maxPercentageObj?.percentage < 0.8) {
      const intervalTime =
        maxPercentageObj.timestamp / 1000 / 3600 / 24 - this.systemTime;
      this.reachTo100Symbol = this.reachTo80Symbol = '>';
      this.reachTo100 = this.reachTo80 = `${floor(intervalTime) + 1}`;
    } else {
      if (!isUndefined(forecastMaxPercentageObj)) {
        let intervalTime =
          forecastMaxPercentageObj.timestamp / 1000 / 3600 / 24 -
          this.systemTime;
        this.reachTo100 = `${floor(intervalTime) + 1}`;
      } else {
        this.reachTo100 = '0';
      }
      if (existingMaxPercentageObj?.percentage >= 0.8) {
        this.reachTo80 = '0';
      } else {
        const forecastMaxPercentageObjs: any = filter(
          res.forecastDatas,
          (resu: any) => {
            return resu.percentage >= 0.8;
          }
        );

        // filter后可能最终结果为[]
        if (size(forecastMaxPercentageObjs)) {
          this.reachTo80 = `${floor(
            first(forecastMaxPercentageObjs)['timestamp'] / 1000 / 3600 / 24 -
              this.systemTime
          ) + 1}`;
        } else {
          this.reachTo80 = '0';
        }
      }
      this.reachTo100Symbol = '>';
    }
  }
  formatData(data) {
    return data.map(item => {
      return [
        this.formatTime(item.timestamp),
        (item.percentage * 100).toFixed(0)
      ];
    });
  }

  formatTime(value) {
    return this.datePipe.transform(
      new Date(Number(value)),
      'yyyy-MM-dd HH:mm:ss',
      SYSTEM_TIME.timeZone
    );
  }

  refreshData() {
    this.cardInfo.loading = true;
    this.getCapcacityTendency();
  }
  createChart() {
    setTimeout(() => {
      this.chart = echarts.init(
        this.el.nativeElement.querySelector('#capacity-diction-chart'),
        'dark'
      );
      this.chartOptions = {
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
          axisLabel: {
            color: this.getAxisLabelColor(),
            showMaxLabel: true,
            formatter: value => {
              return value
                .split(' ')
                .reverse()
                .join('\n');
            }
          }
        },
        yAxis: {
          type: 'value',
          axisLabel: {
            color: this.getAxisLabelColor(),
            formatter: '{value}%'
          },
          splitLine: {
            lineStyle: {
              type: 'solid',
              color: this.getYAxisLine(),
              width: 1
            }
          },
          max: 100,
          min: 0,
          interval: 20
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
              var seriesName = this.i18n.get('common_home_capacity_label');
              var value = item.value[1] + '%';
              tooltipContent += `  
                      <div style="display: flex; align-items: center; justify-content: space-between;width:150px">  
                          <span>
                          <span style="display: inline-block; width: 10px; height: 10px; background-color: #3388FF; margin-right: 5px;"></span>  
                          
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
            type: 'line',
            smooth: 0.6,
            symbol: 'none',
            lineStyle: {
              color: '#3388FF',
              width: 2
            },
            areaStyle: {
              color: 'rgba(51, 136, 255, 0.3)'
            },
            markLine: {
              symbol: 'none',
              data: [
                {
                  yAxis: 80,
                  label: {
                    formatter: ''
                  },
                  lineStyle: {
                    color: '#fa8241',
                    width: 2
                  }
                }
              ]
            },
            data: this.existingDatas
          },
          {
            type: 'line',
            smooth: 0.6,
            symbol: 'none',
            lineStyle: {
              color: '#3388FF',
              type: 'dashed',
              width: 2
            },
            areaStyle: {
              color: 'rgba(255,255,255,0)'
            },
            data: this.forecastDatas
          }
        ]
      };

      this.chart.setOption(this.chartOptions);
      this.cardInfo.loading = false;
    }, 0);
  }
}
