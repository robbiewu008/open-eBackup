import { DatePipe } from '@angular/common';
import {
  Component,
  ElementRef,
  Input,
  OnChanges,
  OnInit,
  SimpleChanges
} from '@angular/core';
import { Router } from '@angular/router';
import {
  CapacityApiService,
  CAPACITY_UNIT,
  CookieService,
  GlobalService,
  I18NService,
  LANGUAGE
} from 'app/shared';
import * as echarts from 'echarts';
import {
  assign,
  filter,
  find,
  first,
  floor,
  isEmpty,
  maxBy,
  size,
  union,
  isUndefined
} from 'lodash';
import { CapacityCalculateLabel } from './../../../pipe/capacity.pipe';

@Component({
  selector: 'aui-capacity-forecast-chart',
  templateUrl: './capacity-forecast-chart.component.html',
  styleUrls: ['./capacity-forecast-chart.component.css'],
  providers: [DatePipe, CapacityCalculateLabel]
})
export class CapacityForecastChartComponent implements OnInit, OnChanges {
  @Input() curNode;
  isNoData = false;
  reachTo80;
  reachTo100;
  reachTo80Symbol;
  reachTo100Symbol;
  forecastDatas;
  forecastData = [];
  systemTime = new Date().getTime() / 1000 / 3600 / 24;
  reachTo80Tip = '';
  reachTo100Tip = '';
  spaceLabel = this.i18n.language === LANGUAGE.CN ? '' : ' ';
  constructor(
    public el: ElementRef,
    public i18n: I18NService,
    public datePipe: DatePipe,
    private router: Router,
    public cookieService: CookieService,
    private globalService: GlobalService,
    public capacityApiService: CapacityApiService,
    public capacityCalculateLabel: CapacityCalculateLabel
  ) {}
  ngOnChanges(changes: SimpleChanges): void {
    if (!isEmpty(changes.curNode?.previousValue)) {
      this.getCapcacityTendency();
    }
  }

  ngOnInit() {
    this.getCapcacityTendency();
  }

  getCapcacityTendency() {
    const params = { akDoException: false, akLoading: false };
    if (this.curNode) {
      assign(params, {
        clustersId: this.curNode.clusterId,
        clustersType: this.curNode.clusterType,
        memberEsn: this.curNode.memberEsn || ''
      });
    }
    this.capacityApiService
      .queryClusterStorageTendencyUsingGET(params)
      .subscribe({
        next: res => {
          if (!size(res.existingDatas) && !size(res.forecastDatas)) {
            this.isNoData = true;
            return;
          }
          this.forecastData = res.forecastDatas;
          this.createForecastCharts(res);
        },
        error: () => {
          this.isNoData = true;
        }
      });
  }

  createForecastCharts(res) {
    const totalCap = [],
      usedCap = [],
      percentage = [],
      timestamp = [],
      timeObj = {};
    this.forecastDatas = union(res.existingDatas, res.forecastDatas);
    this.forecastDatas.forEach((obj: any) => {
      totalCap.push(obj.totalCapacity);
      usedCap.push(obj.usedCapacity);
      percentage.push(obj.percentage * 100);
      const key = this.datePipe.transform(obj.timestamp, 'yyyy-MM');
      const value = this.datePipe.transform(
        obj.timestamp,
        'yyyy-MM-dd HH:mm:ss'
      );
      if (isEmpty(timeObj[key])) {
        timeObj[key] = [value];
      } else {
        timeObj[key].push(value);
      }
      timestamp.push(value);
    });

    const existingMaxPercentageObj: any = maxBy(
      res.existingDatas,
      'percentage'
    );
    // 如果forecastDatas为[]maxBy结果为undefined
    const forecastMaxPercentageObj: any = maxBy(
      res.forecastDatas,
      'percentage'
    );
    const maxPercentageObj: any = maxBy(
      union(res.existingDatas, res.forecastDatas),
      'percentage'
    );

    if (maxPercentageObj.percentage < 0.8) {
      const intervalTime =
        maxPercentageObj.timestamp / 1000 / 3600 / 24 - this.systemTime;
      this.reachTo100Symbol = this.reachTo80Symbol = '>';
      this.reachTo100 = this.reachTo80 = `${floor(intervalTime) + 1}`;
    } else {
      if (!isUndefined(forecastMaxPercentageObj)) {
        let intervalTime =
          forecastMaxPercentageObj.timestamp / 1000 / 3600 / 24 -
          this.systemTime;
        this.reachTo100 = `${floor(intervalTime) + 1}`;
      } else {
        this.reachTo100 = '0';
      }
      if (existingMaxPercentageObj.percentage >= 0.8) {
        this.reachTo80 = '0';
      } else {
        const forecastMaxPercentageObjs: any = filter(
          res.forecastDatas,
          (resu: any) => {
            return resu.percentage >= 0.8;
          }
        );

        // filter后可能最终结果为[]
        if (size(forecastMaxPercentageObjs)) {
          this.reachTo80 = `${floor(
            first(forecastMaxPercentageObjs)['timestamp'] / 1000 / 3600 / 24 -
              this.systemTime
          ) + 1}`;
        } else {
          this.reachTo80 = '0';
        }
      }
      this.reachTo100Symbol = '>';
    }

    this.reachTo80Tip = this.i18n.get('common_capacity_reach80_label', [
      this.reachTo80Symbol,
      this.reachTo80
    ]);
    this.reachTo100Tip = this.i18n.get('common_capacity_reach100_label', [
      this.reachTo100Symbol,
      this.reachTo100
    ]);
    const capacityChart = echarts.init(
      this.el.nativeElement.querySelector('#forecast-chart')
    );
    const temp = Math.round(size(timestamp) / 6) || 1;
    let indexNum = 0,
      showIndex = 0;
    const capacityOption = {
      grid: {
        left: '8%',
        top: 20,
        right: '8%',
        bottom: 25
      },
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
          const usageText = this.i18n.get(
            'common_capcity_usage_label',
            [],
            true
          );
          const totalText = this.i18n.get(
            'common_total_capacity_label',
            [],
            true
          );
          const usedText = this.i18n.get('common_used_capcity_label', [], true);
          const capacityObj: any = find(this.forecastDatas, (d: any) => {
            return (
              this.datePipe.transform(d.timestamp, 'yyyy-MM-dd HH:mm:ss') ===
              params[0].axisValue
            );
          });

          let usedCapacity =
            (capacityObj.totalCapacity * params[0].value) / 100;
          if (!isEmpty(capacityObj)) {
            usedCapacity = capacityObj.usedCapacity;
          }

          const used = this.capacityCalculateLabel.transform(
            usedCapacity,
            '1.3-3',
            CAPACITY_UNIT.MB,
            true
          );

          const total = this.capacityCalculateLabel.transform(
            capacityObj.totalCapacity,
            '1.3-3',
            CAPACITY_UNIT.MB,
            true
          );

          const tip = `<div style='font-size: 12px; color: #D4D9E6'>${params[0].name}</div>
          <div style='font-size: 12px; color: #D4D9E6; padding-right:10px;'>${usageText}
                  <span style='font-size: 12px; color: #F7FAFF;'>${params[0].value}%</span>
                  </div>
                  <div style='font-size: 12px; color: #D4D9E6; padding-right:10px;'>${totalText}
                  <span style='font-size: 12px; color: #F7FAFF;'>${total}</span>
                  </div>
                  <div style='font-size: 12px; color: #D4D9E6; padding-right:10px;'>${usedText}
                  <span style='font-size: 12px; color: #F7FAFF;'>${used}</span>
                  </div>`;
          return tip;
        }
      },
      xAxis: {
        type: 'category',
        data: timestamp,
        boundaryGap: false,
        axisTick: {
          alignWithLabel: true,
          lineStyle: {
            width: 2
          },
          axisLabel: {
            fontSize: 10
          },
          show: false
        },
        axisLine: {
          lineStyle: {
            color: '#E6EBF5',
            width: 2
          }
        },
        axisLabel: {
          color: '#9EA4B3',
          interval: (index, val) => {
            if (
              (showIndex === index && indexNum < 6) ||
              index === size(timestamp) - 1
            ) {
              showIndex += temp;
              indexNum++;
              return true;
            }
          },
          formatter: val => {
            return val.slice(0, 10);
          }
        }
      },
      yAxis: {
        type: 'value',
        min: 0,
        max: 100,
        interval: 20,
        axisLabel: {
          show: true,
          formatter: '{value}%',
          fontSize: 10,
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
      visualMap: [
        {
          show: false,
          seriesIndex: 1,
          dimension: 0,
          pieces: [
            {
              gt: 0,
              lte: res.existingDatas.length,
              color: '#6e94fa'
            },
            { gt: res.existingDatas.length, color: 'transparent' }
          ]
        },
        {
          show: false,
          seriesIndex: 0,
          pieces: [
            { gt: 0, lte: res.peakPoint, color: '#6e94fa' },
            { gt: res.peakPoint, color: '#f3aaac' }
          ]
        }
      ],
      series: [
        {
          data: percentage,
          showSymbol: false,
          markLine: {
            data: [{ name: 'danger', yAxis: res.peakPoint || 100 }],
            label: {
              show: false
            },
            silent: true,
            symbol: 'none',
            animation: false,
            lineStyle: {
              type: 'dashed',
              color: '#f3aaac',
              width: 1
            }
          },
          type: 'line',
          lineStyle: {
            type: 'dashed',
            borderWidth: 2,
            width: 1
          },
          smooth: 0.1
        },
        {
          data: percentage,
          showSymbol: false,
          type: 'line',
          lineStyle: {
            type: 'solid',
            width: 1
          },
          smooth: 0.4,
          areaStyle: {
            color: {
              type: 'linear',
              x: 0,
              y: 0.5,
              x2: 0.33,
              y2: 1,
              colorStops: [],
              global: false // 缺省为 false
            }
          }
        }
      ]
    };
    capacityChart.setOption(capacityOption);
  }
}
