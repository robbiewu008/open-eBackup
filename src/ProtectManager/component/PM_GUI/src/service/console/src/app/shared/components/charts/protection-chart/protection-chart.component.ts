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
import { THEME_TRIGGER_ACTION, ThemeEnum } from 'app/shared/consts';
import { GlobalService, I18NService } from 'app/shared/services';
import { getAppTheme } from 'app/shared/utils';
import * as echarts from 'echarts';

@Component({
  selector: 'protection-chart',
  templateUrl: './protection-chart.component.html',
  styleUrls: ['./protection-chart.component.less']
})
export class ProtectionChartComponent implements OnInit {
  private _protection;
  protectionRate: number | string = 100;
  @Input()
  set protection(value) {
    this._protection = value;
    this.updateChart();
    let protectionRate =
      (value.protected / (value.protected + value.unprotected)) * 100;
    if (isNaN(protectionRate)) {
      this.protectionRate = 0;
    } else {
      this.protectionRate = protectionRate.toFixed(0);
    }
  }

  get protection() {
    return this._protection;
  }

  protectionChart: any;
  protectionOption: any;
  constructor(
    public el: ElementRef,
    private i18n: I18NService,
    private globalService: GlobalService
  ) {}
  ngOnInit(): void {
    this.createChart();
    this.globalService.getState(THEME_TRIGGER_ACTION).subscribe(() => {
      this.protectionOption?.series?.forEach(item => {
        item.color = ['#3388FF', this.getChartColor(), 'rgba(0, 0, 0, 0)'];
      });
      this.updateChart();
    });
  }

  getChartColor() {
    return getAppTheme(this.i18n) === ThemeEnum.dark ? '#333' : '#f2f2f2';
  }

  createChart() {
    this.protectionChart = echarts.init(
      this.el.nativeElement.querySelector('#protection-chart')
    );
    this.protectionOption = {
      series: [
        {
          name: 'Access From',
          type: 'pie',
          radius: ['92%', '100%'],
          color: ['#3388FF', this.getChartColor(), 'rgba(0, 0, 0, 0)'],
          itemStyle: {
            borderRadius: 18
          },
          label: {
            show: false,
            position: 'center'
          },
          startAngle: 225,
          endAngle: 315,
          data: [
            { value: 0, name: 'protected' },
            { value: 100, name: 'unprotected' },
            { value: 33, name: 'placeholder' } //为前两项的三分之一，需要考虑前两项为0的情况
          ],
          hoverAnimation: false
        },
        {
          name: 'Access From',
          type: 'pie',
          radius: ['87%', '90%'],
          color: ['#3388FF', this.getChartColor(), 'rgba(0, 0, 0, 0)'],
          itemStyle: {
            borderRadius: 8
          },
          label: {
            show: false,
            position: 'center'
          },
          startAngle: 225,
          endAngle: 315,
          data: [
            { value: 0, name: 'protected' },
            { value: 100, name: 'unprotected' },
            { value: 33, name: 'placeholder' }
          ],
          hoverAnimation: false
        }
      ]
    };
    this.protectionChart.setOption(this.protectionOption);
  }
  updateChart() {
    this.protectionOption?.series?.forEach(item => {
      if (!this._protection.unprotected && !this._protection.protected) {
        item.data = [
          { value: 0, name: 'protected' },
          { value: 3, name: 'unprotected' },
          {
            value: 1,
            name: 'placeholder'
          }
        ];
      } else {
        item.data = [
          { value: this._protection.protected, name: 'protected' },
          { value: this._protection.unprotected, name: 'unprotected' },
          {
            value:
              (this._protection.protected + this._protection.unprotected) / 3,
            name: 'placeholder'
          }
        ];
      }
    });

    this.protectionChart?.setOption(this.protectionOption);
  }
}
