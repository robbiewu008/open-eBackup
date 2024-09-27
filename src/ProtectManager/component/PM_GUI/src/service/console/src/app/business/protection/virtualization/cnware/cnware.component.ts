import { Component, OnInit } from '@angular/core';
import { ResourceType } from 'app/shared';

@Component({
  selector: 'aui-cnware',
  templateUrl: './cnware.component.html',
  styleUrls: ['./cnware.component.less']
})
export class CnwareComponent implements OnInit {
  type = ResourceType.CNWARE;
  constructor() {}

  ngOnInit(): void {}
}
