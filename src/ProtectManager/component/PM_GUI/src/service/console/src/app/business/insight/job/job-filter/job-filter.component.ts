import { Component, OnInit, Output, EventEmitter } from '@angular/core';

@Component({
  selector: 'aui-job-filter',
  templateUrl: './job-filter.component.html',
  styleUrls: ['./job-filter.component.less']
})
export class JobFilterComponent implements OnInit {
  @Output() change = new EventEmitter<any>();

  constructor() {}

  ngOnInit() {}

  refresh() {
    this.change.emit();
  }
}
