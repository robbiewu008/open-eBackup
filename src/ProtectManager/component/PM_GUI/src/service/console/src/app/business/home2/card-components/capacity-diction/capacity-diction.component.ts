import { Component, Input, ViewChild } from '@angular/core';

@Component({
  selector: 'capacity-diction',
  templateUrl: './capacity-diction.component.html',
  styleUrls: ['./capacity-diction.component.less']
})
export class CapacityDictionComponent {
  @Input() cardInfo: any = {};
  @ViewChild('capacityDictionChart', { static: false }) capacityDictionChart;
  refreshData() {
    this.capacityDictionChart.refreshData();
  }
}
