import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-copy-application',
  templateUrl: './copy-application.component.html',
  styleUrls: ['./copy-application.component.less']
})
export class CopyApplicationComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().application];
  typeTitle = this.i18n.get('common_application_label');
  routerType = 'copy';

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
