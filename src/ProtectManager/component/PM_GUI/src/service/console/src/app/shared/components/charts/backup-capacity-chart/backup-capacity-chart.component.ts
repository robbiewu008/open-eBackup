import { Component, OnInit, ElementRef } from '@angular/core';
import * as echarts from 'echarts';
import {
  I18NService,
  CapacityApiService,
  CapacityCalculateLabel,
  CookieService,
  ApiQuotaService,
  GlobalService
} from 'app/shared';
import { find, first, get, remove as _remove } from 'lodash';
import { CAPACITY_UNIT, CommonConsts } from 'app/shared/consts';

@Component({
  selector: 'aui-backup-capacity-chart',
  templateUrl: './backup-capacity-chart.component.html',
  styleUrls: ['./backup-capacity-chart.component.css'],
  providers: [CapacityCalculateLabel]
})
export class BackupCapacityChartComponent implements OnInit {
  options;
  unitconst = CAPACITY_UNIT;
  capacity = {} as any;

  constructor(
    private el: ElementRef,
    public i18n: I18NService,
    public cookieService: CookieService,
    private globalService: GlobalService,
    public userQuotaService: ApiQuotaService,
    public capacityApiService: CapacityApiService,
    public capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnInit() {
    this.getData();
  }

  getData() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      userId: this.cookieService.get('userId')
    };

    this.userQuotaService.listUserQuotaInfoUsingGet(params).subscribe(res => {
      const quota = first(get(res, 'records'));
      const freeCapacity = quota?.backupTotalQuota - quota?.backupUsedQuota;
      const data = {
        totalCapacity: quota?.backupTotalQuota,
        usedCapacity: quota?.backupUsedQuota,
        freeCapacity: freeCapacity < 0 ? 0 : freeCapacity
      };
      this.createCapacityChart(data);
    });
  }

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
      CAPACITY_UNIT.BYTE,
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
              value:
                res.totalCapacity === 0 && res.usedCapacity === 0
                  ? 1
                  : res.freeCapacity,
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
              value:
                res.totalCapacity === 0 && res.usedCapacity === 0
                  ? 1
                  : res.freeCapacity,
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
