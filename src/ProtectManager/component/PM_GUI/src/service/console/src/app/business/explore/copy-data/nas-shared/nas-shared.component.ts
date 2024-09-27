import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-nas-shared',
  templateUrl: './nas-shared.component.html',
  styleUrls: ['./nas-shared.component.less']
})
export class NasSharedComponent implements OnInit {
  header = this.i18n.get('common_nas_shared_label');
  resourceType = ResourceType.Storage;
  childResourceType = [DataMap.Resource_Type.NASShare.value];

  constructor(private i18n: I18NService) {}

  ngOnInit() {}
}
