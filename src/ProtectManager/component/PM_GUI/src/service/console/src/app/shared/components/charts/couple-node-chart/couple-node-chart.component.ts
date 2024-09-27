import { Component, ElementRef, OnInit } from '@angular/core';
import { ClustersApiService, DataMap } from 'app/shared';
import { CookieService, I18NService } from 'app/shared/services';
import * as echarts from 'echarts';
import { assign } from 'lodash';

@Component({
  selector: 'aui-couple-node-chart',
  templateUrl: './couple-node-chart.component.html',
  styleUrls: ['./couple-node-chart.component.less']
})
export class CoupleNodeChartComponent implements OnInit {
  nodeCharts: any;
  chartsOptions;
  onlineNode = 0;
  offlineNode = 0;
  isAllCluster = true;
  constructor(
    private el: ElementRef,
    private i18n: I18NService,
    public cookieService: CookieService,
    public clustersApiService: ClustersApiService
  ) {}

  ngOnInit(): void {
    this.getAllCusterShow();
    this.createChart();
    this.getNodeInfo();
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
    this.nodeCharts = echarts.init(
      this.el.nativeElement.querySelector('#nodes-chart')
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
          name: 'node',
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
            { value: 0, name: this.i18n.get('common_online_node_label') },
            { value: 0, name: this.i18n.get('common_offline_node_label') }
          ]
        }
      ]
    };
    this.nodeCharts.setOption(this.chartsOptions);
    this.nodeCharts.on('mouseover', () => {
      this.nodeCharts.dispatchAction({
        type: 'downplay'
      });
    });
  }

  updateChart(res) {
    this.nodeCharts.setOption({
      graphic: {
        id: 'total',
        style: {
          text: res.onlineNode + res.offlineNode
        }
      },
      series: {
        data: [
          {
            value: res.onlineNode,
            name: this.i18n.get('common_online_node_label')
          },
          {
            value: res.offlineNode,
            name: this.i18n.get('common_offline_node_label')
          }
        ]
      }
    });
    // 无数据
    if (res.onlineNode === 0 && res.offlineNode === 0) {
      this.nodeCharts.setOption({
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

  getNodeInfo() {
    const params = {
      pageNo: 0,
      pageSize: 20,
      akLoading: false
    };
    this.clustersApiService.pageQueryPacificNodes(params).subscribe(res => {
      this.onlineNode = res.records.filter(item => {
        return (
          Number(item.status) === DataMap.DistributedClusterStatus.healthy.value
        );
      }).length;

      this.offlineNode = res.records.filter(item => {
        return (
          Number(item.status) ===
          DataMap.DistributedClusterStatus.abnormal.value
        );
      }).length;

      this.updateChart(
        assign(
          {},
          {
            onlineNode: this.onlineNode,
            offlineNode: this.offlineNode
          }
        )
      );
    });
  }
}
