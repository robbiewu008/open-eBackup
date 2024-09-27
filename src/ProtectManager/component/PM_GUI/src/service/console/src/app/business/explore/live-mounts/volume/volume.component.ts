import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-volume',
  templateUrl: './volume.component.html',
  styleUrls: ['./volume.component.less']
})
export class VolumeComponent implements OnInit {
  header = this.i18n.get('protection_volume_label');
  resourceType = ResourceType.HOST;
  childResourceType = [DataMap.Resource_Type.volume.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
