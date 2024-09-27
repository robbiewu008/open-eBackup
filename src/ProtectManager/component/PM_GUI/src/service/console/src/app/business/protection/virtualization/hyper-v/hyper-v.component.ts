import { Component, OnInit } from '@angular/core';
import { ResourceType } from 'app/shared';

@Component({
  selector: 'aui-hyper-v',
  templateUrl: './hyper-v.component.html',
  styleUrls: ['./hyper-v.component.less']
})
export class HyperVComponent implements OnInit {
  type = ResourceType.HYPERV;
  constructor() {}

  ngOnInit() {}
}
