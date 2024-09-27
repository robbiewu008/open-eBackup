import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-real-time-detect',
  templateUrl: './real-time-detect.component.html',
  styleUrls: ['./real-time-detect.component.less']
})
export class RealTimeDetectComponent implements OnInit {
  activeIndex: string = 'honeypot';

  constructor() {}

  ngOnInit(): void {}
}
