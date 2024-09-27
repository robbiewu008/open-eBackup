import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-copy-file-service',
  templateUrl: './copy-file-service.component.html',
  styleUrls: ['./copy-file-service.component.less']
})
export class CopyFileServiceComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().fileService];
  typeTitle = this.i18n.get('common_file_systems_label');
  routerType = 'copy';

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
