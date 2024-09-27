import { Component, OnInit } from '@angular/core';
import { ResourceType } from 'app/shared';

@Component({
  selector: 'aui-apsara-stack',
  templateUrl: './apsara-stack.component.html',
  styleUrls: ['./apsara-stack.component.less']
})
export class ApsaraStackComponent implements OnInit {
  type = ResourceType.ApsaraStack;

  constructor() {}

  ngOnInit(): void {}
}
