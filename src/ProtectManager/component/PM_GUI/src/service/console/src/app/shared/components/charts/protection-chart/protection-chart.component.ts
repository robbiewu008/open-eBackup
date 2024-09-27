import { Component, OnInit, ElementRef, Input } from '@angular/core';
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
  constructor(public el: ElementRef) {}
  ngOnInit(): void {
    this.createChart();
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
          color: ['#3388FF', '#333', 'rgba(0, 0, 0, 0)'],
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
            { value: 100, name: 'protected' },
            { value: 0, name: 'unprotected' },
            { value: 33, name: 'placeholder' } //为前两项的三分之一，需要考虑前两项为0的情况
          ],
          hoverAnimation: false
        },
        {
          name: 'Access From',
          type: 'pie',
          radius: ['87%', '90%'],
          color: ['#3388FF', '#333', 'rgba(0, 0, 0, 0)'],
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
            { value: 100, name: 'protected' },
            { value: 0, name: 'unprotected' },
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
      item.data = [
        { value: this._protection.protected, name: 'protected' },
        { value: this._protection.unprotected, name: 'unprotected' },
        {
          value:
            (this._protection.protected + this._protection.unprotected) / 3,
          name: 'placeholder'
        }
      ];
    });
    this.protectionChart?.setOption(this.protectionOption);
  }
}
