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
  OnInit,
  Input,
  OnChanges,
  SimpleChanges
} from '@angular/core';
import { I18NService, DataMapService } from 'app/shared/services';
import * as echarts from 'echarts';
import { each } from 'lodash';
import { COPY_DATA_ECHARTS_OPTION } from 'app/shared';

@Component({
  selector: 'aui-copy-data-statistical',
  templateUrl: './copy-data-statistical.component.html',
  styleUrls: ['./copy-data-statistical.component.less']
})
export class CopyDataStatisticalComponent implements OnInit, OnChanges {
  echarts;
  legends = [];
  @Input() data;

  constructor(
    private i18n: I18NService,
    public dataMapService: DataMapService
  ) {}

  ngOnChanges(changes: SimpleChanges): void {
    this.getCharts();
  }

  ngOnInit() {
    this.getCharts();
  }

  getCharts() {
    if (!this.data) {
      return;
    }
    const colors = [];
    const seriesData = [];
    const legends = [];
    const typeArr = this.dataMapService.toArray('copydata_validStatus');
    const typeConfig = this.dataMapService.getConfig('copydata_validStatus');
    const total = this.data.Total;
    each(typeArr, option => {
      const num = this.data[option.value] || 0;
      if (option.value === typeConfig.mounting.value) {
        return;
      }
      colors.push(option.color);
      seriesData.push({
        value: num,
        name: option.label
      });
      legends.push({
        color: option.color,
        value: num,
        name: option.label
      });
    });
    this.legends = legends;
    this.echarts = echarts.init(document.getElementById('echarts'));
    this.echarts.setOption({
      tooltip: {
        trigger: 'item',
        formatter: '{b}: {c} ({d}%)'
      },
      graphic: {
        type: 'text',
        left: 'center',
        top: '60%',
        style: {
          text: this.i18n.get('common_total_label'),
          textAlign: 'center',
          fill: '#6C7280',
          fontSize: 16,
          fontWeight: 300
        }
      },
      title: {
        text: total,
        left: 'center',
        top: '38%',
        textStyle: {
          fontSize: 40,
          align: 'center',
          fontWeight: 400
        }
      },
      series: [
        {
          type: 'pie',
          radius: ['78%', '92%'],
          color: colors,
          avoidLabelOverlap: false,
          hoverAnimation: false,
          label: {
            show: false,
            position: 'center'
          },
          emphasis: {
            show: false
          },
          labelLine: {
            show: false
          },
          data: seriesData,
          itemStyle: {
            borderWidth: 1,
            borderColor: '#ffffff'
          }
        }
      ]
    });
  }
}
