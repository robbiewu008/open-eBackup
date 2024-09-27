import { Component, ElementRef, OnInit } from '@angular/core';
import { CookieService, DataMapService, I18NService } from 'app/shared';
import {
  AlarmAndEventApiService,
  ApiMultiClustersService
} from 'app/shared/api/services';
import { AlarmColorConsts } from 'app/shared/consts';
import * as echarts from 'echarts';

@Component({
  selector: 'aui-alarm-chart',
  templateUrl: './alarm-chart.component.html',
  styleUrls: ['./alarm-chart.component.css']
})
export class AlarmChartComponent implements OnInit {
  warningItem = 0;
  criticalItem = 0;
  majorItem = 0;
  minorItem = 0;
  totalItem = 0;
  alarmsOption;
  alarmChart;
  alarmConsts = AlarmColorConsts;
  isMultiCluster = true;

  constructor(
    private el: ElementRef,
    public i18n: I18NService,
    public cookieService: CookieService,
    public apiService: AlarmAndEventApiService,
    private dataMapService: DataMapService,
    private multiClustersServiceApi: ApiMultiClustersService
  ) {}

  ngOnInit() {
    this.getAllCusterShow();
    this.initAsyncData();
    this.createAlarmsChart();
  }

  getAllCusterShow() {
    const clusterObj = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isMultiCluster =
      !clusterObj ||
      (clusterObj && clusterObj['icon'] === 'aui-icon-all-cluster');
  }

  initChartData(res) {
    this.criticalItem = res.critical;
    this.warningItem = res.warning;
    this.majorItem = res.major;
    this.totalItem =
      this.criticalItem + this.warningItem + this.minorItem + this.majorItem;

    if (!this.totalItem) {
      this.alarmChart.setOption({
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
            fill: '#545a67',
            width: 30,
            height: 30
          }
        }
      });
    } else {
      this.alarmChart.setOption({
        graphic: {
          id: 'number1',
          type: 'text',
          $action: 'replace',
          left: 'center',
          top: '43%',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: this.totalItem,
            textAlign: 'center',
            fill: '#FFFFFF',
            width: 30,
            height: 30,
            fontSize: 32
          }
        },
        series: {
          data: [
            { value: this.criticalItem, name: this.i18n.get('critical') },
            { value: this.majorItem, name: this.i18n.get('major') },
            { value: this.warningItem, name: this.i18n.get('warning') }
          ]
        }
      });
    }
  }

  initAsyncData() {
    if (this.isMultiCluster) {
      this.multiClustersServiceApi
        .getMultiClusterAlarms({ akLoading: false })
        .subscribe(res => this.initChartData(res));
    } else {
      this.apiService
        .queryAlarmCountBySeverityUsingGET({ akLoading: false })
        .subscribe(res => this.initChartData(res));
    }
  }

  createAlarmsChart() {
    this.alarmChart = echarts.init(
      this.el.nativeElement.querySelector('#alarm-chart')
    );
    this.alarmsOption = {
      graphic: [
        {
          id: 'number1',
          type: 'text',
          left: 'center',
          top: '43%',
          z: 2,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: '0',
            textAlign: 'center',
            fill: '#FFFFFF',
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
            fill: '#545A67',
            width: 30,
            height: 30
          }
        }
      ],
      series: [
        {
          name: 'Alarms',
          type: 'pie',
          radius: [84, 101],
          color: ['#F45C5E', '#FA8E5A', '#FDCA5A'],
          avoidLabelOverlap: false,
          selectedMode: false,
          legendHoverLink: false,
          cursor: 'unset',
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
            borderColor: '#24272E'
          },
          data: [
            {
              value: undefined,
              name: 'Critical'
            },
            {
              value: undefined,
              name: 'Major'
            },
            {
              value: undefined,
              name: 'Minor'
            },
            {
              value: undefined,
              name: 'Warning'
            }
          ]
        }
      ]
    };
    this.alarmChart.setOption(this.alarmsOption);
    this.alarmChart.on('mouseover', () => {
      this.alarmChart.dispatchAction({
        type: 'downplay'
      });
    });
  }

  selectChange(e) {}
}
