import { Component, OnInit } from '@angular/core';
import { DataMap, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-commonshare',
  templateUrl: './commonshare.component.html',
  styleUrls: ['./commonshare.component.less']
})
export class CommonshareComponent implements OnInit {
  header = DataMap.Resource_Type.commonShare.label;
  resourceType = ResourceType.Agentless;
  childResourceType = [DataMap.Resource_Type.commonShare.value];
  constructor() {}

  ngOnInit(): void {}
}
