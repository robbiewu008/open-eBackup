import { Input, Component, ViewChild } from '@angular/core';

@Component({
  selector: 'performance',
  templateUrl: './performance.component.html',
  styleUrls: ['./performance.component.less']
})
export class PerformanceComponent {
  @Input() cardInfo: any = {};
  @ViewChild('performanceTimeChart', { static: false }) performanceTimeChart;
  refreshData() {
    this.performanceTimeChart.refreshData();
  }
}
