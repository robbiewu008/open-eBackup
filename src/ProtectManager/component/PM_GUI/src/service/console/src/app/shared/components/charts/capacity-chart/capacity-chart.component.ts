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

@Component({
  selector: 'capacity-chart',
  templateUrl: './capacity-chart.component.html',
  styleUrls: ['./capacity-chart.component.less']
})
export class CapacityChartComponent implements OnInit {
  @Input() color: string = '';
  @Input() warningColor: string = '';
  @Input() text: string = '已用';
  protectionChart: any;
  protectionOption: any;
  _percent: number = 0;
  @Input()
  get percent() {
    return this._percent;
  }
  set percent(value) {
    this._percent = value;
    this.updateChart();
  }
  constructor(public el: ElementRef) {}
  ngOnInit(): void {
    this.createChart();
  }
  getProtectionOption() {
    let percent = Number(this.percent);
    let unpercent = 100 - this.percent;
    let color = this.color;
    if (this.warningColor !== '' && percent >= 80) {
      color = this.warningColor;
    }
    if (isNaN(percent) || isNaN(unpercent)) {
      percent = 100;
      unpercent = 0;
    }
    return {
      series: [
        {
          name: 'Access From',
          type: 'pie',
          radius: ['85%', '100%'],
          color: [color, '#333'],
          itemStyle: {
            borderRadius: 10
          },
          label: {
            show: false,
            position: 'center'
          },
          startAngle: 90,
          data: [
            { value: percent, name: 'percent' },
            { value: unpercent, name: 'unpercent' }
          ],
          hoverAnimation: false
        }
      ]
    };
  }
  createChart() {
    this.protectionChart = echarts.init(
      this.el.nativeElement.querySelector('#capacity-chart')
    );
    this.protectionChart.setOption(this.getProtectionOption());
  }
  updateChart() {
    this.protectionChart?.setOption(this.getProtectionOption());
  }
}
