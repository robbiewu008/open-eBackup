import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { ReportsListComponent } from './reports-list/reports-list.component';

@Component({
  selector: 'aui-reports',
  templateUrl: './reports.component.html',
  styleUrls: ['./reports.component.less']
})
export class ReportsComponent implements OnInit, OnDestroy {
  @ViewChild(ReportsListComponent, { static: false })
  reportsListComponent: ReportsListComponent;
  constructor() {}

  ngOnDestroy() {}

  ngOnInit() {}

  onChange() {
    this.reportsListComponent.ngOnInit();
    this.reportsListComponent.ngAfterViewInit();
  }
}
