import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-dorado-file-system',
  templateUrl: './dorado-file-system.component.html',
  styleUrls: ['./dorado-file-system.component.less']
})
export class DoradoFileSystemComponent implements OnInit {
  header = this.i18n.get('common_nas_file_system_label');
  resourceType = ResourceType.Storage;
  childResourceType = this.appUtilsService.isDistributed
    ? [DataMap.Resource_Type.ndmp.value]
    : [
        DataMap.Resource_Type.NASFileSystem.value,
        DataMap.Resource_Type.ndmp.value
      ];

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {}
}
