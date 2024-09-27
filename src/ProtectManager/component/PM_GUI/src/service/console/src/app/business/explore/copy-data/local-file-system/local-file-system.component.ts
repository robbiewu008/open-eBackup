import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-local-file-system',
  templateUrl: './local-file-system.component.html',
  styleUrls: ['./local-file-system.component.less']
})
export class LocalFileSystemComponent implements OnInit {
  header = this.i18n.get('common_local_file_system_label');
  resourceType = ResourceType.Storage;
  childResourceType = [DataMap.Resource_Type.LocalFileSystem.value];

  constructor(private i18n: I18NService) {}

  ngOnInit() {}
}
