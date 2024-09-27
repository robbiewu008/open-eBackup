import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-copy-database',
  templateUrl: './copy-database.component.html',
  styleUrls: ['./copy-database.component.less']
})
export class CopyDatabaseComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().database];
  typeTitle = this.i18n.get('common_database_label');
  routerType = 'copy';

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
