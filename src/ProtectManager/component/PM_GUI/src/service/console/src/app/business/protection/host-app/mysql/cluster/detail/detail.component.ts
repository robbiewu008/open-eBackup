import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-detail',
  templateUrl: './detail.component.html',
  styleUrls: ['./detail.component.less']
})
export class DetailComponent implements OnInit {
  data;
  activeIndex = 0;

  constructor() {}

  ngOnInit(): void {}
}
