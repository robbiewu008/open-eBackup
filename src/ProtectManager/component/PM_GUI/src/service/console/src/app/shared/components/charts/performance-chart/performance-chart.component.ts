import {
  Component,
  ElementRef,
  Input,
  OnChanges,
  OnDestroy,
  OnInit,
  SimpleChanges
} from '@angular/core';
import { OptionItem } from '@iux/live';
import {
  I18NService,
  CAPACITY_UNIT,
  CapacityCalculateLabel,
  GlobalService
} from 'app/shared';
import { PerformanceApiDescService } from 'app/shared/api/services';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import * as echarts from 'echarts';
import { assign, filter, isEmpty, isNaN, reduce, size, zip } from 'lodash';
import { Subscription, timer } from 'rxjs';

@Component({
  selector: 'aui-performance-chart',
  templateUrl: './performance-chart.component.html',
  styleUrls: ['./performance-chart.component.css'],
  providers: [CapacityCalculateLabel]
})
export class PerformanceChartComponent implements OnInit, OnDestroy, OnChanges {
  @Input() curNode;
  loading = false;
  onResizeCallback = [];

  unitconst = CAPACITY_UNIT;
  performanceTimeOptions: OptionItem[];
  performanceTimeOption;
  readAvg = 0;
  writeAvg = 0;
  startTime: number;
  endTime: number;
  performanceSub: Subscription = new Subscription();
  showChart = false;
  noData = this.i18n.get('common_no_data_label');

  timeType = {
    lastMinute: 0,
    lastHour: 1,
    last24Hour: 2,
    lastWeek: 3,
    lastMonth: 4,
    lastYear: 5
  };

  capacityChart;

  constructor(
    private el: ElementRef,
    public i18n: I18NService,
    private globalService: GlobalService,
    public performanceApiService: PerformanceApiDescService,
    public systemTimeService: SystemTimeService,
    public capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnChanges(changes: SimpleChanges): void {
    if (!isEmpty(changes.curNode?.previousValue)) {
      this.loading = true;
      this.getPerformance();
    }
  }

  ngOnInit() {
    this.initConditions();
    this.initTimeOptios();
    this.performanceTimer();
  }

  ngOnDestroy() {
    this.performanceSub.unsubscribe();
  }

  initTimeOptios() {
    this.performanceTimeOptions = [
      {
        label: this.i18n.get('common_last_five_minute_label'),
        value: this.timeType.lastMinute,
        isLeaf: true
      },
      {
        label: this.i18n.get('common_last_half_hour_label'),
        value: this.timeType.lastHour,
        isLeaf: true
      },
      {
        label: this.i18n.get('common_last_day_label'),
        value: this.timeType.last24Hour,
        isLeaf: true
      },
      {
        label: this.i18n.get('common_last_week_label'),
        value: this.timeType.lastWeek,
        isLeaf: true
      },
      {
        label: this.i18n.get('common_last_month_label'),
        value: this.timeType.lastMonth,
        isLeaf: true
      },
      {
        label: this.i18n.get('common_last_year_label'),
        value: this.timeType.lastYear,
        isLeaf: true
      }
    ];
    this.performanceTimeOption = this.performanceTimeOptions[2].value;
  }

  initConditions() {
    const currentTime = new Date().getTime();
    this.startTime = currentTime - 300 * 1000;
    this.endTime = currentTime;
  }

  addZero(num) {
    return num < 10 ? '0' + num : num;
  }

  toDateTime(ms) {
    const time = new Date(ms);
    return (
      time.getFullYear() +
      '-' +
      this.addZero(time.getMonth() + 1) +
      '-' +
      this.addZero(time.getDate()) +
      ' ' +
      this.addZero(time.getHours()) +
      ':' +
      this.addZero(time.getMinutes()) +
      ':' +
      this.addZero(time.getSeconds())
    );
  }

  selectChange(e) {
    this.loading = true;
    this.getPerformance();
  }

  creatPerformanceChart(seriesData, maxBindWidth) {
    const node: any = document.getElementById('performance-chart');
    if (!node) {
      return;
    }
    if (isEmpty(this.capacityChart)) {
      this.capacityChart = echarts.init(
        this.el.nativeElement.querySelector('#performance-chart')
      );
    }
    window.addEventListener('resize', () => {
      this.capacityChart.resize();
    });
    const capacityOption = {
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
            text: `KB/s`,
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
        backgroundColor: '#6F6F6F',
        axisPointer: {
          type: 'line',
          z: 10,
          lineStyle: {
            color: '#d7dae2'
          }
        },
        extraCssText: 'border-radius: 0; padding: 12px 16px',
        formatter: params => {
          let tip = `<div style='font-size: 12px; color: #D4D9E6'>${
            isNaN(+params[0].axisValue)
              ? params[0].axisValue
              : this.toDateTime(+params[0].axisValue)
          }</div>`;
          params.forEach(res => {
            const value = this.capacityCalculateLabel.transform(
              res.data[1],
              '1.3-3',
              CAPACITY_UNIT.KB,
              true
            );
            tip += `<div style='font-size: 12px; color: #D4D9E6; padding-right:10px;'>
                  <span style='font-size: 12px; color: #F7FAFF;'>${value}/s</span>
                  </div>`;
          });
          return tip;
        }
      },
      xAxis: {
        splitLine: {
          show: false
        },
        interval: (() => {
          switch (this.performanceTimeOption) {
            case 0:
              return 50 * 1000;
            case 1:
              return 5 * 60 * 1000;
            case 2:
              return 4 * 3600 * 1000;
            case 3:
              return 28 * 3600 * 1000;
            case 4:
              return 5 * 24 * 3600 * 1000;
            case 5:
              return 61 * 24 * 3600 * 1000;
            default:
              return 50 * 1000;
          }
        })(),
        min: (() => {
          switch (this.performanceTimeOption) {
            case 0:
              return this.endTime - 6 * 50 * 1000;
            case 1:
              return this.endTime - 6 * 5 * 60 * 1000;
            case 2:
              return this.endTime - 6 * 4 * 3600 * 1000;
            case 3:
              return this.endTime - 6 * 28 * 3600 * 1000;
            case 4:
              return this.endTime - 6 * 5 * 24 * 3600 * 1000;
            case 5:
              return this.endTime - 365 * 24 * 3600 * 1000;
            default:
              return this.endTime - 6 * 50 * 1000;
          }
        })(),
        max: this.endTime,
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
          color: '#9EA4B3',
          formatter: params => {
            params = this.toDateTime(params);
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
        max: maxBindWidth,
        minInterval: 0.1,
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
      series: [
        {
          data: seriesData,
          type: 'line',
          color: '#6e94fa',
          showSymbol: seriesData.length === 1,
          itemStyle: {
            borderWidth: 3,
            lineStyle: {
              width: 2
            }
          }
        }
      ]
    };
    this.capacityChart.setOption(capacityOption);
    this.addEventOnResize(() => {
      this.capacityChart.resize();
    })();
  }

  getPerformance(mask = true) {
    this.systemTimeService.getSystemTime(false).subscribe(res => {
      this.endTime = new Date(res.time).getTime();
      if (!this.endTime) {
        this.endTime = new Date(res.time.replace(/-/g, '/')).getTime();
      }
      switch (this.performanceTimeOption) {
        case 0:
          this.startTime = this.endTime - 300 * 1000;
          break;
        case 1:
          this.startTime = this.endTime - 1800 * 1000;
          break;
        case 2:
          this.startTime = this.endTime - 24 * 3600 * 1000;
          break;
        case 3:
          this.startTime = this.endTime - 7 * 24 * 3600 * 1000;
          break;
        case 4:
          this.startTime = this.endTime - 30 * 24 * 3600 * 1000;
          break;
        case 5:
          this.startTime = this.endTime - 365 * 24 * 3600 * 1000;
          break;
        default:
          break;
      }
      const params = {
        indicatorList: [3],
        staticsMode: 'avg',
        startTime: this.startTime,
        endTime: this.endTime,
        akLoading: false,
        akDoException: false
      };
      if (this.curNode) {
        assign(params, {
          clustersId: this.curNode.clusterId,
          clustersType: this.curNode.clusterType,
          memberEsn: this.curNode.memberEsn || ''
        });
      }
      this.performanceApiService.getPerformanceUsingGET(params).subscribe(
        res => {
          this.loading = false;
          this.showChart = true;
          if (isEmpty(res) || isEmpty(res[0].indicatorValues)) {
            this.readAvg = 0;
            this.creatPerformanceChart([], 1);
            return;
          }
          const xAxisDataTemp = res[0].timestamps;
          const seriesReadData = res[0].indicatorValues;
          const seriesReadDataTemp = filter(seriesReadData, item => {
            return !isNaN(+item);
          });
          const maxRead = Math.ceil(
            Math.max.apply(null, seriesReadDataTemp) * 1.5
          );
          const maxBindWidth = maxRead || 1;
          const allReadData = reduce(
            seriesReadData,
            (sum, n) => {
              return +sum + (isNaN(+n) ? 0 : +n);
            },
            0
          );
          this.readAvg = +(allReadData / size(seriesReadData)).toFixed(2);
          const seriesData = zip(xAxisDataTemp, seriesReadData);
          this.creatPerformanceChart(seriesData, maxBindWidth);
        },
        err => {
          this.loading = false;
          if (err.error && err.error.errorCode === '1677930005') {
            this.showChart = false;
          } else {
            this.showChart = true;
            this.readAvg = 0;
            this.creatPerformanceChart([], 1);
          }
          this.performanceSub.unsubscribe();
        }
      );
    });
  }

  refresh() {
    this.loading = true;
    this.getPerformance();
  }

  performanceTimer() {
    this.getPerformance(true);
    this.performanceSub = timer(0, 10 * 1e3).subscribe(res => {
      this.getPerformance(false);
    });
  }

  addEventOnResize(fn) {
    this.onResizeCallback.push(fn);
    return () => {
      window.onresize = () => {
        this.onResizeCallback.forEach(callback => {
          callback();
        });
      };
    };
  }
}
