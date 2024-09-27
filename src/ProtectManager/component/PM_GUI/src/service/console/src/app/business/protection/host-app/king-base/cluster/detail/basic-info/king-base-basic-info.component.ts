import { Component, OnInit, Input } from '@angular/core';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-king-base-basic-info',
  templateUrl: './king-base-basic-info.component.html',
  styleUrls: ['./king-base-basic-info.component.less']
})
export class KingBaseBasicInfoComponent implements OnInit {
  @Input() data;
  online;

  constructor() {}

  ngOnInit(): void {
    this.online =
      this.data.linkStatus === DataMap.resource_LinkStatus_Special.normal.value;
  }
}
