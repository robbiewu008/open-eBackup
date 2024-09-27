import { Component, OnInit, AfterViewInit } from '@angular/core';

@Component({
  selector: 'aui-click-house',
  templateUrl: './click-house.component.html',
  styleUrls: ['./click-house.component.less']
})
export class ClickHouseComponent implements OnInit, AfterViewInit {
  constructor() {}

  ngOnInit() {}

  ngAfterViewInit() {}

  onChange() {
    this.ngOnInit();
    this.ngAfterViewInit();
  }
}
