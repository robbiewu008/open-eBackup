import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-database-summary',
  templateUrl: './database.component.html',
  styleUrls: ['./database.component.less']
})
export class DatabaseComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().database];
  typeTitle = this.i18n.get('common_database_label');

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {}
}
