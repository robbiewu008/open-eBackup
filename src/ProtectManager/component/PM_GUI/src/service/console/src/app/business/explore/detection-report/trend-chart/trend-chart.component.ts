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
import { DataMap, DataMapService, I18NService } from 'app/shared';
import * as echarts from 'echarts';
import { each, find, isEmpty, last, size } from 'lodash';

@Component({
  selector: 'aui-detection-trend-chart',
  templateUrl: './trend-chart.component.html',
  styleUrls: ['./trend-chart.component.less']
})
export class TrendChartComponent implements OnInit, OnChanges {
  @Input() seriesData = [];
  @Input() isSnapshotReport;
  @Input() largeSizeChart;
  types = [];
  chartTags = [];
  ADD_COLOR = '#63B6F7';
  CHANGE_COLOR = '#62DAD0';
  DELETE_COLOR = '#F4B853';
  SUSPICIOUS_COLOR = '#F45C5E';
  UNINFECTED_COLOR = '#93DCA4';
  currentData;

  constructor(private i18n: I18NService) {}

  ngOnChanges(changes: SimpleChanges) {
    if (!isEmpty(changes.seriesData?.currentValue)) {
      this.initChartData();
      this.initTag(last(this.seriesData));
    }
  }

  ngOnInit() {
    this.initChartData();
  }

  initTag(data) {
    this.types = [
      {
        label: this.i18n.get('explore_file_add_label'),
        content: data?.added_file_count || 0,
        background: this.ADD_COLOR
      },
      {
        label: this.i18n.get('explore_file_change_label'),
        content: data?.changed_file_count || 0,
        background: this.CHANGE_COLOR
      },
      {
        label: this.i18n.get('explore_file_delete_label'),
        content: data?.deleted_file_count || 0,
        background: this.DELETE_COLOR
      },
      {
        label: this.i18n.get('explore_file_suspicious_label'),
        content: data?.infected_file_count || 0,
        background: this.SUSPICIOUS_COLOR
      }
    ];
    this.chartTags = [
      {
        label: this.i18n.get('explore_new_file_num_label'),
        background: this.ADD_COLOR,
        count: true
      },
      {
        label: this.i18n.get('explore_modify_file_count_label'),
        background: this.CHANGE_COLOR,
        count: true
      },
      {
        label: this.i18n.get('explore_delete_file_count_label'),
        background: this.DELETE_COLOR,
        count: true
      },
      {
        label: this.i18n.get('explore_suspicious_file_num_label'),
        background: this.SUSPICIOUS_COLOR,
        count: true
      }
    ];
    if (this.isSnapshotReport) {
      this.chartTags.push(
        {
          label: this.i18n.get('explore_latest_uninfected_label'),
          background: this.UNINFECTED_COLOR,
          count: false
        },
        {
          label: this.i18n.get('explore_infected_snapshot_label'),
          background: this.SUSPICIOUS_COLOR,
          count: false
        }
      );
    }
  }

  createChart(
    timeScale,
    addFileData,
    changedFileData,
    deletedFileData,
    infectedFileData,
    antiStatusData
  ) {
    const node: any = document.getElementById('trend-chart');
    if (!node) {
      return;
    }
    const chart = echarts.init(node);
    const maxyAxis = Math.max(
      ...addFileData,
      ...changedFileData,
      ...deletedFileData,
      ...infectedFileData
    );
    const seriesData = [
      {
        name: this.i18n.get('explore_new_file_num_label'),
        type: 'line',
        smooth: true,
        data: addFileData
      },
      {
        name: this.i18n.get('explore_modify_file_count_label'),
        type: 'line',
        smooth: true,
        data: changedFileData
      },
      {
        name: this.i18n.get('explore_delete_file_count_label'),
        type: 'line',
        smooth: true,
        data: deletedFileData
      },
      {
        name: this.i18n.get('explore_suspicious_file_num_label'),
        type: 'line',
        smooth: true,
        data: infectedFileData
      }
    ];
    const options = {
      grid: {
        y: 40,
        y2: 40,
        x: 40,
        x2: 50,
        containLabel: true
      },
      color: [
        this.ADD_COLOR,
        this.CHANGE_COLOR,
        this.DELETE_COLOR,
        this.SUSPICIOUS_COLOR
      ],
      graphic: [
        {
          id: 'unit',
          type: 'text',
          left: 38,
          top: 10,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: this.i18n.get('explore_unit_number_label'),
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
        backgroundColor: '#6F6F6F',
        extraCssText: 'border-radius: 0; padding: 12px 16px',
        formatter: params => {
          this.currentData = params;
          let tip = `<div style='font-size: 12px; color: #D4D9E6'>${params[0].axisValue}</div>`;
          if (this.isSnapshotReport) {
            const antiStatus = find(this.seriesData, {
              timestamp: params[0].axisValue
            })?.anti_status;
            tip += `<div style="margin-bottom: 10px;"><span style="font-size: 12px; color: #F7FAFF">${
              antiStatus === DataMap.detectionSnapshotStatus.infected.value
                ? this.i18n.get('explore_infected_snapshot_label')
                : this.i18n.get('explore_latest_uninfected_label')
            }</span></div>`;
          }
          params.forEach(res => {
            if (res.seriesName) {
              tip += `<div style='font-size: 12px; color: #D4D9E6; padding-right:10px;'>
                  <span style='font-size: 12px; color: #F7FAFF;'>${
                    res.seriesName
                  }${this.i18n.isEn ? ': ' : '：'}${res.value}</span>
                  </div>`;
            }
          });
          return tip;
        }
      },
      xAxis: {
        type: 'category',
        data: timeScale,
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
          rotate: size(timeScale) > 10 ? 80 : 0,
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
        max: Math.floor(maxyAxis * 1.2),
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
      series: this.isSnapshotReport
        ? [
            ...seriesData,
            {
              name: '',
              type: 'line',
              data: antiStatusData,
              symbol: 'rect',
              symbolSize: 10,
              lineStyle: {
                opacity: 0
              },
              emphasis: {
                scale: 1
              }
            }
          ]
        : seriesData
    };
    chart.setOption(options);
    chart.getZr().on('click', () => {
      if (isEmpty(this.currentData)) {
        return;
      }
      this.initTag({
        added_file_count: this.currentData[0]?.value,
        changed_file_count: this.currentData[1]?.value,
        deleted_file_count: this.currentData[2]?.value,
        infected_file_count: this.currentData[3]?.value
      });
    });
    chart.getZr().on('mousemove', () => {
      chart.getZr().setCursorStyle('pointer');
    });
  }

  initChartData() {
    const timeScale = [];
    const addFileData = [];
    const changedFileData = [];
    const deletedFileData = [];
    const infectedFileData = [];
    const antiStatusData = [];
    each(this.seriesData, item => {
      timeScale.push(item.timestamp);
      addFileData.push(item.added_file_count || 0);
      changedFileData.push(item.changed_file_count || 0);
      deletedFileData.push(item.deleted_file_count || 0);
      infectedFileData.push(item.infected_file_count || 0);
      if (this.isSnapshotReport) {
        antiStatusData.push({
          value: 0,
          itemStyle: {
            color:
              item.anti_status ===
              DataMap.detectionSnapshotStatus.infected.value
                ? this.SUSPICIOUS_COLOR
                : this.UNINFECTED_COLOR
          }
        });
      }
    });
    if (!isEmpty(timeScale)) {
      this.createChart(
        timeScale,
        addFileData,
        changedFileData,
        deletedFileData,
        infectedFileData,
        antiStatusData
      );
    }
  }
}
