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
import { Component, ElementRef, Input, OnChanges, OnInit } from '@angular/core';
import { I18NService } from 'app/shared/services/i18n.service';
import 'echarts/lib/chart/bar';
import 'echarts/lib/component/grid';
import 'echarts/lib/component/legend';
import echarts from 'echarts/lib/echarts';

@Component({
  selector: 'aui-summary-histogram',
  templateUrl: './summary-histogram.component.html',
  styleUrls: ['./summary-histogram.component.less']
})
export class SummaryHistogramComponent implements OnInit, OnChanges {
  constructor(public i18n: I18NService, private el: ElementRef) {}
  @Input() protectedArr;
  @Input() unprotectedArr;
  @Input() labelArray;
  splitNumber = 2;
  baseNumber = 4;
  folder = 2;
  yMax;
  interval;

  ngOnInit() {}

  ngOnChanges() {
    if (this.protectedArr && this.unprotectedArr) {
      setTimeout(() => {
        this.getYMax(this.getMax());
        this.getInterval();
        this.zeroStyle();
        this.createChart();
      }, 0);
    }
  }

  zeroStyle() {
    const zeroObject = {
      itemStyle: {
        opacity: 1,
        color: 'transparent'
      },
      value: 0
    };
    this.protectedArr = this.protectedArr.map(value => {
      return value || zeroObject;
    });
    this.unprotectedArr = this.unprotectedArr.map(value => {
      return value || zeroObject;
    });
  }

  getMax(): number {
    return Math.ceil(
      Math.max.apply(null, this.protectedArr.concat(this.unprotectedArr))
    );
  }

  getYMax(max: number) {
    if (max <= this.baseNumber) {
      this.yMax = this.baseNumber;
    } else {
      let temp = this.baseNumber;
      for (; temp < max; ) {
        temp = temp * this.folder;
      }
      this.yMax = temp;
    }
  }

  getInterval() {
    this.interval = this.yMax / this.splitNumber;
  }

  createChart() {
    const protectChart = echarts.init(
      this.el.nativeElement.querySelector('#chart')
    );

    const option = {
      grid: {
        y: 18,
        x: 30,
        x2: 0,
        containLabel: true
      },
      color: ['#6C92FA', '#E6EBF5'],
      legend: {
        data: [
          this.i18n.get('common_protected_label'),
          this.i18n.get('common_unprotected_label')
        ],
        selectedMode: false,
        y: 'bottom',
        icon: 'circle',
        itemWidth: 10,
        itemHeight: 10,
        itemGap: 90,
        textStyle: {
          color: '#6C7280'
        }
      },
      xAxis: [
        {
          data: this.labelArray,
          type: 'category',
          axisTick: {
            alignWithLabel: true,
            lineStyle: {
              width: 2
            }
          },
          axisLine: {
            lineStyle: {
              color: '#E6EBF5',
              width: 2
            }
          },
          axisLabel: {
            textStyle: {
              color: '#9EA4B3'
            }
          }
        }
      ],
      yAxis: [
        {
          type: 'value',
          min: 0,
          max: this.yMax,
          interval: this.interval,
          splitLine: {
            lineStyle: {
              type: 'dashed',
              color: '#E6EBF5',
              width: 2
            }
          },
          axisLine: {
            lineStyle: {
              color: 'transparent'
            }
          },
          axisLabel: {
            textStyle: {
              color: '#9EA4B3'
            }
          }
        }
      ],
      series: [
        {
          name: this.i18n.get('common_protected_label'),
          data: this.protectedArr,
          type: 'bar',
          barWidth: 20,
          barMinHeight: 2.5,
          barGap: 1,
          cursor: 'unset',
          label: {
            show: true,
            position: 'top',
            textStyle: {
              color: '#282B33'
            }
          }
        },
        {
          name: this.i18n.get('common_unprotected_label'),
          data: this.unprotectedArr,
          type: 'bar',
          barWidth: 20,
          barMinHeight: 2.5,
          cursor: 'unset',
          label: {
            show: true,
            position: 'top',
            textStyle: {
              color: '#282B33'
            }
          }
        }
      ]
    };

    protectChart.setOption(option);
    protectChart.on('mouseover', () => {
      protectChart.dispatchAction({
        type: 'downplay'
      });
    });

    window.onresize = () => {
      protectChart.resize();
    };
  }
}
