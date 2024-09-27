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
import * as echarts from 'echarts';
import { CapacityApiService } from 'app/shared';
import { filter, first, floor, maxBy, size, union, isUndefined } from 'lodash';
import { I18NService } from 'app/shared';

@Component({
  selector: 'capacity-diction-chart',
  templateUrl: './capacity-diction-chart.component.html',
  styleUrls: ['./capacity-diction-chart.component.less']
})
export class CapacityDictionChartComponent implements OnInit {
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
  constructor(
    private i18n: I18NService,
    public el: ElementRef,
    public capacityApiService: CapacityApiService
  ) {}

  ngOnInit(): void {
    this.cardInfo.loading = true;
  }

  getCapcacityTendency() {
    this.isNoData = true;
    this.reachTo80 = '--';
    this.reachTo100 = '--';
    this.reachTo80Symbol = undefined;
    this.reachTo100Symbol = undefined;
    const params = {
      akDoException: false,
      akLoading: false,
      clustersId: this.cardInfo?.selectNode[0],
      clustersType: this.cardInfo?.clusterType
    };
    this.capacityApiService
      .queryClusterStorageTendencyUsingGET(params)
      .subscribe(
        res => {
          this.isNoData = false;
          this.existingDatas = this.formatData(res.existingDatas);
          this.forecastDatas = this.formatData(res.forecastDatas);
          this.handleData(res);
          this.createChart();
        },
        error => {
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

    if (maxPercentageObj.percentage < 80) {
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
      if (existingMaxPercentageObj.percentage >= 80) {
        this.reachTo80 = '0';
      } else {
        const forecastMaxPercentageObjs: any = filter(
          res.forecastDatas,
          (resu: any) => {
            return resu.percentage >= 80;
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
    const date = new Date(Number(value));
    var year = date.getFullYear();
    var month = String(date.getMonth() + 1).padStart(2, '0');
    var day = String(date.getDate()).padStart(2, '0');
    return `${year}-${month}-${day}`;
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
      let option = {
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
          boundaryGap: false
        },
        yAxis: {
          type: 'value',
          axisLabel: {
            formatter: '{value} %'
          },
          max: 100,
          min: 0,
          interval: 20
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

      this.chart.setOption(option);
      this.cardInfo.loading = false;
    }, 0);
  }
}
