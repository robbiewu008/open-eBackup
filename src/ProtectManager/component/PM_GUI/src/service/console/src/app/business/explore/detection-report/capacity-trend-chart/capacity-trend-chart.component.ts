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
  Input,
  OnChanges,
  OnInit,
  SimpleChanges
} from '@angular/core';
import {
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  DataMap,
  DataMapService,
  I18NService
} from 'app/shared';
import * as echarts from 'echarts';
import { assign, each, isEmpty, map, size } from 'lodash';

@Component({
  selector: 'aui-capacity-trend-chart',
  templateUrl: './capacity-trend-chart.component.html',
  styleUrls: ['./capacity-trend-chart.component.less'],
  providers: [CapacityCalculateLabel]
})
export class CapacityTrendChartComponent implements OnInit, OnChanges {
  @Input() capacityData;

  sizeMap = {
    [CAPACITY_UNIT.KB]: 1024,
    [CAPACITY_UNIT.MB]: 1024 * 1024,
    [CAPACITY_UNIT.GB]: 1024 * 1024 * 1024,
    [CAPACITY_UNIT.TB]: 1024 * 1024 * 1024 * 1024,
    [CAPACITY_UNIT.PB]: 1024 * 1024 * 1024 * 1024 * 1024,
    [CAPACITY_UNIT.EB]: 1024 * 1024 * 1024 * 1024 * 1024 * 1024
  };

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnChanges(changes: SimpleChanges) {
    if (!isEmpty(changes.capacityData?.currentValue)) {
      this.initChart();
    }
  }

  ngOnInit() {}

  transformToTargetUnit(value, unit, upper) {
    if (upper) {
      return value / this.sizeMap[unit];
    }
    return value * this.sizeMap[unit];
  }

  createChart(times, datas) {
    const node: any = document.getElementById('capacity-trend-chart');
    if (!node) {
      return;
    }
    const chart = echarts.init(node);
    // 处理容量
    const sizeArr = map(datas, 'value');
    const max: string = this.capacityCalculateLabel.transform(
      Math.max(...sizeArr),
      '1.1-3',
      CAPACITY_UNIT.BYTE
    );
    const maxSize = parseInt(max.split(' ')[0]);
    const maxUnit = max.split(' ')[1];
    each(datas, item => {
      assign(item, {
        value: this.transformToTargetUnit(item.value, maxUnit, true)
      });
    });
    const options = {
      grid: {
        y: 40,
        y2: 40,
        x: 10,
        x2: 50,
        containLabel: true
      },
      graphic: [
        {
          id: 'unit',
          type: 'text',
          left: 8,
          top: 16,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: `(${maxUnit})`,
            textAlign: 'right',
            fill: '#9EA4B3',
            width: 31,
            height: 14,
            fontSize: 12
          }
        }
      ],
      tooltip: {
        trigger: 'axis',
        axisPointer: {
          type: 'line',
          z: 10,
          lineStyle: {
            color: '#d7dae2',
            cursor: 'pointer'
          }
        },
        extraCssText: 'border-radius: 0; padding: 12px 16px',
        backgroundColor: '#6F6F6F',
        formatter: params => {
          let tip = `<div style='font-size: 12px; color: #D4D9E6'>${params[0].axisValue}</div>`;
          params.forEach(res => {
            tip += `<div style='font-size: 12px; color: #D4D9E6; padding-right:10px;'>
                  <span style='font-size: 12px; color: #F7FAFF;'>${this.i18n.get(
                    'explore_capacity_size_label'
                  )}${
              this.i18n.isEn ? ': ' : '：'
            }${this.capacityCalculateLabel.transform(
              this.transformToTargetUnit(res.value, maxUnit, false),
              '1.1-3',
              CAPACITY_UNIT.BYTE
            )}</span>
                  </div>`;
          });
          return tip;
        }
      },
      xAxis: {
        type: 'category',
        data: times,
        splitLine: {
          show: false
        },
        boundaryGap: false,
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
          rotate: size(times) > 10 ? 80 : 0,
          color: '#9EA4B3',
          formatter: params => {
            let newParamsName = '';
            const paramsNameNumber = params.length;
            const provideNumber = 10;
            const rowNumber = Math.ceil(paramsNameNumber / provideNumber);
            if (paramsNameNumber > provideNumber) {
              for (let p = 0; p < rowNumber; p++) {
                let tempStr = '';
                const start = p * provideNumber;
                const end = start + provideNumber;
                if (p == rowNumber - 1) {
                  tempStr = params.substring(start, paramsNameNumber);
                } else {
                  tempStr = params.substring(start, end) + '\n';
                }
                newParamsName += tempStr;
              }
            } else {
              newParamsName = params;
            }
            return newParamsName;
          }
        }
      },
      yAxis: {
        type: 'value',
        min: 0,
        max: maxSize < 2 ? 2 : Math.floor(maxSize * 1.5),
        minInterval: 1,
        axisLabel: {
          formatter: item => {
            return item + '';
          },
          show: true,
          color: '#9EA4B3'
        },
        splitLine: {
          lineStyle: {
            type: 'dashed',
            color: '#E6EBF5',
            width: 1
          }
        },
        axisLine: {
          lineStyle: {
            color: 'transparent'
          }
        }
      },
      series: {
        data: datas,
        type: 'line',
        color: '#6C92FA',
        symbolSize: 8,
        symbol: 'circle'
      }
    };
    chart.setOption(options);
  }

  initChart() {
    if (isEmpty(this.capacityData)) {
      return;
    }
    const times = map(this.capacityData, 'time');
    const datas = map(this.capacityData, item => {
      return {
        value: item.size,
        itemStyle: {
          color:
            item.status === DataMap.detectionSnapshotStatus.infected.value
              ? '#F45C5E'
              : '#6C92FA'
        }
      };
    });
    this.createChart(times, datas);
  }
}
