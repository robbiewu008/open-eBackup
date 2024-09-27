import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-dorado-file-system',
  templateUrl: './dorado-file-system.component.html',
  styleUrls: ['./dorado-file-system.component.less']
})
export class DoradoFileSystemComponent implements OnInit {
  header = this.i18n.get('common_nas_file_system_label');
  resourceType = ResourceType.Storage;
  childResourceType = [DataMap.Resource_Type.NASFileSystem.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
