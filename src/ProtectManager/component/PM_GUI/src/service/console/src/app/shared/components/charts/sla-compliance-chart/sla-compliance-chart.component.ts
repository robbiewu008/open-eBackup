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
import { Component, ElementRef, OnInit } from '@angular/core';
import { ApiMultiClustersService, ProjectedObjectApiService } from 'app/shared';
import { CookieService, I18NService } from 'app/shared/services';
import * as echarts from 'echarts';
import { assign } from 'lodash';

@Component({
  selector: 'aui-sla-compliance-chart',
  templateUrl: './sla-compliance-chart.component.html',
  styleUrls: ['./sla-compliance-chart.component.less']
})
export class SlaComplianceChartComponent implements OnInit {
  slaCharts: any;
  chartsOptions;
  satisfiedItems = 0;
  unsatisfiedItems = 0;
  isAllCluster = true;
  constructor(
    private el: ElementRef,
    private i18n: I18NService,
    public cookieService: CookieService,
    private multiClustersServiceApi: ApiMultiClustersService,
    private projectedObjectApiService: ProjectedObjectApiService
  ) {}

  ngOnInit() {
    this.getAllCusterShow();
    this.createChart();
    this.getSlaCompliance();
  }

  getAllCusterShow() {
    const clusterObj = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isAllCluster =
      !clusterObj ||
      (clusterObj && clusterObj['icon'] === 'aui-icon-all-cluster');
  }

  createChart() {
    this.slaCharts = echarts.init(
      this.el.nativeElement.querySelector('#sla-complicance-chart')
    );
    this.chartsOptions = {
      graphic: [
        {
          id: 'total',
          type: 'text',
          left: 'center',
          top: '42%',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: '0',
            textAlign: 'center',
            fill: '#282B33',
            width: 30,
            height: 30,
            fontSize: 32
          }
        },
        {
          type: 'text',
          left: 'center',
          top: '58%',
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
          name: 'sla',
          type: 'pie',
          color: ['#7adfa0', '#e6ebf5'],
          radius: [84, 101],
          avoidLabelOverlap: false,
          cursor: 'unset',
          label: {
            show: false
          },
          emphasis: {
            scale: false,
            label: {
              show: false
            }
          },
          labelLine: {
            show: false
          },
          data: [
            { value: 0, name: this.i18n.get('common_sla_compliance_y_label') },
            { value: 0, name: this.i18n.get('common_sla_compliance_n_label') }
          ]
        }
      ]
    };
    this.slaCharts.setOption(this.chartsOptions);
    this.slaCharts.on('mouseover', () => {
      this.slaCharts.dispatchAction({
        type: 'downplay'
      });
    });
  }

  updateChart(res) {
    this.satisfiedItems = res.inCompliance;
    this.unsatisfiedItems = res.outOfCompliance;
    this.slaCharts.setOption({
      graphic: {
        id: 'total',
        style: {
          text: res.inCompliance + res.outOfCompliance
        }
      },
      series: {
        data: [
          {
            value: res.inCompliance,
            name: this.i18n.get('common_sla_compliance_y_label')
          },
          {
            value: res.outOfCompliance,
            name: this.i18n.get('common_sla_compliance_n_label')
          }
        ]
      }
    });
    // 无数据
    if (this.satisfiedItems === 0 && res.outOfCompliance === 0) {
      this.slaCharts.setOption({
        graphic: {
          id: 'mask',
          type: 'ring',
          left: 'center',
          top: 'center',
          z: 2,
          zlevel: 101,
          cursor: 'unset',
          shape: {
            r: 101,
            r0: 82
          },
          style: {
            fill: '#e6ebf5',
            width: 30,
            height: 30
          }
        }
      });
    }
  }

  getSlaCompliance() {
    if (this.isAllCluster) {
      this.multiClustersServiceApi
        .getMultiClusterSla({ akLoading: false })
        .subscribe(res => this.updateChart(res));
    } else {
      this.projectedObjectApiService
        .queryProtectionCompliance({ akLoading: false })
        .subscribe(res =>
          this.updateChart(
            assign(
              {},
              {
                inCompliance: res.in_compliance,
                outOfCompliance: res.out_of_compliance
              }
            )
          )
        );
    }
  }
}
