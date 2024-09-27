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
import { Component, OnInit, ElementRef } from '@angular/core';
import * as echarts from 'echarts';
import {
  I18NService,
  CapacityApiService,
  CapacityCalculateLabel,
  CookieService
} from 'app/shared';
import { remove as _remove } from 'lodash';
import { CAPACITY_UNIT } from 'app/shared/consts';

@Component({
  selector: 'aui-system-capacity-chart',
  templateUrl: './system-capacity-chart.component.html',
  styleUrls: ['./system-capacity-chart.component.css'],
  providers: [CapacityCalculateLabel]
})
export class SystemCapacityChartComponent implements OnInit {
  options;
  capacity = {} as any;

  constructor(
    private el: ElementRef,
    public i18n: I18NService,
    public cookieService: CookieService,
    public capacityApiService: CapacityApiService,
    public capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnInit() {}

  criticalValueStyle(
    total: number | string,
    option: any,
    ringColor: string,
    notValueColor: string
  ) {
    if (total === 0 || total === '--') {
      option.graphic.push({
        type: 'ring',
        left: 'center',
        top: 'center',
        z: 2,
        zlevel: 100,
        cursor: 'unset',
        shape: {
          r: 100,
          r0: 85
        },
        style: {
          fill: ringColor,
          width: 30,
          height: 30
        }
      });
      if (total === '--') {
        _remove(option.graphic, (value: any) => {
          return value.id === 'number' || value.id === 'unit';
        });
        option.graphic.push(
          {
            type: 'rect',
            left: 90.5,
            top: '45%',
            z: 2,
            cursor: 'unset',
            zlevel: 100,
            shape: {
              width: 18,
              height: 2
            },
            style: {
              fill: notValueColor
            }
          },
          {
            type: 'rect',
            right: 90.5,
            top: '45%',
            z: 2,
            cursor: 'unset',
            zlevel: 100,
            shape: {
              width: 18,
              height: 2
            },
            style: {
              fill: notValueColor
            }
          }
        );
      }
    }
  }

  createCapacityChart(res) {
    if (!res) {
      return;
    }
    const total = this.capacityCalculateLabel.transform(
      res.totalCapacity,
      '1.3-3',
      CAPACITY_UNIT.KB,
      true
    );
    const capacityChart = echarts.init(
      this.el.nativeElement.querySelector('#capacity-forecast-chart')
    );
    const capacityOption = {
      graphic: [
        {
          id: 'number',
          type: 'text',
          right: '53%',
          top: '41%',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: total.slice(0, -6),
            textAlign: 'center',
            fill: '#282B33 ',
            width: 100,
            height: 30,
            fontSize: 32
          }
        },
        {
          id: 'number1',
          type: 'text',
          right: '35%',
          top: '45%',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: total.slice(-6, -2),
            textAlign: 'center',
            fill: '#282B33 ',
            width: 100,
            height: 30,
            fontSize: 20
          }
        },
        {
          id: 'unit',
          type: 'text',
          right: '17%',
          top: '45%',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: total.slice(-2),
            textAlign: 'right',
            fill: '#9EA4B3',
            width: 30,
            height: 30,
            fontSize: 18
          }
        },
        {
          type: 'text',
          left: 'center',
          top: '57%',
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
            fill: '#E6EBF5',
            width: 30,
            height: 30
          }
        }
      ],
      series: [
        {
          color: ['#6C92FA', 'transparent'],
          name: 'Capacity',
          type: 'pie',
          radius: [84, 101],
          avoidLabelOverlap: false,
          cursor: 'unset',
          z: 2,
          zlevel: 100,
          label: {
            show: false
          },
          emphasis: {
            scale: false,
            label: {
              show: false,
              fontSize: '30',
              fontWeight: 'bold'
            }
          },
          labelLine: {
            show: false
          },
          itemStyle: {
            borderWidth: 2,
            borderColor: '#ffffff'
          },
          data: [
            {
              value: res.usedCapacity,
              name: 'Used',
              itemStyle: {
                color: '#6C92FA'
              }
            },
            {
              value: res.freeCapacity,
              name: 'Available',
              itemStyle: {
                opacity: 0
              }
            }
          ]
        },
        {
          name: 'Capacity',
          type: 'pie',
          radius: [96, 101],
          avoidLabelOverlap: false,
          z: 1,
          cursor: 'unset',
          zlevel: 98,
          label: {
            show: false
          },
          emphasis: {
            scale: false,
            label: {
              show: false,
              fontSize: '30',
              fontWeight: 'bold'
            }
          },
          labelLine: {
            show: false
          },
          itemStyle: {
            borderWidth: 2,
            borderColor: '#ffffff'
          },
          data: [
            {
              value: res.usedCapacity,
              name: 'Used',
              itemStyle: {
                opacity: 0
              }
            },
            {
              value: res.freeCapacity,
              name: 'Available',
              itemStyle: {
                color: '#E6EBF5'
              },
              emphasis: { color: '#E6EBF5' }
            }
          ]
        }
      ]
    };
    this.criticalValueStyle(
      total.slice(0, -2),
      capacityOption,
      '#E6EBF5',
      '#2A2A2A'
    );
    capacityChart.setOption(capacityOption);
    capacityChart.on('mouseover', () => {
      capacityChart.dispatchAction({
        type: 'downplay'
      });
    });
    this.capacity = res;
  }
}
