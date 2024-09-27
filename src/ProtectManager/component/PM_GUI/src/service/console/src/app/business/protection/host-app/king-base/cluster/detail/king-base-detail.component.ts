import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-king-base-detail',
  templateUrl: './king-base-detail.component.html',
  styleUrls: ['./king-base-detail.component.less']
})
export class KingBaseDetailComponent implements OnInit {
  data;
  activeIndex = 0;

  constructor() {}

  ngOnInit(): void {}
}
