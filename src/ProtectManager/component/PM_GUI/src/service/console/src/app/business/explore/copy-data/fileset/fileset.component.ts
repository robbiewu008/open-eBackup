import { Component, OnInit } from '@angular/core';
import { I18NService, DataMap, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-fileset',
  templateUrl: './fileset.component.html',
  styleUrls: ['./fileset.component.less']
})
export class FilesetComponent implements OnInit {
  header = this.i18n.get('common_fileset_label');
  resourceType = ResourceType.FILESET;
  childResourceType = [DataMap.Resource_Type.fileset.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
